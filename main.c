/*
 * File:   main.c
 * Author: THAMUNNA
 *
 * Created on April 24, 2025, 3:20 PM
 */



/*
 * Name: THAMUNNA OP
 * Date of Submission: 8-5-25
 * Description: Project on Car Black Box
 * 
 * By considering todays busy life, every one wants to reach the destination as soon as possible 
   by ignoring the rules and regulations. So one solution could be by implementing a Black Box
   which would log critical activities on the car.
   
 * As mentioned above the root cause of the negligence in the driving would be meeting the
   daily schedule and go off duty, or to earn extra money by illegal trips etc,. So buy
   implementing the mentioned solution it would be easy to keep track of how the vehicle is
   being used, handled and control the efficiency of the vehicle.
   
 * The proposed solution is to log all the critical events like the gear shifts with current speed,
   the engine temperature, fuel consumption per trip, trip distance etc., The system should allow
   a password based access to the transport managers to view or download the log to PC if required.
   
*/

#include "main.h"
#pragma config WDTE=OFF

void init_config()
{
    //initialize clcd
    init_clcd();
    //initialize adc
    init_adc();
    //initialize digital keypad
    init_digital_keypad();
    //initialize i2c
    init_i2c(1000000);
    //initialize trc
    init_ds1307();
    //initialize timer2
    init_timer2();
    init_uart(9600);
    PEIE=1;
    GIE=1;
}
void main(void) {
    init_config();
    unsigned char control_flag=DASHBOARD_SCREEN;
    unsigned char event[3]="ON";
    unsigned char speed=0;
    unsigned char *gear[]={"GN", "GR", "G1", "G2", "G3", "G4"};
    unsigned char gr=0;//variable to point to index number
    unsigned char key;
    unsigned char reset_flag,menu_pos;
    int wait_time = 0;
    
      log_event(event,speed);
    ext_eeprom_24C02_str_write(0x00,"1010");
    log_event(event,speed);
    while(1)
    {
        speed=read_adc()/10.3;//to make the speeed value only between 0-99
        key=read_digital_keypad(STATE);
        for(unsigned int i=3000;i--;);//delay to avoid bouncing effect
        
        if(key==SW1)
        {
            //collision
           strcpy(event,"C0");
           log_event(event,speed);
        }
        else if(key==SW2 && gr<6)
        {
            //increment gear
            strcpy(event,gear[gr]);
            gr++;
            log_event(event,speed);
        }
        else if (key==SW3 && gr>0)
        {
            //decrement the gear
            gr--;
             strcpy(event,gear[gr]);
             log_event(event,speed);
             
        }
        else if((key==SW4 ||key==SW5)&&control_flag==DASHBOARD_SCREEN)// show enter password screen if up or down pressed at dashboard
        {
            control_flag=LOGIN_SCREEN;
             clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
            __delay_us(500);   
            clcd_print("Enter Password",LINE1(1));// print enter password prompt
            clcd_write(LINE2(4),INST_MODE);//BRINGING cursor to 2nd line as it is instructn we pass 2nd argument as 0i.e,MACR0 INST_MODE
            clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);//FOR CURSOR BLINKING
            __delay_us(100);
            reset_flag=RESET_PASSWORD;
            TMR2ON = 1;// turn on timer to return to dashboard if inactive
        }
        

        else if (key == LPSW4 && control_flag == MAIN_MENU_SCREEN) // at longpress at login menu select the current menu option
        {
            switch(menu_pos)
            {
                case 0: // view log
                    clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
                    __delay_us(500); 
                    clcd_print("Stored Logs",LINE1(0)); // print view log header
                    control_flag = VIEW_LOG_FLAG; // set flag to enter view log function
                    reset_flag = RESET_VIEW_LOG_POS;
                    break;
                case 1: // clear log
                    control_flag = CLEAR_LOG_FLAG; // set flag to enter clear logs function
                    break;
                case 2:
                    control_flag = DOWNLOAD_LOG_FLAG;  // set flag to enter download logs function
                    reset_flag = PRINT_UART;
                    break;
                case 3:
                    control_flag = SET_TIME_FLAG;  // set flag to enter set time menu
                    reset_flag = RESET_TIME;
                    clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
                    __delay_us(500);                        
                    clcd_print("    SET TIME    ",LINE1(0));                    
                    break;
                case 4:
                    control_flag = CHANGE_PASSWORD_FLAG;  // set flag to enter change password menu
                    reset_flag = RESET_PASSWORD;
                    clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
                    __delay_us(500);                        
                    clcd_print("ENTER NEW PASSWD",LINE1(0)); // print menu header
                    clcd_putch(' ',LINE2(5)); // set cursor at position for cursor blink
                    clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
                    break;
            }            
        }
        else if(key == LPSW5) // if longpress down go back to dashboard
        {
            clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
            __delay_us(500);
            clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
            __delay_us(100);
            control_flag = DASHBOARD_SCREEN; // set flag to enter dashboard display            
        }        
        
        
        
        switch(control_flag)
        {
            case DASHBOARD_SCREEN: // enter dashboard display function
                display_dashboard(event, speed);
                break;
            case LOGIN_SCREEN:
                switch(login(key,reset_flag)) // enter login menu
                {
                    case RETURN_BACK: // in case of timeout
                        TMR2ON = 0; // turn off timer
                        control_flag = DASHBOARD_SCREEN;
                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE); // turn off cursor
                         clcd_write(CLEAR_DISP_SCREEN, INST_MODE); // Clear display
                        __delay_us(100);
                        break;
                    case LOGIN_SUCCESS: // in case of correct password
                        control_flag = MAIN_MENU_SCREEN; // set flag to enter the main menu
                        reset_flag = RESET_MENU;
                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                        __delay_us(100);
                        continue;                    
                }
                break;
            case MAIN_MENU_SCREEN:
                
                menu_pos = menu_screen(key,reset_flag);// enter login menu and update menu pos in main function
                break;
                
            case VIEW_LOG_FLAG:
                view_log(key,reset_flag); // enter view log function
                
              if (reset_flag != RESET_VIEW_LOG_POS && key == LPSW4)
               {
                   control_flag = MAIN_MENU_SCREEN;
                   reset_flag = RESET_MENU;
                   clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
                   __delay_us(500);
                }
                break;
                
            case CLEAR_LOG_FLAG: // if clear log selected
                
                clear_log();//function to clear the log
                clcd_write(CLEAR_DISP_SCREEN, INST_MODE); // Clear display
                __delay_us(500); // Small delay for LCD to clear
                 if(key==LPSW4)
                {
                    control_flag=MAIN_MENU_SCREEN;
                }
                
                break;
                
            case DOWNLOAD_LOG_FLAG:
                 download_log_uart(reset_flag); // enter download log function
                 if (reset_flag != PRINT_UART && key == LPSW4) // Long press detected after download
                  {
                     control_flag = MAIN_MENU_SCREEN;
                     reset_flag = RESET_MENU;
                     clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
                     __delay_us(500);
                  }
                break;
                
            case CHANGE_PASSWORD_FLAG:
                switch ( change_password( key ,reset_flag ) )
				{
					case TASK_SUCCESS :
                        wait_time = 100;
                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                        break;
                    
					case RETURN_BACK :
                        control_flag = MAIN_MENU_SCREEN;
                        reset_flag = RESET_MENU;
                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                        continue;
				}

				if (wait_time > 0 && --wait_time == 0 )
				{
					control_flag = MAIN_MENU_SCREEN;
					reset_flag = RESET_MENU;
                    CLEAR_DISP_SCREEN;
					continue;
				}
                
                break;
            case SET_TIME_FLAG:
                
				if ( set_time( key ,reset_flag ) == TASK_SUCCESS )
					wait_time = 100;
                
				if (wait_time > 0 && --wait_time == 0 )
				{
					control_flag = MAIN_MENU_SCREEN;
					reset_flag = RESET_MENU;
                     clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
                    __delay_us(500);
					continue;
				}
				break;// enter set time function
                
        }        
        reset_flag = RESET_NOTHING; // change reset flag so function is initialised on each keypress and not on each loop
    }    
    return;
}