#include "../h/pcb.h"
#include "../h/const.h"
// #include <stdio.h>

static pcb_t pcbTable[MAXPROC]; /* Static array for PCB storage */
static pcb_t *pcbFree_h = NULL; /* Head of free PCB list */

/**
 * Initializes the pcbFree list to contain all elements of the static array.
 * Called once during data structure initialization.
 */
void initPcbs()
{
    // printf("Initializing PCBs...\n"); // Debug message
    for (int i = 0; i < MAXPROC - 1; i++)
    {
        pcbTable[i].p_next = &pcbTable[i + 1]; // Link each PCB to the next one
    }
    pcbTable[MAXPROC - 1].p_next = NULL; // Last PCB points to NULL
    pcbFree_h = &pcbTable[0];            // Head points to the first PCB
    // printf("PCB Initialization Complete.\n"); // Confirm completion
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
    allocated->p_sib_left = NULL;
    allocated->p_sib_right = NULL;
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
 * Returns a NULL pointer, representing an empty process queue.
 */
pcb_t *mkEmptyProcQ()
{
    return NULL;
}
/**
 * Returns TRUE if the queue is empty, FALSE otherwise.
 */
int emptyProcQ(pcb_t *tp)
{
    return (tp == NULL);
}

/**
 * Inserts a PCB into the queue pointed to by *tp.
 * Updates the tail pointer if necessary.
 */
void insertProcQ(pcb_t **tp, pcb_t *p)
{
    if (p == NULL)
        return;

    if (*tp == NULL)
    {
        // If queue is empty, initialize it with p
        p->p_next = p;
        p->p_prev = p;
    }
    else
    {
        // Insert at the end of the circular queue
        p->p_next = (*tp)->p_next; // New node points to the old first node
        p->p_prev = *tp;           // New node points back to old tail

        (*tp)->p_next->p_prev = p; // Old first node points back to new node
        (*tp)->p_next = p;         // Old tail points to new node
    }

    *tp = p; // Update tail pointer to the new last node
}

/**
 * Removes and returns the first PCB (head) from the queue.
 * Updates the tail pointer if necessary.
 */
pcb_t *removeProcQ(pcb_t **tp)
{
    if (*tp == NULL)
        return NULL; // Empty queue

    pcb_t *head = (*tp)->p_next;

    if (head == *tp)
    {
        // Only one element in the queue
        *tp = NULL;
    }
    else
    {
        // Remove the head from the circular queue
        (*tp)->p_next = head->p_next;
        head->p_next->p_prev = *tp;
    }

    head->p_next = NULL;
    head->p_prev = NULL;
    return head;
}

/**
 * Removes a specific PCB from the queue.
 * Returns NULL if not found, otherwise returns the PCB.
 */

pcb_t *outProcQ(pcb_t **tp, pcb_t *p)
{
    if (*tp == NULL || p == NULL)
        return NULL; // Queue is empty or invalid input

    pcb_t *current = (*tp)->p_next; // Start from the head

    do
    {
        if (current == p)
        { // Found the PCB to remove
            if (current == *tp && current->p_next == current)
            {
                // If it's the only element in the queue
                *tp = NULL;
            }
            else
            {
                // Remove from the list
                current->p_prev->p_next = current->p_next;
                current->p_next->p_prev = current->p_prev;

                if (*tp == current)
                {
                    *tp = current->p_prev; // Update tail if necessary
                }
            }

            // Clear pointers before returning
            p->p_next = NULL;
            p->p_prev = NULL;
            return p; // Return the removed PCB
        }
        current = current->p_next;
    } while (current != (*tp)->p_next); // Stop when we loop back to the start

    return NULL; // PCB not found in the queue
}

/**
 * Returns the first PCB in the queue without removing it.
 */
pcb_t *headProcQ(pcb_t *tp)
{
    if (tp == NULL)
        return NULL;
    return tp->p_next;
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
    if (prnt == NULL || p == NULL)
        return;

    pcb_t *first_sib = prnt->p_child;

    prnt->p_child = p;          // Insert p as the first child
    p->p_prnt = prnt;           // Set parent pointer
    p->p_sib_right = first_sib; // Link new child to former first child
    p->p_sib_left = NULL;       // No left sibling for the first child

    if (first_sib != NULL)
    {
        first_sib->p_sib_left = p; // Update left sibling of old first child
    }
    p->p_prnt = prnt; // Set parent pointer
}

/**
 *  Make the first child of the pcb pointed to by p no longer a child of p.
 *  Return NULL if p has no children.
 *  Otherwise, return a pointer to this removed first child pcb.
 */
pcb_t *removeChild(pcb_t *p)
{
    if (emptyChild(p))
        return NULL;
    pcb_t *removed = p->p_child;
    return outChild(removed);
}

/**
 * Make the pcb pointed to by p no longer the child of its parent.
 * If the pcb pointed to by p has no parent, return NULL.
 * Otherwise, return p.
 */
pcb_t *outChild(pcb_t *p)
{
    if (p == NULL || p->p_prnt == NULL)
        return NULL;

    if (p->p_prnt->p_child == p)
    {
        // If `p` is the first child, update parent's child pointer
        p->p_prnt->p_child = p->p_sib_right;
    }

    // Check before accessing `p_sib_left`
    if (p->p_sib_left != NULL)
    {
        p->p_sib_left->p_sib_right = p->p_sib_right;
    }

    // Check before accessing `p_sib_right`
    if (p->p_sib_right != NULL)
    {
        p->p_sib_right->p_sib_left = p->p_sib_left;
    }

    // Clear removed child's links
    p->p_prnt = NULL;
    p->p_sib_left = NULL;
    p->p_sib_right = NULL;

    return p; // Return the removed child
}