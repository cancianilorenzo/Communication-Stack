//MSP430FR5994
#include <msp430.h>
#include "driverlib.h"
#include <TRAP\TRAP.h>
#include <BoardLib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(void)

{
    initBoard();
    pinDeclaration();
    setBoardFrequency();
    UARTInit();
    setTimers();
    startEnergySimulation();
    TRAPTimer();

    sprintf(message, "INIT0 ");
    UART_TXData(message, strlen(message));

    while (1)
    {

/*-------------------------- BEGIN CODE TO TEST THE FRAM -----------------------------------------------*/
//        if (store != '0')
//        {
//            store = '0';
//            while (1)
//            {
//                sprintf(message, "1 ");
//                UART_TXData(message, strlen(message));
//            }
//        }
//        if (store == '0')
//        {
//            store = '1';
//            while (1)
//            {
//                sprintf(message, "0 ");
//                UART_TXData(message, strlen(message));
//            }
//        }
/*-------------------------- END CODE TO TEST THE FRAM -----------------------------------------------*/


        //canSendTRAP(1);

        //Store state
//
//        if (dataStored.state0 == '1' || dataStored.state1 == '1')
//        {
//
//            if (nodeStatus != BURST_RX && dataStatus != DATA_RX)
//            {
//
//                if (canSendTRAP(nodeState[0], nodeState[LEFT_NODE])
//                        && dataStored.state0 == '1') //Manca controllo se dato salvato (chiamato dall'app quindi non serve??)
//                {
//                    dataSend12(dataStored.data0);
//                    dataStored.state0 = '0';
//                    sprintf(message, "SEND ");
//                    UART_TXData(message, strlen(message));
//                    nodeState[LEFT_NODE] = 0;
//
//                }
//
//                if (canSendTRAP(nodeState[0], nodeState[RIGHT_NODE])
//                        && dataStored.state1 == '1') //Manca controllo dato su FRAM
//                {
//                    dataSend15(dataStored.data1);
//                    dataStored.state1 = '0';
//                    sprintf(message, "SEND1 ");
//                    UART_TXData(message, strlen(message));
//                    dataStatus = DATA_WAIT;
//                    nodeState[RIGHT_NODE] = 0;
//
//                }
//
//            }
//        }
//
//        if (canStore0 == 1 || canStore1 == 1)
//        {
//            if (canStore0 == 1)
//            {
//                FRAMWrite(data0, 0);
//                sprintf(message, "REC ");
//                UART_TXData(message, strlen(message));
//                canStore0 = 0;
//            }
//
//            if (canStore1 == 1)
//            {
//                FRAMWrite(data1, 1);
//                sprintf(message, "REC1 ");
//                UART_TXData(message, strlen(message));
//                dataStatus = DATA_WAIT;
//                canStore1 = 0;
//            }
//
//        }
//        if (canSendTRAP(1))
//        {
//            dataSend12("010101010101010101");
//            nodeState[1] = 0;
//            sprintf(message, "SEND ");
//            UART_TXData(message, strlen(message));
//            FRAMWrite("010101010101", 1);
//        }

    }

}

