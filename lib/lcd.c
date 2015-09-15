#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdarg.h>

#include "lcd.h"


struct lcd_s *lcd_alloc()
{
   struct lcd_s *lcd = (struct lcd_s *)malloc(sizeof(struct lcd_s));
   
   return lcd;
}


void lcd_free(struct lcd_s **lcd)
{
   free(*lcd);
   *lcd=NULL;
}


int lcd_init(struct lcd_s *lcd, int reset)
{
   lcd->x=0;
   lcd->y=0;
   if(lcd->iface_init)
      return lcd->iface_init(lcd->iface_context, reset);
   else
      return -1;
}


int lcd_close(struct lcd_s *lcd)
{
   if(lcd->iface_close)
      return lcd->iface_close(lcd->iface_context);
   else
      return -1;
}


int lcd_backlight(struct lcd_s *lcd, enum backlight_state_e state)
{
   if(lcd->iface_backlight)
      return lcd->iface_backlight(lcd->iface_context, state);
   else
      return -1;
}


int lcd_clear(struct lcd_s *lcd)
{
   if(lcd->iface_clear)
      return lcd->iface_clear(lcd->iface_context);
   else
      return -1;
}


int lcd_gotoxy(struct lcd_s *lcd, uint16_t x, uint16_t y)
{
   if(!lcd->iface_gotoxy)
      return -1;
      
   lcd->x=x;
   lcd->y=y;

   return 0;
//   return lcd->iface_gotoxy(lcd->iface_context, lcd->x, lcd->y);
}

   
int lcd_print(struct lcd_s *lcd, char *str)
{
   if(lcd->iface_print)
      return lcd->iface_print(lcd->iface_context, &(lcd->x), &(lcd->y), str);
   else
      return -1;
}


int lcd_printf(struct lcd_s *lcd, char const* fmt, ...)
{
   char *str=(char *)malloc(lcd->nb_rows * lcd->nb_columns);
   if(!str)
      return -1;
      
   va_list args;

   va_start(args, fmt);
   vsnprintf(str, lcd->nb_rows * lcd->nb_columns, fmt, args);
   va_end(args);

   int ret=lcd_print(lcd, str);
   
   free(str);
   
   return ret;
}


