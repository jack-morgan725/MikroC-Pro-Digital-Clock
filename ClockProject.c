
//===========================================================================================================================================================>

int selected_segment  = 3;              //--> Current ative segment when SETTING MODE enabled.
int rb_0_setting_mode = 0;
int rb_0_state = 0;
int rb_1_state = 0;
int rb_2_state = 0;                     //--> Current state of RB2 switch.
int timer_0_overflow = 30;              //--> Number of overflows before segment values are updated.
int timer_2_overflow = 8;               //--> Number of overflows before segment displays are updated.
int timer_4_overflow = 1000;
int timer_0_count = 0;                  //--> Increments for each Timer 0 interrupt.
int timer_2_count = 0;                  //--> Increments for each Timer 1 interrupt.
int timer_4_count = 0;
int index = 0;
int digit[]     = {0, 0, 2, 1};         //--> Digits 1, 2, 3, and 4
int dis_value[] = {1, 2, 4, 8};         //--> DISPLAY values for PORTA.
int seg_value[] = {0x3F,                //--> DISPLAY values for PORTD.
                   0x06,
                   0x5B,
                   0x4F,
                   0x66,
                   0x6D,
                   0x7D,
                   0x07,
                   0x7F,
                   0x67};

//=========> Starts timer zero (DIGIT UPDATER).
void startT0() {
    T0CON.T0PS0   = 1;        //--> 30.517 overflows for 1 second delay when prescaler set to 256.
    T0CON.T0PS1   = 1;        //--> Set PRESCALER to increment timer every 256 instructions.
    T0CON.T0PS2   = 1;
    T0CON.PSA     = 0;        //--> Turn PRESCALER ON.
    T0CON.T0CS    = 0;        //--> Set time clock source to internal clock.
    T0CON.T08BIT  = 1;        //--> Set timer to 8-bit mode (overflows at 256).
    INTCON.TMR0IE = 1;        //--> Enable interrupts from timer 0.
    INTCON.GIE    = 1;        //--> Enable global interrupts so that interrupt can reach CPU.
    T0CON.TMR0ON  = 1;        //--> Start the timer.
}

//=========> Starts timer two (SEGMENT UPDATER).
void startT2() {
    T2CON.T2CKPS0  = 1;        //--> Set PRESCALER set to 01 (4). POSTSCALER set to 0000 (1).
    T2CON.T2CKPS1  = 0;
    T2CON.T2OUTPS0 = 0;
    T2CON.T2OUTPS1 = 0;
    T2CON.T2OUTPS2 = 0;
    T2CON.T2OUTPS3 = 0;
    INTCON.PEIE    = 1;        //--> Enable peripheral interrupts.
    PIE1.TMR2IE    = 1;        //--> Enable interrupts from timer 2.
    T2CON.TMR2ON   = 1;        //--> Turn timer on.
}

//=========> Starts timer four (SEGMENT FLASHER).
void startT4() {
    T4CON.T4CKPS0  = 1;        //--> Set PRESCALER set to 01 (4). POSTSCALER set to 0000 (1).
    T4CON.T4CKPS1  = 0;
    T4CON.T4OUTPS0 = 0;
    T4CON.T4OUTPS1 = 0;
    T4CON.T4OUTPS2 = 0;
    T4CON.T4OUTPS3 = 0;
    PIE5.TMR4IE    = 1;        //--> Enable interrupts from timer 2.
    T4CON.TMR4ON   = 1;        //--> Turn timer on.
}

//=========> Updates clock segments. Called once per 4MS.
void update_clock_segment() {

    if (index != selected_segment || rb_0_setting_mode == 0) {      //--> If SETTING mode is enabled DO NOT update SELECTED_SEGMENT.
        LATA = dis_value[index];                                    //--> Select segment to update.
        if (LATA != 4)
            LATD = seg_value[digit[index]];                         //--> Update segment.
        else
            LATD = seg_value[digit[index]] + 128;                   //--> Include dot for 3rd segment.
    }

    index += 1;                                                     //--> Move to next segment.
    if (index == 4)                                                 //--> If last segment reached -> loop around.
        index = 0;

    if (timer_4_count >= timer_4_overflow) {                        //--> If SETTING mode enabled and T4 has overflown -> Display selected segment.

        LATA = dis_value[selected_segment];                         //--> Turn selected segment display on.
            if (LATA != 4)
                LATD = seg_value[digit[selected_segment]];          //--> Set appropriate digit.
        else
            LATD = seg_value[digit[selected_segment]] + 128;        //--> Set appropriate digit with dot if selected digit = 3 (LATA 4).
        timer_4_count = 0;
    }
}

//=========> Updates clock digits. Called once per second.
void update_digits() {
    timer_0_count = 0;                                       //--> Reset the timer.

    if (digit[1] != 6)                                       //--> If 2nd digit < 6 -> increment 1st digit.
        digit[0] += 1;


    if (digit[0] == 10) {                                    //--> If 1st digit == 10 (MAX value 9 passed) ->
        digit[0] =  0;                                       //--> Reset 1st digit back to 0.
        digit[1] += 1;                                       //--> Increment 2nd digit.
    }

    if (digit[1] == 6) {                                     //--> If 2nd digit == 6 (MAX value 6 passed) ->
       digit[1] = 0;                                         //--> Reset 2nd digit back to 0.
       digit[2] += 1;                                        //--> Incrment 3rd digit.
    }

    if (digit[2] == 10) {                                    //--> If 3rd digit == 10 (MAX value 9 passed) ->
       digit[2] = 0;                                         //--> Reset 3rd digit back to 0.
       digit[3] += 1;                                        //--> Increment 4th digit.
    }

    if (digit[3] == 2 && digit[2] == 4) {                    //--> If 4th digit == 2 AND 3rd digit == 4
       digit[3] = 0;                                         //--> Reset 4th digit back to 0.
       digit[2] = 0;                                         //--> Reset 3rd digit back to 0.
       digit[1] = 0;                                         //--> Reset 2nd digit back to 0.
       digit[0] = 0;                                         //--> Reset 1st digit back to 0.
    }
}

//=========> Increments selected clock digit in SETTING MODE.
void update_selected_digit() {

    digit[selected_segment] += 1;

    if (digit[0] > 9) {                                      //--> If 1st digit > 9 -> reset to 0. Increment 2nd digit.
        digit[0] = 0;
        digit[1] += 1;
    }

    if (digit[1] == 6) {
        digit[1] =  0;
        digit[2] += 1;
    }

    if (digit[1] > 6) {                                      //--> If 2nd digit > 6 -> Reset to 0. Increment 3rd digit.
        digit[1] =  0;
        digit[2] += 1;
    }

    if (digit[2] > 9) {                                      //--> If 3rd digit > 9  -> Reset to 0. Increment 4th digit.
        digit[2] =  0;
        digit[3] += 1;
    }

    if (digit[3] == 2 &&
        digit[2] >= 4 ||
        digit[3] > 2) {                                      //--> If 4th digit == 2 AND 3rd digit == 4
        digit[3] = 0;                                        //--> Reset 4th digit back to 0.
        digit[2] = 0;                                        //--> Reset 3rd digit back to 0.
        digit[1] = 0;                                        //--> Reset 2nd digit back to 0.
        digit[0] = 0;                                        //--> Reset 1st digit back to 0.
    }
}

//=========> Interrupt function. Called once interrupt reaches CPU.
void interrupt() {

//=========> Timer 0 (DIGIT UPDATER) interrupt.
    if (INTCON.TMR0IF == 1) {                                //--> Timer 0 interrupt.

        timer_0_count += 1;                                  //--> Keep track of number of overflows.
        if (timer_0_count >= timer_0_overflow)
            update_digits();

        INTCON.TMR0IF = 0;                                   //--> RESET Timer 0 FLAG now that interrupt has been serviced.
    }

//=========> Timer 2 (SEGMENT UPDATER) interrupt.
    if (PIR1.TMR2IF == 1) {                                  //--> Timer 2 interrupt.

        timer_2_count += 1;                                  //--> Count number of interrupts generated.
        if (timer_2_count >= timer_2_overflow) {
            update_clock_segment();
            timer_2_count = 0;
        }

        PIR1.TMR2IF = 0;                                     //--> RESET Timer 2 FLAG now that interrupt has been serviced.
    }

//=========> Timer 4 (SEGMENT FLASHER) interrupt.
    if (PIR5.TMR4IF == 1) {                                  //--> Timer 4 interrupt.

       timer_4_count += 1;
       PIR5.TMR4IF = 0;
    }
}

//=========> Main loop.
void main() {

    startT0();                                               //--> Start clock 0. (DIGIT UPDATER).
    startT2();                                               //--> Start clock 2. (SEGMENT UPDATER).

    ANSELA = 0x00;                                           //--> Set PORTA, PORTD, and PORTB to DIGITAL.
    ANSELD = 0x00;
    ANSELB = 0x00;

    TRISD = 0x00;                                            //--> Set PORTA and PORTD to OUTPUT.
    TRISA = 0x00;
    TRISB = 0xFF;                                            //--> Set PORTB as INPUT.

    while(1) {
        if (PORTB.F2 == 1) {                                 //--> If RB2 pressed -> SPEED UP.
            rb_2_state = 1;
            timer_0_overflow = 1;
        }

        Delay_ms(100);                                       //--> Delay for button debounce.
        if (rb_2_state == 1 && PORTB.F2 == 0) {              //--> Check if RB2 has been released.
            rb_2_state = 0;                                  //--> If RB2 released -> SLOW DOWN.
            timer_0_overflow = 30;
        }

          if (PORTB.F0 == 1) {                               //--> If RB0 pressed -> Stop digits updating.
            rb_0_setting_mode = 1;
            T0CON.TMR0ON = 0;
        }

        if (rb_0_setting_mode == 1 && PORTB.F0 == 0) {       //--> If RB0 released -> Enter SETTING mode.
            startT4();
            while (rb_0_setting_mode == 1) {
                if (PORTB.F0 == 1) {                         //--> If PORTB pressed whilst in SETTING mode -> RB0 state = 1.
                    rb_0_state = 1;
                }
                Delay_ms(100);                               //--> Delay for button debounce.
                if (rb_0_state == 1 && PORTB.F0 == 0) {      //--> If RB0 state == 1 and RB0 has been RELEASED ->
                    rb_0_state = 0;                          //--> RB0 state reset.
                    selected_segment -= 1;                   //--> Move to next segment.

                    if (selected_segment == -1) {            //--> If last segment already reached ->
                        rb_0_setting_mode = 0;               //--> Exit SETTING mode.
                        selected_segment  = 3;               //--> Reset segment position.
                        T0CON.TMR0ON = 1;                    //--> Start timer 0. Start updating digits again.
                    }
                }
                if (PORTB.F1 == 1) {                         //--> If RB1 pressed ->
                    rb_1_state = 1;
                }
                Delay_ms(100);                               //--> Delay for button debounce.
                if (rb_1_state == 1 && PORTB.F1 == 0) {      //--> If RB1 released -> Increment selected digit.
                    rb_1_state = 0;
                    update_selected_digit();
                }
            }
            T4CON.TMR4ON = 0;                                //--> Turn Timer 4 (SEGMENT FLASHER OFF). [Z]
        }
    }
}


//===========================================================================================================================================================================>
//==> Timer 0 (8/16-bit timer with PRESCALER) Math:
//--> Interrupt generated at: 32.768 ms.
//--> Each instruction takes 0.5 microseconds to execute.
//--> PRESCALER is set to 256 (111).
//--> Timer is 8-bit. Overflows at 256 * 256 (65536 instructions)
//--> 65536 * 0.5 = 32768 microseconds -> 32.768 milliseconds.
//--> If overflow count = 30. Overflows every 983 milliseconds.
//===========================================================================================================================================================================>
//==> Timer 2 (8-bit timer with PRESCALER and POSTSCALER) Math:
//--> Interrupt generated at: 0.512 milliseconds.
//--> Each instruction takes 0.5 microseconds to execute.
//--> PRESCALER is set to 4 (01).
//--> POSTSCALER is set to 1 (0000).
//--> Timer is 8-bit. Overflows at 4 * 256 (1024 instructions).
//--> 1024 * 0.5 = 512 microseconds -> 0.512 milliseconds.
//--> If overflow count = 8. Overflows every 4,096 MS.
//===========================================================================================================================================================================>
//==> Timer 4 Math (8-bit timer with PRESCALER and POSTSCLAER):
//--> Interrupt generated at:
//--> Each instruction takes 0.5 microseconds to execute.
//--> PRESCALER is set to 4 (01)
//--> POSTSCALER is set to 1 (0000)
//--> Timer is 8-bit. Overflows at 4 * 256 (1024 instructions).
//--> 1024 * 0.5 = 512 microseconds -> 0.512 milliseconds.
//--> If overflow count = 800. Overflows every 0.512 seconds.
//===========================================================================================================================================================================>