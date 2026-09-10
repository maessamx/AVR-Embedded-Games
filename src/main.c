#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define SET_BIT(REG , pin) ( (REG) |= (1 << (pin)) )
#define TOG_BIT(reg , pin) ( (reg) ^= (1 << (pin)) )
#define CLR_BIT(reg , pin) ( (reg) &= ~(1 << (pin)) )
#define GET_BIT(reg , pin) ( ((reg) & (1 << (pin))) >> (pin) )

#define LED_RED        PC0
#define LED_GREEN      PC1
#define BUZZER         PC5
#define DIG_LEFT       PC6
#define DIG_RIGHT      PC7
#define LCD_RS         PA1
#define LCD_EN         PA2

static const uint8_t SEGM_CHARS[] = {
    0x3F << 1, 
    0x06 << 1, 
    0x5B << 1, 
    0x4F << 1, 
    0x66 << 1, 
    0x6D << 1, 
    0x7D << 1, 
    0x07 << 1, 
    0x7F << 1, 
    0x6F << 1  
};

static const char KEYPAD_MAP[4][4] = {
    {'7', '8', '9', '/'},
    {'4', '5', '6', '*'},
    {'1', '2', '3', '-'},
    {'C', '0', '=', '+'}
};

typedef struct {
    char    name[5];
    uint8_t id;
    char    password[5];
} User;

static const User users[] = {
    {"P1",  1, "1111"},
    {"P2",  2, "2222"},
    {"P3",  3, "3333"},
    {"P4",  4, "4444"}
};
#define NUM_USERS (sizeof(users) / sizeof(users[0]))

uint8_t EEMEM nvHighScore[NUM_USERS];

static unsigned int g_seed_counter = 0;

void buzzer_beep(uint16_t duration_ms) {
    SET_BIT(PORTC, BUZZER);
    while (duration_ms--) {
        _delay_ms(1);
    }
    CLR_BIT(PORTC, BUZZER);
}

static inline void lcd_pulse_enable(void) 
{
    // here we open the EN to begin display on LCD 
    SET_BIT(PORTA, LCD_EN);   _delay_us(10);
    // here we make EN = 0 to make LCD display on screen bec we know that LCD display when E change from 1 to 0
    CLR_BIT(PORTA, LCD_EN);   _delay_us(10);
}

void lcd_write_nibble(uint8_t nibble) {
    uint8_t current_rs = PORTA & 0x02; 
    PORTA = current_rs | ((nibble & 0xF0) >> 1);
    lcd_pulse_enable();
}

void lcd_send_cmd(uint8_t cmd) {
    // close the 7seg
    CLR_BIT(PORTC, DIG_LEFT);
    CLR_BIT(PORTC, DIG_RIGHT);

    CLR_BIT(PORTA, LCD_RS); // make RS = 0 to know that data coming is instruct
    lcd_write_nibble(cmd); 
    lcd_write_nibble(cmd << 4);
    _delay_ms(2); 
}

void lcd_send_char(char c) {
    CLR_BIT(PORTC, DIG_LEFT);
    CLR_BIT(PORTC, DIG_RIGHT);
    SET_BIT(PORTA, LCD_RS); // make RS = 1 to know that data coming is word
    lcd_write_nibble(c); 
    lcd_write_nibble(c << 4);
    _delay_us(50);
}

void lcd_print_string(const char *str) {
    while (*str) {
        lcd_send_char(*str++);
    }
}

void lcd_hardware_init(void) {
    DDRA  = 0xFE; 
    DDRC |= 0xE3; 
    DDRB  = 0xFF;
    
    _delay_ms(50); 
    
    lcd_write_nibble(0x30); 
    _delay_ms(5);
    lcd_write_nibble(0x30); 
    _delay_us(200);
    lcd_write_nibble(0x30); 
    _delay_us(200);
    
    lcd_write_nibble(0x20); 
    _delay_ms(2);
    
    lcd_send_cmd(0x28); 
    lcd_send_cmd(0x0C); 
    lcd_send_cmd(0x01);
    _delay_ms(2);
}

char keypad_scan(void) 
{
    DDRB |= 0xF0;  PORTB |= 0xF0;
    DDRD &= ~0x3C; PORTD |= 0x3C;
    
    const uint8_t row_pins[] = {PB4, PB5, PB6, PB7};
    const uint8_t col_pins[] = {PD2, PD3, PD4, PD5};
    
    for (uint8_t r = 0; r < 4; r++) {
        CLR_BIT(PORTB, row_pins[r]);
        _delay_us(5);
        
        for (uint8_t c = 0; c < 4; c++) {
            if (!GET_BIT(PIND, col_pins[c])) {
                _delay_ms(20);
                while (!GET_BIT(PIND, col_pins[c]));
                return KEYPAD_MAP[r][c];
            }
        }
        SET_BIT(PORTB, row_pins[r]);
    }
    return 0;
}

void display_refresh_score(uint8_t score1, uint8_t score2) {
    uint8_t lcd_ctrl_bits = PORTA & 0x01; 

    uint8_t left_pattern  = SEGM_CHARS[score1 % 10];
    uint8_t right_pattern = SEGM_CHARS[score2 % 10];

    PORTA = lcd_ctrl_bits | left_pattern; 
    SET_BIT(PORTC, DIG_LEFT);
    _delay_ms(2);                           
    CLR_BIT(PORTC, DIG_LEFT);
    
    PORTA = lcd_ctrl_bits | right_pattern; 
    SET_BIT(PORTC, DIG_RIGHT);
    _delay_ms(2);                           
    CLR_BIT(PORTC, DIG_RIGHT);
    
    PORTA &= 0x01;
}

static int8_t find_user(uint8_t id, const char *pass) {
    for (uint8_t i = 0; i < NUM_USERS; i++) {
        if (users[i].id == id && strcmp(users[i].password, pass) == 0) {
            return (int8_t)i;
        }
    }
    return -1;
}

uint8_t login_user(uint8_t player_num) {
    char input_buffer[5];
    
    while (1) {
        lcd_send_cmd(0x01);
        lcd_print_string("P"); 
        lcd_send_char(player_num + '0'); 
        lcd_print_string(" Enter ID:");
        lcd_send_cmd(0xC0);
        
        char id_key = 0;
        while (!id_key) {
            g_seed_counter++;
            char k = keypad_scan();
            if (k >= '1' && k <= '9') id_key = k;
        }
        lcd_send_char(id_key);
        _delay_ms(300);
        uint8_t entered_id = id_key - '0';
        
        lcd_send_cmd(0x01);
        lcd_print_string("Enter Password");
        lcd_send_cmd(0xC0);
        
        uint8_t idx = 0;
        memset(input_buffer, 0, sizeof(input_buffer));
        while (idx < 4) {
            g_seed_counter++;
            char key = keypad_scan();
            if (key >= '0' && key <= '9') {
                input_buffer[idx++] = key;
                lcd_send_char('*');
                buzzer_beep(20);
            }
        }
        
        int8_t match = find_user(entered_id, input_buffer);
        
        if (match >= 0) {
            srand(g_seed_counter);
            SET_BIT(PORTC, LED_GREEN);
            CLR_BIT(PORTC, LED_RED);
            
            uint8_t hi = eeprom_read_byte(&nvHighScore[match]);
            
            lcd_send_cmd(0x01);
            lcd_print_string("Welcome ");
            lcd_print_string(users[match].name);
            _delay_ms(1500);
            
            return (uint8_t)match;
        } else {
            SET_BIT(PORTC, LED_RED);
            CLR_BIT(PORTC, LED_GREEN);
            lcd_send_cmd(0x01); 
            lcd_print_string("Wrong ID/Pass!"); 
            buzzer_beep(200); 
            _delay_ms(800);
        }
    }
}

uint8_t select_mod(void) {
    lcd_send_cmd(0x01);
    lcd_print_string("Select Mod:");
    lcd_send_cmd(0xC0);
    lcd_print_string("1)Spd 2)Sz 3)No");
    
    char key = 0;
    while (key != '1' && key != '2' && key != '3') {
        key = keypad_scan();
    }
    
    lcd_send_cmd(0x01);
    if (key == '1')      lcd_print_string("Mod: Speed");
    else if (key == '2') lcd_print_string("Mod: Paddle Size");
    else                 lcd_print_string("Mod: None");
    _delay_ms(1000);
    
    return (uint8_t)(key - '0');
}

static bool update_high_score(uint8_t user_idx, uint8_t score) {
    uint8_t stored = eeprom_read_byte(&nvHighScore[user_idx]);
    if (score > stored) {
        eeprom_update_byte(&nvHighScore[user_idx], score);
        return true;
    }
    return false;
}

uint8_t paddle_game(uint8_t mod, uint8_t p1_user, uint8_t p2_user) 
{
    lcd_send_cmd(0x01);// clear
    lcd_print_string("Loading Game...");
    _delay_ms(1000);

    int8_t ball_x = 7, ball_y = 0;
    int8_t velocity_x = 1, velocity_y = 1;
    uint8_t paddle_p1_y = 0, paddle_p2_y = 0;
    uint8_t score_p1 = 0, score_p2 = 0;
    
    // the display of screen (LCD) for first row
    char render_buffer_l1[17];
    // the display of screen (LCD) for sec row
    char render_buffer_l2[17];

    while (1) {
        char action_key = keypad_scan();
        
        if (action_key == '7') paddle_p1_y = 0; 
        if (action_key == '1') paddle_p1_y = 1;
        if (action_key == '9') paddle_p2_y = 0; 
        if (action_key == '3') paddle_p2_y = 1;

        if (action_key == '5') {
            buzzer_beep(300);
            return mod;   
        }

        ball_x += velocity_x; 
        ball_y += velocity_y;
        
        if (ball_y < 0) { ball_y = 0; velocity_y = 1; } 
        if (ball_y > 1) { ball_y = 1; velocity_y = -1; }

        if (ball_x < 0 || ball_x > 15) {
            if (ball_x < 0) score_p2++; else score_p1++;
            buzzer_beep(300);
            
            CLR_BIT(PORTA, LCD_RS);
            CLR_BIT(PORTA, LCD_EN); // stop enable to make 7seg show num
            CLR_BIT(PORTC, DIG_LEFT);
            CLR_BIT(PORTC, DIG_RIGHT);

            // dispaly score on 7seg we make dor loop to display for long time bev without it you can't see the score
            for (uint8_t cycles = 0; cycles < 30; cycles++) {
                display_refresh_score(score_p1, score_p2);
            }
            
            if (score_p1 >= 10 || score_p2 >= 10) {
                buzzer_beep(800); 
                lcd_send_cmd(0x01);
                
                if (score_p1 >= 10) {
                    lcd_print_string("  PLAYER 1 WINS!");
                } else {
                    lcd_print_string("  PLAYER 2 WINS!");
                }
                
                _delay_ms(3000);
                lcd_send_cmd(0x01);
                lcd_print_string(" GAME OVER ");
                _delay_ms(2000);
                
                return mod;
            }
            
            // if game not end 
            ball_x = 7; 
            ball_y = 0; 
            // the velocity get it random
            velocity_x = (rand() % 2 == 0) ? 1 : -1;
            velocity_y = (rand() % 2 == 0) ? 1 : -1;
            
            continue; 
        }

        // is the paddle and the ball in-front each other 
        if (ball_x == 0 && ball_y == paddle_p1_y)  { velocity_x = 1;  buzzer_beep(5); }
        if (ball_x == 15 && ball_y == paddle_p2_y) { velocity_x = -1; buzzer_beep(5); }

        memset(render_buffer_l1, ' ', 16); 
        memset(render_buffer_l2, ' ', 16);
        render_buffer_l1[16] = render_buffer_l2[16] = '\0';
        
        if (paddle_p1_y == 0) render_buffer_l1[0]  = '|'; else render_buffer_l2[0]  = '|';
        if (paddle_p2_y == 0) render_buffer_l1[15] = '|'; else render_buffer_l2[15] = '|';
        
        if (ball_x >= 0 && ball_x < 16) {
            if (ball_y == 0) render_buffer_l1[ball_x] = 'O'; else render_buffer_l2[ball_x] = 'O';
        }

        lcd_send_cmd(0x80);
        lcd_print_string(render_buffer_l1);

        lcd_send_cmd(0xC0);
        lcd_print_string(render_buffer_l2);
        
        // important to make 7 seg on during the game and edit the speed for the ball
        for (uint8_t cycles = 0; cycles < 30; cycles++) {
            display_refresh_score(score_p1, score_p2);
        }
    }
}
uint8_t guess_game(void) {
    uint8_t attempts = 10;
    uint8_t score = 0;

    lcd_send_cmd(0x01);
    lcd_print_string("READY TO PLAY?");
    _delay_ms(2000);

    while (attempts) {
        // off all led
        CLR_BIT(PORTC, PC0);
        CLR_BIT(PORTC, PC1);
        CLR_BIT(PORTC, PC2);

        // choose random led from 3 to on 
        uint8_t num = rand() % 3;      
        SET_BIT(PORTC, num);           

        char correct_key = (num == 0) ? '1' : (num == 1) ? '2' : '3';
        char action_key  = 0;
        // ده العداد اللي لو خلص لمبه تانيه عشوائيه تنور مخلي الديلاي 10 و عندك العداد 100 يعني 10 ف 100 يساوي 1000 يعني 1 ثانيه بس قبل ما يختار لمبه تانيه
        uint8_t cnt = 0;

        while (!action_key && cnt < 100) {
            char k = keypad_scan();
            if (k == '1' || k == '2' || k == '3') {
                action_key = k;
            }
            _delay_ms(10);
            cnt++;
        }

        CLR_BIT(PORTC, num);           

        if (action_key == correct_key) {
            score++;
        } else {
            buzzer_beep(300);
        }

        attempts--;

        lcd_send_cmd(0x01);
        lcd_print_string("Attempts: ");
        // عمليه الحسبه ديه عشان تدخل ارقم عل خانتين لو 8 يبقي 08
        lcd_send_char((attempts / 10) + '0');
        lcd_send_char((attempts % 10) + '0');
        _delay_ms(500);
    }

    lcd_send_cmd(0x01);
    lcd_print_string("Score: ");
    lcd_send_char((score / 10) + '0');
    lcd_send_char((score % 10) + '0');
    _delay_ms(4000);

    return score;
}

int main(void) 
{
    lcd_hardware_init();
    
    uint8_t p1_user = login_user(1);
    uint8_t p2_user = login_user(2);
    
    while (1) {
        uint8_t mod = select_mod();
        
        if (mod == 3) {
            CLR_BIT(PORTC, LED_GREEN);
            SET_BIT(PORTC, LED_RED);
            lcd_send_cmd(0x01);
            lcd_print_string("Logging out...");
            _delay_ms(1000);
            
            p1_user = login_user(1);
            p2_user = login_user(2);
            continue;
        }if(mod ==1){
            paddle_game(mod, p1_user, p2_user);
        }
        if(mod ==2){
            guess_game();
        }
    }
}