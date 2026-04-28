#ifndef KT_RINGBUF_H
#define KT_RINGBUF_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Generic ring buffer (FIFO) suitable for single-producer single-consumer
 *
 * Design assumptions (certified safe on Cortex-M3/M4/M7):
 *  - head is ONLY written by the producer (ISR), ONLY read by consumer
 *  - tail is ONLY written by the consumer (main loop), ONLY read by producer
 *  - Both head and tail are `volatile` to prevent compiler reordering
 *  - No critical sections / interrupt disable required
 *
 *  One slot is intentionally kept empty to distinguish "full" vs. "empty".
 *  Effective capacity = (size - 1) bytes.
 */
typedef struct {
    uint8_t  *buffer;           /**< Pointer to backing storage          */
    uint16_t  size;             /**< Total size of backing storage       */
    volatile uint16_t head;     /**< Producer index (ISR writes here)    */
    volatile uint16_t tail;     /**< Consumer index (main loop advances) */
} kt_ringbuf_t;

/**
 * @brief Initialize a ring buffer instance
 * @param rb      Pointer to ring buffer descriptor
 * @param buffer  Backing storage array
 * @param size    Size of backing storage (must be > 1)
 */
void kt_ringbuf_init(kt_ringbuf_t *rb, uint8_t *buffer, uint16_t size);

/**
 * @brief Put one byte into the ring buffer (call from producer, e.g. ISR)
 * @param rb    Ring buffer instance
 * @param byte  Byte to store
 * @return 0 on success, -1 if buffer is full
 */
int kt_ringbuf_put(kt_ringbuf_t *rb, uint8_t byte);

/**
 * @brief Get one byte from the ring buffer (call from consumer)
 * @param rb    Ring buffer instance
 * @param byte  Output: received byte
 * @return 0 on success, -1 if buffer is empty
 */
int kt_ringbuf_get(kt_ringbuf_t *rb, uint8_t *byte);

/**
 * @brief Check if ring buffer is empty
 * @return 1 if empty, 0 otherwise
 */
int kt_ringbuf_is_empty(kt_ringbuf_t *rb);

/**
 * @brief Check if ring buffer is full
 * @return 1 if full, 0 otherwise
 */
int kt_ringbuf_is_full(kt_ringbuf_t *rb);

/**
 * @brief Return number of bytes currently available for reading
 * @return Byte count (0 .. size-1)
 */
uint16_t kt_ringbuf_available(kt_ringbuf_t *rb);

#ifdef __cplusplus
}
#endif

#endif /* KT_RINGBUF_H */