#include "myalloc.h"
#include <stdio.h>
#include <unistd.h>

#define ALIGNMENT 16 // Must be power of 2
#define GET_PAD(x) ((ALIGNMENT - 1) - (((x)-1) & (ALIGNMENT - 1)))
#define PADDED_SIZE(x) ((x) + GET_PAD(x))
#define PTR_OFFSET(p, offset) ((void *)((char *)(p) + (offset)))

int SBRK_SIZE = 1024;

struct block *head = NULL;

void print_data(void)
{
  struct block *b = head;

  if (b == NULL)
  {
    printf("[empty]\n");
    return;
  }

  while (b != NULL)
  {
    // Uncomment the following line if you want to see the pointer values
    printf("[%p:%d,%s]", b, b->size, b->in_use ? "used" : "free");
    // printf("[%d,%s]", b->size, b->in_use ? "used" : "free");
    if (b->next != NULL)
    {
      printf(" -> ");
    }

    b = b->next;
  }

  printf("\n");
}

void *myalloc(int size)
{
  struct block *current, *previous;

  // request a chunk of memory from the OS
  if (head == NULL)
  {
    head = sbrk(SBRK_SIZE);
    head->next = NULL;
    head->size = SBRK_SIZE - PADDED_SIZE(sizeof(struct block));
    head->in_use = 0;
  }

  // set current node in linked list to the start of the sbrk
  current = head;

  // keep traversing to the first open block!
  while (((current->next) != NULL) && (((current->in_use) == 1) || ((current->size) < size)))
  {
    // move pointer to next block
    previous = current;
    current = current->next;
  }

  // required size perfectly fits
  if (size == (current->size))
  {
    current->in_use = 1;
  }
  // required size less than available in block
  else if (size < (current->size))
  {
    current->in_use = 1;
  }
  // required size larger than what is available, can't fit
  else
  {
    return NULL;
  }

  // return pointer at start of data
  unsigned long padded_block_size;
  padded_block_size = PADDED_SIZE(sizeof(struct block));
  return PTR_OFFSET(current, padded_block_size);
}

int main()
{
  void *p;

  print_data();
  p = myalloc(16);
  print_data();
  p = myalloc(16);
  printf("%p\n", p);
}
