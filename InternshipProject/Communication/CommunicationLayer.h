#ifndef COMMUNICATION_COMMUNICATIONLAYER_H_
#define COMMUNICATION_COMMUNICATIONLAYER_H_

/*-----------------------------------------------------------------------------------------------------------------------------------------*/
/*Node status to know exactly in which state is the board for DATA side*/
extern int dataStatus;
#define DATA_WAIT           0
#define DATA_TX             1
#define DATA_RX             2
/*-----------------------------------------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------------------------------------------------*/
/*FRAM UTILITIES*/
#define FRAM_TX_NUMBER 0x03 //# OF DATA STORED IN FRAM FOR TX
#define FRAM_RX_NUMBER 0x03 //# OF DATA STORED IN FRAM FOR RX

#define NODE_NUMBER 0x00

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

unsigned int canGetData(); //CHECK IF THERE IS DATA STORED CORRECTLY IN FRAM (RX SIDE)
struct storedData getdata(); //POP DATA OUT OF FRAM AND RETURN A STRUCT OF storedData
void producedData(unsigned char, unsigned char, unsigned char, unsigned char); //STORE char* DATA IN FRAM (TX SIDE)
void dataSend(); //SEND STORED DATA TO OTHER NODES
/*-----------------------------------------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------------------------------------------------*/
/*CRC UTILITIES*/
#define BIT_MASK_0 0xBF //10111111
#define BIT_MASK_1 0xFA //11111010
#define BIT_MASK_2 0xEF //11101111
#define BIT_MASK_3 0x7F //01111111

#define CRC_SEED  0xBEEF
/*-----------------------------------------------------------------------------------------------------------------------------------------*/

void setTimers();
void FRAMInit();

extern unsigned char writePointer;
extern unsigned int currentReceived;
extern unsigned char RXPointer;
extern storedData storedRX[FRAM_RX_NUMBER];

void startCommunicationLayer();


#endif

