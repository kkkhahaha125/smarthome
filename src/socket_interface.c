#include <pthread.h>


#include "socket.h"
#include "socket_interface.h"
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

static int s_fd = -1;
static mqd_t mqd = -1;

static int tcpsocket_init(void)
{
    s_fd = socket_init(IPADDR, PORT);
    if(-1 == s_fd){
    
        return -1;
    }
    return 0;

}

static void tcpsocket_final(void)
{
    if(-1 != s_fd){
        close(s_fd);
        s_fd = -1;
    }

    return ;
}

void *dealWithClient(void *arg){


    int c_fd = *((int *)arg);
    char readBuffer[6] = {0};
    int n_read = -1;

    pthread_detach(pthread_self());
    while(1){
        memset(readBuffer, 0, sizeof(readBuffer));
        n_read = recv(c_fd, readBuffer, sizeof(readBuffer), 0);
        printf("%s|%s|%d,len: %d,recv: %02X %02X %02X %02X %02X %02X\n", __FILE__, __func__, __LINE__, n_read, readBuffer[0], readBuffer[1], readBuffer[2], readBuffer[3], readBuffer[4], readBuffer[5]);
        if(n_read > 0){

            if(readBuffer[0] == 0xAA && readBuffer[1] == 0x55 &&
               readBuffer[5] == 0xAA && readBuffer[4] == 0x55)
            {
                if(-1 == mqd){
                      printf("%s|%s|%d mqd error\n", __FILE__, __func__, __LINE__);
                      break;
                }
                send_message(mqd, (void *)readBuffer, 6);
                
            }

            if(strstr(readBuffer, "close")){
                break;

            }
    
        }else if(n_read == 0 || n_read == -1){
            break;
        }
    }


    printf("client %d byebye!\n", c_fd);   
    close(c_fd); //与这个客户端断开连接

    pthread_exit(0);

}

static void *tcpsocket_get(void *arg)
{

    int c_fd = -1;
    int c_fd_tr = -1;
    int clen = sizeof(struct sockaddr_in);
    pthread_t psocket_tid = -1;
    struct sockaddr_in c_addr = {0};
    int keepalive = 1; // 开启TCP KeepAlive功能
    int keepidle = 5; // tcp_keepalive_time 3s内没收到数据开始发送心跳包
    int keepcnt = 3; // tcp_keepalive_probes 保活探测包的重试次数上限
    int keepintvl = 3; // tcp_keepalive_intvl 每次探测间隔5秒

    

    //从创建线程时传进来的参数arg获取要操作的消息队列mqd
    if(NULL != arg){
        mqd = ((ctrl_info_t *)arg)->mqd;
    }else{
        printf("tcpsocket arg NULL\n");
        pthread_exit(0);
    }


    if(-1 == s_fd){
        if(-1 == tcpsocket_init()){
            printf("tcpsocket_init failed\n");
            pthread_exit(0);
        }

    }

    printf("%s thread start\n", __func__);
    
    while(1){
        c_fd = accept(s_fd, (struct sockaddr *)&c_addr, &clen);
        if(-1 == c_fd){
            perror("accept");
            continue;
        }

        
        // 配置SO_KEEPALIVE（套接字通用层）
        if (setsockopt(c_fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive, sizeof(keepalive)) == -1) {
            perror("setsockopt SO_KEEPALIVE failed");
            // 错误处理（如关闭套接字、退出）
            close(c_fd);
            continue;
        }

        // 配置TCP层保活参数（核心修正：SOL_TCP → IPPROTO_TCP）
        if (setsockopt(c_fd, IPPROTO_TCP, TCP_KEEPIDLE, (void *)&keepidle, sizeof(keepidle)) == -1) {
            perror("setsockopt TCP_KEEPIDLE failed");
        }
        if (setsockopt(c_fd, IPPROTO_TCP, TCP_KEEPCNT, (void *)&keepcnt, sizeof(keepcnt)) == -1) {
            perror("setsockopt TCP_KEEPCNT failed");
        }
        if (setsockopt(c_fd, IPPROTO_TCP, TCP_KEEPINTVL, (void *)&keepintvl, sizeof(keepintvl)) == -1) {
            perror("setsockopt TCP_KEEPINTVL failed");
        }

        printf("Accept a connection from %s:%d\n", inet_ntoa(c_addr.sin_addr), ntohs(c_addr.sin_port));

        c_fd_tr = c_fd;
        pthread_create(&psocket_tid, NULL, dealWithClient, (void *)&c_fd_tr);

     }

     
    pthread_exit(0);
}

/*
static void *tcpsocket_set(void *arg)
{


}
*/
struct control tcpsocket_control = {
    .control_name = "tcpsocket",  
    .init = tcpsocket_init,
    .final = tcpsocket_final,
    .get = tcpsocket_get,
    .set = NULL,
    .next = NULL
};


struct control *add_tcpsocket_to_ctrl_list(struct control *phead)
{
    return add_interface_to_ctrl_list(phead, &tcpsocket_control);
}