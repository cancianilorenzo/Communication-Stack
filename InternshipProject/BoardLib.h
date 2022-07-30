#ifndef BOARDLIB_H_
#define BOARDLIB_H_

#include <string.h> //For memset

/*-----------------------------------------------------------------------------------------------------------------------------------------*/
/*Energy simulation params, can be remove in real world application*/
extern int energyLevel;

#define ENERGY_CHANGE       50      // energy variation parameter
#define ENERGY_INCREMENT    5       // energy maximum increment
#define MAX_ENERGY          100     //MAx energy that can be stored in a node

#define ENERGY_CONSUMED_TX 70 //Energy consumed in TX
#define ENERGY_CONSUMED_RX 35 //Energy consumed in RX

void startEnergyTimer(int);
void startEnergySimulation();
void interruptEnergy(void);

/*-----------------------------------------------------------------------------------------------------------------------------------------*/





//DATA_TX PORT
#define DATA_TX_PORT_0 GPIO_PORT_P2
//#define DATA_TX_PORT_1 GPIO_PORT_P1
//#define DATA_TX_PORT_2
//#define DATA_TX_PORT_3
//#define DATA_TX_PORT_4
//#define DATA_TX_PORT_5
//#define DATA_TX_PORT_6
//#define DATA_TX_PORT_7

//DATA_TX PIN
#define DATA_TX_PIN_0 GPIO_PIN6
//#define DATA_TX_PIN_1 GPIO_PIN5
//#define DATA_TX_PIN_2
//#define DATA_TX_PIN_3
//#define DATA_TX_PIN_4
//#define DATA_TX_PIN_5
//#define DATA_TX_PIN_6
//#define DATA_TX_PIN_7

//DATA_RX VECTOR
#define DATA_RX_VECTOR PORT8_VECTOR
//DATA_TX PORT
#define DATA_RX_PORT_0 GPIO_PORT_P8
//#define DATA_RX_PORT_1 GPIO_PORT_P3
//#define DATA_RX_PORT_2
//#define DATA_RX_PORT_3
//#define DATA_RX_PORT_4
//#define DATA_RX_PORT_5
//#define DATA_RX_PORT_6 GPIO_PORT_P3
//#define DATA_RX_PORT_7

//DATA_RX PIN
#define DATA_RX_PIN_0 GPIO_PIN0
//#define DATA_RX_PIN_1 GPIO_PIN1
//#define DATA_RX_PIN_2
//#define DATA_RX_PIN_3
//#define DATA_RX_PIN_4
//#define DATA_RX_PIN_5
//#define DATA_RX_PIN_6 GPIO_PIN6
//#define DATA_RX_PIN_7




#define RX_SIZE 10
extern char dataRec[];

/*-----------------------------------------------------------------------------------------------------------------------------------------*/
//typedef struct DATA
//{
//    char data[1000];
//    char state;
//} DATA;
//
////FRAM declaration
//#pragma PERSISTENT(dataStored)
//extern DATA dataStored = {.data = "c",
//   .state = 'c'};
#pragma PERSISTENT(dataStore)
extern char dataStore[1000];
//memset(dataStored, 0, sizeof(dataStored));

#pragma PERSISTENT(store)
extern char store;

 /*-----------------------------------------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------------------------------------------------*/
/*Node status to know exactly in which state is the board for DATA side*/
extern int dataStatus;
#define DATA_WAIT           0
#define DATA_TX             1
#define DATA_RX             2


/*-----------------------------------------------------------------------------------------------------------------------------------------*/

#define MSG_SIZE 64 //Uart MSG size
extern char message[];

/*-----------------------------------------------------------------------------------------------------------------------------------------*/


void initBoard();
void setTimers();
void setBoardFrequency();
void pinDeclaration();
void UARTInit();
void UART_TXData();



void FRAMWrite(char*, int); //Function to write on FRAM
void dataSend(char*, int);


void interruptON(int);
void interruptOFF(int);
int readPin(int);



#endif
