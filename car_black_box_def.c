/*
 * File:   car_black_box_def.c
 * Author: THAMUNNA
 *
 * Created on April 24, 2025, 3:22 PM
 */

//function definition for car black box
#include "main.h"

// variable declarations
unsigned char clock_reg[3];
char time[7];// "HH:MM:SS"
char log[11];//"HHMMSSEVSP"
char log_pos=0;
unsigned char sec;//variable for waiting for 60 sec in case of wrong pswd entry
unsigned char return_time;
char * menu[] = {"View Log", "Clear Log", "Download Log", "Set Time", "Change Passwd"};

// function reads time data from RTC
static void get_time()
{
    clock_reg[0] = read_ds1307(HOUR_ADDR); // HH -> BCD 
    clock_reg[1] = read_ds1307(MIN_ADDR); // MM -> BCD 
    clock_reg[2] = read_ds1307(SEC_ADDR); // SS -> BCD 
     // HH ->23
     time[0] = ((clock_reg[0] >> 4) & 0x03) + '0';
    time[1] = (clock_reg[0] & 0x0F) + '0';
    
  
    // MM 
    time[2] = ((clock_reg[1] >> 4) & 0x07) + '0';
    time[3] = (clock_reg[1] & 0x0F) + '0';
    
    
    // SS
    time[4] = ((clock_reg[2] >> 4) & 0x07) + '0';
    time[5] = (clock_reg[2] & 0x0F) + '0';
    time[6] = '\0';
    
    
}

// function displays time on clcd

static void display_time()
{
    get_time();
    //HH:MM:SS
    clcd_putch(time[0],LINE2(2));
    clcd_putch(time[1],LINE2(3));
    clcd_putch(':' ,LINE2(4));
    clcd_putch(time[2],LINE2(5));
    clcd_putch(time[3],LINE2(6));
    clcd_putch(':' ,LINE2(7));
    clcd_putch(time[4],LINE2(8));
    clcd_putch(time[5],LINE2(9));
}

// function displays dashboard menu
void display_dashboard(unsigned char event[],unsigned char speed)
{
    //line1 display the title msg
    clcd_print("TIME     E  SP",LINE1(2));
    
    //display time
    display_time();
    //display gear event
    clcd_print(event,LINE2(11));
    //display speed(0-99)
    clcd_putch((speed/10) + '0',LINE2(14));
    clcd_putch((speed %10) + '0',LINE2(15));
}

// function logs an event and stores in external EEPROM
    
void store_event()
{
    char addr;
    if(log_pos==10)
        log_pos=0;//again we want to start from 0th position
    
    addr=0x05 +log_pos *10;//each log is for 10 bytes of data
    ext_eeprom_24C02_str_write(addr,log);
    log_pos++;
}

// function updates log array to log data

void log_event(unsigned char event[],unsigned char speed)
{
    get_time();
    strncpy(log,time,6);
    strncpy(&log[6],event,2);
    log[8]=(speed/10)+'0';
    log[9]=(speed%10)+'0';
    
    log[10]='\0';
    store_event();// store collected data to external EEPROM
}

// function displays login screen
unsigned char login(unsigned char key,unsigned char reset_flag)
{
    static unsigned char user_password[4];
    static unsigned char i;
    static unsigned char attempt_left;
    
    if(reset_flag==RESET_PASSWORD)
    {
        i=0;
        attempt_left=3;
        user_password[0]='\0';
        user_password[1]='\0';
        user_password[2]='\0';
        user_password[3]='\0';
        key=ALL_RELEASED;
        return_time=5;
    }
    
    
    
    //SW4=>1,SW4=>0
    if(key==SW4 && i<4)
    {
        clcd_putch('*',LINE2(4 + i));
        user_password[i]='1';
        i++;
        return_time=5;

    }
    else if(key==SW5 && i<4)
    {
        clcd_putch('*',LINE2(4 + i));
         user_password[i]='0';
         i++;
         return_time=5;//provide fresh new 5 sec before pressing a switch

    }
    if(return_time==0)
    {
        //change back to dashboard SCRN
        return RETURN_BACK;
    }
    //check we read 4 digit pasword
    if(i==4)
    {
        char s_passwd[4];
        //read the stored passwd from the user
        for(int j=0;j<4;j++)
        {
          s_passwd[j]=  ext_eeprom_24C02_read(j);
        }
    
        //compare stored paswd and user passwd
        if(strncmp(user_password,s_passwd,4)==0)
        {
            clear_display();
             clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
            __delay_us(100);
            clcd_print("Login Success",LINE1(1));
            __delay_ms(3000);
            //change to main menu scrn
            return LOGIN_SUCCESS;
        }  
        else
        {
            attempt_left--;
            if(attempt_left==0)
            {
                clear_display();
                 clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                __delay_us(100);
                clcd_print("you are blocked",LINE1(0));
                clcd_print("wait for ",LINE2(0));
                clcd_print("secs ",LINE2(12));

                //wait for 60s
                sec=60;
                while(sec)
                {
                    clcd_putch((sec/10)+'0',LINE2(9));
                     clcd_putch((sec%10)+'0',LINE2(10));
                     

                }
                
                 __delay_ms(3000);
                 attempt_left=3;
            }
            else
            {
                clear_display();
                 clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                __delay_us(100);
                clcd_print("wrong password",LINE1(0));
                clcd_print(" attempt left",LINE2(2));
                clcd_putch(attempt_left +'0',LINE2(0));//as attempt left is digit convert it to character 
                 __delay_ms(3000);
            }
              clear_display();
              clcd_print("Enter Password",LINE1(1));
             clcd_write(LINE2(4),INST_MODE);//BRINGING cursor to 2nd line as it is instructn we pass 2nd argument as 0IE,MACR0 IST_MODE
             clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);//FOR CURSOR BLINKING
            __delay_us(100);
            i=0;
        }
    }
}

// function displays login menu screen
char menu_screen(unsigned char key, unsigned char reset_flag)
{
    static menu_pos, key_flag;
    if(reset_flag == RESET_MENU) // reset variables if reset flag passed
    {
        menu_pos = 0;
        key_flag = 0;
        key = ALL_RELEASED;
        clear_display();
    }
    
    if(key == SW5 && menu_pos < 4) // increment menu selector
    {
        menu_pos++;
        key_flag = 0;
        clear_display();
    }    
    else if(key == SW4 && menu_pos > 0) // decrement menu selector
    {
        menu_pos--;
        key_flag = 1;
        clear_display();
    }    
    // handle edge case of menu selector being 0
    if(menu_pos == 0) 
    {
        clcd_putch('*',LINE1(0));
        clcd_print(menu[menu_pos],LINE1(2));
        clcd_print(menu[menu_pos + 1],LINE2(2));
    }
    else
    {
        clcd_putch('*',LINE2(0));
        clcd_print(menu[menu_pos - 1],LINE1(2));
        clcd_print(menu[menu_pos],LINE2(2));
    }

    return menu_pos;
}

// function to display stored logs

void view_log(unsigned char key, unsigned char reset_flag)
{
    char slog[11];// Buffer to hold a single log entry (10 bytes + null if needed)
    unsigned char add; // EEPROM address being accessed
    static int spos;// Current log index (position)
    static unsigned char log_state = 0; // 0 - show logs, 1 - wait to return

    if(reset_flag == RESET_SPOS || reset_flag == RESET_VIEW_LOG_POS)
    {
        spos = 0;// Start from the first log
        log_state = 0; // Set state to view log
        return;
    }            

    if(log_pos > 0 && log_state == 0) // if at least one log is present and not done viewing
    {
        //scroll the log
        if(key == SW4 && spos < log_pos - 1)
        {
            spos++;
        }
        else if(key == SW5 && spos > 0)
        {
            spos--;
        }
       // Read the log entry from EEPROM (each log is 10 bytes, starts from addr 5)
        for(int i = 0; i < 10; i++)
        {
            add = (spos * 10) + 5 + i;   // Calculate EEPROM address of the i-th byte of the log
            slog[i] = ext_eeprom_24C02_read(add);// Read the byte into slog
        }

        clcd_print("Viewing Logs     ", LINE1(0));//Print title on LC
        clcd_putch(spos + '0', LINE2(0));
         // Display time part: HH:MM:SS (from slog[0] to slog[5])
        clcd_putch(slog[0], LINE2(2));
        clcd_putch(slog[1], LINE2(3));
        clcd_putch(':', LINE2(4));
        clcd_putch(slog[2], LINE2(5));
        clcd_putch(slog[3], LINE2(6));
        clcd_putch(':', LINE2(7));
        clcd_putch(slog[4], LINE2(8));
        clcd_putch(slog[5], LINE2(9));
         // Display event or status part (e.g., gear info, collision, etc.)
        clcd_putch(slog[6], LINE2(11));
        clcd_putch(slog[7], LINE2(12));
        clcd_putch(slog[8], LINE2(14));
        clcd_putch(slog[9], LINE2(15));

        // If it's the last log, 
        if (spos == log_pos - 1)
        {
            clcd_print("LPSW4->Main Menu", LINE1(0));// Show return hint
            log_state = 1; // mark log viewing done
        }
    }
    // If no logs are present in EEPROM
    else if(log_pos == 0)
    {
        clcd_print("No logs         ", LINE1(0));
        clcd_print("LPSW4->Main Menu", LINE2(0));
        log_state = 1;
    }
}

void clear_log(void) {
    for (unsigned char addr = 0x05; addr < 0x05 + log_pos * 10; addr++) {
        ext_eeprom_24C02_byte_write(addr, 0xFF);
    }
    log_pos = 0;
    clear_display();
    clcd_print("Logs cleared ",LINE1(1));
    clcd_print("successfully", LINE2(1));
    __delay_ms(2000);
    
}

// function prints all log data stored in external EEPROM into UART terminal(TERA TERM)
void download_log_uart(unsigned char reset_flag)
{
    static unsigned char download_done = 0;
    if(reset_flag == PRINT_UART && download_done == 0) // reset flag used so that UART printing happens only once
    {
        puts("=== DOWNLOADING LOGS ===");
        putchar('\n');
        char disp_str[10];
        for(int log = 0; log < log_pos; log++) // iterate over all the logs
        {
            for(int i = 0; i < 10; i++) // read all data from external EEPROM for each log
            {            
                disp_str[i] = ext_eeprom_24C02_read((log*10)+5+i);
            }
            // format and ouput log data onto serial terminal
            putchar(log + '0');
            putchar(' ');
            putchar(disp_str[0]);
            putchar(disp_str[1]);
            putchar(':');
            putchar(disp_str[2]);
            putchar(disp_str[3]);
            putchar(':');
            putchar(disp_str[4]);
            putchar(disp_str[5]);
            putchar(' ');
            putchar(disp_str[6]);
            putchar(disp_str[7]);
            putchar(' ');
            putchar(disp_str[8]);
            putchar(disp_str[9]);
            putchar('\n');
        }
        puts("=== DOWNLOAD COMPLETE ===");
        putchar('\n');
    }
    else // display message once download of log data is done
    {
        clcd_print("DOWNLOADED DATA ",LINE1(0));
        clcd_print("return to main  ",LINE2(0));
    }
}



// function changes password used to access login screen

unsigned char change_password(unsigned char key, unsigned char reset_flag )
{
	static char r_pwd[9];// Stores the full 8-character password (4 for entry + 4 for re-entry) + 1 null terminator
	static int n_p_pos = 0;// Position tracker for how many characters have been entered
	static char p_chg = 0;// Flag indicating whether password has already been changed in this session
    
    

    /*logic to change password*/
// This resets everything when called with reset_flag
	if ( reset_flag == RESET_PASSWORD )
	{
		strncpy(r_pwd, "    ",4);// Clear the first 4 characters
		n_p_pos = 0; // Reset position counter to start entering from beginning
		p_chg = 0;    // Reset change flag, so password can be changed
		return_time = 30;// Set timeout counter to 30 
        
        
	}
     //If time runs out (e.g., user is inactive), return back to main menu
	if (!return_time)
		return RETURN_BACK;
     // If password already changed, return without doing anything
	if(p_chg)
		return TASK_FAIL;
    // Prompt the user to enter new password (for first 4 digits)
	if ( n_p_pos < 4 )
    {
	clcd_print("Enter new pwd    ",LINE1(0));
    clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
    __delay_us(100);
    }
    // After first 4 digits entered, prompt to re-enter for confirmation
	else if ( n_p_pos > 3 && n_p_pos < 8 )
		clcd_print("Re enter new pwd    ",LINE1(0));
    
	
    
// Read key press input
	switch( key)
	{
		case SW5 :// If SW5 is pressed, store '0'
			r_pwd[n_p_pos] = '0';// Store '0' at current position
			clcd_putch('*',LINE2(n_p_pos%4)); // Display * on screen
			n_p_pos++;// Move to next position
            return_time = 30; // Reset inactivity timeout
            if ( n_p_pos == 4 )
            CLEAR_LINE2;// Clear second line before re-entry
			break;
            
		case SW4 :
			r_pwd[n_p_pos] = '1';// Store '1' at current position
			clcd_putch('*',LINE2(n_p_pos%4));
			n_p_pos++;
            return_time = 30;
            if ( n_p_pos == 4 )
            CLEAR_LINE2;
             
			break;
	}
// After all 8 characters are entered (4 + 4), do password match check
	if ( n_p_pos == 8)
	{
		if ( strncmp(r_pwd, &r_pwd[4], 4) == 0 )// Compare first 4 chars with last 4 chars
		{
        /*sucessfully changing password*/
			r_pwd[8]=0;
			ext_eeprom_24C02_str_write( 0x00, &r_pwd[4]);
			n_p_pos++;
            clear_display();
			clcd_print("Password changed",LINE1(0));
			clcd_print("successfully ",LINE2(2));
			p_chg = 1;// Mark that password was changed successfully
			return TASK_SUCCESS;
		}
		else
		{
            /*displaying password change fail*/
            clear_display();
			clcd_print ( "Password  change" , LINE1(0));
			clcd_print("failed",LINE2(5));
			p_chg = 1;
			return TASK_SUCCESS;
		}
	}

	return TASK_FAIL;
}
//function to change the time
char set_time(unsigned char key ,unsigned char reset_time) 
    {
    /*logic to change time*/
    /* Variables used to update time */
	static unsigned int new_time[3]; // Stores new hours (0), minutes (1), and seconds (2)
	static int blink_pos ;// Tracks which time field (HH/MM/SS) is currently selected for editing
	static unsigned char wait , blink ;// Used for blinking effect during time entry
	char dummy;  // Temporary variable to hold BCD-encoded time values
	static char t_done = 0;  // Flag to indicate if time update is complete
    
       //  initialize time setting mode
	if ( reset_time == RESET_TIME )
	{
		get_time();// Reads current time into global 'time' array
        // Convert BCD to decimal: (upper nibble * 10) + lower nibble
		new_time[0] = (time[0] & 0x0F ) * 10 + (time[1] & 0x0F) ;// Hours
		new_time[1] = (time[2]  & 0x0F ) * 10 + (time[3] & 0x0F) ; // Minutes
		new_time[2] = (time[4] & 0x0F ) * 10 + (time[5] & 0x0F) ;// Seconds
		clcd_print("Time (HH:MM:SS)",LINE1(0));// Display title on LCD
		blink_pos = 2;// Start editing seconds
		wait = 0;    // Reset blink timer
		blink = 0;   // Blink OFF initially
		key = ALL_RELEASED;
		t_done = 0;
	}
  // If time was already set successfully, stop further updates
	if( t_done)
		return TASK_FAIL;
 // Handle key presses for time adjustment
	switch ( key )
	{

		case SW4 :
			new_time[blink_pos]++;// Increment the value at current field (HH/MM/SS)
			break;
		case SW5 :
			blink_pos = (blink_pos + 1) % 3;// Move to next field (loop between 0-1-2
			break;
		case SW6 :
            /*storing new time*/
			get_time();// Read current time from RTC again to keep consistency
			dummy = ((new_time[0] / 10 ) << 4 ) | new_time[0] % 10 ;// BCD encode hour
			clock_reg[0] = (clock_reg[0] & 0xc0) | dummy ;// Preserve 12/24hr format bits
			write_ds1307(HOUR_ADDR, clock_reg[0] ); // Write hour to RTC
			dummy = ((new_time[1] / 10 ) << 4 ) | new_time[1] % 10 ;// BCD encode min
			clock_reg[1] = (clock_reg[1] & 0x80) | dummy;
			write_ds1307(MIN_ADDR, clock_reg[1] );
			dummy = ((new_time[2] / 10 ) << 4 ) | new_time[2] % 10 ;// BCD encode sec
			clock_reg[2] = (clock_reg[2] & 0x80) | dummy;
			write_ds1307(SEC_ADDR, clock_reg[2] );
			clear_display();                     // Clear the LCD
			clcd_print("Time changed",LINE1(2)); // Success message
			clcd_print("Successfully",LINE2(2));
            for(unsigned long int i = 0;i < 20000;i++);//delay
			t_done = 1;
			return  TASK_SUCCESS;
	}
// If values exceed valid range, roll over
	if( new_time[0]  > 23 )
		new_time[0] = 0;
	if ( new_time[1] > 59 )
		new_time[1] = 0;
	if ( new_time[2] > 59 )
		new_time[2] = 0;
// Blinking logic and display update (every 50 calls)
	if ( wait ++ == 50 )
	{
		wait = 0;
		blink = !blink;
		clcd_putch( new_time[0] / 10 + 48, LINE2(0));
		clcd_putch( new_time[0] % 10 + 48 , LINE2(1));
		clcd_putch( ':', LINE2(2));
		clcd_putch( new_time[1] / 10 + 48, LINE2(3));
		clcd_putch( new_time[1] % 10 + 48, LINE2(4));
		clcd_putch( ':', LINE2(5));
		clcd_putch( new_time[2] / 10 + 48 , LINE2(6));
		clcd_putch( new_time[2] % 10 + 48, LINE2(7));
/*logic to blink at the current pos*/

		if ( blink)
		{
			switch( blink_pos )
			{
				case 0:
					clcd_print("  ",LINE2(0));
					break;
				case 1:
					clcd_print("  ",LINE2(3));
					break;
				case 2:
					clcd_print("  ",LINE2(6));
					break;
			}
		}
	}
	return TASK_FAIL;
}


void clear_display()
{
     clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
            __delay_us(500); 
}