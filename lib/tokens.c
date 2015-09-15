#include <stdio.h>

#include "const.h"
#include "utils.h"
#include "tokens.h"
#include "hd44780.h"

struct token_s keys_names_list[]={
   {"interface",       INTERFACE_ID},
   {"i2c_port",        I2C_PORT_ID},
   {"i2c_device_type", I2C_DEVICE_TYPE_ID},
   {"i2c_device_addr", I2C_DEVICE_ADDR_ID},
   {"pcf8574_pin_map", PCF8574_PIN_MAP_ID},
   {"lcd_type",        LCD_TYPE_ID},
   {"lcd_screen",      LCD_SCREEN_ID},
   {NULL,0}
};

// liste des interfaces connues (uniquement i2c pour l'instant)
struct token_s interface_list[]={
   {"i2c",       I2C_ID},
   {NULL,0}
};

// liste des "periphériques" i2c connus (uniquement pcf8574 pour l'instant)
struct token_s i2c_device_type_list[]={
   {"pcf8574",   PCF8574_ID},
   {NULL,0}
};

// liste de type de LCD connus pour le moment
struct token_s lcd_type_list[]={
   {"hd44780",   HD44780_ID},
   {NULL,0}
};

// configurations d'ecran lcd connus
struct token_s lcd_screen_list[]={
   {"8x1",    LCD8x1},
   {"16x1T1", LCD16x1T1},
   {"16x1T2", LCD16x1T2},
   {"8x2",    LCD8x2},
   {"16x2",   LCD16x2},
   {"20x2",   LCD20x2},
   {"16x4",   LCD16x4},
   {"20x4",   LCD20x4},
   {NULL,0}
};


int getTokenID(struct token_s tokens_list[], char *str)
{
   if(!str)
      return -1;
 
   uint16_t i;
   for(i=0;tokens_list[i].str;i++)
   {
      if(strcmplower(tokens_list[i].str, str) == 0)
         return tokens_list[i].id;
   }
   return -1;
}
