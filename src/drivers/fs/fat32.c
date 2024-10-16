///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  The FAT32 file system driver.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <paging.h>
#include <types.h>
#include <alloc.h>
#include <ata.h> 
#include <bootinfo.h>
#include <vfs.h>
#include <fat32.h>
#include <pmem.h> 
#include <vmem.h> 
#include <utils.h>
#include <err.h>
#include <tty.h>
#include <x86.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Initializes a FAT32 file system.
///
/// @param  allocator   The allocator used for allocating the FAT32 struct.
/// @param  drive       The drive where the file system is located.
/// @param  partition   The number of the partition where the file system is located.
/// @param  vbr         A pointer to the VBR of the partition.
///
/// @returns    A pointer to the newly allocated FAT32 struct.
///////////////////////////////////////////////////////////////////////////////////////////////////

fat32_t* fat32_init(allocator_t *allocator, ata_t drive, u8 partition, vbr_t *vbr) {

    fat32_t *fat32fs = (fat32_t*)allocator->alloc(allocator, sizeof(fat32_t));

    fat32fs->fs = (fs_t) {
        drive,
        partition,
        (u64)bootinfo->partitions[partition].lba_start
    };

    fat32fs->fat_start_lba = fat32fs->fs.partition_start_lba
        + vbr->reserved_sectors;

    fat32fs->data_start_lba = fat32fs->fat_start_lba
        + vbr->num_fats * vbr->big_sectors_per_fat;

    fat32fs->root_start_cluster = vbr->root_directory_start;

    fat32fs->cur_fat_cluster = -1;
    fat32fs->sectors_per_cluster = vbr->sectors_per_cluster;

    mem_set((u8*)fat32fs->fats_loaded, -1, MAX_FAT_CACHE * sizeof(u32));
    fat32fs->fats = (u8*)P2V(
            pmem_alloc(kernel_pt4, MAX_FAT_CACHE * fat32fs->sectors_per_cluster)
            );

    mem_set((u8*)fat32fs->dirs_loaded, -1, MAX_DIR_CACHE * sizeof(u32));
    fat32fs->dirs = (u8*)P2V(
            pmem_alloc(kernel_pt4, MAX_DIR_CACHE * fat32fs->sectors_per_cluster)
            );

    fat32fs->root_dir = (u8*)P2V(pmem_alloc(kernel_pt4, fat32fs->sectors_per_cluster));

    fat32_load_cluster(fat32fs, fat32fs->root_dir, fat32fs->root_start_cluster);

    return fat32fs;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Loads a FAT32 cluster into RAM.
///
/// @param  fs      The FAT32 file system.
/// @param  dest    The location/buffer where the cluster should be loaded. 
/// @param  cluster The cluster number of the cluster to load.
///////////////////////////////////////////////////////////////////////////////////////////////////

void fat32_load_cluster(fat32_t *fs, u8 *dest, u32 cluster) {

    ata_read28(
            fs->fs.drive, 
            dest, 
            fs->data_start_lba + (cluster - 2) * fs->sectors_per_cluster, 
            fs->sectors_per_cluster
            );
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Loads multiple FAT clusters of a file.
///
/// @param  fs              The FAT32 file system.
/// @param  dest            The destination where to load the clusters.
/// @param  cluster         The cluster number of the first cluster to load.
/// @param  max_clusters    Limit for how many clusters to load.
///
/// @returns    The amount of loaded clusters.
///////////////////////////////////////////////////////////////////////////////////////////////////

u32 fat32_load_cluster_chain(fat32_t *fs, u8 *dest, u32 cluster, u32 max_clusters) {

    u32 cur_cluster = cluster;

    for (u64 i = 0; i < max_clusters; i++) {

        fat32_load_cluster(fs, dest + i * fs->sectors_per_cluster * 512, cur_cluster);;
        cur_cluster = fat32_next_cluster(fs, cur_cluster);
        if (cur_cluster >= FAT32_EOF) return i + 1;
    }
    return max_clusters;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Determines the next FAT cluster of a file.
///
/// @param  fs          The FAT32 file system.
/// @param  cur_cluster The previous cluster number.
///
/// @returns    The number of the following cluster.
///
/// @todo   Proper caching algorithm.
///////////////////////////////////////////////////////////////////////////////////////////////////

u32 fat32_next_cluster(fat32_t *fs, u32 cur_cluster) {

    u64 fat_cluster = cur_cluster / MAX_FAT_CLUSTER_ENTRIES;

    // check if the FAT region is cached
    for (u64 i = 0; i < MAX_FAT_CACHE; i++) {
        if (fs->fats_loaded[i] == fat_cluster) {

            return *(u32*)(
                fs->fats 
                + i * fs->sectors_per_cluster * 512 
                + (cur_cluster % MAX_FAT_CLUSTER_ENTRIES) * sizeof(u32)
                );
        }
    }

    // read it from disk otherwise
    for (u64 i = 0; i < MAX_FAT_CACHE; i++) {
        if (fs->fats_loaded[i] != 0) continue;

        fs->fats_loaded[i] = fat_cluster;
        ata_read28(
                fs->fs.drive, 
                fs->fats + i * fs->sectors_per_cluster * 512, 
                fs->fat_start_lba + fat_cluster * fs->sectors_per_cluster, 
                fs->sectors_per_cluster
                );

        return *(u32*)(
            fs->fats 
            + i * fs->sectors_per_cluster * 512 
            + (cur_cluster % MAX_FAT_CLUSTER_ENTRIES) * sizeof(u32)
            );
    }

    // if all spots are occupied simply delete the first one
    // todo: better decision making here
    fs->fats_loaded[0] = fat_cluster;
    ata_read28(
            fs->fs.drive, 
            fs->fats + 0 * fs->sectors_per_cluster * 512, 
            fs->fat_start_lba + fat_cluster * fs->sectors_per_cluster, 
            fs->sectors_per_cluster
            );
    return *(u32*)(
            fs->fats 
            + 0 * fs->sectors_per_cluster * 512 
            + (cur_cluster % MAX_FAT_CLUSTER_ENTRIES) * sizeof(u32)
            );
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Ensures that a directory is cached.
/// 
/// @param  fs          The FAT32 file system.
/// @param  dir_cluster The cluster number of the directory to cache.
///
/// @returns    A pointer to the directory cache.
///
/// @todo   Proper caching algorithm.
///////////////////////////////////////////////////////////////////////////////////////////////////

u8* fat32_cache_dir(fat32_t *fs, u32 dir_cluster) {

    for (u64 i = 0; i < MAX_DIR_CACHE; i++) {

        if (fs->dirs_loaded[i] != dir_cluster) continue;

        // already cached
        return fs->dirs + i * fs->sectors_per_cluster * 512;
    }
    // not cached yet (simply put it into the first spot)
    // todo: better decision making here
    fs->dirs_loaded[0] = dir_cluster;
    fat32_load_cluster(fs, fs->dirs, dir_cluster);
    return fs->dirs;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Compares two file paths to the next /, \ or \0.
/// 
/// @param  path_input  The path to check for.
/// @param  path_entry  The name from the directory entry.
///
/// @returns    How many characters match.
///////////////////////////////////////////////////////////////////////////////////////////////////

u64 fat32_cmp_path(const char *path_input, const char *path_entry) {

    for (u64 i = 0; i < FAT32_NAME_SIZE; i++) {
        if (path_input[i] == path_entry[i]) continue;
        
        if ((path_input[i] == '\\') || (path_input[i] == '/') || (path_input[i] == 0)) {
            for (u64 j = i; j < FAT32_ENTIRE_NAME_SIZE; j++) {
                
                if (path_entry[j] != ' ') return -1;
            }
            return i + 1;
        }
        
        if (path_input[i] == '.') {
            for (u64 j = i; j < FAT32_NAME_SIZE; j++) {
                
                if (path_entry[j] != ' ') return -1;
            }
            
            i++;
            for (u64 j = FAT32_NAME_SIZE; j < FAT32_ENTIRE_NAME_SIZE; ++i, ++j) {
                if (path_input[i] == path_entry[j]) continue;
                
                if ((path_input[i] == '\\') || (path_input[i] == '/') || (path_input[i] == 0)) {
                    
                    for (u64 h = j; h < FAT32_ENTIRE_NAME_SIZE; h++) {
                
                        if (path_entry[h] != ' ') return -1;
                    }
                    return i + 1;
                }
                return -1;
            }
            
            return i + 1;
        }
        return -1;
    }
    return FAT32_ENTIRE_NAME_SIZE + 1;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Loads a file into RAM.
///
/// @param  fs          The FAT32 file system.
/// @param  allocator   The allocator used for allocating the File struct.
/// @param  filepath    The path of the file to load.
///
/// @returns    A pointer to the newly allocated File struct.
///////////////////////////////////////////////////////////////////////////////////////////////////

file_t* fat32_load_file(fat32_t *fs, allocator_t *allocator, const char *filepath) {

    directory_entry_t *entry;
    u64 next_index;
    u32 start_cluster = -1;

    // absolute paths starting from root_dir
    filepath++;
    u8 *cur_dir = fs->root_dir;
    u32 cur_cluster = fs->root_start_cluster;

repeat:
    for (u64 i = 0; i < MAX_DIR_CLUSTER_ENTRIES; i++) {

        entry = (directory_entry_t*)(cur_dir + i * sizeof(directory_entry_t));
        next_index = fat32_cmp_path(filepath, entry->name);

        // no match
        if (next_index == -1) continue;

        // match
        // next dir/file of path
        filepath += next_index;

        // entry is the final file
        if (!(entry->attr & ATTR_DIR)) {
            start_cluster = DWORD(entry->cluster_high, entry->cluster_low);
            break;
        }

        // entry is another directory
        cur_cluster = DWORD(entry->cluster_high, entry->cluster_low);

        // cache and select new directory
        cur_dir = fat32_cache_dir(fs, cur_cluster);

        // start from the top in new directory
        i = 0;
    }

    // no entry match
    if (start_cluster == -1) {

        // get next cluster for the current directory
        cur_cluster = fat32_next_cluster(fs, cur_cluster);
        if (cur_cluster >= FAT32_EOF) panic("File could not be found: %s", filepath);

        // cache and select new cluster
        cur_dir = fat32_cache_dir(fs, cur_cluster);

        goto repeat;
    }

    
    u32 max_clusters = entry->filesize / (512 * fs->sectors_per_cluster);
    if (entry->filesize % (512 * fs->sectors_per_cluster)) max_clusters++;
    
    u8 *dest = (u8*)P2V(pmem_alloc_clean(kernel_pt4, max_clusters * fs->sectors_per_cluster));

    fat32_load_cluster_chain(fs, dest, start_cluster, max_clusters);

    file_t *f = (file_t*)allocator->alloc(&allocator, sizeof(file_t));
    f->access_date = entry->access_date;
    f->create_date = entry->create_date;
    f->create_100ms = entry->create_100ms;
    f->create_time = entry->create_time;
    f->modified_date = entry->modified_date;
    f->modified_time = entry->modified_time;
    f->filesize = entry->filesize;
    f->data = dest;
    mem_cpy((u8*)f->name, (u8*)entry->name, FAT32_ENTIRE_NAME_SIZE);

    return f;
}
