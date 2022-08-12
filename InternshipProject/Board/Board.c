#include "Board.h"
#include <msp430.h>
#include "driverlib.h"
#include <stdio.h>

//Uart message
char message[MSG_SIZE];

void setBoardFrequency()
{
    // Configure one FRAM waitstate as required by the device datasheet for MCLK
    // operation beyond 8MHz _before_ configuring the clock system.
    FRCTL0 = FRCTLPW | NWAITS_1;

    // Clock System Setup
    CSCTL0_H = CSKEY_H;                     // Unlock CS registers
    CSCTL1 = DCOFSEL_0;                     // Set DCO to 1MHz
    // Set SMCLK = MCLK = DCO, ACLK = VLOCLK
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;

    // Per Device Errata set divider to 4 before changing frequency to
    // prevent out of spec operation from overshoot transient
    CSCTL3 = DIVA__4 | DIVS__4 | DIVM__4; // Set all corresponding clk sources to divide by 4 for errata
    CSCTL1 = DCOFSEL_4 | DCORSEL; // Set DCO to 16MHz (Digital Controlled Oscillator)

    // Delay by ~10us to let DCO settle. 60 cycles = 20 cycles buffer + (10us / (1/4MHz))
    __delay_cycles(60);
    CSCTL3 = DIVA__32 | DIVS__16 | DIVM__1; // SMCLK set to 1MHz, ACLK set to 250Hz
    CSCTL0_H = 0; // Lock CS registers                      // Lock CS registers

}

void pinDeclaration()
{
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN6);
    GPIO_selectInterruptEdge(GPIO_PORT_P5, GPIO_PIN6,
    GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_clearInterrupt(GPIO_PORT_P5, GPIO_PIN6);
    GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN6);
}


void UART_TXData(char *c, size_t size)
{
    //Can't find sysTick like on MSP432 so this is a possible approach
    int position;
    int i;
    for (position = 0; position < size; position++)
    {
        UCA0TXBUF = c[position];
        for (i = 0; i < 5000; i++)
            ;
    }
}


void UARTInit()
{
    //FOR DEBUG TO PC
    P2SEL0 &= ~(BIT0 | BIT1);
    P2SEL1 |= (BIT0 | BIT1);                           // USCI_A3 UART operation

    UCA0CTLW0 = UCSWRST;                                   // Put eUSCI in reset
    UCA0CTLW0 |= UCSSEL__SMCLK;                    // CLK = SMCLK
    UCA0BRW = 8;
    UCA0MCTLW = 0xD600;
    UCA0CTLW0 &= ~UCSWRST;                                 // Initialize eUSCI

    UCA0IE |= UCRXIE;
}


void initBoard(){
    WDTCTL = WDTPW | WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5; // Disable the GPIO power-on default high-impedance mode
    __enable_interrupt(); // enable global interrupts

    setBoardFrequency();
    pinDeclaration();
    UARTInit();
}


