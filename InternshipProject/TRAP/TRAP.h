#ifndef TRAP_H_
#define TRAP_H_

#define DEBUG 0
#define NODES 2



extern int energyLevel;
/*Energy simulation params, can be remove in real world application*/
//extern int energyLevel;

#define ENERGY_CHANGE       70      // energy variation parameter
#define ENERGY_INCREMENT    5       // energy maximum increment
#define MAX_ENERGY          100     //MAx energy that can be stored in a node

#define ENERGY_CONSUMED_TX 70 //Energy consumed in TX
#define ENERGY_CONSUMED_RX 35 //Energy consumed in RX

//#define ENERGY_UPDATE_RATE 125


/************ ON/OFF KEY MODULATION FREQUENCY IN khz ***********/
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
#define OOK_NODE 10 //Current node frequency
extern int OOK_NODE_INCOME[NODES];
/*-----------------------------------------------------------------------------------------------------------------------------------------*/

/************ HANDLER PIN DEFINITION ***********/
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
//HANDLER DEFINITION
#define BURST_RX_VECTOR PORT1_VECTOR
//GPIO DEFINITION
#define BURST_RX_PORT GPIO_PORT_P1
#define BURST_RX_PIN GPIO_PIN4
#define BURST_TX_PORT GPIO_PORT_P1
#define BURST_TX_PIN GPIO_PIN3

/*-----------------------------------------------------------------------------------------------------------------------------------------*/

/************ TIMER REGISTERS DEFINITION ***********/
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
//TIMER FOR NODE IDENTIFICATION
#define NODE_ID_CCCR TA2CCTL0
#define NODE_ID_CR TA2CTL
#define NODE_ID_EV TA2CCR0
/*-----------------------------------------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------------------------------------------------*/
//TIMER FOR TIMEOUT BURST RECEPTION
#define BURST_TIMEOUT_CCCR TA3CCTL0
#define BURST_TIMEOUT_CR TA3CTL
#define BURST_TIMEOUT_EV TA3CCR0
/*-----------------------------------------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------------------------------------------------*/
//TIMER FOR BURST REPETITION
#define BURST_REPETITION_CCCR TA1CCTL0
#define BURST_REPETITION_CR TA1CTL
#define BURST_REPETITION_EV TA1CCR0

//VALUE FOR BURST REPETITION
//FOR CLOCK AT 8MHZ
//#define BURST_REPETITION_PERIOD 500 //For repeat the burst every 1 second
//FOR CLOCK AT 16MHZ
#define BURST_REPETITION_PERIOD 250 //For repeat the burst every 1 second
/*-----------------------------------------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------------------------------------------------*/
//TIMER FOR PULSES SEND
#define PULSES_SEND_CCCR TB0CCTL0
#define PULSES_SEND_CR TB0CTL
#define PULSES_SEND_EV TB0CCR0
/*-----------------------------------------------------------------------------------------------------------------------------------------*/



/************ HANDLER TIMER NAME DEFINITION ***********/
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
//HANDLER DEFINITION
#define BURST_REPETITION TIMER1_A0_VECTOR
#define PULSES_SEND TIMER0_B0_VECTOR
#define NODE_ID TIMER3_A0_VECTOR

/*-----------------------------------------------------------------------------------------------------------------------------------------*/


/************ NODE STATUS TO AVOID CONFLICTS IN RX/TX ***********/
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
extern int nodeStatus;
#define BURST_WAIT           0
#define BURST_TX             1
#define BURST_RX             2
/*-----------------------------------------------------------------------------------------------------------------------------------------*/


/************ ENERGY AND BURST PARAMS ***********/
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
#define MAX_ENERGY 100
#define MIDDLE_ENERGY 70
#define LOW_ENERGY 35
#define LONG_BURST 256
#define MIDDLE_BURST 128
#define SHORT_BURST 64
#define BURST_GUARD 40 //Burst Guard
/*-----------------------------------------------------------------------------------------------------------------------------------------*/

/************ TIMEOUT BURST FOR RX ***********/
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
#define TIMEOUT   15
/*-----------------------------------------------------------------------------------------------------------------------------------------*/

/************ ARRAY TO STORE NODE STATE ***********/
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
extern int nodeState[];
/*-----------------------------------------------------------------------------------------------------------------------------------------*/



void TRAPTimer();
void selectBurstLengthTRAP(int);
int canSendTRAP(int);
void TRAPGPIO();
void resetTRAP(int);

void startTRAPLayer();

#endif


