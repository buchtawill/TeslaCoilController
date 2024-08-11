#include "timers.h"
#include "main.h"


static uint32_t millis = 0;

void increment_millis(){
    millis++;
}

uint32_t get_millis(){
    return millis;
}

void reset_millis(){
    millis = 0;
}