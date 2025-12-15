#ifndef __GDEVICE__H
#define __GDEVICE__H
#include <stdio.h>
#include <wiringPi.h>

struct gdevice
{
    char dev_name[128]; //设备名称
    int key; //key值，用于匹配控制指令的值
    int gpio_pin; //控制的gpio引脚
    int gpio_mode; //输入输出模式
    int gpio_status; //高低电平状态
    int check_face_status; //是否进行人脸检测状态
    int voice_set_status; //是否设置语音播报
    struct gdevice *next;
};

struct gdevice *find_device_by_key(struct gdevice *pgdevhead, int key);

int set_gpio_gdevice_status(struct gdevice *pgdev);

//struct gdevice *add_device_to_gdevice_list(struct gdevice *pgdevhead, struct gdevice *gdev);




#endif