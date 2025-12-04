#ifndef __GLOBAL_H
#define __GLOBAL_H

#include "control.h"

typedef struct{
    mqd_t mqd;
    struct control *ctrl_phead;
}ctrl_info_t;


#endif
