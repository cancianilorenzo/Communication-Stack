#ifndef BOARDLIB_H_
#define BOARDLIB_H_

#include <string.h> //For memset


#define FRAM_TX_NUMBER 0x03

#define FRAM_RX_NUMBER 0x03


#define BIT_MASK_0 0xBF //10111111
#define BIT_MASK_1 0xFA //11111010
#define BIT_MASK_2 0xEF //11101111
#define BIT_MASK_3 0x7F //01111111

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
#define NODE_NUMBER 0x00
#define DATA_SIZE 40
#define CRC_SEED  0xBEEF



void initBoard();
void setTimers();
void setBoardFrequency();
void pinDeclaration();
void UARTInit();
void UART_TXData();
void UART_TXDataTOBOARD(unsigned char);

void producedData(char*);



int FRAMWriteTX(char*); //Function to write on FRAM
int FRAMWriteRX(char*, int); //Function to write on FRAM
void dataSend();
int dataToSend();
char* stringToBinary(char*);
char* intToBinary(int, int);
int binaryToInt(char*, int);
void parseData();
void setID();
void setData();
void setCRC();
int FRAMReadSenderRX();
char* FRAMReadDataRX(int);
void FRAMInit();

//struct storedData;
//struct storedData;
typedef struct storedData
{
    unsigned char nodeNumber;
    unsigned char data0;
    unsigned char data1;
    unsigned char data2;
    unsigned char data3;
    unsigned char CRC0;
    unsigned char CRC1;
    unsigned char timeStamp;
    unsigned char saved;
} storedData;

unsigned int canGetData();
struct storedData getdata();





#endif
