#ifdef __cplusplus
extern "C"{
#endif

#include "task.h"
#include "unistd.h" 
#include <getopt.h>

#ifdef __cplusplus
}
#endif


static int optIdx=0;
static struct option longOpt[] = {
    {"led",no_argument,0,'l'},
    {"pwm",required_argument,0,'w'},
    {"key",no_argument,0, 'k'},
    {"lcd",no_argument,0,'c'},
    {"oled",no_argument,0,'o'},
    {"gyroscope",no_argument,0,'g'},
    {"rtc",no_argument,0,'r'},
    {"t&h",no_argument,0,'t'},
    {"digital",no_argument,0,'d'},
    {"video",no_argument,0,'v'},
    {"help",no_argument,0,'h'}
};

void GetHelp()
{
    printf("\033[0;33;34m--help         -h    实验内容 \033[m \n");
    printf("\033[0;33;34m--led          -l    Led 控制实验 \033[m \r\n");
    printf("\033[0;33;34m--pwm          -w    呼吸灯实验  \033[m \r\n");
    printf("\033[0;33;34m--key          -k    按键实验   \033[m \r\n");
    printf("\033[0;33;34m--gyroscope    -g    Spi驱动和Adxl345三轴加速度计实验 \033[m \r\n");
    printf("\033[0;33;34m--lcd          -c    Spi驱动和LCD实验 \033[m\r\n");  
    printf("\033[0;33;34m--oled         -o    I2C驱动和Oled屏实验 \033[m \r\n");
    printf("\033[0;33;34m--t&h          -t    I2C驱动和温度传感器实验 \033[m\r\n");
    printf("\033[0;33;34m--digital      -d    I2C驱动和数码管实验 \033[m\r\n");   
    printf("\033[0;33;34m--rtc          -r    I2C驱动和rtc时钟实验 \033[m\r\n"); 
    printf("\033[0;33;34m--video        -v    v4l2摄像头实验 \033[m\r\n");  
}



int main(int argc,char* argv[])
{
    int c;
    c = getopt_long(argc, argv,"lp:kgrtdh",longOpt,&optIdx);

    switch(c)
    {
        case 'l':
        {
            printf("led test\n");
            Led_Test();
        }break;
        case 'p':
        {
            printf("pwm test %s\n",optarg);
            Led_Pwm_Test();
        }break;
        case 'k':
        {
            printf("key test\n");
            Key_Test();
        }break;
        case 'c':
        {
            printf("key test\n");
            Oled_Test();
        }break;
        case 'g':
        {
            printf("key test\n");
            Adxl345_Test();
        }break;
        case 'r':
        {
            printf("key test\n");
            Ds1399u_Test();
        }break;
        case 't':
        {
            printf("key test\n");
            Sht20_Test();
        }break;
        case 'd':
        {
            printf("key test\n");
            Pca9557_Test();
        }break;
        case 'v':
        {
            printf("key test\n");
            v4l2_test();
        }break;
        case 'h':
        {
            GetHelp();
        }break;
        default :{
            GetHelp();
        }
    }
    return 0;
}