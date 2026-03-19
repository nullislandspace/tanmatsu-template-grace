// WARNING: This is a generated file, do not edit it!
// clang-format off

#include <kbelf.h>

extern char const symbol_pax_info_png_fd[] asm("pax_info_png_fd");
extern char const symbol_pax_info_png_buf[] asm("pax_info_png_buf");
extern char const symbol_pax_decode_png_fd[] asm("pax_decode_png_fd");
extern char const symbol_pax_decode_png_buf[] asm("pax_decode_png_buf");
extern char const symbol_pax_insert_png_fd[] asm("pax_insert_png_fd");
extern char const symbol_pax_insert_png_buf[] asm("pax_insert_png_buf");

static kbelf_builtin_sym const symbols[] = {
    { .name = "pax_info_png_fd", .vaddr = (size_t) symbol_pax_info_png_fd },
    { .name = "pax_info_png_buf", .vaddr = (size_t) symbol_pax_info_png_buf },
    { .name = "pax_decode_png_fd", .vaddr = (size_t) symbol_pax_decode_png_fd },
    { .name = "pax_decode_png_buf", .vaddr = (size_t) symbol_pax_decode_png_buf },
    { .name = "pax_insert_png_fd", .vaddr = (size_t) symbol_pax_insert_png_fd },
    { .name = "pax_insert_png_buf", .vaddr = (size_t) symbol_pax_insert_png_buf },
};

kbelf_builtin_lib const app_elf_lib_pax_codecs = {
    .path        = "libpax-codecs.so",
    .symbols_len = 6,
    .symbols     = symbols,
};
