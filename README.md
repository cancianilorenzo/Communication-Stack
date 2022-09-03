<p align="center">
  <h2 align="center">A Tiny Network Stack for Battery-free Communication</h2>
</p>
<br>

## Table of Contents
- [Repo Structure](#repo-structure)
- [Goal Description](#goal-description)
- [Prerequisites](#prerequisites)
- [Usage](#usage)
- [Example](#example)
- [TestBed](#testbed)

---

## Goal Description
The use of battery-free systems is a revolutionary frontier for the Internet of Things; in
fact, it allows network nodes to be unconstrained by the use of batteries.
To achieve this, battery-free systems use ambient energy to charge a capacitor and use it
as a power source. Because of the unpredictability of environmental energy sources, the
node is often subject to power failure. Communication then becomes intermittent and
data loss particularly frequent.
To solve this problem, we implemented a tiny communication stack that implements non-
volatile memory management for data storage and the TRAP protocol for energy aware-
ness between nodes.

---

## Prerequisites
The project was created using version 11.1 of Code Composer Studio.<br>
To have full compatibility use this version. However, earlier or later versions of the IDE should still work without major changes.<br>
The compiler used is the Texas Instruments TI v21.6.0.LTS.

---

## Usage
Clone this repository and import the "CommunicationStack" project to Code Composer Studio.<br>
The project contains 4 main folders:<br>
- Board
- Communication 
- Physical
- TRAP

The system needs to be set up as needed, to do this let's go through the possible changes to be made to the files.<br>

### TRAP folder
<b><p>TRAP.h file</p></b>
<br>
In this file you should change:
- TX frequency for burst editing with the wanted value (in kHz) the following line:
```C
#define OOK_NODE 15 //Current node frequency
```
- Number of nodes in your system editing the following line:
```C
#define NODES 2
```
- RX and TX GPIO (note that for RX you need also specify the correct PORTX_VECTOR for interrupt):
```C
//HANDLER DEFINITION
#define BURST_RX_VECTOR PORT1_VECTOR
//GPIO DEFINITION
#define BURST_RX_PORT GPIO_PORT_P1
#define BURST_RX_PIN GPIO_PIN4
#define BURST_TX_PORT GPIO_PORT_P1
#define BURST_TX_PIN GPIO_PIN3
```

<b><p>TRAP.c file</p></b>
<br>
In this file you should change:
- RX frequency for burst adding all the value in kHz from LOWER to HIGHER including the current node TX value:
```C
int OOK_NODE_INCOME[NODES] = {10, 35};
```

### Communication folder
<b><p>CommunicationLayer.h file</p></b>
<br>
In this file you should change:

- RX and TX buffer size editing the following lines:
```C
#define FRAM_TX_NUMBER 0x03 //# OF DATA STORED IN FRAM FOR TX
#define FRAM_RX_NUMBER 0x03 //# OF DATA STORED IN FRAM FOR RX
```

- Node number ID within the network editing the following line:
```C
#define NODE_NUMBER 0x00
```

### Board folder

This folder contains the board clocks definitions, you should not need to change if your board can go to 16MhZ, if not, stick to the instructions on the datasheet of the board you are using and set the following values:
- ACLK at 250Hz
- SMCLK at 1MHz

### Application Layer
After setting up the system correctly, you can proceed to use the system. Everything is handled by the main.c file, which is the application layer.
As first thing you need to start the various layers that make up the system, to do this you need to call, once, the following functions:
```C
initBoard();
startPhysicalLayer();
startCommunicationLayer();
startTRAPLayer();
```

Subsequently, the use of the system is simple; thanks to the functions exposed by the Communication Layer, it is possible to send and receive packets to and from nodes in the network.<br>
```C
producedData(0xab, 0x12, 0x23, 0x67, 0x00); //To save data in TX buffer
dataSend(); //To send data to other nodes
getdata(); //To get data from RX buffer
```

## Example
This section shows an example of using the system:
```C
#include "Board/Board.h"
#include "TRAP\TRAP.h"
#include "Communication/CommunicationLayer.h"
#include "Physical/Physicallayer.h"

int main(void)
{
    initBoard();
    startPhysicalLayer();
    startCommunicationLayer();
    startTRAPLayer();
    while (1)
    {
        //4bytes data, latest byte destination node
        producedData(0xab, 0x12, 0x23, 0x67, 0x00);

        //Send data
        dataSend();

        //Retrive received data
        storedData data = getdata();
    }
}
```
---

## TestBed
