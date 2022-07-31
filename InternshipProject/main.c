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

    TRAPGPIO();
    TRAPTimer();

    sprintf(message, "INIT0 ");
    UART_TXData(message, strlen(message));

    while (1)
    {

//        if (!dataToSend())
//        {
//            //dataProduction();
//            //....
//            //FRAMWrite(productData);
//        }
//        if (dataToSend())
//        {
        if (canSendTRAP(0))
        {
            dataSend("0111101101", 0);
            resetTRAP(0);
        }
//        }
    }

}

