#ifndef __MYOLED__H
#define __MYOLED__H

typedef enum {
    OLED_INIT_SUCCESS = 0,        // 成功
    OLED_INIT_ERR_OPEN = -1,      // oled_open 失败
    OLED_INIT_ERR_SEND_CMD = -2,  // oled_init 发送指令失败
    OLED_INIT_ERR_UNKNOWN = -3    // 未知错误
} oled_init_err_t;


int myoled_init(void);
int myoled_show(void*);
int myoled_final(void);

#endif

