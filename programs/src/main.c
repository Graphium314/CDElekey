#ifndef F_CPU
#define F_CPU 3333333UL  // 20/6=3.333... MHz
#endif

#include <avr/io.h>
// #include <avr/iotn404.h>
#include <stdbool.h>
#include <util/delay.h>

#define FASE_DEFAULT 0
#define FASE_DASH 1
#define FASE_DOT 2
#define FASE_SPACE_DASH 3
#define FASE_SPACE_DOT 4

/*
 * pin settings
 *
 * PA0: UPD
 * PB0: SW2 (input)
 * PB1: SW1 (input)
 * PB2: KEY (output)
 * PB3: swap SW1 and SW2 (input)
 * PA7: speed control (input)
 */

int sw_dash_bm = PIN1_bm, sw_dot_bm = PIN0_bm;

void key_on() { VPORTB.OUT |= (1 << 2); }
void key_off() { VPORTB.OUT &= ~(1 << 2); }
bool is_sw_dash_on() { return !(VPORTB.IN & sw_dash_bm); }
bool is_sw_dot_on() { return !(VPORTB.IN & sw_dot_bm); }
bool swap_sw() { return !(VPORTB.IN & PIN3_bm); }

int main() {
    // set PB2 as output
    VPORTB.DIR |= PIN2_bm;

    // set PB1, PB2, PB3 pull-up
    PORTB.PIN0CTRL = PORT_PULLUPEN_bm;
    PORTB.PIN1CTRL = PORT_PULLUPEN_bm;
    PORTB.PIN3CTRL = PORT_PULLUPEN_bm;
    // settings for ADC
    ADC0.CTRLA |= ADC_RESSEL_10BIT_gc;  // 10bit resolution
    ADC0.CTRLA |= ADC_FREERUN_bm;       // free run mode
    ADC0.CTRLC |= ADC_REFSEL_VDDREF_gc;  // VDD reference
    ADC0.MUXPOS = ADC_MUXPOS_AIN7_gc;    // set PA7 analog input
    ADC0.CTRLA |= ADC_ENABLE_bm;         // enable ADC
    ADC0.COMMAND |= ADC_STCONV_bm;       // start conversion

    int counter = 0;
    int fase_now = FASE_DEFAULT;
    int fase_next = FASE_DEFAULT;
    int dot_length;

    if (swap_sw()) {
        sw_dash_bm = PIN0_bm;
        sw_dot_bm = PIN1_bm;
    }

    key_off();

    while (1) {
      dot_length = 6000/(ADC0.RES/7+50); // 10-39wpm
        if (fase_now == FASE_DEFAULT) {
            if (is_sw_dash_on()) {
                fase_now = FASE_DASH;
                counter = 0;
                key_on();
            } else if (is_sw_dot_on()) {
                fase_now = FASE_DOT;
                counter = 0;
                key_on();
            }
        } else if (fase_now == FASE_DASH) {
            counter++;
            if (is_sw_dot_on()) {
                fase_next = FASE_DOT;
            }
            if (counter >= 3 * dot_length) {
                key_off();
                fase_now = FASE_SPACE_DASH;
                counter = 0;
            }
        } else if (fase_now == FASE_DOT) {
            counter++;
            if (is_sw_dash_on()) {
                fase_next = FASE_DASH;
            }
            if (counter >= dot_length) {
                key_off();
                fase_now = FASE_SPACE_DOT;
                counter = 0;
            }
        } else if (fase_now == FASE_SPACE_DASH) {
            counter++;
            if (is_sw_dot_on()) {
                fase_next = FASE_DOT;
            } else if (fase_next == FASE_DEFAULT && is_sw_dash_on()) {
                fase_next = FASE_DASH;
            }
            if (counter >= dot_length) {
                fase_now = fase_next;
                fase_next = FASE_DEFAULT;
                if (fase_now == FASE_DASH || fase_now == FASE_DOT) {
                    key_on();
                }
                counter = 0;
            }
        } else if (fase_now == FASE_SPACE_DOT) {
            counter++;
            if (is_sw_dash_on()) {
                fase_next = FASE_DASH;
            } else if (fase_next == FASE_DEFAULT && is_sw_dot_on()) {
                fase_next = FASE_DOT;
            }
            if (counter >= dot_length) {
                fase_now = fase_next;
                fase_next = FASE_DEFAULT;
                if (fase_now == FASE_DASH || fase_now == FASE_DOT) {
                    key_on();
                }
                counter = 0;
            }
        }
        _delay_ms(1);
    }
    return 0;
}