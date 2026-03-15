#include <xc.h>
#include "digital_keypad.h"

//initialization or configuration related to DKP
void init_digital_keypad(void)
{
    /* Set Keypad Port as input */
    KEYPAD_PORT_DDR = KEYPAD_PORT_DDR | INPUT_LINES;
}


//detect which switch is pressed and it will return the switch press value
//input mode , level or state

unsigned char read_digital_keypad(unsigned char mode)
{
    static unsigned char once = 1;
    static unsigned char prev_key;
    static unsigned char longpress;
    
    if (mode == LEVEL_DETECTION)
    {
        return KEYPAD_PORT & INPUT_LINES;
    }
    else
    {
        if (((KEYPAD_PORT & INPUT_LINES) != ALL_RELEASED) && once)
        {
            once = 0;// Lock further entries until key is released
            longpress = 0;// Reset long press counter
            prev_key = KEYPAD_PORT & INPUT_LINES; // Store the pressed key           
        }        
        else if( !once && (prev_key == (KEYPAD_PORT & INPUT_LINES)) && longpress < 30)
        {
            longpress++;
        }
        else if( longpress == 30)
        {
            longpress++;
            return 0x80 | prev_key;
        }
        else if ((KEYPAD_PORT & INPUT_LINES) == ALL_RELEASED && !once)
        {
            once = 1;// Ready for next press
            if(longpress < 30)
            {
                return prev_key;// It was a short press
            }
        }
    }    
    return ALL_RELEASED;
}