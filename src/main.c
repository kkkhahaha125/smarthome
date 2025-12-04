#include <stdio.h>
#include <pthread.h>
#include <wiringPi.h>
#include "voice_interface.h"
#include "socket_interface.h"
#include "msg_queue.h"
#include "control.h"
#include "global.h"


int main(int argc, char **argv)
{
    struct control *control_phead = NULL;
    struct control *control_pointer = NULL;
    ctrl_info_t *ctrl_info = NULL;
    pthread_t *tid  = NULL;
    int node_num = 0;
    int i;

    ctrl_info = (ctrl_info_t *)malloc(sizeof(ctrl_info_t));
    if(NULL == ctrl_info){
        perror("ctrl_info malloc");
        return -1;
    }
    ctrl_info->mqd = -1;
    ctrl_info->ctrl_phead = NULL;

    //初始化
    if (-1 == wiringPiSetup()) { 
        printf("wiringPi初始化失败！\n");
        return -1;
    }

    //创建好消息队列
    ctrl_info->mqd = msg_queue_create();

    if(-1 == ctrl_info->mqd){
        return -1;
    }

    ctrl_info->ctrl_phead = add_voice_to_ctrl_list(ctrl_info->ctrl_phead);
    ctrl_info->ctrl_phead = add_tcpsocket_to_ctrl_list(ctrl_info->ctrl_phead);
//  ctrl_info->ctrl_phead = add_fire_to_ctrl_list(ctrl_info->ctrl_phead);

    control_pointer = ctrl_info->ctrl_phead;
    while(NULL != control_pointer){
        if(NULL != control_pointer->init){
             control_pointer->init();
        }     
        control_pointer = control_pointer->next;
        node_num++;
    }

    tid = malloc(sizeof(pthread_t) * node_num);
    if(NULL == tid){
        perror("tid malloc");
        return -1;
    }

    control_pointer = ctrl_info->ctrl_phead;
    for(i = 0; i < node_num; i++){
        if(NULL != control_pointer->get)
            pthread_create(&tid[i], NULL, (void *)control_pointer->get, (void *)ctrl_info);
        control_pointer = control_pointer->next;
    }
    while(1){}
    printf("1\n");
    for(i = 0; i < node_num; i++){
        pthread_join(tid[i], NULL);
    }
    printf("2\n");
    control_pointer = ctrl_info->ctrl_phead;
    for(i = 0; i < node_num; i++){
        if(NULL != control_pointer->final){
             control_pointer->final();
        }     
        control_pointer = control_pointer->next;
    }
    printf("3\n");
    msg_queue_final(ctrl_info->mqd);
    printf("4\n");

    if(NULL != tid){
        free(tid);
    }
    
    if(NULL != tid){
        free(ctrl_info);    
    }


    return 0;
}

