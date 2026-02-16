#include "unity.h"

/**
 * @brief Trivial sample test to verify Ceedling setup.
 *        Can be deleted once real component tests exist.
 */

void setUp(void) {}
void tearDown(void) {}

void test_sample_true_is_true(void)
{
    TEST_ASSERT_TRUE(1);
}

void test_sample_addition(void)
{
    TEST_ASSERT_EQUAL_INT(4, 2 + 2);
}
