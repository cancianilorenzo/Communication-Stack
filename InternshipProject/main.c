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
    sprintf(message, "INIT1 ");
    UART_TXData(message, strlen(message));

    TRAPGPIO();
    TRAPTimer();

    while (1)
    {

        if (canSendTRAP(1))
        {
            dataSend();
            resetTRAP(1);
        }

        if (canGetData())
        {
            storedData data = getdata();
//            sprintf(message, "POP %x ", data.data0);
//            UART_TXData(message, strlen(message));
            producedData("111110011111001111111111111100111");

        }
    }

}

