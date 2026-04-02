#include "ble_nus_model.h"

#include <stdatomic.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "host/ble_att.h"

typedef struct ble_nus_model_context_s
{
    SemaphoreHandle_t state_mutex;
    _Atomic bool initialized;
    _Atomic bool connected;
    uint16_t conn_handle;
    uint16_t tx_value_handle;
    uint16_t mtu;
    uint16_t adv_interval_ms;
    uint8_t own_addr_type;
    bool notify_enabled;
    ble_nus_rx_cb_t rx_callback;
    ble_nus_state_cb_t state_callback;
    char device_name[21];
} ble_nus_model_context_t;

static ble_nus_model_context_t s_self;

esp_err_t ble_nus_model_init(const char *device_name, uint16_t adv_interval_ms, uint16_t default_adv_interval_ms,
                             uint16_t default_mtu, size_t max_device_name_len)
{
    if (!device_name)
        return ESP_ERR_INVALID_ARG;

    size_t name_len = strlen(device_name);
    if (!name_len || name_len > max_device_name_len)
        return ESP_ERR_INVALID_ARG;

    if (s_self.state_mutex)
        return ESP_ERR_INVALID_STATE;

    s_self.state_mutex = xSemaphoreCreateMutex();
    if (!s_self.state_mutex)
        return ESP_ERR_NO_MEM;

    memset(s_self.device_name, 0, sizeof(s_self.device_name));
    memcpy(s_self.device_name, device_name, name_len);

    s_self.adv_interval_ms = adv_interval_ms ? adv_interval_ms : default_adv_interval_ms;
    s_self.conn_handle = 0xFFFFU;
    s_self.tx_value_handle = 0U;
    s_self.mtu = default_mtu;
    s_self.own_addr_type = 0U;
    s_self.notify_enabled = false;
    s_self.rx_callback = NULL;
    s_self.state_callback = NULL;
    atomic_store(&s_self.connected, false);
    atomic_store(&s_self.initialized, false);

    return ESP_OK;
}

void ble_nus_model_reset(uint16_t default_adv_interval_ms, uint16_t default_mtu)
{
    if (s_self.state_mutex)
    {
        vSemaphoreDelete(s_self.state_mutex);
        s_self.state_mutex = NULL;
    }

    s_self.conn_handle = 0xFFFFU;
    s_self.tx_value_handle = 0U;
    s_self.mtu = default_mtu;
    s_self.adv_interval_ms = default_adv_interval_ms;
    s_self.own_addr_type = 0U;
    s_self.notify_enabled = false;
    s_self.rx_callback = NULL;
    s_self.state_callback = NULL;
    memset(s_self.device_name, 0, sizeof(s_self.device_name));
    atomic_store(&s_self.connected, false);
    atomic_store(&s_self.initialized, false);
}

bool ble_nus_model_is_initialized(void)
{
    return atomic_load(&s_self.initialized);
}

void ble_nus_model_set_initialized(bool initialized)
{
    atomic_store(&s_self.initialized, initialized);
}

bool ble_nus_model_is_connected(void)
{
    return atomic_load(&s_self.connected);
}

const char *ble_nus_model_get_device_name(void)
{
    return s_self.device_name;
}

uint16_t ble_nus_model_get_adv_interval_ms(void)
{
    uint16_t value = 0U;
    if (!s_self.state_mutex)
        return value;

    if (pdTRUE == xSemaphoreTake(s_self.state_mutex, portMAX_DELAY))
    {
        value = s_self.adv_interval_ms;
        xSemaphoreGive(s_self.state_mutex);
    }

    return value;
}

void ble_nus_model_set_conn_state(bool connected, uint16_t conn_handle, uint16_t default_conn_handle,
                                  uint16_t default_mtu)
{
    if (!s_self.state_mutex)
        return;

    if (pdTRUE == xSemaphoreTake(s_self.state_mutex, portMAX_DELAY))
    {
        s_self.conn_handle = connected ? conn_handle : default_conn_handle;
        s_self.notify_enabled = false;
        s_self.mtu = default_mtu;
        atomic_store(&s_self.connected, connected);
        xSemaphoreGive(s_self.state_mutex);
    }
}

void ble_nus_model_set_notify_enabled(bool enabled)
{
    if (!s_self.state_mutex)
        return;

    if (pdTRUE == xSemaphoreTake(s_self.state_mutex, portMAX_DELAY))
    {
        s_self.notify_enabled = enabled;
        xSemaphoreGive(s_self.state_mutex);
    }
}

void ble_nus_model_set_mtu(uint16_t mtu)
{
    if (!s_self.state_mutex)
        return;

    if (pdTRUE == xSemaphoreTake(s_self.state_mutex, portMAX_DELAY))
    {
        s_self.mtu = mtu;
        xSemaphoreGive(s_self.state_mutex);
    }
}

void ble_nus_model_set_tx_value_handle(uint16_t value_handle)
{
    if (!s_self.state_mutex)
        return;

    if (pdTRUE == xSemaphoreTake(s_self.state_mutex, portMAX_DELAY))
    {
        s_self.tx_value_handle = value_handle;
        xSemaphoreGive(s_self.state_mutex);
    }
}

uint16_t ble_nus_model_get_tx_value_handle(void)
{
    uint16_t value = 0U;
    if (!s_self.state_mutex)
        return value;

    if (pdTRUE == xSemaphoreTake(s_self.state_mutex, portMAX_DELAY))
    {
        value = s_self.tx_value_handle;
        xSemaphoreGive(s_self.state_mutex);
    }

    return value;
}

void ble_nus_model_set_own_addr_type(uint8_t own_addr_type)
{
    if (!s_self.state_mutex)
        return;

    if (pdTRUE == xSemaphoreTake(s_self.state_mutex, portMAX_DELAY))
    {
        s_self.own_addr_type = own_addr_type;
        xSemaphoreGive(s_self.state_mutex);
    }
}

uint8_t ble_nus_model_get_own_addr_type(void)
{
    uint8_t value = 0U;
    if (!s_self.state_mutex)
        return value;

    if (pdTRUE == xSemaphoreTake(s_self.state_mutex, portMAX_DELAY))
    {
        value = s_self.own_addr_type;
        xSemaphoreGive(s_self.state_mutex);
    }

    return value;
}

void ble_nus_model_set_rx_callback(ble_nus_rx_cb_t callback)
{
    if (!s_self.state_mutex)
        return;

    if (pdTRUE == xSemaphoreTake(s_self.state_mutex, portMAX_DELAY))
    {
        s_self.rx_callback = callback;
        xSemaphoreGive(s_self.state_mutex);
    }
}

void ble_nus_model_set_state_callback(ble_nus_state_cb_t callback)
{
    if (!s_self.state_mutex)
        return;

    if (pdTRUE == xSemaphoreTake(s_self.state_mutex, portMAX_DELAY))
    {
        s_self.state_callback = callback;
        xSemaphoreGive(s_self.state_mutex);
    }
}

ble_nus_rx_cb_t ble_nus_model_get_rx_callback(void)
{
    ble_nus_rx_cb_t callback = NULL;
    if (!s_self.state_mutex)
        return callback;

    if (pdTRUE == xSemaphoreTake(s_self.state_mutex, portMAX_DELAY))
    {
        callback = s_self.rx_callback;
        xSemaphoreGive(s_self.state_mutex);
    }

    return callback;
}

ble_nus_state_cb_t ble_nus_model_get_state_callback(void)
{
    ble_nus_state_cb_t callback = NULL;
    if (!s_self.state_mutex)
        return callback;

    if (pdTRUE == xSemaphoreTake(s_self.state_mutex, portMAX_DELAY))
    {
        callback = s_self.state_callback;
        xSemaphoreGive(s_self.state_mutex);
    }

    return callback;
}

bool ble_nus_model_get_snapshot(ble_nus_model_snapshot_t *snapshot)
{
    if (!snapshot || !s_self.state_mutex)
        return false;

    if (pdTRUE != xSemaphoreTake(s_self.state_mutex, portMAX_DELAY))
        return false;

    snapshot->initialized = atomic_load(&s_self.initialized);
    snapshot->connected = atomic_load(&s_self.connected);
    snapshot->notify_enabled = s_self.notify_enabled;
    snapshot->conn_handle = s_self.conn_handle;
    snapshot->tx_value_handle = s_self.tx_value_handle;
    snapshot->mtu = s_self.mtu;
    snapshot->adv_interval_ms = s_self.adv_interval_ms;
    snapshot->own_addr_type = s_self.own_addr_type;
    snapshot->rx_callback = s_self.rx_callback;
    snapshot->state_callback = s_self.state_callback;

    xSemaphoreGive(s_self.state_mutex);
    return true;
}
