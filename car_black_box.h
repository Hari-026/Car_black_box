#ifndef CAR_BLACK_BOX_H
#define	CAR_BLACK_BOX_H

// function declarations
void display_dashboard(unsigned char event[], unsigned char speed);
void log_event(unsigned char event[], unsigned char speed);
unsigned char login(unsigned char key, unsigned char reset_flag);
void clcd_write(unsigned char byte, unsigned char mode);
char menu_screen(unsigned char key, unsigned char reset_flag);
void view_log(unsigned char key, unsigned char reset_flag);
void clear_log(void);
void download_log_uart(unsigned char reset_flag);
unsigned char change_password(unsigned char key, unsigned char reset_flag);
char set_time(unsigned char key ,unsigned char reset_time);
void clear_display();
#endif	/* CAR_BLACK_BOX_H */