#include "../h/pcb.h"
#include "../h/const.h" 

static pcb_t pcbTable[MAXPROC]; /* Static array for PCB storage */
static pcb_t *pcbFree_h = NULL; /* Head of free PCB list */

/**
 * Initializes the pcbFree list to contain all elements of the static array.
 * Called once during data structure initialization.
 */
void initPcbs()
{
    for (int i = 0; i < MAXPROC - 1; i++)
    {
        pcbTable[i].p_next = &pcbTable[i + 1]; // Link each PCB to the next one
    }
    pcbTable[MAXPROC - 1].p_next = NULL; // Last PCB points to NULL
    pcbFree_h = &pcbTable[0];            // Head points to the first PCB
}

/**
 * Allocates a PCB from the pcbFree list.
 * Returns a pointer to the PCB or NULL if the list is empty.
 */
pcb_t *allocPcb()
{
    if (pcbFree_h == NULL)
        return NULL; // No available PCB

    pcb_t *allocated = pcbFree_h;  // Get the first PCB
    pcbFree_h = pcbFree_h->p_next; // Move head to next PCB

    /* Reset all fields */
    allocated->p_next = NULL;
    allocated->p_prev = NULL;
    allocated->p_prnt = NULL;
    allocated->p_child = NULL;
    allocated->p_sib = NULL;
    allocated->p_s = (state_t){0}; // Reset processor state
    allocated->p_time = 0;
    allocated->p_semAdd = NULL;
    allocated->p_supportStruct = NULL;

    return allocated;
}

/**
 * Frees a PCB and inserts it back into the pcbFree list.
 */
void freePcb(pcb_t *p)
{
    if (p == NULL)
        return;

    p->p_next = pcbFree_h; // Insert PCB at the front of the free list
    pcbFree_h = p;
}
