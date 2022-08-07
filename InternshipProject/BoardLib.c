#include "BoardLib.h"
#include <msp430.h>
#include "driverlib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

unsigned int currentReceived = 0;

//Uart message
char message[MSG_SIZE];

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

//Energy simulation
int energyLevel = 0;
int energy_count = 0;
int energy_count_limit = 0;
int energy_increment = 0;

void UARTInit()
{
    //FOR DEBUG TO PC
    P2SEL0 &= ~(BIT0 | BIT1);
    P2SEL1 |= (BIT0 | BIT1);                           // USCI_A3 UART operation

    UCA0CTLW0 = UCSWRST;                                   // Put eUSCI in reset
    UCA0CTLW0 |= UCSSEL__SMCLK;                    // CLK = SMCLK
    UCA0BRW = 8;
    UCA0MCTLW = 0xD600;
    UCA0CTLW0 &= ~UCSWRST;                                 // Initialize eUSCI

    UCA0IE |= UCRXIE;

    //FOR DATA SENDING BETWEEN BOARDS
    P2SEL0 &= ~(BIT5 | BIT6);
    P2SEL1 |= (BIT5 | BIT6);                           // USCI_A3 UART operation

    UCA1CTLW0 = UCSWRST;                                   // Put eUSCI in reset
    UCA1CTLW0 |= UCSSEL__SMCLK;                    // CLK = SMCLK
    UCA1BRW = 8;
    UCA1MCTLW = 0xD600;
    UCA1CTLW0 &= ~UCSWRST;                                 // Initialize eUSCI

    UCA1IE |= UCRXIE;

}

void setTimers()
{
    //Timer A0_0 ---- ENERGY UPDATE + DATA RX ----
    TA0CCTL0 = CCIE; // enable capture control interupt
    TA0CTL = TASSEL_1 + MC_1 + ID_0; // Use ACLK in up mode, /1 divider --> 250Hz
    TA0CCR0 = 0; // set interupt value
    TA0CCTL0 &= 0x10; // set compare mode

}

void setBoardFrequency()
{
    // Configure one FRAM waitstate as required by the device datasheet for MCLK
    // operation beyond 8MHz _before_ configuring the clock system.
    FRCTL0 = FRCTLPW | NWAITS_1;

    // Clock System Setup
    CSCTL0_H = CSKEY_H;                     // Unlock CS registers
    CSCTL1 = DCOFSEL_0;                     // Set DCO to 1MHz
    // Set SMCLK = MCLK = DCO, ACLK = VLOCLK
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;

    // Per Device Errata set divider to 4 before changing frequency to
    // prevent out of spec operation from overshoot transient
    CSCTL3 = DIVA__4 | DIVS__4 | DIVM__4; // Set all corresponding clk sources to divide by 4 for errata
    CSCTL1 = DCOFSEL_4 | DCORSEL; // Set DCO to 16MHz (Digital Controlled Oscillator)

    // Delay by ~10us to let DCO settle. 60 cycles = 20 cycles buffer + (10us / (1/4MHz))
    __delay_cycles(60);
    CSCTL3 = DIVA__32 | DIVS__16 | DIVM__1; // SMCLK set to 1MHz, ACLK set to 250Hz
    CSCTL0_H = 0; // Lock CS registers                      // Lock CS registers

}

void pinDeclaration()
{

//    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN5);
//    GPIO_selectInterruptEdge(GPIO_PORT_P5, GPIO_PIN5,
//    GPIO_HIGH_TO_LOW_TRANSITION);
//    GPIO_clearInterrupt(GPIO_PORT_P5, GPIO_PIN5);
//    GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN5);
//
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN6);
    GPIO_selectInterruptEdge(GPIO_PORT_P5, GPIO_PIN6,
    GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_clearInterrupt(GPIO_PORT_P5, GPIO_PIN6);
    GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN6);
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
        initialized = 0xFF;
    }
}

void startEnergySimulation()
{
    TA0CCR0 = ENERGY_UPDATE_RATE;
    energy_count_limit = (ENERGY_CHANGE / 2)
            + (rand() % (ENERGY_CHANGE / 2 + 1));
    energy_increment = rand() % (ENERGY_INCREMENT + 1);
}

void initBoard()
{
    WDTCTL = WDTPW | WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5; // Disable the GPIO power-on default high-impedance mode
    __enable_interrupt(); // enable global interrupts

    pinDeclaration();
    setBoardFrequency();
    UARTInit();
    setTimers();
    startEnergySimulation();
    FRAMInit();

}

void UART_TXData(char *c, size_t size)
{
    //Can't find sysTick like on MSP432 so this is a possible approach
    int position;
    int i;
    for (position = 0; position < size; position++)
    {
        UCA0TXBUF = c[position];
        for (i = 0; i < 5000; i++)
            ;
    }
}

void UART_TXDataTOBOARD(unsigned char c)
{
//    sprintf(message, "DUB ");
//    UART_TXData(message, strlen(message));
    unsigned int j;
    UCA1TXBUF = (unsigned char) c;
    for (j = 0; j < 30000; j++)
        ;

}

void checkRXData()
{

    if (currentReceived == 8)
    {
        sprintf(message, "REC ");
        UART_TXData(message, strlen(message));

        if (RXPointer >= (unsigned char) FRAM_RX_NUMBER)
        {
            RXPointer = 0x00;
        }

//        sprintf(message, "--%x", storedRX[RXPointer].nodeNumber);
//        UART_TXData(message, strlen(message));
//        sprintf(message, "%x", storedRX[RXPointer].data0);
//        UART_TXData(message, strlen(message));
//        sprintf(message, "%x", storedRX[RXPointer].data1);
//        UART_TXData(message, strlen(message));
//        sprintf(message, "%x", storedRX[RXPointer].data2);
//        UART_TXData(message, strlen(message));
//        sprintf(message, "%x", storedRX[RXPointer].data3);
//        UART_TXData(message, strlen(message));
//        sprintf(message, "%x", storedRX[RXPointer].CRC0);
//        UART_TXData(message, strlen(message));
//        sprintf(message, "%x", storedRX[RXPointer].CRC1);
//        UART_TXData(message, strlen(message));
//        sprintf(message, "%x--", storedRX[RXPointer].timeStamp);
//        UART_TXData(message, strlen(message));

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

        }

        currentReceived = 0;
    }
    else
    {
        sprintf(message, "RECERR ");
        UART_TXData(message, strlen(message));
//        producedData("111110011111001111111111111100111");
        currentReceived = 0;
    }
    dataStatus = DATA_WAIT;
    energyLevel = energyLevel - ENERGY_CONSUMED_RX;

}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void interruptEnergy(void)
{

//    sprintf(message, "DSI %d ", dataStatus);
//    UART_TXData(message, strlen(message));

    if (dataStatus == DATA_WAIT)
    {
        int energy_step = rand() % (energy_increment + 1);
        energyLevel = energyLevel + energy_step;
        if (energyLevel >= MAX_ENERGY)
            energyLevel = MAX_ENERGY;
        energy_count++;

        energy_step = 120 + energy_step / 20;

        if (energy_count >= energy_count_limit)
        {
            energy_count_limit = (ENERGY_CHANGE / 2)
                    + (rand() % (ENERGY_CHANGE / 2 + 1));
            energy_increment = rand() % (ENERGY_INCREMENT + 1);
            energy_count = 0;

        }
//        sprintf(message, "UE ");
//        UART_TXData(message, strlen(message));
    }

    if (dataStatus == DATA_RX)
    {
        TA0CCR0 = 0;
        checkRXData();
        dataStatus = DATA_WAIT;
        TA0CCR0 = ENERGY_UPDATE_RATE;

    }

}

/////////////////////////////////
#pragma vector = PORT5_VECTOR
__interrupt void P5_ISR(void)
{

    if (GPIO_getInterruptStatus(GPIO_PORT_P5, GPIO_PIN6))
    {
        producedData("111110011111001111111111111100111");
        GPIO_clearInterrupt(GPIO_PORT_P5, GPIO_PIN6);

    }
}
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
            sprintf(message, "SEND ");
            dataStatus = DATA_TX;
            UART_TXData(message, strlen(message));
            UART_TXDataTOBOARD(storedTX[sendPointer].nodeNumber);
            UART_TXDataTOBOARD(storedTX[sendPointer].data0);
            UART_TXDataTOBOARD(storedTX[sendPointer].data1);
            UART_TXDataTOBOARD(storedTX[sendPointer].data2);
            UART_TXDataTOBOARD(storedTX[sendPointer].data3);
            UART_TXDataTOBOARD(storedTX[sendPointer].CRC0);
            UART_TXDataTOBOARD(storedTX[sendPointer].CRC1);
            UART_TXDataTOBOARD(storedTX[sendPointer].timeStamp);
            storedTX[sendPointer].saved = 0x00;
            energyLevel = energyLevel - ENERGY_CONSUMED_TX;
            sendPointer = sendPointer + 0x01;
            dataStatus = DATA_WAIT;
        }
    }

}

#pragma vector = USCI_A1_VECTOR //IS EUSCI BUT USCI WORKS ANYWAY FOR COMPATIBILITY WITH OLD BOARDS
__interrupt void UARTA1(void)
{

    if (((energyLevel == ENERGY_CONSUMED_RX)
            || (energyLevel > ENERGY_CONSUMED_RX)) && dataStatus != DATA_TX)
    {
//        sprintf(message, "ISR ");
//        UART_TXData(message, strlen(message));
        dataStatus = DATA_RX;
        TA0CCR0 = 0;

        switch (currentReceived)
        {
        case 0:
            storedRX[RXPointer].nodeNumber = UCA1RXBUF;
            break;
        case 1:
            storedRX[RXPointer].data0 = UCA1RXBUF;
            break;
        case 2:
            storedRX[RXPointer].data1 = UCA1RXBUF;
            break;
        case 3:
            storedRX[RXPointer].data2 = UCA1RXBUF;
            break;
        case 4:
            storedRX[RXPointer].data3 = UCA1RXBUF;
            break;
        case 5:
            storedRX[RXPointer].CRC0 = UCA1RXBUF;
            break;
        case 6:
            storedRX[RXPointer].CRC1 = UCA1RXBUF;
            break;
        case 7:
            storedRX[RXPointer].timeStamp = UCA1RXBUF;
            break;
        default:
            break;
        }
        currentReceived++;

        TA0CCR0 = 250;
    }
    else
    {
//        sprintf(message, "ERDR "); //Error data reception
//        UART_TXData(message, strlen(message));

    }

}

void producedData(char *data)
{
//simulate a circular buffer, old data will be overwrite after FRAM_TX_NUMBER writes (suppose to be too old)
    if (writePointer >= FRAM_TX_NUMBER)
    {
        writePointer = 0x00;
    }

    unsigned char data0[9];
    unsigned char data1[9];
    unsigned char data2[9];
    unsigned char data3[9];
    strncpy((char*) data0, (char*) data, 8);
    data0[8] = '\0';
    strncpy((char*) data1, (char*) (data + 8), 8);
    data1[8] = '\0';
    strncpy((char*) data2, (char*) (data + 16), 8);
    data2[8] = '\0';
    strncpy((char*) data3, (char*) (data + 24), 8);
    data3[8] = '\0';

    unsigned char data0TX = strtol((char*) data0, NULL, 2);
    unsigned char data1TX = strtol((char*) data1, NULL, 2);
    unsigned char data2TX = strtol((char*) data2, NULL, 2);
    unsigned char data3TX = strtol((char*) data3, NULL, 2);
    storedTX[writePointer].data0 = data0TX;
    storedTX[writePointer].data1 = data1TX;
    storedTX[writePointer].data2 = data2TX;
    storedTX[writePointer].data3 = data3TX;

    data0TX &= BIT_MASK_0;
    data1TX &= BIT_MASK_1;
    data2TX &= BIT_MASK_2;
    data3TX &= BIT_MASK_3;

    data0TX |= data1TX;
    data2TX |= data3TX;

    data0TX &= data2TX;

    CRC_setSeed(CRC_BASE, CRC_SEED);
    CRC_set16BitData(CRC_BASE, data0TX);
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
    if (readPointer >= FRAM_RX_NUMBER)
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
    storedData res = storedRX[readPointer];
    storedRX[readPointer].saved = 0x00;
    readPointer = readPointer + 0x01;
    return res;

}

