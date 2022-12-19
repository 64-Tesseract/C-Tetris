// User config
#define DROPTIME_EXP 0.01  // How fast drop time decreases as pieces are placed
#define DROPTIME_ADD 0.15  // Minimum drop time
#define SPACE_DROP_CONFIRMS true  // Whether dropping with spacebar needs another press to place


// Constants, no touchie
#define DROPTIME(x) (exp(-DROPTIME_EXP * (float)x) + DROPTIME_ADD) * 100

struct FALLING { int id; int squares[8]; int colour; char symbol; };
static struct FALLING SHAPES[7] = {
    {0, {0, 0,   0, 1,   0,-1,   1,-1}, 1, 'L'},
    {1, {0, 0,   0,-1,   0, 1,   1, 1}, 2, 'J'},
    {2, {0, 0,   1, 0,   0, 1,   1,-1}, 3, 'S'},
    {3, {0, 0,   1, 0,   0,-1,   1, 1}, 4, 'Z'},
    {4, {0, 0,   1, 0,   0,-1,   0, 1}, 5, 'T'},
    {5, {0, 0,   0, 1,   0,-1,   0, 2}, 6, 'I'},
    {6, {0,-1,   1,-1,   0, 0,   1, 0}, 7, 'O'}
};
