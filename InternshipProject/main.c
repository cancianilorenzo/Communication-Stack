/*
 ----TODO
 Initialize ALL PORTS --> unused port to LOW to avoid energy consumption
 Fix DATA_TX Timer (65ms impossibile with the actual MCLK)

 */

#include <msp430.h>
#include "driverlib.h"
#include <stdio.h> //printf() doesn't work
#include <TRAP\TRAP.h>
#include <BoardLib.h>
#include <time.h>
#include <stdlib.h>

#define ACTUAL_NODE 0
//#define ACTUAL_NODE 1
//#define ACTUAL_NODE 2

#define ENERGY_CHANGE       50      // energy variation parameter
#define ENERGY_INCREMENT    5       // energy maximum increment

#define MAX_ENERGY 100
#define MIDDLE_ENERGY 70
#define LOW_ENERGY 35

#define ENERGY_UPDATE 500 //500ms energy update
#define BURST_REPETITION 1000 //Repetition of burst in ms

#define ENERGY_CONSUMED_TX 70 //Energy consumed in TX
#define ENERGY_CONSUMED_RX 35 //Energy consumed in RX

#define NODES 3 //# of nodes
#define OOK_NODE0 30
#define OOK_NODE1 25
#define OOK_NODE2 35
#define TIMEOUT   100     // timeout for the burst rx pulses (us)

//Array to store burstLength of other nodes
int nodeState[NODES];
int nodeNum;
int sendPulses;

int count = 0; //pulses incoming

//#define DATA_TX_TIME        50000   // data transmission time

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
    setBoardFrequency();
    __delay_cycles(60);
    setTimers();
    printf("End init\n");

    pinDeclaration();

    nodeState[0] = 100;

    energy_count_limit = (ENERGY_CHANGE / 2)
            + (rand() % (ENERGY_CHANGE / 2 + 1));
    energy_increment = rand() % (ENERGY_INCREMENT + 1);

    //start energy timer A0
    TA0CCR0 = 0;
    TA0CCR0 = (62.5 * 0.5);

    //start burst timer A4
    TA4CCR0 = 0; //Reset timer, can be removed
    TA4CCR0 = 62.5;

    while (1)
    {
        //select burst
        nodeState[0] = selectBurstLength(energyLevel);
        //---------------------------- TODO EDIT FUNCTION --------------------------------
        if (energy_count >= energy_count_limit)
        {
            energy_count_limit = (ENERGY_CHANGE / 2)
                    + (rand() % (ENERGY_CHANGE / 2 + 1));
            energy_increment = rand() % (ENERGY_INCREMENT + 1);
            energy_count = 0;
        }

        //custom values for other nodes for testing goal
        //nodeState[1] = MIDDLE_BURST;
        //nodeState[2] = MIDDLE_BURST;

        if (nodeStatus != BURST_RX)
        {
            //Custom priority switched with a true/false random condition (0/1)
            int num = (rand() % (1 - 0 + 1)) + 0;
            if (num == 0)
            {
                if (nodeState[0] == LONG_BURST && nodeState[1] == MIDDLE_BURST
                        && ACTUAL_NODE == 0)
                {
                    //Send data from 0 to 1
                    printf("[DATA] From 0 to 1\n");
                    //nodeState[1] = 0;
                    dataSend();

                }
                if (nodeState[2] == LONG_BURST && nodeState[0] == MIDDLE_BURST
                        && ACTUAL_NODE == 2)
                {
                    //Send data from 2 to 0
                    printf("[DATA] From 2 to 0\n");
                    //nodeState[0] = 0;
                    dataSend();

                }
                if (nodeState[1] == LONG_BURST && nodeState[2] == MIDDLE_BURST
                        && ACTUAL_NODE == 1)
                {
                    //Send data from 1 to 2
                    printf("[DATA] From 1 to 2\n");
                    //nodeState[2] = 0;
                    dataSend();

                }

            }
            else
            {
                if (nodeState[0] == LONG_BURST && nodeState[2] == MIDDLE_BURST
                        && ACTUAL_NODE == 0)
                {
                    //Send data from 0 to 2
                    printf("[DATA] From 0 to 2\n");
                    //nodeState[2] = 0;
                    dataSend();

                }
                if (nodeState[1] == LONG_BURST && nodeState[0] == MIDDLE_BURST
                        && ACTUAL_NODE == 1)
                {
                    //Send data from 1 to 0
                    printf("[DATA] From 1 to 0\n");
                    //nodeState[0] = 0;
                    dataSend();

                }
                if (nodeState[2] == LONG_BURST && nodeState[1] == MIDDLE_BURST
                        && ACTUAL_NODE == 2)
                {
                    //Send data from 2 to 1
                    printf("[DATA] From 2 to 1\n");
                    //nodeState[1] = 0;
                    dataSend();

                }

            }
        }

    }

}

void dataSend()
{
    TA1CCR0 = 0; //Stop timer A1
    TA1CCR0 = 40000; //Start timer A1

    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
    energyLevel = energyLevel - ENERGY_CONSUMED_TX;
    dataStatus = DATA_TX;
    nodeStatus = BURST_WAIT;
}

//*************************************
//               ISR
//*************************************

//HANDLER INTERRUPT TIMER A0
#pragma vector = TIMER0_A0_VECTOR
__interrupt void T0A0_ISR(void)
{
    //GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
    //GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN1);

//Alessandro's timer 3
    if (dataStatus == DATA_WAIT)
    {
        int energy_step = rand() % (energy_increment + 1);
        energyLevel = energyLevel + energy_step;
        if (energyLevel >= MAX_ENERGY)
            energyLevel = MAX_ENERGY;
        energy_count++;

        energy_step = 120 + energy_step;
        //printf("[UPDATE] EnergyLevel --> %d\n", energyLevel);

    }
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void T1A0_ISR(void)
{
    //Timer ausiliario
    //Reached timeout for burst reception (no more pulse on the pin)
    /*if (nodeStatus == BURST_RX)
     {
     //Fermo timer T2A0, ne leggo il valore, divido per capire che nodo è, assegno al nodo il burst rettificato
     nodeStatus = BURST_WAIT;
     TA1CCR0 = 0; //Stop timer A1
     }*/
    if (dataStatus == DATA_TX)
    {
        printf("[DATA_SEND] Data Sent\n");
        dataStatus = DATA_WAIT;
        TA1CCR0 = 0; //Stop timer A1
        energyLevel = 0;
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    }

    if (dataStatus == DATA_RX)
    {
        printf("[DATA_REC] Data Received\n");
        dataStatus = DATA_WAIT;
    }
}
/*
 #pragma vector = TIMER2_A0_VECTOR
 __interrupt void T2A0_ISR(void)
 {
 // timer per il calcolo della frequenza di ricezione ----- TODO ---- TAxR counter
 }
 */
//Timer BURST REPETITION
#pragma vector = TIMER4_A0_VECTOR
__interrupt void T4A0_ISR(void)
{
    TB0CCR0 = 0; //Stop timer B0
    sendPulses = 0;
    nodeStatus = BURST_TX;
    printf("SendBurst\n");
    //printf("[BURST] EnergyLevel --> %d\n", energyLevel);
    TB0CCR0 = (2000 / OOK_NODE0);
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN1);
}

#pragma vector = TIMER0_B0_VECTOR
__interrupt void T0B0_ISR(void)
{
    GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN3);
    //__delay_cycles(100000);
    //printf("PULSES SEND --> %d\n", sendPulses);
    sendPulses++;

    if (sendPulses == (nodeState[0] * 2)) //ON-OFF PIN --> 2 cycles
    {

        TB0CCR0 = 0; //Stop timer
        sendPulses = 0;
        //printf("Fine invio burst\n");
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN1);
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN3);
        nodeStatus = BURST_WAIT;
        //printf("END PULSES SEND\n");
    }
}

//Handler BURST_RX pin 1.2
#pragma vector = PORT3_VECTOR
__interrupt void P3_ISR(void)
{

    if (P3IFG & BIT1)
    {
        GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);

        /* if (energyLevel == MAX_ENERGY && nodeStatus != BURST_TX
         && dataStatus == DATA_WAIT)
         printf("BURST_RX\n");
         {
         nodeStatus = BURST_RX;
         if (count == 0)
         {
         TA1CCR0 = 0; //Stop timer A1
         TA1CCR0 = (2000 / TIMEOUT);
         }
         TA1CCR0 = 0; //Stop timer A1
         TA1CCR0 = (2000 / TIMEOUT);
         count++;

         }*/

        //Clear interrupt flag
        P3IFG &= ~BIT1;

    }

    if (P3IFG & BIT0)
    {
        printf("[DATA_REC] BEFORE -- EnergyLevel --> %d\n", energyLevel);
        if ((energyLevel == ENERGY_CONSUMED_RX) || (energyLevel > ENERGY_CONSUMED_RX))
        {
            //Stop timer 1 A0
            //Start timer 1 A0
            dataStatus = DATA_RX;
            energyLevel = energyLevel - ENERGY_CONSUMED_RX;
            TA1CCR0 = 0; //Stop timer A1
            TA1CCR0 = 40000; //Start timer A1
            printf("[DATA_REC] AFTER -- EnergyLevel --> %d\n", energyLevel);
        }
        else
        {
            printf("[DATA_REC] Error in energyLevel\n");
        }
        P3IFG &= ~BIT0;

    }

}

