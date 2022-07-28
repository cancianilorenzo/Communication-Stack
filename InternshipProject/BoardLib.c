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



//-----------------------------------
//memset(dataStored.data, 0, sizeof(dataStored.data));
//dataStored.state = '0';
//------------------------------------------



//int x = 1;

//Energy simulation
int energyLevel = 0;
int energy_count = 0;
int energy_count_limit = 0;
int energy_increment = 0;


//Data Status
int dataStatus = 0;

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
    //Timer A0_0 ---- ENERGY UPDATE ----
    TA0CCTL0 = CCIE; // enable capture control interupt
    TA0CTL = TASSEL_1 + MC_1 + ID_0;  // Use ACLK in up mode, /1 divider
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
    //BURST RX
    P1IES = BIT4;  // set interrupt on edge select
    P1IFG = 0;              // clear interrupt flags
    P1IE = BIT4;  // set interupt enable on pins

    //DATA RX
    P3IES = (BIT0 | BIT1 | BIT2);  // set interrupt on edge select
    P3IFG = 0;              // clear interrupt flags
    P3IE = (BIT0 | BIT1 | BIT2);  // set interupt enable on pins

    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN2); //Pin real Data send
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN5); //Pin real Data send
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN3); //Pin real Burst send
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0); //Pin notify Data send
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN1); //Pin notify Burst send
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN1);

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

#pragma vector = TIMER0_A0_VECTOR
__interrupt void interruptEnergy(void)
{

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
    }

}

/*ADDED FOR DATA-RX HANDLER*/
#define DATA_SIZE 16
char data0[DATA_SIZE]; //16 bytes --> only 1 data stored
char data1[DATA_SIZE]; //16 bytes --> only 1 data stored
int currentDataBuffer = 0;
unsigned int canStore0 = 0;
unsigned int canStore1 = 0;
/*END ADDED FOR...*/

#pragma vector = PORT3_VECTOR
__interrupt void P3_ISR(void)
{

// Data RX ISR
    if (P3IFG & BIT1)
    {

        if (((energyLevel == ENERGY_CONSUMED_RX)
                || (energyLevel > ENERGY_CONSUMED_RX))
                && dataStatus == DATA_WAIT)
        {
            sprintf(message, "REC31 "); //Error data reception
            UART_TXData(message, strlen(message));
//            dataStatus = DATA_RX;
//
//            if (canStore0 == 0)
//            {
//                dataStatus = DATA_WAIT;
//                if (currentDataBuffer < DATA_SIZE + 1)
//                {
//                    //a raising edge cannto happen if the was no falling edge before!!!!
//                    data0[currentDataBuffer] = '0';
//                    currentDataBuffer++;
//                    data0[currentDataBuffer] = '1';
//                    currentDataBuffer++;
//                    if (currentDataBuffer > DATA_SIZE)
//                    {
//                        canStore0 = 1;
//                        energyLevel = energyLevel - ENERGY_CONSUMED_RX;
//                        currentDataBuffer = 0;
//                    }
//                }
//
//            }

        }
        else
        {
            sprintf(message, "ER31 "); //Error data reception
            UART_TXData(message, strlen(message));
        }

        P3IFG &= ~BIT1;
    }

    if (P3IFG & BIT2)
    {

        if (((energyLevel == ENERGY_CONSUMED_RX)
                || (energyLevel > ENERGY_CONSUMED_RX))
                && dataStatus == DATA_WAIT)
        {
//            dataStatus = DATA_RX;
//
//            if (canStore1 == 0)
//            {
//                dataStatus = DATA_WAIT;
//                if (currentDataBuffer < DATA_SIZE + 1)
//                {
//
//                    //a raising edge cannto happen if the was no falling edge before!!!!
//                    data1[currentDataBuffer] = '0';
//                    currentDataBuffer++;
//                    data1[currentDataBuffer] = '1';
//                    currentDataBuffer++;
//                    if (currentDataBuffer > DATA_SIZE)
//                    {
//                        canStore1 = 1;
//                        energyLevel = energyLevel - ENERGY_CONSUMED_RX;
//                        currentDataBuffer = 0;
//                    }
//                }
//
//            }

        }
        else
        {
            sprintf(message, "0ER32 "); //Error data reception
            UART_TXData(message, strlen(message));
        }

        P3IFG &= ~BIT2;
    }

    dataStatus = DATA_WAIT;

}

//typedef struct DATA
//{
//    char data[256];
//    char state; //Char to have atomicity --> CPU operate on 1byte x time
//} DATA;
////FRAM compiler instructions
//#if defined(__TI_COMPILER_VERSION__)
//#pragma PERSISTENT(dataStored)
//DATA dataStored = { .data = "e", .state = 'e' };
//#else
//#error Compiler not supported!
//#endif

void FRAMWrite(char *data, int pos)
{

    int i;

    store = '0';
    for (i = 0; i < strlen(data); i++)
    {
        dataStore[i] = data[i];
    }
    store = '1'; //Succesfully stored in FRAM;
}


void dataSend12(char *messageToSend)
{

    int len = strlen(messageToSend);
    int i = 0;
    for (i = 0; i < len; i++)
    {
        dataStatus = DATA_TX;
        if (messageToSend[i] == '1')
        {
            GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN2);
            __delay_cycles(75550);
        }
        else
        {
            GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN2);
            __delay_cycles(75550);
        }
    }
    energyLevel = energyLevel - ENERGY_CONSUMED_TX;
    dataStatus = DATA_WAIT;
}


void dataSend15(char *messageToSend)
{

    int len = strlen(messageToSend);
    int i = 0;
    for (i = 0; i < len; i++)
    {
        dataStatus = DATA_TX;
        if (messageToSend[i] == '1')
        {
            GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN5);
            __delay_cycles(75550); //Needed if other board clock is slower
        }
        else
        {
            GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN5);
            __delay_cycles(75550); //Needed if other board clock is slower
        }
    }
    energyLevel = energyLevel - ENERGY_CONSUMED_TX;
    dataStatus = DATA_WAIT;
}

