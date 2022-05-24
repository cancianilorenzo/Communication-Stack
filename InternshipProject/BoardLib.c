/*
 Lorenzo Canciani
 lorenzo.canciani@studenti.unitn.it
 2022
 */

#include "BoardLib.h"
#include <msp430.h>
#include <msp430.h>
#include "driverlib.h"
#include <stdlib.h>

void initBoard()
{
    /* MPUCTL0 = MPUPW;
     MPUSEGB2 = 0x1000; // memory address 0x10000
     MPUSEGB1 = 0x0fc0; // memory address 0x0fc00
     MPUSAM &= ~MPUSEG2WE; // disallow writes
     MPUSAM |= MPUSEG2VS;  // reset CPU on violation
     MPUCTL0 = MPUPW | MPUENA;
     MPUCTL0_H = 0;*/

    WDTCTL = WDTPW | WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5; // Disable the GPIO power-on default high-impedance mode
    __enable_interrupt(); // enable global interrupts

}

void UARTInit()
{
    P2SEL0 &= ~(BIT0 | BIT1);
    P2SEL1 |= (BIT0 | BIT1);                           // USCI_A3 UART operation

    UCA0CTLW0 = UCSWRST;                                   // Put eUSCI in reset
    UCA0CTLW0 |= UCSSEL__SMCLK;                    // CLK = SMCLK
    // Baud Rate calculation
    // 8000000/(16*9600) = 52.083
    // Fractional portion = 0.083
    // User's Guide Table 21-4: UCBRSx = 0x04
    // UCBRFx = int ( (52.083-52)*16) = 1
    UCA0BRW = 8;                                            // 8000000/16/9600
    UCA0MCTLW = 0xD600;
    UCA0CTLW0 &= ~UCSWRST;                                 // Initialize eUSCI


    UCA0IE |= UCRXIE;

}

void setTimers()
{
    //Should work
    //Timer A0_0 ---- ENERGY UPDATE ---- WORKS
    TA0CCTL0 = CCIE; // enable capture control interupt
    TA0CTL = TASSEL_1 + MC_1 + ID_0;  // Use ACLK in up mode, /1 divider
    TA0CCR0 = 0; // set interupt value
    TA0CCTL0 &= 0x10; // set compare mode


    //TODO cambiare
    //Timer A1_0
    TA1CCTL0 = CCIE; // enable capture control interupt
    TA1CTL = TASSEL_2 + MC_1 + ID_3; // 1MHz/8 --> 125khz
    TA1CCR0 = 0; // set interupt value
    TA1CCTL0 &= 0x10; // set compare mode


    //Should be ok
    //Timer A2_0 ---- NODE IDENTIFICATION
    TA2CCTL0 = CCIE; // enable capture control interupt
    TA2CTL = TASSEL_2 + MC_1 + ID_2; // Use SMCLK in up mode, /8 divider --> 2MHz
    TA2CCR0 = 0; // set interupt value
    TA2CCTL0 &= 0x10; // set compare mode


    //TODO cambiare
    //Timer A3_0 ------ TIMEOUT BURST RECEPTION
    TA3CCTL0 = CCIE; // enable capture control interupt
    TA3CTL = TASSEL_1 + MC_1 + ID_3; // Use SMCLK in up mode, /8 divider
    TA3CCR0 = 0; // set interupt value
    TA3CCTL0 &= 0x10; // set compare mode

    //Timer A4_0 ---- BURST REPETITION ---- SOULD BE OK ------ TODO TEST
    TA4CCTL0 = CCIE; // enable capture control interupt
    TA4CTL = TASSEL_1 + MC_1 + ID_0;  // Use ACLK in up mode, /8 divider
    TA4CCR0 = 0; // set interupt value
    TA4CCTL0 &= 0x10; // set compare mode


    //OK
    //Timer B0_0 ---- PULSES SEND ---- WORKS
    TB0CCTL0 = CCIE; // enable capture control interupt
    TB0CTL = TASSEL_2 + MC_1 + ID_0;  // Use SMCLK in up mode, /8 divider
    TB0CCR0 = 0; // set interupt value
    TB0CCTL0 &= 0x10; // set compare mode

}

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
    //3.1 DATA RX----3.0 BURST RX
     P3IES = (BIT0 | BIT1);  // set interrupt on edge select
     P3IFG = 0;              // clear interrupt flags
     P3IE = (BIT0 | BIT1);  // set interupt enable on pins

     GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN2); //Pin real Data send
     GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN3); //Pin real Burst send
     GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0); //Pin notify Data send
     GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
     GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN1); //Pin notify Burst send
     GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN1);



   /* P3DIR |= BIT4;
    P3SEL0 |= BIT4;
    P3SEL1 |= BIT4;*/
    /*
     P5DIR |= BIT5;
     P5SEL0 |= BIT5;
     P5SEL1 |= BIT5;
     */
//    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN5);

}

//NEW UART NEED TEST ------------------------------- TODO
/*void UART_TXData(uint8_t* c, size_t size)
 {
 int position;
 for (position = 0; position < size; position++)
 {
 UCA0TXBUF = c[position];
 // for(i = 0; i < 5000; i++);//delay
 while(UCBUSY);
 }
 }*/

void UART_TXData(uint8_t *c, size_t size)
{
    int position;
    int i;
    for (position = 0; position < size; position++)
    {
        UCA0TXBUF = c[position];
        for (i = 0; i < 5000; i++)
            ;    //delay
    }
}
