#include "../h/asl.h"
#include "../h/pcb.h"
#include "../h/const.h"

#define MAXSEMD MAXPROC // Same as MAXPROC

// Static array of semaphores
static semd_t semdTable[MAXSEMD + 2]; // +2 for dummy head/tail

// Pointers to ASL and Free List
static semd_t *semd_h;     // Active Semaphore List 
static semd_t *semdFree_h; // Unused semaphores

/**
 * Initializes semdFree_h with semdTable[MAXSEMD] and sets up semd_h
 * with dummy head and tail nodes for efficient ASL traversal.
 * Called once during system initialization.
 */
void initASL()
{
    // Initialize dummy head and tail nodes
    semd_h = &semdTable[0];      // Head dummy node
    semd_h->s_semAdd = (int *)0; // Set head value

    semd_t *tail = &semdTable[MAXSEMD + 1]; // Tail dummy node
    tail->s_semAdd = (int *)MAXINT;         // Set tail value

    semd_h->s_next = tail; // Link head to tail
    tail->s_next = NULL;   // Tail points to NULL

    // Initialize the Free List
    semdFree_h = &semdTable[1]; // First free semaphore 

    for (int i = 1; i < MAXSEMD; i++)
    {
        semdTable[i].s_next = &semdTable[i + 1]; // Link free list elements
    }

    semdTable[MAXSEMD].s_next = NULL; // Last free element points to NULL
}

