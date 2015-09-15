#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <inttypes.h>
#include <string.h>

#include "const.h"
#include "utils.h"
#include "tokens.h"

#include "i2c.h"
#include "hd44780.h"
#include "i2c_hd44780.h"
#include "i2c_hd44780_pcf8574.h"


struct i2c_hd44780_iface_context_s *i2c_hd44780_iface_context_alloc(int port, int device, enum device_type_e device_type, enum hd44780_type_e hd44780_type, int *pins_map)
{
   struct i2c_hd44780_iface_context_s *iface_context = (struct i2c_hd44780_iface_context_s *)malloc(sizeof(struct i2c_hd44780_iface_context_s));

   if(!iface_context)
      return NULL;

   iface_context->fd=-1;
   iface_context->port=port;
   iface_context->device_type=device_type;
   iface_context->device=device;
   iface_context->hd44780_type=hd44780_type;
   iface_context->backlight=0;

   int i=0;
   for(i=0;i<8;i++)
   {
      iface_context->pins_map[i]=pins_map[i];
   }

   switch(iface_context->device_type)
   {
      case PCF8574:
         iface_context->i2c_device_drivers.device_init       = i2c_hd44780_pcf8574_init;
         iface_context->i2c_device_drivers.device_write_byte = i2c_hd44780_pcf8574_write_byte;
         iface_context->i2c_device_drivers.device_backlight  = i2c_hd44780_pcf8574_backlight;
         break;
      default:
         return NULL;
   }

   return iface_context;
}


void i2c_hd44780_iface_context_free(struct i2c_hd44780_iface_context_s **iface_context)
{
   free(*iface_context);
   *iface_context=NULL;
}


int i2c_hd44780_backlight(void *iface_context, int blight)
{
   struct i2c_hd44780_iface_context_s *context=(struct i2c_hd44780_iface_context_s *)iface_context;

   if(!context || !context->i2c_device_drivers.device_backlight)
      return -1;

   context->backlight=blight;

   context->i2c_device_drivers.device_backlight(context->fd, context->backlight, context->pins_map);

   return 0;
}


int i2c_hd44780_write_data(void *iface_context, char data)
{
   struct i2c_hd44780_iface_context_s *context=(struct i2c_hd44780_iface_context_s *)iface_context;

   if(!context || !context->i2c_device_drivers.device_write_byte)
      return -1;

   context->i2c_device_drivers.device_write_byte(context->fd, data, DATA, context->backlight, context->pins_map);

   return 0;
}


int i2c_hd44780_write_cmnd(void *iface_context, char data)
{
   struct i2c_hd44780_iface_context_s *context=(struct i2c_hd44780_iface_context_s *)iface_context;

   if(!context || !context->i2c_device_drivers.device_write_byte)
      return -1;

   context->i2c_device_drivers.device_write_byte(context->fd, data, CMND, context->backlight, context->pins_map);

   return 0;
}


int i2c_hd44780_init(void *iface_context, int reset)
{
   struct i2c_hd44780_iface_context_s *context=(struct i2c_hd44780_iface_context_s *)iface_context;

   if(!context || !context->i2c_device_drivers.device_init)
      return -1;

   context->fd=i2c_open(context->port);

   if(context->fd<0)
   {
      return -1;
   }

   i2c_select_slave(context->fd, context->device);

   if(reset)
      context->i2c_device_drivers.device_init(context->fd, context->pins_map);

   return 0;
}


int i2c_hd44780_close(void *iface_context)
{
   if(!iface_context)
      return -1;

   struct i2c_hd44780_iface_context_s *context=(struct i2c_hd44780_iface_context_s *)iface_context;

   return i2c_close(context->fd);
}


int i2c_hd44780_gotoxy(void *iface_context, uint16_t x, uint16_t y)
{
   struct i2c_hd44780_iface_context_s *context=(struct i2c_hd44780_iface_context_s *)iface_context;

   if(!context)
      return -1;

   int addr=hd44780_get_char_addr(context->hd44780_type, x, y);

   if(addr<0)
      return -1;

   char cmnd=(char)(0b10000000 | (addr & 0b01111111));

   int ret=i2c_hd44780_write_cmnd(iface_context, cmnd);

   mymicrosleep(50);

   return ret;
}


int i2c_hd44780_clear(void *iface_context)
{
   struct i2c_hd44780_iface_context_s *context=(struct i2c_hd44780_iface_context_s *)iface_context;

   int ret=i2c_hd44780_write_cmnd(iface_context, HD44780_CMD_CLEAR_SCREEN);

   mymicrosleep(1500);

   return ret;
}


int i2c_hd44780_print(void *iface_context, uint16_t *x, uint16_t *y, char *str)
{
   struct i2c_hd44780_iface_context_s *context=(struct i2c_hd44780_iface_context_s *)iface_context;

   if(!context)
      return -1;

   int i;
   int set_addr_flag=1;
   for(i=0;str[i];i++)
   {
      if(str[i]=='\n')
      {
         *y=*y+1;
         *x=0;
         set_addr_flag=1;
         continue;
      }

      if(str[i]<0b0001000 || str[i]>0b01111010) // caractère affichable ?
         continue;

      if(set_addr_flag==1)
      {
         int addr=hd44780_get_char_addr(context->hd44780_type, *x, *y);
         if(addr<0)
            return -1;
         char cmnd=(char)(0b10000000 | (addr & 0b01111111));
         i2c_hd44780_write_cmnd(context, cmnd);
         mymicrosleep(50);
      }

      int ret=i2c_hd44780_write_data(context, str[i]);
      if(ret<0)
         return -1;

      ret=hd44780_next_xy(context->hd44780_type, x, y);
      if(ret<0)
         return -1;
      else if(ret==1) // saut de ligne au prochain tour ?
         set_addr_flag=1;
      else
         set_addr_flag=0;
   }

   return 0;
}


struct lcd_s *i2c_hd44780_lcd_alloc( char *params[])
{
   int n = -1;

   struct lcd_s *lcd;
   lcd=lcd_alloc();
   if(lcd==NULL)
   {
      fprintf(stderr,"%s - malloc error : ",error_str);
      return NULL;
   }


   int port = -1;
   if(params[I2C_PORT_ID]==NULL)
   {
      fprintf(stderr,"%s - i2c_port is mandatory for i2c interface\n", error_str);
      return NULL;
   }
   int ret=sscanf(params[I2C_PORT_ID],"%d%n",&port,&n);
   if(port < 0 || n != strlen(params[I2C_PORT_ID]))
   {
      fprintf(stderr,"%s - i2c_port must be numeric and positiv\n", error_str);
      return NULL;
   }
#ifdef DEBUG
   fprintf(stderr,"%s - i2c_port = %d\n", debug_str, port);
#endif 

   int addr = -1;
   if(params[I2C_DEVICE_ADDR_ID]==NULL)
   {
      fprintf(stderr,"%s - i2c_device_addr is mandatory for i2c interface\n", error_str);
      return NULL;
   }
   if(params[I2C_DEVICE_ADDR_ID][1]=='x') // decimal ou hexa ?
      sscanf(params[I2C_DEVICE_ADDR_ID],"%x%n",&addr,&n);
   else
      sscanf(params[I2C_DEVICE_ADDR_ID],"%d%n",&addr,&n);
 
   if(addr < 1 || addr > 127 || n != strlen(params[I2C_DEVICE_ADDR_ID]))
   {
      fprintf(stderr,"%s - i2c_device_addr must be in range 1 to 127\n", error_str);
      return NULL;
   }
#ifdef DEBUG
   else
      fprintf(stderr,"%s - i2c_device_addr = %x\n", debug_str, addr);
#endif 

   int lcd_screen = getTokenID(lcd_type_list, params[LCD_TYPE_ID]);
   if(lcd_screen<0)
   {
      fprintf(stderr,"%s - unknown lcd type : %s\n", error_str, params[LCD_TYPE_ID]);
      return NULL;
   }

   
   int i2c_device_type_id = getTokenID(i2c_device_type_list, params[I2C_DEVICE_TYPE_ID]);
   if(i2c_device_type_id<0)
   {
      fprintf(stderr,"%s - unknown i2c_device_type : %s\n", error_str, params[I2C_DEVICE_TYPE_ID]);
      return NULL;
   }


   switch(i2c_device_type_id)
   {
      case PCF8574_ID:
      {
         int pcf8574_map[8];
         int i,j;

         if(params[PCF8574_PIN_MAP_ID]==NULL)
         {
            fprintf(stderr,"%s - pcf8574_pin_map is mandatory for i2c/pcf8574 interface/device\n", error_str);
            return NULL;
         }
         sscanf(params[PCF8574_PIN_MAP_ID],"%d,%d,%d,%d,%d,%d,%d,%d%n",&pcf8574_map[0],&pcf8574_map[1],&pcf8574_map[2],&pcf8574_map[3],&pcf8574_map[4],&pcf8574_map[5],&pcf8574_map[6],&pcf8574_map[7],&n);
         if(n != strlen(params[PCF8574_PIN_MAP_ID]))
         {
             fprintf(stderr,"%s - incorrect or incomplet pcf8574 mapping\n", error_str);
             return NULL;
         }
         for(i=0;i<8;i++)
         {
            if(pcf8574_map[i]<0 || pcf8574_map[i]>7)
            {
               fprintf(stderr,"%s - incorrect pcf8574 mapping value (%d)\n", error_str, pcf8574_map[i]);
               return NULL;
            }

            for(j=i+1;j<8;j++)
            {
               if(pcf8574_map[i]==pcf8574_map[j])
               {
                  fprintf(stderr,"%s - duplicate pcf8574 mapping value (%d)\n", error_str, pcf8574_map[i]);
                  lcd_free(&lcd);
                  return NULL;                    
               }
            }
         }

         lcd->iface_context   = i2c_hd44780_iface_context_alloc(port, addr, PCF8574, lcd_screen, pcf8574_map);
         if(lcd->iface_context == NULL)
         {
            lcd_free(&lcd);
            lcd=NULL;
            return NULL;
         }

         lcd->iface_init      = i2c_hd44780_init;
         lcd->iface_close     = i2c_hd44780_close;
         lcd->iface_backlight = i2c_hd44780_backlight;
         lcd->iface_clear     = i2c_hd44780_clear;
         lcd->iface_print     = i2c_hd44780_print;
         lcd->iface_gotoxy    = i2c_hd44780_gotoxy;
         return lcd;
         break;
      }

      default:
         fprintf(stderr,"%s - unknown i2c_device_type : %s\n", error_str, params[I2C_DEVICE_TYPE_ID]);
         return NULL;
   }
}
