#include "lk8ex1.h"
#include "unity.h"

TEST_SOURCE_FILE("../components/lk8ex1/src/lk8ex1_model.c")
TEST_SOURCE_FILE("../components/lk8ex1/src/lk8ex1_hardware.c")
TEST_SOURCE_FILE("../components/lk8ex1/src/lk8ex1_conductor.c")

#include <stdio.h>
#include <string.h>

void setUp(void)
{
}
void tearDown(void)
{
}

static uint8_t xor_between_dollar_and_star(const char *sentence)
{
    const char *p = sentence;
    if (*p == '$')
    {
        p++;
    }
    uint8_t checksum = 0;
    while (*p && *p != '*')
    {
        checksum ^= (uint8_t)*p;
        p++;
    }
    return checksum;
}

static lk8ex1_data_t make_typical_data(void)
{
    return (lk8ex1_data_t){
        .pressure_pa = 101325,
        .altitude_m = 1000,
        .vario_cms = 50,
        .temperature_dc = 235,
        .battery_mv = 85,
    };
}

static esp_err_t format_into(const lk8ex1_data_t *data, char *buffer)
{
    return lk8ex1_format(data, buffer, LK8EX1_MAX_SENTENCE_LEN);
}

void test_format_typical_values(void)
{
    lk8ex1_data_t data = make_typical_data();
    char buffer[LK8EX1_MAX_SENTENCE_LEN];
    TEST_ASSERT_EQUAL(ESP_OK, format_into(&data, buffer));

    TEST_ASSERT_EQUAL_STRING_LEN("$LK8EX1,", buffer, 8);

    size_t len = strlen(buffer);
    TEST_ASSERT_EQUAL_CHAR('\r', buffer[len - 2]);
    TEST_ASSERT_EQUAL_CHAR('\n', buffer[len - 1]);

    TEST_ASSERT_NOT_NULL(strstr(buffer, "1013"));
    TEST_ASSERT_NOT_NULL(strstr(buffer, "1000"));
    TEST_ASSERT_NOT_NULL(strstr(buffer, ",50,"));
    TEST_ASSERT_NOT_NULL(strstr(buffer, ",235,"));
    TEST_ASSERT_NOT_NULL(strstr(buffer, ",85,*"));

    TEST_ASSERT_TRUE(lk8ex1_validate(buffer));
}

void test_format_no_gps_altitude(void)
{
    lk8ex1_data_t data = {96000, 99999, -120, 180, 3700};
    char buffer[LK8EX1_MAX_SENTENCE_LEN];
    TEST_ASSERT_EQUAL(ESP_OK, format_into(&data, buffer));
    TEST_ASSERT_NOT_NULL(strstr(buffer, ",99999,"));
    TEST_ASSERT_NOT_NULL(strstr(buffer, ",3700,*"));
    TEST_ASSERT_TRUE(lk8ex1_validate(buffer));
}

void test_format_no_battery(void)
{
    lk8ex1_data_t data = {101325, 452, 0, 210, 999};
    char buffer[LK8EX1_MAX_SENTENCE_LEN];
    TEST_ASSERT_EQUAL(ESP_OK, format_into(&data, buffer));
    TEST_ASSERT_NOT_NULL(strstr(buffer, ",999,*"));
    TEST_ASSERT_TRUE(lk8ex1_validate(buffer));
}

void test_format_calibrated_altitude(void)
{
    lk8ex1_data_t data = {101325, 452, 50, 235, 4100};
    char buffer[LK8EX1_MAX_SENTENCE_LEN];
    TEST_ASSERT_EQUAL(ESP_OK, format_into(&data, buffer));
    TEST_ASSERT_NOT_NULL(strstr(buffer, ",452,"));
    TEST_ASSERT_TRUE(lk8ex1_validate(buffer));
}

void test_format_zero_vario(void)
{
    lk8ex1_data_t data = {100000, 99999, 0, 200, 90};
    char buffer[LK8EX1_MAX_SENTENCE_LEN];
    TEST_ASSERT_EQUAL(ESP_OK, format_into(&data, buffer));
    TEST_ASSERT_NOT_NULL(strstr(buffer, ",0,"));
    TEST_ASSERT_TRUE(lk8ex1_validate(buffer));
}

void test_format_negative_vario(void)
{
    lk8ex1_data_t data = {101000, 99999, -200, 150, 75};
    char buffer[LK8EX1_MAX_SENTENCE_LEN];
    TEST_ASSERT_EQUAL(ESP_OK, format_into(&data, buffer));
    TEST_ASSERT_NOT_NULL(strstr(buffer, ",-200,"));
    TEST_ASSERT_TRUE(lk8ex1_validate(buffer));
}

void test_format_null_data(void)
{
    char buffer[LK8EX1_MAX_SENTENCE_LEN];
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, lk8ex1_format(NULL, buffer, sizeof(buffer)));
}

void test_format_null_buffer(void)
{
    lk8ex1_data_t data = {0};
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, lk8ex1_format(&data, NULL, LK8EX1_MAX_SENTENCE_LEN));
}

void test_format_buffer_too_small(void)
{
    lk8ex1_data_t data = make_typical_data();
    char buffer[10];
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_SIZE, lk8ex1_format(&data, buffer, sizeof(buffer)));
}

void test_format_exact_min_buffer(void)
{
    lk8ex1_data_t data = make_typical_data();
    char buffer[LK8EX1_MAX_SENTENCE_LEN];
    TEST_ASSERT_EQUAL(ESP_OK, format_into(&data, buffer));
    TEST_ASSERT_TRUE(lk8ex1_validate(buffer));
}

void test_checksum_matches_manual_xor(void)
{
    lk8ex1_data_t data = make_typical_data();
    char buffer[LK8EX1_MAX_SENTENCE_LEN];
    format_into(&data, buffer);

    TEST_ASSERT_EQUAL_HEX8(xor_between_dollar_and_star(buffer), lk8ex1_checksum(buffer, strlen(buffer)));
}

void test_checksum_null_sentence(void)
{
    TEST_ASSERT_EQUAL_HEX8(0, lk8ex1_checksum(NULL, 10));
}

void test_checksum_zero_length(void)
{
    TEST_ASSERT_EQUAL_HEX8(0, lk8ex1_checksum("$LK8EX1*00", 0));
}

void test_checksum_missing_dollar(void)
{
    TEST_ASSERT_EQUAL_HEX8(0, lk8ex1_checksum("LK8EX1,101325*FF", 16));
}

void test_checksum_missing_star(void)
{
    TEST_ASSERT_EQUAL_HEX8(0, lk8ex1_checksum("$LK8EX1,101325FF", 17));
}

void test_checksum_known_simple(void)
{
    TEST_ASSERT_EQUAL_HEX8(0x03, lk8ex1_checksum("$AB*03\r\n", 8));
}

void test_validate_correct_sentence(void)
{
    lk8ex1_data_t data = make_typical_data();
    char buffer[LK8EX1_MAX_SENTENCE_LEN];
    format_into(&data, buffer);
    TEST_ASSERT_TRUE(lk8ex1_validate(buffer));
}

void test_validate_corrupted_payload(void)
{
    lk8ex1_data_t data = make_typical_data();
    char buffer[LK8EX1_MAX_SENTENCE_LEN];
    format_into(&data, buffer);
    buffer[5] = 'Z';
    TEST_ASSERT_FALSE(lk8ex1_validate(buffer));
}

void test_validate_corrupted_checksum(void)
{
    lk8ex1_data_t data = make_typical_data();
    char buffer[LK8EX1_MAX_SENTENCE_LEN];
    format_into(&data, buffer);

    char *star = strchr(buffer, '*');
    TEST_ASSERT_NOT_NULL(star);
    star[1] = 'F';
    star[2] = 'F';
    TEST_ASSERT_FALSE(lk8ex1_validate(buffer));
}

void test_validate_null_input(void)
{
    TEST_ASSERT_FALSE(lk8ex1_validate(NULL));
}

void test_validate_missing_dollar(void)
{
    TEST_ASSERT_FALSE(lk8ex1_validate("LK8EX1,1013,1000,50,235,85,*00\r\n"));
}

void test_validate_missing_star(void)
{
    TEST_ASSERT_FALSE(lk8ex1_validate("$LK8EX1,1013,1000,50,235,85,00\r\n"));
}

void test_validate_too_short(void)
{
    TEST_ASSERT_FALSE(lk8ex1_validate("$*0A"));
    TEST_ASSERT_FALSE(lk8ex1_validate(""));
    TEST_ASSERT_FALSE(lk8ex1_validate("$A"));
}

void test_validate_known_good(void)
{
    TEST_ASSERT_TRUE(lk8ex1_validate("$AB*03\r\n"));
}

void test_validate_multiple_formats(void)
{
    lk8ex1_data_t test_cases[] = {
        {101325, 99999, 0, 200, 85},
        {96000, 452, -300, 150, 3700},
        {110000, 1500, 500, 350, 4200},
        {80000, 8848, 100, 50, 3000},
    };

    char buffer[LK8EX1_MAX_SENTENCE_LEN];
    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++)
    {
        TEST_ASSERT_EQUAL(ESP_OK, format_into(&test_cases[i], buffer));
        TEST_ASSERT_TRUE(lk8ex1_validate(buffer));
    }
}
