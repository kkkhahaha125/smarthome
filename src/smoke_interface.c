#include <pthread.h>
#include <wiringPi.h>


#include "smoke_interface.h"
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

#define SMOKE_PIN 6
#define SMOKE_MODE INPUT 

static int smoke_init(void)
{   
    //配置pin6为输入模式
    pinMode(SMOKE_PIN, SMOKE_MODE);
    return 0;
}

static void smoke_final(void)
{

}


static void *smoke_get(void *arg)
{
    int status = HIGH;
    int switch_status = 0;
    mqd_t mqd = -1;
    unsigned char buffer[6] = {0xAA, 0x55, 0x00, 0x00, 0x55, 0xAA};
    
    if(NULL != arg){
        mqd = ((ctrl_info_t *)arg)->mqd;
    }else{
        printf("smoke arg NULL\n");
        pthread_exit(0);
    }
    
    pthread_detach(pthread_self());
  
    while(1)
    {
        status = digitalRead(SMOKE_PIN);
        //检测到烟雾，开启烟雾报警
        if(LOW == status && 0 == switch_status){
            buffer[2] = 0x45;
            buffer[3] = 0x01;
            switch_status == 1;
            if(-1 == send_message(mqd, buffer, 6)){
                switch_status = 0;
            }
        //检测到烟雾消失，解除烟雾报警
        }else if(HIGH == status && 1 == switch_status){
            buffer[2] = 0x45;
            buffer[3] = 0x00;
            switch_status == 0;
            if(-1 == send_message(mqd, buffer, 6)){
                switch_status = 1;
            }

        }
    }
    

    pthread_exit(0);
}

/*
static void *smoke_set(void *arg)
{


}
*/
struct control smoke_control = {
    .control_name = "smoke",  
    .init = smoke_init,
    .final = smoke_final,
    .get = smoke_get,
    .set = NULL,
    .next = NULL
};


struct control *add_smoke_to_ctrl_list(struct control *phead)
{
    //头插法加入新节点进入链表
    if(NULL == phead){
        phead = &smoke_control;
    }else{
        smoke_control.next = phead;
        phead = &smoke_control;

    }

    return phead;

}