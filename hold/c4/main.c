#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// data structures
// =============================================================================

// color of the stone.
enum COLOR {
        NONE = 0,  // not valid for stone
        BLACK,
        WHITE,
};

// position of the stone.
// - x is the row and y is col.
struct pos {
        int x;
        int y;
};

// board
struct board {
        int w;  // map to x, major, row
        int h;  // map to y, minor, col
        enum COLOR stones[];
};

// player abstract
struct player {
        char *name;
        void *data;  // private data.
                     //
        struct pos (*next_pos)(void *);
        void (*record_pos)(void *, struct pos);

        void (*free_fn)(void *);
};

// game context
struct context {
        enum COLOR starting_color;

        struct board *board;
        void (*board_free_fn)(struct board *);

        struct player *blackp;
        struct player *whitep;
};

// =============================================================================
// prototypes
// =============================================================================

typedef int error_t;
#define OK 0

// helpers
#define LOG(fmt, ...) _LOGF(fmt, __VA_ARGS__)  // TODO use log
#define LOG1(fmt)     _LOGF1(fmt)              // TODO use log

#define _LOGF(fmt, ...)                                 \
        do {                                            \
                size_t len = strlen(fmt);               \
                if (len >= 1 && fmt[len - 1] != '\n') { \
                        printf(fmt "\n", __VA_ARGS__);  \
                } else {                                \
                        printf(fmt, __VA_ARGS__);       \
                }                                       \
        } while (0)

#define _LOGF1(fmt)                                     \
        do {                                            \
                size_t len = strlen(fmt);               \
                if (len >= 1 && fmt[len - 1] != '\n') { \
                        printf(fmt "\n");               \
                } else {                                \
                        printf(fmt);                    \
                }                                       \
        } while (0)

#define ASSERT_VALID_COLOR(c) ((c) == WHITE || (c) == BLACK)

// ctx
void ctx_deinit(struct context *ctx);

// board
struct board *board_new(int w, int h);
void board_free(struct board *);

void board_draw(struct board *);
error_t board_put_new_stone(struct board *, struct pos, enum COLOR valid_c,
                            int *winner);

// player constructor
struct player *random_player(char *name);
struct player *ml_player(char *name);

// game
void play_game(struct context *ctx);

// =============================================================================
// main
// =============================================================================
int
main()
{
        struct board *b = board_new(/*row=*/6, /*col=*/7);

        struct context ctx = {
            .starting_color = BLACK,
            .board          = b,
            .board_free_fn  = board_free,
            .blackp         = random_player("black player"),
            .whitep         = random_player("white player"),
        };
        play_game(&ctx);

        ctx_deinit(&ctx);
        return 0;
}

// =============================================================================
// impl
// =============================================================================
void
play_game(struct context *ctx)
{
        struct player *bp = ctx->blackp;
        struct player *wp = ctx->whitep;
        LOG("black player name: %s", bp->name);
        LOG("white player name: %s", wp->name);

        enum COLOR current_player_color = ctx->starting_color;
        LOG("starting color   : %s",
            current_player_color == BLACK ? "black" : "white");

        struct player *winner = NULL;
        struct board *b       = ctx->board;
        int iter_count        = 0;
        int aborted           = 0;

        while (1) {
                iter_count++;
                board_draw(b);

                struct player *p = current_player_color == BLACK ? bp : wp;
                LOG("player name now: %s", p->name);

                struct pos pos = p->next_pos(p->data);
                LOG("next pos x: %d", pos.x);
                LOG("next pos y: %d", pos.y);

                int has_winner;
                if (OK != board_put_new_stone(b, pos, current_player_color,
                                              &has_winner)) {
                        aborted = 1;
                        LOG1("aborted...");  // TODO better err handling
                        goto quit;
                }

                if (has_winner) {
                        board_draw(b);

                        winner = p;
                        goto quit;
                }

                continue;
        quit:
                LOG("end game with %d iteration(s)", iter_count);
                if (aborted) {
                        LOG1("end reason: aborted.");

                } else {
                        LOG1("end reason: finished.");
                        if (winner == NULL)
                                LOG1("tie");
                        else
                                LOG("found winner: %s", winner->name);
                }
                break;
        }
}

// =============================================================================
// ctx

void
ctx_deinit(struct context *ctx)
{
        // TODO
}

// =============================================================================
// board

struct board *
board_new(int w, int h)
{
        struct board *b;
        int board_size = w * h;
        int byte_size  = sizeof(*b) + sizeof(enum COLOR) * board_size;

        // b->stones are pre-filled.
        b    = calloc(1, byte_size);
        b->w = w;
        b->h = h;
        return b;
}

void
board_free(struct board *b)
{
        // TODO;
}

void
board_draw(struct board *b)
{
        // TODO
        LOG1("drawing...");
}

error_t
board_put_new_stone(struct board *b, struct pos pos, enum COLOR color,
                    int *winner)
{
        ASSERT_VALID_COLOR(color);
        // fake code

        *winner = 1;
        return OK;
}

// =============================================================================
// player

struct pos
random_pos(void *data)
{
        return (struct pos){
            .x = 0,
            .y = 0,
        };
}

struct player *
random_player(char *name)
{
        struct player *p = malloc(sizeof(*p));
        p->name          = name;  // TODO unsafe, use sds_t
        p->data          = NULL;
        p->next_pos      = random_pos;
        p->record_pos    = NULL;  // TODO
        p->free_fn       = free;  // TODO
        return p;
}

struct player *
ml_player(char *name)
{
        struct player *p = malloc(sizeof(*p));
        p->name          = name;  // TODO unsafe, use sds_t
        p->data          = NULL;
        p->next_pos      = random_pos;
        p->record_pos    = NULL;  // TODO
        p->free_fn       = free;  // TODO
        return p;
}
