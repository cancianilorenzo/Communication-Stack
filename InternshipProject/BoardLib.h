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

//IF CLOCK AT 16MHZ uncomment rows below
#define ENERGY_UPDATE_RATE 125
#define RX_TX_RATE 1


//IF CLOCK AT 8MHZ uncomment rows below
//#define ENERGY_UPDATE_RATE 250
//#define RX_TX_RATE 2





//DATA_TX PORT
#define DATA_TX_PORT GPIO_PORT_P2

//DATA_TX PIN
#define DATA_TX_PIN GPIO_PIN6

//DATA_RX VECTOR
#define DATA_RX_VECTOR PORT3_VECTOR
//DATA_TX PORT
#define DATA_RX_PORT GPIO_PORT_P3

//DATA_RX PIN
#define DATA_RX_PIN GPIO_PIN0




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
#define SIZE_ID 8
#define NODE_NUMBER 11
#define DATA_SIZE 40
#define CRC_SEED  0xBEEF



void initBoard();
void setTimers();
void setBoardFrequency();
void pinDeclaration();
void UARTInit();
void UART_TXData();



int FRAMWrite(char*); //Function to write on FRAM
void dataSend(char*);
int dataToSend();
char* stringToBinary(char*);
char* intToBinary(int, int);
int binaryToInt(char*, int);
void parseData();
void setID();
void setData();
void setCRC();



#endif
