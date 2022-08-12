#include "TRAP\TRAP.h"
#include "Communication/CommunicationLayer.h"
#include "Board/Board.h"
#include <msp430.h>
#include <stdio.h>
#include <string.h>

#pragma vector = USCI_A1_VECTOR //IS EUSCI BUT USCI WORKS ANYWAY FOR COMPATIBILITY WITH OLD BOARDS
__interrupt void UART_RXDataTOBOARD(void)
{

    if (((energyLevel == ENERGY_CONSUMED_RX)
            || (energyLevel > ENERGY_CONSUMED_RX)) && dataStatus != DATA_TX)
    {
//        sprintf(message, "ISR ");
//        UART_TXData(message, strlen(message));
        dataStatus = DATA_RX;
        TA0CCR0 = 0;
        TA0CCR0 = 250;

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

    }
    else
    {
        sprintf(message, "ERDR "); //Error data reception
        UART_TXData(message, strlen(message));

    }

}


void UART_TXDataTOBOARD(unsigned char c)
{
    unsigned int j;
    UCA1TXBUF = (unsigned char) c;
    for (j = 0; j < 30000; j++)
        ;

}


void startPhysicalLayer(){
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
