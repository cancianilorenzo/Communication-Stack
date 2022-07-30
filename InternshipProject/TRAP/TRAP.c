#include "TRAP.h"
#include "BoardLib.h"
#include <msp430.h>
#include "driverlib.h"

/*-----------------------------------------------------------------------------------------------------------------------------------------*/
//Auxiliary stuff for debug
char debugUART[64];
#include <string.h>
#include <stdio.h>
int burstDebug = 0;
/*-----------------------------------------------------------------------------------------------------------------------------------------*/

int nodeState[NODES];
int nodeStatus = 0;

#ifndef BOARDLIB_H_
#define DATA_WAIT           0
#define DATA_TX             1
#define DATA_RX             2
int dataStatus = DATA_WAIT;
#endif

/*-----------------------------------------------------------------------------------------------------------------------------------------*/
//TIMER DECLARATION
void TRAPTimer()
{
    //NODE IDENTIFICATION
    NODE_ID_CCCR = CCIE;
    NODE_ID_CR = TASSEL_2 + MC_1 + ID_3; // Use SMCLK in up mode, /8 divider --> 2MHz
    NODE_ID_EV = 0;
    NODE_ID_CCCR &= 0x10;

    //TIMEOUT BURST RECEPTION
    BURST_TIMEOUT_CCCR = CCIE;
    BURST_TIMEOUT_CR = TASSEL_1 + MC_1 + ID_3; // Use SMCLK in up mode, /8 divider
    BURST_TIMEOUT_EV = 0;
    BURST_TIMEOUT_CCCR &= 0x10;

    //BURST REPETITION
    BURST_REPETITION_CCCR = CCIE;
    BURST_REPETITION_CR = TASSEL_1 + MC_1 + ID_0;  // Use ACLK in up mode
    BURST_REPETITION_EV = BURST_REPETITION_PERIOD;
    BURST_REPETITION_CCCR &= 0x10;

    // PULSES SEND
    PULSES_SEND_CCCR = CCIE;
    PULSES_SEND_CR = TASSEL_2 + MC_1 + ID_0; // Use SMCLK in up mode, /8 divider
    PULSES_SEND_EV = 0;
    PULSES_SEND_CCCR &= 0x10;
}
/*-----------------------------------------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------------------------------------------------*/
//FUNCTION TO SELECT THE CORRECT BURST FOR THE PASSED ENERGY LEVEL
int burstValue = 0; //Pulses to send
void selectBurstLengthTRAP(int energyLevel)
{
    if (energyLevel == MAX_ENERGY)
    {
        burstValue = LONG_BURST;
    }
    else if (energyLevel > MIDDLE_ENERGY || energyLevel == MIDDLE_ENERGY)
    {
        burstValue = MIDDLE_BURST;
    }
    else if (energyLevel < MIDDLE_ENERGY)
    {
        burstValue = SHORT_BURST;
    }
}
/*-----------------------------------------------------------------------------------------------------------------------------------------*/

int count = 0; //Pulses RX
int timerValue = 0; //To store the counter value for frequencyID
float frequency = 0; //Frequency calcolated from TimerA2

/*-----------------------------------------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------------------------------------------------*/
//INTERRUPT FOR BURST_REPETITION TIMER
int sendPulses;
#pragma vector = BURST_REPETITION
__interrupt void interruptBurstRepetition(void)
{
    if (dataStatus != DATA_TX)
    {
        selectBurstLengthTRAP(energyLevel);
        TB0CCR0 = 0; //Stop timer B0
        sendPulses = 0;
        nodeStatus = BURST_TX;
        GPIO_setOutputLowOnPin(BURST_TX_PORT, BURST_TX_PIN);
        TB0CCR0 = (1000 / (OOK_NODE * 2));
//        burstDebug++;
//        if (burstDebug == 100)
//        {
//            sprintf(debugUART, "BT ");
//            UART_TXData(debugUART, strlen(debugUART));
//            burstDebug = 0;
//        }

    }
}
/*-----------------------------------------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------------------------------------------------*/
//INTERRUPT FOR PULSES_SEND TIMER
#pragma vector = PULSES_SEND
__interrupt void interruptPulsesSending(void)
{
    if (sendPulses == (burstValue * 2)) //ON-OFF PIN --> 2 cycles
    {
        TB0CCR0 = 0; //Stop timer
        GPIO_setOutputLowOnPin(BURST_TX_PORT, BURST_TX_PIN);
        nodeStatus = BURST_WAIT;
    }

    if (sendPulses != (burstValue * 2))
    {
        GPIO_toggleOutputOnPin(BURST_TX_PORT, BURST_TX_PIN);
        sendPulses++;
    }

}
/*-----------------------------------------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------------------------------------------------*/

#pragma vector = BURST_RX_VECTOR
__interrupt void interruptBurstRX(void)
{
//Burst RX ISR
//    if (P1IFG & BIT4)
    if (GPIO_getInterruptStatus(BURST_RX_PORT, BURST_RX_PIN))
    {
//        sprintf(debugUART, "RECB ");
//        UART_TXData(debugUART, strlen(debugUART));

        if ((nodeStatus != BURST_TX) && (dataStatus == DATA_WAIT))
        {
//            sprintf(debugUART, "0 ");
//            UART_TXData(debugUART, strlen(debugUART));
            timerValue = TA2R;

            {
                nodeStatus = BURST_RX;
                if (count == 0)
                {
                    NODE_ID_CR = TASSEL_2 + MC_2 + ID_3; //250khz --> SMCLK 1MHZ!!!
                    BURST_TIMEOUT_CR = TASSEL_1 + MC_1 + ID_3;
                }
                if (count == 20)
                {
                    NODE_ID_CR = TASSEL_2 + MC_0 + ID_0; //Stop timer
                }
                count++;
//                sprintf(debugUART, "%d ", count);
//                UART_TXData(debugUART, strlen(debugUART));
                BURST_TIMEOUT_EV = 0; //Stop timer
                BURST_TIMEOUT_EV = TIMEOUT; //restart timer to avoid glitches

            }
        }

//        P1IFG &= ~BIT4;
        GPIO_clearInterrupt(BURST_RX_PORT, BURST_RX_PIN);

    }

}
/*-----------------------------------------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------------------------------------------------*/

int OOK_NODE_INCOME[] = {10, 15, 20, 25, 30, 35, 40, 45, 50 };

#pragma vector = NODE_ID
__interrupt void frequencyAllocation(void)
{
    BURST_TIMEOUT_CR = TASSEL_1 + MC_0 + ID_3; //Stop timer
    NODE_ID_CR = TASSEL_2 + MC_0 + ID_0; //Stop timer

//    sprintf(debugUART, "C: %d ", count);
//    UART_TXData(debugUART, strlen(debugUART));

    if (count > (64 - BURST_GUARD))
    {
        frequency = (float) 125 / ((float) timerValue / (float) 21);
//        sprintf(debugUART, "F %.1f ", frequency);
//        UART_TXData(debugUART, strlen(debugUART));

        int i;
        for (i = 0; i < NODES - 1; i++)
        {
//            if((frequency > (OOK_NODE_INCOME[i] - 2.5)) && (frequency < (OOK_NODE_INCOME[i] + 2.5)))
//            if (frequency > ((OOK_NODE_INCOME[i] + 2.5) - 5))
            if (frequency < OOK_NODE_INCOME[i] + 0.3)
            {
                if (count > ((LONG_BURST - BURST_GUARD) - 1))
                {
                    nodeState[i] = LONG_BURST;
                }
                else if (count > ((MIDDLE_BURST - BURST_GUARD) - 1))
                {
                    nodeState[i] = MIDDLE_BURST;
                }
                else if (count > ((SHORT_BURST - BURST_GUARD) - 1))
                {
                    nodeState[i] = SHORT_BURST;
                }

//                sprintf(debugUART, "NODE%d %d ", i, nodeState[i]);
//                UART_TXData(debugUART, strlen(debugUART));
                break;

            }
        }

    }
    count = 0;
    TA2R = 0;
    frequency = 0;
    timerValue = 0;
    nodeStatus = BURST_WAIT;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------------------------------------------------*/

#pragma vector = TIMER2_A0_VECTOR
__interrupt void T2A0_ISR(void)
{
//    sprintf(debugUART, "TA2 ");
//    UART_TXData(debugUART, strlen(debugUART));
// This is needed to avoid blocking board
// if there is an overflow on timer
}

/*-----------------------------------------------------------------------------------------------------------------------------------------*/

int canSendTRAP(int choosenNode)
{

    int canSend = 0;

    if (burstValue == LONG_BURST
            && (nodeState[choosenNode] == MIDDLE_BURST
                    || nodeState[choosenNode] == LONG_BURST))
    {
        canSend = 1;
    }

    return canSend;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------------------------------------------------*/
void TRAPGPIO()
{
//    BURST RX
//    P1IES = BIT4;  // pull-up
//    P1IFG = 0;              // clear interrupt flags
//    P1IE = BIT4;  // set interupt enable on pins

    GPIO_setAsInputPinWithPullUpResistor(BURST_RX_PORT, BURST_RX_PIN);
    GPIO_selectInterruptEdge(BURST_RX_PORT, BURST_RX_PIN,
                             GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_clearInterrupt(BURST_RX_PORT, BURST_RX_PIN);
    GPIO_enableInterrupt(BURST_RX_PORT, BURST_RX_PIN);

    GPIO_setAsOutputPin(BURST_TX_PORT, BURST_TX_PIN); //Pin real Burst send


}
/*-----------------------------------------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------------------------------------------------*/
void resetTRAP(int nodeNumber){
    nodeState[nodeNumber] = 0;
    burstValue = 0;
}
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
