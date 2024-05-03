#ifndef COMMON_H
#define COMMON_H

#include <xc.h>

static inline void BLUE_LED_ON(void) {
    LATA2 = 0;
}

static inline void BLUE_LED_OFF(void) {
    LATA2 = 1;
}

static inline void BLUE_LED_TOGGLE(void) {
    LATA2 ^= 1;
}

static inline void GREEN_LED_ON(void) {
    LATA3 = 0;
}

static inline void GREEN_LED_OFF(void) {
    LATA3 = 1;
}

static inline void GREEN_LED_TOGGLE(void) {
    LATA3 ^= 1;
}

static inline void RED_LED_ON(void) {
    LATA4 = 0;
}

static inline void RED_LED_OFF(void) {
    LATA4 = 1;
}

static inline void RED_LED_TOGGLE(void) {
    LATA4 ^= 1;
}

#endif
