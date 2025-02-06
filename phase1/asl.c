#include "../h/asl.h"
#include "../h/pcb.h"
#include "../h/const.h"

#define MAXSEMD MAXPROC /* Same as MAXPROC*/

/* Static array of semaphores*/
static semd_t semdTable[MAXSEMD + 2]; /* +2 for dummy head/tail*/

/* Pointers to ASL and Free List*/
static semd_t *semd_h;     /* Active Semaphore List */
static semd_t *semdFree_h; /* Unused semaphores*/

/**
 * Initializes semdFree_h with semdTable[MAXSEMD] and sets up semd_h
 * with dummy head and tail nodes for efficient ASL traversal.
 * Called once during system initialization.
 */
void initASL()
{
    /* Initialize dummy head and tail nodes*/
    semd_h = &semdTable[0];      /* Head dummy node*/
    semd_h->s_semAdd = (int *)0; /* Set head value*/

    semd_t *tail = &semdTable[MAXSEMD + 1]; /* Tail dummy node*/
    tail->s_semAdd = (int *)MAXINT;         /* Set tail value*/

    semd_h->s_next = tail; /* Link head to tail*/
    tail->s_next = NULL;   /* Tail points to NULL*/

    /*Initialize the Free List*/
    semdFree_h = &semdTable[1]; /* First free semaphore */

    for (int i = 1; i < MAXSEMD; i++)
    {
        semdTable[i].s_next = &semdTable[i + 1]; /* Link free list elements*/
    }

    semdTable[MAXSEMD].s_next = NULL; /* Last free element points to NULL*/
}

/**
 * Traverses the ASL to find a semaphore descriptor matching `semAdd`.
 * Returns a pointer to the descriptor, or NULL if not found.
 */
static semd_t *findSemd(int *semAdd)
{
    semd_t *current = semd_h->s_next; /* Start from first real node */

    while (current != NULL && current->s_semAdd < semAdd)
    {
        current = current->s_next;
    }

    /* If found, return the descriptor; otherwise, return NULL*/
    if (current != NULL && current->s_semAdd == semAdd)
    {
        return current;
    }

    return NULL;
}

/**
 * Inserts the pcb p at the tail of the process queue associated with
 * the semaphore at semAdd. If the semaphore is inactive, allocates a 
 * new descriptor from semdFree_h and inserts it into the ASL in sorted order. 
 * Returns TRUE if a new descriptor is needed but semdFree_h is empty, 
 * otherwise returns FALSE.
 */
int insertBlocked(int *semAdd, pcb_t *p)
{
    if (p == NULL)
        return TRUE; /* Invalid input*/

    semd_t *semd = findSemd(semAdd); /* Look for existing semaphore*/

    if (semd == NULL)
    {
        /* Allocate new semd_t from the free list*/
        if (semdFree_h == NULL)
            return TRUE; /* No free descriptors available*/

        semd = semdFree_h;               /*Take first free descriptor*/
        semdFree_h = semdFree_h->s_next; /* Update free list*/

        /* Initialize new semaphore descriptor*/
        semd->s_semAdd = semAdd;
        semd->s_procQ = mkEmptyProcQ();

        /* Insert into ASL in sorted order*/
        semd_t *prev = semd_h;
        while (prev->s_next != NULL && prev->s_next->s_semAdd < semAdd)
        {
            prev = prev->s_next;
        }
        semd->s_next = prev->s_next;
        prev->s_next = semd;
    }

    /* Insert process into the process queue*/
    insertProcQ(&(semd->s_procQ), p);
    return FALSE;
}

/**
 * Removes and returns the first pcb from the process queue of the
 * semaphore at semAdd. If the semaphore is not found, returns NULL.
 * If the process queue becomes empty, removes the semaphore descriptor
 * from the ASL and returns it to semdFree_h.
 */
pcb_t *removeBlocked(int *semAdd)
{
    semd_t *semd = findSemd(semAdd); /* Find the semaphore descriptor*/

    if (semd == NULL)
        return NULL; /* Semaphore not found*/

    /* Remove the first pcb from the process queue*/
    pcb_t *removedpcb = removeProcQ(&(semd->s_procQ));
    if (removedpcb == NULL)
        return NULL; /* No process was blocked on this semaphore*/

    removedpcb->p_semAdd = NULL; /*Clear pcb's semaphore reference*/

    /* If the process queue is now empty, remove the semaphore descriptor from ASL*/
    if (emptyProcQ(semd->s_procQ))
    {
        /* Find and remove the semaphore descriptor from ASL*/
        semd_t *prev = semd_h;
        while (prev->s_next != NULL && prev->s_next != semd)
        {
            prev = prev->s_next;
        }
        if (prev->s_next == semd)
        {
            prev->s_next = semd->s_next; /* Unlink semd from ASL*/
        }

        /* Return the semaphore descriptor to the free list*/
        semd->s_next = semdFree_h;
        semdFree_h = semd;
    }

    return removedpcb;
}
