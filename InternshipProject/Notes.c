/*
 * Notes.c
 *
 *  Created on: 27 Apr 2022
 *      Author: Lorenzo
 */


       if ((frequency == OOK_NODE1) || ((frequency <= OOK_NODE1 + 5))
                || (frequency >= (OOK_NODE1 - 5)))
        {
            if (count > ((LONG_BURST - BURST_GUARD) - 1))
            {
                nodeState[1] = LONG_BURST;
            }
            else if (count > ((MIDDLE_BURST - BURST_GUARD) - 1))
            {
                nodeState[1] = MIDDLE_BURST;
            }
            else if (count > ((SHORT_BURST - BURST_GUARD) - 1))
            {
                nodeState[1] = SHORT_BURST;
            }
        }
        else if ((frequency == OOK_NODE2) || ((frequency <= OOK_NODE2 + 5))
                || (frequency >= (OOK_NODE2 - 5)))
        {
            if (count > ((LONG_BURST - BURST_GUARD) - 1))
            {
                nodeState[1] = LONG_BURST;
            }
            else if (count > ((MIDDLE_BURST - BURST_GUARD) - 1))
            {
                nodeState[1] = MIDDLE_BURST;
            }
            else if (count > ((SHORT_BURST - BURST_GUARD) - 1))
            {
                nodeState[1] = SHORT_BURST;
            }
        }



