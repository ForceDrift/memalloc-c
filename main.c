#include <assert.h>
#include <bits/pthreadtypes.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
typedef struct header {

  size_t size;
  u_int8_t status;
  struct header *next;
} header_t;

header_t *head = NULL;
header_t *global_base = NULL;

// get block if no free exist
header_t *request_space(size_t size) {
  header_t *memblock = (header_t *)sbrk(0);
  // line between heap and unallocated mmemory
  void *request = sbrk(size + sizeof(header_t));

  if (request == (void *)-1) {
    return NULL;
  }
  memblock->size = size;
  memblock->status = 1;
  memblock->next = NULL;
  return memblock;
}

header_t *find_free_block(header_t **last, size_t size) {
  header_t *current = global_base;
  while (current && (current->status != 0 && current->size < size)) {
    *last = current;
    current = current->next;
  }
  return current;
}

void *gc_malloc(size_t size) {
  header_t *block;
  header_t *last = NULL;
  // set intial block
  if (!global_base) {
    block = request_space(size);
    global_base = block;
  } else {
    block = find_free_block(&last, size);
    if (block == NULL) {
      block = request_space(size);
    }
    block->status = 1;
  }
  return (void *)(block + 1);
}

int main(int argc, char *argv[]) {

  int *ptr = (int *)gc_malloc(sizeof(int) * 5);

  if (ptr == NULL) {
    printf("Allocation Failed");
  }
  printf("Successfully allocated %zu bytes (5 ints) at data address %p\n",

         sizeof(int) * 5, (void *)ptr);
  for (int i = 0; i < 5; i++)
    ptr[i] = i + 1;

  for (int i = 0; i < 5; i++)
    printf("%d ", ptr[i]);
  return 0;
}
