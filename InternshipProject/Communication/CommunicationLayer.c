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
//FRAM VARIABLE INITIALIZATION
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
    //Timer A0_0 ---- ENERGY UPDATE + DATA RX ----
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
        sprintf(message, "INIT-FRAM ");
        UART_TXData(message, strlen(message));

        //se prima inizializzazione
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
        producedData(0xab, 0xf3, 0xff, 0xf3);
        initialized = 0xFF;
    }
}

void startCommunicationLayer(){
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
        sprintf(message, "RECERR ");
        UART_TXData(message, strlen(message));
        currentReceived = 0;
    }
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

/////////////////////////////////
//#pragma vector = PORT5_VECTOR
//__interrupt void P5_ISR(void)
//{
//
//    if (GPIO_getInterruptStatus(GPIO_PORT_P5, GPIO_PIN6))
//    {
//        //producedData("111110011111001111111111111100111");
//        GPIO_clearInterrupt(GPIO_PORT_P5, GPIO_PIN6);
//
//    }
//}
/////////////////////////////////

////FUNCTION TO SEND DATA
void dataSend()
{
    if (dataStatus != DATA_RX)
    {
        if (sendPointer == FRAM_TX_NUMBER)
        {
            sendPointer = 0x00;
        }
        if (storedTX[sendPointer].saved == 0xFF)
        {
            dataStatus = DATA_TX;
            UART_TXDataTOBOARD(storedTX[sendPointer].nodeNumber);
            UART_TXDataTOBOARD(storedTX[sendPointer].data0);
            UART_TXDataTOBOARD(storedTX[sendPointer].data1);
            UART_TXDataTOBOARD(storedTX[sendPointer].data2);
            UART_TXDataTOBOARD(storedTX[sendPointer].data3);
            UART_TXDataTOBOARD(storedTX[sendPointer].CRC0);
            UART_TXDataTOBOARD(storedTX[sendPointer].CRC1);
            UART_TXDataTOBOARD(storedTX[sendPointer].timeStamp);
            storedTX[sendPointer].saved = 0x00;
            sprintf(message, "--SEND %x-- ",storedTX[sendPointer].data0 ,sendPointer);
            UART_TXData(message, strlen(message));
            energyLevel = energyLevel - ENERGY_CONSUMED_TX;
            sendPointer = sendPointer + 0x01;
            dataStatus = DATA_WAIT;
        }
    }

}

void producedData(unsigned char data0, unsigned char data1, unsigned char data2,
                  unsigned char data3)
{
//simulate a circular buffer, old data will be overwrite after FRAM_TX_NUMBER writes (suppose to be too old)
    if (writePointer == FRAM_TX_NUMBER)
    {
        writePointer = 0x00;
    }
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

//    sprintf(message, "CRCR %x", CRCResult);
//    UART_TXData(message, strlen(message));

    storedTX[writePointer].CRC0 = CRCResult >> 8;
    storedTX[writePointer].CRC1 = CRCResult;
    storedTX[writePointer].timeStamp = 0x4D;
    storedTX[writePointer].saved = 0xFF;

    writePointer = writePointer + 0x01;
}

unsigned int canGetData()
{
    if (readPointer == FRAM_RX_NUMBER)
    {
        readPointer = 0x00;
    }

    unsigned int res = 0;

    if (storedRX[readPointer].saved == 0xFF)
    {
        res = 1;
    }
    return res;

}

struct storedData getdata()
{
    //producedData("111110011111001111111111111100111");
    storedData res = storedRX[readPointer];
    storedRX[readPointer].saved = 0x00;
    readPointer = readPointer + 0x01;
    return res;

}

