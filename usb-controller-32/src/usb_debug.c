#include "usb_debug.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* Ring buffer storage */
volatile usb_dbg_entry_t usb_dbg_ring[USB_DBG_RING_SIZE];
volatile uint32_t usb_dbg_head = 0;   /* next write slot (producer) */
volatile uint32_t usb_dbg_tail = 0;   /* next read slot  (consumer) */
volatile bool     usb_dbg_on  = false;

/* Declared in logger.h / logger.cpp — C-callable UART write */
extern void logger_write_line(const char *line);

void usb_debug_put(const char *msg)
{
    if (!usb_dbg_on) return;

    uint32_t h = usb_dbg_head;
    uint32_t idx = h % USB_DBG_RING_SIZE;

    /* Copy message, truncate if needed */
    strncpy((char *)usb_dbg_ring[idx].msg, msg, USB_DBG_MSG_LEN - 1);
    ((char *)usb_dbg_ring[idx].msg)[USB_DBG_MSG_LEN - 1] = '\0';

    /* Advance head — if ring is full, oldest entry is silently dropped */
    usb_dbg_head = h + 1;
}

void usb_debug_printf(const char *fmt, ...)
{
    if (!usb_dbg_on) return;

    char buf[USB_DBG_MSG_LEN];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    usb_debug_put(buf);
}

void usb_debug_drain(void)
{
    /* Drain up to 8 messages per call to avoid starving other tasks */
    int count = 0;
    while (usb_dbg_tail != usb_dbg_head && count < 8) {
        uint32_t idx = usb_dbg_tail % USB_DBG_RING_SIZE;
        logger_write_line((const char *)usb_dbg_ring[idx].msg);
        usb_dbg_tail++;
        count++;
    }
}
