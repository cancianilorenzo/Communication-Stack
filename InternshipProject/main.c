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
    sprintf(message, "INIT0 ");
    UART_TXData(message, strlen(message));

    TRAPGPIO();
    TRAPTimer();


    sprintf(message, "DP ");
    UART_TXData(message, strlen(message));

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
            sprintf(message, "POP %x ", data.data0);
            UART_TXData(message, strlen(message));

        }
    }

}

