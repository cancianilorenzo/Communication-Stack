/*
 Lorenzo Canciani
 lorenzo.canciani@studenti.unitn.it
 2022
 */

#include "BoardLib.h"
#include <msp430.h>
#include <msp430.h>
#include "driverlib.h"

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

void setTimers()
{
    //Timer A0_0 ---- ENERGY UPDATE
    TA0CCTL0 = CCIE; // enable capture control interupt
    //Set timer speed at 62.5 kHz, min speed for CLK at 16 MHz
    TA0CTL = TASSEL_1 + MC_1 + ID_3;  // Use ACLK in up mode, /8 divider
    TA0CCR0 = 0; // set interupt value
    TA0CCTL0 &= 0x10; // set compare mode

    //Timer A1_0
     TA1CCTL0 = CCIE; // enable capture control interupt
     TA1CTL = TASSEL_1 + MC_1 + ID_3;  // Use SMCLK in up mode, /8 divider
     TA1CCR0 = 0; // set interupt value
     TA1CCTL0 &= 0x10; // set compare mode

     //Timer A2_0 ---- NODE IDENTIFICATION
     //---TODO----probabilmente non devo utilizzare gli interrupt
     TA2CCTL0 = CCIE; // enable capture control interupt
     TA2CTL = TASSEL_1 + MC_1 + ID_0;  // Use ACLK in up mode, /1 divider --> 500kHz
     TA2CCR0 = 0; // set interupt value
     TA2CCTL0 &= 0x10; // set compare mode

    //Timer A4_0 ---- BURST REPETITION
    TA4CCTL0 = CCIE; // enable capture control interupt
    TA4CTL = TASSEL_1 + MC_1 + ID_3;  // Use ACLK in up mode, /8 divider
    TA4CCR0 = 0; // set interupt value
    TA4CCTL0 &= 0x10; // set compare mode

    //Timer B0_0 ---- PULSES SEND
    TB0CCTL0 = CCIE; // enable capture control interupt
    TB0CTL = TASSEL_2 + MC_1 + ID_3;  // Use SMCLK in up mode, /8 divider
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
    CSCTL3 = DIVA__32 | DIVS__1 | DIVM__1; // Set all dividers to 1 for 16MHz operation NO! ACLK divideb by 32 (16MHz/32 --> 500kHz)
    CSCTL0_H = 0; // Lock CS registers                      // Lock CS registers

}

void pinDeclaration()
{
    //3.0 DATA RX----3.1 BURST RX
    P3IES = (BIT0 | BIT1);  // set interrupt on edge select
    P3IFG = 0;              // clear interrupt flags
    P3IE = (BIT0 | BIT1);  // set interupt enable on pins

    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN2); //Pin real Data send
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN3); //Pin real Burst send
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0); //Pin notify Data send
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN1); //Pin notify Burst send
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN1);
}
