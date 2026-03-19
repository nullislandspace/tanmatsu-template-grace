// WARNING: This is a generated file, do not edit it!
// clang-format off

#include <kbelf.h>

// Weak references: symbols provided by ROM or graceloader resolve to their
// address. Symbols not present resolve to NULL (zero cost, no code pulled in).
extern char const __attribute__((weak)) symbol_sched_yield[] asm("sched_yield");
extern char const __attribute__((weak)) symbol_sched_get_priority_min[] asm("sched_get_priority_min");
extern char const __attribute__((weak)) symbol_sched_get_priority_max[] asm("sched_get_priority_max");
extern char const __attribute__((weak)) symbol_pthread_mutexattr_init[] asm("pthread_mutexattr_init");
extern char const __attribute__((weak)) symbol_pthread_mutexattr_destroy[] asm("pthread_mutexattr_destroy");
extern char const __attribute__((weak)) symbol_pthread_mutexattr_gettype[] asm("pthread_mutexattr_gettype");
extern char const __attribute__((weak)) symbol_pthread_mutexattr_settype[] asm("pthread_mutexattr_settype");
extern char const __attribute__((weak)) symbol_pthread_mutex_init[] asm("pthread_mutex_init");
extern char const __attribute__((weak)) symbol_pthread_mutex_destroy[] asm("pthread_mutex_destroy");
extern char const __attribute__((weak)) symbol_pthread_mutex_lock[] asm("pthread_mutex_lock");
extern char const __attribute__((weak)) symbol_pthread_mutex_trylock[] asm("pthread_mutex_trylock");
extern char const __attribute__((weak)) symbol_pthread_mutex_unlock[] asm("pthread_mutex_unlock");
extern char const __attribute__((weak)) symbol_pthread_mutex_timedlock[] asm("pthread_mutex_timedlock");
extern char const __attribute__((weak)) symbol_pthread_condattr_init[] asm("pthread_condattr_init");
extern char const __attribute__((weak)) symbol_pthread_condattr_destroy[] asm("pthread_condattr_destroy");
extern char const __attribute__((weak)) symbol_pthread_condattr_getclock[] asm("pthread_condattr_getclock");
extern char const __attribute__((weak)) symbol_pthread_condattr_setclock[] asm("pthread_condattr_setclock");
extern char const __attribute__((weak)) symbol_pthread_condattr_getpshared[] asm("pthread_condattr_getpshared");
extern char const __attribute__((weak)) symbol_pthread_condattr_setpshared[] asm("pthread_condattr_setpshared");
extern char const __attribute__((weak)) symbol_pthread_cond_init[] asm("pthread_cond_init");
extern char const __attribute__((weak)) symbol_pthread_cond_destroy[] asm("pthread_cond_destroy");
extern char const __attribute__((weak)) symbol_pthread_cond_signal[] asm("pthread_cond_signal");
extern char const __attribute__((weak)) symbol_pthread_cond_broadcast[] asm("pthread_cond_broadcast");
extern char const __attribute__((weak)) symbol_pthread_cond_wait[] asm("pthread_cond_wait");
extern char const __attribute__((weak)) symbol_pthread_cond_timedwait[] asm("pthread_cond_timedwait");
extern char const __attribute__((weak)) symbol_pthread_attr_init[] asm("pthread_attr_init");
extern char const __attribute__((weak)) symbol_pthread_attr_destroy[] asm("pthread_attr_destroy");
extern char const __attribute__((weak)) symbol_pthread_attr_getstacksize[] asm("pthread_attr_getstacksize");
extern char const __attribute__((weak)) symbol_pthread_attr_setstacksize[] asm("pthread_attr_setstacksize");
extern char const __attribute__((weak)) symbol_pthread_attr_getdetachstate[] asm("pthread_attr_getdetachstate");
extern char const __attribute__((weak)) symbol_pthread_attr_setdetachstate[] asm("pthread_attr_setdetachstate");
extern char const __attribute__((weak)) symbol_pthread_getschedparam[] asm("pthread_getschedparam");
extern char const __attribute__((weak)) symbol_pthread_setschedparam[] asm("pthread_setschedparam");
extern char const __attribute__((weak)) symbol_pthread_create[] asm("pthread_create");
extern char const __attribute__((weak)) symbol_pthread_join[] asm("pthread_join");
extern char const __attribute__((weak)) symbol_pthread_detach[] asm("pthread_detach");
extern char const __attribute__((weak)) symbol_pthread_exit[] asm("pthread_exit");
extern char const __attribute__((weak)) symbol_pthread_self[] asm("pthread_self");
extern char const __attribute__((weak)) symbol_pthread_equal[] asm("pthread_equal");
extern char const __attribute__((weak)) symbol_pthread_once[] asm("pthread_once");
extern char const __attribute__((weak)) symbol_pthread_key_create[] asm("pthread_key_create");
extern char const __attribute__((weak)) symbol_pthread_key_delete[] asm("pthread_key_delete");
extern char const __attribute__((weak)) symbol_pthread_getspecific[] asm("pthread_getspecific");
extern char const __attribute__((weak)) symbol_pthread_setspecific[] asm("pthread_setspecific");
extern char const __attribute__((weak)) symbol_pthread_cancel[] asm("pthread_cancel");
extern char const __attribute__((weak)) symbol_pthread_setcancelstate[] asm("pthread_setcancelstate");
extern char const __attribute__((weak)) symbol_pthread_rwlock_rdlock[] asm("pthread_rwlock_rdlock");
extern char const __attribute__((weak)) symbol_pthread_rwlock_tryrdlock[] asm("pthread_rwlock_tryrdlock");
extern char const __attribute__((weak)) symbol_pthread_rwlock_unlock[] asm("pthread_rwlock_unlock");
extern char const __attribute__((weak)) symbol_pthread_rwlock_wrlock[] asm("pthread_rwlock_wrlock");
extern char const __attribute__((weak)) symbol_pthread_rwlock_trywrlock[] asm("pthread_rwlock_trywrlock");

static kbelf_builtin_sym const symbols[] = {
    { .name = "sched_yield", .vaddr = (size_t) symbol_sched_yield },
    { .name = "sched_get_priority_min", .vaddr = (size_t) symbol_sched_get_priority_min },
    { .name = "sched_get_priority_max", .vaddr = (size_t) symbol_sched_get_priority_max },
    { .name = "pthread_mutexattr_init", .vaddr = (size_t) symbol_pthread_mutexattr_init },
    { .name = "pthread_mutexattr_destroy", .vaddr = (size_t) symbol_pthread_mutexattr_destroy },
    { .name = "pthread_mutexattr_gettype", .vaddr = (size_t) symbol_pthread_mutexattr_gettype },
    { .name = "pthread_mutexattr_settype", .vaddr = (size_t) symbol_pthread_mutexattr_settype },
    { .name = "pthread_mutex_init", .vaddr = (size_t) symbol_pthread_mutex_init },
    { .name = "pthread_mutex_destroy", .vaddr = (size_t) symbol_pthread_mutex_destroy },
    { .name = "pthread_mutex_lock", .vaddr = (size_t) symbol_pthread_mutex_lock },
    { .name = "pthread_mutex_trylock", .vaddr = (size_t) symbol_pthread_mutex_trylock },
    { .name = "pthread_mutex_unlock", .vaddr = (size_t) symbol_pthread_mutex_unlock },
    { .name = "pthread_mutex_timedlock", .vaddr = (size_t) symbol_pthread_mutex_timedlock },
    { .name = "pthread_condattr_init", .vaddr = (size_t) symbol_pthread_condattr_init },
    { .name = "pthread_condattr_destroy", .vaddr = (size_t) symbol_pthread_condattr_destroy },
    { .name = "pthread_condattr_getclock", .vaddr = (size_t) symbol_pthread_condattr_getclock },
    { .name = "pthread_condattr_setclock", .vaddr = (size_t) symbol_pthread_condattr_setclock },
    { .name = "pthread_condattr_getpshared", .vaddr = (size_t) symbol_pthread_condattr_getpshared },
    { .name = "pthread_condattr_setpshared", .vaddr = (size_t) symbol_pthread_condattr_setpshared },
    { .name = "pthread_cond_init", .vaddr = (size_t) symbol_pthread_cond_init },
    { .name = "pthread_cond_destroy", .vaddr = (size_t) symbol_pthread_cond_destroy },
    { .name = "pthread_cond_signal", .vaddr = (size_t) symbol_pthread_cond_signal },
    { .name = "pthread_cond_broadcast", .vaddr = (size_t) symbol_pthread_cond_broadcast },
    { .name = "pthread_cond_wait", .vaddr = (size_t) symbol_pthread_cond_wait },
    { .name = "pthread_cond_timedwait", .vaddr = (size_t) symbol_pthread_cond_timedwait },
    { .name = "pthread_attr_init", .vaddr = (size_t) symbol_pthread_attr_init },
    { .name = "pthread_attr_destroy", .vaddr = (size_t) symbol_pthread_attr_destroy },
    { .name = "pthread_attr_getstacksize", .vaddr = (size_t) symbol_pthread_attr_getstacksize },
    { .name = "pthread_attr_setstacksize", .vaddr = (size_t) symbol_pthread_attr_setstacksize },
    { .name = "pthread_attr_getdetachstate", .vaddr = (size_t) symbol_pthread_attr_getdetachstate },
    { .name = "pthread_attr_setdetachstate", .vaddr = (size_t) symbol_pthread_attr_setdetachstate },
    { .name = "pthread_getschedparam", .vaddr = (size_t) symbol_pthread_getschedparam },
    { .name = "pthread_setschedparam", .vaddr = (size_t) symbol_pthread_setschedparam },
    { .name = "pthread_create", .vaddr = (size_t) symbol_pthread_create },
    { .name = "pthread_join", .vaddr = (size_t) symbol_pthread_join },
    { .name = "pthread_detach", .vaddr = (size_t) symbol_pthread_detach },
    { .name = "pthread_exit", .vaddr = (size_t) symbol_pthread_exit },
    { .name = "pthread_self", .vaddr = (size_t) symbol_pthread_self },
    { .name = "pthread_equal", .vaddr = (size_t) symbol_pthread_equal },
    { .name = "pthread_once", .vaddr = (size_t) symbol_pthread_once },
    { .name = "pthread_key_create", .vaddr = (size_t) symbol_pthread_key_create },
    { .name = "pthread_key_delete", .vaddr = (size_t) symbol_pthread_key_delete },
    { .name = "pthread_getspecific", .vaddr = (size_t) symbol_pthread_getspecific },
    { .name = "pthread_setspecific", .vaddr = (size_t) symbol_pthread_setspecific },
    { .name = "pthread_cancel", .vaddr = (size_t) symbol_pthread_cancel },
    { .name = "pthread_setcancelstate", .vaddr = (size_t) symbol_pthread_setcancelstate },
    { .name = "pthread_rwlock_rdlock", .vaddr = (size_t) symbol_pthread_rwlock_rdlock },
    { .name = "pthread_rwlock_tryrdlock", .vaddr = (size_t) symbol_pthread_rwlock_tryrdlock },
    { .name = "pthread_rwlock_unlock", .vaddr = (size_t) symbol_pthread_rwlock_unlock },
    { .name = "pthread_rwlock_wrlock", .vaddr = (size_t) symbol_pthread_rwlock_wrlock },
    { .name = "pthread_rwlock_trywrlock", .vaddr = (size_t) symbol_pthread_rwlock_trywrlock },
};

kbelf_builtin_lib const app_elf_lib_pthread = {
    .path        = "libpthread.so",
    .symbols_len = 51,
    .symbols     = symbols,
};
