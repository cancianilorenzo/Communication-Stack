# TRAP simulation on MSP430FR5994

## Goal
This repository contains code to simulate the TRAP protocol with an arbitrary number of nodes.</br>

## Why MSP430FRxxxx
The choice to use MSP430 with FRAM technology was made due to the need to save data on non-volatile memory in order to be able to restore it in the event of a power failure. 

## Code flow
The operation of the code is not complicated; it relies on the TRAP protocol to exchange the energy level between boards and manage the sending and receiving of data. </br>
Timers are used to simulate the node's energy increase (this could be replaced in a real case with the use of a solar panel connected to a capacitor), to identify the frequency of the received burst and thus associate the correct energy level to the right node, to avoid getting stuck in burst reception in case of errors, to repet the burst transmission and to send the burst to the other nodes at the correct frequency. </br>
For MSP430FR5994 (the current target board, but the code can easily be ported to other msp430FR59xx boards) the choice of timer was as follows: </br>

    - TA0 --> Energy simulation
    - TA2 --> Node indentification
    - TA3 --> Timeout Burst reception
    - TA4 --> Burst repetition
    - TB0 --> Pulses send





