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


/**
 *  Return TRUE if the pcb pointed to by p has no children.
 *  Return FALSE otherwise. 
 */
int emptyChild(pcb_t *p)
{
    if (p->p_child == NULL)
        return TRUE;

    return FALSE;
}

/**
 * Make the pcb pointed to by p a child of the pcb pointed to by prnt.
 */
void insertChild(pcb_t *prnt, pcb_t *p)
{
    pcb_t *first_sib = prnt->p_child;
    prnt->p_child = p;
    p->p_sib_right = first_sib;
    first_sib->p_sib_left = p;
    p->p_prnt = prnt;
}

/**
 *  Make the first child of the pcb pointed to by p no longer a child of
 * p. Return NULL if initially there were no children of p. Otherwise,
 * return a pointer to this removed first child pcb. 
 */
pcb_t *removeChild(pcb_t *p)
{
    if (emptyChild(p))
        return NULL;
    pcb_t *removed = p->p_child;
    return outChild(removed);
}

/**
 *  Make the pcb pointed to by p no longer the child of its parent. If
 * the pcb pointed to by p has no parent, return NULL; otherwise, return
 * p. Note that the element pointed to by p need not be the first child of
 * its parent. 
 */
pcb_t *outChild(pcb_t *p)
{
    if (p->p_prnt==NULL)
        return NULL;

    if (p->p_prnt->p_child == p)
        p->p_prnt->p_child = p->p_sib_right;

    p->p_sib_left->p_sib_right = p->p_sib_right;
    p->p_sib_right->p_sib_left = p->p_sib_left;

    p->p_prnt = NULL;
    p->p_sib_left = NULL;
    p->p_sib_right = NULL;

    
}