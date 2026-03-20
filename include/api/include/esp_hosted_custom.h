#pragma once

#include <stdint.h>
#include "esp_err.h"

typedef void esp_hosted_custom_recv_cb_t(uint8_t type, uint8_t* payload, uint16_t payload_length);
typedef void esp_hosted_firwmare_version_cb_t(uint32_t version);

esp_err_t esp_hosted_send_custom(uint8_t type, uint8_t* payload, uint16_t payload_length);
esp_err_t esp_hosted_set_custom_callback(esp_hosted_custom_recv_cb_t callback);
esp_err_t esp_hosted_set_firmware_version_callback(esp_hosted_firwmare_version_cb_t* callback);
