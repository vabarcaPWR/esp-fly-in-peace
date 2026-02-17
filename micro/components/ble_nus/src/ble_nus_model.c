#include "ble_nus_model.h"

#include <stdatomic.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "host/ble_att.h"

static SemaphoreHandle_t s_state_mutex;
static _Atomic bool s_initialized;
static _Atomic bool s_connected;
static uint16_t s_conn_handle;
static uint16_t s_tx_value_handle;
static uint16_t s_mtu;
static uint16_t s_adv_interval_ms;
static uint8_t s_own_addr_type;
static bool s_notify_enabled;
static ble_nus_rx_cb_t s_rx_callback;
static ble_nus_state_cb_t s_state_callback;
static char s_device_name[21];

esp_err_t ble_nus_model_init(const char *device_name, uint16_t adv_interval_ms, uint16_t default_adv_interval_ms,
                             uint16_t default_mtu, size_t max_device_name_len)
{
    if (device_name == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    size_t name_len = strlen(device_name);
    if (name_len == 0U || name_len > max_device_name_len)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (s_state_mutex != NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }

    s_state_mutex = xSemaphoreCreateMutex();
    if (s_state_mutex == NULL)
    {
        return ESP_ERR_NO_MEM;
    }

    memset(s_device_name, 0, sizeof(s_device_name));
    memcpy(s_device_name, device_name, name_len);

    s_adv_interval_ms = adv_interval_ms;
    s_conn_handle = 0xFFFFU;
    s_tx_value_handle = 0U;
    s_mtu = default_mtu;
    s_own_addr_type = 0U;
    s_notify_enabled = false;
    s_rx_callback = NULL;
    s_state_callback = NULL;
    atomic_store(&s_connected, false);
    atomic_store(&s_initialized, false);

    if (s_adv_interval_ms == 0U)
    {
        s_adv_interval_ms = default_adv_interval_ms;
    }

    return ESP_OK;
}

void ble_nus_model_reset(uint16_t default_adv_interval_ms, uint16_t default_mtu)
{
    if (s_state_mutex != NULL)
    {
        vSemaphoreDelete(s_state_mutex);
        s_state_mutex = NULL;
    }

    s_conn_handle = 0xFFFFU;
    s_tx_value_handle = 0U;
    s_mtu = default_mtu;
    s_adv_interval_ms = default_adv_interval_ms;
    s_own_addr_type = 0U;
    s_notify_enabled = false;
    s_rx_callback = NULL;
    s_state_callback = NULL;
    memset(s_device_name, 0, sizeof(s_device_name));
    atomic_store(&s_connected, false);
    atomic_store(&s_initialized, false);
}

bool ble_nus_model_is_initialized(void)
{
    return atomic_load(&s_initialized);
}

void ble_nus_model_set_initialized(bool initialized)
{
    atomic_store(&s_initialized, initialized);
}

bool ble_nus_model_is_connected(void)
{
    return atomic_load(&s_connected);
}

const char *ble_nus_model_get_device_name(void)
{
    return s_device_name;
}

uint16_t ble_nus_model_get_adv_interval_ms(void)
{
    uint16_t value = 0U;
    if (s_state_mutex == NULL)
    {
        return value;
    }

    if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE)
    {
        value = s_adv_interval_ms;
        xSemaphoreGive(s_state_mutex);
    }

    return value;
}

void ble_nus_model_set_conn_state(bool connected, uint16_t conn_handle, uint16_t default_conn_handle,
                                  uint16_t default_mtu)
{
    if (s_state_mutex == NULL)
    {
        return;
    }

    if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE)
    {
        s_conn_handle = connected ? conn_handle : default_conn_handle;
        s_notify_enabled = false;
        s_mtu = default_mtu;
        atomic_store(&s_connected, connected);
        xSemaphoreGive(s_state_mutex);
    }
}

void ble_nus_model_set_notify_enabled(bool enabled)
{
    if (s_state_mutex == NULL)
    {
        return;
    }

    if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE)
    {
        s_notify_enabled = enabled;
        xSemaphoreGive(s_state_mutex);
    }
}

void ble_nus_model_set_mtu(uint16_t mtu)
{
    if (s_state_mutex == NULL)
    {
        return;
    }

    if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE)
    {
        s_mtu = mtu;
        xSemaphoreGive(s_state_mutex);
    }
}

void ble_nus_model_set_tx_value_handle(uint16_t value_handle)
{
    if (s_state_mutex == NULL)
    {
        return;
    }

    if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE)
    {
        s_tx_value_handle = value_handle;
        xSemaphoreGive(s_state_mutex);
    }
}

uint16_t ble_nus_model_get_tx_value_handle(void)
{
    uint16_t value = 0U;
    if (s_state_mutex == NULL)
    {
        return value;
    }

    if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE)
    {
        value = s_tx_value_handle;
        xSemaphoreGive(s_state_mutex);
    }

    return value;
}

void ble_nus_model_set_own_addr_type(uint8_t own_addr_type)
{
    if (s_state_mutex == NULL)
    {
        return;
    }

    if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE)
    {
        s_own_addr_type = own_addr_type;
        xSemaphoreGive(s_state_mutex);
    }
}

uint8_t ble_nus_model_get_own_addr_type(void)
{
    uint8_t value = 0U;
    if (s_state_mutex == NULL)
    {
        return value;
    }

    if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE)
    {
        value = s_own_addr_type;
        xSemaphoreGive(s_state_mutex);
    }

    return value;
}

void ble_nus_model_set_rx_callback(ble_nus_rx_cb_t callback)
{
    if (s_state_mutex == NULL)
    {
        return;
    }

    if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE)
    {
        s_rx_callback = callback;
        xSemaphoreGive(s_state_mutex);
    }
}

void ble_nus_model_set_state_callback(ble_nus_state_cb_t callback)
{
    if (s_state_mutex == NULL)
    {
        return;
    }

    if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE)
    {
        s_state_callback = callback;
        xSemaphoreGive(s_state_mutex);
    }
}

ble_nus_rx_cb_t ble_nus_model_get_rx_callback(void)
{
    ble_nus_rx_cb_t callback = NULL;
    if (s_state_mutex == NULL)
    {
        return callback;
    }

    if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE)
    {
        callback = s_rx_callback;
        xSemaphoreGive(s_state_mutex);
    }

    return callback;
}

ble_nus_state_cb_t ble_nus_model_get_state_callback(void)
{
    ble_nus_state_cb_t callback = NULL;
    if (s_state_mutex == NULL)
    {
        return callback;
    }

    if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE)
    {
        callback = s_state_callback;
        xSemaphoreGive(s_state_mutex);
    }

    return callback;
}

bool ble_nus_model_get_snapshot(ble_nus_model_snapshot_t *snapshot)
{
    if (snapshot == NULL || s_state_mutex == NULL)
    {
        return false;
    }

    if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) != pdTRUE)
    {
        return false;
    }

    snapshot->initialized = atomic_load(&s_initialized);
    snapshot->connected = atomic_load(&s_connected);
    snapshot->notify_enabled = s_notify_enabled;
    snapshot->conn_handle = s_conn_handle;
    snapshot->tx_value_handle = s_tx_value_handle;
    snapshot->mtu = s_mtu;
    snapshot->adv_interval_ms = s_adv_interval_ms;
    snapshot->own_addr_type = s_own_addr_type;
    snapshot->rx_callback = s_rx_callback;
    snapshot->state_callback = s_state_callback;

    xSemaphoreGive(s_state_mutex);
    return true;
}
