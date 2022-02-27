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
    // printf("[%p:%d,%s]", b, b->size, b->in_use ? "used" : "free");
    printf("[%d,%s]", b->size, b->in_use ? "used" : "free");
    if (b->next != NULL)
    {
      printf(" -> ");
    }

    b = b->next;
  }

  printf("\n");
}

//                         p <- pointer to remaining
//                         v
// |-------544-------------|-----------480-----------|
// |16|-------512-------|16|-> NULL
//                  insert:512
//                      v
// |16|0|0|0|0|0|0|0|0|0|16|0|0|0|0|0|0|0|0|0|0|0|0|0|
// ^  ^                                             ^
// 0  p                                            1024
//    |-----------------1008------------------------| <- remaining space: head.size
// |-----------------1024---------------------------| <- total sbrk

void split_space(struct block *current_node, int padded_request_size)
{
  int padded_block_size = PADDED_SIZE(sizeof(struct block));
  struct block *new_node = PTR_OFFSET(current_node, padded_request_size + padded_block_size);
  new_node->size = current_node->size - padded_block_size - padded_request_size;
  current_node->size = padded_request_size;
  new_node->in_use = 0;

  new_node->next = current_node->next;
  current_node->next = new_node;
}

void myfree(void *pointer)
{
  // set block that passed-in address points to as free
  struct block *node = PTR_OFFSET(pointer, -PADDED_SIZE(sizeof(struct block)));
  node->in_use = 0;

  node = head;
  // traverse singly linked list O(n)
  while (node->next != NULL)
  {
    if (!node->in_use && !node->next->in_use)
    {
      node->size += node->next->size + PADDED_SIZE(sizeof(struct block));

      printf("cur node %p\n", node);
      printf("next node %p\n", node->next);
      printf("next next node %p\n", node->next->next);
      node->next = node->next->next;
      printf("new next node now %p\n", node->next);
    }
    else
    {
      node = node->next;
    }
  }
}

void *myalloc(int size)
{
  // request a chunk of memory from the OS,
  // note the example below uses 1kb as the sbrk request size

  // |16|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|
  // ^  ^                                             ^
  // 0  p                                            1024
  //    |-----------------1008------------------------| <- remaining space: head.size
  // |-----------------1024---------------------------| <- total sbrk

  if (head == NULL)
  {
    head = sbrk(SBRK_SIZE);
    head->next = NULL;
    head->size = SBRK_SIZE - PADDED_SIZE(sizeof(struct block));
    head->in_use = 0;
  }

  // pad user input to maintain alignment
  int padded_request_size = PADDED_SIZE(size);
  int padded_block_size = PADDED_SIZE(sizeof(struct block));

  // set current node in linked list to the start of the sbrk
  struct block *current_node = head;

  // keep traversing to the first open block!
  while (current_node != NULL)
  {
    if (!current_node->in_use && current_node->size >= padded_request_size)
    {
      split_space(current_node, padded_request_size);

      // mark in use return pointer to current space
      current_node->in_use = 1;
      return PTR_OFFSET(current_node, padded_block_size);
    }
    // move pointer to next block
    current_node = current_node->next;
  }
  // out of memory
  return NULL;
}

void split_test_run1()
{
  void *p;

  p = myalloc(512);
  print_data();

  myfree(p);
  print_data();
}

void split_test_run2()
{
  myalloc(10);
  print_data();
  myalloc(20);
  print_data();
  myalloc(30);
  print_data();
  myalloc(40);
  print_data();
  myalloc(50);
  print_data();
}

void split_test_run3()
{
  void *p;

  myalloc(10);
  print_data();
  p = myalloc(20);
  print_data();
  myalloc(30);
  print_data();
  myfree(p);
  print_data();
  myalloc(40);
  print_data();
  myalloc(10);
  print_data();
}

void coalesce_test1()
{
  // 0 bytes      32 bytes
  // b1  -------> b2 --------> NULL
  // v            v
  // |--16--|-10-6|--16--|------976--------|
  // |-----32-----|-----------992----------|
  // b1    | |                 | |
  // v     v v coalesce blocks v v
  // |--16--|-------------1008-------------|
  void *p;

  p = myalloc(10);
  print_data();

  myfree(p);
  print_data();
}

void coalesce_test2()
{
  void *p, *q;

  p = myalloc(10);
  print_data();
  q = myalloc(20);
  print_data();

  myfree(p);
  print_data();
  myfree(q);
  print_data();
}

int main()
{
  // -----------
  // SPLIT TESTS
  // -----------
  // printf("\nSPLIT RUN 1:\n");
  // test_run1();
  // printf("\nSPLIT RUN 2:\n");
  // test_run2();
  // printf("\nSPLIT RUN 3:\n");
  // test_run3();

  // --------------
  // COALESCE TESTS
  // --------------
  // printf("\nCOALESCE RUN 1:\n");
  // coalesce_test1();
  printf("\nCOALESCE RUN 2:\n");
  coalesce_test2();
}
