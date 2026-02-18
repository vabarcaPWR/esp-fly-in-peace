#include "lk8ex1_model.h"

#include <string.h>

static const char *find_char(const char *str, size_t len, char target)
{
    for (size_t index = 0; index < len; index++)
    {
        if (str[index] == target)
            return &str[index];
    }
    return NULL;
}

uint8_t lk8ex1_model_checksum_payload(const char *data, size_t len)
{
    uint8_t checksum = 0;
    for (size_t index = 0; index < len; index++)
    {
        checksum ^= (uint8_t)data[index];
    }
    return checksum;
}

uint8_t lk8ex1_model_checksum_sentence(const char *sentence, size_t len)
{
    if (!sentence || !len)
        return 0;

    const char *payload_start = find_char(sentence, len, '$');
    if (!payload_start)
        return 0;
    payload_start++;

    size_t remaining = (size_t)(sentence + len - payload_start);
    const char *payload_end = find_char(payload_start, remaining, '*');
    if (!payload_end)
        return 0;

    return lk8ex1_model_checksum_payload(payload_start, (size_t)(payload_end - payload_start));
}

bool lk8ex1_model_validate_sentence(const char *sentence, lk8ex1_model_parse_hex_cb_t parse_hex_cb)
{
    if (!sentence || !parse_hex_cb)
        return false;

    size_t len = strlen(sentence);
    if (len < 7U || '$' != sentence[0])
        return false;

    const char *star = find_char(sentence + 1, len - 1, '*');
    if (!star)
        return false;

    size_t chars_after_star = (size_t)(sentence + len - star - 1);
    if (chars_after_star < 2U)
        return false;

    uint8_t embedded_checksum = 0;
    if (!parse_hex_cb(star + 1, &embedded_checksum))
        return false;

    uint8_t computed_checksum = lk8ex1_model_checksum_sentence(sentence, len);
    return computed_checksum == embedded_checksum;
}
