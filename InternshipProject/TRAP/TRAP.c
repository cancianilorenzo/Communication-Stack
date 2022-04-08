/*
 Lorenzo Canciani
 lorenzo.canciani@studenti.unitn.it
 2022
 */

#include "TRAP.h"

#define MAX_ENERGY 100
#define MIDDLE_ENERGY 70
#define LOW_ENERGY 50
#define LONG_BURST 256
#define MIDDLE_BURST 128
#define SHORT_BURST 64

int selectBurstLength(int energyLevel)
{
    if (energyLevel == MAX_ENERGY)
    {
        return LONG_BURST;
    }
    else if (energyLevel >= MIDDLE_ENERGY)
    {
        return MIDDLE_BURST;
    }
    else if (energyLevel <= LOW_ENERGY)
    {
        return SHORT_BURST;
    }
    return -1;
}
