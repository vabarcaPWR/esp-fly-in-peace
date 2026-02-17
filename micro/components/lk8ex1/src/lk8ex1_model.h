#ifndef LK8EX1_MODEL_H
#define LK8EX1_MODEL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef bool (*lk8ex1_model_parse_hex_cb_t)(const char *hex_digits, uint8_t *out_checksum);

uint8_t lk8ex1_model_checksum_payload(const char *data, size_t len);
uint8_t lk8ex1_model_checksum_sentence(const char *sentence, size_t len);
bool lk8ex1_model_validate_sentence(const char *sentence, lk8ex1_model_parse_hex_cb_t parse_hex_cb);

#endif
