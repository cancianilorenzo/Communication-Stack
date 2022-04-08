#include <msp430.h>
#include "driverlib.h"



uint8_t i = 0;

void setTimer()
{
    //Timer A0_1
    TA0CCTL0 = CCIE; // enable capture control interupt
    TA0CTL = TASSEL_1 + MC_1 + ID_3;  // Use SMCLK in up mode, /8 divider
    TA0CCR0 = 10000; // set interupt value
    TA0CCTL0 &= 0x10; // set compare mode

    //Timer A1_1
    TA1CCTL0 = CCIE; // enable capture control interupt
    TA1CTL = TASSEL_2 + MC_1 + ID_3;  // Use SMCLK in up mode, /8 divider
    TA1CCR0 = 10000; // set interupt value
    TA1CCTL0 &= 0x10; // set compare mode

    //Timer A4_1
    TA4CCTL0 = CCIE; // enable capture control interupt
    TA4CTL = TASSEL_1 + MC_1 + ID_3;  // Use SMCLK in up mode, /8 divider
    TA4CCR0 = 10000; // set interupt value
    TA4CCTL0 &= 0x10; // set compare mode

    //Timer B0_1
    TB0CCTL0 = CCIE; // enable capture control interupt
    TB0CTL = TASSEL_2 + MC_1 + ID_3;  // Use SMCLK in up mode, /8 divider
    TB0CCR0 = 10000; // set interupt value
    TB0CCTL0 &= 0x10; // set compare mode



}

void setPin()
{
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN1);

    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN2);
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN4);
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN5);
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN7);
    //Power off all pin to reduce power consuption
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN2);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN4);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN5);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN7);
}

void initBoard()
{
    WDT_A_hold(WDT_A_BASE);
    PMM_unlockLPM5(); // Disable the GPIO power-on default high-impedance mode
    __enable_interrupt(); // enable global interrupts
}

int main(void)
{
    setPin();
    initBoard();
    setTimer();

    while (1);

}

//HANDLER INTERRUPT TIMER A0 A1
#pragma vector = TIMER0_A0_VECTOR
__interrupt void T0A1_ISR(void)
{
    GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN2);
    /*if(i == 3){
        TA1CCR0 = 10000; // set interupt value
    }
    if(i == 10){
        TA1CTL &= ~MC; // stop timer
    }
    i++;*/
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void T1A1_ISR(void)
{
    GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN4);
    //Timer_A_clearTimerInterrupt(TIMER_A1_BASE); Does not change anything
    //Probably VIC is already cleaned
}


#pragma vector = TIMER4_A0_VECTOR
__interrupt void T4A1_ISR(void)
{
    GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN5);
    //Timer_A_clearTimerInterrupt(TIMER_A1_BASE); Does not change anything
    //Probably VIC is already cleaned
}


#pragma vector = TIMER0_B0_VECTOR
__interrupt void T0B0_ISR(void)
{
    GPIO_toggleOutputOnPin(GPIO_PORT_P4, GPIO_PIN7);
    //Timer_A_clearTimerInterrupt(TIMER_A1_BASE); Does not change anything
    //Probably VIC is already cleaned
}
