/* 
  get_path.h
  Ben Miller

*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/* function prototype.  It returns a pointer to a linked list for the path
   elements. */

typedef struct pathelement {
  char *element;			/* a dir in the path */
  struct pathelement *next;		/* pointer to next node */
} pathelement_t;

pathelement_t *get_path();
