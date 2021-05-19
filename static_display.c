/******************************************************************************
* EE138                                                                       *
* Long Tran                                                                   *
* Lab 1                                                                       *
* 2/10/21                                                                     *
*                                - WEEK 1 -                                   *
* Task 1: Display 4 unique symbols onto the 7-segment display.                *
* Ex. display " 1 2 3 4" or "A C 5 F"                                         *
* Important Ports:                                                            *
* P4.x is OUTPUT to control LED                                               *
* P8.x is OUTPUT to display digit (1 at a time)                               *
* P9.x is INPUT from keypad (NOT USED)                                        *
******************************************************************************/

#include "msp.h"

// Digit Array Lookup Table
int digit_array[16] = { //bits from dp-a (left-right)
0b11000000,  // 0
0b11111001,  // 1
0b10100100,  // 2
0b10110000,  // 3
0b10011001,  // 4
0b10010010,  // 5
0b10000010,  // 6
0b11111000,  // 7
0b10000000,  // 8
0b10010000,  // 9
0b10001000,  // A
0b10000011,  // b
0b11000110,  // C
0b10100001,  // d
0b10000110,  // E
0b10001110,  // F
};

int task1_array[4] = {
0b11111000,  // 7
0b10000010,  // 6
0b10001000,  // A
0b11000000,  // 0
};

/* prototypes */
void wait(int t);
void gpio_init(void);

// display "76A0"
void main(void)
{
    gpio_init(); int i;

    while(1){
        for(i = 0; i < 4; i++){             // P4 and P8 are negative logic (1 is off)
            P4->OUT = 0xFF;                 // clear current display
            P8->OUT = 0xFF & ~(0x20 >> i);  // shift active display
            P4->OUT = task1_array[i];       // display char
        }// wait(1);
    }
}

// delay using counter
void wait(int t){
    int count = 0;
    while (count < t){count++;}
}

// initialize GPIO
void gpio_init(void){
    P4->DIR = 0xFF;          // P4 is LED output
    P8->DIR = 0xFF;          // P8 is Display output
}

