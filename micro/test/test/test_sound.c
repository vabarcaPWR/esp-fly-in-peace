#include "sound.h"
#include "mock_piezo.h"
#include "unity.h"

TEST_SOURCE_FILE("../components/sound/src/sound.c")

static int piezo_provider_call_count;

static esp_err_t fake_init(void)
{
    return ESP_OK;
}

static esp_err_t fake_update(double vario_cms)
{
    (void)vario_cms;
    return ESP_OK;
}

static esp_err_t fake_play_startup(void)
{
    return ESP_OK;
}

static const char *fake_get_name(void)
{
    return "piezo";
}

static const sound_generator_t *stub_get_piezo_sound_generator(int cmock_num_calls)
{
    (void)cmock_num_calls;
    piezo_provider_call_count++;

    static const sound_generator_t fake_gen = {
        .init = fake_init,
        .update = fake_update,
        .play_startup = fake_play_startup,
        .get_name = fake_get_name,
    };

    return &fake_gen;
}

void setUp(void)
{
    piezo_provider_call_count = 0;
    get_piezo_sound_generator_StubWithCallback(stub_get_piezo_sound_generator);
}

void tearDown(void) {}

void test_get_sound_generator_null_name_returns_null(void)
{
    TEST_ASSERT_NULL(get_sound_generator(NULL));
}

void test_get_sound_generator_unknown_name_returns_null(void)
{
    TEST_ASSERT_NULL(get_sound_generator("unknown"));
}

void test_get_sound_generator_unknown_does_not_query_piezo(void)
{
    (void)get_sound_generator("unknown");
    TEST_ASSERT_EQUAL_INT(0, piezo_provider_call_count);
}

void test_get_sound_generator_piezo_returns_non_null(void)
{
    TEST_ASSERT_NOT_NULL(get_sound_generator("piezo"));
}

void test_get_sound_generator_piezo_queries_piezo_backend(void)
{
    (void)get_sound_generator("piezo");
    TEST_ASSERT_EQUAL_INT(1, piezo_provider_call_count);
}

void test_get_sound_generator_piezo_backend_name_matches(void)
{
    const sound_generator_t *gen = get_sound_generator("piezo");
    TEST_ASSERT_NOT_NULL(gen);
    TEST_ASSERT_EQUAL_STRING("piezo", gen->get_name());
}

void test_get_sound_generator_piezo_backend_has_play_startup(void)
{
    const sound_generator_t *gen = get_sound_generator("piezo");
    TEST_ASSERT_NOT_NULL(gen);
    TEST_ASSERT_NOT_NULL(gen->play_startup);
}

void test_get_sound_generator_piezo_play_startup_returns_ok(void)
{
    const sound_generator_t *gen = get_sound_generator("piezo");
    TEST_ASSERT_NOT_NULL(gen);
    TEST_ASSERT_EQUAL(ESP_OK, gen->play_startup());
}
