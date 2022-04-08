#include "Timer.h"
#include "TRAP.h"
#include <msp430.h>
#include "driverlib.h"

void timerInitilization()
{
    //Timer A0_1 FOR ENERGY INTERRUPT GENERATION
    TA0CCR0 = 0; // stop timer if started before
    TA0CCTL0 = CCIE; // enable capture control interupt
    TA0CTL = TASSEL_1 + MC_1 + ID_3; // Use SMCLK in up mode, /8 divider --> 2Mhz

    //TA0CTL &= ~MC; // stop timer
    //Timer A1_1
    TA1CCTL0 = CCIE;
    TA1CTL = TASSEL_2 + MC_1 + ID_3;

    //Timer A4_1
    TA4CCTL0 = CCIE;
    TA4CTL = TASSEL_1 + MC_1 + ID_3;

    //Timer B0_1
    TB0CCTL0 = CCIE;
    TB0CTL = TASSEL_2 + MC_1 + ID_3;
}
//------------------START STOP TIMER---------------------------------------------

void startTimerA0(int endValue)
{
    TA0CCR0 = endValue;
    TA0CCTL0 &= 0x10;
}

void stopTimerA0()
{
    TA0CCR0 = 0; //Should work, hopefully
}

void startTimerA1(int endValue)
{
    TA1CCR0 = endValue;
    TA1CCTL0 &= 0x10;
}

void stopTimerA1()
{
    TA1CCR0 = 0;
}

void startTimerA4(int endValue)
{
    TA4CCR0 = endValue;
    TA4CCTL0 &= 0x10;
}

void stopTimerA4()
{
    TA4CCR0 = 0;
}

void startTimerB0(int endValue)
{
    TB0CCR0 = endValue;
    TB0CCTL0 &= 0x10;
}

void stopTimerB0()
{
    TB0CCR0 = 0;
}

//------------------TIMER HANDLER---------------------------------------------

void handlerTimerA0()
{
    //For the moment not used
    //Energy is simulated and update on main.c side!!
}

/*Burst activation handler*/
void handlerTimerA1(int pulsesCountTX, MOD_Status nodeMode)
{
    stopTimerB0();
    pulsesCountTX = 0;

    nodeMode = BURST_TX;
    startTimerB0(10000/(2*TX_FREQUENCY));

}

/*BURST_RX TIMEOUT | DATA_TX | DATA_RX*/
void handlerTimerA4(MOD_Status nodeMode, TX_Status nodeStatus, int pulsesCountRX)
{
    //Reached timeout for BURST_RX
    if (nodeMode == BURST_RX)
    {
        nodeMode = BURST_WAIT;
        pulsesCountRX = 0;
        stopTimerA4();
    }

    //Finish DATA_TX
    if (nodeStatus == DATA_TX)
    {
        stopTimerA4();
        nodeStatus = DATA_WAIT;

        //Power off pin to show TX ----- TODO
    }

    //Finish DATA_RX
    if (nodeStatus == DATA_RX)
    {
        nodeStatus = DATA_WAIT;
    }
}


/*Burst generation handler*/
void handlerTimerB0(MOD_Status nodeMode, TX_Status nodeStatus, int pulsesCountTX, int burstLength)
{
    GPIO_toggleOutputOnPin(GPIO_PORT_P6, GPIO_PIN0);
    pulsesCountTX++;
    if (pulsesCountTX == burstLength)
    {
        stopTimerB0();
        pulsesCountTX = 0; //reset pulses
        nodeMode = BURST_WAIT;
    }
}


