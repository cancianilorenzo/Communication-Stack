#include <msp430.h>
#include "driverlib.h"

#include <stdlib.h>     /* srand, rand */
#include <time.h>       /*time for random number*/
#include "TRAP.h"
#include "Timer.h"

/*-------------------------------------------------------------------------------------------------*/
//CONSTANT DECLARATION
#define NODE 0; //Node number, useful if UART is used to debug on Computer

#define ENERGY_UPDATE 500
#define ENERGY_CHANGE 50
#define ENERGY_INCREMENT 5

#define DATA_TX_TIME 5000 //Time needed to receive data (max time that board can wait on hang/receive)


/*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*/
//FUNCTION DECLARATION
void setBoardFrequency(); //set board + FRAM frequency
void pinDeclaration();
/*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*/
//Auxiliary variables
uint8_t energyLevel = 0;

TX_Status stat = DATA_WAIT; //Does not send and does not receive
MOD_Status mod = BURST_WAIT;

uint16_t count = 0; //pulses received
uint16_t pulses = 0; //pulses sended
uint16_t burst_length = 0;

uint16_t energy_count = 0;
uint16_t energy_count_limit = 0;
uint8_t energy_increment = 0;

/*-------------------------------------------------------------------------------------------------*/

int main()
{
    MPUCTL0 = MPUPW;
    MPUSEGB2 = 0x1000; // memory address 0x10000
    MPUSEGB1 = 0x0fc0; // memory address 0x0fc00
    MPUSAM &= ~MPUSEG2WE; // disallow writes
    MPUSAM |= MPUSEG2VS;  // reset CPU on violation
    MPUCTL0 = MPUPW | MPUENA;
    MPUCTL0_H = 0;
    WDT_A_hold(WDT_A_BASE);
    PMM_unlockLPM5(); // Disable the GPIO power-on default high-impedance mode
    __enable_interrupt(); // enable global interrupts

    setBoardFrequency(); //set clock to 16Mhz
    pinDeclaration();

    timerInitilization();

    burst_length = SHORT_BURST;

    startTimerA1(BURST_REPETITION);
    startTimerA0(ENERGY_UPDATE);

    srand(time(NULL));
    energy_count_limit = (ENERGY_CHANGE / 2) + (rand() % (ENERGY_CHANGE / 2 + 1));
    energy_increment = rand() % (ENERGY_INCREMENT + 1);

    while (1)
    {

        burst_length = selectBurstLength(energyLevel); //Define in TRAP.h

        /*ENERGY INCREMENT FUNCTION TBM ----------------------------------------------------------- TODO*/
        if (energy_count >= energy_count_limit)
        {
            energy_count_limit = (ENERGY_CHANGE / 2)
                    + (rand() % (ENERGY_CHANGE / 2 + 1));
            energy_increment = rand() % (ENERGY_INCREMENT + 1);
            energy_count = 0;
        }

        sendData(energyLevel, stat, count, mod, DATA_TX_TIME);

    }

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
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1; // Set all dividers to 1 for 16MHz operation
    CSCTL0_H = 0; // Lock CS registers                      // Lock CS registers

}




void pinDeclaration()
{
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN0); //Pin Data send
    GPIO_setAsOutputPin(GPIO_PORT_P6, GPIO_PIN0); //Pin Burst send

    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN1); //Pin Data receive
    P5IE |= BIT1; // P5.1 interrupt enabled
    P5IES |= BIT1; // P5.1 Hi/lo edge
    P5IFG &= ~BIT1; // P5.1 IFG cleared

    GPIO_setAsOutputPin(GPIO_PORT_P6, GPIO_PIN1); //Pin Burst receive
    P6IE |= BIT1;
    P6IES |= BIT1;
    P6IFG &= ~BIT1;
}




/*Energy generation (simulation) handler --  TIMER AO_1*/
#pragma vector = TIMER0_A0_VECTOR
__interrupt void T0A1_ISR(void)
{
    if (stat == DATA_WAIT)
    {
        uint8_t energy_step = rand() % (energy_increment + 1);
        energyLevel = energyLevel + energy_step;
        if (energyLevel >= ENERGY_FULL_STORAGE)
        {
            energyLevel = ENERGY_FULL_STORAGE;
        }
        energy_step = 120 + energy_step;
    }
}



/*Burst activation handler --  TIMER A1_1*/
#pragma vector = TIMER1_A0_VECTOR
__interrupt void T1A1_ISR(void)
{
    handlerTimerA1(pulses, mod);
}




/*BURST_RX TIMEOUT | DATA_TX | DATA_RX  --  TIMER A4_1*/
#pragma vector = TIMER4_A0_VECTOR
__interrupt void T4A1_ISR(void)
{
    handlerTimerA4(mod, stat, pulses);
}




/*Burst generation handler --  TIMER BO_1*/
#pragma vector = TIMER0_B0_VECTOR
__interrupt void T0B1_ISR(void)
{
    handlerTimerB0(mod, stat, pulses, burst_length);
}




#pragma vector=PORT5_VECTOR   // PORT5 interupt vector
__interrupt void handler_Data_RX(void)
{

    if (energyLevel >= ENERGY_CONSUMED_RX)
    {
        stopTimerA4();
        startTimerA4(DATA_TX_TIME);

        stat = DATA_RX;
        energyLevel = energyLevel - ENERGY_CONSUMED_RX;

    }
    else //Error in reception
    {
        stat = DATA_WAIT;
        energyLevel = 0; //Some error occur
    }
    P5IFG &= ~BIT1; // clean interrupt flag
}




#pragma vector=PORT6_VECTOR   // PORT5 interupt vector
__interrupt void handler_Burst_RX(void)
{

    if ((energyLevel == ENERGY_FULL_STORAGE) && (mod != BURST_TX)
            && (stat == DATA_WAIT))
    {
        //ISR RECEIVE BURTS FROM NEAR NODES
        mod = BURST_RX;
        if (count == 0)
        {
            startTimerA4(TIMEOUT + 1);
        }
        stopTimerA4();
        count++;
    }
    else
    {
        count = 0;
    }
    P6IFG &= ~BIT1; // clean interrupt flag
}
