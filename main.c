/******************************************************************************
* Long Tran                                                                   *
*                                                                             *
* Device:  MSP432P401R LaunchPad                                              *
* Program: Display input to keypad                                            *
*                                                                             *
* Important Ports:                                                            *
* P4 is OUTPUT for 7-seg display digit pattern                                *
* P8 is OUTPUT to control active digits in row                                *
* P9 is INPUT from keypad column                                              *
*                                                                             *
* Demo: https://youtu.be/VUVd9RPUBLM                                          *
******************************************************************************/

#include "msp.h"
#define N 100       // debounce loop count
#define d 16        // digit-bit index to clear display

int digit_array[17] = { // DIGIT-BIT LOOKUP TABLE
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
0b11111111,  // Blank Display
};

int keypad_table[4][9] = {              // KEYPAD LAYOUT
    {d, 10,  3, d, 2, d, d, d,  1},     // 4 x 9 array for multiplixer keypad
    {d, 11,  6, d, 5, d, d, d,  4},     // only columns 1, 2, 4, 8 are valid
    {d, 12,  9, d, 8, d, d, d,  7},
    {d, 13, 15, d, 0, d, d, d, 14}};

typedef enum{       // KEYPAD DEBOUNCING FSM
    IDLE,           // scan for keypad input
    PRESS,          // debounce keypad press
    PROCESS,        // accept input key
    RELEASE         // debounce keypad release
}KeyStates;

typedef struct{
    KeyStates state;    // STATES OF KEYPAD FSM
    int x;              // x position of pressed key
    int y;              // y position of pressed key
    int display[4];     // array for keeping the last four pressed numbers
    int display_count;  // display array index
    int pulses;         // debouncing pulses
}Keypad;

void gpio_init(void);   // initialize GPIO

void main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;             // disable watchdog
    gpio_init(); int i, j, temp;                            // initalize GPIO
    Keypad key = {IDLE, 0, 0, {0xFF,0xFF,0xFF,0xFF}, 0, 0}; // Initalize keypad structure

    while(1){

        key.x = 0;
        key.y = 0;

        for(i = 0; i < 4; i++){             // UPDATE DISPLAY
            P4->OUT = 0xFF;                 // clear current display
            P8->OUT = 0xFF & ~(BIT5 >> i);  // shift active display
            P4->OUT = key.display[i];       // display char

            for(j = 0; j < 4; j++){         // SCAN FOR INPUT
                temp = (P9->IN) & 0x0F;
                if(temp > 0 ){              // if key pressed detected
                    key.x = temp;           // acknowledge input x position
                    key.y = i;              // acknowledge input y position
                }
            }
        }

        switch (key.state){

            case IDLE:                              // INPUT DETECT
                if(key.x != 0){
                    key.state = PRESS;              // go to PRESS state
                    key.pulses = 0;
                }break;

            case PRESS:                             // PULSE COUNTER
                P4->OUT = 0xFF;                     // clear current display
                P8->OUT = 0xFF & ~(BIT5 >> key.y);  // switch to row where input was prev found
                temp = (P9->IN) & 0x0F;             // read column input
                if(temp = key.x){
                    key.pulses++;
                    if(key.pulses > N){
                        key.pulses = 0;
                        key.state = PROCESS;        // process successful input
                       }
                   }else{
                       key.state = IDLE;            // else, restart
                   }break;
                   // ACKNOWLEDGE INPUT
            case PROCESS:                           

                if(key.display_count > 3){          // reset display array
                    key.display_count = 0;
                }
                key.display[key.display_count] = digit_array[keypad_table[key.y][key.x]];
                key.display_count++;
                key.state = RELEASE;                     // wait for keypad release
                break;

            case RELEASE:                                // PULSE COUNTER
                P4->OUT = 0xFF;                          // clear output to display
                P8->OUT = 0xFF & ~(BIT5 >> key.y);       // switch to row where input was prev found
                temp = (P9->IN) & 0x0F;                  // read keypad column input
                if(temp == 0){
                    key.pulses++;
                    if(key.pulses > N){
                        key.pulses = 0;
                        key.state = IDLE;   // release successful
                       }
                }else{
                    key.pulses = 0;
                    key.state = RELEASE;    // keypad input sensed, repeat
                    }break;

        }// switch end
    }// while(1) end
}// main end

void gpio_init(void){        // INITIALIZE GPIO
    P4->DIR = 0xFF;          // P4 is LED output
    P8->DIR = 0xFF;          // P8 is display output
    P9->DIR = 0x00;          // P9 is keypad input
}
