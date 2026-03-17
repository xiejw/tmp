#include "progress.h"

#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

static int64_t         s_total;
static char            s_title[PROGRESS_TITLE_MAX + 1];
static struct timespec s_start;

static int term_width(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0)
        return ws.ws_col;
    return 80;
}

static double elapsed_s(void)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (double)(now.tv_sec - s_start.tv_sec)
         + (double)(now.tv_nsec - s_start.tv_nsec) * 1e-9;
}

/* Format ETA into buf: "Xs", "Xm", or "Xh". */
static void fmt_eta(double s, char *buf, int len)
{
    if (s < 60.0)
        snprintf(buf, len, "%.0fs", s);
    else if (s < 3600.0)
        snprintf(buf, len, "%.0fm", s / 60.0);
    else
        snprintf(buf, len, "%.0fh", s / 3600.0);
}

void forge_progress_bar_open(int64_t total, const char *title)
{
    s_total = total;
    snprintf(s_title, sizeof(s_title), "%s", title);
    clock_gettime(CLOCK_MONOTONIC, &s_start);
}

void forge_progress_bar_advance(int64_t current)
{
    int    width   = term_width();
    double elapsed = elapsed_s();
    double pct     = (s_total > 0) ? (double)current / (double)s_total : 0.0;
    if (pct > 1.0) pct = 1.0;

    /* Build right-side string: "ETA: 5s  42%" or "100%" */
    char right[32];
    if (current > 0 && pct < 1.0) {
        double eta = elapsed * (1.0 - pct) / pct;
        char eta_buf[16];
        fmt_eta(eta, eta_buf, sizeof(eta_buf));
        snprintf(right, sizeof(right), "ETA: %s  %.0f%%", eta_buf, pct * 100.0);
    } else {
        snprintf(right, sizeof(right), "%.0f%%", pct * 100.0);
    }

    /* Bar width: total - title - " [" - "] " - right */
    int title_len = (int)strlen(s_title);
    int right_len = (int)strlen(right);
    int bar_width = width - title_len - 1 - 2 - 1 - right_len;
    if (bar_width < 4) bar_width = 4;

    int filled = (int)(pct * bar_width);
    if (filled > bar_width) filled = bar_width;

    /* Render: \r + erase line */
    fprintf(stdout, "\r\033[2K");

    /* Title */
    fprintf(stdout, "%s [", s_title);

    /* Filled portion — green */
    fprintf(stdout, "\033[32m");
    for (int i = 0; i < filled; i++)
        fputs("\xe2\x96\x88", stdout); /* UTF-8: █ */
    fprintf(stdout, "\033[0m");

    /* Empty portion — dim */
    fprintf(stdout, "\033[2m");
    for (int i = filled; i < bar_width; i++)
        fputs("\xe2\x96\x91", stdout); /* UTF-8: ░ */
    fprintf(stdout, "\033[0m");

    fprintf(stdout, "] ");

    /* ETA (yellow) + percentage (cyan) */
    if (current > 0 && pct < 1.0) {
        double eta = elapsed * (1.0 - pct) / pct;
        char eta_buf[16];
        fmt_eta(eta, eta_buf, sizeof(eta_buf));
        fprintf(stdout, "\033[33mETA: %s\033[0m  \033[36m%.0f%%\033[0m",
                eta_buf, pct * 100.0);
    } else {
        fprintf(stdout, "\033[36m%.0f%%\033[0m", pct * 100.0);
    }

    fflush(stdout);
}

void forge_progress_bar_close(void)
{
    forge_progress_bar_advance(s_total);
    fputc('\n', stdout);
    fflush(stdout);
}
