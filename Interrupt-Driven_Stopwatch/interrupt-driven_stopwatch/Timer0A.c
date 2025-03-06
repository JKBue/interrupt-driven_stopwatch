// Timer0A.c
// Runs on Tiva-C

// Adapted from SysTick.c from the book:
/* "Embedded Systems: Introduction to MSP432 Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
   Volume 1, Program 4.7
*/

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "Timer0A.h"
#include "SysTick.h"

uint32_t sysClkFreq = 80000000; // Assume 80 MHz clock by default

// Set clock freq. so Timer0A_Wait10ms delays for exactly 10 ms if clock is not 80 MHz
void Timer0A_Init( uint32_t clkFreq ){
  sysClkFreq = clkFreq;
  SYSCTL_RCGCTIMER_R |= 0x00000001;  // 0) Activate Timer0
  TIMER0_CTL_R &= ~0x00000001;       // 1) Disable Timer0A during setup
  TIMER0_CFG_R = 0;                  // 2) Configure for 32-bit timer mode
  TIMER0_TAMR_R = 1;                 // 3) Configure for one-shot mode, only calls interrupt once
  TIMER0_TAPR_R = 0;                 // 5) No prescale
  TIMER0_IMR_R = 1;                  // 6-9) Yes interrupts

  // NVIC
  NVIC_EN0_R |= 1 << 19;             // set 1 to 19th bit
  NVIC_PRI4_R |= 1 << 29;

  return;
}

// Time delay using busy wait
// The delay parameter is in units of the core clock (units of 12.5 nsec for 80 MHz clock)
//   Adapted from Program 9.8 from the book:
/*   "Embedded Systems: Introduction to ARM Cortex-M Microcontrollers",
     ISBN: 978-1477508992, Jonathan Valvano, copyright (c) 2013
     Volume 1, Program 9.8
*/
void Timer0A_Wait( uint32_t delay ){

  if(delay <= 1){ return; } // Immediately return if requested delay less than one clock
  TIMER0_TAILR_R = delay - 1;        // 4) Specify reload value
  TIMER0_CTL_R |= 0x00000001;        // 10) Enable Timer0A

  //while( TIMER0_TAR_R ){} // Doesn't work; Wait until timer expires (value equals 0)
  // Or, clear interrupt and wait for raw interrupt flag to be set
  // in busy wait mode v want to let timer interrupt the program

  // while( !(TIMER0_RIS_R & 0x1) ){}
  return;
}

// Time delay using busy wait
// This assumes 80 MHz system clock
void Timer0A_Wait1ms( uint32_t delay ){
  uint32_t i;
  for( i = 0; i < delay; i++ ){
    Timer0A_Wait(sysClkFreq/1000);  // wait 1ms
  }
  return;
}

void SSI2_write(unsigned char data) {
    GPIO_PORTC_DATA_R &= ~0x80;       // assert slave select
    SSI2_DR_R = data;            // write data
    while (SSI2_SR_R & 0x10) {}  // wait for transmit done
    GPIO_PORTC_DATA_R |= 0x80;        // deassert slave select
}

/* delay n milliseconds (50 MHz CPU clock) */
void delayMs(int n) {
    int i, j;
    for(i = 0 ; i< n; i++)
        for(j = 0; j < 6265; j++)
            {}  /* do nothing for 1 ms */
}

// sets i and x
const static unsigned char digitPattern[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};
int i = 0;
int x1 = 0;
int x2 = 0;
int x3 = 0;
int x4 = 0;
int check = 0;

void Timer_Handle(){
    if (!check) {
        SSI2_write(digitPattern[x4]);    // write digit pattern to the seven segments
        SSI2_write((1 << 0));            // select digit
        SysTick_Wait1ms(2);              // give time for LED

        SSI2_write(digitPattern[x3]);    // write digit pattern to the seven segments
        SSI2_write((1 << 1));            // select digit
        SysTick_Wait1ms(2);

        SSI2_write(digitPattern[x2]);    // write digit pattern to the seven segments
        SSI2_write((1 << 2));            // select digit
        SysTick_Wait1ms(2);

        SSI2_write(digitPattern[x1] ^ 0x80);    // write digit pattern to the seven segments
        SSI2_write((1 << 3));            // select digit
        SysTick_Wait1ms(2);
    } else if (check) {
        SSI2_write(digitPattern[x4]);    // write digit pattern to the seven segments
        SSI2_write((1 << 0));            // select digit
        if (++x4 > 9) {
            x4 = 0;                      // fixes digit
            ++x3;                        // ups z3 by 1
        }
        SysTick_Wait1ms(2);

        SSI2_write(digitPattern[x3]);    // write digit pattern to the seven segments
        SSI2_write((1 << 1));            // select digit
        if (x3 > 9) {
            x3 = 0;
            ++x2;
        }
        SysTick_Wait1ms(2);

        SSI2_write(digitPattern[x2]);    // write digit pattern to the seven segments
        SSI2_write((1 << 2));            // select digit
        if (x2 > 9) {
            x2 = 0;
            ++x1;
        }
        SysTick_Wait1ms(2);

        SSI2_write(digitPattern[x1] ^ 0x80);    // write digit pattern to the seven segments
        SSI2_write((1 << 3));            // select digit
        if (x1 > 9) {
            x1 = 0;
        }
        SysTick_Wait1ms(2);
    }
    //if (++i > 9) {
    //    i = 0;
    //}
    //if (++x1 > 3){
    //    x1 = 0;
    //}
    //delayMs(2);                     // 1000 / 60 / 4 = 4.17

    TIMER0_ICR_R = 1;               // interrupt flag addy, clear bit in here to no handle
    SysTick_Wait1ms(2);
    Timer0A_Wait(2);

}

void Port_Handle() {
    SysTick_Wait10ms(2);
    if (GPIO_PORTE_MIS_R & 0b0001){          // if start/pause was pressed (PE0)
        // call same functions but repeat the value
        if (check) {
            check = 0;
        } else if (!check) {
            check = 1;
        }

        GPIO_PORTE_ICR_R|= 0b0001;
    } else if (GPIO_PORTE_MIS_R & 0b0010){   // if increment was pressed (PE1)
        if (!check) {
            check = 1;
            Timer_Handle();  // idk if will work if not copy whole function here
            check = 0;
        } else {
            Timer_Handle();
        }

        GPIO_PORTE_ICR_R |= 0b0010;
    } else if (GPIO_PORTE_MIS_R & 0b1000){   // if reset was pressed (PE3)
        x4 = 0;                      // fixes digit to 0
        SysTick_Wait1ms(2);

        x3 = 0;
        SysTick_Wait1ms(2);

        x2 = 0;
        SysTick_Wait1ms(2);

        x1 = 0;
        SysTick_Wait1ms(2);
        GPIO_PORTE_ICR_R |= 0b1000;
    }
}
