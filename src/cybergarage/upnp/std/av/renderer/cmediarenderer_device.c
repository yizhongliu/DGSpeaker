/************************************************************
*
*   CyberLink for C
*
*   Copyright (C) Satoshi Konno 2005
*
*   File: cmediarenderer_device.c
*
*   Revision:
*       2009/06/11
*        - first release.
*       2013/03/18 Liu Liming
*        - add function cg_upnpav_dmr_get_macaddr to get the
*          first avarible macaddr
*
************************************************************/

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <cybergarage/upnp/std/av/cmediarenderer.h>

#include <netdb.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/ioctl.h>
#include <net/if.h>

#include <sys/stat.h>



/****************************************
* Device Description
****************************************/


/****************************************
* Functions
****************************************/

BOOL cg_upnpav_dmr_conmgr_init(CgUpnpAvRenderer *dmr);
BOOL cg_upnpav_dmr_avtransport_init(CgUpnpAvRenderer *dmr);
BOOL cg_upnpav_dmr_renderingctrl_init(CgUpnpAvRenderer *dmr);

BOOL cg_upnpav_dmr_conmgr_actionreceived(CgUpnpAction *action);
BOOL cg_upnpav_dmr_avtransport_actionreceived(CgUpnpAction *action);
BOOL cg_upnpav_dmr_renderingctrl_actionreceived(CgUpnpAction *action);

BOOL cg_upnpav_dmr_conmgr_queryreceived(CgUpnpStateVariable *statVar);
BOOL cg_upnpav_dmr_avtransport_queryreceived(CgUpnpStateVariable *statVar);
BOOL cg_upnpav_dmr_renderingctrl_queryreceived(CgUpnpStateVariable *statVar);


long file_getlength(char *fileName)
{
    struct stat buf;

    if(stat(fileName, &buf ) == -1)
    {
        return -1;
    }

    return buf.st_size;
}

BOOL file_load(char **content,char *fileName)
{
    FILE *fp;
    size_t nRead;
    size_t readCnt;
    long fileLen;
    

    fileLen = file_getlength(fileName);
    if (fileLen <= 0)
    {
        return FALSE;
    }

    *content = (char *)malloc(fileLen + 1);
    memset(*content ,0, fileLen + 1);
    if ( NULL == *content )
    {
        fprintf(stderr, "Memory allocation failure !\n");
        return FALSE;
    }

    fp = fopen(fileName, "r");
    if (fp == NULL) 
    {
        free(*content);
        (*content) = NULL;
        printf("file open error......\n");
        
        return FALSE;
    }

    readCnt = 0;
    
    do {
        int remaining  = fileLen - readCnt;
        int chunkSize = remaining < FILE_READ_CHUNK_SIZE ? remaining : FILE_READ_CHUNK_SIZE;
        nRead = fread(*content+readCnt, sizeof(char), chunkSize, fp);
        readCnt += nRead;
    } while (0 < nRead);

    fclose(fp);
    return TRUE;
}

BOOL cg_upnp_device_loaddescriptionfile(CgUpnpDevice *dev, char *fileName)
{
    char *description;
    int descriptionLen;
    BOOL parseSuccess;

    if(NULL == dev)
    {
        fprintf(stderr, "dev is NULL !\n");
        return FALSE;
    }
   
    printf("filename =%s  \n", fileName); 
    if(!file_load(&description,fileName))
    {
        fprintf(stderr, "description load  failure !\n");
        return FALSE;
    }
    
    if(NULL == description)
    {
        return FALSE;
    }
    
    descriptionLen = strlen(description);
    
    parseSuccess = cg_upnp_device_parsedescription(dev, description, descriptionLen);   
    if(!parseSuccess)
    {
        fprintf(stderr, "description parse  failure !\n");
        free(description);
        return FALSE;
    }
    
    free(description);

    return TRUE;
}


BOOL cg_upnp_service_loaddescriptionfile(CgUpnpService *service, char *fileName)
{
    char *description;
    int descriptionLen;
    BOOL parseSuccess;

    if(NULL == service)
    {
        fprintf(stderr, "service is NULL !\n");
    }
    
     if(!file_load(&description,fileName))
     {
        fprintf(stderr, "description load  failure !\n");
        return FALSE;
     }
    if(NULL == description)
    {
        return FALSE;
    }

    descriptionLen = strlen(description);

    parseSuccess = cg_upnp_service_parsedescription(service, description, descriptionLen);
    if(!parseSuccess)
    {
        fprintf(stderr, "description parse  failure !\n");
    }

    free(description);

    return parseSuccess;
}
#if 0
/****************************************
 * cg_upnpav_dmr_dev_sudprecieved
 ****************************************/
BOOL cg_upnpav_dmr_dev_sudprecieved(CgUpnpSUDPPacket *sudpPkt)  //add by llm 20130912
{
    char *session;
    CgUpnpAvRenderer *dmr;
    CgUpnpDevice *dev;
    

    printf("enter sudprecieved.............\n");

    dev = (CgUpnpDevice *)cg_upnp_sudp_packet_getuserdata(sudpPkt);
    if (!dev)
        return FALSE;

    dmr = (CgUpnpAvRenderer *)cg_upnp_device_getuserdata(dev);
    if (!dmr)
        return FALSE;


    /*filter session*/
    session = cg_upnp_sudp_packet_getsession(sudpPkt);
    printf("session = %s\n", session);
    if(session == NULL)
    {
        printf("sudp session contain nothing\n");       
    }

    printf("var = %s\n",cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_UDPSESSION));
    if(!cg_streq(session, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_UDPSESSION)))
    {
        printf("session is not the current one\n");
        return FALSE;
    }

    
    /*handle sudp msg*/ 
    if (cg_strstr(cg_string_getvalue(sudpPkt->dgmPkt->data), CG_UPNP_SUDP_PLAY) >= 0)
    {
        printf("enter play.............\n");
        return cg_upnpav_dmr_sudp_play_receive(sudpPkt);
    }

    if (cg_strstr(cg_string_getvalue(sudpPkt->dgmPkt->data), CG_UPNP_SUDP_SEEK) >= 0)
    {    
        return cg_upnpav_dmr_sudp_seek_receive(sudpPkt);
    }

    if (cg_strstr(cg_string_getvalue(sudpPkt->dgmPkt->data), CG_UPNP_SUDP_PAUSE) >= 0)
    {    
        return cg_upnpav_dmr_sudp_pause_receive(sudpPkt);
    }
    
    if (cg_strstr(cg_string_getvalue(sudpPkt->dgmPkt->data), CG_UPNP_SUDP_TEST) >= 0)
    {    
        return cg_upnpav_dmr_sudp_test_receive(sudpPkt);
    }
}

#endif


/****************************************
 * cg_upnpav_dmr_get_macaddr
 ****************************************/
static const char *PATH_PROC_NET_DEV = "/proc/net/dev";
BOOL cg_upnpav_dmr_get_macaddr(char *mac)
{
    FILE *fd;
    int s;
    char buffer[256+1];
    char *ifname;
    char *sep;
    struct ifreq req;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0)
            return FALSE;

    fd = fopen(PATH_PROC_NET_DEV, "r");
    fgets(buffer, sizeof(buffer)-1, fd); //eat title line   
    fgets(buffer, sizeof(buffer)-1, fd);
    while (!feof(fd))
    {
        ifname = buffer;
        sep;
        if (fgets(buffer, sizeof(buffer)-1, fd) == NULL)
            break;
        sep = strrchr(buffer, ':');
        if (sep)
            *sep = 0;
        while (*ifname == ' ')
            ifname++;
    
        strcpy(req.ifr_name, ifname); 
    
        if (ioctl(s, SIOCGIFFLAGS, &req) < 0)
            continue;
        if (!(req.ifr_flags & IFF_UP))
            continue;
        if (!(req.ifr_flags & IFF_RUNNING)) //llm
            continue;
        if (req.ifr_flags & IFF_LOOPBACK)
            continue;

        if(ioctl(s,SIOCGIFHWADDR,&req)<0)
            continue;
        
        sprintf(mac,"%02x:%02x:%02x:%02x:%02x:%02x",
                            (unsigned char)req.ifr_hwaddr.sa_data[0],
                            (unsigned char)req.ifr_hwaddr.sa_data[1],
                            (unsigned char)req.ifr_hwaddr.sa_data[2],
                            (unsigned char)req.ifr_hwaddr.sa_data[3],
                            (unsigned char)req.ifr_hwaddr.sa_data[4],
                            (unsigned char)req.ifr_hwaddr.sa_data[5]);
        printf("get_mac:%s\n",mac);
        close(s);
        fclose(fd);
        return TRUE;
    }
        
    close(s);
    fclose(fd);
    return FALSE;
}


/****************************************
* checkIpThread
****************************************/

int checkIpThreadExit = 1; 


void* checkIpThread (CgUpnpDevice *device)
{
    CgNetworkInterfaceList *current = NULL;
    CgNetworkInterfaceList *old = NULL;
    CgNetworkInterface *netIfOld = NULL;
    CgNetworkInterface *netIfNew = NULL;
    CgNetworkInterface *tmp = NULL;
    BOOL found;
    BOOL bret;
    
    int httpPort;
    char *bindAddr = NULL;
    CgHttpServer *httpServer = NULL;
      
    CG_HTTP_LISTENER httpListener = NULL;

//  CgUpnpSUDPServer *sudpServer;
//  CgUpnpSUDPServerList *sudpServerList;
    CgUpnpSSDPServer *ssdpServer; 
    CgUpnpSSDPServerList *ssdpServerList; 

    printf("enter check thread______________\n");
    
    if(device == NULL)
    {
        printf("device is NULL!!!!!!!!!!\n");
        return; 
    }

//  old = cg_net_interfacelist_new();
//  cg_net_gethostinterfaces(old);  
    
    while(checkIpThreadExit == 1)
    {  
     
        usleep(1000*2000);
        
        #if 1
        current = cg_net_interfacelist_new();
        old = device->ifCache;
    
        if (current == NULL || old == NULL )
        {
            continue;
        }
       
        cg_net_gethostinterfaces(current);
        tmp = cg_net_interfacelist_gets(current);
           
        while (tmp != NULL)
        {   
            found = FALSE;
    
            netIfNew = tmp; 
            tmp = cg_net_interface_next(netIfNew);  
            
            for (netIfOld = cg_net_interfacelist_gets(old); netIfOld != NULL;
                 netIfOld = cg_net_interface_next(netIfOld))
            {
                if (cg_net_interface_cmp(netIfOld, netIfNew) == 0)
                {
                    found = TRUE;
                    break;
                }
            }
            
            /* New interface was not found in old ones,*/
            if (found == FALSE)
            {
                printf("[notify] new network interface add\n");
                printf("++++++++++++++++++++++++++++++++++\n");
                printf("++++++++++++++++++++++++++++++++++\n");
                printf("++++++++++++++++++++++++++++++++++\n");
                printf("new interfacd is %s\n",cg_net_interface_getaddress(netIfNew));

                #if 1   
                httpPort = cg_upnp_device_gethttpport(device);
                bindAddr = cg_net_interface_getaddress(netIfNew);
                if (cg_strlen(bindAddr) <= 0)
                    break;

                #if 1
                bret = cg_upnp_device_start(device);
                if(bret == FALSE)
                {
                    printf("dmr start fail!!!!!!\n");
                    break;
                }
                #endif

                

                printf("dmr restart success\n");
                #endif
                                        
                break;
            }
        }//end while (current != NULL)
            
        if (found == TRUE)
        {
            cg_net_interfacelist_delete(current);
        }
        #endif
    }//end while(1)
      
}




/****************************************
 * cg_upnpav_dmr_addprotocolinfo
 ****************************************/

void cg_upnpav_dmr_addprotocolinfo(CgUpnpAvRenderer *dmr, CgUpnpAvProtocolInfo *info)
{
    CgString *protocolInfos;
    CgUpnpAvProtocolInfo *protocolInfo;
    CgUpnpService *service;
    CgUpnpStateVariable *stateVar;

    cg_upnpav_protocolinfolist_add(dmr->protocolInfoList, info);

    protocolInfos = cg_string_new();
    for (protocolInfo = cg_upnpav_dmr_getprotocolinfos(dmr); protocolInfo; protocolInfo = cg_upnpav_protocolinfo_next(protocolInfo)) {
        if (0 < cg_string_length(protocolInfos))
            cg_string_addvalue(protocolInfos, ",");
        cg_string_addvalue(protocolInfos, cg_upnpav_protocolinfo_getstring(protocolInfo));
    }

    service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_TYPE);
    stateVar = cg_upnp_service_getstatevariablebyname(service, CG_UPNPAV_DMR_CONNECTIONMANAGER_SINKPROTOCOLINFO);
    cg_upnp_statevariable_setvalue(stateVar, cg_string_getvalue(protocolInfos));

    cg_string_delete(protocolInfos);
}



/****************************************
* cg_upnpav_dmr_actionreceived
****************************************/

BOOL cg_upnpav_dmr_actionreceived(CgUpnpAction *action)
{
    CgUpnpDevice *dev;
    CgUpnpService *service;
    CgUpnpAvRenderer *dmr;
    CG_UPNPAV_ACTION_LISTNER userActionListener;

    service = cg_upnp_action_getservice(action);
    if (!service)
        return FALSE;

    dev = (CgUpnpDevice *)cg_upnp_service_getdevice(service);
    if (!dev)
        return FALSE;

    dmr = (CgUpnpAvRenderer *)cg_upnp_device_getuserdata(dev);
    if (!dmr)
        return FALSE;

    userActionListener = cg_upnpav_dmr_getactionlistener(dmr);
    if (userActionListener) {
        if (userActionListener(action))
            return TRUE;
    }

    if (cg_streq(cg_upnp_service_getservicetype(service), CG_UPNPAV_DMR_AVTRANSPORT_SERVICE_TYPE))
        return cg_upnpav_dmr_avtransport_actionreceived(action);

    if (cg_streq(cg_upnp_service_getservicetype(service), CG_UPNPAV_DMR_RENDERINGCONTROL_SERVICE_TYPE))
        return cg_upnpav_dmr_renderingctrl_actionreceived(action);

    if (cg_streq(cg_upnp_service_getservicetype(service), CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_TYPE))
        return cg_upnpav_dmr_conmgr_actionreceived(action);

    return FALSE;
}

/****************************************
* cg_upnpav_dmr_queryreceived
****************************************/

BOOL cg_upnpav_dmr_queryreceived(CgUpnpStateVariable *statVar)
{
    CgUpnpDevice *dev;
    CgUpnpService *service;
    CgUpnpAvRenderer *dmr;
    CG_UPNPAV_STATEVARIABLE_LISTNER userQueryListener;

    service = cg_upnp_statevariable_getservice(statVar);
    if (!service)
        return FALSE;

    dev = (CgUpnpDevice *)cg_upnp_service_getdevice(service);
    if (!dev)
        return FALSE;

    dmr = (CgUpnpAvRenderer *)cg_upnp_device_getuserdata(dev);
    if (!dmr)
        return FALSE;

    userQueryListener = cg_upnpav_dmr_getquerylistener(dmr);
    if (userQueryListener) {
        if (userQueryListener(statVar))
            return TRUE;
    }

    if (cg_streq(cg_upnp_service_getservicetype(service), CG_UPNPAV_DMR_AVTRANSPORT_SERVICE_TYPE))
        return cg_upnpav_dmr_avtransport_queryreceived(statVar);

    if (cg_streq(cg_upnp_service_getservicetype(service), CG_UPNPAV_DMR_RENDERINGCONTROL_SERVICE_TYPE))
        return cg_upnpav_dmr_renderingctrl_queryreceived(statVar);

    if (cg_streq(cg_upnp_service_getservicetype(service), CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_TYPE))
        return cg_upnpav_dmr_conmgr_queryreceived(statVar);

    return FALSE;
}

/****************************************
* cg_upnpav_dmr_device_httprequestrecieved
****************************************/

void cg_upnpav_dmr_device_httprequestrecieved(CgHttpRequest *httpReq)
{
    CgUpnpAvRenderer *dmr;
    CgUpnpDevice *dev;
    CG_UPNPAV_HTTP_LISTENER userHttpListener;

    dev = (CgUpnpDevice *)cg_http_request_getuserdata(httpReq);
    if (!dev) {
        cg_upnp_device_httprequestrecieved(httpReq);
        return;
    }

    dmr = (CgUpnpAvRenderer *)cg_upnp_device_getuserdata(dev);
    if (!dmr) {
        cg_upnp_device_httprequestrecieved(httpReq);
        return;
    }

    userHttpListener = cg_upnpav_dmr_gethttplistener(dmr);
    if (userHttpListener) {
        if (userHttpListener(httpReq))
            return;
    }

    cg_upnp_device_httprequestrecieved(httpReq);
}

/****************************************
* cg_upnpav_dmr_new
****************************************/

CgUpnpAvRenderer *cg_upnpav_dmr_new()
{
    CgUpnpAvRenderer *dmr;
    char *lastChange;
  
    char macaddr[18];

    #ifdef USING_GSTREAMER
         printf("============================================\n");
    #endif

//  cg_upnp_setnmprmode(TRUE);  //add by llm 20130619 don't use URLbase

    dmr = (CgUpnpAvRenderer *)malloc(sizeof(CgUpnpAvRenderer));
    if(dmr == NULL)
    {
        printf("malloc memory for dmr fail!!!!!!!\n");
        return NULL;
    }

    dmr->dev = cg_upnp_device_new();
    if (!dmr->dev) {
        free(dmr);
        printf("malloc memory for dev fail!!!!!!!\n");
        return NULL;
    }

    if (cg_upnp_device_loaddescriptionfile(dmr->dev, CG_UPNPAV_DMR_DEVICE_DESCRIPTION) == FALSE) {
        cg_upnp_device_delete(dmr->dev);
        free(dmr);
        printf("parsedescription fail !!!!!!!!!!!\n");
        return NULL;
    }
    #if 0
    if (cg_upnp_device_parsedescription(dmr->dev, CG_UPNPAV_DMR_DEVICE_DESCRIPTION, cg_strlen(CG_UPNPAV_DMR_DEVICE_DESCRIPTION)) == FALSE) {
        cg_upnp_device_delete(dmr->dev);
        free(dmr);
        printf("parsedescription fail !!!!!!!!!!!\n");
        return NULL;
    }
    #endif
    
    dmr->protocolInfoList = cg_upnpav_protocolinfolist_new();
    dmr->trackInfoList = cg_upnpav_trackinfolist_new(); //add by llm 20130319

    if (cg_upnpav_dmr_conmgr_init(dmr) == FALSE) {
        cg_upnp_device_delete(dmr->dev);
        free(dmr);
        printf("conmgr_init fail !!!!!!!!!!!\n");
        return NULL;
    }

    if (cg_upnpav_dmr_renderingctrl_init(dmr) == FALSE) {
        cg_upnp_device_delete(dmr->dev);
        free(dmr);
        printf("renderingctrl_init fail !!!!!!!!!!!\n");
        return NULL;
    }

    if (cg_upnpav_dmr_avtransport_init(dmr) == FALSE) {
        cg_upnp_device_delete(dmr->dev);
        free(dmr);
        printf("avtransport_init fail !!!!!!!!!!!\n");
        return NULL;
    }

    dmr->mutex = cg_mutex_new();
    if (!dmr->mutex) {
        cg_upnpav_dmr_delete(dmr);
        return NULL;
    }
    
    
    cg_upnp_device_setactionlistener(dmr->dev, cg_upnpav_dmr_actionreceived);
    cg_upnp_device_setquerylistener(dmr->dev, cg_upnpav_dmr_queryreceived);
    cg_upnp_device_sethttplistener(dmr->dev, cg_upnpav_dmr_device_httprequestrecieved);
//  cg_upnp_device_setsudplistener(dmr->dev,cg_upnpav_dmr_dev_sudprecieved);

    cg_upnpav_dmr_setactionlistener(dmr, NULL);
    cg_upnpav_dmr_setquerylistener(dmr, NULL);
    cg_upnpav_dmr_sethttplistener(dmr, NULL);

    cg_upnp_device_setuserdata(dmr->dev, dmr);
    cg_upnp_device_updateudn(dmr->dev);
   
    cg_upnpav_dmr_get_macaddr(macaddr);    
    cg_upnp_device_setserialnumber(dmr->dev, macaddr);

    dmr->uriMode = UNICAST;
#if 0
    dmr->uniMutex = cg_mutex_new();
    if (!dmr->uniMutex) {
        cg_upnpav_dmr_delete(dmr);
        return NULL;
    }

    dmr->multiMutex = cg_mutex_new();
    if (!dmr->multiMutex) {
        cg_upnpav_dmr_delete(dmr);
        return NULL;
    }
#endif
    return dmr;
}

/****************************************
* cg_upnpav_dmr_delete
****************************************/

void cg_upnpav_dmr_delete(CgUpnpAvRenderer *dmr)
{
    if (dmr == NULL)
        return;

    if (dmr->mutex)
        cg_mutex_delete(dmr->mutex);

    if (dmr->protocolInfoList)
        cg_upnpav_protocolinfolist_delete(dmr->protocolInfoList);

    /*add by llm 20130319*/
    if (dmr->trackInfoList)
        cg_upnpav_trackinfolist_delete(dmr->trackInfoList);

    if (dmr->transportMutex)
        cg_mutex_delete(dmr->transportMutex);
    /*end llm*/
#if 0
    if (dmr->uniMutex)
        cg_mutex_delete(dmr->uniMutex);

    if (dmr->multiMutex)
        cg_mutex_delete(dmr->multiMutex);
#endif
    cg_upnp_device_delete(dmr->dev);

    free(dmr);
}



