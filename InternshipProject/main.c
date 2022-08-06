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
    FRAMInit();

    TRAPGPIO();
    TRAPTimer();

    unsigned char primo = 0x02;
    unsigned char secondo = 0x09;
    sprintf(message, "ret %d ", primo < secondo);
    UART_TXData(message, strlen(message));

    producedData("10100100111110011001111111110010");
    producedData("111110011111001111111111111100111");

    while (1)
    {
        if (canGetData())
        {
            storedData data = getdata();
            sprintf(message, "SDG %x ", data.saved);
            UART_TXData(message, strlen(message));
        }
    }

}

