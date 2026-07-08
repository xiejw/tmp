// htop-style TUI demo.
//
// Top half of the terminal shows horizontal meter bars (CPU usage, CPU memory,
// and a fake background-driven metric), each colored green/yellow/red by a
// threshold. The bars refresh every 3s from a background thread. The bottom half
// is a scrolling log that the main thread appends to once per second.
//
// Single file, no dependencies beyond libc/pthread and a Linux /proc filesystem.
// Written in modern C++ (built with -std=c++20 on the local g++ 10.5; trivially
// upgradable to c++23 by swapping the snprintf helpers for std::format/print).
//
//   g++ -std=c++20 -O2 -pthread -o htop_demo htop_demo.cpp
//   ./htop_demo        # Ctrl-C to quit

#include <atomic>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <deque>
#include <fstream>
#include <mutex>
#include <random>
#include <string>
#include <thread>

#include <sys/ioctl.h>
#include <unistd.h>

namespace {

// ------------------------------- shared state -------------------------------

struct Metrics {
    std::atomic<double> cpu{0.0};   // % CPU busy
    std::atomic<double> mem{0.0};   // % memory used
    std::atomic<double> fake{0.0};  // random 0..100, background-thread owned
};

Metrics g_metrics;
std::mutex g_render_mtx;                 // serializes all writes to the terminal
std::atomic<bool> g_running{true};       // cleared by SIGINT to stop the loops

int g_rows = 24;
int g_cols = 80;
int g_split = 12;                        // last row of the top (bars) region

// ------------------------------- ANSI helpers -------------------------------

constexpr const char* kReset  = "\033[0m";
constexpr const char* kGreen  = "\033[32m";
constexpr const char* kYellow = "\033[33m";
constexpr const char* kRed    = "\033[31m";
constexpr const char* kBold   = "\033[1m";
constexpr const char* kDim    = "\033[2m";
constexpr const char* kCyan   = "\033[36m";

std::string move_to(int row, int col) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "\033[%d;%dH", row, col);
    return buf;
}

const char* color_for(double pct) {
    if (pct < 30.0) return kGreen;
    if (pct > 80.0) return kRed;
    return kYellow;
}

void write_all(const std::string& s) {
    size_t off = 0;
    while (off < s.size()) {
        ssize_t n = ::write(STDOUT_FILENO, s.data() + off, s.size() - off);
        if (n <= 0) break;
        off += static_cast<size_t>(n);
    }
}

// ------------------------------- metric sources -----------------------------

// Reads the aggregate "cpu" line of /proc/stat. Returns false if unavailable.
bool read_cpu_totals(unsigned long long& total, unsigned long long& idle) {
    std::ifstream f("/proc/stat");
    if (!f) return false;
    std::string cpu;
    unsigned long long user = 0, nice = 0, sys = 0, idl = 0, iowait = 0,
                       irq = 0, softirq = 0, steal = 0;
    f >> cpu >> user >> nice >> sys >> idl >> iowait >> irq >> softirq >> steal;
    if (cpu != "cpu") return false;
    idle  = idl + iowait;
    total = user + nice + sys + idl + iowait + irq + softirq + steal;
    return true;
}

// % memory used, from /proc/meminfo. Returns -1 if unavailable.
double read_mem_percent() {
    std::ifstream f("/proc/meminfo");
    if (!f) return -1.0;
    std::string key;
    unsigned long long value = 0;
    std::string unit;
    unsigned long long total = 0, available = 0;
    while (f >> key >> value >> unit) {
        if (key == "MemTotal:") total = value;
        else if (key == "MemAvailable:") available = value;
        if (total && available) break;
    }
    if (total == 0) return -1.0;
    return 100.0 * static_cast<double>(total - available) / static_cast<double>(total);
}

// -------------------------------- log buffer --------------------------------

std::deque<std::string> g_log;   // ring buffer, only touched by the main thread

std::string timestamp() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
    localtime_r(&t, &tm);
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
    return buf;
}

// -------------------------------- rendering ---------------------------------

// Draws one meter bar on the given row. Label is left-padded to a fixed width so
// all bars line up regardless of name length.
std::string render_bar(int row, const char* label, double pct) {
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;

    constexpr int kLabelW = 8;    // "CPU     "
    const int kTailW = 9;         // "] 100.0%"
    const int kBrackets = 1;      // leading "["
    int inner = g_cols - kLabelW - kTailW - kBrackets - 2;
    if (inner < 4) inner = 4;

    int filled = static_cast<int>(std::lround(pct / 100.0 * inner));
    if (filled > inner) filled = inner;

    std::string bar;
    bar.reserve(g_cols + 32);
    bar += move_to(row, 1);
    bar += "\033[2K";  // clear line

    char lbl[16];
    std::snprintf(lbl, sizeof(lbl), "%-*s", kLabelW, label);
    bar += kBold;
    bar += lbl;
    bar += kReset;

    bar += "[";
    bar += color_for(pct);
    bar.append(filled, '|');
    bar += kReset;
    bar.append(inner - filled, ' ');
    bar += "] ";

    char tail[16];
    std::snprintf(tail, sizeof(tail), "%s%5.1f%%%s", color_for(pct), pct, kReset);
    bar += tail;
    return bar;
}

// Redraws the entire top region: title + three bars + a separator line.
void draw_bars() {
    std::string out;
    out += move_to(1, 1);
    out += "\033[2K";
    out += kBold;
    out += kCyan;
    out += "  htop-demo — system meters (refresh 3s)";
    out += kReset;

    out += render_bar(3, "CPU", g_metrics.cpu.load());
    out += render_bar(4, "Mem", g_metrics.mem.load());
    out += render_bar(5, "Fake", g_metrics.fake.load());

    // Separator between the meters region and the log region.
    out += move_to(g_split, 1);
    out += "\033[2K";
    out += kDim;
    out.append(g_cols, '-');
    out += kReset;

    std::lock_guard<std::mutex> lk(g_render_mtx);
    write_all(out);
}

// Redraws the log region (rows g_split+1 .. g_rows) from the ring buffer.
void draw_log() {
    int first = g_split + 1;
    int capacity = g_rows - first + 1;
    if (capacity < 1) capacity = 1;

    while (static_cast<int>(g_log.size()) > capacity) g_log.pop_front();

    std::string out;
    int row = first;
    for (const auto& line : g_log) {
        out += move_to(row, 1);
        out += "\033[2K";
        // Truncate to terminal width so wrapping never shifts the layout.
        if (static_cast<int>(line.size()) > g_cols)
            out += line.substr(0, g_cols);
        else
            out += line;
        ++row;
    }
    // Clear any leftover rows below the current log content.
    for (; row <= g_rows; ++row) {
        out += move_to(row, 1);
        out += "\033[2K";
    }

    std::lock_guard<std::mutex> lk(g_render_mtx);
    write_all(out);
}

// -------------------------------- lifecycle ---------------------------------

void query_terminal_size() {
    winsize ws{};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 0 && ws.ws_col > 0) {
        g_rows = ws.ws_row;
        g_cols = ws.ws_col;
    }
    g_split = g_rows / 2;
    if (g_split < 6) g_split = 6;  // keep room for title + 3 bars + separator
}

void enter_screen() {
    write_all("\033[?25l");  // hide cursor
    write_all("\033[2J");    // clear screen
    write_all("\033[H");     // home
}

void leave_screen() {
    std::string out;
    out += kReset;
    out += "\033[?25h";               // show cursor
    out += move_to(g_rows, 1);        // park cursor at the bottom
    out += "\n";
    write_all(out);
}

void on_sigint(int) { g_running.store(false); }

// Background thread: samples metrics every 3s and redraws the bars.
void metrics_loop(std::stop_token st) {
    std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<double> dist(0.0, 100.0);

    unsigned long long prev_total = 0, prev_idle = 0;
    bool have_prev = false;

    using namespace std::chrono_literals;
    while (!st.stop_requested() && g_running.load()) {
        unsigned long long total = 0, idle = 0;
        if (read_cpu_totals(total, idle)) {
            if (have_prev && total > prev_total) {
                double dtotal = static_cast<double>(total - prev_total);
                double didle  = static_cast<double>(idle - prev_idle);
                double busy   = (dtotal - didle) / dtotal * 100.0;
                if (busy < 0) busy = 0;
                if (busy > 100) busy = 100;
                g_metrics.cpu.store(busy);
            }
            prev_total = total;
            prev_idle  = idle;
            have_prev  = true;
        }

        double mem = read_mem_percent();
        if (mem >= 0) g_metrics.mem.store(mem);

        g_metrics.fake.store(dist(rng));

        draw_bars();

        // Sleep in small slices so we react to shutdown promptly.
        for (int i = 0; i < 30 && !st.stop_requested() && g_running.load(); ++i)
            std::this_thread::sleep_for(100ms);
    }
}

}  // namespace

int main() {
    query_terminal_size();

    std::signal(SIGINT, on_sigint);
    std::signal(SIGTERM, on_sigint);

    enter_screen();
    draw_bars();

    std::jthread metrics(metrics_loop);

    const char* levels[] = {"INFO ", "INFO ", "WARN ", "INFO ", "ERROR"};
    const char* msgs[] = {
        "request handled",
        "cache hit for key",
        "flushing write buffer",
        "connection accepted",
        "slow query detected",
        "worker heartbeat ok",
        "retrying upstream call",
        "gc pause completed",
    };
    const int nlevels = sizeof(levels) / sizeof(levels[0]);
    const int nmsgs = sizeof(msgs) / sizeof(msgs[0]);

    using namespace std::chrono_literals;
    unsigned long counter = 0;
    while (g_running.load()) {
        const char* lvl = levels[counter % nlevels];
        const char* msg = msgs[counter % nmsgs];
        const char* col = (std::strcmp(lvl, "ERROR") == 0) ? kRed
                        : (std::strcmp(lvl, "WARN ") == 0) ? kYellow
                        : kGreen;

        std::string line = std::string(kDim) + "[" + timestamp() + "] " + kReset
                         + col + lvl + kReset + kDim + " worker#"
                         + std::to_string(counter % 4) + kReset + ": " + msg
                         + " (req=" + std::to_string(counter) + ")";
        g_log.push_back(line);

        draw_log();

        ++counter;
        for (int i = 0; i < 10 && g_running.load(); ++i)
            std::this_thread::sleep_for(100ms);
    }

    metrics.request_stop();
    leave_screen();
    return 0;
}
