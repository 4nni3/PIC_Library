#define _XTAL_FREQ 32000000

#define LCD_ADD 0x7c

#define CLEAR 0x01
#define HOME 0x02

void lcd_init();
void lcd_cmd(char c);
void lcd_dat(char d);
void lcd_str(char *str);
