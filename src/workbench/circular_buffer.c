#include <stdlib.h>

typedef struct circular_buffer_s circular_buffer;

struct circular_buffer_s
{
  int head;
  int tail;
  int size;
  void** buffer;
};

circular_buffer* circular_buffer_new(int size)
{
  circular_buffer* cbuf = malloc(sizeof(circular_buffer_s) + sizeof(void*) * size);
  cbuf->head = 0;
  cbuf->tail = 0;
  cbuf->max_size = 0;
  cbuf->current_size;
  return cbuf;
}

int is_empty(circular_buffer* cb)
{
  return cb->current_size == 0;
}

int is_full(circular_buffer* cb)
{
  return cb->current_size == cb->max_size;
}

void add(circular_buffer* cb, void* data) {
  if (is_full(cb))
  {
    // Error: buffer is full
    return;
  }
  
  cb->buffer[cb->tail] = data;
  cb->tail = (cb->tail + 1) % cb->max_size;
  cb->current_size++;
}

void* remove(circular_buffer* cb) {
  if (is_empty(cb)) {
    // Error: buffer is empty
    return -1;
  }
  
  void* data = cb->buffer[cb->head];
  cb->head = (cb->head + 1) % cb->max_size;;
  cb->current_size--;
  
  return data;
}
