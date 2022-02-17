#ifndef MYALLOC_H
#define MYALLOC_H

struct block
{
  int size;
  int in_use;
  struct block *next;
};

void *myalloc(int size);

#endif