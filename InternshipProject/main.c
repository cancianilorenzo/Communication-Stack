/*
 Lorenzo Canciani
 lorenzo.canciani@studenti.unitn.it
 2022
 */

//Try to put state int instead of unsigned char, should fix
#include <msp430.h>
#include "driverlib.h"
#include <TRAP\TRAP.h>
#include <BoardLib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define ACTUAL_NODE 0
#define LEFT_NODE 1
#define RIGHT_NODE 2

#define ENERGY_CHANGE       50      // energy variation parameter
#define ENERGY_INCREMENT    5       // energy maximum increment

#define MAX_ENERGY 100
#define MIDDLE_ENERGY 70
#define LOW_ENERGY 35

#define ENERGY_UPDATE 500 //500ms energy update
#define BURST_GUARD 40 //Burst Guard

#define ENERGY_CONSUMED_TX 70 //Energy consumed in TX
#define ENERGY_CONSUMED_RX 35 //Energy consumed in RX

#define NODES 3 //# of nodes
#define OOK_NODE0 15
#define OOK_NODE1 25
#define OOK_NODE2 35

//Identification frequency
#define IDOOK_NODE0 19
#define IDOOK_NODE1 29
#define IDOOK_NODE2 39

//Timeout in BURST RX
#define TIMEOUT   15
//#define DATA_TX_TIME 65535 //Max 65536

#define MSG_SIZE 64 //Uart MSG size
#define DATA_SIZE 16

#define WRITE_SIZE 17 //16 char for data + 1 char for state of FSM --> 48 + 1 bytes (Total FRAM space 256KB)

//Array to store burstLength of other nodes
int nodeState[NODES];
int nodeNum;
int sendPulses;

//UART message
char message[MSG_SIZE];

int count = 0; //pulses incoming
int timerValue = 0;

float frequency;

char data0[DATA_SIZE]; //16 bytes --> only 1 data stored
char data1[DATA_SIZE]; //16 bytes --> only 1 data stored
int currentDataBuffer = 0;
unsigned int canStore0 = 0;
unsigned int canStore1 = 0;

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

typedef struct DATA
{
    char data0[16];
    char data1[16];
    char data2[16];
    char state0;
    char state1;
    char state2;
} DATA;

void dataSend12(char*); //function to call when nodes start transimitting data on pin 1.2
void dataSend14(char*); //function to call when nodes start transimitting data on pin 1.4
void FRAMWrite(char*, int); //Function to write on FRAM

//FRAM declaration
#if defined(__TI_COMPILER_VERSION__)
#pragma PERSISTENT(dataStored)
DATA dataStored = { .data0 = "e", .data1 = "e", .data2 = "e", .state0 = 'e',
                    .state1 = 'e', .state2 = 'e' };
#elif defined(__IAR_SYSTEMS_ICC__)
 __persistent struct Data data =
 {   0};
 #elif defined(__GNUC__)
 unsigned long __attribute__((persistent)) FRAM_write[WRITE_SIZE] =
 {   0};
#else
#error Compiler not supported!
#endif

int main(void)

{

    initBoard();
    pinDeclaration();
    setBoardFrequency();
    UARTInit();
    setTimers();

    sprintf(message, "INIT0 ");
    UART_TXData(message, strlen(message));

    //nodeState[1] = 256; //init value for node[0] burstLength

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

        // ENERGY SIMULATION
        if (energy_count >= energy_count_limit)
        {
            energy_count_limit = (ENERGY_CHANGE / 2)
                    + (rand() % (ENERGY_CHANGE / 2 + 1));
            energy_increment = rand() % (ENERGY_INCREMENT + 1);
            energy_count = 0;
        }

        //Store state

        if (dataStored.state0 == '1' || dataStored.state1 == '1')
        {
//                sprintf(message, "WS ");
//                UART_TXData(message, strlen(message));

            if (nodeStatus != BURST_RX && dataStatus != DATA_RX)
            {

                if (nodeState[ACTUAL_NODE] == LONG_BURST
                        && (nodeState[LEFT_NODE] == MIDDLE_BURST
                                || nodeState[LEFT_NODE] == LONG_BURST)
                        && dataStored.state0 == '1')
                {
                    dataSend12(dataStored.data0);
                    dataStored.state0 = '0';
                    sprintf(message, "SEND ");
                    UART_TXData(message, strlen(message));
                    nodeState[LEFT_NODE] = 0;

                }

                if (nodeState[ACTUAL_NODE] == LONG_BURST
                        && (nodeState[RIGHT_NODE] == MIDDLE_BURST
                                || nodeState[RIGHT_NODE] == LONG_BURST)
                        && dataStored.state1 == '1')
                {
                    dataSend14(dataStored.data1);
                    dataStored.state1 = '0';
                    sprintf(message, "SEND1 ");
                    UART_TXData(message, strlen(message));
                    nodeState[RIGHT_NODE] = 0;

                }

            }
        }

        if (canStore0 == 1 || canStore1 == 1)
        {
            if (canStore0 == 1)
            {
                FRAMWrite(data0, 0);
                sprintf(message, "REC ");
                UART_TXData(message, strlen(message));
//                sprintf(message, "%s ", dataStored.data0);
//                UART_TXData(message, strlen(message));
                canStore0 = 0;
            }

            if (canStore1 == 1)
            {
                FRAMWrite(data1, 1);
                sprintf(message, "REC1 ");
                UART_TXData(message, strlen(message));
                canStore1 = 0;
            }

        }

        //dataStored.state0, dataStored.state1, dataStored.state2
        /*
         ******************************* FSM ************************************************************
         */
//        switch (dataStored.state)
//        {
//
//        case '0':
//            //IDLE Waiting for data RX
//            if (currentStored0 || currentStored1)
//            {
//                dataStored.state = '1';
//            }
//            break;
//
//        case '1':
//            //DATA TX
//            if (nodeStatus != BURST_RX && dataStatus != DATA_RX)
//            {
//                nodeState[0] = selectBurstLength(energyLevel);
//                if (nodeState[ACTUAL_NODE] == LONG_BURST
//                        && (nodeState[1] == MIDDLE_BURST
//                                || nodeState[1] == LONG_BURST))
//                {
//                    if (currentStored0)
//                    {
//                        dataSend12(dataStored.data0);
//                        nodeState[1] = 0;
//                        dataStored.state = '0';
//                    }
//
//                }
////                if (nodeState[ACTUAL_NODE] == LONG_BURST
////                        && (nodeState[2] == MIDDLE_BURST
////                                || nodeState[2] == LONG_BURST))
////                {
////
////                    if (currentStored1)
////                    {
////                        dataSend14(dataStored.data1);
////                        //nodeState[2] = 0;
////                    }
////                    dataStored.state = '0';
////                }
//                dataStored.state = '0';
//            }
//
//            break;
//
//        case '2':
//            if (canStore0)
//            {
//                FRAMWrite(data0, 0);
//                currentDataBuffer = 0;
//                currentStored0 = 1;
//                canStore0 = 0; //Accept data in incoming
//            }
//            /*if (canStore1)
//             {
//             FRAMWrite(data1, 1);
//             currentDataBuffer = 0;
//             currentStored1 = 1;
//             canStore1 = 0; //Accept data in incoming
//             }*/
//            dataStored.state = '0';
//
//            break;
//
//        default:
//            sprintf(message, "Error, reset the board!");
//            UART_TXData(message, strlen(message));
//        }
    }

}

//---------------------------------------------------------------------ISR AND FUNCTIONS-----------------------------------------------------------------------------------------//

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

        energy_step = 120 + energy_step / 20;
    }

}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//

//---------------------------------------------------------------------TIMER DATA TX and RX---------------------------------------------------------------------------------------//
#pragma vector = TIMER1_A0_VECTOR
__interrupt void T1A0_ISR(void)
{
//    if (dataStatus == DATA_TX)
//    {
//        TA1CCR0 = 0; //Stop timer A1
//        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN2);
//        energyLevel = 0;
//        dataStatus = DATA_WAIT;
//    }

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
        TB0CCR0 = 0; //Stop timer B0
        sendPulses = 0;
        nodeStatus = BURST_TX;
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN3);
        TB0CCR0 = (1000 / (OOK_NODE0 * 2));
//        sprintf(message, "BT ");
//        UART_TXData(message, strlen(message));
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
//        sprintf(message, "BT ");
//        UART_TXData(message, strlen(message));
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
//            sprintf(message, " node2 ");
//            UART_TXData(message, strlen(message));
            if (count > ((LONG_BURST - BURST_GUARD) - 1))
            {
                nodeState[2] = LONG_BURST;
            }
            else if (count > ((MIDDLE_BURST - BURST_GUARD) - 1))
            {
                nodeState[2] = MIDDLE_BURST;
            }
            else if (count > ((SHORT_BURST - BURST_GUARD) - 1))
            {
                nodeState[2] = SHORT_BURST;
            }
        }
        else if ((frequency > (IDOOK_NODE1 - 9.9)))
        {
            sprintf(message, " node2 ");
            UART_TXData(message, strlen(message));
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
    frequency = 0; //for frequency reset
    timerValue = 0;
    nodeStatus = BURST_WAIT;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//

//---------------------------------------------------------------------PORT 3 ISR DATA + BURST RX---------------------------------------------------------------------------------------//
#pragma vector = PORT3_VECTOR
__interrupt void P3_ISR(void)
{
//Burst RX ISR
    if (P3IFG & BIT0)
    {
// TODO modify condition to enter the cycle

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

// Data RX ISR
    if (P3IFG & BIT1)
    {

        if (((energyLevel == ENERGY_CONSUMED_RX)
                || (energyLevel > ENERGY_CONSUMED_RX))
                && dataStatus == DATA_WAIT)
        {
            dataStatus = DATA_RX;
            TA1CCR0 = 0; //Stop timer A1
//            TA1CCR0 = DATA_TX_TIME; //Start timer A1

//            sprintf(message, "0DR31 ");
//            UART_TXData(message, strlen(message));

            if (canStore0 == 0)
            {
                dataStatus = DATA_WAIT;
                if (currentDataBuffer < DATA_SIZE + 1)
                {
                    //0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16
                    //a raising edge cannto happen if the was no falling edge before!!!!
                    data0[currentDataBuffer] = '0';
                    currentDataBuffer++;
                    data0[currentDataBuffer] = '1';
                    currentDataBuffer++;
//                    sprintf(message, "1 %d ", currentDataBuffer);
//                    UART_TXData(message, strlen(message));
                    if (currentDataBuffer > DATA_SIZE)
                    {
//                        sprintf(message, "DENTRO 17 ", currentDataBuffer);
//                        UART_TXData(message, strlen(message));
                        //data0[DATA_SIZE - 1] = '\0';
                        //data0[currentDataBuffer] = '1';
                        canStore0 = 1;
                        energyLevel = energyLevel - ENERGY_CONSUMED_RX;
                        currentDataBuffer = 0;
                    }
                }

            }

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

        if (((energyLevel == ENERGY_CONSUMED_RX)
                || (energyLevel > ENERGY_CONSUMED_RX))
                && dataStatus == DATA_WAIT)
        {
            dataStatus = DATA_RX;
            TA1CCR0 = 0; //Stop timer A1
            //            TA1CCR0 = DATA_TX_TIME; //Start timer A1

            //            sprintf(message, "0DR31 ");
            //            UART_TXData(message, strlen(message));

            if (canStore1 == 0)
            {
                dataStatus = DATA_WAIT;
                if (currentDataBuffer < DATA_SIZE + 1)
                {
                    //0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16
                    //a raising edge cannto happen if the was no falling edge before!!!!
                    data1[currentDataBuffer] = '0';
                    currentDataBuffer++;
                    data1[currentDataBuffer] = '1';
                    currentDataBuffer++;
                    //                    sprintf(message, "1 %d ", currentDataBuffer);
                    //                    UART_TXData(message, strlen(message));
                    if (currentDataBuffer > DATA_SIZE)
                    {
                        //                        sprintf(message, "DENTRO 17 ", currentDataBuffer);
                        //                        UART_TXData(message, strlen(message));
                        //data0[DATA_SIZE - 1] = '\0';
                        //data0[currentDataBuffer] = '1';
                        canStore1 = 1;
                        energyLevel = energyLevel - ENERGY_CONSUMED_RX;
                        currentDataBuffer = 0;
                    }
                }

            }

        }
        else
        {
            sprintf(message, "0ER32 "); //Error data reception
            UART_TXData(message, strlen(message));
        }

        P3IFG &= ~BIT2;
    }

    dataStatus = DATA_WAIT;

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//

#pragma vector = TIMER2_A0_VECTOR
__interrupt void T2A0_ISR(void)
{
// This is needed to avoid blocking board
// if there is an overflow on timer
}

//--------------------------------------------------------------------- SUPPORT FUNCTION ---------------------------------------------------------------------------------------//

void FRAMWrite(char *data, int pos)
{
    int i;

    if (pos == 0)
    {
        dataStored.state0 = '0';
        for (i = 0; i < strlen(data); i++)
        {
            dataStored.data0[i] = data[i];
        }
        dataStored.state0 = '1'; //Succesfully stored in FRAM
//        sprintf(message, "ST-%c ", dataStored.state0);
//        UART_TXData(message, strlen(message));
    }

    if (pos == 1)
    {
        dataStored.state1 = '0';
        for (i = 0; i < strlen(data); i++)
        {
            dataStored.data1[i] = data[i];
        }
        dataStored.state1 = '1'; //Succesfully stored in FRAM
//        sprintf(message, "SD1 ");
//        UART_TXData(message, strlen(message));
    }

}
//--------------------------------------------------------------------- DATASEND FUNCTION ---------------------------------------------------------------------------------------//
void dataSend12(char *messageToSend)
{

    //010101010101010101

    TA1CCR0 = 0; //Stop timer 1
    int len = strlen(messageToSend);
    int i = 0;
    for (i = 0; i < len; i++)
    {
        dataStatus = DATA_TX;
//        __delay_cycles(7372800);
        if (messageToSend[i] == '1')
        {
            GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN2);
            __delay_cycles(75550);
            //sprintf(message, " 1 ");
            //UART_TXData(message, strlen(message));
        }
        else
        {
            GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN2);
            __delay_cycles(75550);
            //sprintf(message, " 0 ");
            //UART_TXData(message, strlen(message));
        }
    }
    energyLevel = energyLevel - ENERGY_CONSUMED_TX;
    //GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN2);
    dataStatus = DATA_WAIT;
    nodeStatus = BURST_WAIT;
}

void dataSend14(char *messageToSend)
{
    //010101010101010101

    TA1CCR0 = 0; //Stop timer 1
    int len = strlen(messageToSend);
    int i = 0;
    for (i = 0; i < len; i++)
    {
        dataStatus = DATA_TX;
//        __delay_cycles(7372800);
        if (messageToSend[i] == '1')
        {
            GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4);
            __delay_cycles(75550);
            //sprintf(message, " 1 ");
            //UART_TXData(message, strlen(message));
        }
        else
        {
            GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN4);
            __delay_cycles(75550);
            //sprintf(message, " 0 ");
            //UART_TXData(message, strlen(message));
        }
    }
    energyLevel = energyLevel - ENERGY_CONSUMED_TX;
    //GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN2);
    dataStatus = DATA_WAIT;
    nodeStatus = BURST_WAIT;
}
