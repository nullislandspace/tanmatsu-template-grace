// Graceloader app entry point glue
// kbelf calls _start as the ELF entry point, which calls app_main()

extern void app_main(void);

void _start(int argc, char const** argv, char const** envp) {
    (void)argc;
    (void)argv;
    (void)envp;
    app_main();
}
