#ifndef MAIN_CARD_DB_H
#define MAIN_CARD_DB_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAIN_CARD_MAX_COUNT 10U

typedef enum {
    MAIN_CARD_ADD_OK = 0,
    MAIN_CARD_ADD_EXISTS,
    MAIN_CARD_ADD_FULL
} main_card_add_result_t;

typedef enum {
    MAIN_CARD_DEL_OK = 0,
    MAIN_CARD_DEL_NOT_FOUND
} main_card_del_result_t;

typedef struct {
    uint8_t used;
    uint8_t uid[4];
} main_card_record_t;

main_card_add_result_t main_card_db_add(const uint8_t uid[4]);
main_card_del_result_t main_card_db_remove(const uint8_t uid[4]);
uint8_t main_card_db_contains(const uint8_t uid[4]);
void main_card_db_clear(void);
void main_card_db_print(void);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_CARD_DB_H */
