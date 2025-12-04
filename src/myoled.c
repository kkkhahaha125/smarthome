#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>

#include "font.h"
#include "oled.h"


#include "myoled.h"


#define FILENAME "/dev/i2c-3"
static struct display_info disp;

int oled_show(void *arg)
{

    unsigned char *buffer = (unsigned char *)arg;
    #if 0
    oled_putstrto(&disp, 0, 9+1, "This garbage is :");
    disp.font = font2;
    switch(buffer[2]){
    
        case 0x41:
            oled_putstrto(&disp, 0, 20, "dry waste");
            break;
        case 0x42:
            oled_putstrto(&disp, 0, 20, "wet waste");
            break;
        case 0x43:
            oled_putstrto(&disp, 0, 20, "recyclable waste");
            break;
        case 0x44:
            oled_putstrto(&disp, 0, 20, "hazardous waste");
            break;
        case 0x45:
            oled_putstrto(&disp, 0, 20, "recognition failed");
            break;

    }
    #endif
    disp.font = font2;

    oled_send_buffer(&disp);

    return 0;

}

int myoled_init(void){

    int e;
    disp.address = OLED_I2C_ADDR;
    disp.font = font2;

    e = oled_open(&disp, FILENAME);
    e = oled_init(&disp);

    return 0;


}










