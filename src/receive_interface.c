#include <pthread.h>


#include "control.h"
#include "receive_interface.h"
#include "msg_queue.h"
#include "global.h"
#include "face.h"
#include "myoled.h"

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

typedef struct {
    int msg_len;
    unsigned char *buffer;
    ctrl_info_t *cttl_info;
}recv_msg_t;


 
static mqd_t mqd = -1;



static int receive_init(void)
{
    myoled_init();
    face_init();


    return 0;

}

static void receive_final(void)
{
    face_final();
    myoled_final();

    return ;
}


static void *receive_get(void *arg)
{

   

    pthread_detach(pthread_self());


     
    pthread_exit(0);
}

/*
static void *receive_set(void *arg)
{


}
*/
struct control receive_control = {
    .control_name = "receive",  
    .init = receive_init,
    .final = receive_final,
    .get = receive_get,
    .set = NULL,
    .next = NULL
};


struct control *add_receive_to_ctrl_list(struct control *phead)
{
    return add_interface_to_ctrl_list(phead, &receive_control);
}