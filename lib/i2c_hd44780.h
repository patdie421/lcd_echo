#ifndef __i2c_hd44780_h
#define __i2c_hd44780_h

#include <inttypes.h>

#include "lcd.h"
#include "hd44780.h"
#include "i2c_hd44780_pcf8574.h" 

enum device_type_e { PCF8574=1 };

struct i2c_device_drivers_s
{
   int (*device_init)(int fd, char *pins_map);
   int (*device_write_byte)(int fd, unsigned char data, enum i2c_hd44780_pcf8574_RS_e rs, int blight, i2c_hd44780_pcf8574_pins_map_t pcf8574_pins);
   int (*device_backlight)(int fd, int blight, char *pins_map);
};

struct i2c_hd44780_iface_context_s
{
   int port;
   int device;
   enum device_type_e device_type;
   struct i2c_device_drivers_s i2c_device_drivers;
   
   int fd;
   enum hd44780_type_e hd44780_type;
   int backlight;

   char pins_map[8];
};


int i2c_hd44780_backlight(void *iface_context, int backlight);
int i2c_hd44780_init(void *iface_context, int reset);
int i2c_hd44780_close(void *iface_context);
int i2c_hd44780_gotoxy(void *iface_context, uint16_t x, uint16_t y);
int i2c_hd44780_clear(void *iface_context);
int i2c_hd44780_print(void *iface_context, uint16_t *x, uint16_t *y, char *str);

struct i2c_hd44780_iface_context_s *i2c_hd44780_iface_context_alloc(int port, int device, enum device_type_e device_type, enum hd44780_type_e hd44780_type, int *pins_map);
void i2c_hd44780_iface_context_free(struct i2c_hd44780_iface_context_s **iface_context);

struct lcd_s *i2c_hd44780_lcd_alloc( char *params[]);

#endif
