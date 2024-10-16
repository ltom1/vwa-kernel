#include <types.h>
#include <hello.h>


void say_hello(void) {

    *((u8*)0xb8000) = 'X';
}
