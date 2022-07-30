#include "BoardLib.h"
#include <msp430.h>
#include "driverlib.h"
#include <stdlib.h>

/*FOR DEBUG*/
#include <string.h>
#include <stdio.h>

char message[MSG_SIZE];

char dataStore[1000] = "c";
char store = 'c';

//DATA_TX
int TX = -1;
char auxString[RX_SIZE] = "0111101101";
int currentSend = 0;

//-----------------------------------
//memset(dataStored.data, 0, sizeof(dataStored.data));
//dataStored.state = '0';
//------------------------------------------

int received = 0;
int RX = -1;
char dataRec[RX_SIZE];
int dataStatus;

int currentDataBuffer = 0;
int alreadyRec = 0;

//int x = 1;

//Energy simulation
int energyLevel = 0;
int energy_count = 0;
int energy_count_limit = 0;
int energy_increment = 0;

void initBoard()
{
    //MPU initialization, useful if operations couldbe risky --> not in CCS, already managed
    /* MPUCTL0 = MPUPW;
     MPUSEGB2 = 0x1000; // memory address 0x10000
     MPUSEGB1 = 0x0fc0; // memory address 0x0fc00
     MPUSAM &= ~MPUSEG2WE; // disallow writes
     MPUSAM |= MPUSEG2VS;  // reset CPU on violation
     MPUCTL0 = MPUPW | MPUENA;
     MPUCTL0_H = 0;*/

    WDTCTL = WDTPW | WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5; // Disable the GPIO power-on default high-impedance mode
    __enable_interrupt(); // enable global interrupts

}

void UARTInit()
{
    P2SEL0 &= ~(BIT0 | BIT1);
    P2SEL1 |= (BIT0 | BIT1);                           // USCI_A3 UART operation

    UCA0CTLW0 = UCSWRST;                                   // Put eUSCI in reset
    UCA0CTLW0 |= UCSSEL__SMCLK;                    // CLK = SMCLK
    UCA0BRW = 8;
    UCA0MCTLW = 0xD600;
    UCA0CTLW0 &= ~UCSWRST;                                 // Initialize eUSCI

    UCA0IE |= UCRXIE;

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
    //RX
#ifdef DATA_RX_PORT_0
    //node0
    GPIO_setAsInputPinWithPullUpResistor(DATA_RX_PORT_0, DATA_RX_PIN_0);
    GPIO_selectInterruptEdge(DATA_RX_PORT_0, DATA_RX_PIN_0,
    GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_clearInterrupt(DATA_RX_PORT_0, DATA_RX_PIN_0);
    GPIO_enableInterrupt(DATA_RX_PORT_0, DATA_RX_PIN_0);
#endif

#ifdef DATA_RX_PORT_1
    //node1
    GPIO_setAsInputPinWithPullUpResistor(DATA_RX_PORT_1, DATA_RX_PIN_1);
    GPIO_selectInterruptEdge(DATA_RX_PORT_1, DATA_RX_PIN_1,
    GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_clearInterrupt(DATA_RX_PORT_1, DATA_RX_PIN_1);
    GPIO_enableInterrupt(DATA_RX_PORT_1, DATA_RX_PIN_1);
#endif

#ifdef DATA_RX_PORT_2
    //node2
    GPIO_setAsInputPinWithPullUpResistor(DATA_RX_PORT_2, DATA_RX_PIN_2);
    GPIO_selectInterruptEdge(DATA_RX_PORT_2, DATA_RX_PIN_2,
    GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_clearInterrupt(DATA_RX_PORT_2, DATA_RX_PIN_2);
    GPIO_enableInterrupt(DATA_RX_PORT_2, DATA_RX_PIN_2);
    #endif

#ifdef DATA_RX_PORT_3
    //node3
    GPIO_setAsInputPinWithPullUpResistor(DATA_RX_PORT_3, DATA_RX_PIN_3);
    GPIO_selectInterruptEdge(DATA_RX_PORT_3, DATA_RX_PIN_3,
    GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_clearInterrupt(DATA_RX_PORT_3, DATA_RX_PIN_3);
    GPIO_enableInterrupt(DATA_RX_PORT_3, DATA_RX_PIN_3);
    #endif

#ifdef DATA_RX_PORT_4
    //node4
    GPIO_setAsInputPinWithPullUpResistor(DATA_RX_PORT_4, DATA_RX_PIN_4);
    GPIO_selectInterruptEdge(DATA_RX_PORT_4, DATA_RX_PIN_4,
    GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_clearInterrupt(DATA_RX_PORT_4, DATA_RX_PIN_4);
    GPIO_enableInterrupt(DATA_RX_PORT_4, DATA_RX_PIN_4);
    #endif

#ifdef DATA_RX_PORT_5
    //node5
    GPIO_setAsInputPinWithPullUpResistor(DATA_RX_PORT_5, DATA_RX_PIN_5);
    GPIO_selectInterruptEdge(DATA_RX_PORT_5, DATA_RX_PIN_5,
    GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_clearInterrupt(DATA_RX_PORT_5, DATA_RX_PIN_5);
    GPIO_enableInterrupt(DATA_RX_PORT_1, DATA_RX_PIN_1);
    #endif

#ifdef DATA_RX_PORT_6
    //node6
    GPIO_setAsInputPinWithPullUpResistor(DATA_RX_PORT_6, DATA_RX_PIN_6);
    GPIO_selectInterruptEdge(DATA_RX_PORT_6, DATA_RX_PIN_6,
    GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_clearInterrupt(DATA_RX_PORT_6, DATA_RX_PIN_6);
    GPIO_enableInterrupt(DATA_RX_PORT_6, DATA_RX_PIN_6);
    #endif

#ifdef DATA_RX_PORT_7
    //node7
    GPIO_setAsInputPinWithPullUpResistor(DATA_RX_PORT_7, DATA_RX_PIN_7);
    GPIO_selectInterruptEdge(DATA_RX_PORT_7, DATA_RX_PIN_7,
    GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_clearInterrupt(DATA_RX_PORT_7, DATA_RX_PIN_7);
    GPIO_enableInterrupt(DATA_RX_PORT_7, DATA_RX_PIN_7);
    #endif

    //TX
#ifdef DATA_TX_PORT_0
    GPIO_setAsOutputPin(DATA_TX_PORT_0, DATA_TX_PIN_0);
#endif

#ifdef DATA_RX_PORT_1
    GPIO_setAsOutputPin(DATA_TX_PORT_1, DATA_TX_PIN_1);
#endif

#ifdef DATA_RX_PORT_7
    GPIO_setAsOutputPin(DATA_TX_PORT_2, DATA_TX_PIN_2);
    #endif

#ifdef DATA_RX_PORT_7
    GPIO_setAsOutputPin(DATA_TX_PORT_3, DATA_TX_PIN_3);
    #endif

#ifdef DATA_RX_PORT_7
    GPIO_setAsOutputPin(DATA_TX_PORT_4, DATA_TX_PIN_4);
    #endif

#ifdef DATA_RX_PORT_7
    GPIO_setAsOutputPin(DATA_TX_PORT_5, DATA_TX_PIN_5);
    #endif

#ifdef DATA_RX_PORT_7
    GPIO_setAsOutputPin(DATA_TX_PORT_6, DATA_TX_PIN_6);
    #endif

#ifdef DATA_RX_PORT_7
    GPIO_setAsOutputPin(DATA_TX_PORT_7, DATA_TX_PIN_7);
    #endif

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

void startEnergyTimer(int value)
{
    TA0CCR0 = value;
}

void startEnergySimulation()
{
    startEnergyTimer(125);
    energy_count_limit = (ENERGY_CHANGE / 2)
            + (rand() % (ENERGY_CHANGE / 2 + 1));
    energy_increment = rand() % (ENERGY_INCREMENT + 1);
}

int readPin(int number)
{
    int value = 0;
#ifdef DATA_RX_PIN_0
    if (number == 0)
    {
        if (GPIO_INPUT_PIN_HIGH
                == GPIO_getInputPinValue(DATA_RX_PORT_0, DATA_RX_PIN_0))
        {
            value = 1;
        }
    }
#endif

#ifdef DATA_RX_PIN_1
    if (number == 1)
    {

        if (GPIO_INPUT_PIN_HIGH
                == GPIO_getInputPinValue(DATA_RX_PORT_1, DATA_RX_PIN_1))
        {
            value = 1;
        }
    }
#endif

#ifdef DATA_RX_PIN_2
    if (number == 2)
    {

        if (GPIO_INPUT_PIN_HIGH
                == GPIO_getInputPinValue(DATA_RX_PORT_2, DATA_RX_PIN_2))
        {
            value = 1;
        }
    }
#endif

#ifdef DATA_RX_PIN_3
    if (number == 3)
    {

        if (GPIO_INPUT_PIN_HIGH
                == GPIO_getInputPinValue(DATA_RX_PORT_3, DATA_RX_PIN_3))
        {
            value = 1;
        }
    }
#endif

#ifdef DATA_RX_PIN_4
    if (number == 4)
    {

        if (GPIO_INPUT_PIN_HIGH
                == GPIO_getInputPinValue(DATA_RX_PORT_4, DATA_RX_PIN_4))
        {
            value = 1;
        }
    }
#endif

#ifdef DATA_RX_PIN_5
    if (number == 5)
    {

        if (GPIO_INPUT_PIN_HIGH
                == GPIO_getInputPinValue(DATA_RX_PORT_5, DATA_RX_PIN_5))
        {
            value = 1;
        }
    }
#endif

#ifdef DATA_RX_PIN_6
    if (number == 6)
    {

        if (GPIO_INPUT_PIN_HIGH
                == GPIO_getInputPinValue(DATA_RX_PORT_6, DATA_RX_PIN_6))
        {
            value = 1;
        }
    }
#endif

#ifdef DATA_RX_PIN_7
    if (number == 7)
    {

        if (GPIO_INPUT_PIN_HIGH
                == GPIO_getInputPinValue(DATA_RX_PORT_7, DATA_RX_PIN_7))
        {
            value = 1;
        }
    }
#endif

    return value;
}

void interruptON(int number)
{

    //Edit with SWITCH case
#ifdef DATA_RX_PIN_0
    if (number == 0)
    {

        GPIO_enableInterrupt(DATA_RX_PORT_0, DATA_RX_PIN_0);
    }
#endif

#ifdef DATA_RX_PIN_1
    if (number == 1)
    {

        GPIO_enableInterrupt(DATA_RX_PORT_1, DATA_RX_PIN_1);
    }
#endif

#ifdef DATA_RX_PIN_2
    if (number == 2)
    {

        GPIO_enableInterrupt(DATA_RX_PORT_2, DATA_RX_PIN_2);
    }
#endif

#ifdef DATA_RX_PIN_3
    if (number == 3)
    {

        GPIO_enableInterrupt(DATA_RX_PORT_3, DATA_RX_PIN_3);
    }
#endif

#ifdef DATA_RX_PIN_4
    if (number == 4)
    {

        GPIO_enableInterrupt(DATA_RX_PORT_4, DATA_RX_PIN_4);
    }
#endif

#ifdef DATA_RX_PIN_5
    if (number == 5)
    {

        GPIO_enableInterrupt(DATA_RX_PORT_5, DATA_RX_PIN_5);
    }
#endif

#ifdef DATA_RX_PIN_6
    if (number == 6)
    {

        GPIO_enableInterrupt(DATA_RX_PORT_6, DATA_RX_PIN_6);
    }
#endif

#ifdef DATA_RX_PIN_7
    if (number == 7)
    {

        GPIO_enableInterrupt(DATA_RX_PORT_7, DATA_RX_PIN_7);
    }
#endif
    received = 0;
    TA0CCR0 = 125;
    RX = -1;
    dataStatus = DATA_WAIT;
}

void PinHigh(int number)
{
#ifdef DATA_TX_PIN_0
    if (number == 0)
    {
        GPIO_setOutputHighOnPin(DATA_TX_PORT_0, DATA_TX_PIN_0);
    }
#endif

#ifdef DATA_TX_PIN_1
    if (number == 1)
    {
        GPIO_setOutputHighOnPin(DATA_TX_PORT_1, DATA_TX_PIN_1);
    }
#endif

#ifdef DATA_TX_PIN_2
    if(number == 2){
        GPIO_setOutputHighOnPin(DATA_TX_PORT_2, DATA_TX_PIN_2);
        }
#endif

#ifdef DATA_TX_PIN_3
    if(number == 3){
        GPIO_setOutputHighOnPin(DATA_TX_PORT_3, DATA_TX_PIN_3);
        }
#endif

#ifdef DATA_TX_PIN_4
    if(number == 4){
        GPIO_setOutputHighOnPin(DATA_TX_PORT_4, DATA_TX_PIN_4);
        }
#endif

#ifdef DATA_TX_PIN_5
    if(number == 5){
        GPIO_setOutputHighOnPin(DATA_TX_PORT_5, DATA_TX_PIN_5);
        }
#endif

#ifdef DATA_TX_PIN_6
    if(number == 6){
        GPIO_setOutputHighOnPin(DATA_TX_PORT_6, DATA_TX_PIN_6);
        }
#endif

#ifdef DATA_TX_PIN_7
    if(number == 7){
        GPIO_setOutputHighOnPin(DATA_TX_PORT_7, DATA_TX_PIN_7);
        }
#endif

}

void PinLow(int number)
{
#ifdef DATA_TX_PIN_0
    if (number == 0)
    {
        GPIO_setOutputLowOnPin(DATA_TX_PORT_0, DATA_TX_PIN_0);
    }
#endif

#ifdef DATA_TX_PIN_1
    if (number == 1)
    {
        GPIO_setOutputLowOnPin(DATA_TX_PORT_1, DATA_TX_PIN_1);
    }
#endif

#ifdef DATA_TX_PIN_2
    if(number == 2){
            GPIO_setOutputLowOnPin(DATA_TX_PORT_2, DATA_TX_PIN_2);
        }
#endif

#ifdef DATA_TX_PIN_3
    if(number == 3){
            GPIO_setOutputLowOnPin(DATA_TX_PORT_3, DATA_TX_PIN_3);
        }
#endif

#ifdef DATA_TX_PIN_4
    if(number == 4){
            GPIO_setOutputLowOnPin(DATA_TX_PORT_4, DATA_TX_PIN_4);
        }
#endif

#ifdef DATA_TX_PIN_5
    if(number == 5){
            GPIO_setOutputLowOnPin(DATA_TX_PORT_5, DATA_TX_PIN_5);
        }
#endif

#ifdef DATA_TX_PIN_6
    if(number == 6){
            GPIO_setOutputLowOnPin(DATA_TX_PORT_6, DATA_TX_PIN_6);
        }
#endif

#ifdef DATA_TX_PIN_7
    if(number == 7){
            GPIO_setOutputLowOnPin(DATA_TX_PORT_7, DATA_TX_PIN_7);
        }
#endif

}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void interruptEnergy(void)
{

//    sprintf(message, "DSI %d ", dataStatus);
//    UART_TXData(message, strlen(message));

    if (dataStatus == DATA_WAIT && RX == -1)
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

    //-------------RX----------------------
    if (dataStatus == DATA_RX && RX != -1)
    {
        if (received != RX_SIZE)
        {
            if (readPin(RX))
            {
                dataRec[received] = '1';
//                sprintf(message, "1 ");
//                UART_TXData(message, strlen(message));
            }
            else
            {
                dataRec[received] = '0';
//                sprintf(message, "0 ");
//                UART_TXData(message, strlen(message));
            }

            received++;

        }
        else
        {
            sprintf(message, "REC%d ", RX);
            UART_TXData(message, strlen(message));
//            int i;
//            for (i = 0; i < RX_SIZE; i++)
//            {
//                sprintf(message, "%c ", dataRec[i]);
//                UART_TXData(message, strlen(message));
//            }
//            sprintf(message, "-- ");
//            UART_TXData(message, strlen(message));
            interruptON(RX);
            dataStatus = DATA_WAIT;

        }
    }

    //-------------TX----------------------
        if (dataStatus == DATA_TX && TX != -1)
        {
            if (currentSend != strlen(auxString))
            {
                if (auxString[currentSend] == '1')
                {
                    PinHigh(TX);
                    //GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN2);
//                    sprintf(message, "1 ");
//                    UART_TXData(message, strlen(message));
                }
                else
                {
                    PinLow(TX);
    //                GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN2);
//                    sprintf(message, "0 ");
//                    UART_TXData(message, strlen(message));
                }
                currentSend++;
            }
            else
            {
                sprintf(message, "SEND ");
                UART_TXData(message, strlen(message));
                dataStatus = DATA_WAIT;
                currentSend = 0;
                TX = -1;
                TA0CCR0 = 0;
                TA0CCR0 = 125;
            }
        }

}

//------------------------------------------------------------ DATA RX

void interruptOFF(int number)
{
    dataStatus = DATA_RX;
#ifdef DATA_RX_PORT_0
    if (number == 0)
    {
        GPIO_disableInterrupt(DATA_RX_PORT_0, DATA_RX_PIN_0);
    }
#endif

#ifdef DATA_RX_PORT_1
    if (number == 1)
    {
        GPIO_disableInterrupt(DATA_RX_PORT_1, DATA_RX_PIN_1);
    }
#endif

#ifdef DATA_RX_PORT_2
    if (number == 2)
    {
        GPIO_disableInterrupt(DATA_RX_PORT_2, DATA_RX_PIN_2);
    }
    #endif

#ifdef DATA_RX_PORT_3
    if (number == 3)
    {
        GPIO_disableInterrupt(DATA_RX_PORT_3, DATA_RX_PIN_3);
    }
    #endif

#ifdef DATA_RX_PORT_4
    if (number == 4)
    {
        GPIO_disableInterrupt(DATA_RX_PORT_4, DATA_RX_PIN_4);
    }
    #endif

#ifdef DATA_RX_PORT_5
    if (number == 5)
    {
        GPIO_disableInterrupt(DATA_RX_PORT_5, DATA_RX_PIN_5);
    }
    #endif

#ifdef DATA_RX_PORT_6
    if (number == 6)
    {
        GPIO_disableInterrupt(DATA_RX_PORT_6, DATA_RX_PIN_6);
    }
    #endif

#ifdef DATA_RX_PORT_7
    if (number == 7)
    {
        GPIO_disableInterrupt(DATA_RX_PORT_7, DATA_RX_PIN_7);
    }
    #endif

    energyLevel = energyLevel - ENERGY_CONSUMED_RX;
    TA0CCR0 = 0;
    TA0CCR0 = 1;
}

#pragma vector = DATA_RX_VECTOR
__interrupt void P3_ISR(void)
{

// Data RX ISR
#ifdef DATA_RX_PORT_0
//    if (P3IFG & BIT0)
    if (GPIO_getInterruptStatus(DATA_RX_PORT_0, DATA_RX_PIN_0))

    {

        if (((energyLevel == ENERGY_CONSUMED_RX)
                || (energyLevel > ENERGY_CONSUMED_RX))
                && dataStatus == DATA_WAIT)
        {
            if (alreadyRec == 0)
            {
                if (RX == -1)
                {
                    RX = 0;
                    alreadyRec = 1;
                    interruptOFF(RX);
                }
            }
            else
            {
                alreadyRec = 0;
            }

        }
        else
        {
            sprintf(message, "ER30 "); //Error data reception
            UART_TXData(message, strlen(message));
        }

//        P3IFG &= ~BIT0;
        GPIO_clearInterrupt(DATA_RX_PORT_0, DATA_RX_PIN_0);

    }
#endif

#ifdef DATA_RX_PORT_1
//    if (P3IFG & BIT1)
    if (GPIO_getInterruptStatus(DATA_RX_PORT_1, DATA_RX_PIN_1))

    {

        if (((energyLevel == ENERGY_CONSUMED_RX)
                || (energyLevel > ENERGY_CONSUMED_RX))
                && dataStatus == DATA_WAIT)
        {
            if (alreadyRec == 0)
            {
                if (RX == -1)
                {
                    RX = 1;
                    alreadyRec = 1;
                    interruptOFF(RX);
                }
            }
            else
            {
                alreadyRec = 0;
            }

        }
        else
        {
            sprintf(message, "ER31 "); //Error data reception
            UART_TXData(message, strlen(message));
        }

//        P3IFG &= ~BIT1;
        GPIO_clearInterrupt(DATA_RX_PORT_1, DATA_RX_PIN_1);

    }
#endif

#ifdef DATA_RX_PORT_2
//    if (P3IFG & BIT2)
    if (GPIO_getInterruptStatus(DATA_RX_PORT_2, DATA_RX_PIN_2))

    {

        if (((energyLevel == ENERGY_CONSUMED_RX)
                || (energyLevel > ENERGY_CONSUMED_RX))
                && dataStatus == DATA_WAIT)
        {
            if (alreadyRec == 0)
            {
                if (RX == -1)
                {
                    RX = 2;
                    alreadyRec = 1;
                    interruptOFF(RX);
                }
            }
            else
            {
                alreadyRec = 0;
            }

        }
        else
        {
            sprintf(message, "ER32 "); //Error data reception
            UART_TXData(message, strlen(message));
        }

//        P3IFG &= ~BIT2;
        GPIO_clearInterrupt(DATA_RX_PORT_2, DATA_RX_PIN_2);

    }
#endif

#ifdef DATA_RX_PORT_3
//    if (P3IFG & BIT3)
    if (GPIO_getInterruptStatus(DATA_RX_PORT_3, DATA_RX_PIN_3))

    {

        if (((energyLevel == ENERGY_CONSUMED_RX)
                || (energyLevel > ENERGY_CONSUMED_RX))
                && dataStatus == DATA_WAIT)
        {
            if (alreadyRec == 0)
            {
                if (RX == -1)
                {
                    RX = 3;
                    alreadyRec = 1;
                    interruptOFF(RX);
                }
            }
            else
            {
                alreadyRec = 0;
            }

        }
        else
        {
            sprintf(message, "ER33 "); //Error data reception
            UART_TXData(message, strlen(message));
        }

//        P3IFG &= ~BIT3;
        GPIO_clearInterrupt(DATA_RX_PORT_3, DATA_RX_PIN_3);

    }
#endif

#ifdef DATA_RX_PORT_4
//    if (P3IFG & BIT4)
    if (GPIO_getInterruptStatus(DATA_RX_PORT_4, DATA_RX_PIN_4))

    {

        if (((energyLevel == ENERGY_CONSUMED_RX)
                || (energyLevel > ENERGY_CONSUMED_RX))
                && dataStatus == DATA_WAIT)
        {
            if (alreadyRec == 0)
            {
                if (RX == -1)
                {
                    RX = 4;
                    alreadyRec = 1;
                    interruptOFF(RX);
                }
            }
            else
            {
                alreadyRec = 0;
            }

        }
        else
        {
            sprintf(message, "ER34 "); //Error data reception
            UART_TXData(message, strlen(message));
        }

//        P3IFG &= ~BIT4;
        GPIO_clearInterrupt(DATA_RX_PORT_4, DATA_RX_PIN_4);
    }
#endif

#ifdef DATA_RX_PORT_5
//    if (P3IFG & BIT5)
    if (GPIO_getInterruptStatus(DATA_RX_PORT_5, DATA_RX_PIN_5))

    {

        if (((energyLevel == ENERGY_CONSUMED_RX)
                || (energyLevel > ENERGY_CONSUMED_RX))
                && dataStatus == DATA_WAIT)
        {
            if (alreadyRec == 0)
            {
                if (RX == -1)
                {
                    RX = 5;
                    alreadyRec = 1;
                    interruptOFF(RX);
                }
            }
            else
            {
                alreadyRec = 0;
            }

        }
        else
        {
            sprintf(message, "ER35 "); //Error data reception
            UART_TXData(message, strlen(message));
        }

//        P3IFG &= ~BIT5;
        GPIO_clearInterrupt(DATA_RX_PORT_5, DATA_RX_PIN_5);
    }
#endif

#ifdef DATA_RX_PORT_6
//    if (P3IFG & BIT6)
    if (GPIO_getInterruptStatus(DATA_RX_PORT_6, DATA_RX_PIN_6))

    {

        if (((energyLevel == ENERGY_CONSUMED_RX)
                || (energyLevel > ENERGY_CONSUMED_RX))
                && dataStatus == DATA_WAIT)
        {
            if (alreadyRec == 0)
            {
                if (RX == -1)
                {
                    RX = 6;
                    alreadyRec = 1;
                    interruptOFF(RX);
                }
            }
            else
            {
                alreadyRec = 0;
            }

        }
        else
        {
            sprintf(message, "ER36 "); //Error data reception
            UART_TXData(message, strlen(message));
        }

//        P3IFG &= ~BIT6;
        GPIO_clearInterrupt(DATA_RX_PORT_6, DATA_RX_PIN_6);

    }
#endif

#ifdef DATA_RX_PORT_7
//    if (P3IFG & BIT7)
    if (GPIO_getInterruptStatus(DATA_RX_PORT_7, DATA_RX_PIN_7))
    {

        if (((energyLevel == ENERGY_CONSUMED_RX)
                || (energyLevel > ENERGY_CONSUMED_RX))
                && dataStatus == DATA_WAIT)
        {
            if (alreadyRec == 0)
            {
                if (RX == -1)
                {
                    RX = 7;
                    alreadyRec = 1;
                    interruptOFF(RX);
                }
            }
            else
            {
                alreadyRec = 0;
            }

        }
        else
        {
            sprintf(message, "ER37 "); //Error data reception
            UART_TXData(message, strlen(message));
        }

//        P3IFG &= ~BIT7;
        GPIO_clearInterrupt(DATA_RX_PORT_7, DATA_RX_PIN_7);

    }
#endif
}
//------------------------------------------------------------------------DA VEDERE DOPO
//void FRAMWrite(char *data, int pos)
//{
//
//int i;
//
//store = '0';
//for (i = 0; i < strlen(data); i++)
//{
//    dataStore[i] = data[i];
//}
//store = '1'; //Succesfully stored in FRAM;
//}
//


void dataSend(char *messageToSend, int numberPort)
{
    if (TX == -1)
    {
        int i = 0;
        for (i = 0; i < strlen(messageToSend); i++)
        {
            auxString[i] = messageToSend[i];
        }
        TX = numberPort;
        energyLevel = energyLevel - ENERGY_CONSUMED_TX;
        dataStatus = DATA_TX;
        TA0CCR0 = 0;
        TA0CCR0 = 1;
    }
}

