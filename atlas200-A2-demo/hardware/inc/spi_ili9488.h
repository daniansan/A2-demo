

#ifndef __SPI_ILI9488_H__
#define __SPI_ILI9488_H__



#include "unistd.h"
#include "fcntl.h"
#include "sys/ioctl.h"
#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "linux/spi/spidev.h"
#include "stdarg.h"


//定义LCD的尺寸
#define LCD_W 320
#define LCD_H 480


#define LCD_SPI_DEV			"/dev/spidev7.0"
#define LCD_SPI_MODE 		0x00
#define LCD_SPI_BITS		8
#define LCD_SPI_SPEED		2500000

#define LCD_RS				"gpio81"//GPIO2_17
#define LCD_RST				"gpio82"//GPIO2_18
#define LCD_CS				"gpio226"//GPIO7_2

//POINT_COLOR
#define WHITE            0xFFFFFF
#define BLACK            0x000000
#define BLUE             0x0000FF
#define BRED             0XF81F
#define GRED             0XFFE0
#define GBLUE            0X07FF
#define RED              0xFF0000
//#define MAGENTA          0xF81F
#define GREEN            0x00FF00
#define CYAN             0x7FFF
#define YELLOW           0xFFE0
#define BROWN            0XBC40
#define BRRED            0XFC07
#define GRAY             0X8430
#define GRAY175          0XAD75
#define GRAY151          0X94B2
#define GRAY187          0XBDD7
#define GRAY240          0XF79E

int spi_lcd_init(void);

void lcd_clear(uint32_t color);
void lcd_address_set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void lcd_set_color(uint32_t back, uint32_t fore);

void lcd_draw_point(uint16_t x, uint16_t y);
void lcd_draw_point_color(uint16_t x, uint16_t y, uint32_t color);
void lcd_draw_circle(uint16_t x0, uint16_t y0, uint8_t r);
void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void lcd_fill(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint32_t color);
void lcd_fill_array(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, void *pcolor);

void lcd_show_num(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint32_t size);
uint8_t lcd_show_string(uint16_t x, uint16_t y, uint32_t size, const char *fmt, ...);
uint8_t lcd_show_image(uint16_t x, uint16_t y, uint16_t length, uint16_t wide, const uint8_t *p);

void lcd_enter_sleep(void);
void lcd_exit_sleep(void);
void lcd_display_on(void);
void lcd_display_off(void);

int ili9488_test();

#endif
