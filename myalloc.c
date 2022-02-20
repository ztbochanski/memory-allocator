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

void split_space(struct block *current_node, int padded_request_size, int padded_block_size, int required_space)
{
  if (current_node->size >= required_space)
  {
    // new node with remaining unused space
    struct block *new_node = current_node + padded_block_size + padded_request_size;
    new_node->size = current_node->size - padded_block_size - padded_request_size;
    // point current node to new node and set size
    current_node->next = new_node;
    current_node->size = padded_request_size;
  }
}

void myfree(void *pointer)
{
  struct block *node = pointer;
  // pre-decrement by type size to point to node start
  --node;
  node->in_use = 0;
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
      // split node if enough space
      int required_space = padded_request_size + padded_block_size + 16;
      if (current_node->size >= required_space)
      {
        split_space(current_node, padded_request_size, padded_block_size, required_space);
      }

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

void test_run1()
{
  void *p;

  p = myalloc(512);
  print_data();

  myfree(p);
  print_data();
}

void test_run2()
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

void test_run3()
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

int main()
{
  printf("\nRUN 1:\n");
  test_run1();
  printf("\nRUN 2:\n");
  test_run2();
  printf("\nRUN 3:\n");
  test_run3();
}
