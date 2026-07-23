#ifndef TEST_STUB_TUYA_COMMON_H
#define TEST_STUB_TUYA_COMMON_H

#include <stddef.h>
#include <stdint.h>

typedef void *THREAD_HANDLE;
typedef void (*TAL_LOG_OUTPUT_CB)(void);

typedef struct {
    uint32_t stackDepth;
    int priority;
    const char *thrdname;
} THREAD_CFG_T;

#define TAL_LOG_LEVEL_DEBUG 0
#define THREAD_PRIO_1 1
#define OPERATING_SYSTEM 0
#define SYSTEM_LINUX 1

#define PROJECT_NAME "mob_display"
#define PROJECT_VERSION "0.1.0"
#define OPEN_VERSION "test"
#define OPEN_COMMIT "test"
#define PLATFORM_CHIP "T5AI"
#define PLATFORM_BOARD "TUYA_T5AI_BOARD"
#define PLATFORM_COMMIT "test"
#define DISPLAY_NAME "lcd"

#define PR_NOTICE(...) do { } while (0)

void tkl_log_output(void);
void tal_log_init(int level, int buffer_size, TAL_LOG_OUTPUT_CB output);
void tal_system_sleep(uint32_t milliseconds);
void tal_thread_delete(THREAD_HANDLE thread);
int tal_thread_create_and_start(
    THREAD_HANDLE *thread,
    void *stack,
    void *stack_end,
    void (*entry)(void *),
    void *argument,
    THREAD_CFG_T *configuration
);

void board_register_hardware(void);
void lv_vendor_init(const char *display_name);
void lv_vendor_disp_lock(void);
void lv_vendor_disp_unlock(void);
void lv_vendor_start(uint32_t period_ms, uint32_t stack_size);

#endif /* TEST_STUB_TUYA_COMMON_H */
