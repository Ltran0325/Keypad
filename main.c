/*******************************************************************************
*                       MSP432 Keypad 7-Segment Solution                       *
*                                                                              *
* Author:  Long Tran                                                           *
* Device:  MSP432P401R LaunchPad                                               *
* Program: Display input to keypad                                             *
*                                                                              *
* Important Ports:                                                             *
* P4 is OUTPUT for 7-seg display digit pattern                                 *
* P8 is OUTPUT to control active digits in row                                 *
* P9 is INPUT from keypad column                                               *
*                                                                              *
* Demo: https://youtu.be/VUVd9RPUBLM                                           *
*******************************************************************************/

// Include header file(s) and define constants
#include "msp.h"
#define N 100   // debounce loop count
#define d 16    // digit-bit index to clear display

// Define digit-bit lookup table
const int look_up[17] = {
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

// Define keypad layout
const int keypad_table[4][9] = {
    {d, 13, 15, d, 0, d, d, d, 14},   // 4x9 multiplixer keypad
    {d, 10,  3, d, 2, d, d, d,  1},   // only columns 1, 2, 4, 8 are valid for single keypress
    {d, 11,  6, d, 5, d, d, d,  4},
    {d, 12,  9, d, 8, d, d, d,  7}};

// Define keypad structure to handle related variables
typedef struct{         // KEYPAD STRUCTURE
    enum{IDLE, PRESS, PROCESS, RELEASE} state; // keypad state variable
    int x;              // x position of pressed key
    int y;              // y position of pressed key
    int display[4];     // array for keeping the last four pressed numbers
    int display_count;  // display array index
    int pulses;         // debouncing pulses
    int k;              // points to active row of keypad
}Keypad;

// Define prototypes
void gpio_init(void); // initialize GPIO
void wait(int t);     // busy wait

void main(void)
{
    // Initialize GPIO and keypad structure
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;                // disable watchdog
    gpio_init(); int temp;                                     // initalize GPIO
    Keypad key = {IDLE, 0, 0, {0xFF,0xFF,0xFF,0xFF}, 0, 0, 0}; // Initalize keypad structure

    while(1){
        // Display digit-k
        P4->OUT = 0xFF;                    // blank 7-seg display
        P8->OUT = 0xFF & ~(BIT5 >> key.k); // enable k-th keypad row
        P4->OUT = key.display[key.k];      // display k-th char in array

        // scan input key (at row k)
        temp = (P9->IN) & 0x0F;
        
        // increment k index
        key.k++;
        if (key.k >= 4){key.k = 0;}
        
        // reduce flickering
        wait(100);
        
        // Switch keypad debouncing state
        switch (key.state){

            // Wait for input
            case IDLE:
            {
                // go to PRESS state if input detected
                if(temp > 0 ){                            
                     key.x = temp;      // acknowledge input x position
                     key.y = key.k;     // acknowledge input y position
                     key.state = PRESS;                   
                     key.pulses = 0;
                 }break;
            }

            // Accept input if N pulses of HIGH detected
            case PRESS:
            {
                if(key.k == key.y && temp == key.x){ // pulse repeat
                    key.pulses++;
                }
                if(key.k == key.y && temp != key.x){ 
                    key.state = IDLE;                // input fail
                }
                if(key.pulses > N){
                    key.state = PROCESS;             // input success
                }break;
            }

            // Update display array with accepted input
            case PROCESS:
            {
                // process input into digit display array (decode)
                key.display[key.display_count] = look_up[keypad_table[key.y][key.x]];   
                
                // increment display digit index
                key.display_count++;
                if(key.display_count > 3){key.display_count = 0;}
                
                key.pulses = 0;
                key.state = RELEASE;
                break;
            }
            
            // Accept release if N pulses of LOW detected
            case RELEASE:
            {
                if(key.k == key.y && temp == 0){  // release repeat
                    key.pulses++;
                }
                if(key.k == key.y && temp != 0){    
                    key.pulses = 0;               // release fail
                }
                if(key.pulses > N){
                    key.state = IDLE;             // release success
                }break;
            }
            
        }// switch end
    }// while(1) end
}// main end

void gpio_init(void){
    P4->DIR = 0xFF;  // P4 is LED output
    P8->DIR = 0xFF;  // P8 is display output
    P9->DIR = 0x00;  // P9 is keypad input
}

void wait(int t){
    while(t >= 0){t--;}
}
