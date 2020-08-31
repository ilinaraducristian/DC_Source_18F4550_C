#include "config.h"
#include <xc.h>
#include "main.h"

char counter;
__bit view_mode_latch;
__bit view_mode;

void main(void) {
    // Oscillator setup
    OSCCON = 0b01110000;
    
    // I/O Setup
    TRISA = 1;
    LATA = 0;
    
    TRISB = 0x80;
    LATB = 0;
    
    TRISC = 0;
    LATC = 0;
    
    TRISD = 0;
    LATD = 0;
    
    // Interrupts setup
    INTCON = 0b11001000;
    INTCON2bits.RBPU = 0;
//    INTCON2 = 0b10000000;
//    INTCON3 = 0;
//    RCONbits.IPEN = 1;
    
    // Timer 0 setup
    T0CON = 0b10001000;
    TMR0H = 0;
    TMR0L = 0;
    
    // PWM module setup
    PR2 = 255;
    CCP2CON = 0b00001100;
    T2CONbits.T2CKPS1 = 1;
    T2CONbits.TMR2ON = 1;
    CCPR2L = 0;
    
    // A/D setup
    ADCON0bits.ADON = 1;
    ADCON1 = 0b00001110;
    ADCON2bits.ADCS0 = 1;
    ADRESH = 0;
    ADRESL = 0;

    while(1) {
        // refacem timer pt 100ms, 500 e prea mult
        if(INTCONbits.TMR0IF == 1) {
            INTCONbits.TMR0IF = 0;
            counter++;
            if(counter == 15) {
                counter = 0;
                ADCON0bits.GO = 1;
                while(ADCON0bits.GO == 1);

                CCP2CONbits.DC2B1 = (ADRESL & 0x80)>>7;
                CCP2CONbits.DC2B0 = (ADRESL & 0x40)>>6;
                CCPR2L = ADRESH;

                if(view_mode == 0) {
                    LATD = ADRESH;
                }else {
                    LATD = 255 >> (8 - ((ADRESH << 2) + (ADRESL >> 6) ) / 114);
                }
            }
        }
    }
    return;
}

void interrupt_high_priority(void) {
    if(PORTBbits.RB7 == 1)  {
        INTCONbits.RBIF = 0;
        return;
    }
    INTCONbits.RBIF = 0;
    
    // 500ms delay
    TMR0H = 0;
    TMR0L = 0;
    counter = 0;
    INTCONbits.TMR0IF = 0;

    while(counter < 15) {
        if(INTCONbits.TMR0IF == 1) {
            INTCONbits.TMR0IF = 0;
            counter++;
        }
        if(INTCONbits.RBIF == 1) {
            break;
        }
    }
    
    // after 500ms
    if(INTCONbits.RBIF == 0) {
        // long press
        view_mode = view_mode_latch;
    }else if(INTCONbits.RBIF == 1) {
        // tap
        view_mode_latch = !view_mode_latch;
        LATCbits.LC0 = view_mode_latch;
    }
    
    TMR0H = 0;
    TMR0L = 0;
    counter = 0;
    INTCONbits.TMR0IF = 0;
    INTCONbits.RBIF = 0;
    return;
}
