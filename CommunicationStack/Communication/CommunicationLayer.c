#include <msp430.h>
#include "driverlib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "CommunicationLayer.h"
#include <TRAP\TRAP.h>
#include <Physical\PhysicalLayer.h>
#include <Board\Board.h>

unsigned int currentReceived = 0;

/*-----------------------------------------------------------------------------------------------------------------------------------------*/
#pragma PERSISTENT(writePointer)
unsigned char writePointer = 0x00;

#pragma PERSISTENT(RXPointer)
unsigned char RXPointer = 0x00;

#pragma PERSISTENT(sendPointer)
unsigned char sendPointer = 0x00;

#pragma PERSISTENT(readPointer)
unsigned char readPointer = 0x00;

#pragma PERSISTENT(initialized)
unsigned char initialized = 0x00;

#pragma PERSISTENT(storedTX)
storedData storedTX[FRAM_TX_NUMBER] = { 0x00 };

#pragma PERSISTENT(storedRX)
storedData storedRX[FRAM_RX_NUMBER] = { 0x00 };
/*-----------------------------------------------------------------------------------------------------------------------------------------*/

int dataStatus = DATA_WAIT;

void setTimers()
{
    //Timer A0_0 ---- DATA RX ----
    TA0CCTL0 = CCIE; // enable capture control interupt
    TA0CTL = TASSEL_1 + MC_1 + ID_0; // Use ACLK in up mode, /1 divider --> 250Hz
    TA0CCR0 = 0; // set interupt value
    TA0CCTL0 &= 0x10; // set compare mode

}

//FUNCTION TO ALLOCATE ALL DATA IN FRAM
void FRAMInit()
{
    if (initialized == 0x00)
    {

        writePointer = 0x00;
        sendPointer = 0x00;
        RXPointer = 0x00;
        readPointer = 0x00;

        unsigned int i;
        for (i = 0; i <= FRAM_TX_NUMBER; i++)
        {
            storedTX[i].nodeNumber = 0x00;
            storedTX[i].data0 = 0x00;
            storedTX[i].data1 = 0x00;
            storedTX[i].data2 = 0x00;
            storedTX[i].data3 = 0x00;
            storedTX[i].CRC0 = 0x00;
            storedTX[i].CRC1 = 0x00;
            storedTX[i].timeStamp = 0x00;
            storedTX[i].nodeRX = 0x00;
            storedTX[i].saved = 0x00;
        }
        for (i = 0; i <= FRAM_RX_NUMBER; i++)
        {
            storedRX[i].nodeNumber = 0x00;
            storedRX[i].data0 = 0x00;
            storedRX[i].data1 = 0x00;
            storedRX[i].data2 = 0x00;
            storedRX[i].data3 = 0x00;
            storedRX[i].CRC0 = 0x00;
            storedRX[i].CRC1 = 0x00;
            storedRX[i].timeStamp = 0x00;
            storedRX[i].saved = 0x00;
        }
        initialized = 0xFF;
    }
}

void startCommunicationLayer()
{
    setTimers();
    FRAMInit();
}

void checkRXData()
{

    if (currentReceived == 8)
    {

        if (RXPointer == (unsigned char) FRAM_RX_NUMBER)
        {
            RXPointer = 0x00;
        }

        unsigned char data0RX = storedRX[RXPointer].data0;
        unsigned char data1RX = storedRX[RXPointer].data1;
        unsigned char data2RX = storedRX[RXPointer].data2;
        unsigned char data3RX = storedRX[RXPointer].data3;

        data0RX &= BIT_MASK_0;
        data1RX &= BIT_MASK_1;
        data2RX &= BIT_MASK_2;
        data3RX &= BIT_MASK_3;

        data0RX |= data1RX;
        data2RX |= data3RX;

        data0RX &= data2RX;

        CRC_setSeed(CRC_BASE, CRC_SEED);
        CRC_set16BitData(CRC_BASE, data0RX);
        unsigned int CRCResult = CRC_getResult(CRC_BASE);

        unsigned char CRC0 = CRCResult >> 8;
        unsigned char CRC1 = CRCResult;

        if ((CRC0 == storedRX[RXPointer].CRC0)
                && (CRC1 == storedRX[RXPointer].CRC1))
        {
            storedRX[RXPointer].saved = 0xFF;
            RXPointer = RXPointer + 0x01;
            sprintf(message, "REC ");
            UART_TXData(message, strlen(message));

        }

        currentReceived = 0;
    }
    else
    {
        currentReceived = 0;
    }
    TA0CCR0 = 0;
    dataStatus = DATA_WAIT;
    energyLevel = energyLevel - ENERGY_CONSUMED_RX;

}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void interruptEnergy(void)
{
    if (dataStatus == DATA_RX)
    {
        TA0CCR0 = 0;
        checkRXData();
        dataStatus = DATA_WAIT;

    }

}

void dataSend()
{
    if (dataStatus != DATA_RX)
    {
        if (sendPointer == FRAM_TX_NUMBER)
        {
            sendPointer = 0x00;
        }

        int toSend = -1;
        if (storedTX[sendPointer].saved == 0xFF
                && (canSendTRAP(storedTX[sendPointer].nodeRX)))
        {
            toSend = sendPointer;
        }
        else
        {
            unsigned int i;
            for (i = sendPointer + 1; i < FRAM_TX_NUMBER; i++)
            {
                if (storedTX[i].saved == 0xFF
                        && canSendTRAP(storedTX[i].nodeRX))
                {
                    toSend = i;
                    break;
                }

            }

            if (toSend == -1)
            {
                for (i = 0; i < sendPointer; i++)
                {
                    if (storedTX[i].saved == 0xFF
                            && canSendTRAP(storedTX[i].nodeRX))
                    {
                        toSend = i;
                        break;
                    }

                }

            }
        }

        if (toSend != -1)
        {
            dataStatus = DATA_TX;
            unsigned int timeValue = rand() % 5;

            if (timeValue > storedTX[toSend].timeStamp)
            {
                storedTX[toSend].saved = 0x00;
                if (toSend == sendPointer)
                {
                    sendPointer = sendPointer + 0x01;
                }
            }
            else
            {
                if (canSendTRAP(storedTX[toSend].nodeRX))
                {
                    UART_TXDataTOBOARD(storedTX[toSend].nodeNumber);
                    UART_TXDataTOBOARD(storedTX[toSend].data0);
                    UART_TXDataTOBOARD(storedTX[toSend].data1);
                    UART_TXDataTOBOARD(storedTX[toSend].data2);
                    UART_TXDataTOBOARD(storedTX[toSend].data3);
                    UART_TXDataTOBOARD(storedTX[toSend].CRC0);
                    UART_TXDataTOBOARD(storedTX[toSend].CRC1);
                    UART_TXDataTOBOARD(storedTX[toSend].timeStamp);
                    storedTX[toSend].saved = 0x00;
                    sprintf(message, "SEND ");
                    UART_TXData(message, strlen(message));
                    energyLevel = energyLevel - ENERGY_CONSUMED_TX;
                }

                resetTRAP(storedTX[toSend].nodeRX);
                if (toSend == sendPointer)
                {
                    sendPointer = sendPointer + 0x01;
                }
            }

            dataStatus = DATA_WAIT;
        }
        toSend = -1;
    }
}
void producedData(unsigned char data0, unsigned char data1, unsigned char data2,
                  unsigned char data3, unsigned char nodeRX)
{
//simulate a circular buffer, old data will be overwrite after FRAM_TX_NUMBER writes (suppose to be too old)
    if (writePointer == FRAM_TX_NUMBER)
    {
        writePointer = 0x00;
    }
    storedTX[writePointer].nodeNumber = NODE_NUMBER;
    storedTX[writePointer].data0 = data0;
    storedTX[writePointer].data1 = data1;
    storedTX[writePointer].data2 = data2;
    storedTX[writePointer].data3 = data3;

    data0 &= BIT_MASK_0;
    data1 &= BIT_MASK_1;
    data2 &= BIT_MASK_2;
    data3 &= BIT_MASK_3;

    data0 |= data1;
    data2 |= data3;

    data0 &= data2;

    CRC_setSeed(CRC_BASE, CRC_SEED);
    CRC_set16BitData(CRC_BASE, data0);
    unsigned int CRCResult = CRC_getResult(CRC_BASE);

    storedTX[writePointer].CRC0 = CRCResult >> 8;
    storedTX[writePointer].CRC1 = CRCResult;
    storedTX[writePointer].timeStamp = 0x03;
    storedTX[writePointer].nodeRX = nodeRX;

    storedTX[writePointer].saved = 0xFF;

    writePointer = writePointer + 0x01;
    energyLevel = energyLevel - ENERGY_CONSUMED_DP;
}

unsigned int canGetData()
{
    if (readPointer == FRAM_RX_NUMBER)
    {
        readPointer = 0x00;
    }

    //Clear all exipred packets
    unsigned int i;
    unsigned int timeValue = rand() % 5;
    for (i = 0; i < FRAM_RX_NUMBER; i++)
    {
        if (timeValue > storedTX[i].timeStamp)
        {
            storedRX[i].saved = 0x00;
        }
    }

    //set readPointer

    int toGet = -1;
    if (storedRX[readPointer].saved == 0xFF)
    {
        toGet = readPointer;
    }
    else
    {
        unsigned int i;
        for (i = readPointer; i < FRAM_RX_NUMBER; i++)
        {

            if (storedTX[i].saved == 0xFF)
            {
                toGet = i;
                break;
            }

        }

        if (toGet == -1)
        {
            for (i = 0; i < readPointer; i++)
            {
                if (storedRX[i].saved == 0xFF)
                {
                    toGet = i;
                    break;
                }

            }

        }
    }
    readPointer = toGet;

    unsigned int res = 0;

    if (storedRX[readPointer].saved == 0xFF)
    {
        res = 1;
    }
    return res;

}

struct storedData getdata()
{
    storedData res = { 0x00 };
    if (canGetData())
    {
        res = storedRX[readPointer];
        storedRX[readPointer].saved = 0x00;
        readPointer = readPointer + 0x01;
    }

    return res;

}

