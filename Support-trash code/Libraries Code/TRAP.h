#ifndef TRAP_H_INCLUDED
#define TRAP_H_INCLUDED

#define SHORT_BURST 64 //Pulses to send low-energy info
#define MIDDLE_BURST 128 //Pulses to send middle-energy info
#define LONG_BURST 256 //Pulses to send high-energy info (PROBABLY FULL??) NEED THINKING
#define BURST_GUARD 40 //Guard band for burst
#define BURST_REPETITION 1000 //Time between two burst repetition
#define TX_FREQUENCY 30 //Frequency at which it trasmit (Needed to understand which node is sending)
#define TIMEOUT 100 //Max time between two pulses, if more communication fails!!!
#define ENERGY_FULL_STORAGE 100 //Max storage
#define ENERGY_MIDDLE_STORAGE 70 //Middle storage
#define ENERGY_CONSUMED_RX 35 //Energy consumed in RX
#define ENERGY_CONSUMED_TX 70 //Energy consumed in TX
#define ENERGY_FULL_STORAGE 100 //Max storage
#define ENERGY_MIDDLE_STORAGE 70 //Middle storage

typedef enum
{
    DATA_WAIT = 0, //0x00
    DATA_TX = 1, //0x01
    DATA_RX = 2 //0x02
} TX_Status;

typedef enum
{
    BURST_WAIT = 0, //0x00
    BURST_TX = 1, //0x01
    BURST_RX = 2 //0x02
} MOD_Status;


void TRAP(int energy_level); //float?

#endif