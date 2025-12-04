#include "msg_queue.h"

#define QUEUE_NAME "/mq_queue"
#define MESSAGE "mqueue,test,lalala"

mqd_t msg_queue_create(void)
{
    //创建消息队列
	mqd_t mqd = -1;

	struct mq_attr attr;
	attr.mq_flags = 0; //阻塞
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = 256;
	attr.mq_curmsgs = 0;

	mqd = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0666, &attr);
	if( -1 == mqd){
		perror("mq_open");
		return -1;
	}
    printf("%s| %s| %d:mqd=%d\n", __FILE__, __func__, __LINE__, mqd);

    return mqd;


}

void msg_queue_final(mqd_t mqd)
{
	if(-1 != mqd)
		mq_close(mqd);
	mq_unlink(QUEUE_NAME);
	
	return;
}

int  send_message(mqd_t mqd, void *msg, int msg_len)
{
	
    if(-1 == mq_send(mqd, (const char *)msg, msg_len, 0)){

        perror("mqs_send");
        return -1;
    }

	return 0;
}




