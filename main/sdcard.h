#pragma once

#include "esp_err.h"

typedef enum {
    SD_STATUS_NOT_PRESENT,
    SD_STATUS_OK,
    SD_STATUS_ERROR,
} sd_status_t;

esp_err_t   sd_mount(void);
sd_status_t sd_status(void);
