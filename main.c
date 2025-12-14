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

pthread_mutex_t global_malloc_lock;
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
  // set intial block
  if (!global_base) {
    block = request_space(size);
    global_base = block;
  } else {
    find_free_block(&block, size);
  }
  return (block + 1);
}

int main(int argc, char *argv[]) {
  int *ptr = (int *)gc_malloc(sizeof(int) * 5);

  if (ptr == NULL) {
    printf("Allocation Failed");
  }

  for (int i = 0; i < 5; i++)
    ptr[i] = i + 1;

  for (int i = 0; i < 5; i++)
    printf("%d ", ptr[i]);
  return 0;
}
