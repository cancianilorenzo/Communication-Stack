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

#define ENERGY_UPDATE_RATE 125


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



int FRAMWrite(char*); //Function to write on FRAM
int dataToSend();



#endif
