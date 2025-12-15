
#include "gdevice.h"


struct gdevice *find_device_by_key(struct gdevice *pgdevhead, int key)
{
    struct gdevice *p = NULL;

    if(NULL == pgdevhead){

        return NULL;
    }

    p = pgdevhead;

    while(NULL != p){
        if(p->key == key){
            return p;
        }
        p = p->next;

    }

    return NULL;
}

int set_gpio_gdevice_status(struct gdevice *pgdev)
{

    if(NULL == pgdev){
        return -1;
    }

    if(-1 != pgdev->gpio_pin){
        //设置引脚的输入/输出模式
        pinMode(pgdev->gpio_pin, pgdev->gpio_mode);


        //条件满足表示，设置输出模式的引脚的输出高低电平
        if(OUTPUT == pgdev->gpio_mode){
            digitalWrite(pgdev->gpio_pin, pgdev->gpio_status);
        }


    }

    return 0;
}

/*
struct gdevice *add_device_to_gdevice_list(struct gdevice *pgdevhead, struct gdevice *gdev)
{

    //头插法加入新节点进入链表
    if(NULL == pgdevhead){
        pgdevhead = gdev; 
    }else{
        gdev->next = pgdevhead;
        pgdevhead = gdev;

    }

    return pgdevhead;

}
*/




