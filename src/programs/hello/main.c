#include <types.h>
#include <hello.h>


void _start(void) {

    asm("int 0x80");
    say_hello();
    for(;;);
}
