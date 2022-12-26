#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include "tetris.h"


bool playing = true;
bool paused = false;
int grid[200] = {0};
int top_row = 20;
int last_cols = 0;
int drop_time, drop_timer, pieces_placed = 0;
bool down_conf = false;
struct FALLING falling_shape, next_shape, saved_shape = {0};
bool shape_saved, shape_swapped = false;
int combo, score = 0;


void draw_border (int dx) {
    attrset(COLOR_PAIR(0));

    if (COLS > 35) {
        mvprintw(20, dx - 2, "':");
        mvprintw(20, dx + 20, ":'");

        for (int n = 0; n < 10; n++) {
            mvprintw(20, dx + n * 2, "==");
        }

        for (int n = 0; n < 20; n++) {
            mvprintw(n, dx - 2, "||");
            mvprintw(n, dx + 20, "||");
        }

    } else {
        mvprintw(20, dx - 1, "'");
        mvprintw(20, dx + 20, "'");

        for (int n = 0; n < 10; n++) {
            mvprintw(20, dx + n * 2, "==");
        }

        for (int n = 0; n < 20; n++) {
            mvprintw(n, dx - 1, "|");
            mvprintw(n, dx + 20, "|");
        }
    }
}


void render () {
    int dx = fmax(COLS > 35 ? COLS / 2 - 16 : COLS / 2 - 11, 0);

    clear();
    draw_border(dx);

    for (int y = top_row; y < 20; y++) {
        for (int x = 0; x < 10; x++) {
            int index = x + y * 10;
            if (grid[index] != 0) {
                attrset(COLOR_PAIR(grid[index]));
                mvprintw(y, x * 2 + dx, "  "); 
            }
        }
    }

    attrset(COLOR_PAIR(falling_shape.colour));
    for (int s = 0; s < 4; s++) {
        int sx = falling_shape.squares[s * 2 + 1]; int sy = falling_shape.squares[s * 2];
        mvprintw(sy, sx * 2 + dx, "  ");
    }

    if (COLS > 35) {
        attrset(COLOR_PAIR(0));
        mvprintw(0, dx + 23, "Next:");
        mvprintw(4, dx + 23, "Saved:");
        mvprintw(8, dx + 23, "Placed:");
        mvprintw(9, dx + 24, "%d", pieces_placed);
        mvprintw(12, dx + 23, "Score:");
        mvprintw(13, dx + 24, "%d", score);

        attrset(COLOR_PAIR(next_shape.colour));
        for (int s = 0; s < 4; s++) {
            int sx = next_shape.squares[s * 2 + 1]; int sy = next_shape.squares[s * 2];
            mvprintw(sy + 1, sx * 2 + dx + 26, "  ");
        }

        if (shape_saved) {
            attrset(COLOR_PAIR(saved_shape.colour));
            for (int s = 0; s < 4; s++) {
                int sx = saved_shape.squares[s * 2 + 1]; int sy = saved_shape.squares[s * 2];
                mvprintw(sy + 5, sx * 2 + dx + 26, "  ");
            }
        }

    } else {
        attrset(COLOR_PAIR(0));
        mvprintw(8, dx + 22, "%x", pieces_placed);
        mvprintw(12, dx + 22, "%x", score);

        attrset(COLOR_PAIR(next_shape.colour + 7));
        mvprintw(0, dx + 22, "%c", next_shape.symbol);

        if (shape_saved) {
            attrset(COLOR_PAIR(saved_shape.colour + 7));
            mvprintw(4, dx + 22, "%c", saved_shape.symbol);
        }
    }

    if (paused) {
        attrset(COLOR_PAIR(0));
        mvprintw(9, dx + 7, "PAUSED");
        mvprintw(10, dx + 3, "(p to resume)");
    }
}


bool intersecting_block (int x, int y) {
    if (x < 0 || x >= 10 || y > 19) return true;
    else if (y >= 0) return grid[x + y * 10] != 0;
    return false;
}


bool intersecting_falling (int squares[8]) {
    for (int s = 0; s < 4; s++) {
        int sx = squares[s * 2 + 1]; int sy = squares[s * 2];

        if (intersecting_block(sx, sy)) return true;
    }

    return false;
}


void force_move (struct FALLING* shape, int dx, int dy) {
    for (int s = 0; s < 4; s++) {
        shape->squares[s * 2 + 1] += dx; shape->squares[s * 2] += dy;
    }
}


void get_next_shape () {
    falling_shape = next_shape; next_shape = SHAPES[rand() % 7];
    force_move(&falling_shape, 5, 0);
}


void save_shape () {
    if (!shape_swapped) {
        if (shape_saved) {
            struct FALLING swap = saved_shape;
            saved_shape = SHAPES[falling_shape.id];
            falling_shape = swap;
            force_move(&falling_shape, 5, 0);

        } else {
            saved_shape = SHAPES[falling_shape.id];
            get_next_shape();
            shape_saved = true;
        }

        shape_swapped = true;
        drop_timer = drop_time;
        down_conf = false;
        render();
    }
}


void falling_place () {
    for (int s = 0; s < 4; s++) {
        int sx = falling_shape.squares[s * 2 + 1]; int sy = falling_shape.squares[s * 2];

        if (sy < 0 || grid[sx + sy * 10] != 0) {
            playing = false;
            return;
        }

        grid[sx + sy * 10] = falling_shape.colour;
        top_row = fmin(top_row, sy);
    }

    int cleared = 0;
    for (int y = 19; y >= top_row; ) {
        bool full = true;

        for (int x = 0; x < 10; x++) {
            if (grid[x + y * 10] == 0) {
                full = false;
                break;
            }
        }

        if (full) {
            for (int ty = y; ty >= top_row; ty--) {
                for (int tx = 0; tx < 10; tx++) {
                    grid[tx + ty * 10] = ty >= 0 ? grid[tx + ty * 10 - 10] : 0;
                }
            }

            top_row++;
            cleared++;

        } else {
            y--;
        }
    }

    if (cleared != 0) {
        score += (cleared * 10 + pow(cleared, 2) - 1) * (1 + 0.1 * combo);
        combo++;

    } else {
        combo /= 2;
    }

    pieces_placed++;
    shape_swapped = false;
    get_next_shape();
}


bool falling_move (int dx, int dy) {
    struct FALLING temp_shape = falling_shape;
    bool placed = false;

    force_move(&temp_shape, dx, dy);
    bool intersecting = intersecting_falling(temp_shape.squares);
    bool down = dx == 0 && dy == 1;

    if (!intersecting) {
        falling_shape = temp_shape;
        if (down_conf) drop_timer = drop_time;
        down_conf = false;
        render();
    }

    if (down) {
        if (intersecting) {
            if (down_conf) {
                falling_place();
                placed = true;
                drop_time = DROPTIME(pieces_placed);
                render();
            }

            down_conf = !down_conf;
        }

        drop_timer = drop_time;
    }

    return placed;
}


void falling_drop () {
#if SPACE_DROP_CONFIRMS
    // Drop piece to ground but don't place it, unless it's already on ground
    if (!falling_move(0, 1)) {  // If it hasn't just placed...
        do; while (!falling_move(0, 1) && !down_conf);  // Drop again until it's placed or is about to be placed
    }
    down_conf = false;
#else
    // Drop piece until it's placed
    while (!falling_move(0, 1));
#endif
}


void falling_spin (int dir) {
    for (int s_axis = 0; s_axis < 4; s_axis++) {
        struct FALLING temp_shape = falling_shape;

        for (int s = 0; s < 4; s++) {
            /*
            (y - y_axis) = (x - x_axis) * dir
            y = (x - x_axis) * dir + y_axis
            */
            temp_shape.squares[s * 2] = (falling_shape.squares[s * 2 + 1] - falling_shape.squares[s_axis * 2 + 1]) * dir + falling_shape.squares[s_axis * 2];
            /*
            (x - x_axis) = (y - y_axis) * -dir
            x = (y_axis - y) * dir + x_axis
            */
            temp_shape.squares[s * 2 + 1] = (falling_shape.squares[s_axis * 2] - falling_shape.squares[s * 2]) * dir + falling_shape.squares[s_axis * 2 + 1];
        }

        if (!intersecting_falling(temp_shape.squares)) {
            falling_shape = temp_shape;
            down_conf = false;
            render();
            break;
        }
    }
}


int main (int argc, char *argv[]) {
    srand(time(0));
    falling_shape = SHAPES[rand() % 7]; next_shape = SHAPES[rand() % 7];
    force_move(&falling_shape, 5, 0);
    drop_time = DROPTIME(0); drop_timer = drop_time;

    initscr();
    noecho();
    start_color();
    curs_set(FALSE);
    nodelay(stdscr, TRUE);

    for (int c = 1; c < 8; c++)  {
        init_pair(c, 0, c);
        init_pair(c + 7, c, 0);
    }

    render();

    while (playing) {
        char c = getch();

        if (c == 'p') {
            paused = !paused;
            render();
        } else if (!paused) {
            switch (c) {
                case 'a':
                case 'h':
                    falling_move(-1, 0);
                    break;
                case 'd':
                case 'l':
                    falling_move(1, 0);
                    break;
                case 's':
                    falling_move(0, 1);
                    break;
                case '[':
                case 'j':
                    falling_spin(-1);
                    break;
                case ']':
                case 'k':
                    falling_spin(1);
                    break;
                case ' ':
                    falling_drop();
                    break;
                case 'q':
                case 'c':
                    save_shape();
                    break;
            }
        }

        if (last_cols != COLS) {
            last_cols = COLS;
            render();
        }

        if (!paused) {
            if (drop_timer-- <= 0)
                falling_move(0, 1);
        }

        refresh();
        usleep(10000);
    }
    
    usleep(100000);
    while (getch() == ERR) usleep(10000);
    
    endwin();
    return 0;
}
