/*


 ----TODO
 Initialize ALL PORTS --> unused port to LOW to avoid energy consumption
 move all floating point operation in RAM to reduce power consuption (Faster operation--> lower consuption)


 Rivedere logica invio dati
 */

#include <msp430.h>
#include "driverlib.h"
#include <TRAP\TRAP.h>
#include <BoardLib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define ACTUAL_NODE 0

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
#define IDOOK_NODE0 19
#define IDOOK_NODE1 29
#define IDOOK_NODE2 39
#define TIMEOUT   15
#define DATA_TX_TIME 62500

#define MSG_SIZE 64

//Array to store burstLength of other nodes
int nodeState[NODES];
int nodeNum;
int sendPulses;

//UART message
char message[MSG_SIZE];

int count = 0; //pulses incoming
int timerValue = 0;

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

//Energy simulation
int energyLevel = 0;
int energy_count = 0;
int energy_count_limit = 0;
int energy_increment = 0;
int showEnergy = 0;

void dataSend12(); //function to call when nodes start transimitting data on pin 1.2
void dataSend14(); //function to call when nodes start transimitting data on pin 1.4

int main(void)
{

    initBoard();
    pinDeclaration();
    setBoardFrequency();
    UARTInit();
    setTimers();

    nodeState[0] = 100; //init value for node[0] burstLength

    energy_count_limit = (ENERGY_CHANGE / 2)
            + (rand() % (ENERGY_CHANGE / 2 + 1));
    energy_increment = rand() % (ENERGY_INCREMENT + 1);

    //Energy simulation Timer
    TA0CCR0 = 125;

    //Burst repetition timer
    TA4CCR0 = 250; //250 per 1 sec

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
        if (nodeStatus != BURST_RX && dataStatus != DATA_RX)
        {

            if (nodeState[0] == LONG_BURST && (nodeState[1] == MIDDLE_BURST || nodeState[1] == LONG_BURST))
            {
                dataSend12();
                nodeState[1] = 0;

            }
            if (nodeState[0] == LONG_BURST && nodeState[2] == MIDDLE_BURST)
            {
                dataSend14();
                nodeState[2] = 0;
            }

        }

    }

}

//---------------------------------------------------------------------ISR AND FUNCTIONS---------------------------------------------------------------------------------------//

//---------------------------------------------------------------------dataSend() Function---------------------------------------------------------------------------------------//
void dataSend12()
{
    TA1CCR0 = 0; //Stop timer 1
    sprintf(message, "DS12 ");
    UART_TXData(message, strlen(message));
    TA1CCR0 = DATA_TX_TIME; // set end value of timer
    energyLevel = energyLevel - ENERGY_CONSUMED_TX;
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN2);
    dataStatus = DATA_TX;
    nodeStatus = BURST_WAIT;
}

void dataSend14()
{
    TA1CCR0 = 0; //Stop timer 1
    sprintf(message, "DS14 ");
    UART_TXData(message, strlen(message));
    TA1CCR0 = DATA_TX_TIME; // set end value of timer
    energyLevel = energyLevel - ENERGY_CONSUMED_TX;
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4);
    dataStatus = DATA_TX;
    nodeStatus = BURST_WAIT;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//

//---------------------------------------------------------------------TIMER ENERGY UPDATE---------------------------------------------------------------------------------------//
#pragma vector = TIMER0_A0_VECTOR
__interrupt void T0A0_ISR(void)
{

    if (dataStatus == DATA_WAIT)
    {
        showEnergy++;
        int energy_step = rand() % (energy_increment + 1);
        energyLevel = energyLevel + energy_step;
        if (energyLevel >= MAX_ENERGY)
            energyLevel = MAX_ENERGY;
        energy_count++;

        energy_step = 120 + energy_step;

//        if (showEnergy == 500)
//        {
//            showEnergy = 0;
//            sprintf(message, "EL %d ", energyLevel);
//            UART_TXData(message, strlen(message));
//        }
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
    }

    if (dataStatus == DATA_RX)
    {
        TA1CCR0 = 0; //Stop timer A1
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
//        sprintf(message, "BT ");
//        UART_TXData(message, strlen(message));
        TB0CCR0 = 0; //Stop timer B0
        sendPulses = 0;
        nodeStatus = BURST_TX;
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN3);
        TB0CCR0 = (1000 / (OOK_NODE0 * 2));
    }

}

#pragma vector = TIMER0_B0_VECTOR
__interrupt void T0B0_ISR(void)
{
    if (sendPulses == (nodeState[ACTUAL_NODE] * 2)) //ON-OFF PIN --> 2 cycles
    {
        TB0CCR0 = 0; //Stop timer
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN3);
        nodeStatus = BURST_WAIT;
    }

    if (sendPulses != (nodeState[ACTUAL_NODE] * 2))
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
    TA3CTL = TASSEL_1 + MC_0 + ID_3; //Stop timer
    TA2CTL = TASSEL_2 + MC_0 + ID_0; //Stop timer

    if (count > (64 - BURST_GUARD))
    {

        frequency = (float) 250 / ((float) timerValue / (float) count);

        if ((frequency > (IDOOK_NODE2 - 9.9)))

        {
            if (count > ((LONG_BURST - BURST_GUARD) - 1))
            {
                nodeState[2] = LONG_BURST;
//                sprintf(message, "C%d node2 ", LONG_BURST);
//                UART_TXData(message, strlen(message));
            }
            else if (count > ((MIDDLE_BURST - BURST_GUARD) - 1))
            {
                nodeState[2] = MIDDLE_BURST;
//                sprintf(message, "C%d node2 ", MIDDLE_BURST);
//                UART_TXData(message, strlen(message));
            }
            else if (count > ((SHORT_BURST - BURST_GUARD) - 1))
            {
                nodeState[2] = SHORT_BURST;
//                sprintf(message, "C%d node2 ", SHORT_BURST);
//                UART_TXData(message, strlen(message));
            }
        }
        else if ((frequency > (IDOOK_NODE1 - 9.9)))
        {
            if (count > ((LONG_BURST - BURST_GUARD) - 1))
            {
                nodeState[1] = LONG_BURST;
//                sprintf(message, "C%d node1 ", LONG_BURST);
//                UART_TXData(message, strlen(message));
            }
            else if (count > ((MIDDLE_BURST - BURST_GUARD) - 1))
            {
                nodeState[1] = MIDDLE_BURST;
//                sprintf(message, "C%d node1 ", MIDDLE_BURST);
//                UART_TXData(message, strlen(message));
            }
            else if (count > ((SHORT_BURST - BURST_GUARD) - 1))
            {
                nodeState[1] = SHORT_BURST;
//                sprintf(message, "C%d node1 ", SHORT_BURST);
//                UART_TXData(message, strlen(message));
            }
        }

    }
    count = 0;
    TA2R = 0;
    frequency = 0;
    timerValue = 0;
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
            timerValue = TA2R;

            {
                nodeStatus = BURST_RX;
                if (count == 0)
                {
                    TA2CTL = TASSEL_2 + MC_2 + ID_2; //250khz
                    TA3CTL = TASSEL_1 + MC_1 + ID_3;
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
                || (energyLevel > ENERGY_CONSUMED_RX)
                        && dataStatus == DATA_WAIT)
        {
            dataStatus = DATA_RX;
            TA1CCR0 = 0; //Stop timer A1
            TA1CCR0 = DATA_TX_TIME; //Start timer A1
            energyLevel = energyLevel - ENERGY_CONSUMED_RX;
            sprintf(message, "0DR31 ");
            UART_TXData(message, strlen(message));
        }
        else
        {
            sprintf(message, "0ER31 "); //Error data reception
            UART_TXData(message, strlen(message));
        }

        P3IFG &= ~BIT1;
    }

    if (P3IFG & BIT2)
    {

        if ((energyLevel == ENERGY_CONSUMED_RX)
                || (energyLevel > ENERGY_CONSUMED_RX)
                        && dataStatus == DATA_WAIT)
        {
            dataStatus = DATA_RX;
            TA1CCR0 = 0; //Stop timer A1
            TA1CCR0 = DATA_TX_TIME; //Start timer A1
            energyLevel = energyLevel - ENERGY_CONSUMED_RX;
            sprintf(message, "0DR32 ");
            UART_TXData(message, strlen(message));
        }
        else
        {
            sprintf(message, "0ER32 "); //Error data reception
            UART_TXData(message, strlen(message));
        }

        P3IFG &= ~BIT2;
    }

}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//

#pragma vector = TIMER2_A0_VECTOR
__interrupt void T2A0_ISR(void)
{

}
