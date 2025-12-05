#include <pthread.h>
#include "voice_interface.h"
#include "uartTool.h"
#include "msg_queue.h"
#include "global.h"

#if 0
struct control
{
    char control_name[128]; //监听模块名称
    int (*init)(void); //初始化函数
    void (*final)(void);//结束释放函数
    void *(*get)(void *arg);//监听函数，如语音监听
    void *(*set)(void *arg); //设置函数，如语音播报
    struct control *next;
};

#endif

static int serial_fd = -1;

static int voice_init(void)
{
    serial_fd = serialOpen(SERIAL_DEV, BAUD);


    return serial_fd;
}

static void voice_final(void)
{

    if(-1 != serial_fd){
        close(serial_fd);
        serial_fd = -1;
    }


}
//接收语言指令（语音线程就会执行这个函数）
static void *voice_get(void *arg)
{

    unsigned char buffer[6] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0}; 
    int len = 0;
    mqd_t mqd = -1;
    int i;

    pthread_detach(pthread_self());
    
    if(-1 == serial_fd){
        voice_init();
        if(-1 == serial_fd){
            printf("%s|%s|%d: open serial failed\n", __FILE__, __func__, __LINE__);
            pthread_exit(0);
        }
    }

    //从创建线程时传进来的参数arg获取要操作的消息队列mqd
    if(NULL != arg){
        mqd = ((ctrl_info_t *)arg)->mqd;
    }else{
        printf("voice arg NULL\n");
        pthread_exit(0);
    }

    while(1){

        //从串口获取语音模块发来的信息
        len = serialGetString(serial_fd, buffer);
        for (i = 0; i < 6; i++) {
            printf("%02X ", buffer[i]);
        }
        printf("\n"); 

        if(len > 0){

            if(buffer[0] == 0xAA && buffer[1] == 0x55 &&
               buffer[5] == 0xAA && buffer[4] == 0x55)
            {
                send_message(mqd, (void *)buffer, 6);
                
            }
            memset(buffer,0,sizeof(buffer));
        }



    }


    
    pthread_exit(0);
}

//语音播报
static void *voice_set(void *arg)
{
    unsigned char *buffer = (unsigned char *)arg;

    if(-1 == serial_fd){
        voice_init();
        if(-1 == serial_fd){
            printf("%s|%s|%d: open serial failed\n", __FILE__, __func__, __LINE__);
            pthread_exit(0);
        }
    }

    if(NULL != buffer){
        serialPutString(serial_fd, buffer, 0);
    }

    return;
    
}
struct control voice_control = {
    .control_name = "voice",  
    .init = voice_init,
    .final = voice_final,
    .get = voice_get,
    .set = voice_set,
    .next = NULL
};


struct control *add_voice_to_ctrl_list(struct control *phead)
{
    return add_interface_to_ctrl_list(phead, &voice_control);
}
