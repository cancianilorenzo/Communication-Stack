/*


 ----TODO
 Secondo TRAP, riceve il livello d'energia solo se l'energia del nodo è massima?
 Initialize ALL PORTS --> unused port to LOW to avoid energy consumption


 Rivedere logica invio dati
 */

#include <msp430.h>
#include "driverlib.h"
#include <TRAP\TRAP.h>
#include <BoardLib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ACTUAL_NODE OOK_NODE0

#define ENERGY_CHANGE       50      // energy variation parameter
#define ENERGY_INCREMENT    5       // energy maximum increment

#define MAX_ENERGY 100
#define MIDDLE_ENERGY 70
#define LOW_ENERGY 35

#define ENERGY_UPDATE 500 //500ms energy update
#define BURST_REPETITION 1000 //Repetition of burst in ms
#define BURST_GUARD 40 //Burst Guard

#define ENERGY_CONSUMED_TX 70 //Energy consumed in TX
#define ENERGY_CONSUMED_RX 35 //Energy consumed in RX

#define NODES 3 //# of nodes
#define OOK_NODE0 15
#define OOK_NODE1 25
#define OOK_NODE2 35
#define TIMEOUT   7
#define DATA_TX_TIME 400

#define COUNT_FREQ_ID 20
#define MSG_SIZE 64

//Array to store burstLength of other nodes
int nodeState[NODES];
int nodeNum;
int sendPulses;
char message[MSG_SIZE];

int count = 0; //pulses incoming

float frequency;

typedef enum
{
    DATA_WAIT = 0, DATA_TX = 1, DATA_RX = 2
} DATA_Status;

DATA_Status dataStatus = DATA_WAIT;

typedef enum
{
    BURST_WAIT = 0, BURST_TX = 1, BURST_RX = 2
} NODE_Status;

NODE_Status nodeStatus = BURST_WAIT;

int energyLevel = 0;

int energy_count = 0;
int energy_count_limit = 0;
int energy_increment = 0;

void dataSend(); //function to call when nodes start transimitting data

int main(void)
{
    initBoard();
    //__delay_cycles(60);
    pinDeclaration();
    setBoardFrequency();
    //__delay_cycles(60); //Already present in BoarLib.c
    UARTInit();
    setTimers();

    nodeState[0] = 100; //init value for node[0] burstLength

    energy_count_limit = (ENERGY_CHANGE / 2)
            + (rand() % (ENERGY_CHANGE / 2 + 1));
    energy_increment = rand() % (ENERGY_INCREMENT + 1);

    //start energy timer A0
    TA0CCR0 = 0;
    //----------------------------- TODO -------------------------
    TA0CCR0 = 125;

    //start burst timer A4
    TA4CCR0 = 0; //Reset timer, can be removed
    //----------------------------- TODO -------------------------
    TA4CCR0 = 250; //250 per 1 sec

    //TA1CCR0 = DATA_TX_TIME; // set end value of timer

    while (1)
    {
        //select burst
        nodeState[0] = selectBurstLength(energyLevel);
        //---------------------------- TODO EDIT FUNCTION IN THE FUTURE --------------------------------
        if (energy_count >= energy_count_limit)
        {
            energy_count_limit = (ENERGY_CHANGE / 2)
                    + (rand() % (ENERGY_CHANGE / 2 + 1));
            energy_increment = rand() % (ENERGY_INCREMENT + 1);
            energy_count = 0;
        }

        //----------------------------- TODO Rivedere giro nodi
        if (nodeStatus != BURST_RX)
        {
            //Custom priority switched with a true/false random condition (0/1)

            if (nodeState[0] == LONG_BURST && nodeState[1] == MIDDLE_BURST
                    && ACTUAL_NODE == OOK_NODE0)
            {
                dataSend();

            }
            if (nodeState[2] == LONG_BURST && nodeState[0] == MIDDLE_BURST
                    && ACTUAL_NODE == OOK_NODE2)
            {
                dataSend();

            }
            if (nodeState[1] == LONG_BURST && nodeState[2] == MIDDLE_BURST
                    && ACTUAL_NODE == OOK_NODE1)
            {
                dataSend();

            }

        }

    }

}

//---------------------------------------------------------------------ISR AND FUNCTIONS---------------------------------------------------------------------------------------//

//---------------------------------------------------------------------TIMER ENERGY UPDATE---------------------------------------------------------------------------------------//
#pragma vector = TIMER0_A0_VECTOR
__interrupt void T0A0_ISR(void)
{

    if (dataStatus == DATA_WAIT)
    {
        int energy_step = rand() % (energy_increment + 1);
        energyLevel = energyLevel + energy_step;
        if (energyLevel >= MAX_ENERGY)
            energyLevel = MAX_ENERGY;
        energy_count++;

        energy_step = 120 + energy_step;

        sprintf(message, "UE ");
        UART_TXData(message, strlen(message));
    }

}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//

//---------------------------------------------------------------------TIMER DATA TX and RX---------------------------------------------------------------------------------------//
#pragma vector = TIMER1_A0_VECTOR
__interrupt void T1A0_ISR(void)
{
    if (dataStatus == DATA_TX)
    {
        TA1CCR0 = 0; //Stop timer A1
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN2);
        energyLevel = 0;
        dataStatus = DATA_WAIT;
        sprintf(message, "DTX ");
        UART_TXData(message, strlen(message));
    }

    if (dataStatus == DATA_RX)
    {
        TA1CCR0 = 0; //Stop timer A1
        //GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN1);
        //sprintf(message, "EDRX ");
        UART_TXData(message, strlen(message));
        dataStatus = DATA_WAIT;
    }
    TA1CCR0 = 0;

}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//

//---------------------------------------------------------------------TIMER BURST REPETITION TX---------------------------------------------------------------------------------------//
#pragma vector = TIMER4_A0_VECTOR
__interrupt void T4A0_ISR(void)
{
    if (dataStatus != DATA_TX)
    {
        TB0CCR0 = 0; //Stop timer B0
        sendPulses = 0;
        nodeStatus = BURST_TX;
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN3);
        TB0CCR0 = (2000 / ACTUAL_NODE);
        sprintf(message, "BTX ");
        UART_TXData(message, strlen(message));
    }

}

#pragma vector = TIMER0_B0_VECTOR
__interrupt void T0B0_ISR(void)
{
    if (sendPulses == (nodeState[0] * 2)) //ON-OFF PIN --> 2 cycles
    {
        TB0CCR0 = 0; //Stop timer
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN3);
        nodeStatus = BURST_WAIT;
    }

    if (sendPulses != (nodeState[0] * 2))
    {
        GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN3);
        sendPulses++;
    }

}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//

//---------------------------------------------------------------------TIMER TIMEOUT BURST RX---------------------------------------------------------------------------------------//
#pragma vector = TIMER3_A0_VECTOR
__interrupt void T3A0_ISR(void)
{
    if (count > (64 - BURST_GUARD))
    {
        sprintf(message, "BRX "); //Entra due volte qua.... TODO
        UART_TXData(message, strlen(message));
        sprintf(message, "%d ", count);
        UART_TXData(message, strlen(message));
        //sprintf(message, "%.2f ", TA2R);
        //UART_TXData(message, strlen(message));
        frequency = 16000.000 / ((float) TA2R / (float) 21);
        //sprintf(message, "%.2f ", frequency);
        //UART_TXData(message, strlen(message));
        frequency = frequency * 1000;
        //sprintf(message, "%.2f ", frequency);
        //UART_TXData(message, strlen(message));
        printf("%.2f ", frequency);

    }
    count = 0;
    TA2R = 0;
    nodeStatus = BURST_WAIT;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//

//---------------------------------------------------------------------PORT 3 ISR DATA + BURST RX---------------------------------------------------------------------------------------//
#pragma vector = PORT3_VECTOR
__interrupt void P3_ISR(void)
{
    if (P3IFG & BIT0)
    {
        if (/*(energyLevel == MAX_ENERGY) &&*/(nodeStatus != BURST_TX)
                && (dataStatus == DATA_WAIT))
        {

            {
                nodeStatus = BURST_RX;
                if (count == 0)
                {
                    TA2CTL = TASSEL_2 + MC_2 + ID_0; //change timer mode to avoid set offset ----- TODO explain better AND NEED TEST
                    //TA3CCR0 = TIMEOUT; //restart timer to avoid glitches
                }

                else if (count == COUNT_FREQ_ID)
                {
                    TA2CTL = TASSEL_2 + MC_0 + ID_0;
                    //TA3CCR0 = TIMEOUT; //restart timer to avoid glitches
                }
                count++;
                TA3CCR0 = 0; //Stop timer A1
                TA3CCR0 = TIMEOUT; //restart timer to avoid glitches

            }
        }

        P3IFG &= ~BIT0;

    }

    if (P3IFG & BIT1)
    {

        if ((energyLevel == ENERGY_CONSUMED_RX)
                || (energyLevel > ENERGY_CONSUMED_RX))
        {
            dataStatus = DATA_RX;
            TA1CCR0 = 0; //Stop timer A1
            TA1CCR0 = DATA_TX_TIME; //Start timer A1
            energyLevel = energyLevel - ENERGY_CONSUMED_RX;
            sprintf(message, "DRX ");
            UART_TXData(message, strlen(message));
        }
        else
        {
            sprintf(message, "ERDRX "); //Error data reception
            UART_TXData(message, strlen(message));
        }
        P3IFG &= ~BIT1;

    }

}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//

#pragma vector = TIMER2_A0_VECTOR
__interrupt void T2A0_ISR(void)
{
    //TAxR
    //Need to implement this section of code otherwise when timer reach end value ----------- TODO check
    //app crash
}

//---------------------------------------------------------------------dataSend() Function---------------------------------------------------------------------------------------//
void dataSend()
{
    TA1CCR0 = 0; //Stop timer 1
    TA1CCR0 = DATA_TX_TIME; // set end value of timer
    energyLevel = energyLevel - ENERGY_CONSUMED_TX;
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN2);
    dataStatus = DATA_TX;
    nodeStatus = BURST_WAIT;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
