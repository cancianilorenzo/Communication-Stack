//MSP430FR5994
#include "Board/Board.h"
#include "TRAP\TRAP.h"
#include "Communication/CommunicationLayer.h"
#include "Physical/Physicallayer.h"
#include <stdio.h>
#include <string.h>


/*
          Code to simulate a RESET
          TA0R = 0x3FFF;
        ((void (*)())(unsigned int)&TA0R)();
*/


int main(void)
{
    initBoard();
    startPhysicalLayer();
    startCommunicationLayer();
    startTRAPLayer();

    sprintf(message, "INIT0 ");
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
            sprintf(message, "D: %x ", data.data0);
            UART_TXData(message, strlen(message));
            if(data.data0 == 0xab){
                producedData(0xf9, 0xf3, 0xff, 0xf3);
            }
            else{
                producedData(0xab, 0xf3, 0xff, 0xf3);
            }
            //producedData(data.data0, data.data1, data.data2, data.data3);
//            producedData(0xab, 0xf3, 0xff, 0xf3);
//            producedData(0xf9, 0xf3, 0xff, 0xf3);

        }
    }

}

