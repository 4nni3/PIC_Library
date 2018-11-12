#include <xc.h>

void oled_init();
void oled_clear();
void oled_str(char *str, char lineNum);
void oled_addStr(char *str);
void oled_bigStr(char *str, char lineNum);