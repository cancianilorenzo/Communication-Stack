<p align="center">
  <h2 align="center">TRAP library for n nodes</h2>
</p>


## Table of contents
- [Description](#description)
- [Setup](#setup)
- [Usage](#usage)
- [Example](#example)

### Description
TRAP folder contains TRAP.c and TRAP.h files that permit to use TRAP (TRAnsiently-powered Protocol) with an arbitrary number of MSP430 boards.


### Setup

To set the system correctly, you have to set the right frequencies on the MCLK, SMCLK and ACLK signals. In particular, this implementation of TRAP is based on having:
- ACLK at 250Hz
- SMCLK at 1MHz
<br><br>

The following code (for MSP430FR5994) can help you to set the board correctly with MCLK at 16MHz (source <a href="https://dev.ti.com/">TI DevTools</a>)

```
    // Configure one FRAM waitstate as required by the device datasheet for MCLK
    // operation beyond 8MHz _before_ configuring the clock system.
    FRCTL0 = FRCTLPW | NWAITS_1;

    // Clock System Setup
    CSCTL0_H = CSKEY_H;                     // Unlock CS registers
    CSCTL1 = DCOFSEL_0;                     // Set DCO to 1MHz
    // Set SMCLK = MCLK = DCO, ACLK = VLOCLK
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;

    CSCTL3 = DIVA__4 | DIVS__4 | DIVM__4; // Set all corresponding clk sources to divide by 4 for errata
    CSCTL1 = DCOFSEL_4 | DCORSEL; // Set DCO to 16MHz (Digital Controlled Oscillator)

    // Delay by ~10us to let DCO settle. 60 cycles = 20 cycles buffer + (10us / (1/4MHz))
    __delay_cycles(60);
    CSCTL3 = DIVA__32 | DIVS__16 | DIVM__1; // SMCLK set to 1MHz, ACLK set to 250Hz
    CSCTL0_H = 0; // Lock CS registers                      // Lock CS registers
```
<br>
The following code (for MSP430FR5969) can help you to set the board correctly with MCLK at 8MHz (source <a href="https://dev.ti.com/">TI DevTools</a>)

```
    //no waitstaet of FRAM because no over 8MHz clock speed!!
    // Startup clock system with max DCO setting ~8MHz
    CSCTL0_H = CSKEY >> 8;                    // Unlock clock registers
    CSCTL1 = DCOFSEL_3 | DCORSEL;             // Set DCO to 8MHz
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 = DIVA__32 | DIVS__8 | DIVM__1;     // Set all dividers
    CSCTL0_H = 0;                       // Lock CS registers
```
<br><br>
Now that you have modified the clocks, you can proceed to define the frequencies used by the boards (we will call them nodes from here on).

<b>Note that this operation have to be performed for each node!</b>
<br>

<b><p>TRAP.h file</p></b>
<br>
In this file you should change:
- TX frequency for burst editing with the wanted value (in kHz) the following line:
```
#define OOK_NODE 15 //Current node frequency
```
- Number of nodes in your system editing the following line:
```
#define NODES 2
```
- RX and TX GPIO (note that for RX you need also specify the correct PORTX_VECTOR for interrupt):
```
//HANDLER DEFINITION
#define BURST_RX_VECTOR PORT1_VECTOR
//GPIO DEFINITION
#define BURST_RX_PORT GPIO_PORT_P1
#define BURST_RX_PIN GPIO_PIN4
#define BURST_TX_PORT GPIO_PORT_P1
#define BURST_TX_PIN GPIO_PIN3
```
For convenience I have implemented a debug mode that allows information (including the receive frequency and number of pulses received) to be displayed via the UART, if you have implemented it. To activate it remove the comment from this line, to remove it comment the line.
```
//#define DEBUG 0
```

Output with DEBUG option 
<br>
<img src="https://github.com/cancianilorenzo/TRAP-on-MSP430FR/blob/main/OutputDebug.png">
<br>
C: "Pulses counted" F "Calculated frequency" NODE"X" "Energy level stored"

<b><p>TRAP.c file</p></b>
<br>
In this file you should change:
- RX frequency for burst adding all the value in kHz from LOWER to HIGHER including the current node TX value:
```
int OOK_NODE_INCOME[NODES] = {10, 35};
```

### Usage

Now that everything is set up for your needs, let's move on usage
<br>
Library expose few functions ready to go, you need to put this at the begin of your main function and then you can forget about TRAP, it will keep going in the backgroud
```
TRAPGPIO(); //Initialize GPIO needed from TRAP
TRAPTimer(); //Start TRAP timers, so TRAP as it is based on timers
```

To check whether your application can send data according to the TRAP protocol, simply call this function 
```
canSendTRAP(0);
```
passing as argument the position of the node in the previously defined OOK_NODE_INCOME array. 
<br>
This function will return 1 if the send can be done, 0 otherwise.
<br><br>
After the data sending you need to notify TRAP to update saved energy level of nodes, you can do this easily calling the following function
```
resetTRAP(0);
```
Passing, again, as argument the position of the node in the previously defined OOK_NODE_INCOME array

### Example

The example below shows a configuration for communication between two nodes. For the TRAP.c and TRAP.h files, only the portions modified according to the instructions given in [setup](#setup).
<br>
<p>NODE 0</p>
TRAP.h

```
#define NODES 2
...
#define OOK_NODE 15 //Current node frequency
...
//HANDLER DEFINITION
#define BURST_RX_VECTOR PORT3_VECTOR
//GPIO DEFINITION
#define BURST_RX_PORT GPIO_PORT_P3
#define BURST_RX_PIN GPIO_PIN0
#define BURST_TX_PORT GPIO_PORT_P1
#define BURST_TX_PIN GPIO_PIN2
```
As you can see NODE0 receive burst on pin 3.0 and send burst from pin 1.2

TRAP.c

```
int OOK_NODE_INCOME[NODES] = {15, 25};
```
<br>
<p>NODE 1</p>
TRAP.h

```
#define NODES 2
...
#define OOK_NODE 25 //Current node frequency
...
//HANDLER DEFINITION
#define BURST_RX_VECTOR PORT1_VECTOR
//GPIO DEFINITION
#define BURST_RX_PORT GPIO_PORT_P1
#define BURST_RX_PIN GPIO_PIN2
#define BURST_TX_PORT GPIO_PORT_P3
#define BURST_TX_PIN GPIO_PIN0
```
As you can see NODE0 receive burst on pin 1.2 and send burst from pin 3.0

TRAP.c

```
int OOK_NODE_INCOME[NODES] = {15, 25};
```

<br>
APPLICATION FOR NODE0

```
int main(void){
TRAPGPIO();
TRAPTimer();

if (canSendTRAP(1)){
  //SEND_DATA();
  resetTRAP(1);
}

```

<br>
APPLICATION FOR NODE1

```
int main(void){
TRAPGPIO();
TRAPTimer();

if (canSendTRAP(0)){
  //SEND_DATA();
  resetTRAP(0);
}

```
