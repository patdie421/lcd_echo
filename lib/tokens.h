#ifndef __tokens_h
#define __tokens_h

enum token_id_e {
   _FIRST_PARAM=0,

   INTERFACE_ID=_FIRST_PARAM,
   I2C_PORT_ID,
   I2C_DEVICE_TYPE_ID,
   I2C_DEVICE_ADDR_ID,
   PCF8574_PIN_MAP_ID,
   LCD_TYPE_ID,
   LCD_SCREEN_ID,
   _LAST_PARAM,

   _FIRST_VALUE=_LAST_PARAM,

   PCF8574_ID,
   I2C_ID=_FIRST_VALUE,
   HD44780_ID,
/*
   LCD8X1_ID,
   LCD16X1T1_ID,
   LCD16X1T2_ID,
   LCD8X2_ID,
   LCD16X2_ID,
   LCD20X2_ID,
   LCD16X4_ID,
   LCD20X4_ID
*/
};

struct token_s
{
   char *str;
   enum token_id_e id;
};

extern struct token_s keys_names_list[];
extern struct token_s interface_list[];
extern struct token_s i2c_device_type_list[];
extern struct token_s lcd_type_list[];
extern struct token_s lcd_screen_list[];

int getTokenID(struct token_s tokens_list[], char *str);

#endif