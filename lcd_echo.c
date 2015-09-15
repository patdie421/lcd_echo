#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <getopt.h>

#include "const.h"
#include "utils.h"
#include "lcd.h"
#include "tokens.h"
#include "i2c_hd44780.h"
#include "i2c_hd44780_pcf8574.h"

//
// doc à voir :
// http://forum.allaboutcircuits.com/threads/clarification-on-hd44780-20x4.69302/
// https://fr.wikipedia.org/wiki/HD44780
// http://web.alfredstate.edu/weimandn/lcd/lcd_addressing/lcd_addressing_index.html
//

int load_config_file(char *file, char *params[])
{
   FILE *fd;
 
   fd=fopen(file, "r");
   if(!fd)
   {
      perror("");
      return -1;
   }
 
   int i;
 
   for(i=0;i<_LAST_PARAM-_FIRST_PARAM;i++)
   {
      free(params[i]);
      params[i]=NULL;
   }
 
   char line[255];
   char *noncomment[2];
   char key[20], value[40];
   char *tkey,*tvalue;
 
   i=0;
   while(!feof(fd))
   {
      if(fgets(line,sizeof(line),fd) != NULL)
      {
         i++;
         // suppression du commentaire éventuellement présent dans la ligne
         strsplit(line, '#', noncomment, 2);
         char *keyvalue=strtrim(noncomment[0]);
         if(strlen(keyvalue)==0)
            continue;
 
         // lecture d'un couple key/value
         key[0]=0;
         value[0]=0;
         sscanf(keyvalue,"%[^=]=%[^\n]", key, value);
 
         if(key[0]==0 || value[0]==0)
         {
            fprintf(stderr,"%s - syntax error : line %d not a \"key = value\" line\n", i, warning_str);
            goto load_config_file_clean_exit;
         }
         
         char *tkey=strtrim(key);
         char *tvalue=strtrim(value);
         strtolower(tkey);
         strtolower(tvalue);

         int id;
         id=getTokenID(keys_names_list, tkey);
         if(id<0)
         {
            fprintf(stderr,"%s - Unknow parameter\n", warning_str);
            continue;
         }
 
         params[id-_FIRST_PARAM]=(char *)malloc(strlen(tvalue)+1);
         if(params[id-_FIRST_PARAM]==NULL)
         {
            fprintf(stderr,"%s - malloc error\n", warning_str);
            continue;
         }
 
         strcpy(params[id-_FIRST_PARAM],tvalue);
      }
   }
 
   return 0;
   
load_config_file_clean_exit:
   for(i=0;i<_LAST_PARAM-_FIRST_PARAM;i++)
   {
      free(params[i]);
      params[i]=NULL;
   }
   return -1;
}
 
 
static struct option long_options[] = {
   {"interface",         required_argument, 0,  'c'                  },
   {"x",                 required_argument, 0,  'x'                  },
   {"y",                 required_argument, 0,  'y'                  },
   {"nobacklight",       no_argument,       0,  'n'                  },
   {"reset",             no_argument,       0,  'r'                  },
   {"cls",               no_argument,       0,  'e'                  },
   {"help",              no_argument,       0,  'h'                  },
   {0,                   0,                 0,  0                    }
};


void usage(char *cmd)
{
   char *usage_text[]={
     //12345678901234567890123456789012345678901234567890123456789012345678901234567890
      "",
      "echo de caractères sur un écran LCD.",
      "",
      "Options :",
      "  --interface, -c     : fichier de configuration d'un écran lcd.",
      "                        Par défaut le fichier est /etc/lcd_echo.cfg.",
      "  --x, -x             : affichage du premier caractère à la colonne x. La",
      "                        première colonne est numérotée 0. Par défaut, x=0.",
      "  --y, -y             : affichage du premier caractère à la ligne y. La",
      "                        première ligne est numérotée 0. Par défaut, y=0.",
      "  --nobacklight       : le rétroéclairage est éteint (allumé par défaut).",
      "  --reset             : réinitialisation complète de l'afficheur.",
      "  --cls, -e           : efface l'écran",
      "  --help, -h          : affiche ce message."
      "",
      "Utilisation :",
      "   Initialisation de l'afficheur (à faire avant le premier affichage)",
      "   $ echo_lcd -r",
      "   Efface l'afficheur et affiche hello",
      "   $ echo_lcd --cls \"hello\"",
      "   On ajout \", world\" après \"hello\"",
      "   $ echo_lcd --x=6 \", world\"",
      "   Avec un fichier de configuration personnalisé",
      "   $ echo_lcd --interface=i2c_hd44780.cfg \"interface I2C\"",
      "",
      "Afficheur pris en charge :",
      "- HD44780 via interface I2C/PCF8574(A)",
      "- (autres à venir)",
      "",
      NULL
   };
   
   if(!cmd)
      cmd="echo_lcd";
   printf("\nusage : %s [options] [text à afficher]\n", cmd);
   int16_t i;
   for(i=0;usage_text[i];i++)
      printf("%s\n",usage_text[i]);
}
 
 
int main(int argc, char *argv[])
{
   int i=0;
   int ret=0;
   char *params[_LAST_PARAM - _FIRST_PARAM];
   struct lcd_s *lcd=NULL;
  
   // raz params
   for(i=0;i<_LAST_PARAM - _FIRST_PARAM;i++)
      params[i]=NULL;
 
   int option_index = 0; // getopt_long function need int
   int c; // getopt_long function need int
   int n=0;
   int x=0,y=0;
   uint16_t reset=0, cls=0;
   uint16_t backlight = 1;
  
   char arg[255], *targ; // copie optarg + ptr trim
   char *cfgfile=NULL;
  
   while ((c = getopt_long(argc, (char * const *)argv, "nrehc:x:y:", long_options, &option_index)) != -1)
   {
      if(optarg)
         strcpy(arg, optarg);

      char *targ=strtrim(arg);
 
      switch (c)
      {
         case 'h':
            usage((char *)argv[0]);
            exit(0);
            break;
         case 'c':
            if(cfgfile)
            {
               free(cfgfile);
               cfgfile=NULL;
            }
            cfgfile=(char *)malloc(strlen(targ)+1);
            if(cfgfile==NULL)
            {
               fprintf(stderr,"%s - malloc error : ",error_str);
               perror("");
               goto main_clean_exit;
            }
            strcpy(cfgfile, targ);
            break;
          case 'x':
            sscanf(targ,"%d%n",&x,&n);
            if(x<0 || n!=strlen(targ))
            {
               fprintf(stderr,"%s - x must be an integer greater or equal to zero\n", error_str);
               goto main_clean_exit;
            }
            break;
         case 'y':
            sscanf(targ,"%d%n",&y,&n);
            if(y<0 || n!=strlen(targ))
            {
               fprintf(stderr,"%s - y must be an integer greater or equal to zero\n", error_str);
               goto main_clean_exit;
            }
            break;
         case 'n':
            backlight=0;
            break;
         case 'r':
            reset=1;
            break;
         case 'e':
            cls=1;
            break;
         default:
            fprintf(stderr,"%s - unknow option : %c %s\n", c, error_str, argv[optind]);
            goto main_clean_exit;
            break;
      }
   }

   if(cfgfile==NULL)
   {
      cfgfile=malloc(strlen(default_cfgfile)+1);
      if(cfgfile==NULL)
      {
         fprintf(stderr,"%s - malloc error : ",error_str);
         perror("");
         goto main_clean_exit;
      }
      strcpy(cfgfile,default_cfgfile);
#ifdef DEBUG
      fprintf(stderr,"%s - using default interface configuration file : %s\n",debug_str, cfgfile);
#endif
   }
 
   // load config file
   ret=load_config_file(cfgfile, params);
   if(ret==-1)
   {
      fprintf(stderr,"%s - can't load interface configuration file (%s) : ",error_str, cfgfile);
      perror("");
      goto main_clean_exit;
   }

#ifdef DEBUG  
   // display all parameters found
   for(i=0;i<_LAST_PARAM;i++)
   {
      if(params[i]!=NULL)
         fprintf(stderr, "%s - %d : %s\n", debug_str, i, params[i]);
      else
         fprintf(stderr, "%s - %d : NULL\n", debug_str, i);
   }
#endif
 
   // get interface
   int interface_id = getTokenID(interface_list, params[INTERFACE_ID]);
   if(interface_id<0)
   {
      ret=1;
      fprintf(stderr,"%s - unknown interface : %s\n", error_str, params[INTERFACE_ID]);
      goto main_clean_exit;
   }

 
   // get lcd type
   int lcd_type_id = getTokenID(lcd_type_list, params[LCD_TYPE_ID]);
   if(lcd_type_id<0)
   {
      ret=1;
      fprintf(stderr,"%s - unknown lcd type : %s\n", error_str, params[LCD_TYPE_ID]);
      goto main_clean_exit;
   }

   // build interface context from params
   void *iface_context = NULL;
   
   switch(interface_id)
   {
      case I2C_ID:
         switch(lcd_type_id)
         {
            case HD44780_ID:
               lcd=i2c_hd44780_lcd_alloc(params);
               break;
            default:
               fprintf(stderr,"%s - incompatible interface/lcd_type : %s\n", error_str, params[LCD_TYPE_ID]);
               goto main_clean_exit;
               break;
         }
         break;
         
      default:
         ret=1;
         fprintf(stderr,"%s - unknow interface : %s\n", error_str, params[INTERFACE_ID]);
         goto main_clean_exit;
         break;
   }
 
   if(lcd==NULL)
   {
      goto main_clean_exit;
   }
   
#ifdef DEBUG
   fprintf(stderr,"%s - x=%d y=%d backlight=%d cls=%d reset=%d\n",debug_str,x,y,backlight,cls,reset);
#endif

   // construction de lcd
   if(lcd_init(lcd,reset)<0)
   {
      fprintf(stderr,"%s - can't init device : ",error_str);
      perror("");
      goto main_clean_exit;
   }
   if(cls)
      lcd_clear(lcd);

   lcd_backlight(lcd, backlight);

   lcd_gotoxy(lcd, x, y);

   if (optind < argc) {
      while (optind < argc)
      {
         lcd_print(lcd, argv[optind++]);
         if(optind < argc)
         {
            lcd_print(lcd, " ");
#ifdef DEBUG
            fprintf(stderr," ");
#endif
         }
      }
#ifdef DEBUG
      fprintf(stderr,"\n");
#endif
   }
#ifdef DEBUG
   else
      fprintf(stderr,"%s - nothing to print !!!\n", warning_str);
#endif

main_clean_exit:
   if(lcd)
      lcd_free(&lcd);

   if(cfgfile)
   {
      free(cfgfile);
      cfgfile=NULL;
   }

   for(i=0;i<_LAST_PARAM - _FIRST_PARAM;i++)
   {
      if(params[i]);
      {
         free(params[i]);
         params[i]=NULL;
      }
   }
   return ret;
}
