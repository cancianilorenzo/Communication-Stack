//MSP430FR5994
#include "Board/Board.h"
#include "TRAP\TRAP.h"
#include "Communication/CommunicationLayer.h"
#include "Physical/Physicallayer.h"
#include <stdio.h>
#include <string.h>

int main(void)
{
    initBoard();
    startPhysicalLayer();
    startCommunicationLayer();
    startTRAPLayer();

    while (1)
    {
        //4bytes data, latest byte destination node
        producedData(0xab, 0x12, 0x23, 0x67, 0x00);

        //Send data
        dataSend();

        //Retrive received data
        storedData data = getdata();

    }

}

