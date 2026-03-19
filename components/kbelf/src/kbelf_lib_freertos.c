// WARNING: This is a generated file, do not edit it!
// clang-format off

#include <kbelf.h>

extern char const symbol_xTaskCreatePinnedToCore[] asm("xTaskCreatePinnedToCore");
extern char const symbol_xTaskCreateStaticPinnedToCore[] asm("xTaskCreateStaticPinnedToCore");
extern char const symbol_vTaskDelete[] asm("vTaskDelete");
extern char const symbol_vTaskDelay[] asm("vTaskDelay");
extern char const symbol_vTaskDelayUntil[] asm("vTaskDelayUntil");
extern char const symbol_xTaskGetCurrentTaskHandle[] asm("xTaskGetCurrentTaskHandle");
extern char const symbol_xQueueGenericCreate[] asm("xQueueGenericCreate");
extern char const symbol_xQueueGenericSend[] asm("xQueueGenericSend");
extern char const symbol_xQueueReceive[] asm("xQueueReceive");
extern char const symbol_xQueuePeek[] asm("xQueuePeek");
extern char const symbol_uxQueueMessagesWaiting[] asm("uxQueueMessagesWaiting");
extern char const symbol_vQueueDelete[] asm("vQueueDelete");
extern char const symbol_xQueueCreateMutex[] asm("xQueueCreateMutex");
extern char const symbol_xQueueSemaphoreTake[] asm("xQueueSemaphoreTake");
extern char const symbol_xQueueGiveFromISR[] asm("xQueueGiveFromISR");
extern char const symbol_xEventGroupCreate[] asm("xEventGroupCreate");
extern char const symbol_xEventGroupSetBits[] asm("xEventGroupSetBits");
extern char const symbol_xEventGroupWaitBits[] asm("xEventGroupWaitBits");
extern char const symbol_xEventGroupClearBits[] asm("xEventGroupClearBits");
extern char const symbol_vEventGroupDelete[] asm("vEventGroupDelete");
extern char const symbol_xTaskResumeAll[] asm("xTaskResumeAll");
extern char const symbol_vTaskSuspendAll[] asm("vTaskSuspendAll");
extern char const symbol_xTaskGetTickCount[] asm("xTaskGetTickCount");
extern char const symbol_xTaskGetTickCountFromISR[] asm("xTaskGetTickCountFromISR");

static kbelf_builtin_sym const symbols[] = {
    { .name = "xTaskCreatePinnedToCore", .vaddr = (size_t) symbol_xTaskCreatePinnedToCore },
    { .name = "xTaskCreateStaticPinnedToCore", .vaddr = (size_t) symbol_xTaskCreateStaticPinnedToCore },
    { .name = "vTaskDelete", .vaddr = (size_t) symbol_vTaskDelete },
    { .name = "vTaskDelay", .vaddr = (size_t) symbol_vTaskDelay },
    { .name = "vTaskDelayUntil", .vaddr = (size_t) symbol_vTaskDelayUntil },
    { .name = "xTaskGetCurrentTaskHandle", .vaddr = (size_t) symbol_xTaskGetCurrentTaskHandle },
    { .name = "xQueueGenericCreate", .vaddr = (size_t) symbol_xQueueGenericCreate },
    { .name = "xQueueGenericSend", .vaddr = (size_t) symbol_xQueueGenericSend },
    { .name = "xQueueReceive", .vaddr = (size_t) symbol_xQueueReceive },
    { .name = "xQueuePeek", .vaddr = (size_t) symbol_xQueuePeek },
    { .name = "uxQueueMessagesWaiting", .vaddr = (size_t) symbol_uxQueueMessagesWaiting },
    { .name = "vQueueDelete", .vaddr = (size_t) symbol_vQueueDelete },
    { .name = "xQueueCreateMutex", .vaddr = (size_t) symbol_xQueueCreateMutex },
    { .name = "xQueueSemaphoreTake", .vaddr = (size_t) symbol_xQueueSemaphoreTake },
    { .name = "xQueueGiveFromISR", .vaddr = (size_t) symbol_xQueueGiveFromISR },
    { .name = "xEventGroupCreate", .vaddr = (size_t) symbol_xEventGroupCreate },
    { .name = "xEventGroupSetBits", .vaddr = (size_t) symbol_xEventGroupSetBits },
    { .name = "xEventGroupWaitBits", .vaddr = (size_t) symbol_xEventGroupWaitBits },
    { .name = "xEventGroupClearBits", .vaddr = (size_t) symbol_xEventGroupClearBits },
    { .name = "vEventGroupDelete", .vaddr = (size_t) symbol_vEventGroupDelete },
    { .name = "xTaskResumeAll", .vaddr = (size_t) symbol_xTaskResumeAll },
    { .name = "vTaskSuspendAll", .vaddr = (size_t) symbol_vTaskSuspendAll },
    { .name = "xTaskGetTickCount", .vaddr = (size_t) symbol_xTaskGetTickCount },
    { .name = "xTaskGetTickCountFromISR", .vaddr = (size_t) symbol_xTaskGetTickCountFromISR },
};

kbelf_builtin_lib const app_elf_lib_freertos = {
    .path        = "libfreertos.so",
    .symbols_len = 24,
    .symbols     = symbols,
};
