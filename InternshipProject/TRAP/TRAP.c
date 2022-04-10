/*
 Lorenzo Canciani
 lorenzo.canciani@studenti.unitn.it
 2022
 */

#include "TRAP.h"

#define MAX_ENERGY 100
#define MIDDLE_ENERGY 70
#define LOW_ENERGY 35
#define LONG_BURST 256
#define MIDDLE_BURST 128
#define SHORT_BURST 64

int selectBurstLength(int energyLevel)
{
    int burst = -1;

    if (energyLevel == MAX_ENERGY)
    {
        burst = LONG_BURST;
    }
    else if (energyLevel > MIDDLE_ENERGY || energyLevel == MIDDLE_ENERGY)
    {
        burst = MIDDLE_BURST;
    }
    else if (energyLevel < MIDDLE_ENERGY)
    {
        burst = SHORT_BURST;
    }
    return burst;
}
