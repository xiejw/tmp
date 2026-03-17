#pragma once

#include <stdarg.h>

/* Start a spinner with an optional status message (printf-style). */
void forge_status_open(const char *fmt, ...);

/* Log a line above the spinner (printf-style). */
void forge_status_printf(const char *fmt, ...);

/* Stop the spinner and clean up. */
void forge_status_close(void);
