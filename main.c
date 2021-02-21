/******************************************************************************
* Long Tran                                                                   *
* 									      *
* Device:  MSP432P401R LaunchPad					      *
* Program: Display input to keypad                                            *
* 									      *
* Important Ports:                                                            *
* P4 is OUTPUT for 7-seg display digit pattern                                             *
* P8 is OUTPUT to control active digits in row                               *
* P9 is INPUT from keypad       					      *
*                                            			              *
* Demo: https://youtu.be/VUVd9RPUBLM					      *
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

enum keyStates{     // KEYPAD DEBOUNCING STATES
    IDLE,           // scan for keypad input
    PRESS,          // debounce keypad press
    PROCESS,        // accept input key
    RELEASE         // debounce keypad release
};

void gpio_init(void);   // initialize GPIO

int gl_count = 0;       // input pulse counter

void main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;    // disable watchdog
    gpio_init(); int i, j, temp;                   // initalize GPIO
    int key_x;                                     // x position of pressed key
    int key_y;                                     // y position of pressed key
    int display[4] = {0xFF, 0xFF, 0xFF, 0xFF};     // clear display
    int display_count = 0;                         // display array index
    enum keyStates state = IDLE;                   // begin in IDLE state

    while(1){

        key_x = 0;
        key_y = 0;

        for(i = 0; i < 4; i++){             // UPDATE DISPLAY
            P4->OUT = 0xFF;                 // clear current display
            P8->OUT = 0xFF & ~(0x20 >> i);  // shift active display
            P4->OUT = display[i];           // display char

            for(j = 0; j < 4; j++){         // SCAN FOR INPUT
                temp = (P9->IN) & 0x0F;
                if(temp > 0 ){              // if key pressed detected
                    key_x = temp;           // acknowledge input x position
                    key_y = i;              // acknowledge input y position
                }
            }
        }

        switch (state){

            case IDLE:                              // INPUT DETECT
                if(key_x != 0){
                    state = PRESS;                  // go to PRESS state
                    gl_count = 0;
                }break;
                
            case PRESS:                             // PULSE COUNTER
                P4->OUT = 0xFF;                     // clear current display
                P8->OUT = 0xFF & ~(0x20 >> key_y);  // switch to row where input was prev found
                temp = (P9->IN) & 0x0F;             // read column input
                if(temp = key_x){
                    gl_count++;
                    if(gl_count > N){
                       gl_count = 0;
                       state = PROCESS;             // process successful input
                       }
                   }else{
                       state = IDLE;                // else, restart
                   }break;

            case PROCESS:                           // ACKNOWLEDGE INPUT

                if(display_count > 3){              // reset display array
                    display_count = 0;
                }
                display[display_count] = digit_array[keypad_table[key_y][key_x]];
                display_count++;
                state = RELEASE;                    // wait for keypad release
                break;

            case RELEASE:                           // PULSE COUNTER
                P4->OUT = 0xFF;                     // clear output to display
                P8->OUT = 0xFF & ~(0x20 >> key_y);  // switch to row where input was prev found
                temp = (P9->IN) & 0x0F;             // read keypad column input
                if(temp == 0){
                    gl_count++;
                    if(gl_count > N){
                        gl_count = 0;
                        state = IDLE;               // release successful
                       }
                }else{
                    gl_count = 0;
                    state = RELEASE;                // keypad input sensed, repeat
                    }break;

        }// switch end
    }// while(1) end
}// main end

void gpio_init(void){        // INITIALIZE GPIO
    P4->DIR = 0xFF;          // P4 is LED output
    P8->DIR = 0xFF;          // P8 is display output
    P9->DIR = 0x00;          // P9 is keypad input
}
