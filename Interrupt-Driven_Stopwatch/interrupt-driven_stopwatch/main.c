#include <stdint.h>
#include "SysTick.h"
#include "PLL.h"
#include "tm4c123gh6pm.h"
#include "Timer0A.h"


void GPIOPortB_Init() {
    SYSCTL_RCGC2_R |= 0b10;        // 1) turns on clock B
    GPIO_PORTB_AMSEL_R &= ~(0xFF); // 3) disable analog function on PB5-0
    GPIO_PORTB_PCTL_R &= ~(0xFF);  // 4) enable regular GPIO
    GPIO_PORTB_DIR_R |= 0xFF;      // 5) outputs on PB5-0
    GPIO_PORTB_AFSEL_R &= ~(0xFF); // 6) regular function on PB5-0
    GPIO_PORTB_DEN_R |= 0xFF;      // 7) enable digital on PB5-0
}

void GPIOPortE_Init() {
    SYSCTL_RCGC2_R |= 0b10000;     // 1) turns on clock for E
    SysTick_Wait10ms(2);
    GPIO_PORTE_AMSEL_R &= ~(0xFF); // 3) disable analog function on PE2-0
    GPIO_PORTE_PCTL_R &= ~(0xFF);  // 4) enable regular GPIO
    GPIO_PORTE_DIR_R &= ~(0xFF);   // 5) inputs on PE2-0
    GPIO_PORTE_AFSEL_R &= ~(0xFF); // 6) regular function on PE2-0
    GPIO_PORTE_DEN_R |= 0xFF;      // 7) enable digital on PE2-0

    NVIC_EN0_R |= 1 << 4;             // set 1 to 4th bit
    NVIC_PRI4_R |= 0;

    GPIO_PORTE_IM_R |= (1<<0)|(1<<1)|(1<<3);        // enable interrupts
    GPIO_PORTE_IS_R &= ~(1<<0)|~(1<<1)|~(1<<3);     // enable edge trigger
    GPIO_PORTE_IEV_R &= (1<<0)|(1<<1)|(1<<3);    // enable negative trigger
    GPIO_PORTE_IBE_R &= ~(1<<0)|~(1<<1)|~(1<<3);
}


/* Seven-segment display counter
 *
 * This program counts number 0-3 on the seven segment display.
 * The seven segment display is driven by a shift register which is
 * connected to SSI2 in SPI mode.
 *
 * Built and tested with Keil MDK-ARM v5.28 and TM4C_DFP v1.1.0
 */

// #include "TM4C123.h" // use diff header files

void delayMs(int n);
void sevenseg_init(void);
void SSI2_write(unsigned char data);

int main(void) {

    sevenseg_init();    // initialize SSI2 that connects to the shift registers
    SysTick_Init();
    Timer0A_Init(80000000);
    GPIOPortE_Init();

    Timer0A_Wait(2);

    while(1) {
        // was moved to Timer0A
        // here to catch processor
    }



}

// enable SSI2 and associated GPIO pins (need to change to fit our board)
void sevenseg_init(void) {
    SYSCTL_RCGC2_R |= 0x02;   // enable clock to GPIOB (change first part to SYSCTC_RCGCGPIO_R)
    SYSCTL_RCGC2_R |= 0x04;   // enable clock to GPIOC
    SYSCTL_RCGCSSI_R |= 0x04;    // enable clock to SSI2

    // PORTB 7, 4 for SSI2 TX and SCLK
    GPIO_PORTB_AMSEL_R &= ~0x90;      // turn off analog of PORTB 7, 4
    GPIO_PORTB_AFSEL_R |= 0x90;       // PORTB 7, 4 for alternate function
    GPIO_PORTB_PCTL_R &= ~0xF00F0000; // clear functions for PORTB 7, 4
    GPIO_PORTB_PCTL_R |= 0x20020000;  // PORTB 7, 4 for SSI2 function
    GPIO_PORTB_DEN_R |= 0x90;         // PORTB 7, 4 as digital pins (change first part to GPIO_PORTB_DEN_R)

    // PORTC 7 for SSI2 slave select
    GPIO_PORTC_AMSEL_R &= ~0x80;      // disable analog of PORTC 7
    GPIO_PORTC_DATA_R |= 0x80;        // set PORTC 7 idle high
    GPIO_PORTC_DIR_R |= 0x80;         // set PORTC 7 as output for SS
    GPIO_PORTC_DEN_R |= 0x80;         // set PORTC 7 as digital pin

    SSI2_CR1_R = 0;              // turn off SSI2 during configuration
    SSI2_CC_R = 0;               // use system clock
    SSI2_CPSR_R = 16;            // clock prescaler divide by 16 gets 1 MHz clock
    SSI2_CR0_R = 0x0007;         // clock rate div by 1, phase/polarity 0 0, mode freescale, data size 8
    SSI2_CR1_R = 2;              // enable SSI2 as master
}


// This function enables slave select, writes one byte to SSI2,
// wait for transmit complete and deassert slave select.
/*void SSI2_write(unsigned char data) {
    GPIO_PORTC_DATA_R &= ~0x80;       // assert slave select
    SSI2_DR_R = data;            // write data
    while (SSI2_SR_R & 0x10) {}  // wait for transmit done
    GPIO_PORTC_DATA_R |= 0x80;        // deassert slave select
}*/

// get rid of below since not set for 80 MHz
/* delay n milliseconds (50 MHz CPU clock) */
/*void delayMs(int n) {
    int i, j;
    for(i = 0 ; i< n; i++)
        for(j = 0; j < 6265; j++)
            {}  // do nothing for 1 ms
}*/

