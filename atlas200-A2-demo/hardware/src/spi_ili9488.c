#include "spi_ili9488.h"
#include "lcd_font.h"

#define LCD_CLEAR_SEND_NUMBER (LCD_H * LCD_W *3)
#define LCD_BUF_SIZE  (LCD_H * LCD_W *3)

static int lcdspifd;
static int lcdrstfd;
static int lcddcfd;
static int lcdcsfd;
static uint32_t BACK_COLOR = BLACK, FORE_COLOR = WHITE;
static uint32_t LCD_WIDTH = LCD_W, LCD_HEIGHT = LCD_H;
static uint32_t LCD_FULL_COLOR = WHITE;

void LCD_DC_SET() // RS_LOW write data
{
    write(lcddcfd,"1",2);
}

void LCD_DC_CLR()  // RS_LOW write reg
{
    write(lcddcfd,"0",2);
}
void LCD_CS_CLR()
{
    write(lcdcsfd,"0",2);
}
void LCD_CS_SET()
{
    write(lcdcsfd,"1",2);
}

void LCD_RESET()
{
    write(lcdrstfd,"0",2);
    usleep(2000);
    write(lcdrstfd,"1",2);
    usleep(2000);
} 


char SpiDevInit()
{
    int ret;
    uint8_t mode = LCD_SPI_MODE;
    uint8_t bits = LCD_SPI_BITS;
    lcdspifd = open(LCD_SPI_DEV,O_RDWR);
    if(lcdspifd<0)
    {
        printf("Open spi dev error\r\n");
        return -1;
    }
    if(ioctl(lcdspifd,SPI_IOC_WR_MODE,&mode)<0)
    {
        printf("Failed to set SPI mode\r\n");
        close(lcdspifd);
        return -1;
    }
    if(ioctl(lcdspifd,SPI_IOC_WR_BITS_PER_WORD,&bits) <0)
    {
        printf("Failed to set SPI bits per word\r\n");
        close(lcdspifd);
        return -1;
    }
    lcddcfd = open("/sys/class/gpio/gpio81/value",O_RDWR);
    if(lcddcfd <0)
    {
        printf("Open gpio81 error\r\n");
        close(lcdspifd);
        return -1;
    }
    lcdrstfd = open("/sys/class/gpio/gpio82/value",O_RDWR);
    if( lcdrstfd<0)
    {
        printf("Open gpio82 error\r\n");
        close(lcdspifd);
        close(lcddcfd);
        return -1;
    }
    lcdcsfd = open("/sys/class/gpio/gpio226/value",O_RDWR);
    if(lcdcsfd< 0)
    {
        printf("Open gpio226 error\r\n");
        return -1;
    }
    printf("SPI set mode :%d\r\n", mode);
    printf("SPI set bits :%d\r\n", bits);
    printf("SPI CD GPIO :%d\r\n", lcddcfd);
    printf("SPI RESET GPIO :%d\r\n", lcdrstfd);
    printf("SPI CS GPIO :%d\r\n", lcdcsfd);
	
    write(lcdrstfd,"1",2);
    write(lcddcfd,"1",2);
    write(lcdcsfd,"1",2);
    usleep(100);

    return 0;
}


static uint8_t lcd_write_cmd(const uint8_t cmd)
{
    uint8_t tx_buffer[2] = {0};
    uint8_t rx_buffer[2] = {0};
    tx_buffer[0] = cmd;
    struct spi_ioc_transfer transfer = {
        .tx_buf = (unsigned long)tx_buffer,
        .rx_buf = (unsigned long)rx_buffer,
        .len = 1,
        .delay_usecs = 0,
        .speed_hz = LCD_SPI_SPEED,  // SPI speed in Hz
        .bits_per_word = LCD_SPI_BITS,
    };
    LCD_CS_CLR();
    LCD_DC_CLR();       // RS_LOW write reg
    if (ioctl(lcdspifd, SPI_IOC_MESSAGE(1), &transfer) < 0) 
    {
        printf("Failed to perform SPI transfer\r\n");
        return -1;
    }
    LCD_CS_SET();
}

static uint8_t lcd_write_data(const uint8_t data)
{
    uint8_t tx_buffer[4] = {0};
    uint8_t rx_buffer[4] = {0};
    tx_buffer[0] = data;
    struct spi_ioc_transfer transfer = {
        .tx_buf = (unsigned long)tx_buffer,
        .rx_buf = (unsigned long)rx_buffer,
        .len = 1,
        .delay_usecs = 0,
        .speed_hz = LCD_SPI_SPEED,  // SPI speed in Hz
        .bits_per_word = LCD_SPI_BITS,
    };
    LCD_CS_CLR();
    LCD_DC_SET();       // RS_High write data
    if (ioctl(lcdspifd, SPI_IOC_MESSAGE(1), &transfer) < 0) 
    {
        printf("Failed to perform SPI transfer\r\n");
        return -1;
    }
    LCD_CS_SET();
}

static uint8_t lcd_write_half_word(const uint16_t da)
{
    uint8_t tx_buffer[4] = {0};
    uint8_t rx_buffer[4] = {0};
    tx_buffer[0] = da>>8;
    tx_buffer[1] = da;
    struct spi_ioc_transfer transfer = {
        .tx_buf = (unsigned long)tx_buffer,
        .rx_buf = (unsigned long)rx_buffer,
        .len = 2,
        .delay_usecs = 0,
        .speed_hz = LCD_SPI_SPEED,  // SPI speed in Hz
        .bits_per_word = LCD_SPI_BITS,
    };
    LCD_CS_CLR();
    LCD_DC_SET();       // RS_High write data
    if (ioctl(lcdspifd, SPI_IOC_MESSAGE(1), &transfer) < 0) 
    {
        printf("Failed to perform SPI transfer\r\n");
        return -1;
    }
    LCD_CS_SET();
}

static uint8_t lcd_write_three_bytes(const uint32_t da)
{
    uint8_t tx_buffer[4] = {0};
    uint8_t rx_buffer[4] = {0};
    tx_buffer[0] = da>>16;
    tx_buffer[1] = da>>8;
    tx_buffer[2] = da;
    struct spi_ioc_transfer transfer = {
        .tx_buf = (unsigned long)tx_buffer,
        .rx_buf = (unsigned long)rx_buffer,
        .len = 3,
        .delay_usecs = 0,
        .speed_hz = LCD_SPI_SPEED,  // SPI speed in Hz
        .bits_per_word = LCD_SPI_BITS,
    };
    LCD_CS_CLR();
    LCD_DC_SET();       // RS_High write data
    if (ioctl(lcdspifd, SPI_IOC_MESSAGE(1), &transfer) < 0) 
    {
        printf("Failed to perform SPI transfer\r\n");
        return -1;
    }
    LCD_CS_SET();
}
static uint8_t lcd_write_bytes(const uint8_t* data, uint32_t len)
{
    uint8_t tx_buf[2048];
    struct spi_ioc_transfer transfer = {0};
    uint16_t i=0;
    LCD_CS_CLR();
    LCD_DC_SET();
    while(len)
    {
        if(!(len/2048))
        {
            memcpy(tx_buf,data,len);
            transfer.tx_buf = (unsigned long)tx_buf;
            transfer.len = len;
            len -= len;
        }
        else
        {
            memcpy(tx_buf,data,2048);
            transfer.tx_buf = (unsigned long)tx_buf;
            transfer.len = 2048;
            len = len-2048;
            data = data+2048;
        }

        transfer.speed_hz = LCD_SPI_SPEED;
        transfer.bits_per_word = LCD_SPI_BITS;
        if (ioctl(lcdspifd, SPI_IOC_MESSAGE(1), &transfer) < 0) 
        {
            printf("Failed to perform SPI transfer  line:%d \r\n",__LINE__);
                return -1;
        }
    }
    LCD_CS_SET();
}


int spi_lcd_init()
{
    SpiDevInit();

    LCD_RESET();

    //************* Start Initial Sequence **********//

    //lcd_write_cmd(0x21);

    lcd_write_cmd(0xE0);
    lcd_write_data(0x00);
    lcd_write_data(0x07);
    lcd_write_data(0x0f);
    lcd_write_data(0x0D);
    lcd_write_data(0x1B);
    lcd_write_data(0x0A);
    lcd_write_data(0x3c);
    lcd_write_data(0x78);
    lcd_write_data(0x4A);
    lcd_write_data(0x07);
    lcd_write_data(0x0E);
    lcd_write_data(0x09);
    lcd_write_data(0x1B);
    lcd_write_data(0x1e);
    lcd_write_data(0x0f);

    lcd_write_cmd(0xE1);
    lcd_write_data(0x00);
    lcd_write_data(0x22);
    lcd_write_data(0x24);
    lcd_write_data(0x06);
    lcd_write_data(0x12);
    lcd_write_data(0x07);
    lcd_write_data(0x36);
    lcd_write_data(0x47);
    lcd_write_data(0x47);
    lcd_write_data(0x06);
    lcd_write_data(0x0a);
    lcd_write_data(0x07);
    lcd_write_data(0x30);
    lcd_write_data(0x37);
    lcd_write_data(0x0f);

    lcd_write_cmd(0xC0);
    lcd_write_data(0x10);
    lcd_write_data(0x10);

    lcd_write_cmd(0xC1);
    lcd_write_data(0x41);

    lcd_write_cmd(0xC5);
    lcd_write_data(0x00);
    lcd_write_data(0x22);
    lcd_write_data(0x80);

    lcd_write_cmd(0x36);
#ifndef LCD_HOR_SCREEN
    lcd_write_data(0x48);
#else
    lcd_write_data(0x28);
#endif

    lcd_write_cmd(0x3A);   //Interface Mode Control
    lcd_write_data(0x77);   // 24bits/pixel 

    lcd_write_cmd(0XB0);   //Interface Mode Control
    lcd_write_data(0x00);
    
    lcd_write_cmd(0xB1);   //Frame rate 70HZ
    lcd_write_data(0xB0);
    lcd_write_data(0x11);
    lcd_write_cmd(0xB4);
    lcd_write_data(0x02);
    lcd_write_cmd(0xB6);   //RGB/MCU Interface Control
    lcd_write_data(0x02);
    lcd_write_data(0x02);

    lcd_write_cmd(0xB7);
    lcd_write_data(0xC6);

    lcd_write_cmd(0xE9);
    lcd_write_data(0x00);

    lcd_write_cmd(0XF7);
    lcd_write_data(0xA9);
    lcd_write_data(0x51);
    lcd_write_data(0x2C);
    lcd_write_data(0x82);

    lcd_fill(0, 0, LCD_WIDTH, LCD_HEIGHT, LCD_FULL_COLOR);

    lcd_write_cmd(0x11); //sleep out
    usleep(1000);
    lcd_write_cmd(0x29); //dispaly on
    usleep(1000);
}

/**
 * Set background color and foreground color
 *
 * @param   back    background color
 * @param   fore    fore color
 *
 * @return  void
 */
void lcd_set_color(uint32_t back, uint32_t fore)
{
    BACK_COLOR = back;
    FORE_COLOR = fore;
}

void lcd_display_on(void)
{
    
}

void lcd_display_off(void)
{
    
}

void lcd_enter_sleep(void)
{
    lcd_write_cmd(0x28);
    lcd_write_cmd(0x10);
}

void lcd_exit_sleep(void)
{
    lcd_write_cmd(0x11);
    lcd_write_cmd(0x29);
}

/**
 * Set drawing area
 *
 * @param   x1      start of x position
 * @param   y1      start of y position
 * @param   x2      end of x position
 * @param   y2      end of y position
 *
 * @return  void
 */
void lcd_address_set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    lcd_write_cmd(0x2a);
    lcd_write_data(x1 >> 8);
    lcd_write_data(x1);
    lcd_write_data(x2 >> 8);
    lcd_write_data(x2);

    lcd_write_cmd(0x2b);
    lcd_write_data(y1 >> 8);
    lcd_write_data(y1);
    lcd_write_data(y2 >> 8);
    lcd_write_data(y2);

    lcd_write_cmd(0x2C);
}

/**
 * clear the lcd.
 *
 * @param   color       Fill color
 *
 * @return  void
 */
void lcd_clear(uint32_t color)
{
    uint32_t i, j;
    uint8_t data[3] = {0};
    uint8_t *buf = NULL;

    data[0] = color >> 16;
    data[1] = color >> 8;
    data[2] = color;
    lcd_address_set(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

    buf = malloc(LCD_BUF_SIZE);
    if (buf)
    {
        /* color is 16 bit */
        for (j = 0; j < LCD_BUF_SIZE / 3; j++)
        {
            buf[j * 3] =  data[0];
            buf[j * 3 + 1] =  data[1];
            buf[j * 3 + 2] =  data[2];
        }
        lcd_write_bytes(buf,LCD_BUF_SIZE);
        free(buf);
    }
    else
    {
        for (i = 0; i < LCD_WIDTH; i++)
        {
            for (j = 0; j < LCD_HEIGHT; j++)
            {
                lcd_write_bytes(data,3);
            }
        }
    }
}

/**
 * display a point on the lcd using the given colour.
 *
 * @param   x       x position
 * @param   y       y position
 * @param   color   color of point
 *
 * @return  void
 */
void lcd_draw_point_color(uint16_t x, uint16_t y, uint32_t color)
{
    lcd_address_set(x, y, x, y);
    lcd_write_three_bytes(color);
}

/**
 * display a point on the lcd.
 *
 * @param   x   x position
 * @param   y   y position
 *
 * @return  void
 */
void lcd_draw_point(uint16_t x, uint16_t y)
{
    lcd_draw_point_color(x, y, FORE_COLOR);
}

/**
 * full color on the lcd.
 *
 * @param   x_start     start of x position
 * @param   y_start     start of y position
 * @param   x_end       end of x position
 * @param   y_end       end of y position
 * @param   color       Fill color
 *
 * @return  void
 */
void lcd_fill(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint32_t color)
{
    uint32_t i = 0, j = 0;
    uint32_t size = 0, size_remain = 0;
    uint8_t *fill_buf = NULL;

    size = (x_end - x_start) * (y_end - y_start) * 3;

    if (size > LCD_CLEAR_SEND_NUMBER)
    {
        /* the number of remaining to be filled */
        size_remain = size - LCD_CLEAR_SEND_NUMBER;
        size = LCD_CLEAR_SEND_NUMBER;
    }

    lcd_address_set(x_start, y_start, x_end, y_end);

    fill_buf = (uint8_t *)malloc(LCD_CLEAR_SEND_NUMBER);
    if (fill_buf)
    {
        /* fast fill */
        while (1)
        {
            for (i = 0; i < size / 3; i++)
            {
                fill_buf[3 * i] = color >> 16;
                fill_buf[3 * i + 1] = color >> 8;
                fill_buf[3 * i + 2] = color;
            }
            lcd_write_bytes(fill_buf,size);

            /* Fill completed */
            if (size_remain == 0)
                break;

            /* calculate the number of fill next time */
            if (size_remain > LCD_CLEAR_SEND_NUMBER)
            {
                size_remain = size_remain - LCD_CLEAR_SEND_NUMBER;
            }
            else
            {
                size = size_remain;
                size_remain = 0;
            }

        }
        free(fill_buf);
    }
    else
    {
        for (i = y_start; i <= y_end; i++)
        {
            for (j = x_start; j <= x_end; j++)
                lcd_write_three_bytes(color);
        }
    }
}

/**
 * full color array on the lcd.
 *
 * @param   x_start     start of x position
 * @param   y_start     start of y position
 * @param   x_end       end of x position
 * @param   y_end       end of y position
 * @param   pcolor      Fill color array's pointer
 *
 * @return  void
 */
void lcd_fill_array(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, void *pcolor)
{
    uint32_t size = 0;
    uint8_t *array = NULL;

    size = (x_end - x_start + 1) * (y_end - y_start + 1) * 3 /* 24bit */;
    array = (uint8_t *)malloc(size);

    if (!array) {
        return ;
    }

    uint32_t *color_p = (uint32_t *)pcolor;

    for (uint16_t i = 0; i < size / 3; i++)
    {
        array[3 * i] = *color_p >> 16;
        array[3 * i + 1] = *color_p >> 8;
        array[3 * i + 2] = *color_p;
        color_p++;
    }

    lcd_address_set(x_start, y_start, x_end, y_end);
    lcd_write_bytes(array,size);

    free(array);
}

/**
 * display a line on the lcd.
 *
 * @param   x1      x1 position
 * @param   y1      y1 position
 * @param   x2      x2 position
 * @param   y2      y2 position
 *
 * @return  void
 */
void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint16_t t;
    uint32_t i = 0;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, row, col;

    if (y1 == y2)
    {
        /* fast draw transverse line */
        lcd_address_set(x1, y1, x2, y2);

        uint8_t line_buf[1440] = {0};

        for (i = 0; i < x2 - x1; i++)
        {
            line_buf[3 * i] = FORE_COLOR >> 16;
            line_buf[3 * i + 1] = FORE_COLOR >> 8;
            line_buf[3 * i + 2] = FORE_COLOR;
        }

        lcd_write_bytes(line_buf,(x2 - x1) * 3);

        return ;
    }

    delta_x = x2 - x1;
    delta_y = y2 - y1;
    row = x1;
    col = y1;
    if (delta_x > 0)incx = 1;
    else if (delta_x == 0)incx = 0;
    else
    {
        incx = -1;
        delta_x = -delta_x;
    }
    if (delta_y > 0)incy = 1;
    else if (delta_y == 0)incy = 0;
    else
    {
        incy = -1;
        delta_y = -delta_y;
    }
    if (delta_x > delta_y)distance = delta_x;
    else distance = delta_y;
    for (t = 0; t <= distance + 1; t++)
    {
        lcd_draw_point(row, col);
        xerr += delta_x ;
        yerr += delta_y ;
        if (xerr > distance)
        {
            xerr -= distance;
            row += incx;
        }
        if (yerr > distance)
        {
            yerr -= distance;
            col += incy;
        }
    }
}

/**
 * display a rectangle on the lcd.
 *
 * @param   x1      x1 position
 * @param   y1      y1 position
 * @param   x2      x2 position
 * @param   y2      y2 position
 *
 * @return  void
 */
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    lcd_draw_line(x1, y1, x2, y1);
    lcd_draw_line(x1, y1, x1, y2);
    lcd_draw_line(x1, y2, x2, y2);
    lcd_draw_line(x2, y1, x2, y2);
}


/**
 * display a circle on the lcd.
 *
 * @param   x       x position of Center
 * @param   y       y position of Center
 * @param   r       radius
 *
 * @return  void
 */
void lcd_draw_circle(uint16_t x0, uint16_t y0, uint8_t r)
{
    int a, b;
    int di;
    a = 0;
    b = r;
    di = 3 - (r << 1);
    while (a <= b)
    {
        lcd_draw_point(x0 - b, y0 - a);
        lcd_draw_point(x0 + b, y0 - a);
        lcd_draw_point(x0 - a, y0 + b);
        lcd_draw_point(x0 - b, y0 - a);
        lcd_draw_point(x0 - a, y0 - b);
        lcd_draw_point(x0 + b, y0 + a);
        lcd_draw_point(x0 + a, y0 - b);
        lcd_draw_point(x0 + a, y0 + b);
        lcd_draw_point(x0 - b, y0 + a);
        a++;
        //Bresenham
        if (di < 0)di += 4 * a + 6;
        else
        {
            di += 10 + 4 * (a - b);
            b--;
        }
        lcd_draw_point(x0 + a, y0 + b);
    }
}

static void lcd_show_char(uint16_t x, uint16_t y, uint8_t data, uint32_t size)
{
    uint8_t temp;
    uint8_t num = 0;;
    uint8_t pos, t;
    uint32_t colortemp = FORE_COLOR;
    uint8_t *font_buf = NULL;

    if (x > LCD_WIDTH - size / 2 || y > LCD_HEIGHT - size)return;

    data = data - ' ';
#ifdef ASC2_1608
    if (size == 16)
    {
        lcd_address_set(x, y, x + size / 2 - 1, y + size - 1);//(x,y,x+8-1,y+16-1)

        font_buf = (uint8_t *)malloc(size * size / 2 * 3);
        if (!font_buf)
        {
            /* fast show char */
            for (pos = 0; pos < size * (size / 2) / 8; pos++)
            {
                temp = asc2_1608[(uint16_t)data * size * (size / 2) / 8 + pos];
                for (t = 0; t < 8; t++)
                {
                    if (temp & 0x80)colortemp = FORE_COLOR;
                    else colortemp = BACK_COLOR;
                    lcd_write_three_bytes(colortemp);
                    temp <<= 1;
                }
            }
        }
        else
        {
            for (pos = 0; pos < size * (size / 2) / 8; pos++)
            {
                temp = asc2_1608[(uint16_t)data * size * (size / 2) / 8 + pos];
                for (t = 0; t < 8; t++)
                {
                    if (temp & 0x80)colortemp = FORE_COLOR;
                    else colortemp = BACK_COLOR;
                    font_buf[3 * (8 * pos + t)] = colortemp >> 16;
                    font_buf[3 * (8 * pos + t) + 1] = colortemp >> 8;
                    font_buf[3 * (8 * pos + t) + 2] = colortemp;
                    temp <<= 1;
                }
            }
            lcd_write_bytes(font_buf,size * size / 2 * 3);
            free(font_buf);
        }
    }
    else
#endif

#ifdef ASC2_2412
        if (size == 24)
        {
            lcd_address_set(x, y, x + size / 2 - 1, y + size - 1);

            font_buf = (uint8_t *)malloc(size * size / 2 * 3);
            if (!font_buf)
            {
                /* fast show char */
                for (pos = 0; pos < (size * 16) / 8; pos++)
                {
                    temp = asc2_2412[(uint16_t)data * (size * 16) / 8 + pos];
                    if (pos % 2 == 0)
                    {
                        num = 8;
                    }
                    else
                    {
                        num = 4;
                    }

                    for (t = 0; t < num; t++)
                    {
                        if (temp & 0x80)colortemp = FORE_COLOR;
                        else colortemp = BACK_COLOR;
                        lcd_write_three_bytes(colortemp);
                        temp <<= 1;
                    }
                }
            }
            else
            {
                for (pos = 0; pos < (size * 16) / 8; pos++)
                {
                    temp = asc2_2412[(uint16_t)data * (size * 16) / 8 + pos];
                    if (pos % 2 == 0)
                    {
                        num = 8;
                    }
                    else
                    {
                        num = 4;
                    }

                    for (t = 0; t < num; t++)
                    {
                        if (temp & 0x80)colortemp = FORE_COLOR;
                        else colortemp = BACK_COLOR;
                        if (num == 8)
                        {
                            font_buf[3 * (12 * (pos / 2) + t)] = colortemp >> 16;
                            font_buf[3 * (12 * (pos / 2) + t) + 1] = colortemp >> 8;
                            font_buf[3 * (12 * (pos / 2) + t) + 2] = colortemp;
                        }
                        else
                        {
                            font_buf[3 * (8 + 12 * (pos / 2) + t)] = colortemp >> 16;
                            font_buf[3 * (8 + 12 * (pos / 2) + t) + 1] = colortemp >> 8;
                            font_buf[3 * (8 + 12 * (pos / 2) + t) + 2] = colortemp;
                        }
                        temp <<= 1;
                    }
                }
                lcd_write_bytes(font_buf, size * size / 2 * 3);
                free(font_buf);
            }
        }
        else
#endif

#ifdef ASC2_3216
            if (size == 32)
            {
                lcd_address_set(x, y, x + size / 2 - 1, y + size - 1);

                font_buf = (uint8_t *)malloc(size * size / 2 * 3);
                if (!font_buf)
                {
                    /* fast show char */
                    for (pos = 0; pos < size * (size / 2) / 8; pos++)
                    {
                        temp = asc2_3216[(uint16_t)data * size * (size / 2) / 8 + pos];
                        for (t = 0; t < 8; t++)
                        {
                            if (temp & 0x80)colortemp = FORE_COLOR;
                            else colortemp = BACK_COLOR;
                            lcd_write_three_bytes(colortemp);
                            temp <<= 1;
                        }
                    }
                }
                else
                {
                    for (pos = 0; pos < size * (size / 2) / 8; pos++)
                    {
                        temp = asc2_3216[(uint16_t)data * size * (size / 2) / 8 + pos];
                        for (t = 0; t < 8; t++)
                        {
                            if (temp & 0x80)colortemp = FORE_COLOR;
                            else colortemp = BACK_COLOR;
                            font_buf[3 * (8 * pos + t)] = colortemp >> 16;
                            font_buf[3 * (8 * pos + t) + 1] = colortemp >> 8;
                            font_buf[3 * (8 * pos + t) + 2] = colortemp;
                            temp <<= 1;
                        }
                    }
                    lcd_write_bytes(font_buf, size * size / 2 * 3);
                    free(font_buf);
                }
            }
            else
#endif
            {
            }
}

/**
 * display the number on the lcd.
 *
 * @param   x       x position
 * @param   y       y position
 * @param   num     number
 * @param   len     length of number
 * @param   size    size of font
 *
 * @return  void
 */
void lcd_show_num(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint32_t size)
{
    lcd_show_string(x, y, size, "%d", num);
}

/**
 * display the string on the lcd.
 *
 * @param   x       x position
 * @param   y       y position
 * @param   size    size of font
 * @param   p       the string to be display
 *
 * @return   0: display success
 *          -1: size of font is not support
 */
uint8_t lcd_show_string(uint16_t x, uint16_t y, uint32_t size, const char *fmt, ...)
{
#define LCD_STRING_BUF_LEN 128

    va_list args;
    uint8_t buf[LCD_STRING_BUF_LEN] = {0};
    uint8_t *p = NULL;

    if (size != 16 && size != 24 && size != 32)
    {
        return -1;
    }

    va_start(args, fmt);
    vsnprintf((char *)buf, 100, (const char *)fmt, args);
    va_end(args);

    p = buf;
    while (*p != '\0')
    {
        if (x > LCD_WIDTH - size / 2)
        {
            x = 0;
            y += size;
        }
        if (y > LCD_HEIGHT - size)
        {
            y = x = 0;
            lcd_clear(RED);
        }
        lcd_show_char(x, y, *p, size);
        x += size / 2;
        p++;
    }

    return 0;
}


/**
 * display the image on the lcd.
 *
 * @param   x       x position
 * @param   y       y position
 * @param   length  length of image
 * @param   wide    wide of image
 * @param   p       image
 *
 * @return   0: display success
 *          -1: the image is too large
 */
uint8_t lcd_show_image(uint16_t x, uint16_t y, uint16_t wide, uint16_t length, const uint8_t *p)
{
    if (x + wide > LCD_WIDTH || y + length > LCD_HEIGHT)
    {
        return -1;
    }

    lcd_address_set(x, y, x + wide - 1, y + length - 1);

    lcd_write_bytes(p,  length * wide * 3);

    return 0;
}


int ili9488_test()
{
    spi_lcd_init();

    lcd_clear(BLACK);


    // lcd_set_color(WHITE, BLACK);

    lcd_show_string(10, 69, 16, "Hello, RT-Thread!");
    lcd_show_string(10, 69+16, 24, "RT-Thread");
    lcd_show_string(10, 69+16+24, 32, "RT-Thread");

    lcd_draw_line(0, 69+16+24+32, LCD_H, 69+16+24+32);
    lcd_draw_rectangle(20,80,120,200);

    lcd_draw_point(160, 310);

    lcd_fill(20, 200, 340, 480, RED);

    
    for (int i = 0; i < 150; i += 4)
    {
        lcd_draw_circle(160, 310, i);
    }

    usleep(2000);
    printf("LCD test end\n");
    return 0;
}
