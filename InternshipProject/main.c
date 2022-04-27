/*
 Possibile controllare frequenza scheda????????
 Secondo TRAP, riceve solo se energia massima?
 ----TODO
 Initialize ALL PORTS --> unused port to LOW to avoid energy consumption
 Fix DATA_TX Timer (65ms impossibile with the actual MCLK)
 At 30kHz about 0.017s to send the (256*2) burst
 */

#include <msp430.h>
#include "driverlib.h"
#include <TRAP\TRAP.h>
#include <BoardLib.h>
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
#define BURST_GUARD 40 //Burst Guard

#define ENERGY_CONSUMED_TX 70 //Energy consumed in TX
#define ENERGY_CONSUMED_RX 35 //Energy consumed in RX

#define NODES 3 //# of nodes
#define OOK_NODE0 15
#define OOK_NODE1 25
#define OOK_NODE2 35
#define TIMEOUT   50
#define DATA_TX_TIME 400

#define COUNT_FREQ_ID 5

//Array to store burstLength of other nodes
int nodeState[NODES];
int nodeNum;
int sendPulses;
char *message;

int count = 0; //pulses incoming
int timerA2Value = 0;

int frequency = 0;

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
    //__delay_cycles(60); //Already present in BoarLib.c
    UARTInit();
    setTimers();
    message = "EI ";
    UART_TXData(message);

    pinDeclaration();

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
                    dataSend();

                }
                if (nodeState[2] == LONG_BURST && nodeState[0] == MIDDLE_BURST
                        && ACTUAL_NODE == 2)
                {
                    dataSend();

                }
                if (nodeState[1] == LONG_BURST && nodeState[2] == MIDDLE_BURST
                        && ACTUAL_NODE == 1)
                {
                    dataSend();

                }

            }
            else
            {
                if (nodeState[0] == LONG_BURST && nodeState[2] == MIDDLE_BURST
                        && ACTUAL_NODE == 0)
                {
                    dataSend();

                }
                if (nodeState[1] == LONG_BURST && nodeState[0] == MIDDLE_BURST
                        && ACTUAL_NODE == 1)
                {
                    dataSend();

                }
                if (nodeState[2] == LONG_BURST && nodeState[1] == MIDDLE_BURST
                        && ACTUAL_NODE == 2)
                {
                    dataSend();

                }

            }
        }

    }

}

void dataSend()
{
    TA1CCR0 = 0; //Stop timer 1
    TA1CCR0 = DATA_TX_TIME; // set end value of timer

    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
    energyLevel = energyLevel - ENERGY_CONSUMED_TX;
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN2);
    message = "SD ";
    UART_TXData(message);
    dataStatus = DATA_TX;
    nodeStatus = BURST_WAIT;
}

//*************************************
//               ISR
//*************************************

//HANDLER ENERGY GENERATION Timer
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
        //printf("[UPDATE] EnergyLevel --> %d\n", energyLevel);
        message = "UE ";
        UART_TXData(message);
    }

}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void T1A0_ISR(void)
{

    //Timer ausiliario
    //Reached timeout for burst reception (no more pulse on the pin)
    if ((nodeStatus == BURST_RX) && (dataStatus == DATA_RX))
    {
        nodeStatus = BURST_WAIT;
        TA1CCR0 = 0; //Stop timer A1
        TA2CCR0 = 0; // Stop timer used to calculate node frequency
        TA2R = 0;
        //frequency = 0;
        //frequency = (250 * timerA2Value); //Perchè avevo inserito questa linea? Debug per capire se fosse effettivamente zero? Hard to say, only God can know
        frequency = (frequency / COUNT_FREQ_ID);
        // printf("FREQUENCYBURSTTIMER --> %d\n", burstTimer);
        // printf("TIMER--> %d\n", timerA2Value);

        //printf("FREQUENCY--> %d\n", frequency);
        //printf("COUNT--> %d\n", count);
        //count = 0;
        itoa(count, message, 10);
        UART_TXData(message);
        message = "--";
        UART_TXData(message);
        itoa(frequency, message, 10);
        UART_TXData(message);
        count = 0;

    }
    if (dataStatus == DATA_TX)
    {
        //printf("[DATA_SEND] Data Sent\n");
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN2);
        dataStatus = DATA_WAIT;
        TA1CCR0 = 0; //Stop timer A1
        energyLevel = 0;
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN1);
    }

    if (dataStatus == DATA_RX)
    {
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN1);
        dataStatus = DATA_WAIT;
        message = "EDR ";
        TA1CCR0 = 0; //Stop timer A1
        UART_TXData(message);
    }

}
//FIXED
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------//
//Timer BURST REPETITION
#pragma vector = TIMER4_A0_VECTOR
__interrupt void T4A0_ISR(void)
{
    if (dataStatus != DATA_TX)
    {
        TB0CCR0 = 0; //Stop timer B0
        sendPulses = 0;
        nodeStatus = BURST_TX;
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN3);
        TB0CCR0 = (2000 / OOK_NODE0);
    }

}


#pragma vector = TIMER0_B0_VECTOR
__interrupt void T0B0_ISR(void)
{
    if (sendPulses == (nodeState[0] * 2)) //ON-OFF PIN --> 2 cycles
    {
        TB0CCR0 = 0; //Stop timer
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN3);
        message = "BT ";
        UART_TXData(message);
        //sendPulses = 0;
        nodeStatus = BURST_WAIT;
    }

    if (sendPulses != (nodeState[0] * 2))
    {
        GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN3);
        sendPulses++;
    }

}
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------//


//Handler BURST_RX pin 1.2
#pragma vector = PORT3_VECTOR
__interrupt void P3_ISR(void)
{

    if (P3IFG & BIT1)
    {
        if (/*(energyLevel == MAX_ENERGY) &&*/(nodeStatus != BURST_TX)
                && (dataStatus == DATA_WAIT))
        {

            //printf("BURST_RX\n");
            {
                nodeStatus = BURST_RX;
                if (count == 0)
                {
                    TA2CCR0 = 65535; // Start timer to count delay for obtain node frequency
                    TA1CCR0 = 0;            //Stop timer A1
                    TA1CCR0 = TIMEOUT;
                }

                else if (count == COUNT_FREQ_ID)
                {
                    timerA2Value = TA2R; //Store value of timer
                    TA2CCR0 = 0;  // Stop timer used to calculate node frequency
                    TA1CCR0 = 0;            //Stop timer A1
                    TA1CCR0 = TIMEOUT;
                }
                count++;
                TA1CCR0 = 0; //Stop timer A1
                TA1CCR0 = TIMEOUT; //restart timer

            }
        }

//Clear interrupt flag
        P3IFG &= ~BIT1;

    }

    //Should work

    if (P3IFG & BIT0)
    {
        if ((energyLevel == ENERGY_CONSUMED_RX)
                || (energyLevel > ENERGY_CONSUMED_RX))
        {
            dataStatus = DATA_RX;
            TA1CCR0 = 0; //Stop timer A1
            TA1CCR0 = DATA_TX_TIME; //Start timer A1
            GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
            GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN1);
            energyLevel = energyLevel - ENERGY_CONSUMED_RX;
            message = "DR ";
            UART_TXData(message);
        }
        else
        {
            message = "EDR "; //Error data reception
            UART_TXData(message);
        }
        P3IFG &= ~BIT0;

    }

}

#pragma vector = TIMER2_A0_VECTOR
__interrupt void T2A0_ISR(void)
{
    //TAxR
    //Need to implement this section of code otherwise when timer reach end value
    //app crash
}

