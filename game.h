/*
 * game.h
 *
 *  Created on: Dec 7, 2017
 *      Author: alex
 *
 *  Modified by: Dishoungh
 */
#include "msp.h"
#include <stdint.h>
#include "msp_boosterpack_lcd/lcd.h"
#include "adc.h"
#include "msp_boosterpack_lcd/serial.h"

#ifndef GAME_H_
#define GAME_H_

//macros for joystick movement
#define JOY_HIGH_THRESHOLD      10000
#define JOY_LOW_THRESHOLD       6000
//macros for start screen
#define START_SCREEN_HEIGHT     20
#define START_SCREEN_WIDTH      LCD_CHAR_WIDTH * 12
#define START_SCREEN_X          30
#define START_SCREEN_Y          40

//macros for game menu
#define GAME_MENU_X             30
#define GAME_MENU_Y             30
#define GAME_MENU_HEIGHT        40
#define GAME_MENU_WIDTH         LCD_CHAR_WIDTH * 11

#define GAME_SELECT_X           GAME_MENU_X + GAME_MENU_WIDTH
#define GAME_SELECT_Y           GAME_MENU_Y
#define GAME_SELECT_HEIGHT      10
#define GAME_SELECT_WIDTH       10

//macros for pong
#define PONG_PADDLE_WIDTH       10
#define PONG_PADDLE_HEIGHT      40
#define PONG_BALL_WIDTH         10
#define PONG_BALL_HEIGHT        10
#define PONG_INITIAL_BALL_X     50
#define PONG_INITIAL_BALL_Y     50
#define PONG_LEFT_PADDLE_X      117
#define PONG_RIGHT_PADDLE_X     0
#define PONG_INITIAL_PADDLE_Y   50

//macros for dodge game
#define DODGE_BOX_HEIGHT        10
#define DODGE_BOX_WIDTH         10
#define DODGE_PLAYER_RADIUS     5
#define DODGE_PLAYER_START_X    60
#define DODGE_PLAYER_START_Y    60
#define DODGE_OBS_1_START_X     20
#define DODGE_OBS_1_START_Y     20
#define DODGE_OBS_2_START_X     20
#define DODGE_OBS_2_START_Y     90
#define DODGE_OBS_3_START_X     90
#define DODGE_OBS_3_START_Y     20
#define DODGE_OBS_4_START_X     90
#define DODGE_OBS_4_START_Y     90

#define DODGE_OBS_2_DELAY       15
#define DODGE_OBS_3_DELAY       30
#define DODGE_OBS_4_DELAY       45

#define DODGE_OBS_PAUSE_DELAY   200

//macros for debug
#define DEBUG_DIRECTION_X       (LCD_MAX_X-(LCD_CHAR_WIDTH*4))
#define DEBUG_VERTICAL_Y        107
#define DEBUG_HORIZONTAL_Y      87
#define DEBUG_BUTTON_X          (LCD_MAX_X-(LCD_CHAR_WIDTH*6))
#define DEBUG_BUTTON_TOGGLE_X   (LCD_MAX_X-(LCD_CHAR_WIDTH*9))
#define DEBUG_BUTTON_Y          67

//Macros for Pattern game
#define PATTERN_SHOW_DELAY      4000000
#define PATTERN_R1_START_X      85
#define PATTERN_R1_START_Y      50
#define PATTERN_R2_START_X      50
#define PATTERN_R2_START_Y      50
#define PATTERN_R3_START_X      15
#define PATTERN_R3_START_Y      50
#define PATTERN_BOX_WIDTH       20
#define PATTERN_BOX_HEIGHT      20

//Macros for Snake Game
#define SNAKE_TIMER_DELAY       125000
#define SNAKE_BORDER_WIDTH      5
#define SNAKE_BORDER_HEIGHT     5
#define SNAKE_PLAYER_START_X    55
#define SNAKE_PLAYER_START_Y    50
#define SNAKE_PLAYER_WIDTH      5
#define SNAKE_PLAYER_HEIGHT     5
#define SNAKE_PLAYER_START_SIZE 5
#define SNAKE_PLAYER_MAX_SIZE   50
#define SNAKE_FOOD_RADIUS       3

//enums used menu navigation
typedef enum {
    DEBUG,
    PONG,
    DODGE,
    PATTERN,
    SNAKE
}GAME;

typedef enum {
    DEBUG_MODE,
    STANDARD_MODE
}GAME_MODE;

//enums used in dodge game
typedef enum {
    DEAD,
    ALIVE,
    PAUSED
}STATE;

typedef enum
{
    LEFT,
    RIGHT,
    UP,
    DOWN,
    NONE
}DIRECTION;

//Enums used in Pattern
typedef enum
{
    SUCCESS,
    FAIL
}PATTERN_STATE;

//Enums Used for Snake Game
typedef struct
{
    RECT body;
    uint8_t prev_x, prev_y;
}SNAKE_PLAYER;

//variable for reading buttons globally
uint8_t button_flag;

//variables for navigating menu
extern const GAME game_array[5];
GAME current_game;
GAME_MODE pong_mode;
RECT start_screen;

//variables used when in debug mode
uint8_t bit_joy_x_string[4];
uint8_t bit_joy_y_string[4];

//variables for pong game
RECT ball;
RECT right;  //right paddle
RECT left;  //left paddle

int8_t yvelocity;
int8_t xvelocity;

uint8_t score_1;
uint8_t score_2;

uint8_t max_score;

uint8_t at_left;
uint8_t at_right;

uint16_t ball_color;
uint16_t left_paddle_color;
uint16_t right_paddle_color;

uint8_t field_bottom;

//variables for dodge
uint8_t timer_count_string[5];

//function to initialize button interrupts
void initialize_buttons(void);
//function to check a rectangle, rectangle collision
//returns 1 if collision, 0 if no collision
uint8_t check_rect_collision(RECT rect0, RECT rect1);
//function to check a circle, rectangle collision
//returns 1 if collision, 0 if no collision
uint8_t check_rect_circ_collision(RECT rect, CIRCLE circ);
//function to draw the start screen
void draw_start_screen(void);
//function to select what game is going to be played
void select_game(void);
//function to run game given
void run_game(GAME game);
//function to enter debug app
void full_debug(void);

/*functions for pong*/
void initialize_pong(void);
void pong_max_score(void);
void pong_paddle_color(void);
void pong_main(void);
void end_pong(void);

/*functions for dodge*/
void run_dodge(void);

/*functions for pattern*/
void run_pattern(void);

/*functions for snake*/
void run_snake(void);

#endif /* GAME_H_ */
