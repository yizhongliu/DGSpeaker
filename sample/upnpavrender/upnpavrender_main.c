#include <string.h>
#include <stdio.h>
//#include "cmediarenderer.h"
//#include <cybergarage/upnp/std/av/cprotocolinfo.h>
//#include <cybergarage/upnp/std/av/logging.h>
//#include "output_gstreamer.h"
//#include "logging.h"
//#include "output_gstreamer.h"
//#include <cybergarage/upnp/std/av/cmediarenderer.h>
//#include <gstreamer_0.10/output_gstreamer.h>

#include "upnpavrender.h"
#if 0

CgUpnpAvRenderer *gdmr = NULL;  //add by llm
/*   检查ip线程while循环退出判断变量  */
int checkIpThreadExit = 1; 


CgUpnpAvRenderer * getUpnpAvRender(void)
{
    return gdmr;
}


void* con_dmr_cb (CgUpnpAvRenderer * dmr)
{
    char c;

    c = getchar();
    while( c != '0')
    {
        switch(c)
        {
            case '1':
              printf("stop dmr..%p\n",dmr);
              cg_upnpav_dmr_stop(dmr);
//              cg_upnp_device_byebye(dmr->dev);
            break;
        
            case '2':
              printf("start dmr...%p\n",dmr);
              cg_upnpav_dmr_start(dmr);
            break;
   
            default:
            break;
 
        }
        printf("enter a new char\n");
        c = getchar();
    }
    printf("exit test thread\n");
}

#endif

/****************************************
* main
****************************************/


int main(int argc, char *argv[])
{
    CgUpnpAvRenderer *dmr = NULL;
    char c;

    dmr_init(argc,argv);
    dmr = getUpnpAvRender();
    
    while(1)
    {
        c = getchar();

        switch(c)
        {
            case '1':
              printf("stop dmr..%p\n",dmr);
              cg_upnpav_dmr_stop(dmr);
//              cg_upnp_device_byebye(dmr->dev);
            break;
        
            case '2':
              printf("start dmr...%p\n",dmr);
              cg_upnpav_dmr_start(dmr);
            break;
   
            default:
            break;
 
        }

    }
    return 0;
}
