#ifndef BOARD_BOARD_H_
#define BOARD_BOARD_H_

void initBoard();
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
/*UART DATA*/
#define MSG_SIZE 64 //Uart MSG size
extern char message[];
void UART_TXData(); //UART TO PC
//void UART_TXDataTOBOARD(unsigned char); //UART TO NODES
/*-----------------------------------------------------------------------------------------------------------------------------------------*/

#endif
