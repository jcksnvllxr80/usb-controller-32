#ifndef USB_DEBUG_H
#define USB_DEBUG_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Compact ISR-safe ring buffer for USB debug tracing.
 * Call usb_debug_put() from ISR or task context.
 * Call usb_debug_drain() from task context to print via UART. */

#define USB_DBG_MSG_LEN  60
#define USB_DBG_RING_SIZE 48

typedef struct {
    char msg[USB_DBG_MSG_LEN];
} usb_dbg_entry_t;

extern volatile usb_dbg_entry_t usb_dbg_ring[USB_DBG_RING_SIZE];
extern volatile uint32_t usb_dbg_head;
extern volatile uint32_t usb_dbg_tail;
extern volatile bool     usb_dbg_on;

/* Push a pre-formatted message (ISR-safe, lock-free single-producer) */
void usb_debug_put(const char *msg);

/* Push with printf-style formatting (ISR-safe, truncates at USB_DBG_MSG_LEN) */
void usb_debug_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

/* Drain all buffered messages to UART — call from task context only */
void usb_debug_drain(void);

/* Enable / disable logging at runtime */
static inline void usb_debug_enable(bool on) { usb_dbg_on = on; }
static inline bool usb_debug_enabled(void)   { return usb_dbg_on; }

#ifdef __cplusplus
}
#endif

#endif /* USB_DEBUG_H */
