#include "kt_ringbuf.h"
#include <stddef.h>

/**
 * @brief Initialize a ring buffer instance
 */
void kt_ringbuf_init(kt_ringbuf_t *rb, uint8_t *buffer, uint16_t size)
{
    if (rb == NULL || buffer == NULL || size < 2) {
        /* Invalid parameters - cannot initialize */
        return;
    }
    rb->buffer = buffer;
    rb->size   = size;
    rb->head   = 0;
    rb->tail   = 0;
}

/**
 * @brief Put one byte into the ring buffer (producer, e.g. ISR)
 * @return 0 on success, -1 if full
 */
int kt_ringbuf_put(kt_ringbuf_t *rb, uint8_t byte)
{
    uint16_t next_head;

    if (rb == NULL || rb->buffer == NULL || rb->size < 2) {
        return -1;
    }

    next_head = rb->head + 1;
    if (next_head >= rb->size) {
        next_head = 0;
    }

    /* Full check: next_head == tail means buffer is full */
    if (next_head == rb->tail) {
        return -1;  /* Full */
    }

    rb->buffer[rb->head] = byte;
    rb->head = next_head;
    return 0;
}

/**
 * @brief Get one byte from the ring buffer (consumer)
 * @return 0 on success, -1 if empty
 */
int kt_ringbuf_get(kt_ringbuf_t *rb, uint8_t *byte)
{
    uint16_t next_tail;

    if (rb == NULL || rb->buffer == NULL || rb->size < 2 || byte == NULL) {
        return -1;
    }

    if (rb->head == rb->tail) {
        return -1;  /* Empty */
    }

    *byte = rb->buffer[rb->tail];

    next_tail = rb->tail + 1;
    if (next_tail >= rb->size) {
        next_tail = 0;
    }
    rb->tail = next_tail;

    return 0;
}

/**
 * @brief Check if ring buffer is empty
 * @return 1 if empty, 0 otherwise
 */
int kt_ringbuf_is_empty(kt_ringbuf_t *rb)
{
    if (rb == NULL || rb->buffer == NULL) {
        return 1;  /* Invalid state → treat as empty */
    }
    return (rb->head == rb->tail) ? 1 : 0;
}

/**
 * @brief Check if ring buffer is full
 * @return 1 if full, 0 otherwise
 */
int kt_ringbuf_is_full(kt_ringbuf_t *rb)
{
    uint16_t next_head;

    if (rb == NULL || rb->buffer == NULL || rb->size < 2) {
        return 1;  /* Invalid state → treat as full (can't write) */
    }

    next_head = rb->head + 1;
    if (next_head >= rb->size) {
        next_head = 0;
    }
    return (next_head == rb->tail) ? 1 : 0;
}

/**
 * @brief Return number of bytes currently available for reading
 * @return Byte count (0 .. size-1)
 */
uint16_t kt_ringbuf_available(kt_ringbuf_t *rb)
{
    if (rb == NULL || rb->buffer == NULL || rb->size < 2) {
        return 0;  /* Invalid state → nothing available */
    }

    if (rb->head >= rb->tail) {
        return rb->head - rb->tail;
    } else {
        return rb->size - rb->tail + rb->head;
    }
}