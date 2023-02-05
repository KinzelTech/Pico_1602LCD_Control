#include <stdio.h>
#include "pico/stdlib.h"


/**********************************************************************/
/*                             Constants                              */
/**********************************************************************/
#define RS  14	//Set to LOW when sending cmds, HIGH when sending data
#define EN  15	//Set to LOW to ignore incoming data, HIGH to receive
#define DB0 16  //Data Bit 0
#define DB1 17	//Data Bit 1
#define DB2 18  //Data Bit 2
#define DB3 19	//Data Bit 3
#define DB4 20	//Data Bit 4
#define DB5 21	//Data Bit 5
#define DB6 22	//Data Bit 6
#define DB7 26	//Data Bit 7
#define HIGH 1
#define LOW  0

#define CLEAR_DISPLAY 0b00000001
#define RETURN_HOME   0b00000010
#define WRITE_RIGHT   0b00000110
#define WRITE_LEFT    0b00000100
#define SHIFT_RIGHT   0b00000101
#define SHIFT_LEFT    0b00000111

//Display, Cursor, and Cursor Blink must be OR'ed together
#define DISPLAY_ON       0b00001100
#define DISPLAY_OFF      0b00001000
#define CURSOR_ON        0b00001010
#define CURSOR_OFF       0b00001000
#define CURSOR_BLINK_ON  0b00001001
#define CURSOR_BLINK_OFF 0b00001000

//Bit Mode, lines, and font type must be OR'ed together
#define BIT_MODE_8       0b00110000
#define BIT_MODE_4       0b00100000
#define LINES_1          0b00100000
#define LINES_2          0b00101000
#define FONT_5X8         0b00100000
#define FONT_5X11        0b00100100


//Set last 6 bits to the desired CGRAM address
#define SET_CGRAM 0b01000000

//Set the last 7 bits to the desired DDRAM address
#define SET_DDRAM 0b10000000

#define ON  1
#define OFF 0

/**********************************************************************/
/*                         Global Variables                           */
/**********************************************************************/
uint8_t d_c_cb = 0b00001000;
uint8_t b_d_f  = 0b00100000;

/**********************************************************************/
/*                       Function Declarations                        */
/**********************************************************************/
uint32_t init(uint reset_set, uint enable, uint databit0, uint databit1, uint databit2, uint databit3, uint databit4, uint databit5, uint databit6, uint databit7);
   //Initiate the LCD's connections
void pulse_enable ();
   //Pulse the enable pin over 2us
void send_cmd     (uint8_t command);
   //Send a command to the LCD board
void send_data    (uint8_t data);
   //Send data to display to the LCD board
void write_char   (char character);
   //Write a character to the LCD
void write_string (char string[]);
   //Write a string to the LCD
void clear_display();
   //Clear the display, and reset the cursor
void return_home  ();
   //Reset the cursor
void write_right  ();
   //Set the write mode to left-to-right
void line_select  (int line_number);
   //Select the line to write to
void display      (uint8_t status);
   //Turn the display on or off
void cursor       (uint8_t status);
   //Turn the cursor on or off
void cursor_blink (uint8_t status);
   //Turn cursor blink on or off
void bit_mode     (uint8_t number_of_bits);
   //Set LCD to 4 databit mode or 8 databit mode
void display_lines(uint8_t number_of_lines);
   //Set LCD to 1 or 2 lines
void font_type    (uint8_t font_size);
   //Set font type to 5x8 or 5x11
int strlength(char string[]);
   //Return the length of a string

/**********************************************************************/
/*                           Main Function                            */
/**********************************************************************/
int main()
{
   stdio_init_all();
   sleep_ms(5000);
   init(RS, EN, DB0, DB1, DB2, DB3, DB4, DB5, DB6, DB7);
  
   sleep_ms(5000);
   while(true)
   {
      sleep_ms(500);
      send_cmd(0b00011000);

   }


   return 0;
}

uint32_t init(uint reset_set, uint enable, uint databit0, uint databit1, uint databit2, uint databit3, uint databit4, uint databit5, uint databit6, uint databit7)
{
   uint32_t pin_mask = 0x0;
   uint pins[] = {reset_set, enable, databit0, databit1, databit2, databit3, databit4, databit5, databit6, databit7};
   uint8_t num_of_pins = 10;   
   uint8_t counter;
   
   //Create a bitmask representing the LCD's pins
   for(counter = 0; counter < num_of_pins; counter++)
      pin_mask |= 0x1 << pins[counter]; 

   //Set all pins to output and default low
   gpio_init_mask(pin_mask);
   gpio_set_dir_out_masked(pin_mask);
   for(counter = 0; counter < num_of_pins; counter++)
      gpio_pull_up(pins[counter]);

   //Set up default screen configuration
   send_cmd(BIT_MODE_8 | LINES_2 | FONT_5X8);
   send_cmd(DISPLAY_ON | CURSOR_ON | CURSOR_BLINK_ON);
   clear_display();
   write_string("ABCDEFGHIJKLMNOP\nQRSTUVWXYZ");

   //write_right();
   //display(ON);
   //cursor(ON);
   //cursor_blink(ON);
   //bit_mode(8);
   //display_lines(2);
   //font_type(FONT_5X11);
   return pin_mask;
}


//HIDDEN
void pulse_enable()
{
   //Minimum on-off enable time is 1200ns
   //Minimum on-off time with RPI is 2us
   sleep_us(2);
   gpio_put(EN, HIGH);
   sleep_us(2);
   gpio_put(EN, LOW);
   sleep_us(100);   //Minimum cmd settling time is 37us for most cmds.
   return;  
}

//HIDDEN
void send_cmd(uint8_t command)
{
   //RS is default low, and RW is hardwired low.  
   //Only need to set databits
   gpio_put(DB0, command & 0x1);
   gpio_put(DB1, command & 0x2);
   gpio_put(DB2, command & 0x4);
   gpio_put(DB3, command & 0x8);
   gpio_put(DB4, command & 0x10);
   gpio_put(DB5, command & 0x20);
   gpio_put(DB6, command & 0x40);
   gpio_put(DB7, command & 0x80);
   pulse_enable();
   printf("\nSending Command - %d0%d%d%d%d%d%d%d%d", gpio_get(RS),
                                                     gpio_get(DB7), gpio_get(DB6), gpio_get(DB5), gpio_get(DB4),
                                                     gpio_get(DB3), gpio_get(DB2), gpio_get(DB1), gpio_get(DB0));
   return;
}

//HIDDEN
void send_data(uint8_t data)
{
   gpio_put(RS, HIGH); //RS must be high to send data
   send_cmd(data);
   gpio_put(RS, LOW);
   return;
}

void write_char(char character)
{
   send_data((uint8_t) character);  
   return;
}

void write_string(char string[])
{
   uint8_t counter;
   uint8_t string_length = strlength(string);   

   for(counter = 0; counter < string_length; counter++)
   {
      //Jump to line 2 if a newline character is present
      if(string[counter] == '\n')
         line_select(2);
      else
         write_char(string[counter]);
     
   }

   return;
}

/*Below are some highlevel cmds for executing standard commands*/
void clear_display()
{
   send_cmd(CLEAR_DISPLAY);
   sleep_ms(2);
   return;
}

void return_home()
{
   send_cmd(RETURN_HOME);
   return;
}

void write_right()
{
   send_cmd(WRITE_RIGHT);
   return;
}

void line_select(int line_number)
{
   if(line_number == 1)
      send_cmd(SET_DDRAM | 0b00000000);
   else if(line_number == 2)
      send_cmd(SET_DDRAM | 0b01000000);
   return;
}

/*The below three functions must be ORED together*/
void display(uint8_t status)
{
   if(status == OFF)
      d_c_cb &= !0b00000100;
   else
      d_c_cb |=  0b00000100;
   send_cmd(DISPLAY_ON);
   return;
}

void cursor(uint8_t status)
{
   if(status == OFF)
      d_c_cb &= !0b00000010;
   else
      d_c_cb |=  0b00000010;
   send_cmd(d_c_cb);
   return;
}

void cursor_blink(uint8_t status)
{
   if(status == OFF)
      d_c_cb &= !0b00000001;
   else
      d_c_cb |=  0b00000001; 
   send_cmd(d_c_cb);
   return;
}



/*The below three functions must be ORed together*/
void bit_mode(uint8_t number_of_bits)
{
   if(number_of_bits == 4)
      b_d_f &= !0b00010000;
   else if(number_of_bits == 8)
      b_d_f |=  0b00010000;
   send_cmd(b_d_f);
   return;
}

void display_lines(uint8_t number_of_lines)
{
   if(number_of_lines == 1)
      b_d_f &= !0b00001000;
   else if(number_of_lines == 2)
      b_d_f |=  0b00001000;
   send_cmd(b_d_f);
   
   return;
}

void font_type(uint8_t font_size)
{
   if(font_size == FONT_5X8)
      b_d_f &= !0b00000100;
   else if(font_size == FONT_5X11)
      b_d_f |=  0b00000100;
   send_cmd(b_d_f);
   
   return;
}

int strlength(char string[])
{
   int counter = 0;
   while(string[counter] != '\0')
      counter++;
   return counter;
}




