/*
 * game.c
 *
 *  Created on: Dec 7, 2017
 *      Author: alex
 *
 *  Modified by: Dishoungh
 */
#include "msp.h"
#include "game.h"
#include "msp_boosterpack_lcd/lcd.h"

const GAME game_array[5] = {DEBUG,PONG,DODGE,PATTERN,SNAKE};

void initialize_buttons(void){
    button_flag = 0;

    //upper button setup
    P5->SEL0 &= ~BIT1;
    P5->SEL0 &= ~BIT1;
    P5->DIR &= ~BIT1;
    P5->REN |= BIT1;
    P5->OUT |= BIT1;            //pullup resistor activated
    P5->IFG &= ~BIT1;
    P5->IES |= BIT1;            //falling edge trigger
    P5->IE |= BIT1;             //interrupts enabled
    NVIC_EnableIRQ(PORT5_IRQn);

    //lower button setup
    P3->SEL0 &= ~BIT3;
    P3->SEL0 &= ~BIT3;
    P3->DIR &= ~BIT3;
    P3->REN |= BIT3;
    P3->OUT |= BIT3;            //pullup resistor activated
    P3->IFG &= ~BIT3;
    P3->IES |= BIT3;            //falling edge trigger
    P3->IE |= BIT3;             //interrupts enabled
    NVIC_EnableIRQ(PORT3_IRQn);
}
/*draws start screen*/
void draw_start_screen(void){
    LCD_erase_screen();

    initialize_rectangle(&start_screen, START_SCREEN_X, START_SCREEN_Y, START_SCREEN_WIDTH, START_SCREEN_HEIGHT, BLACK);
    LCD_write_string("PRESS BUTTON", start_screen.x, start_screen.y + LCD_CHAR_HEIGHT, BLACK, 12);
    LCD_write_string("  TO START  ", start_screen.x, start_screen.y, BLACK, 12);
    while(!button_flag);
    start_screen.x -= LCD_CHAR_WIDTH;
    LCD_erase_rectangle(start_screen);
    start_screen.x += LCD_CHAR_WIDTH;
    button_flag = 0;
}

//allows the user to select what game they want to play
void select_game(void){
    LCD_erase_screen();

    current_game = 4;

    RECT game_menu;
    initialize_rectangle(&game_menu,GAME_MENU_X,GAME_MENU_Y,GAME_MENU_WIDTH,GAME_MENU_HEIGHT,BLACK);

    RECT game_select;
    initialize_rectangle(&game_select,GAME_SELECT_X,GAME_SELECT_Y,GAME_SELECT_WIDTH,GAME_SELECT_HEIGHT,RED);
    LCD_draw_rectangle(game_select);

    LCD_write_string("CHOOSE GAME",game_menu.x,game_menu.y + 5*LCD_CHAR_HEIGHT,RED,11);
    LCD_write_string(" DEBUG     ",game_menu.x,game_menu.y + 4*LCD_CHAR_HEIGHT,BLACK,11);
    LCD_write_string(" PONG      ",game_menu.x,game_menu.y + 3*LCD_CHAR_HEIGHT,BLACK,11);
    LCD_write_string(" DODGE     ",game_menu.x,game_menu.y + 2*LCD_CHAR_HEIGHT,BLACK,11);
    LCD_write_string(" PATTERN   ",game_menu.x,game_menu.y + 1*LCD_CHAR_HEIGHT,BLACK,11);
    LCD_write_string(" SNAKE     ",game_menu.x,game_menu.y + 0*LCD_CHAR_HEIGHT,BLACK,11);


    while(!button_flag){
        ADC14->CTL0 |= ADC14_CTL0_SC;       //start ADC conversion
        if(timer_trigger && !timer_delay){
            //if joystick is moved up and the selection is not already at the top, move selection
            if((bit_joy_y > JOY_HIGH_THRESHOLD) && (current_game > 0)){
                LCD_erase_rectangle(game_select);
                //change game selection
                current_game--;
                //change visual selection box
                game_select.y += LCD_CHAR_HEIGHT;
                LCD_draw_rectangle(game_select);
                timer_delay = 10;
            }
            //if joystick is moved down and the selection is not already at the bottom, move selection
            else if((bit_joy_y < JOY_LOW_THRESHOLD) && (current_game < 4)){
                LCD_erase_rectangle(game_select);
                //change game selection
                current_game++;
                //change visual selection box
                game_select.y -= LCD_CHAR_HEIGHT;
                LCD_draw_rectangle(game_select);
                timer_delay = 10;
            }
        }
    }
    LCD_erase_rectangle(game_select);
    game_menu.x -= LCD_CHAR_WIDTH;
    LCD_erase_rectangle(game_menu);
    button_flag = 0;

    LCD_erase_screen();

    timer_delay = 25;
}
/*runs given game*/
void run_game(GAME game){
    switch(game){
    case PONG:
        pong_mode = STANDARD_MODE;
        //have user select winning score for game
        pong_max_score();
        while(timer_delay);

        //have user select color of paddle
        pong_paddle_color();
        while(timer_delay);

        /*initialize and start the pong game*/
        initialize_pong();

        /*enter main pong game section*/
        pong_main();

        /*display winner and end pong*/
        end_pong();

        button_flag = 0;
        break;

    case DODGE:
        run_dodge();
        button_flag = 0;
        break;

    case DEBUG:
        full_debug();
        button_flag = 0;
        break;

    case PATTERN:
        run_pattern();
        button_flag = 0;
        break;

    case SNAKE:
        run_snake();
        button_flag = 0;
        break;
    }
}

/* Runs Snake Game */
void run_snake(void)
{
    // Objects
    RECT left_border, upper_border, right_border, lower_border;
    SNAKE_PLAYER* player[SNAKE_PLAYER_MAX_SIZE];
    CIRCLE food;
    STATE food_state = DEAD, player_state = ALIVE;
    player[0]->dir = RIGHT;
    uint8_t player_size = 1;

    // Borders
    initialize_rectangle(&left_border, (LCD_MAX_X - SNAKE_BORDER_WIDTH), LCD_MIN_Y, SNAKE_BORDER_WIDTH, LCD_MAX_Y, RED);
    initialize_rectangle(&upper_border, LCD_MIN_X, (LCD_MAX_Y - SNAKE_BORDER_HEIGHT), LCD_MAX_X, SNAKE_BORDER_HEIGHT, RED);
    initialize_rectangle(&right_border, LCD_MIN_X, LCD_MIN_Y, SNAKE_BORDER_WIDTH, LCD_MAX_Y, RED);
    initialize_rectangle(&lower_border, LCD_MIN_X, LCD_MIN_Y, LCD_MAX_X, SNAKE_BORDER_HEIGHT, RED);

    LCD_draw_rectangle(left_border);
    LCD_draw_rectangle(upper_border);
    LCD_draw_rectangle(right_border);
    LCD_draw_rectangle(lower_border);

    // Head
    initialize_rectangle(&player[0]->body, SNAKE_PLAYER_START_X, SNAKE_PLAYER_START_Y, SNAKE_PLAYER_WIDTH, SNAKE_PLAYER_HEIGHT, NAVY);
    LCD_draw_rectangle(player[0]->body);

    timer_count = 0;
    timer_delay = 20;
    while(timer_delay);

    //Delete this
    button_flag = 0;

    //enter game loop
    while(player_state == ALIVE)
    {
        ADC14->CTL0 |= ADC14_CTL0_SC;       //start ADC conversion
        if(timer_trigger && ~timer_delay)
        {
            //Player Head Movement
            if((bit_joy_x > JOY_HIGH_THRESHOLD)) //Right
            {
                player[0]->dir = RIGHT;
            }
            else if((bit_joy_x < JOY_LOW_THRESHOLD)) //Left
            {
                player[0]->dir = LEFT;
            }
            if((bit_joy_y > JOY_HIGH_THRESHOLD)) //Up
            {
                player[0]->dir = UP;
            }
            else if((bit_joy_y < JOY_LOW_THRESHOLD)) //Down
            {
                player[0]->dir = DOWN;
            }

            // Move Snake
            for (uint8_t j = 0; j < player_size; j++)
            {
                player[j]->prev_x = player[j]->body.x;
                player[j]->prev_y = player[j]->body.y;
                // Move Head according to direction
                if (!j)
                {
                    switch(player[j]->dir)
                    {
                        case RIGHT:
                            LCD_erase_rectangle(player[j]->body);
                            (player[j]->body).x -= 5;
                            LCD_draw_rectangle(player[j]->body);
                            break;
                        case LEFT:
                            LCD_erase_rectangle(player[j]->body);
                            (player[j]->body).x += 5;
                            LCD_draw_rectangle(player[j]->body);
                            break;
                        case UP:
                            LCD_erase_rectangle(player[j]->body);
                            (player[j]->body).y += 5;
                            LCD_draw_rectangle(player[j]->body);
                            break;
                        case DOWN:
                            LCD_erase_rectangle(player[j]->body);
                            (player[j]->body).y -= 5;
                            LCD_draw_rectangle(player[j]->body);
                            break;
                        default:
                            break;
                    }
                }
            }

            /*
             *  else
                {
                    if(player[j-1]->prev_x > player[j]->body.x) // Move Tail to the Left
                    {
                        player[j]->dir = LEFT;
                        LCD_erase_rectangle(player[j]->body);
                        player[j]->body.x += 5;
                        LCD_draw_rectangle(player[j]->body);
                    }
                    else if(player[j-1]->prev_x < player[j]->body.x) // Move Tail to the Right
                    {
                        player[j]->dir = RIGHT;
                        LCD_erase_rectangle(player[j]->body);
                        player[j]->body.x -= 5;
                        LCD_draw_rectangle(player[j]->body);
                    }
                    else if(player[j-1]->prev_y > player[j]->body.y) // Move Tail Up
                    {
                        player[j]->dir = UP;
                        LCD_erase_rectangle(player[j]->body);
                        player[j]->body.y += 5;
                        LCD_draw_rectangle(player[j]->body);
                    }
                    else if(player[j-1]->prev_y < player[j]->body.y) // Move Tail Down
                    {
                        player[j]->dir = DOWN;
                        LCD_erase_rectangle(player[j]->body);
                        player[j]->body.y -= 5;
                        LCD_draw_rectangle(player[j]->body);
                    }

                    if (check_rect_collision(player[0]->body, player[j]->body)) //Kill Player
                    {
                        //player_state = DEAD;
                        break;
                    }
                }
            //Spawn food
            if (food_state == DEAD)
            {
                food_state = ALIVE;

                //"Random"-ish spawn until food doesn't spawn inside border (Replace with srand())
                do
                {
                    initialize_circle(&food, (timer_count % LCD_MAX_X), (timer_count % LCD_MAX_Y), SNAKE_FOOD_RADIUS, BLUE, BLUE);
                }while(check_rect_circ_collision(left_border, food) || check_rect_circ_collision(upper_border, food) || check_rect_circ_collision(right_border, food) || check_rect_circ_collision(lower_border, food));
                LCD_draw_circle(food);
            }


            // Check player collision with borders

            if (check_rect_collision(player[0]->body, left_border) || check_rect_collision(player[0]->body, upper_border) || check_rect_collision(player[0]->body, right_border) || check_rect_collision(player[0]->body, lower_border))
            {
                player_state = DEAD;
            }

            //Snake eats food
            if (check_rect_circ_collision(player[0]->body, food))
            {
                food_state = DEAD;
                LCD_erase_circle(food);
                if (player_size < SNAKE_PLAYER_MAX_SIZE)
                {
                    //Snake grows
                    initialize_rectangle(&player[player_size]->body, player[player_size-1]->prev_x, player[player_size-1]->prev_y, SNAKE_PLAYER_WIDTH, SNAKE_PLAYER_HEIGHT, BLUE);
                    LCD_draw_rectangle(player[player_size]->body);
                    player_size++;
                }
            }
            */

            if(button_flag)
            {
                player_state = DEAD;
            }

            //Delay for a little bit
            for (uint32_t i = 0; i < SNAKE_TIMER_DELAY; i++);
        }
    }

    //print ending screen
    LCD_erase_screen();
    LCD_write_string("GAME OVER",40,60,RED,9);
    button_flag = 0;
    while(!button_flag);
}

/* Runs Pattern Game */
void run_pattern(void)
{
    RECT r1;
    initialize_rectangle(&r1, PATTERN_R1_START_X, PATTERN_R1_START_Y, PATTERN_BOX_WIDTH, PATTERN_BOX_HEIGHT, BLACK);
    LCD_draw_rectangle(r1);

    RECT r2;
    initialize_rectangle(&r2, PATTERN_R2_START_X, PATTERN_R2_START_Y, PATTERN_BOX_WIDTH, PATTERN_BOX_HEIGHT, BLACK);
    LCD_draw_rectangle(r2);

    RECT r3;
    initialize_rectangle(&r3, PATTERN_R3_START_X, PATTERN_R3_START_Y, PATTERN_BOX_WIDTH, PATTERN_BOX_HEIGHT, BLACK);
    LCD_draw_rectangle(r3);

    uint8_t* cpu_picks[3], player_picks[3];
    uint8_t i, score = 0;
    PATTERN_STATE state = SUCCESS;
    timer_count = 0;

    /* Game Loop */
    while (state == SUCCESS)
    {
        //Create Pattern and Show Pattern
        for (i = 0; i < 3; i++)
        {
            cpu_picks[i] = (uint8_t)(timer_count % 3);

            if (cpu_picks[i] == 0)                  //Picks first box
            {
                LCD_erase_rectangle(r1);
                initialize_rectangle(&r1, PATTERN_R1_START_X, PATTERN_R1_START_Y, PATTERN_BOX_WIDTH, PATTERN_BOX_HEIGHT, GREEN);
                LCD_draw_rectangle(r1);
            }
            else if(cpu_picks[i] == 1)              //Picks second box
            {
                LCD_erase_rectangle(r2);
                initialize_rectangle(&r2, PATTERN_R2_START_X, PATTERN_R2_START_Y, PATTERN_BOX_WIDTH, PATTERN_BOX_HEIGHT, GREEN);
                LCD_draw_rectangle(r2);
            }
            else                                //Picks third box
            {
                LCD_erase_rectangle(r3);
                initialize_rectangle(&r3, PATTERN_R3_START_X, PATTERN_R3_START_Y, PATTERN_BOX_WIDTH, PATTERN_BOX_HEIGHT, GREEN);
                LCD_draw_rectangle(r3);
            }

            //Delay for a little while
            for(uint32_t j = 0; j < PATTERN_SHOW_DELAY; j++);

            //Re-color Selected Rectangle Back to Black
            if (cpu_picks[i] == 0)                  //Picks first box
            {
                LCD_erase_rectangle(r1);
                initialize_rectangle(&r1, PATTERN_R1_START_X, PATTERN_R1_START_Y, PATTERN_BOX_WIDTH, PATTERN_BOX_HEIGHT, BLACK);
                LCD_draw_rectangle(r1);
            }
            else if(cpu_picks[i] == 1)              //Picks second box
            {
                LCD_erase_rectangle(r2);
                initialize_rectangle(&r2, PATTERN_R2_START_X, PATTERN_R2_START_Y, PATTERN_BOX_WIDTH, PATTERN_BOX_HEIGHT, BLACK);
                LCD_draw_rectangle(r2);
            }
            else                                //Picks third box
            {
                LCD_erase_rectangle(r3);
                initialize_rectangle(&r3, PATTERN_R3_START_X, PATTERN_R3_START_Y, PATTERN_BOX_WIDTH, PATTERN_BOX_HEIGHT, BLACK);
                LCD_draw_rectangle(r3);
            }
        }

        uint8_t current_choice = 0;
        LCD_erase_rectangle(r1);
        initialize_rectangle(&r1, PATTERN_R1_START_X, PATTERN_R1_START_Y, PATTERN_BOX_WIDTH, PATTERN_BOX_HEIGHT, NAVY);
        LCD_draw_rectangle(r1);

        timer_delay = 0;
        button_flag = 0;

        //Pick Boxes to recreate pattern
        for (i = 0; i < 3; i++)
        {
            while(!button_flag)
            {
                ADC14->CTL0 |= ADC14_CTL0_SC;       //start ADC conversion
                if(timer_trigger && !timer_delay)
                {
                    // Move highlight by joystick
                    if((bit_joy_x > JOY_HIGH_THRESHOLD) && (current_choice < 2)) //Select Right
                    {
                        current_choice++;

                        if (current_choice == 1)
                        {
                            LCD_erase_rectangle(r1);
                            initialize_rectangle(&r1, PATTERN_R1_START_X, PATTERN_R1_START_Y, PATTERN_BOX_WIDTH, PATTERN_BOX_HEIGHT, BLACK);
                            LCD_draw_rectangle(r1);

                            LCD_erase_rectangle(r2);
                            initialize_rectangle(&r2, PATTERN_R2_START_X, PATTERN_R2_START_Y, PATTERN_BOX_WIDTH, PATTERN_BOX_HEIGHT, NAVY);
                            LCD_draw_rectangle(r2);

                            LCD_erase_rectangle(r3);
                            initialize_rectangle(&r3, PATTERN_R3_START_X, PATTERN_R3_START_Y, PATTERN_BOX_WIDTH, PATTERN_BOX_HEIGHT, BLACK);
                            LCD_draw_rectangle(r3);
                        }
                        else if (current_choice == 2)
                        {
                            LCD_erase_rectangle(r1);
                            initialize_rectangle(&r1, PATTERN_R1_START_X, PATTERN_R1_START_Y, PATTERN_BOX_WIDTH, PATTERN_BOX_HEIGHT, BLACK);
                            LCD_draw_rectangle(r1);

                            LCD_erase_rectangle(r2);
                            initialize_rectangle(&r2, PATTERN_R2_START_X, PATTERN_R2_START_Y, PATTERN_BOX_WIDTH, PATTERN_BOX_HEIGHT, BLACK);
                            LCD_draw_rectangle(r2);

                            LCD_erase_rectangle(r3);
                            initialize_rectangle(&r3, PATTERN_R3_START_X, PATTERN_R3_START_Y, PATTERN_BOX_WIDTH, PATTERN_BOX_HEIGHT, NAVY);
                            LCD_draw_rectangle(r3);
                        }
                        timer_delay = 10;
                    }
                    else if((bit_joy_x < JOY_LOW_THRESHOLD) && (current_choice > 0)) //Select Left
                    {
                        current_choice--;

                        if (current_choice == 0)
                        {
                            LCD_erase_rectangle(r1);
                            initialize_rectangle(&r1, PATTERN_R1_START_X, PATTERN_R1_START_Y, PATTERN_BOX_WIDTH, PATTERN_BOX_HEIGHT, NAVY);
                            LCD_draw_rectangle(r1);

                            LCD_erase_rectangle(r2);
                            initialize_rectangle(&r2, PATTERN_R2_START_X, PATTERN_R2_START_Y, PATTERN_BOX_WIDTH, PATTERN_BOX_HEIGHT, BLACK);
                            LCD_draw_rectangle(r2);

                            LCD_erase_rectangle(r3);
                            initialize_rectangle(&r3, PATTERN_R3_START_X, PATTERN_R3_START_Y, PATTERN_BOX_WIDTH, PATTERN_BOX_HEIGHT, BLACK);
                            LCD_draw_rectangle(r3);
                        }
                        else if (current_choice == 1)
                        {
                            LCD_erase_rectangle(r1);
                            initialize_rectangle(&r1, PATTERN_R1_START_X, PATTERN_R1_START_Y, PATTERN_BOX_WIDTH, PATTERN_BOX_HEIGHT, BLACK);
                            LCD_draw_rectangle(r1);

                            LCD_erase_rectangle(r2);
                            initialize_rectangle(&r2, PATTERN_R2_START_X, PATTERN_R2_START_Y, PATTERN_BOX_WIDTH, PATTERN_BOX_HEIGHT, NAVY);
                            LCD_draw_rectangle(r2);

                            LCD_erase_rectangle(r3);
                            initialize_rectangle(&r3, PATTERN_R3_START_X, PATTERN_R3_START_Y, PATTERN_BOX_WIDTH, PATTERN_BOX_HEIGHT, BLACK);
                            LCD_draw_rectangle(r3);
                        }
                        timer_delay = 10;
                    }
                }
            }
            button_flag = 0;
            player_picks[i] = current_choice;
        }

        //Compare patterns
        for (i = 0; i < 3; i++)
        {
            if (cpu_picks[i] != player_picks[i])
            {
                state = FAIL;
                break;
            }
        }

        if (state != FAIL)
        {
            score++;
        }
    }

    //Print Score
    LCD_erase_screen();


    uint8_t score_string[5] = "     ";
    itoa(score,score_string);

    //print ending screen
    LCD_write_string("YOU LOST",40,60,RED,9);
    LCD_write_string("SCORE:",55,40,NAVY,11);
    LCD_write_string(score_string,15,40,NAVY,sizeof(score_string)/sizeof(uint8_t));

    button_flag = 0;

    while(!button_flag);
}


/*function that runs the debug app*/
void full_debug(void){
    uint8_t button_toggle = 0;
    CIRCLE test_circle;
    initialize_circle(&test_circle, 50, 25, 20, BLACK, RED);
    LCD_draw_circle(test_circle);
    /*
    LCD_draw_vline(70, 5, 45, RED);
    LCD_draw_vline(30, 5, 45, RED);
    LCD_draw_hline(5, RED);
    LCD_draw_hline(45, RED);*/

    while(1){
        ADC14->CTL0 |= ADC14_CTL0_SC;       //start ADC conversion
        if(timer_trigger && ~timer_delay){
            //Joy Vertical
            if(bit_joy_y > JOY_HIGH_THRESHOLD){
                //Vertical Up
                LCD_write_string("V UP", DEBUG_DIRECTION_X, DEBUG_VERTICAL_Y, BLACK, 4);
            }
            else if(bit_joy_y < JOY_LOW_THRESHOLD){
                //Vertical Down
                LCD_write_string("V DN", DEBUG_DIRECTION_X, DEBUG_VERTICAL_Y, BLACK, 4);
            }
            else{
                //Vertical Mid
                LCD_write_string("V MD", DEBUG_DIRECTION_X, DEBUG_VERTICAL_Y, BLACK, 4);
            }

            //Joy Horizontal
            if(bit_joy_x > JOY_HIGH_THRESHOLD){
                //Horizontal Right
                LCD_write_string("H RT", DEBUG_DIRECTION_X, DEBUG_HORIZONTAL_Y, BLACK, 4);
            }
            else if(bit_joy_x < JOY_LOW_THRESHOLD){
                //Horizontal Left
                LCD_write_string("H LT", DEBUG_DIRECTION_X, DEBUG_HORIZONTAL_Y, BLACK, 4);
            }
            else{
                //Horizontal Mid
                LCD_write_string("H MD", DEBUG_DIRECTION_X, DEBUG_HORIZONTAL_Y, BLACK, 4);
            }

            //write actual joystick bit values
            uint8_t i = 0;
            for(i=0; i < sizeof(bit_joy_y_string)/sizeof(uint8_t); i++){
                bit_joy_y_string[i] = ' ';
            }
            itoa(bit_joy_y,bit_joy_y_string);
            LCD_write_string(bit_joy_y_string,10,DEBUG_VERTICAL_Y,BLACK,4);
            for(i=0; i < sizeof(bit_joy_x_string)/sizeof(uint8_t); i++){
                bit_joy_x_string[i] = ' ';
            }
            itoa(bit_joy_x,bit_joy_x_string);
            LCD_write_string(bit_joy_x_string,10,DEBUG_HORIZONTAL_Y,BLACK,4);

            //Button
            LCD_write_string("BUTTON", DEBUG_BUTTON_X, DEBUG_BUTTON_Y, BLACK, 6);
            if(button_flag){
                //Yes Button
                button_toggle ^= 1;
                button_flag = 0;
            }
            LCD_write_character(char_lib_num[button_toggle],DEBUG_BUTTON_TOGGLE_X, DEBUG_BUTTON_Y, BLACK);
            timer_trigger = 0;
        }
        button_flag = 0;
    }
}

/*initializes the parameters for pong*/
void initialize_pong(void){
    yvelocity = 0;
    xvelocity = 1;

    score_1 = 0;
    score_2 = 0;

    at_left = 0;
    at_right = 0;

    ball_color = BLACK;

    if(pong_mode == DEBUG_MODE){
        field_bottom = 22;
        LCD_draw_hline(21,BLACK);
    }
    else{
        field_bottom = 11;
    }


    initialize_rectangle(&ball, PONG_INITIAL_BALL_X, PONG_INITIAL_BALL_Y, PONG_BALL_WIDTH, PONG_BALL_HEIGHT, ball_color);
    LCD_draw_rectangle(ball);

    initialize_rectangle(&right, PONG_RIGHT_PADDLE_X, PONG_INITIAL_PADDLE_Y, PONG_PADDLE_WIDTH, PONG_PADDLE_HEIGHT, color_array[right_paddle_color]);
    LCD_draw_rectangle(right);

    initialize_rectangle(&left, PONG_LEFT_PADDLE_X, PONG_INITIAL_PADDLE_Y, PONG_PADDLE_WIDTH, PONG_PADDLE_HEIGHT, color_array[left_paddle_color]);
    LCD_draw_rectangle(left);

    //draw a line to separate the field and scoreboard
    LCD_draw_hline(10,BLACK);

    LCD_write_character(char_lib_num[0],10,0,color_array[right_paddle_color]);
    LCD_write_character(char_lib_num[0],110,0,color_array[left_paddle_color]);

    uint8_t scoreboard_string[] = "FIRST TO ";
    LCD_write_string(scoreboard_string,70 - (7*sizeof(scoreboard_string)/2),0,BLACK,sizeof(scoreboard_string)/sizeof(uint8_t));
    LCD_write_character(char_lib_num[max_score],70 - (7*sizeof(scoreboard_string)/2) - 7,0,BLACK);

    button_flag = 0;
}

/*code to choose the maximum score*/
void pong_max_score(void){
    max_score = 5;
    LCD_write_string(" MAX SCORE? ",start_screen.x,start_screen.y + 10,BLACK,12);
    LCD_write_character(char_lib_num[max_score],60,start_screen.y,BLACK);

    while(!button_flag){
        ADC14->CTL0 |= ADC14_CTL0_SC;       //start ADC conversion
        if ((timer_trigger == 1) && !timer_delay){
            if(bit_joy_y > JOY_HIGH_THRESHOLD){
                max_score++;
                max_score = max_score % 10;                 //max score cannot be any larger than 10
                LCD_write_character(char_lib_num[max_score],60,start_screen.y,BLACK);
                timer_delay = 10;
            }
            if((bit_joy_y < JOY_LOW_THRESHOLD)){
                if(max_score == 0){
                    max_score = 9;
                }
                else{
                    max_score--;
                }
                LCD_write_character(char_lib_num[max_score],60,start_screen.y,BLACK);
                timer_delay = 10;
            }

            if(pong_mode == DEBUG_MODE){
                //write actual joystick bit values
                itoa(bit_joy_y,bit_joy_y_string);
                LCD_write_string(bit_joy_y_string,100,11,BLACK,4);

                itoa(bit_joy_x,bit_joy_x_string);
                LCD_write_string(bit_joy_x_string,10,11,BLACK,4);
            }
            timer_trigger = 0;
        }
    }
    start_screen.x -= 7;
    LCD_erase_rectangle(start_screen);
    start_screen.x += 7;
    button_flag = 0;
    timer_delay = 15;
}
/*choose the color of the paddle*/
void pong_paddle_color(void){
    left_paddle_color = 9;
    right_paddle_color = 8;

    LCD_write_string("SELECT COLOR",start_screen.x,start_screen.y + 10,BLACK,12);

    initialize_rectangle(&right, PONG_RIGHT_PADDLE_X, PONG_INITIAL_PADDLE_Y, PONG_PADDLE_WIDTH, PONG_PADDLE_HEIGHT, color_array[right_paddle_color]);
    LCD_draw_rectangle(right);

    initialize_rectangle(&left, PONG_LEFT_PADDLE_X, PONG_INITIAL_PADDLE_Y, PONG_PADDLE_WIDTH, PONG_PADDLE_HEIGHT, color_array[left_paddle_color]);
    LCD_draw_rectangle(left);

    while(!button_flag){
        ADC14->CTL0 |= ADC14_CTL0_SC;       //start ADC conversion
        if ((timer_trigger == 1) && !timer_delay){
            if(bit_joy_y > JOY_HIGH_THRESHOLD){
                left_paddle_color++;
                left_paddle_color = left_paddle_color % 14;
                left.color = color_array[left_paddle_color];
                LCD_draw_rectangle(left);
                timer_delay = 10;
            }
            else if(bit_joy_y < JOY_LOW_THRESHOLD){
                if(left_paddle_color == 0){
                    left_paddle_color = 13;
                }
                else{
                    left_paddle_color--;
                }
                left.color = color_array[left_paddle_color];
                LCD_draw_rectangle(left);
                timer_delay = 10;
            }
            if(bit_joy_x > JOY_HIGH_THRESHOLD){
                right_paddle_color++;
                right_paddle_color = right_paddle_color % 14;
                right.color = color_array[right_paddle_color];
                LCD_draw_rectangle(right);
                timer_delay = 10;
            }
            else if(bit_joy_x < JOY_LOW_THRESHOLD){
                if(right_paddle_color == 0){
                    right_paddle_color = 13;
                }
                else{
                    right_paddle_color--;
                }
                right.color = color_array[right_paddle_color];
                LCD_draw_rectangle(right);
                timer_delay = 10;
            }

            if(pong_mode == DEBUG_MODE){
                //write actual joystick bit values
                itoa(bit_joy_y,bit_joy_y_string);
                LCD_write_string(bit_joy_y_string,100,11,BLACK,4);

                itoa(bit_joy_x,bit_joy_x_string);
                LCD_write_string(bit_joy_x_string,10,11,BLACK,4);
            }
            timer_trigger = 0;
        }
    }
    start_screen.x -= 7;
    LCD_erase_rectangle(start_screen);
    start_screen.x += 7;
    button_flag = 0;
    timer_delay = 15;
}

/*run actual pong game*/
void pong_main(void){
    while((score_1 < max_score) && (score_2 < max_score)){
        ADC14->CTL0 |= ADC14_CTL0_SC;       //start ADC conversion
        if ((timer_trigger == 1) && !timer_delay){
            LCD_erase_rectangle(ball);

            if(pong_mode == DEBUG_MODE){
                //write actual joystick bit values
                itoa(bit_joy_y,bit_joy_y_string);
                LCD_write_string(bit_joy_y_string,100,11,BLACK,4);

                itoa(bit_joy_x,bit_joy_x_string);
                LCD_write_string(bit_joy_x_string,10,11,BLACK,4);
            }

            if((ball.y <= field_bottom) || (ball.y >= (LCD_MAX_Y - ball.height))){
                yvelocity *= -1;
            }
            /*is ball at left edge of field?*/
            if((ball.x >= left.x - ball.width - 1)){
                //if the paddle is there to block ball reverse the direction, if not count score for player 2
                if ((ball.y > (left.y - ball.height)) && (ball.y < (left.y + left.height))){
                    xvelocity *= -1;
                    at_left = 1;
                }
                else {
                    ball.x = 50;
                    ball.y = 50;
                    xvelocity = -1;
                    yvelocity = 0;
                    score_2++;
                    LCD_write_character(char_lib_num[score_2],10,0,color_array[right_paddle_color]);
                    timer_delay = 25;
                }
            }
            /*is ball at right edge of field?*/
            if ((ball.x <= right.width + 1)){
                //if baddle is there to block ball reverse the direction, if not count score for player 1
                if ((ball.y > (right.y - ball.height)) && (ball.y < (right.y + right.height))){
                    xvelocity *= -1;
                    at_right = 1;
                }
                else {
                    ball.x = 50;
                    ball.y = 50;
                    xvelocity = 1;
                    yvelocity = 0;
                    score_1++;
                    LCD_write_character(char_lib_num[score_1],110,0,color_array[left_paddle_color]);
                    timer_delay = 25;
                }
            }
            ball.y += yvelocity;
            ball.x += xvelocity;
            LCD_draw_rectangle(ball);

            //left paddle movement with joystick
            if((bit_joy_y > JOY_HIGH_THRESHOLD) && (left.y != (LCD_MAX_Y - left.height))){  //up && not top
                //move up
                LCD_erase_rectangle(left);
                left.y++;
                LCD_draw_rectangle(left);
                if(at_left){        //if ball is at paddle and paddle is moving, change velocity of ball
                    yvelocity++;
                }
            }
            if((bit_joy_y < JOY_LOW_THRESHOLD) && (left.y >= (field_bottom +1))){  //down && not bottom
                //move down
                LCD_erase_rectangle(left);
                left.y--;
                LCD_draw_rectangle(left);
                if(at_left){        //if ball is at paddle and paddle is moving, change velocity of ball
                    yvelocity--;
                }
            }

            //right - sloppily done with joystick x axis until we have second joystick
            if((bit_joy_x > JOY_HIGH_THRESHOLD) && (right.y != (LCD_MAX_X - right.height))){  //up (right) && not top
                //move up
                LCD_erase_rectangle(right);
                right.y++;
                LCD_draw_rectangle(right);
                if(at_right){        //if ball is at paddle and paddle is moving, change velocity of ball
                    yvelocity++;
                }
            }
            if((bit_joy_x < JOY_LOW_THRESHOLD) && (right.y >= (field_bottom + 1))){  //down (left) && not bottom
                //move down
                LCD_erase_rectangle(right);
                right.y--;
                LCD_draw_rectangle(right);
                if(at_left){        //if ball is at paddle and paddle is moving, change velocity of ball
                    yvelocity--;
                }
            }

            at_right = 0;
            at_left = 0;
            //draw a line to separate the field and scoreboard
            LCD_draw_hline(10,BLACK);



            timer_trigger = 0;
            button_flag = 0;
        }
    }
}
 /*ends the pong game*/
void end_pong(void){
    LCD_erase_screen();
    initialize_rectangle(&start_screen,23,40,LCD_CHAR_WIDTH*14,20,BLACK);
    if(score_1 > score_2){
        LCD_write_string("PLAYER 1 WINS!",start_screen.x,start_screen.y,BLACK,14);
    }
    else{
        LCD_write_string("PLAYER 2 WINS!",start_screen.x,start_screen.y,BLACK,14);
    }
    while(!button_flag);
    start_screen.x -= 7;
    LCD_erase_rectangle(start_screen);
    start_screen.x += 7;
    button_flag = 0;
}

//game to avoid the red enemies for as long as possible
void run_dodge(void)
{

    uint8_t obs_speed = 1;

    //initialize player state and obstacle states
    STATE player_state = ALIVE;
    STATE obs_1_state  = ALIVE;
    STATE obs_2_state  = DEAD;
    STATE obs_3_state  = DEAD;
    STATE obs_4_state  = DEAD;
    STATE bullet_state = DEAD;

    //initialize player and obstacle rectangles
    CIRCLE player;
    DIRECTION xdir;
    DIRECTION ydir;
    initialize_circle(&player,DODGE_PLAYER_START_X,DODGE_PLAYER_START_Y,DODGE_PLAYER_RADIUS,NAVY,NAVY);
    LCD_draw_circle(player);

    RECT obs_1;
    initialize_rectangle(&obs_1,DODGE_OBS_1_START_X,DODGE_OBS_1_START_Y,DODGE_BOX_WIDTH,DODGE_BOX_HEIGHT,RED);
    LCD_draw_rectangle(obs_1);
    int8_t obs_1_x_velocity = obs_speed;
    int8_t obs_1_y_velocity = obs_speed;
    uint8_t obs_1_pause     = 0;

    RECT obs_2;
    initialize_rectangle(&obs_2,DODGE_OBS_2_START_X,DODGE_OBS_2_START_Y,DODGE_BOX_WIDTH,DODGE_BOX_HEIGHT,RED);
    int8_t obs_2_x_velocity = -1 * obs_speed;
    int8_t obs_2_y_velocity = -1 * obs_speed;
    uint8_t obs_2_pause     = 0;

    RECT obs_3;
    initialize_rectangle(&obs_3,DODGE_OBS_3_START_X,DODGE_OBS_3_START_Y,DODGE_BOX_WIDTH,DODGE_BOX_HEIGHT,RED);
    int8_t obs_3_x_velocity = -1 * obs_speed;
    int8_t obs_3_y_velocity = obs_speed;
    uint8_t obs_3_pause     = 0;

    RECT obs_4;
    initialize_rectangle(&obs_4,DODGE_OBS_4_START_X,DODGE_OBS_4_START_Y,DODGE_BOX_WIDTH,DODGE_BOX_HEIGHT,RED);
    int8_t obs_4_x_velocity = -1 * obs_speed;
    int8_t obs_4_y_velocity = obs_speed;
    uint8_t obs_4_pause     = 0;

    // Initialize Bullet
    CIRCLE bullet;
    int8_t bullet_x_velocity;
    int8_t bullet_y_velocity;

    LCD_write_string("TIME:",80,0,BLACK,5);

    LCD_draw_hline(10,BLACK);

    timer_count = 0;
    timer_delay = 20;
    while(timer_delay);

    //enter game loop
    while(player_state == ALIVE)
    {
        ADC14->CTL0 |= ADC14_CTL0_SC;       //start ADC conversion
        if(timer_trigger)
        {
            xdir = NONE;
            ydir = NONE;

            //logic to move player around
            if((bit_joy_x > JOY_HIGH_THRESHOLD) && (player.x >= 1))
            {
                LCD_erase_circle(player);
                player.x--;
                LCD_draw_circle(player);
                xdir = LEFT;
            }
            else if((bit_joy_x < JOY_LOW_THRESHOLD) && player.x <= (LCD_MAX_X-player.radius))
            {
                LCD_erase_circle(player);
                player.x++;
                LCD_draw_circle(player);
                xdir = RIGHT;
            }
            if((bit_joy_y > JOY_HIGH_THRESHOLD) && player.y <= (LCD_MAX_Y-player.radius))
            {
                LCD_erase_circle(player);
                player.y++;
                LCD_draw_circle(player);
                ydir = UP;
            }
            else if((bit_joy_y < JOY_LOW_THRESHOLD) && ((player.y - player.radius) >= 12))
            {
                LCD_erase_circle(player);
                player.y--;
                LCD_draw_circle(player);
                ydir = DOWN;
            }

            //logic for movement of obstacle 1
            LCD_erase_rectangle(obs_1);
            if(obs_1.x <= 0)
                obs_1_x_velocity = obs_speed;
            else if(obs_1.x >= (LCD_MAX_X - obs_1.width))
                obs_1_x_velocity = -1 * obs_speed;
            if(obs_1.y <= 11)
                obs_1_y_velocity = obs_speed;
            else if(obs_1.y >= (LCD_MAX_Y - obs_1.height))
                obs_1_y_velocity = -1 * obs_speed;
            if (obs_1_state != PAUSED)
            {
                obs_1.x += obs_1_x_velocity;
                obs_1.y += obs_1_y_velocity;
            }
            LCD_draw_rectangle(obs_1);

            //logic for movement of obstacle 2
            if(obs_2_state != DEAD)
            {
                LCD_erase_rectangle(obs_2);
                if(obs_2.x <= 0)
                    obs_2_x_velocity = obs_speed;
                else if(obs_2.x >= (LCD_MAX_X - obs_2.width))
                    obs_2_x_velocity = -1 * obs_speed;
                if(obs_2.y <= 11)
                    obs_2_y_velocity = obs_speed;
                else if(obs_2.y >= (LCD_MAX_Y - obs_2.height))
                    obs_2_y_velocity = -1 * obs_speed;
                if (obs_2_state != PAUSED)
                {
                    obs_2.x += obs_2_x_velocity;
                    obs_2.y += obs_2_y_velocity;
                }
                LCD_draw_rectangle(obs_2);
            }
            //after a certain amount of time, add obstacle 2
            else if(timer_count >= DODGE_OBS_2_DELAY)
            {
                obs_2_state = ALIVE;
            }

            //logic for movement of obstacle 3
            if(obs_3_state != DEAD)
            {
                LCD_erase_rectangle(obs_3);
                if(obs_3.x <= 0)
                {
                    obs_3_x_velocity = obs_speed;
                }
                else if(obs_3.x >= (LCD_MAX_X - obs_3.width))
                {
                    obs_3_x_velocity = -1 * obs_speed;
                }
                if(obs_3.y <= 11)
                {
                    obs_3_y_velocity = obs_speed;
                }
                else if(obs_3.y >= (LCD_MAX_Y - obs_3.height))
                {
                    obs_3_y_velocity = -1 * obs_speed;
                }
                if (obs_3_state != PAUSED)
                {
                    obs_3.x += obs_3_x_velocity;
                    obs_3.y += obs_3_y_velocity;
                }
                LCD_draw_rectangle(obs_3);
            }
            //after a certain amount of time, add obstacle 3
            else if(timer_count >= DODGE_OBS_3_DELAY)
            {
                obs_3_state = ALIVE;
            }

            //logic for movement of obstacle 4
            if(obs_4_state == ALIVE)
            {
                LCD_erase_rectangle(obs_4);
                if(obs_4.x <= 0){
                    obs_4_x_velocity = obs_speed;
                }
                else if(obs_4.x >= (LCD_MAX_X - obs_4.width)){
                    obs_4_x_velocity = -1 * obs_speed;
                }
                if(obs_4.y <= 11){
                    obs_4_y_velocity = obs_speed;
                }
                else if(obs_4.y >= (LCD_MAX_Y - obs_4.height)){
                    obs_4_y_velocity = -1 * obs_speed;
                }
                if (obs_4_state != PAUSED)
                {
                    obs_4.x += obs_4_x_velocity;
                    obs_4.y += obs_4_y_velocity;
                }
                LCD_draw_rectangle(obs_4);
            }
            //after a certain amount of time, add obstacle 3
            else if(timer_count >= DODGE_OBS_4_DELAY)
            {
                obs_4_state = ALIVE;
            }

            //check collisions with obstacles, if a collision then the player is dead
            if(check_rect_circ_collision(obs_1, player))
            {
                player_state = DEAD;
            }
            else if(obs_2_state == ALIVE && check_rect_circ_collision(obs_2, player))
            {
                player_state = DEAD;
            }
            else if(obs_3_state == ALIVE && check_rect_circ_collision(obs_3, player))
            {
                player_state = DEAD;
            }
            else if(obs_4_state == ALIVE && check_rect_circ_collision(obs_4, player))
            {
                player_state = DEAD;
            }

            //if the game goes long enough increase the speed
            if(timer_count == 100)
            {
                obs_speed = 2;
            }

            //Spawn Bullet
            if (button_flag && (bullet_state == DEAD) && ((xdir != NONE) || (ydir != NONE)))
            {
                bullet_state = ALIVE;
                initialize_circle(&bullet,player.x,player.y,DODGE_PLAYER_RADIUS-2,RED,RED);

                if (xdir == LEFT)
                    bullet_x_velocity = 2 * -1 * obs_speed;
                else if (xdir == RIGHT)
                    bullet_x_velocity = 2 * obs_speed;
                else
                    bullet_x_velocity = 0;

                if (ydir == UP)
                    bullet_y_velocity = 2 * obs_speed;
                else if (ydir == DOWN)
                    bullet_y_velocity = 2 * -1 * obs_speed;
                else
                    bullet_y_velocity = 0;


                button_flag = 0;
            }

            if (bullet_state == ALIVE)
            {
                /* Checks Collision with Borders or any Obstacles; If so, destroy bullet */
                if (check_rect_circ_collision(obs_1, bullet))          // Collision with Obstacle 1
                {
                    LCD_erase_circle(bullet);
                    bullet_state = DEAD;

                    // Stop Obstacle 1 for a brief period of time
                    obs_1_state = PAUSED;
                }
                else if (check_rect_circ_collision(obs_2, bullet))     // Collision with Obstacle 2
                {
                    LCD_erase_circle(bullet);
                    bullet_state = DEAD;

                    // Stop Obstacle 2 for a brief period of time
                    obs_2_state = PAUSED;
                }
                else if (check_rect_circ_collision(obs_3, bullet))     // Collision with Obstacle 3
                {
                    LCD_erase_circle(bullet);
                    bullet_state = DEAD;

                    // Stop Obstacle 3 for a brief period of time
                    obs_3_state = PAUSED;
                }
                else if (check_rect_circ_collision(obs_4, bullet))     // Collision with Obstacle 4
                {
                    LCD_erase_circle(bullet);
                    bullet_state = DEAD;

                    // Stop Obstacle 4 for a brief period of time
                    obs_4_state = PAUSED;
                }
                else if (((bullet.x <= 0) || (bullet.x >= (LCD_MAX_X - bullet.radius)) || (bullet.y <= 11) || (bullet.y >= (LCD_MAX_Y - bullet.radius)))) // Collision with Any Border
                {
                    LCD_erase_circle(bullet);
                    bullet_state = DEAD;
                }

                /* Bullet Movement Logic */
                if(bullet_state != DEAD)
                {
                    LCD_erase_circle(bullet);
                    bullet.x += bullet_x_velocity;
                    bullet.y += bullet_y_velocity;
                    LCD_draw_circle(bullet);
                }

            }

            if (obs_1_state == PAUSED)
            {
                obs_1_pause++;

                if (obs_1_pause >= DODGE_OBS_PAUSE_DELAY)
                {
                    obs_1_pause = 0;
                    obs_1_state = ALIVE;
                }
            }

            if (obs_2_state == PAUSED)
            {
                obs_2_pause++;

                if (obs_2_pause >= DODGE_OBS_PAUSE_DELAY)
                {
                    obs_2_pause = 0;
                    obs_2_state = ALIVE;
                }
            }

            if (obs_3_state == PAUSED)
            {
                obs_3_pause++;

                if (obs_3_pause >= DODGE_OBS_PAUSE_DELAY)
                {
                    obs_3_pause = 0;
                    obs_3_state = ALIVE;
                }
            }

            if (obs_4_state == PAUSED)
            {
                obs_4_pause++;

                if (obs_4_pause >= DODGE_OBS_PAUSE_DELAY)
                {
                    obs_4_pause = 0;
                    obs_4_state = ALIVE;
                }
            }

            //print time survived
            itoa(timer_count,timer_count_string);
            LCD_write_string(timer_count_string,40,0,BLACK,sizeof(timer_count_string)/sizeof(uint8_t));
            timer_trigger = 0;
        }
    }

    LCD_erase_screen();

    //find final time and reset it
    uint16_t score = timer_count;
    timer_count = 0;
    uint8_t score_string[5] = "     ";
    itoa(score,score_string);

    //print ending screen
    LCD_write_string("GAME OVER",40,60,RED,9);
    LCD_write_string("TIME ALIVE:",55,40,NAVY,11);
    LCD_write_string(score_string,15,40,NAVY,sizeof(score_string)/sizeof(uint8_t));

    button_flag = 0;

    while(!button_flag);

    timer_count = 30;
}
//rectangle collision function
//returns 1 if collision, 0 otherwise
uint8_t check_rect_collision(RECT rect0, RECT rect1){
    if(((rect1.x + rect1.width) > rect0.x) && (rect1.x < (rect0.x + rect0.width)) && ((rect1.y + rect1.height) > rect0.y) && (rect1.y < (rect0.y + rect0.height))){
        return 1;
    }
    else{
        return 0;
    }
}
//rectangle, circle collision function
//returns 1 if collision, 0 otherwise
uint8_t check_rect_circ_collision(RECT rect, CIRCLE circ){
    if(((circ.x + circ.radius) > rect.x) && ((circ.x - circ.radius) < (rect.x + rect.width)) && ((circ.y + circ.radius) > rect.y) && ((circ.y - circ.radius) < (rect.y + rect.height))){
        return 1;
    }
    else{
        return 0;
    }
}

//bottom button interrupt handler
void PORT3_IRQHandler(void){
    if(P3->IFG & BIT3){
        if(!(P3->IN & BIT3)){
            button_flag = 1;
        }
        for(uint32_t i = 0; i < 100000; i++);
        P3->IFG &= ~BIT3;
    }
}

//top button interrupt handler
void PORT5_IRQHandler(void){
    if(P5->IFG & BIT1){
        for(uint32_t i = 0; i < 10000; i++);       //wait for button to stop bouncing
        if(!(P5->IN & BIT1)){                       //check to make sure button is low after bouncing
            button_flag = 1;
        }
        P5->IFG &= ~BIT1;
    }
}
