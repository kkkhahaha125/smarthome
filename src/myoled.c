#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>

#include "font.h"
#include "oled.h"


#include "myoled.h"


#define FILENAME "/dev/i2c-3"

static struct display_info disp;  // 静态结构体，生命周期与程序一致


int myoled_show(void *arg)
{

    unsigned char *buffer = (unsigned char *)arg;
    oled_clear(&disp);
    if(NULL != buffer){
        oled_putstrto(&disp, 0, 9+1, buffer);
    }
    #if 0
    oled_putstrto(&disp, 0, 9+1, "This garbage is :");
    disp.font = font2;
    switch(buffer[2]){
    
        case 0x41:
            oled_putstrto(&disp, 0, 20, "dry waste");
            break;
        case 0x42:
            oled_putstrto(&disp, 0, 20, "wet waste");
            break;
        case 0x43:
            oled_putstrto(&disp, 0, 20, "recyclable waste");
            break;
        case 0x44:
            oled_putstrto(&disp, 0, 20, "hazardous waste");
            break;
        case 0x45:
            oled_putstrto(&disp, 0, 20, "recognition failed");
            break;

    }
    #endif
    disp.font = font2;

    oled_send_buffer(&disp);

    return 0;

}




int myoled_init(void) {
    int ret;
    // 1. 初始化结构体核心字段（显式初始化，避免脏数据）
    memset(&disp, 0, sizeof(struct display_info)); // 清空缓冲区、file 等字段
    disp.address = OLED_I2C_ADDR;                  // I2C 地址 0x3c
    disp.font = font2;                             // 绑定字体

    // 2. 打开 I2C 设备，成功把文件描述符赋值给disp->file,失败则直接返回并打印日志
    ret = oled_open(&disp, FILENAME);
    if (ret < 0) {
        fprintf(stderr, "[OLED ERROR] oled_open failed! filename: %s, err: %d\n", 
                FILENAME, ret);
        return OLED_INIT_ERR_OPEN; // 明确返回“打开失败”
    }
    printf("[OLED INFO] oled_open success, fd: %d\n", disp.file); // 调试日志

    // 3. 初始化 OLED 硬件，失败则清理资源（关闭已打开的 I2C 设备）
    ret = oled_init(&disp);
    if (ret != 0) { // 第三方库返回 666 或其他非 0 值均判定为失败
        fprintf(stderr, "[OLED ERROR] oled_init failed! ret: %d\n", ret);
        oled_close(&disp); // 关键：打开成功但初始化失败，需关闭 I2C 设备释放资源
        return OLED_INIT_ERR_SEND_CMD; // 明确返回“初始化指令发送失败”
    }
    printf("[OLED INFO] oled_init success\n");
    
    oled_clear(&disp);

    // 4. 初始化成功，返回标识
    return OLED_INIT_SUCCESS;
}

int myoled_final(void)
{
    int ret = -1;
    //oled_close会关闭文件描述符disp->file, file是int类型
    ret = oled_close(&disp);
    return ret;
}










