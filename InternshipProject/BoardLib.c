#include "BoardLib.h"
#include <msp430.h>
#include "driverlib.h"
#include <stdlib.h>
#include <string.h>

/*FOR DEBUG*/
#include <stdio.h>

char message[MSG_SIZE];

char dataStore[1000] = "c";
char store = '0';

int dataStatus = DATA_WAIT;

//DATA_TX
char TX_message[DATA_SIZE + 1];
char RX_message[DATA_SIZE + 1];
char RX_nodeId[8 + 1] = "00000000\0";
char RX_data[16 + 1] = "0000000000000000\0";
char RX_CRC[16 + 1] = "0000000000000000\0";

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
    GPIO_setAsInputPinWithPullUpResistor(DATA_RX_PORT, DATA_RX_PIN);
    GPIO_selectInterruptEdge(DATA_RX_PORT, DATA_RX_PIN,
    GPIO_LOW_TO_HIGH_TRANSITION);
    GPIO_clearInterrupt(DATA_RX_PORT, DATA_RX_PIN);
    GPIO_enableInterrupt(DATA_RX_PORT, DATA_RX_PIN);

    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN5);
    GPIO_selectInterruptEdge(GPIO_PORT_P5, GPIO_PIN5,
    GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_clearInterrupt(GPIO_PORT_P5, GPIO_PIN5);
    GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN5);

    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN6);
    GPIO_selectInterruptEdge(GPIO_PORT_P5, GPIO_PIN6,
    GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_clearInterrupt(GPIO_PORT_P5, GPIO_PIN6);
    GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN6);

    //TX
    GPIO_setAsOutputPin(DATA_TX_PORT, DATA_TX_PIN);
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
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
    startEnergyTimer(ENERGY_UPDATE_RATE);
    energy_count_limit = (ENERGY_CHANGE / 2)
            + (rand() % (ENERGY_CHANGE / 2 + 1));
    energy_increment = rand() % (ENERGY_INCREMENT + 1);
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
        unsigned int aux;
        GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN1);
        sprintf(message, "RX ");
        UART_TXData(message, strlen(message));
        sprintf(RX_message, "0000101110101010110010100110000001111111");
//        unsigned int i;
//        for (i = 0; i < DATA_SIZE; i++)
//        {
//            aux = rand() % 2;
//            if (aux == 1)
//            {
//                RX_message[i] = '1';
//            }
//            else
//            {
//                RX_message[i] = '0';
//            }
//        }

        setID();
        setData();
        setCRC();

//        sprintf(message, "%s ", RX_nodeId);
//        UART_TXData(message, strlen(message));
//
//        sprintf(message, "%s ", RX_data);
//        UART_TXData(message, strlen(message));
//
//        sprintf(message, "%s ", RX_CRC);
//        UART_TXData(message, strlen(message));
        unsigned int CRCResult;
        long toCheck = binaryToInt(RX_data,2);
        CRC_setSeed(CRC_BASE, CRC_SEED);
        CRC_set16BitData(CRC_BASE, toCheck);
        CRCResult = CRC_getResult(CRC_BASE);
        sprintf(message, "%d == %d ", CRCResult,binaryToInt(RX_CRC,2));
        UART_TXData(message, strlen(message));
        if (CRCResult == binaryToInt(RX_CRC,2))
        {
            sprintf(message, "DOK CS DATA: %s ID: %d", RX_data, binaryToInt(RX_nodeId,2));
            UART_TXData(message, strlen(message));
            //saveDATA
        }
        else
        {
            sprintf(message, "DERR ");
            UART_TXData(message, strlen(message));
        }

        TA0CCR0 = 0;
        TA0CCR0 = ENERGY_UPDATE_RATE;
        dataStatus = DATA_WAIT;
    }
}

void setID()
{
    unsigned int RX;
    unsigned int parsed;
    parsed = 0;
    for (RX = 0; RX < 8; RX++)
    {
        RX_nodeId[parsed] = RX_message[RX];
        parsed++;
    }
//    RX_nodeId[parsed] = '\0';
}

void setData()
{
    unsigned int RX;
    unsigned int parsed;
    parsed = 0;
    RX = 8;
    for (RX = 8; RX < 24; RX++)
    {
        RX_data[parsed] = RX_message[RX];
        parsed++;
    }
//    RX_data[parsed] = '\0';
}

void setCRC()
{
    unsigned int RX;
    unsigned int parsed;
    parsed = 0;
    for (RX = 24; RX < 40; RX++)
    {
        RX_CRC[parsed] = RX_message[RX];
        parsed++;
    }
//    RX_CRC[parsed] = '\0';
}

//------------------------------------------------------------ DATA RX
#pragma vector = DATA_RX_VECTOR
__interrupt void P3_ISR(void)
{
// Data RX ISR
//    if (P3IFG & BIT0)
    if (GPIO_getInterruptStatus(DATA_RX_PORT, DATA_RX_PIN))

    {

        if (((energyLevel == ENERGY_CONSUMED_RX)
                || (energyLevel > ENERGY_CONSUMED_RX))
        /*&& dataStatus == DATA_WAIT*/)
        {
            dataStatus = DATA_RX;
            TA0CCR0 = 0;
            TA0CCR0 = 250;

        }
        else
        {
            sprintf(message, "ER30 "); //Error data reception
            UART_TXData(message, strlen(message));
        }

//        P3IFG &= ~BIT0;
        GPIO_clearInterrupt(DATA_RX_PORT, DATA_RX_PIN);

    }
}

///////////////////////////////

#pragma vector = PORT5_VECTOR
__interrupt void P5_ISR(void)
{
// Data RX ISR
//    if (P3IFG & BIT0)
    if (GPIO_getInterruptStatus(GPIO_PORT_P5, GPIO_PIN5))

    {

        if (((energyLevel == ENERGY_CONSUMED_RX)
                || (energyLevel > ENERGY_CONSUMED_RX))
        /*&& dataStatus == DATA_WAIT*/)
        {
            dataStatus = DATA_RX;
            TA0CCR0 = 0;
            TA0CCR0 = 250;

        }
        else
        {
            sprintf(message, "ER30 "); //Error data reception
            UART_TXData(message, strlen(message));
        }

//        P3IFG &= ~BIT0;
        GPIO_clearInterrupt(GPIO_PORT_P5, GPIO_PIN5);

    }

    if (GPIO_getInterruptStatus(GPIO_PORT_P5, GPIO_PIN6))

    {
//                sprintf(message, "%s ", RX_nodeId);
//                UART_TXData(message, strlen(message));
//
//                sprintf(message, "%s ", RX_data);
//                UART_TXData(message, strlen(message));
//
                sprintf(message, "%s ", RX_CRC);
                UART_TXData(message, strlen(message));

        GPIO_clearInterrupt(GPIO_PORT_P5, GPIO_PIN6);

    }
}

//////////////////////////////

int FRAMWrite(char *data)
{
    int res = 0;

    store = '0';
    sprintf(dataStore, data);
    store = '1'; //Succesfully stored in FRAM;
    res = 1;

    return res;
}

int dataToSend()
{
    int res = 0;
    if (store == '1')
    {
        res = 1;
    }
    return res;
}

char* stringToBinary(char *s)
{
    //1byte for each char
    if (s == NULL)
        return 0; /* no input string */
    size_t len = strlen(s);
    char *binary = malloc(len * 8 + 1); // each char is one byte (8 bits) and + 1 at the end for null terminator
    binary[0] = '\0';
    size_t i;
    for (i = 0; i < len; ++i)
    {
        char ch = s[i];
        int j;
        for (j = 7; j >= 0; --j)
        {
            if (ch & (1 << j))
            {
                strcat(binary, "1");
            }
            else
            {
                strcat(binary, "0");
            }
        }
    }
    return binary;
}

char* intToBinary(int n, int size)
{

    char *output;
    unsigned int i;
    int change;
    unsigned int actualPos = 1;
    output = (char*) malloc(size * sizeof(char));
    for (i = 0; i < size + 1; ++i)
    {
        output[i] = '0';
    }
    output[size] = '\0';

    for (i = 0; n > 0; i++)
    {
        change = n % 2;
        if (change == 0)
        {
            output[size - actualPos] = '0';
        }
        if (change == 1)
        {
            output[size - actualPos] = '1';
        }
        actualPos++;
        n = n / 2;
    }
    return output;

}

int binaryToInt(char *num, int base)
{
    char *eptr;
    //int binary_num = atoi(num);
    long binary_num = strtol(num, &eptr, base);
//    int decimal_num = 0, base = 1, rem;
//    sprintf(message, "--- BTI %d --- ",binary_num); //Error data reception
//    UART_TXData(message, strlen(message));

//    while (binary_num > 0)
//    {
//        rem = binary_num % 10; /* divide the binary number by 10 and store the remainder in rem variable. */
//        decimal_num = decimal_num + rem * base;
//        binary_num = binary_num / 10; // divide the number with quotient
//        base = base * 2;
//    }
//    return decimal_num;
    return binary_num;

}

void dataSend(char *messageToSend)
{

    strcat(TX_message, intToBinary(NODE_NUMBER, 8));
    unsigned int CRCResult;
    unsigned long toSend = binaryToInt(messageToSend, 2);
    CRC_setSeed(CRC_BASE, CRC_SEED);
    CRC_set16BitData(CRC_BASE, toSend);
    CRCResult = CRC_getResult(CRC_BASE);
    strcat(TX_message, messageToSend);
    strcat(TX_message, intToBinary(CRCResult, 16));

    sprintf(message, "SEND: %s ", TX_message); //Error data reception
    UART_TXData(message, strlen(message));
    unsigned int i = 0;
    for (i = 0; i < strlen(TX_message); i++)
    {
        if (TX_message[i] == '1')
        {
            GPIO_setOutputHighOnPin(DATA_TX_PORT, DATA_TX_PIN);
//            sprintf(message, "1");
//            UART_TXData(message, strlen(message));
        }
        else
        {
            GPIO_setOutputLowOnPin(DATA_TX_PORT, DATA_TX_PIN);
//            sprintf(message, "0");
//            UART_TXData(message, strlen(message));
        }
    }
}

