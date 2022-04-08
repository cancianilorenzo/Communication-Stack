#include "TRAP.h"

uint16_t burst_length = 0;

void selectBurst(int energy_level)
{
    if (energy_level == ENERGY_FULL_STORAGE)
        {
            burst_length = LONG_BURST;
        }
        else if (energy_level >= ENERGY_MIDDLE_STORAGE)
        {
            burst_length = MIDDLE_BURST;
        }
        else if (energy_level < ENERGY_MIDDLE_STORAGE)
        {
            burst_length = SHORT_BURST;
        }

}

void TRAP(){
    if ((energy_level == ENERGY_MIDDLE_STORAGE) && (stat != DATA_RX)
                && (count >= MIDDLE_BURST)) //ENERGY_MIDDLE_STORAGE, OLD CODE ENERGY_FULL_STORAGE
        {
            //define and start timer 0 --> DATA_TX_TIME
            //power on pin for notify the TX start
            energy_level = energy_level - ENERGY_CONSUMED_TX; //update energy value;
            count = 0; //reset received burst counter

            stat = DATA_TX;
            mod = BURST_WAIT;
        }
}