/* This is a simple little snake clone for POSIX-compliant systems.
   Check the README file for planned additions. */

#define _POSIX_C_SOURCE 199309L

#include <curses.h>
#include <inttypes.h>
//#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <ext_getopt.h>

#include "version_config.h"

/* Placing every macro used in the program up here feels wrong, like I'm doing
   the same thing as using global variables :( */
#define COLOR_MAP_SIZE 10
#define LENGTH_INCRIMENTER 4

typedef int_fast32_t i32_f;
typedef uint_fast32_t u32_f;
typedef int8_t i8;
typedef uint8_t u8;

typedef enum {
    NO_DIRECTION,
    NORTH,
    EAST,
    SOUTH,
    WEST,
} DirectionType;

typedef enum {
    NO_ID,
    BORDER_ID,
    SNAKE_ID,
    PELLET_ID
} IdType;

/* Function that, uhh, *checks notes* prints the program's version number to
   the screen.
   TODO: make this function "smarter" by having a detection for the macros used are
   set, etc */
void printVersion(void) {
    printf("\33[1;97m%s\33[m, version \33[1;97m%s\33[m\n\n%s\n",
        PROGRAM_NAME,
        VERSION_NUMBER,
        AUTHOR_SIGNATURE
    );
}

/* This struct is used to represent a 2D pair of values for any purpose.
   This could be, say, the coordinates to a part on a 2D array, or whatever. */
typedef struct {
    i32_f x,y;
} Coords;

// Function that creates a coordinate struct from a pair of values as parameters.
Coords newCoords(u32_f x, u32_f y) {
    return (Coords) {x,y};
}

Coords halfCoords(Coords coords) {
    return (Coords) {coords.x / 2,coords.y / 2};
}

// Some macros for transforming coords to various things. (SUUCH a great comment!)
#define COORD_ARR(coords) [(coords).x][(coords).y]
#define COORD_PAR(coords) (coords).x,(coords).y
#define COORD_PRO(coords) ((coords).x * (coords).y)

/* Feed this function a coordinate struct and some values for a direction and
   a distance, and it will return some coords representing where something would
   travel to from that information. */
Coords travel(Coords position, DirectionType direction, i32_f distance) {
    if(!distance) return position;

    switch(direction) {
        case NORTH:
            return (Coords) {position.x,position.y - distance};
        case EAST:
            return (Coords) {position.x + distance,position.y};
        case SOUTH:
            return (Coords) {position.x,position.y + distance};
        case WEST:
            return (Coords) {position.x - distance,position.y};

        default:
            return position;
    }
}

// Function that adds a specified char to the curses screen, twice. Damn.
void addTwoChars(char the_char) {
    addch(the_char);
    addch(the_char);
}

/* This function is like the last one, but it also changes the position of cursor
   while it's at it. */
void moveAndAddTwoChars(u32_f y, u32_f x, char the_char) {
    move(y,x);
    addTwoChars(the_char);
}

typedef struct ColorPair {
    short fg, bg;
} ColorPair;

typedef struct ColorTable {
    ColorPair border, snake, pellet;
} ColorTable;

int setColorOfId(u8 id, ColorPair colors) {
    return init_pair(id,colors.fg,colors.bg);
}

typedef struct {
    u8 id;
    DirectionType direction;
} PlayfieldPart;

typedef struct {
    Coords size,draw_point;
    PlayfieldPart **grid,
    *array;
} PlayfieldType;

/* This function initializes a 2D array for a playfield. It does not add the
   snake or the food pellet to the array on its own, though. */
PlayfieldPart **makePlayfieldArray(Coords dimensions) {
    u32_f xy_product = COORD_PRO(dimensions);

    // Allocating our memory
    void *returned = malloc (
        sizeof(PlayfieldPart) * xy_product +
        sizeof(PlayfieldPart*) * dimensions.x
    );

    if(!returned) return returned;

    // Setting up first array
    for(u32_f i = 0; i < dimensions.x; i++) {
        ((PlayfieldPart**) returned)[i] =
            (PlayfieldPart*)((PlayfieldPart**) returned + dimensions.x) + dimensions.y * i;
    }

    // Initializing every cell in the array to values representing no data
    memset(((PlayfieldPart**) returned)[0],0,sizeof(PlayfieldPart) * xy_product);

    return returned;
}

/* Function that creates a new playfield array, and the properties for it. */
PlayfieldType newPlayfield(Coords dimensions) {
    if(dimensions.x <= 0 || dimensions.y <= 0) {
        // run some panic code here
    }

    PlayfieldType returned = {
        .size = dimensions,
        .draw_point = newCoords(0,0),
        .grid = makePlayfieldArray(dimensions),
        .array = returned.grid[0] // TODO: Make this part more elagent, where I
    };                            // just directly derefrence it like this

    return returned;
}

// Function that unallocates the memory for our playfield array.
bool nukePlayfield(PlayfieldType *playfield) {
    if(!playfield->grid) return false;

    free(playfield->grid);
    playfield->grid = NULL;
    playfield->array = NULL;

    return true;
}

/* Function that places a pellet (a square with the ID of PELLET_ID) on to a random
   spot on the playfield that isn't already occupied with something else. */
u8 addPellet(PlayfieldType *playfield) {
    if(!playfield) return 1;
    if(!playfield->grid) return 2;

    u32_f random_index, grid_area = COORD_PRO(playfield->size);

    /* We continuously generate random numbers until the index it lands on is not
       being occupied. This could theoretically cause performance problems with
       very long snakes that occupy a large portion of the board, so I might change
       the algorithm later so it will not be like that? But at the same time, this
       will be unlikely to cause problems, realistically. */
    do {
        random_index = rand() / (RAND_MAX / grid_area);
    } while(playfield->array[random_index].id);

    playfield->array[random_index].direction = NO_DIRECTION;
    playfield->array[random_index].id = PELLET_ID;

    return 0;
}

/* Shamefully copy-pasted this function bc I didn't feel like figuring out how to
   reuse the code properly. We need a different function for placing the first pellet
   because I want to make sure it's impossible for a player to collect a pellet
   straight-away at the beginning of the game without doing anything. */
u8 addFirstPellet(PlayfieldType *playfield, u32_f omitted_row) {
    if(!playfield) return 1;
    if(!playfield->grid) return 2;

    u32_f random_x = rand() / (RAND_MAX / playfield->size.x),
          random_y = rand() / (RAND_MAX / (playfield->size.y - 1));

    if(random_y >= omitted_row) random_y++;

    playfield->grid[random_x][random_y].direction = NO_DIRECTION;
    playfield->grid[random_x][random_y].id = PELLET_ID;

    return 0;
}

typedef struct Snake {
    PlayfieldType *owner;
    Coords head, tail;
    u32_f length, target_length;
    DirectionType direction;
    u8 id;
} SnakeType;

/* Function that returns a struct for a new snake struct, while adding the snake to
   a playfield specified. Note that a snake with a length of zero is a perfectly valid input. */
SnakeType newSnake(PlayfieldType *owner, u32_f length, Coords position) {
    SnakeType returned = {
        .owner = owner,
        .head = position,
        .tail = position,
        .length = 1,
        .target_length = length,
        .direction = EAST
    };

    owner->grid COORD_ARR(position).id = SNAKE_ID;
    return returned;
}

/* Function that advances a snake object forward in its playfield once in a
   direction. It will also do the appropriate thing for when we encounter an obstacle
   or a pellet. */
u8 advanceSnake(SnakeType *snake) {
    if(!snake) return 1;

    Coords next_coord = travel(snake->head,snake->direction,1);
    bool pellet_spawn_flag = false;

    // If a collision with a wall or the snake happens, quit the game
    if(next_coord.x < 0 || next_coord.x >= snake->owner->size.x ||
        next_coord.y < 0 || next_coord.y >= snake->owner->size.y ||
        snake->owner->grid COORD_ARR(next_coord).id == SNAKE_ID
    ) {
        return 2;
    }

    /* If colliding with a pellet, increase the terget length and prepare to spawn
       a new pellet later in this function. */
    if(snake->owner->grid COORD_ARR(next_coord).id == PELLET_ID) {
        snake->target_length += LENGTH_INCRIMENTER;
        snake->length++;
        pellet_spawn_flag = true;

        goto post_tail_move; // nothing to see here
    }

    /* move tail, unless length doesn't meet target_length, then
       incriment length by 1 */
    if(snake->length == snake->target_length) {
        snake->owner->grid COORD_ARR(snake->tail).id = NO_ID;
        snake->tail = travel (
            snake->tail,
            snake->owner->grid COORD_ARR(snake->tail).direction,
            1
        );
    } else {
        snake->length++;
    }

    post_tail_move:

    // move head
    snake->owner->grid COORD_ARR(snake->head).direction = snake->direction;
    snake->head = travel(snake->head,snake->direction,1);
    snake->owner->grid COORD_ARR(snake->head).id = SNAKE_ID;

    if(pellet_spawn_flag) addPellet(snake->owner);
    return 0;
}

/* Function that operates on a snake struct to change its direction value. I'm using
   a function to do it instead of doing it directly, because this functon checks
   if the new direction is "valid" (as in, not exactly opposite of the current
   direction) */
u8 setSnakeDirection(SnakeType *snake, DirectionType direction) {
    if(!(snake && direction)) return (!snake ? 1:0) | (!direction ? 2:0);

    // Doing an early return if the direction we're trying to make the snake move in is opposite of
    // the current one
    if(!(((snake->direction) - direction) % 2)) return 4;

    snake->direction = direction;
    return 0;
}

/* This function figures out where the upper-left corner of the playfield should
   be on the curses screen, and saves that to a value in playfield. 
   TODO: Apparantly this function also resets the curses screen. A refactor so that these are not
   intertwined could be a good idea.*/
u8 centerPlayfield(PlayfieldType *playfield) {
    if(!playfield) return 1;

    clear();
    attrset(A_NORMAL);

    Coords top_left;
    getmaxyx(stdscr,top_left.y,top_left.x);

    top_left = newCoords (
        top_left.x / 2 - playfield->size.x - 2,
        top_left.y / 2 - playfield->size.y / 2 - 1
    );

    if(top_left.x < 0 || top_left.y < 0) return 2;

    playfield->draw_point = top_left;
    return 0;
}

/* This function draws everything on the screen. This code is pretty inefficient,
   and I'm not very proud of it, but it shouldn't cause *too* many performance issues
   since curses will only update stuff in the buffer that has changed, anyway. I'm
   still probably going to rewrite it to improve efficiency in the future, however.
   */
int draw(PlayfieldType *playfield) {
    if(!playfield) return 1;

    attrset(A_BOLD | COLOR_PAIR(BORDER_ID));

    u32_f min_x = playfield->draw_point.x,
          min_y = playfield->draw_point.y,
          max_x = min_x + playfield->size.x * 2 + 2,
          max_y = min_y + playfield->size.y + 1;

    move(min_y,min_x);
    for(u32_f i = min_x,maxer_x = max_x + 1; i < maxer_x; i++) {
        addch(' ');
    }

    move(max_y,min_x);
    for(u32_f i = min_x,maxer_x = max_x + 2; i < maxer_x; i++) {
        addch(' ');
    }

    for(u32_f i = min_y; i < max_y; i++) {
        moveAndAddTwoChars(i,min_x,' ');
        moveAndAddTwoChars(i,max_x,' ');
    }

    // drawing innards now
    for(u32_f i_y = 0; i_y < playfield->size.y; i_y++) {
        move(min_y + i_y + 1,min_x + 2);

        for(u32_f i_x = 0; i_x < playfield->size.x; i_x++) {
            attrset(A_BOLD | COLOR_PAIR(playfield->grid[i_x][i_y].id));
            addTwoChars(' ');
        }
    }

    move(0,0);
    refresh();
    return 0;
}

/* This function starts up a curses session, and calls a bunch of functions to set it
   up exactly how I like for this program. */
void initCurses(void) {
    initscr();
    cbreak(); // Might want to replace with raw() someday...
    noecho();
    keypad(stdscr,true);
    start_color();

    // For some reason, on macOS, the SIGWINCH signal either doesn't exist, or something similar.
    // Using a little bit of preprocessor fuckery to disable using this signal on Apple platforms
    // specifically.
    #ifndef __APPLE__
        signal(SIGWINCH,NULL); // I will need to use this to have a special action occur
    #endif                     // if SIGWINCH is raised.

    clear();
}

// in this context, "trans" is an abbreviation for "translate". We are translating a keypress
// (the variable "input") into a DirectionType.
DirectionType transKeypressToDirection(int input) {
    switch(input) {
        case KEY_UP:
        case 'w':
        case 'W':
            return NORTH;

        case KEY_RIGHT:
        case 'd':
        case 'D':
            return EAST;

        case KEY_DOWN:
        case 's':
        case 'S':
            return SOUTH;

        case KEY_LEFT:
        case 'a':
        case 'A':
            return WEST;

        default:
            return NO_DIRECTION;
    }
}

/* This function is where all the magic happens. The core game bits happen right
   here. */
u32_f game(Coords dimensions, ColorTable colors) {
    srand(time(NULL));
    timeout(0);

    // initialize the playfield array, and all the things inside it (snake, pellet)
    PlayfieldType playfield = newPlayfield(dimensions);
    SnakeType snake = newSnake(&playfield,5,newCoords(2,dimensions.y / 2));
    addFirstPellet(&playfield,dimensions.y / 2);

    // Binding each ID for an object to a color
    setColorOfId(BORDER_ID,colors.border);
    setColorOfId(SNAKE_ID,colors.snake);
    setColorOfId(PELLET_ID,colors.pellet);

    centerPlayfield(&playfield);
    draw(&playfield);

    // The main loop. :3
    while(true) {
        int input;

        if((input = getch()) != -1) {
            setSnakeDirection(&snake,transKeypressToDirection(input));
        }

        if(advanceSnake(&snake) == 2) {
            break;
        };

        draw(&playfield);

        // TODO: nanosleep wil suspend for this time no matter what. Make this suspend for the
        // right amount of time when a lot of stuff is done, so the game doesn't slow down a little
        // when that happens.
        nanosleep(&(struct timespec){0,90000000},NULL);
    }

    nukePlayfield(&playfield); // Overly dramatic names for functions that deallocate memory are
    timeout(-1);               // funny.
    return snake.length;
}

// TODO: This program is currently using the *terrible* getopt API present in POSIX for parsing
// command line arguments. getopt is very limiting, so I plan on replacing it with my own
// home-grown argument parser... eventually.
int main(int argc, char *argv[]) {
    CommandOption opts[] = {
        {"h","help","Prints the help text and exits"},
        {"Vv","version","Prints the version number and exits"}
    };

    bool help_flag = false,
    version_flag = false;

    char *short_opts = makeShortOpts(opts,2);

    /* Using the getopt() function provided by POSIX to loop over all command
       options. TODO: Replace getopt() with a better library for this. getopt()
       isn't doing what I want out of it. */
    for(int opt; (opt = getopt(argc,argv,short_opts)) != -1;) {
        switch(opt) {
            case 'h':
                help_flag = true;
                break;

            case 'V':
            case 'v':
                version_flag = true;
                break;

            default:
                putchar('\n');
                printOptions(PROGRAM_NAME,opts,stderr);
                return 1;
        }
    }

    free(short_opts);

    /* Forbidding positional arguments from being passed, since I doubt this program
       will ever have a good use for them anyway. */
    if(argv[optind]) {
        fputs("No positional arguments are allowed.\n",stderr);
        return 2;
    }

    /* Did the help or version flags get set? If so, let's print out help and/or
       version info the stdout, and then exit before even starting the game. */
    if(help_flag) {
        printHelp(PROGRAM_NAME,PROGRAM_DESCRIPTION,opts,NULL);

        if(version_flag) {
            putchar('\n');
            printVersion();
        }

        return EXIT_SUCCESS;
    }

    if(version_flag) {
        printVersion();
        return EXIT_SUCCESS;
    }

    ColorTable colors = {
        {COLOR_WHITE,COLOR_WHITE},
        {COLOR_GREEN,COLOR_GREEN},
        {COLOR_RED,COLOR_RED}
    };

    initCurses();
    u32_f final_score = game(newCoords(30,20),colors);
    endwin();

    printf("Game over! Your final snake length is %ld!\n",final_score);
    return EXIT_SUCCESS;
}
