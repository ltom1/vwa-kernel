# Vorwissenschaftliche Arbeit – x86-64 Hobby Kernel

Dieser Kernel ist im Rahmen meiner Vorwissenschaftlichen Arbeit am BG/BRG Ried im Innkreis entstanden. Der Umfang der Implementierung wurde bewusst auf die im Rahmen der Arbeit möglichen Seiten und den verfügbaren Entwicklungsaufwand begrenzt. Dennoch konnten bereits wesentliche Betriebssystemfunktionen eigenständig implementiert und getestet werden.

Bare-Metal-Betriebssystemkernel für die x86-64-Architektur.

**Plattform:** x86-64 (64 Bit)

## Funktionen

- Ausführung eigener User-Prozesse im CPU-Ring 3 (User Mode)
- ELF64-Binary-Loader zum Laden und Ausführen von Programmen
- Memory Management:
  - Virtueller Speicher und Paging
  - Verwaltung virtueller Adressräume
  - Heap-Allokation
  - Speicherschutz zwischen Kernel- und Userspace
- Systemcalls: Nur Grundstruktur

## Hardware-Treiber und Systemkomponenten

- **Interrupts:** Interruptbehandlung über den Intel 8259 PIC
- **GDT:** Global Descriptor Table zur Definition der CPU-Segmentierung und Privilegstufen
- **Timer:** Programmable Interval Timer (Intel 8253/8254) für zeitgesteuerte Interrupts
- **VGA:** Grafische Textausgabe über den VGA-Text-Modus
- **PS/2 Keyboard:** Tastatureingabe über den PS/2-Controller
- **ATA-PIO:** Sektorweiser Lese- und Schreibzugriff auf ATA-Festplatten über Programmed I/O
- **FAT32:** Dateisystemunterstützung zum Lesen und Verwalten von Dateien auf FAT32-Datenträgern

## Software / Entwicklungsumgebung

- GCC und NASM
- GNU Binutils
- QEMU zur Emulation und zum Testen
- GDB für Kernel-Debugging
