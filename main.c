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

// Include header file(s) and define constants
#include "msp.h"
#define N 300       // debounce loop count
#define d 16        // digit-bit index to clear display

// Define digit-bit lookup table
const int digit_array[17] = { 
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
    {d, 10,  3, d, 2, d, d, d,  1},     // 4x9 multiplixer keypad
    {d, 11,  6, d, 5, d, d, d,  4},     // only columns 1, 2, 4, 8 are valid for single keypres
    {d, 12,  9, d, 8, d, d, d,  7},
    {d, 13, 15, d, 0, d, d, d, 14}};

// Define keypad states of FSM
typedef enum{      
    IDLE,           // scan for keypad input
    PRESS,          // debounce keypad press
    PROCESS,        // accept input key
    RELEASE         // debounce keypad release
}KeyStates;

// Define keypad structure to handle related variables
typedef struct{         // KEYPAD STRUCTURE 
    KeyStates state;    // states of keypad FSM
    int x;              // x position of pressed key
    int y;              // y position of pressed key
    int display[4];     // array for keeping the last four pressed numbers
    int display_count;  // display array index
    int pulses;         // debouncing pulses
    int k;              // points to active row of keypad
}Keypad;

// Define prototypes
void gpio_init(void);   // initialize GPIO
void wait(int t);       // busy wait

void main(void)
{   
    // Initialize GPIO and keypad structure
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;                // disable watchdog
    gpio_init(); int temp;                                     // initalize GPIO
    Keypad key = {IDLE, 0, 0, {0xFF,0xFF,0xFF,0xFF}, 0, 0, 0}; // Initalize keypad structure

    while(1){
        // Display digit-k
        P4->OUT = 0xFF;                     // blank display
        P8->OUT = 0xFF & ~(BIT5 >> key.k);  // enable k-th display
        P4->OUT = key.display[key.k];       // display k-th char in array
        
        // Scan for keypad input in row-k
        temp = (P9->IN) & 0x0F;             // scan input at row-k
        if(temp > 0 ){                      // if key pressed detected
            key.x = temp;                   // acknowledge input x position
            key.y = key.k;                  // acknowledge input y position
        }
        
        // Increment index-k
        key.k++;                            
        if (key.k >= 4){key.k = 0;
        
        // Test program resistance to software updates (observe LED lighting)
        wait(100);                          
                        
        // Switch keypad debouncing state
        switch (key.state){
            
            // Wait for input
            case IDLE:                              
                if(key.x != 0){
                    key.state = PRESS;              
                    key.pulses = 0;
                }break;
            
            // Accept input if N pulses of HIGH detected
            case PRESS:                             
                P4->OUT = 0xFF;                     // blank display
                P8->OUT = 0xFF & ~(BIT5 >> key.y);  // switch to row where input was prev found
                temp = (P9->IN) & 0x0F;             // read column input
                if(temp == key.x){key.pulses++;}    // increment pulses if last input is same as new input
                if(key.pulses > N){
                    key.pulses = 0;
                    key.state = PROCESS;            // input success
                }else{
                    key.state = IDLE;               
                }break;
            
            // Update display array with accepted input
            case PROCESS:                           

                if(key.display_count > 3){key.display_count = 0;}
                key.display[key.display_count] = digit_array[keypad_table[key.y][key.x]];
                key.display_count++;
                key.state = RELEASE;                    
                break;
    
            // Accept release if N pulses of LOW detected
            case RELEASE:                               
                P4->OUT = 0xFF;                          // blank display
                P8->OUT = 0xFF & ~(BIT5 >> key.y);       // switch to row where input was prev found
                temp = (P9->IN) & 0x0F;                  // read keypad column input
                if(temp == 0){key.pulses++;}
                if(key.pulses > N){
                    key.pulses = 0;
                    key.state = IDLE;                    // release successful
                }else{
                    key.pulses = 0;
                    key.state = RELEASE;    
                }break;

        }// switch end
    }// while(1) end
}// main end

void gpio_init(void){        
    P4->DIR = 0xFF;          // P4 is LED output
    P8->DIR = 0xFF;          // P8 is display output
    P9->DIR = 0x00;          // P9 is keypad input
}
    
void wait(int t){           
    while(t >= 0){t--;}
}

