#include "status.h"

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MSG_MAX 512

static const char *FRAMES[] = {
    "⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏",
};
#define NFRAMES (int)(sizeof(FRAMES) / sizeof(FRAMES[0]))

static pthread_t    s_thread;
static pthread_mutex_t s_mu = PTHREAD_MUTEX_INITIALIZER;
static char         s_msg[MSG_MAX];
static volatile int s_running;

/* Print current spinner frame. Caller must NOT hold s_mu. */
static void draw_spinner(int frame)
{
    pthread_mutex_lock(&s_mu);
    fprintf(stdout, "\r\033[2K\033[32m%s\033[0m %s", FRAMES[frame], s_msg);
    fflush(stdout);
    pthread_mutex_unlock(&s_mu);
}

static void *spinner_thread(void *arg)
{
    (void)arg;
    int frame = 0;
    while (s_running) {
        draw_spinner(frame);
        frame = (frame + 1) % NFRAMES;
        usleep(100 * 1000); /* 100 ms */
    }
    return NULL;
}

void forge_status_open(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    pthread_mutex_lock(&s_mu);
    vsnprintf(s_msg, MSG_MAX, fmt, ap);
    pthread_mutex_unlock(&s_mu);
    va_end(ap);

    s_running = 1;
    pthread_create(&s_thread, NULL, spinner_thread, NULL);
}

void forge_status_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    pthread_mutex_lock(&s_mu);
    /* Erase spinner line, print the log line, then redraw spinner below. */
    fprintf(stdout, "\r\033[2K");
    vfprintf(stdout, fmt, ap);
    /* Ensure line ends with newline so spinner renders on a fresh line. */
    const char *str_end = fmt + strlen(fmt);
    if (str_end == fmt || *(str_end - 1) != '\n')
        fputc('\n', stdout);
    fflush(stdout);
    pthread_mutex_unlock(&s_mu);

    va_end(ap);
}

void forge_status_close(void)
{
    s_running = 0;
    pthread_join(s_thread, NULL);
    /* Erase spinner line. */
    fprintf(stdout, "\r\033[2K");
    fflush(stdout);
}
