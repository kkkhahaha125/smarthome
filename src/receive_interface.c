#include <pthread.h>
#include <mqueue.h>
#include <wiringPi.h>

#include "control.h"
#include "receive_interface.h"
#include "msg_queue.h"
#include "global.h"
#include "face.h"
#include "myoled.h"
#include "gdevice.h"

#include "ini.h"

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

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

typedef struct {
    long msg_len;
    unsigned char *buffer;
    ctrl_info_t *ctrl_info;
}recv_msg_t;

static struct gdevice *pdevhead = NULL;

static int handler_gdevice(void* user, const char* section, const char* name,
                   const char* value)
{
    struct gdevice *pdev = NULL;

    if(NULL == pdevhead){
        pdevhead = (struct gdevice *)malloc(sizeof(struct gdevice));
        pdevhead->next = NULL;
        memset(pdevhead, 0, sizeof(struct gdevice));
        strcpy(pdevhead->dev_name, section);
    }else if(0 != strcmp(section, pdevhead->dev_name)){
        pdev = (struct gdevice *)malloc(sizeof(struct gdevice));
        memset(pdev, 0, sizeof(struct gdevice));
        strcpy(pdev->dev_name, section);
        pdev->next = pdevhead;
        pdevhead = pdev;
    }

    if(NULL != pdevhead){
        if(MATCH(pdevhead->dev_name, "key")){
            sscanf(value, "%x", &pdevhead->key);
            printf("%d | pdevhead->key = %x\n", __LINE__, pdevhead->key);
        }else if(MATCH(pdevhead->dev_name, "gpio_pin")){
            pdevhead->gpio_pin = atoi(value);
        }else if(MATCH(pdevhead->dev_name, "gpio_mode")){
            if(strcmp(value, "OUTPUT") == 0){
                pdevhead->gpio_mode = OUTPUT;
            }else if(strcmp(value, "INPUT") == 0){
                pdevhead->gpio_mode = INPUT;
            }
        }else if(MATCH(pdevhead->dev_name, "gpio_status")){
            if(strcmp(value, "LOW") == 0){
                pdevhead->gpio_mode = LOW;
            }else if(strcmp(value, "HIGH") == 0){
                pdevhead->gpio_mode = HIGH;
            }
        }else if(MATCH(pdevhead->dev_name, "check_face_status")){
            pdevhead->check_face_status = atoi(value);

        }else if(MATCH(pdevhead->dev_name, "voice_set_status")){
            pdevhead->voice_set_status = atoi(value);

        }  



    }


    return 1;
}


static int receive_init(void)
{

    if (ini_parse("gdevice.ini", handler_gdevice, NULL) < 0) {
        printf("Can't load 'gdevice.ini'\n");
        return 1;
    }

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

static void *handle_device(void *arg)
{
    recv_msg_t *recv_msg = NULL;
    struct gdevice *cur_gdev = NULL;
    char success_or_failed[20] = "success";
    char oled_msg[256] = {0};
    char *change_status = NULL;
    int ret = -1;
    pthread_t tid = -1;
    int smoke_status = 0;
    double face_result = 0.0;

    pthread_detach(pthread_self());
    if(NULL != arg){
        recv_msg = (recv_msg_t *)arg;
        printf("%s|%s|%d,len: %ld,handle: %02X %02X %02X %02X %02X %02X\n", __FILE__, __func__, __LINE__, recv_msg->msg_len, recv_msg->buffer[0], recv_msg->buffer[1], recv_msg->buffer[2], recv_msg->buffer[3], recv_msg->buffer[4], recv_msg->buffer[5]);

    }else{
        fprintf(stderr,"handle error\n");
        pthread_exit(0);
    } 

    //从消息队列取出的消息buffer[2]判断当前处理的是哪个device
    if(NULL != recv_msg->buffer){
        cur_gdev = find_device_by_key(pdevhead, recv_msg->buffer[2]);
    }else{
        fprintf(stderr,"handle buffer error\n");
        pthread_exit(0);
    }

    
    if(NULL != cur_gdev){
        
        //控制GPIO
        //设置要设置的引脚状态
        cur_gdev->gpio_status = recv_msg->buffer[3] == 0? LOW: HIGH;
        
        //特殊处理开锁
        if(1 == cur_gdev->check_face_status){
            face_result = face_category();
            if(face_result > 0.6){
                //设置引脚状态
                ret = set_gpio_gdevice_status(cur_gdev);
                recv_msg->buffer[2] = 0x47;
            }else{

                recv_msg->buffer[2] = 0x46;
                ret = -1;
            }

        }else{

            //设置引脚状态
            ret = set_gpio_gdevice_status(cur_gdev);
        }

    }else{
        if(recv_msg->buffer[2] != 0x40){
                fprintf(stderr,"can't find device in pdevhead\n");
        }
        pthread_exit(0);
    }

    //通过串口给语音模块发送信息
    if(1 == cur_gdev->voice_set_status){
        if(NULL != recv_msg && NULL != recv_msg->ctrl_info && NULL != recv_msg->ctrl_info->ctrl_phead){

            struct control *pcontrol = recv_msg->ctrl_info->ctrl_phead;
            while(NULL != pcontrol){
                if(strstr(pcontrol->control_name, "voice")){
                    if(0x45 == recv_msg->buffer[2] && 0 == recv_msg->buffer[3]){
                        smoke_status = 1; 
                        pthread_create(&tid, NULL, pcontrol->set, (void *)recv_msg->buffer);
                        break;
                    }

                }
                pcontrol = pcontrol->next;
            }

        }

    }


    //oled显示打开/关闭设备信息
    if(-1 == ret){
        memset(success_or_failed, 0, sizeof(success_or_failed));
        strncpy(success_or_failed, "failed", 6);
    }

    change_status = cur_gdev->gpio_status == LOW ? "Open": "Close";
    sprintf(oled_msg, "%s %s %s!", change_status, cur_gdev->dev_name, success_or_failed);
    //特殊处理火灾险情的oled显示
    if(strcmp(cur_gdev->dev_name, "beep") == 0){
        if(smoke_status == 1){
            memset(oled_msg, 0, sizeof(oled_msg));
            strcpy(oled_msg, "A risk of fire!");
        }else{
            memset(oled_msg, 0, sizeof(oled_msg));
            strcpy(oled_msg, "Fire risk disappears");
        }

    }
    myoled_show(oled_msg);

    //特殊处理lock(5s后关闭锁)
    if(1 == cur_gdev->check_face_status && 0 == ret && face_result > 0.6){
        sleep(5);
        cur_gdev->gpio_status = HIGH;
        set_gpio_gdevice_status(cur_gdev);
    }

    if (recv_msg) {
        if (recv_msg->buffer) free(recv_msg->buffer);
        free(recv_msg);
    }

    pthread_exit(0);
    
}

static void *receive_get(void *arg)
{
    ctrl_info_t *ctrl_info = NULL;
    ssize_t readLen = -1;
    pthread_t tid = -1;
    unsigned char *readBuffer = NULL;
    struct mq_attr attr = {0};


    
    if(NULL != arg){
        ctrl_info = (ctrl_info_t *)arg;

    }else{
        printf("receive arg NULL\n");
        pthread_exit(0);
    }   
    
    if(-1 == mq_getattr(ctrl_info->mqd, &attr)){
        perror("mq_getattr\n");
        pthread_exit(0);
    }


    readBuffer = (unsigned char *)malloc(attr.mq_msgsize);
    memset(readBuffer, 0, attr.mq_msgsize);

    while (1)
    {
        readLen = mq_receive(ctrl_info->mqd, readBuffer, attr.mq_msgsize, NULL);
        printf("%s|%s|%d,len: %ld,mq_receive: %02X %02X %02X %02X %02X %02X\n", __FILE__, __func__, __LINE__, readLen, readBuffer[0], readBuffer[1], readBuffer[2], readBuffer[3], readBuffer[4], readBuffer[5]);
        if(-1 == readLen){
            perror("mq_recvive\n");
            pthread_exit(0);
        }else if(readBuffer[0] == 0xAA && readBuffer[1] == 0x55 &&
                 readBuffer[5] == 0xAA && readBuffer[4] == 0x55)
        {
            recv_msg_t *recv_msg = (recv_msg_t *)malloc(sizeof(recv_msg_t));
            recv_msg->ctrl_info = ctrl_info;
            recv_msg->msg_len = readLen;
            recv_msg->buffer = (unsigned char *)malloc(attr.mq_msgsize);
            memcpy(recv_msg->buffer, readBuffer, readLen);
            pthread_create(&tid, NULL, handle_device, (void *)recv_msg);
        }

    }
    

     
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