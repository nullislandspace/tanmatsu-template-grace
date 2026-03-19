// WARNING: This is a generated file, do not edit it!
// clang-format off

#include <kbelf.h>

// Weak references: symbols provided by ROM or graceloader resolve to their
// address. Symbols not present resolve to NULL (zero cost, no code pulled in).
extern char const __attribute__((weak)) symbol_esp_log_write[] asm("esp_log_write");
extern char const __attribute__((weak)) symbol_esp_log_timestamp[] asm("esp_log_timestamp");
extern char const __attribute__((weak)) symbol_esp_err_to_name[] asm("esp_err_to_name");

static kbelf_builtin_sym const symbols[] = {
    { .name = "esp_log_write", .vaddr = (size_t) symbol_esp_log_write },
    { .name = "esp_log_timestamp", .vaddr = (size_t) symbol_esp_log_timestamp },
    { .name = "esp_err_to_name", .vaddr = (size_t) symbol_esp_err_to_name },
};

kbelf_builtin_lib const app_elf_lib_esp_idf = {
    .path        = "libesp-idf.so",
    .symbols_len = 3,
    .symbols     = symbols,
};
