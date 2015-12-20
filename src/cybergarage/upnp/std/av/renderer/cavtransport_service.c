/************************************************************
*
*   CyberLink for C
*
*   Copyright (C) Satoshi Konno 2005
*
*   File: cavtransport_service.c
*
*   Revision:
*       2009/06/11
*        - first release.
*       2013/03/18 Liu Liming
*        - carry out  function cg_upnpav_dmr_avtransport_actionreceived
*
************************************************************/

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <cybergarage/upnp/std/av/cmediarenderer.h>
#include <cybergarage/upnp/std/av/ctypeconver.h>
#include <gst/gst.h>
#include <stdio.h>
//#include <totem-pl-parse/totem_m3u_parser.h>

#include <sys/time.h> //for test


/****************************************
* Service Description (AVTransport)
****************************************/

/*add by llm*/

#define LLMDEBUG 1

int rssi = 0; //add by zhongyouguang


#ifdef LLMDEBUG
#define   cg_drm_av_debug(err, errCode) printf("_____%s,%d,%s\ndebug msg: %s / errcode: %d\n",__FILE__, __LINE__, __PRETTY_FUNCTION__, err, errCode)
#endif




void cg_upnpav_dmr_set_error(CgUpnpAction *action, int errorCode, const char *errorMsg)
{
    action->upnpStatus->code = CG_UPNP_STATUS_ACTION_FAILED;
    fprintf(stderr, "%s: %s, %d\n", __FUNCTION__, errorMsg, errorCode);
}




/*flag for  playstate*/
static TRANSPORTSTATE transport_state = TRANSPORT_NO_MEDIA_PRESENT;

/*flag for dedicating whether Pause media operation should be executed when the DMS does not support the necessary features*/
static BOOL bHttpStalling = TRUE;

BOOL bGetDuration = FALSE;
BOOL buniPipeline = FALSE;
BOOL bmultiPipeline = FALSE;

static BOOL bSudpServerStart = FALSE;


/****************************************
* cg_upnpav_dmr_avtransport_playprepare
****************************************/
BOOL cg_upnpav_dmr_avtransport_playprepare(CgUpnpAvRenderer *dmr, CgUpnpDevice *dev, CgUpnpService *service, CgUpnpAction *action)
{

    CgUpnpArgument *arg;
    BOOL bActionFlag = TRUE;
    

    arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_SESSION );
    if (!arg)
    {
        cg_drm_av_debug("Action:PlayPrepare, invalid argument", 2);
        return FALSE;
    }
/*
 * I don't remember what sudp is
 */
#if 0
    /*star sudp server*/
    if(bSudpServerStart == FALSE)
    {
        if (cg_upnp_sudp_serverlist_open(dev->sudpServerList) == FALSE)
            return FALSE;   
        cg_upnp_sudp_serverlist_setuserdata(dev->sudpServerList, dev);
        cg_upnp_sudp_serverlist_setlistener(dev->sudpServerList, cg_upnp_device_getsudplistener(dev));
        cg_upnp_sudp_serverlist_start(dev->sudpServerList);

        bSudpServerStart = TRUE;
    }  
#endif
    /*set section value*/
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_UDPSESSION, arg->value->value);


    /*set gstreamer to pause state*/
    cg_mutex_lock(dmr->transportMutex);

    
    if (output_udp_pause(dmr) != 0) 
    {
        cg_drm_av_debug("pause fail!!!", 1);
        bActionFlag = FALSE;
    } 
    else 
    {
        transport_state = TRANSPORT_PAUSED_PLAYBACK;
        cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE, "PAUSED_PLAYBACK");   
        cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "Stop,Play");

        cg_upnpav_dmr_avtransport_change_lastchange(dmr);
    }

    cg_mutex_unlock(dmr->transportMutex);

    return bActionFlag;
    
}


/****************************************
* cg_upnpav_dmr_avtransport_set_transportstate
****************************************/

void cg_upnpav_dmr_avtransport_set_transportstate(TRANSPORTSTATE state)
{
    transport_state = state;
}


/****************************************
* cg_upnpav_dmr_avtransport_GetControlPoint
* 获取控制点的名字，私有的，非强制的
****************************************/
BOOL cg_upnpav_dmr_avtransport_GetControlPoint(CgUpnpAvRenderer *dmr, CgUpnpDevice *dev, CgUpnpService *service, CgUpnpAction *action)
{
    CgUpnpArgument *arg;

    arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_INSTANCEID);
    if (!arg)
        return FALSE;

    arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_X_ControlPoint);
    if(!arg)
    {
        printf("arg is null\n");
        return FALSE;
    }
    cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTCONTROLPOINT));

    return TRUE;
}


/****************************************
* cg_upnpav_dmr_avtransport_SetControlPoint
* 设置控制点的名字，私有的，非强制的
****************************************/
BOOL cg_upnpav_dmr_avtransport_SetControlPoint(CgUpnpAvRenderer *dmr, CgUpnpDevice *dev, CgUpnpService *service, CgUpnpAction *action)
{
    CgUpnpArgument *arg;

    arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_INSTANCEID);
    if (!arg)
        return FALSE;

    arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_X_ControlPoint);
    if(!arg)
    {
        printf("arg is null\n");
        return FALSE;
    }

    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTCONTROLPOINT, arg->value->value);

    cg_upnpav_dmr_avtransport_change_lastchange(dmr);

    return TRUE;
}


/****************************************
* cg_upnpav_dmr_avtransport_SetControlPoint
* 获得WIFI的强的，私有的，非强制的
****************************************/
BOOL cg_upnpav_dmr_avtransport_GetWifiIntes(CgUpnpAvRenderer *dmr, CgUpnpDevice *dev, CgUpnpService *service, CgUpnpAction *action)
{
    CgUpnpArgument *arg;

    char argvalue[8];
    sprintf(argvalue, "%d", rssi);
    printf("rssi = %d\n", rssi);
        
    arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_INSTANCEID);
    if (!arg)
        return FALSE;

    arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_X_WIFIINTES);
    if(!arg)
    {
        printf("arg is null\n");
        return FALSE;
    }

    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTWIFIINTES, argvalue);
    
    cg_upnp_argument_setvalue(arg, argvalue);

    return TRUE;
}


/****************************************
* cg_upnpav_dmr_avtransport_setmulticasturi
****************************************/
BOOL cg_upnpav_dmr_avtransport_setmulticasturi(CgUpnpAvRenderer *dmr, 
                                               CgUpnpDevice *dev,
                                               CgUpnpService *service,
                                               CgUpnpAction *action)
{
    CgUpnpArgument *arg;
    CgUpnpAvTrackInfo *trackInfo;
    char multicastIp[20];
    char multicastPort[10];
    char* lastMulticastIp;
    char* lastMulticastPort;
        
    arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_INSTANCEID);
    if (!arg)
        return FALSE;

    cg_upnpav_trackinfolist_clear(dmr->trackInfoList);

    arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTURI);
    if (!arg)
        return FALSE;

    /* parse multicastUri */
    if( cg_parse_multicastUri(arg->value->value, multicastIp, multicastPort) != 0 )
    {
        cg_drm_av_debug("parse multicastUri error",-1);
        return FALSE;
    }

    cg_mutex_lock(dmr->transportMutex);
    cg_drm_av_debug("dmr->transportMutex++++++++++++++++++++++++++",0);

    /*check uirMode*/
    if(dmr->uriMode == UNICAST && buniPipeline == TRUE)
    {
        output_close_playbinloop();
        
        buniPipeline = FALSE;
        
        dmr->uriMode = MULTICAST;
        
        if( output_gstreamer_rtpbin_init(dmr, multicastIp, multicastPort) !=0 )
        {
             cg_drm_av_debug("rtpbin init error!!!",0);
             
             cg_drm_av_debug("dmr->transportMutex___________________",0);
             cg_mutex_unlock(dmr->transportMutex);
             return FALSE;
        }
        
        bmultiPipeline = TRUE;
    }
    else
    {
        dmr->uriMode = MULTICAST;
        
        /*check whether the pipeline is setted up*/
        if(bmultiPipeline == FALSE)
        {
            if( output_gstreamer_rtpbin_init(dmr, multicastIp, multicastPort) !=0 )
            {
                cg_drm_av_debug("rtpbin init error!!!",0);
                
                cg_drm_av_debug("dmr->transportMutex______________________",0);
                cg_mutex_unlock(dmr->transportMutex);
                return FALSE;
            }
        
            bmultiPipeline = TRUE;
        }
        else
        {
            lastMulticastIp = cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_MULTICASTIP );
            lastMulticastPort = cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_MULTICASTPORT );

            /*check whether multicast ip and port are the same as exiting pipeline */
            if(cg_streq(lastMulticastIp, multicastIp) && cg_streq(lastMulticastPort, multicastPort))
            {
            
            }
            else
            {
                output_close_rtploop();
                usleep(1000*10);
                if( output_gstreamer_rtpbin_init(dmr, multicastIp, multicastPort) !=0 )
                {
                    cg_drm_av_debug("rtpbin init error!!!",0);
                    
                    cg_drm_av_debug("dmr->transportMutex_____________________",0);
                    cg_mutex_unlock(dmr->transportMutex);
                    bmultiPipeline = FALSE;
                    return FALSE;
                }
            }
        }
    }

    /* stop playing */

    switch (transport_state) 
    {
        case TRANSPORT_STOPPED:
        break;
        
        case TRANSPORT_NO_MEDIA_PRESENT:
            cg_upnpav_dmr_avtransport_set_transportstate(TRANSPORT_STOPPED);
            cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE, "STOPPED");        
            cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "Play");
                
        break;
        
        case TRANSPORT_PLAYING:
        case TRANSPORT_TRANSITIONING:
        case TRANSPORT_PAUSED_RECORDING:
        case TRANSPORT_RECORDING:
        case TRANSPORT_PAUSED_PLAYBACK:
            if(output_stop()!=0)
            {
                cg_drm_av_debug("stop fail!!!!", -1);
                cg_upnpav_dmr_set_error(action, CG_UPNPAV_TRANSPORT_E_PLAY_FORMAT_NS, "Playing failed");
            }
            else
            {
                cg_drm_av_debug("stop succeed", 0);
                cg_upnpav_dmr_avtransport_set_transportstate(TRANSPORT_STOPPED);
                cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE, "STOPPED");        
                cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "Play");
              
            }
           
        break;
    }
    
    cg_drm_av_debug("dmr->transportMutex________________________",0);
    cg_mutex_unlock(dmr->transportMutex);

    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_MULTICASTIP, multicastIp );
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_MULTICASTPORT, multicastPort );
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_AVTRANSPORTURI, arg->value->value );

    return TRUE;
    
}

char * returnMallocbuf()
{
    char *out=malloc(5);
    out[0] = '1';
    out[1] = '1';
    out[2] = '2';
    out[3] = '\0';
    printf("out = %s \n", out);
    return out;
}



/****************************************
* cg_upnpav_dmr_avtransport_actionreceived
****************************************/

BOOL cg_upnpav_dmr_avtransport_actionreceived(CgUpnpAction *action)
{
    CgUpnpAvRenderer *dmr;
    CgUpnpDevice *dev;
    CgUpnpService *service;
    char *actionName;
    CgUpnpArgument *arg;
    CgUpnpAvTrackInfo *trackInfo;

    double percentage = 0.0; 
    int currentMediaduration = 0;
    int seekTime = 0;   
    int icurrentTrackNum;
    char strcurrentTrackNum[5];
    BOOL bActionFlag = TRUE;
    char *xmlbuf = NULL;

    int outputRet;

    /*variable for parse Metadata*/
    char *xmlProtocolInfo;
    char *xmlDuration;
    CgXmlParser *xmlParser;
    CgXmlNodeList *rootNode;
    CgXmlNode *didlNode;
    CgXmlNode *resNode;
    CgXmlNode *itemNode;
    char *cDLNAORG_FLAGS;
    char *cDLNAORG_PN;
    char *cDLNAORG_OP;

    /*store Metadata */
    CgString *valueStr;

    actionName = cg_upnp_action_getname(action);
    if (cg_strlen(actionName) <= 0)
        return FALSE;

    printf("--------------------------------------\n");
    printf("[avtransport] actionName : %s\n",actionName);
    // test code add by patrick
    //CTT test  ERROR 7.3.93.2
    if ( 0 == strcmp( actionName, "Pause" ))
    {
        usleep(130000);
    }
    if ( 0 == strcmp( actionName, "Stop" ))
    {
        usleep(30000);
    }

    service = cg_upnp_action_getservice(action);
    if (!service)
        return FALSE;

    dev = (CgUpnpDevice *)cg_upnp_service_getdevice(service);
    if (!dev)
        return FALSE;

    dmr = (CgUpnpAvRenderer *)cg_upnp_device_getuserdata(dev);
    if (!dmr)
        return FALSE;

    /* SetMulticastURI */
    if(cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_SETMULTICASTURI))
    {
        //return cg_upnpav_dmr_avtransport_setmulticasturi(dmr, dev, service, action);
        return FALSE;
    }
  
    /* SetAVTransportURI*/      
    if(cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_SETAVTRANSPORTURI))
    {
        //InstancdID
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_INSTANCEID);
        if (!arg)
            return FALSE;

        cg_upnpav_trackinfolist_clear(dmr->trackInfoList);  //add by llm 20130319

        bHttpStalling = TRUE;
        bGetDuration = FALSE;

        cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_UDPSESSION, "0");

        
 #if 0
        /*check the current playmode is unicast or multicast*/
        if(dmr->uriMode == MULTICAST && bmultiPipeline == TRUE)
        {
            output_close_rtploop();
            bmultiPipeline = FALSE;
            dmr->uriMode = UNICAST;
            if(output_gstreamer_playbin_init(dmr) != 0)
            {
               cg_drm_av_debug("playbin init error!!!",0);
               
               cg_drm_av_debug("dmr->transportMutex______________________",0);
               cg_mutex_unlock(dmr->transportMutex);
               return FALSE;
            }
            buniPipeline = TRUE;
        }
        else
        {
            dmr->uriMode = UNICAST;
            
            /*check whether the pipeline is setted up*/
            if(buniPipeline == FALSE)
            {
                if(output_gstreamer_playbin_init(dmr) != 0)
                {
                    cg_drm_av_debug("playbin init error!!!",0);
                    
                    cg_drm_av_debug("dmr->transportMutex_______________________",0);
                    cg_mutex_unlock(dmr->transportMutex);
                    return FALSE;
                }
                buniPipeline = TRUE;
            }
        }
  #endif
        cg_mutex_lock(dmr->transportMutex);     
        cg_drm_av_debug("dmr->transportMutex++++++++++++++++++++++++++",0);

        /* stop playing */
        switch (transport_state) 
        {
            case TRANSPORT_STOPPED:
            break;
            
            case TRANSPORT_NO_MEDIA_PRESENT:
                   cg_upnpav_dmr_avtransport_set_transportstate(TRANSPORT_STOPPED);
                   cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE, "STOPPED");         
                   cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "Play");
                
            break;
            
            case TRANSPORT_PLAYING:
            case TRANSPORT_TRANSITIONING:
            case TRANSPORT_PAUSED_RECORDING:
            case TRANSPORT_RECORDING:
            case TRANSPORT_PAUSED_PLAYBACK:
                if(output_stop()!=0)
                {
                    cg_drm_av_debug("stop fail", -1);
                    cg_upnpav_dmr_set_error(action, CG_UPNPAV_TRANSPORT_E_PLAY_FORMAT_NS, "Playing failed");
                }
                else
                {
                    cg_drm_av_debug("stop success", 0);
                    cg_upnpav_dmr_avtransport_set_transportstate(TRANSPORT_STOPPED);
                    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE, "STOPPED");        
                    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "Play");
              
                }
           
            break;
        }
        
        cg_drm_av_debug("dmr->transportMutex______________________",0);
        cg_mutex_unlock(dmr->transportMutex);
        

        //CurrentURI
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTURI);
        if (!arg)
            return FALSE;

        printf("CurrentURI = %s\n", arg->value->value); //llm debug

        if(arg->value->value == NULL)
        {
            /*When arg is null, reset playstate to "NO_MEDIA_PRESENT", uri to "",NumberOfTracks to "0" */
            cg_upnpav_dmr_avtransport_set_transportstate(TRANSPORT_NO_MEDIA_PRESENT);
            cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE, "NO_MEDIA_PRESENT");       
            cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "");

            
            cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_AVTRANSPORTURI, "");
            cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKURI, "");

            cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_NUMBEROFTRACKS, "0");         
            cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACK, "0");           
        }
        else
        {          
            if( cg_strstr(arg->value->value,"m3u") > 0 )  
            {
                /*playlist*/
                //totem_m3u_parse_uri(arg->value->value, dmr);
            }
            else                                          
            {
                /*single song*/
                cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_AVTRANSPORTURI, arg->value->value);
                cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKURI, arg->value->value);
            
                cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_NUMBEROFTRACKS, "1"); 
                cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACK, "1");                       

                /*store uri to struct CgUpnpAvTrackInfo*/
                trackInfo = cg_upnpav_trackinfo_new();
                if(trackInfo == NULL)
                {
                    printf("%s %d %s can't malloc trackInfo\n",__FILE__, __LINE__, __PRETTY_FUNCTION__);
                    return FALSE;
                }           
                cg_upnpav_trackinfo_settrack(trackInfo, arg->value->value);
                cg_upnpav_trackinfolist_add(dmr->trackInfoList,trackInfo);              
            }
        }
        

        //CurrentURIMetaData
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTURIMETADATA);
        if (!arg)
            return FALSE;
        
        printf("CurrentURIMetaData = %s\n", arg->value->value); //llm debug

        if(arg->value->value == NULL)
        {
            cg_upnpav_dmr_setprotocolinfo(dmr, "");
        
            cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_AVTRANSPORTURIMETADATA, "");
            cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKMETADATA, "");
        }
        else
        {
            /* parase MetaData*/
            rootNode = cg_xml_nodelist_new();
            xmlParser = cg_xml_parser_new();
            if (cg_xml_parse(xmlParser, rootNode, arg->value->value, strlen(arg->value->value))) 
            {
                didlNode = cg_xml_nodelist_getbyname(rootNode, "DIDL-Lite");
                if (didlNode) 
                {
                    itemNode = cg_xml_node_getchildnodebyname(didlNode, "item");
                    if(itemNode)
                    {
                        resNode = cg_xml_node_getchildnodebyname(itemNode, "res");
                        if(resNode)
                        {
                            /*parase duration from MetaData*/
                            xmlDuration = cg_xml_node_getattributevalue(resNode, "duration");
                            if(xmlDuration)
                            {
                                printf("duration = %s\n",xmlDuration);
                                cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKDURATION, xmlDuration);
                                cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTMEDIADURATION, xmlDuration);
                                bGetDuration = TRUE;
                            }

                            /*parase ProtocolInfo from MetaData*/
                            xmlProtocolInfo = cg_xml_node_getattributevalue(resNode, "protocolInfo");
                            if(xmlProtocolInfo)
                            {
                                printf("current ProtocolInfo = %s\n", xmlProtocolInfo);
                                cg_upnpav_dmr_setprotocolinfo(dmr, xmlProtocolInfo);

                                /* parase DLNA.ORG_PN*/
                                cDLNAORG_PN = strstr(xmlProtocolInfo, "DLNA.ORG_PN");
                                if(cDLNAORG_PN)
                                {
                                    cDLNAORG_FLAGS = strstr(xmlProtocolInfo, "DLNA.ORG_FLAGS");
                                    cDLNAORG_OP = strstr(xmlProtocolInfo, "DLNA.ORG_OP");

                                    /*if the OP and FLAGS param are missing in the DMS protocolinfo for the resource, 
                                    then Pause-Resume must not be supported.*/
                                    if( (!cDLNAORG_FLAGS) && (!cDLNAORG_OP) )
                                    {
                                        printf("DMS do not support byte seek and time seek\n");
                                        bHttpStalling = FALSE;
                                    }
                                    
                                    #if 0
                                    if((*(startDLNAORG_FLAGS+1+15)) == '1' && (*(startDLNAORG_FLAGS+2+15)) == '1')
                                    {
                                        printf("DMS do not support byte seek and time seek\n");
                                        bHttpStalling = FALSE;
                                    }
                                    #endif
                                }
                                
                            }
                            else
                            {
                                cg_upnpav_dmr_setprotocolinfo(dmr, "");
                            }
                        }
                    }
                }
            }
            cg_xml_nodelist_delete(rootNode);
            cg_xml_parser_delete(xmlParser);
            /*end parase*/
            
    //        xmlbuf = xmlescape((const char*)arg->value->value, (int)1);
            printf("set meta value =%s\n", arg->value->value);
            cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_AVTRANSPORTURIMETADATA, arg->value->value);
            cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKMETADATA, arg->value->value);
 
        //    free(xmlbuf);
        }
        
        //change 'PlaybackStorageMedium' from "NONE" to "NETWORK"
        if(cg_streq(cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_PLAYBACKSTORAGEMEDIUM), "NONE"))
        {
            cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_PLAYBACKSTORAGEMEDIUM, "NETWORK");        
        }

        //Set lastChange
        cg_upnpav_dmr_avtransport_change_lastchange(dmr);
        
        return TRUE;
    }
    #if 0
    /*SetNextAVTransportURI*/
    if(cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_SETNEXTAVTRANSPORTURI))
    {
       
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_INSTANCEID);
        if (!arg)
            return FALSE;

        //NextURI
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_NEXTURI);
        if (!arg)
            return FALSE;
        printf("NEXTURI = %s\n", arg->value->value); //llm debug
        cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_NEXTAVTRANSPORTURI, arg->value->value);
        cg_upnpav_dmr_avtransport_change_var(dmr, CG_UPNPAV_DMR_AVTRANSPORT_NEXTAVTRANSPORTURI, arg->value->value);

        //NextURIMetaData
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_NEXTURIMETADATA);
        if (!arg)
            return FALSE;
        printf("NEXTURIMetaData = %s\n", arg->value->value); //llm debug
        cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_NEXTAVTRANSPORTURIMETADATA, arg->value->value);
        cg_upnpav_dmr_avtransport_change_var(dmr, CG_UPNPAV_DMR_AVTRANSPORT_NEXTAVTRANSPORTURIMETADATA, arg->value->value);
        
        return TRUE;
    }
    #endif
#if 1   
    /*GetMediaInfo*/
    if(cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_GETMEDIAINFO))
    {
       
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_INSTANCEID);
        if (!arg)
            return FALSE;

        //NrTracks
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_NRTRACKS);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_NUMBEROFTRACKS));
            
        //MediaDuration 
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_MEDIADURATION);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTMEDIADURATION));
        
        //CurrentURI
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTURI);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_AVTRANSPORTURI));

        //CurrentURIMetaData 
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTURIMETADATA);
        if (!arg)
            return FALSE;

        valueStr = cg_string_new();
        cg_string_setvalue(valueStr, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKMETADATA));
        cg_upnp_argument_setvalue(arg, cg_xml_unescapechars(valueStr));  /*transport symbol */
        cg_string_delete(valueStr);

        //NextURI
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_NEXTURI);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_NEXTAVTRANSPORTURI));

        //NextURIMetaData 
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_NEXTURIMETADATA);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_NEXTAVTRANSPORTURIMETADATA));

        //PlayMedium
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_PLAYMEDIUM);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_PLAYBACKSTORAGEMEDIUM));
        
        //RecordMedium
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_RECORDMEDIUM);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_RECORDSTORAGEMEDIUM));
        
        //WriteStatus
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_WRITESTATUS);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_RECORDMEDIUMWRITESTATUS));
        
        return TRUE;
    }
#endif

#if 1
    /*GetMediaInfo_Ext*/
    if(cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_GETMEDIAINFO_EXT))
    {
        
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_INSTANCEID);
        if (!arg)
            return FALSE;

        //CurrentType
        //NrTracks
        //MediaDuration 
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_MEDIADURATION);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTMEDIADURATION));
        
        //CurrentURI
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTURI);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_AVTRANSPORTURI));

        //CurrentURIMetaData 
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTURIMETADATA);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_AVTRANSPORTURIMETADATA));

        //NextURI
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_NEXTURI);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_NEXTAVTRANSPORTURI));

        //NextURIMetaData 
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_NEXTURIMETADATA);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_NEXTAVTRANSPORTURIMETADATA));

        //PlayMedium
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_PLAYMEDIUM);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_PLAYBACKSTORAGEMEDIUM));
        
        //RecordMedium
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_RECORDMEDIUM);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_RECORDSTORAGEMEDIUM));
        
        //WriteStatus
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_WRITESTATUS);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_RECORDMEDIUMWRITESTATUS));
        
                
        return TRUE;
    }
#endif
    /* GetTransportInfo*/
    if (cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_GETTRANSPORTINFO)) 
    {
        //CurrentTransportState
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE);
        if (!arg)
            return FALSE;
        printf("current CurrentTransportState :%s\n", cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE));
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE));

        //CurrentTransportStatus
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATUS);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATUS_OK); //need modify

        //CurrentSpeed
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTSPEED);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, "1");  //just support default speed
        
        return TRUE;
    }
    #if  1
    /*GetPositionInfo*/
    if(cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_GETPOSITIONINFO))
    {
        
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_INSTANCEID);
        if (!arg)
            return FALSE;
        
        //Track
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_TRACK);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACK));

        //TrackDuration 
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_TRACKDURATION);
        if (!arg)
            return FALSE;
//      printf("trackduration = %s \n", cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKDURATION));
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKDURATION));

        //TrackMetaData
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_TRACKMETADATA);
        if (!arg)
            return FALSE;

        valueStr = cg_string_new();
        cg_string_setvalue(valueStr, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKMETADATA));
        cg_upnp_argument_setvalue(arg, cg_xml_unescapechars(valueStr));
        cg_string_delete(valueStr);
   
        //TrackURI 
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_TRACKURI);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_AVTRANSPORTURI));

        //RelTime
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_RELTIME);
        if (!arg)
            return FALSE;
        printf("Rel time = %s\n", cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_RELATIVETIMEPOSITION));
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_RELATIVETIMEPOSITION));
 
        //AbsTime
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_ABSTIME);
        if (!arg)
            return FALSE;
        printf("Abs time = %s\n", cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_ABSOLUTETIMEPOSITION));
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_ABSOLUTETIMEPOSITION));

        //RelCount
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_RELCOUNT);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_RELATIVECOUNTERPOSITION));

        //AbsCount
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_ABSCOUNT);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_ABSOLUTECOUNTERPOSITION));
        
        return TRUE;
    }
    
    
    /*GetDeviceCapabilities*/
    if(cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_GETDEVICECAPABILITIES))
    {
        //InstanceID
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_INSTANCEID);
        if (!arg)
        {
            cg_drm_av_debug("Action:GetDeviceCapabilities invalid id", CG_UPNPAV_TRANSPORT_E_INVALID_IID);
            return FALSE;
        }

        //PlayMedia
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_PLAYMEDIA);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_POSSIBLEPLAYBACKSTORAGEMEDIA));

        //RecMedia
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_RECMEDIA);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_POSSIBLERECORDSTORAGEMEDIA));

        //RecQualityModes
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_RECQUALITYMODES);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_POSSIBLERECORDQUALITYMODES));

        return TRUE;
    }
    #endif
    /*GetTransportSettings*/
    if(cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_GETTRANSPORTSETTINGS))
    {
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_INSTANCEID);
        if (!arg)
        {
            cg_drm_av_debug("Action:GetTransportSettings invalid id", CG_UPNPAV_TRANSPORT_E_INVALID_IID);
            return FALSE;
        }

        //PlayMode
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_PLAYMODE);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTPALYMODE));

        //RecQualityMode
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_RECQUALITYMODE);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTRECORDQUALITYMODE));

        return TRUE;
        
    }
    
    /*Stop*/
    if(cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_STOP))
    {
        
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_INSTANCEID);
        if (!arg)
        {
            cg_drm_av_debug("Action:Stop invalid id", CG_UPNPAV_TRANSPORT_E_INVALID_IID);
            return FALSE;
        }

        cg_mutex_lock(dmr->transportMutex);        
        cg_drm_av_debug("dmr->transportMutex++++++++++++++++++++++++++",0);
        switch (transport_state) 
        {
            case TRANSPORT_STOPPED:
                printf("action state is stop\n");
                bActionFlag = TRUE;
                cg_upnpav_dmr_avtransport_change_lastchange(dmr);
            break;
            case TRANSPORT_PLAYING:
            case TRANSPORT_TRANSITIONING:
            case TRANSPORT_PAUSED_RECORDING:
            case TRANSPORT_RECORDING:
            case TRANSPORT_PAUSED_PLAYBACK:
                if(output_stop()!=0)
                {
                    printf("stop fail\n");
                    cg_upnpav_dmr_set_error(action, CG_UPNPAV_TRANSPORT_E_PLAY_FORMAT_NS, "Playing failed");
                    bActionFlag = FALSE;
                }
                else
                {
                    printf("stop  success\n");
                    cg_upnpav_dmr_avtransport_set_transportstate(TRANSPORT_STOPPED);
                    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE, "STOPPED");         
                    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "Play,X_DLNA_SeekTime,Seek");
              
                    cg_upnpav_dmr_avtransport_change_lastchange(dmr);
                }
           
            break;

           case TRANSPORT_NO_MEDIA_PRESENT:
               /* action not allowed in these states - error 701 */
               cg_upnpav_dmr_set_error(action, CG_UPNPAV_TRANSPORT_E_TRANSITION_NA,"Transition not allowed");
               bActionFlag = FALSE;
           break;
        }
        
        cg_drm_av_debug("dmr->transportMutex__________________________",0);
        cg_mutex_unlock(dmr->transportMutex);
        return bActionFlag;
    }
    /*Play*/
    if(cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_PLAY))
    {
       
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_INSTANCEID);
        if (!arg)
        {
            cg_drm_av_debug("Action:Stop invalid id", CG_UPNPAV_TRANSPORT_E_INVALID_IID);
            return FALSE;
        }

        #if 1
        //Check input arg of Speed
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_SPEED);
        if (!arg)
        {
            cg_drm_av_debug("Action:Play Speed is null", CG_UPNPAV_TRANSPORT_E_PLAYSPEED_NS);
            return FALSE;
        }

        if((!cg_streq(arg->value->value, "")) && (!cg_streq(arg->value->value, "1")))
        {
            cg_drm_av_debug("Action:Play Speed invaid", CG_UPNPAV_TRANSPORT_E_PLAYSPEED_NS);
            return FALSE;
        }
        #endif
   
        cg_mutex_lock(dmr->transportMutex);     
        cg_drm_av_debug("dmr->transportMutex++++++++++++++++++++++++++",0);

        switch (transport_state) 
        {
            case TRANSPORT_PLAYING:
                
                if (output_play(dmr, 1) != 0) 
                {
                    cg_upnpav_dmr_set_error(action, CG_UPNPAV_TRANSPORT_E_PLAY_FORMAT_NS, "Playing failed");
                    bActionFlag = FALSE;
                } 
                else 
                {
                    transport_state = TRANSPORT_PLAYING;
                    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE, "PLAYING");
              
                    if(bHttpStalling == FALSE)
                        cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "Stop");
                    else
                        cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "Stop,Pause,X_DLNA_SeekTime,Seek");
                
                    cg_upnpav_dmr_avtransport_change_lastchange(dmr);
                }
                
            break;
            case TRANSPORT_STOPPED:
            
                if (output_play(dmr, 1) != 0) 
                {
                    cg_upnpav_dmr_set_error(action, CG_UPNPAV_TRANSPORT_E_PLAY_FORMAT_NS, "Playing failed");
                    bActionFlag = FALSE;
                } 
                else 
                {
                    transport_state = TRANSPORT_PLAYING;
                    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE, "PLAYING");   

                    if(bHttpStalling == FALSE)
                        cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "Stop");
                    else
                        cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "Stop,Pause");
               
                    cg_upnpav_dmr_avtransport_change_lastchange(dmr);

                }
                
            break;
            case TRANSPORT_PAUSED_PLAYBACK:
                if (output_playback() != 0) 
                {
                    cg_upnpav_dmr_set_error(action, CG_UPNPAV_TRANSPORT_E_PLAY_FORMAT_NS, "Playing failed");
                    bActionFlag = FALSE;
                } 
                else 
                {
                    transport_state = TRANSPORT_PLAYING;
                    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE, "PLAYING");
                    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "Stop,Pause");
                    
                    cg_upnpav_dmr_avtransport_change_lastchange(dmr);
                }               
            break;
            case TRANSPORT_NO_MEDIA_PRESENT:
            case TRANSPORT_TRANSITIONING:
            case TRANSPORT_PAUSED_RECORDING:
            case TRANSPORT_RECORDING:
               /* action not allowed in these states - error 701 */
                cg_upnpav_dmr_set_error(action, CG_UPNPAV_TRANSPORT_E_TRANSITION_NA,"Transition not allowed");
                bActionFlag = FALSE;
            break;
        }
        
        cg_drm_av_debug("dmr->transportMutex______________________",0);
        cg_mutex_unlock(dmr->transportMutex);
        
        return bActionFlag;
    }

    /*pause*/
    if(cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_PAUSE))
    {
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_INSTANCEID);
        if (!arg)
        {
            cg_upnpav_dmr_set_error(action, CG_UPNPAV_TRANSPORT_E_INVALID_IID,"Action:Stop invalid id");
            return FALSE;
        }

        #if 1
        /*if DMS don't support http-stalling, don't execute pause action*/
        if(bHttpStalling == FALSE)
        {
           return FALSE;
        }
        #endif

        cg_mutex_lock(dmr->transportMutex);     
        cg_drm_av_debug("dmr->transportMutex++++++++++++++++++++++++++",0);

        switch (transport_state) 
        {
            case TRANSPORT_PLAYING:
            case TRANSPORT_RECORDING:

                outputRet = output_pause();

                if (outputRet == -1) 
                {
                    cg_upnpav_dmr_set_error(action, CG_UPNPAV_TRANSPORT_E_PLAY_FORMAT_NS, "Playing failed");
                    bActionFlag = FALSE;
                } 
                else if(outputRet == 0)
                {
                    cg_upnpav_dmr_avtransport_set_transportstate(TRANSPORT_PAUSED_PLAYBACK);
                    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE, "PAUSED_PLAYBACK");   
                    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "Stop,Play");

                    cg_upnpav_dmr_avtransport_change_lastchange(dmr);
                }
                else if(outputRet == -2)
                {
                    cg_upnpav_dmr_avtransport_set_transportstate(TRANSPORT_STOPPED);
                    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE, "STOPPED");         
                    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "Play,X_DLNA_SeekTime,Seek");
              
                    cg_upnpav_dmr_avtransport_change_lastchange(dmr);
                }

            break;
                 
            case TRANSPORT_STOPPED:
            case TRANSPORT_PAUSED_PLAYBACK:
            case TRANSPORT_NO_MEDIA_PRESENT:
            case TRANSPORT_TRANSITIONING:
            case TRANSPORT_PAUSED_RECORDING:        
               /* action not allowed in these states - error 701 */
                cg_upnpav_dmr_set_error(action, CG_UPNPAV_TRANSPORT_E_TRANSITION_NA,"Transition not allowed");
                bActionFlag = FALSE;
            break;
        }
        
        cg_drm_av_debug("dmr->transportMutex____________________",0);
        cg_mutex_unlock(dmr->transportMutex);
        return bActionFlag;
    }
    
    /*Seek*/
    if(cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_SEEK))
    {
        bActionFlag = FALSE;
        
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_INSTANCEID);
        if (!arg)
        {
            cg_drm_av_debug("Action:Stop invalid id", CG_UPNPAV_TRANSPORT_E_INVALID_IID);
            return FALSE;
        }

        percentage = 0.0;
        currentMediaduration = 0;
        seekTime = 0;

        cg_mutex_lock(dmr->transportMutex);     
        cg_drm_av_debug("dmr->transportMutex++++++++++++++++++++++++++",0);
        
        switch (transport_state) 
        {
            case TRANSPORT_PLAYING:
            case TRANSPORT_PAUSED_PLAYBACK:  //this action should not be here,just for test
            
                arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_UNIT);
                if (!arg)
                {
                    cg_drm_av_debug("Action:Stop invalid id", CG_UPNPAV_TRANSPORT_E_SEEKMODE_NS);
                    bActionFlag = FALSE;
                    break;
                }

                if(cg_streq(arg->value->value, "TRACK_NR"))
                {
                    arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_TARGET);
                    if (!arg)
                    {
                        cg_drm_av_debug("Seek Target error!", CG_UPNPAV_TRANSPORT_E_ILL_SEEKTARGET);
                        bActionFlag = FALSE;
                        break;
                    }
                    printf("TRACK_NR = %s\n", arg->value->value);

                    if(output_stop() != 0)
                    {
                        cg_drm_av_debug("stop fail", 0);
                        bActionFlag = FALSE;
                        break;
                    }
                    
                    if(cg_streq(arg->value->value, "0"))
                    {
                        bActionFlag = TRUE;
                        break;
                    }
                    
                    if( output_play(dmr, cg_atoi(arg->value->value)) != 0)
                    {
                       cg_drm_av_debug("play fail", 0);
                       bActionFlag = FALSE;
                       break;
                    }
                    
                    bActionFlag = TRUE;
                    break;
                }
                
                if(cg_streq(arg->value->value, "REL_TIME"))
                {
                    #if 1
                    arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_TARGET);
                    if (!arg)
                    {
                        cg_drm_av_debug("Seek Target error!", CG_UPNPAV_TRANSPORT_E_ILL_SEEKTARGET);
                        bActionFlag = FALSE;
                        break;
                    }
                    printf("seek time = %s\n", arg->value->value);

                    /*tranform REL_TIME from string to int*/
                    seekTime = cg_time_to_int(arg->value->value);
                    currentMediaduration = cg_time_to_int(cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTMEDIADURATION));
        
                    if( (seekTime<=currentMediaduration) && (currentMediaduration>0))
                    {
                        percentage = (double)seekTime / currentMediaduration;                                                                
                        output_seek(percentage);
                        
                        cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_RELATIVETIMEPOSITION, 
                                                            arg->value->value);
                    }
                    else
                    {
                        cg_drm_av_debug("seekTime error", 0);
                        cg_upnp_action_setstatuscode(action, CG_UPNPAV_TRANSPORT_E_ILL_SEEKTARGET);
                        bActionFlag = FALSE;
                        break;
                    }

                    bActionFlag = TRUE;
                    break;
                    #endif
                    //cg_upnp_action_setstatuscode(action, CG_UPNPAV_TRANSPORT_E_SEEKMODE_NS);
                    //bActionFlag = FALSE;
                    //break;
                }
                
                if(cg_streq(arg->value->value, "X_DLNA_REL_BYTE"))
                {
                    cg_upnp_action_setstatuscode(action, CG_UPNPAV_TRANSPORT_E_SEEKMODE_NS);
                    bActionFlag = FALSE;
                    break;
                }
            

                #if 0

                if(cg_streq(arg->value->value, "ABS_TIME"))
                {
                    arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_TARGET);
                    if (!arg)
                    {
                        cg_drm_av_debug("Seek Target error!", CG_UPNPAV_TRANSPORT_E_ILL_SEEKTARGET);
                        bActionFlag = FALSE;
                        break;
                    }
                    printf("seek time = %s\n", arg->value->value);

                    seekTime = cg_time_to_int(arg->value->value);
                    currentMediaduration = cg_time_to_int(cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTMEDIADURATION));
        
                    if( (seekTime<=currentMediaduration) && (currentMediaduration>0))
                    {
                        percentage = (double)seekTime / currentMediaduration;                                                                
                        output_seek(percentage);
                        
                        cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_ABSOLUTETIMEPOSITION, 
                                                            arg->value->value);
                    }
                    else
                    {
                        cg_drm_av_debug("seekTime error", 0);
                        bActionFlag = FALSE;
                        break;
                    }

                    bActionFlag = TRUE;
                    break;
                }

                
                #endif
           
            break;

            case TRANSPORT_STOPPED:
                arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_UNIT);
                if (!arg)
                {
                    cg_drm_av_debug("Action:Stop invalid id", CG_UPNPAV_TRANSPORT_E_SEEKMODE_NS);
                    bActionFlag = FALSE;
                    break;
                }

                if(cg_streq(arg->value->value, "TRACK_NR"))
                {
                    arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_TARGET);
                    if (!arg)
                    {
                        cg_drm_av_debug("Seek Target error!", CG_UPNPAV_TRANSPORT_E_ILL_SEEKTARGET);
                        bActionFlag = FALSE;
                        break;
                    }
                    printf("TRACK_NR = %s\n", arg->value->value);

                    if(output_stop() != 0)
                    {
                        cg_drm_av_debug("stop fail", 0);
                        bActionFlag = FALSE;
                        break;
                    }
                    
                    if(cg_streq(arg->value->value, "0"))
                    {
                        bActionFlag = TRUE;
                        break;
                    }
                    
                    if( output_play(dmr, cg_atoi(arg->value->value)) != 0)
                    {
                       cg_drm_av_debug("play fail", 0);
                       bActionFlag = FALSE;
                       break;
                    }
                    
                    bActionFlag = TRUE;
                    break;
                }


                if(cg_streq(arg->value->value, "X_DLNA_REL_BYTE"))
                {
                    cg_upnp_action_setstatuscode(action, CG_UPNPAV_TRANSPORT_E_SEEKMODE_NS);
                    bActionFlag = FALSE;
                    break;
                }

                if(cg_streq(arg->value->value, "REL_TIME"))
                {
                    cg_upnp_action_setstatuscode(action, CG_UPNPAV_TRANSPORT_E_SEEKMODE_NS);
                    bActionFlag = FALSE;
                    break;
                }
                
            break;

            
            case TRANSPORT_NO_MEDIA_PRESENT:
            case TRANSPORT_TRANSITIONING:
            case TRANSPORT_PAUSED_RECORDING:
            case TRANSPORT_RECORDING:
               /* action not allowed in these states - error 701 */
                cg_upnpav_dmr_set_error(action, CG_UPNPAV_TRANSPORT_E_TRANSITION_NA,"Transition not allowed");
                bActionFlag = FALSE;
            break;

            default:
                printf("switch case lose default, fuck \n");
                break;
        }
        
        cg_drm_av_debug("dmr->transportMutex________________________",0);
        cg_mutex_unlock(dmr->transportMutex);
        return bActionFlag;
    }

    /* Next */
    if(cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_NEXT))
    {
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_INSTANCEID);
        if (!arg)
        {
            cg_drm_av_debug("Action:Next invalid id", CG_UPNPAV_TRANSPORT_E_INVALID_IID);
            return FALSE;
        }

        cg_mutex_lock(dmr->transportMutex);

        switch (transport_state) 
        {
            case TRANSPORT_PLAYING:
            case TRANSPORT_STOPPED:

               icurrentTrackNum = cg_atoi( cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACK));
               if(icurrentTrackNum >= cg_upnpav_trackinfolist_size(dmr->trackInfoList))
               {
                   cg_drm_av_debug("index num too large!!!!!!!!!!!!!\n", 0);
                   bActionFlag = TRUE;
                   break;
               }
               icurrentTrackNum ++;
               sprintf(strcurrentTrackNum, "%d", icurrentTrackNum);
               cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACK, strcurrentTrackNum);
               
               if(output_stop() != 0)
               {
                   cg_drm_av_debug("stop fail", 0);
                   bActionFlag = FALSE;
                   break;
               }
               if( output_play(dmr, icurrentTrackNum) != 0)
               {
                   cg_drm_av_debug("play fail", 0);
                   bActionFlag = FALSE;
                   break;
               }

               break;

            case TRANSPORT_PAUSED_PLAYBACK:
            case TRANSPORT_NO_MEDIA_PRESENT:
            case TRANSPORT_TRANSITIONING:
            case TRANSPORT_PAUSED_RECORDING:
            case TRANSPORT_RECORDING:
               /* action not allowed in these states - error 701 */
                cg_upnpav_dmr_set_error(action, CG_UPNPAV_TRANSPORT_E_TRANSITION_NA,"Transition not allowed");
                bActionFlag = FALSE;
            break;
        }

        cg_mutex_unlock(dmr->transportMutex);
        return bActionFlag;
    }
    
    /* Previous */
    if(cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_PREVIOUS))
    {
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_INSTANCEID);
        if (!arg)
        {
            cg_upnpav_dmr_set_error(action, CG_UPNPAV_TRANSPORT_E_INVALID_IID,"Action:Previous invalid id");
            return FALSE;
        }

        cg_mutex_lock(dmr->transportMutex);
        
        switch (transport_state) 
        {
            case TRANSPORT_PLAYING:
            case TRANSPORT_STOPPED:

               icurrentTrackNum = cg_atoi( cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACK));
               if(icurrentTrackNum <= 1)
               {
                   printf("index num too small!!!!!!!!!!!!!\n");
                   bActionFlag = TRUE;
                   break;
               }
               
               icurrentTrackNum --;
               sprintf(strcurrentTrackNum, "%d", icurrentTrackNum);
               cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACK, strcurrentTrackNum);
               
               if(output_stop() != 0)
               {
                   cg_drm_av_debug("stop fail", 0);
                   bActionFlag = FALSE;
                   break;
               }
               if( output_play(dmr, icurrentTrackNum) != 0)
               {
                   cg_drm_av_debug("play fail", 0);
                   bActionFlag = FALSE;
                   break;
               }

               break;

            case TRANSPORT_PAUSED_PLAYBACK:
            case TRANSPORT_NO_MEDIA_PRESENT:
            case TRANSPORT_TRANSITIONING:
            case TRANSPORT_PAUSED_RECORDING:
            case TRANSPORT_RECORDING:
               /* action not allowed in these states - error 701 */
                cg_upnpav_dmr_set_error(action, CG_UPNPAV_TRANSPORT_E_TRANSITION_NA,"Transition not allowed");
                bActionFlag = FALSE;
            break;
        }
        
        
        cg_mutex_unlock(dmr->transportMutex);
        return bActionFlag;
    }

    /*CurrentTransportActions*/
    if(cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_GETCURRENTTRANSPORTACTIONS))
    {
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_INSTANCEID);
        if (!arg)
        {
            cg_upnpav_dmr_set_error(action, CG_UPNPAV_TRANSPORT_E_INVALID_IID,"Action:Previous invalid id");
            return FALSE;
        }

        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_ACTIONS);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS));

        return TRUE;
    }

    /*X_GetControlPoint*/
    /*私有的，非强制*/
    if(cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_X_GETCONTROLPOINT))
    {
        return cg_upnpav_dmr_avtransport_GetControlPoint(dmr, dev, service, action);
    }   

    /*X_SetControlPoint*/
    /*私有的，非强制*/
    if(cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_X_SETCONTROLPOINT))
    {
        return cg_upnpav_dmr_avtransport_SetControlPoint(dmr, dev, service, action);
    }   

    /*X_GetWifiIntes*/    
    /*私有的，非强制*/    
    if(cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_X_GETWIFIINTES))    
    {        
        return cg_upnpav_dmr_avtransport_GetWifiIntes(dmr, dev, service, action);    
    } 

    /*PlayPrepare*/
    if(cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_PLAYPREPARE))    
    {        
        return cg_upnpav_dmr_avtransport_playprepare(dmr, dev, service, action);    
    } 
    


}

/****************************************
 * cg_upnpav_dmr_avtransport_queryreceived
 ****************************************/

BOOL cg_upnpav_dmr_avtransport_queryreceived(CgUpnpStateVariable *statVar)
{
    return FALSE;
}

/****************************************
* cg_upnpav_dmr_avtransport_init
****************************************/

BOOL cg_upnpav_dmr_avtransport_init(CgUpnpAvRenderer *dmr)
{
    CgUpnpDevice *dev;
    CgUpnpService *service;
    CgUpnpAction *action;

    dev = cg_upnpav_dmr_getdevice(dmr);
    if (!dev)
        return FALSE;

    service = cg_upnp_device_getservicebytype(dev, CG_UPNPAV_DMR_AVTRANSPORT_SERVICE_TYPE);
    if (!service)
        return FALSE;

    if(cg_upnp_service_loaddescriptionfile(service, CG_UPNPAV_DMR_AVTRANSPORT_SERVICE_DESCRIPTION) == FALSE)
    {
        printf("parase cavtransport error !!! \n");
        return FALSE;
    }

    #if 0
    if (cg_upnp_service_parsedescription(service, CG_UPNPAV_DMR_AVTRANSPORT_SERVICE_DESCRIPTION, cg_strlen(CG_UPNPAV_DMR_AVTRANSPORT_SERVICE_DESCRIPTION)) == FALSE)
        return FALSE;
    #endif
    
    cg_upnp_service_setuserdata(service, dmr);
    for (action=cg_upnp_service_getactions(service); action; action=cg_upnp_action_next(action))
        cg_upnp_action_setuserdata(action, dmr);

    /*variables init*/
    bHttpStalling = TRUE;
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE, "NO_MEDIA_PRESENT");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_PLAYBACKSTORAGEMEDIUM, "NONE");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_RECORDSTORAGEMEDIUM, "NOT_IMPLEMENTED");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_POSSIBLEPLAYBACKSTORAGEMEDIA, "NONE,NETWORK");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_POSSIBLERECORDSTORAGEMEDIA, "NOT_IMPLEMENTED");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTPALYMODE, "NORMAL");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTPLAYSTATE, "1");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_RECORDMEDIUMWRITESTATUS, "NOT_IMPLEMENTED");  
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTRECORDQUALITYMODE, "NOT_IMPLEMENTED");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_POSSIBLERECORDQUALITYMODES, "NOT_IMPLEMENTED");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_NUMBEROFTRACKS, "0");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACK, "0");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKDURATION, "00:00:00");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTMEDIADURATION, "00:00:00");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKMETADATA, "NOT_IMPLEMENTED");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKURI, "");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_AVTRANSPORTURI, "");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_AVTRANSPORTURIMETADATA, "");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_NEXTAVTRANSPORTURI, "NOT_IMPLEMENTED");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_NEXTAVTRANSPORTURIMETADATA, "NOT_IMPLEMENTED");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_RELATIVETIMEPOSITION, "00:00:00");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_ABSOLUTETIMEPOSITION, "00:00:00");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_RELATIVECOUNTERPOSITION, "2147483647");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_ABSOLUTECOUNTERPOSITION, "2147483647");      
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_AAT_INSTANCEID, "0");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "");

    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_UDPSESSION, "111");
    
    cg_upnpav_dmr_avtransport_change_lastchange(dmr);

    dmr->transportMutex = cg_mutex_new();
    if (!dmr->transportMutex) 
    {
        cg_drm_av_debug("can't malloc memory for transportMutex", 0);
        return FALSE;
    }
    
    return TRUE;
}

/****************************************
 * cg_upnpav_dmr_setavtransportlastchange
 ****************************************/

void cg_upnpav_dmr_setavtransportlastchange(CgUpnpAvRenderer *dmr, char *value)
{
    CgUpnpService *service;
    CgUpnpStateVariable *stateVar;

    service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_AVTRANSPORT_SERVICE_TYPE);
    stateVar = cg_upnp_service_getstatevariablebyname(service, CG_UPNPAV_DMR_AVTRANSPORT_LASTCHANGE);
    cg_upnp_statevariable_setvalue(stateVar, value);
}

/****************************************
 * cg_upnpav_dmr_getavtransportlastchange
 ****************************************/

char *cg_upnpav_dmr_getavtransportlastchange(CgUpnpAvRenderer *dmr)
{
    CgUpnpService *service;
    CgUpnpStateVariable *stateVar;

    service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_AVTRANSPORT_SERVICE_TYPE);
    stateVar = cg_upnp_service_getstatevariablebyname(service, CG_UPNPAV_DMR_AVTRANSPORT_LASTCHANGE);
    return cg_upnp_statevariable_getvalue(stateVar);
}


/****************************************
 * cg_upnpav_dmr_avtransport_change_var
 ****************************************/
void cg_upnpav_dmr_avtransport_change_var(CgUpnpAvRenderer *dmr, char *stateVariable, char *newValue)
{
    char *buf = NULL;
    char *xmlbuf = NULL;

    if(stateVariable == NULL)
    {
        printf("stateVariable is NULL_________");
        return;
    }

    if(newValue == NULL) 
    {
        return;
    }

 //   xmlbuf = xmlescape(newValue, (int)1);
    
    asprintf(&buf,
         "<Event xmlns = \"urn:schemas-upnp-org:metadata-1-0/AVT/\"><InstanceID val=\"0\"><%s val=\"%s\"/></InstanceID></Event>",
         stateVariable, newValue);
    
    cg_upnpav_dmr_setavtransportlastchange(dmr, buf);

//  free(xmlbuf);
    free(buf);
    
    return;
}


/****************************************
 * cg_upnpav_dmr_avtransport_setstatevariable
 ****************************************/
void cg_upnpav_dmr_avtransport_setstatevariable(CgUpnpAvRenderer *dmr,char *statevariable, char *value)
{
    CgUpnpService *service;
    CgUpnpStateVariable *stateVar;

    service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_AVTRANSPORT_SERVICE_TYPE);
    stateVar = cg_upnp_service_getstatevariablebyname(service, statevariable);
    cg_upnp_statevariable_setvalue(stateVar, value);
}

/****************************************
 * cg_upnpav_dmr_avtransport_getstatevariable
 ****************************************/
char *cg_upnpav_dmr_avtransport_getstatevariable(CgUpnpAvRenderer *dmr, char *statevariable)
{
    CgUpnpService *service;
    CgUpnpStateVariable *stateVar;

    service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_AVTRANSPORT_SERVICE_TYPE);
    stateVar = cg_upnp_service_getstatevariablebyname(service, statevariable);
    return cg_upnp_statevariable_getvalue(stateVar);
}


/****************************************
 * cg_upnpav_dmr_avtransport_change_lastchange
 ****************************************/
void cg_upnpav_dmr_avtransport_change_lastchange(CgUpnpAvRenderer *dmr)
{
    char *lastChange = NULL;

    printf(
    "<Event xmlns = \"urn:schemas-upnp-org:metadata-1-0/AVT/\">\
<InstanceID val=\"0\">\
<PlaybackStorageMedium val=\"NETWORK\"/>\
<TransportState val=\"%s\"/>\
<CurrentTransportActions val=\"%s\"/>\
<NumberOfTracks val=\"%s\"/>\
<AVTransportURI val=\"%s\"/>\
<CurrentTrackURI val=\"%s\"/>\
<AVTransportURIMetaData val=\"%s\"/>\
<CurrentMediaDuration val=\"%s\"/>\
<CurrentPlayMode val=\"NORMAL\"/>\
<CurrentRecordQualityMode val=\"NOT_IMPLEMENTED\"/>\
<CurrentTrack val=\"%s\"/>\
<CurrentTrackDuration val=\"%s\"/>\
<CurrentTrackMetaData val=\"%s\"/>\
<X_CurrentControlPoint val=\"%s\"/>\
<NextAVTransportURI val=\"NOT_IMPLEMENTED\"/>\
<NextAVTransportURIMetaData val=\"NOT_IMPLEMENTED\"/>\
<PossiblePlaybackStorageMedia val=\"NETWORK\"/>\
<PossibleRecordQualityModes val=\"NOT_IMPLEMENTED\"/>\
<PossibleRecordStorageMedia val=\"NOT_IMPLEMENTED\"/>\
<RecordMediumWriteStatus val=\"NOT_IMPLEMENTED\"/>\
<RecordStorageMedium val=\"NOT_IMPLEMENTED\"/>\
<TransportPlaySpeed val=\"1\"/>\
<TransportStatus val=\"OK\"/>\
</InstanceID>\
</Event>",
     cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE),
     cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS),
     cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_NUMBEROFTRACKS),
     cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_AVTRANSPORTURI),
     cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKURI), 
     cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_AVTRANSPORTURIMETADATA),
     cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTMEDIADURATION),
     cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACK),
     cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKDURATION),
     cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKMETADATA),
     cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTCONTROLPOINT)
     );
   

    printf("before lastchange = \n");

    cg_upnpav_dmr_setavtransportlastchange(dmr, lastChange);

    
    asprintf(&lastChange,
    "<Event xmlns = \"urn:schemas-upnp-org:metadata-1-0/AVT/\">\
<InstanceID val=\"0\">\
<PlaybackStorageMedium val=\"NETWORK\"/>\
<TransportState val=\"%s\"/>\
<CurrentTransportActions val=\"%s\"/>\
<NumberOfTracks val=\"%s\"/>\
<AVTransportURI val=\"%s\"/>\
<CurrentTrackURI val=\"%s\"/>\
<AVTransportURIMetaData val=\"%s\"/>\
<CurrentMediaDuration val=\"%s\"/>\
<CurrentPlayMode val=\"NORMAL\"/>\
<CurrentRecordQualityMode val=\"NOT_IMPLEMENTED\"/>\
<CurrentTrack val=\"%s\"/>\
<CurrentTrackDuration val=\"%s\"/>\
<CurrentTrackMetaData val=\"%s\"/>\
<X_CurrentControlPoint val=\"%s\"/>\
<NextAVTransportURI val=\"NOT_IMPLEMENTED\"/>\
<NextAVTransportURIMetaData val=\"NOT_IMPLEMENTED\"/>\
<PossiblePlaybackStorageMedia val=\"NETWORK\"/>\
<PossibleRecordQualityModes val=\"NOT_IMPLEMENTED\"/>\
<PossibleRecordStorageMedia val=\"NOT_IMPLEMENTED\"/>\
<RecordMediumWriteStatus val=\"NOT_IMPLEMENTED\"/>\
<RecordStorageMedium val=\"NOT_IMPLEMENTED\"/>\
<TransportPlaySpeed val=\"1\"/>\
<TransportStatus val=\"OK\"/>\
</InstanceID>\
</Event>",
     cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE),
     cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS),
     cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_NUMBEROFTRACKS),
     cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_AVTRANSPORTURI),
     cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKURI),
     cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_AVTRANSPORTURIMETADATA),
     cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTMEDIADURATION),
     cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACK),
     cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKDURATION),
     cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKMETADATA),
     cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTCONTROLPOINT)
     );
   

    printf("before lastchange = \n");

    cg_upnpav_dmr_setavtransportlastchange(dmr, lastChange);

    printf("after lastchange = \n");
    free(lastChange);
}

/****************************************
 * cg_upnpav_dmr_avtransport_variable_init
 ****************************************/
void cg_upnpav_dmr_avtransport_variable_init(CgUpnpAvRenderer *dmr)
{
    bHttpStalling = TRUE;
    cg_upnpav_dmr_avtransport_set_transportstate(TRANSPORT_NO_MEDIA_PRESENT);
    
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE, "NO_MEDIA_PRESENT");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_PLAYBACKSTORAGEMEDIUM, "NONE");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_RECORDSTORAGEMEDIUM, "NOT_IMPLEMENTED");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_POSSIBLEPLAYBACKSTORAGEMEDIA, "NONE,NETWORK");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_POSSIBLERECORDSTORAGEMEDIA, "NOT_IMPLEMENTED");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTPALYMODE, "NORMAL");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTPLAYSTATE, "1");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_RECORDMEDIUMWRITESTATUS, "NOT_IMPLEMENTED");  
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTRECORDQUALITYMODE, "NOT_IMPLEMENTED");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_POSSIBLERECORDQUALITYMODES, "NOT_IMPLEMENTED");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_NUMBEROFTRACKS, "0");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACK, "0");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKDURATION, "00:00:00");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTMEDIADURATION, "00:00:00");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKMETADATA, "NOT_IMPLEMENTED");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKURI, "");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_AVTRANSPORTURI, "");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_AVTRANSPORTURIMETADATA, "");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_NEXTAVTRANSPORTURI, "NOT_IMPLEMENTED");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_NEXTAVTRANSPORTURIMETADATA, "NOT_IMPLEMENTED");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_RELATIVETIMEPOSITION, "00:00:00");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_ABSOLUTETIMEPOSITION, "00:00:00");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_RELATIVECOUNTERPOSITION, "2147483647");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_ABSOLUTECOUNTERPOSITION, "2147483647");      
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_AAT_INSTANCEID, "0");
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "");

    cg_upnpav_dmr_avtransport_change_lastchange(dmr);
}

/****************************************
* cg_upnpav_dmr_avtransport_setmulticasturi
****************************************/
BOOL cg_upnpav_dmr_avtransport_setmulticasturi_test(CgUpnpAvRenderer *dmr, char* mulUri)
{
    CgUpnpArgument *arg;
    CgUpnpAvTrackInfo *trackInfo;
    char multicastIp[20];
    char multicastPort[10];
    char* lastMulticastIp;
    char* lastMulticastPort;
        

    /* parse multicastUri */
    if( cg_parse_multicastUri(mulUri, multicastIp, multicastPort) != 0 )
    {
        printf("parse multicastUri error\n");
        return FALSE;
    }


    if(dmr->uriMode == UNICAST && buniPipeline == TRUE)
    {
        output_close_playbinloop();
        buniPipeline = FALSE;
        dmr->uriMode = MULTICAST;
        output_gstreamer_rtpbin_init(dmr, multicastIp, multicastPort);
        bmultiPipeline = TRUE;
    }
    else
    {
        dmr->uriMode = MULTICAST;
        /*check whether the pipeline is setted up*/
        if(bmultiPipeline == FALSE)
        {
            bmultiPipeline = TRUE;
            output_gstreamer_rtpbin_init(dmr, multicastIp, multicastPort);
            
        }
        else
        {
            lastMulticastIp = cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_MULTICASTIP );
            lastMulticastPort = cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_MULTICASTPORT );

            if(cg_streq(lastMulticastIp, multicastIp) && cg_streq(lastMulticastPort, multicastPort))
            {
            
            }
            else
            {
                output_close_rtploop();
                usleep(1000*10);
                output_gstreamer_rtpbin_init(dmr, multicastIp, multicastPort);
            }
        }
    }
    #if 1
    /* stop playing */
    cg_mutex_lock(dmr->transportMutex);

    switch (transport_state) 
    {
        case TRANSPORT_STOPPED:
        break;
        
        case TRANSPORT_NO_MEDIA_PRESENT:
            printf("set transport to stop\n");
            cg_upnpav_dmr_avtransport_set_transportstate(TRANSPORT_STOPPED);
            cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE, "STOPPED");        
            cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "Play");
                
        break;
        
        case TRANSPORT_PLAYING:
        case TRANSPORT_TRANSITIONING:
        case TRANSPORT_PAUSED_RECORDING:
        case TRANSPORT_RECORDING:
        case TRANSPORT_PAUSED_PLAYBACK:
            if(output_stop()!=0)
            {
                printf("stop fail\n");
                
            }
            else
            {
                printf("stop  success\n");
                cg_upnpav_dmr_avtransport_set_transportstate(TRANSPORT_STOPPED);
                cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE, "STOPPED");        
                cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "Play");
              
            }
           
        break;
    }

    cg_mutex_unlock(dmr->transportMutex);

    #endif

    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_MULTICASTIP, multicastIp );
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_MULTICASTPORT, multicastPort );
    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_AVTRANSPORTURI, mulUri );

    return TRUE;
    
}

/****************************************
* cg_upnpav_dmr_avtransport_play
****************************************/
BOOL cg_upnpav_dmr_avtransport_play(CgUpnpAvRenderer *dmr)
{
    BOOL bActionFlag = TRUE;

    if(dmr == NULL)
    {
        return FALSE;
    }

    cg_mutex_lock(dmr->transportMutex);

    switch (transport_state) 
    {
        case TRANSPORT_PLAYING:
                
            if (output_play(dmr, 1) != 0) 
            {           
                bActionFlag = FALSE;
            } 
            else 
            {
                transport_state = TRANSPORT_PLAYING;
                cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE, "PLAYING");
              
                if(bHttpStalling == FALSE)
                    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "Stop");
                else
                    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "Stop,Pause");
                
                cg_upnpav_dmr_avtransport_change_lastchange(dmr);
            }
                
        break;
        
        case TRANSPORT_STOPPED:
            
            if (output_play(dmr, 1) != 0) 
            {
                bActionFlag = FALSE;
            } 
            else 
            {
                transport_state = TRANSPORT_PLAYING;
                cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE, "PLAYING");   

                if(bHttpStalling == FALSE)
                    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "Stop");
                else
                    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "Stop,Pause");
               
                cg_upnpav_dmr_avtransport_change_lastchange(dmr);

            }
                
        break;
        
        case TRANSPORT_PAUSED_PLAYBACK:
            if (output_playback() != 0) 
                {
                    
                    bActionFlag = FALSE;
                } 
                else 
                {
                    transport_state = TRANSPORT_PLAYING;
                    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE, "PLAYING");
                    cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "Stop,Pause");
                    
                    cg_upnpav_dmr_avtransport_change_lastchange(dmr);
                }               
            break;
            case TRANSPORT_NO_MEDIA_PRESENT:
            case TRANSPORT_TRANSITIONING:
            case TRANSPORT_PAUSED_RECORDING:
            case TRANSPORT_RECORDING:
               /* action not allowed in these states - error 701 */
                
            break;
    }

    cg_mutex_unlock(dmr->transportMutex);
    return bActionFlag;
    
}

/****************************************
* cg_upnpav_dmr_avtransport_pause
****************************************/
BOOL cg_upnpav_dmr_avtransport_pause(CgUpnpAvRenderer *dmr)
{
    BOOL bActionFlag = TRUE;
    int outputRet;

    if(dmr == NULL)
    {
        return FALSE;
    }

    cg_mutex_lock(dmr->transportMutex);     
    cg_drm_av_debug("dmr->transportMutex++++++++++++++++++++++++++",0);

    switch (transport_state) 
    {
        case TRANSPORT_PLAYING:
        case TRANSPORT_RECORDING:

            outputRet = output_pause();

            if (outputRet == -1) 
            {
                cg_drm_av_debug("output_pause fail.......................\n", -1);
                bActionFlag = FALSE;
            } 
            else if(outputRet == 0)
            {
                cg_upnpav_dmr_avtransport_set_transportstate(TRANSPORT_PAUSED_PLAYBACK);
                cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE, "PAUSED_PLAYBACK");   
                cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "Stop,Play");

                cg_upnpav_dmr_avtransport_change_lastchange(dmr);
            }
            else if(outputRet == -2)
            {
                cg_upnpav_dmr_avtransport_set_transportstate(TRANSPORT_STOPPED);
                cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE, "STOPPED");         
                cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "Play,X_DLNA_SeekTime,Seek");
              
                cg_upnpav_dmr_avtransport_change_lastchange(dmr);
            }

        break;
                 
        case TRANSPORT_STOPPED:
        case TRANSPORT_PAUSED_PLAYBACK:
        case TRANSPORT_NO_MEDIA_PRESENT:
        case TRANSPORT_TRANSITIONING:
        case TRANSPORT_PAUSED_RECORDING:        
               /* action not allowed in these states - error 701 */
              cg_drm_av_debug("action not allow\n",701);
              bActionFlag = FALSE;
        break;
    }
        
    cg_drm_av_debug("dmr->transportMutex____________________",0);
    cg_mutex_unlock(dmr->transportMutex);
    return bActionFlag;
    
}


int tim_subtract(struct timeval *result, struct timeval *x, struct timeval *y)
{
    int nsec;   
    if(x->tv_sec > y->tv_sec)
        return   -1;
    if((x->tv_sec==y->tv_sec) && (x->tv_usec>y->tv_usec))
        return  -1;

    result->tv_sec = (y->tv_sec - x->tv_sec);
    result->tv_usec = (y->tv_usec - x->tv_usec);

    if(result->tv_usec<0)
    {
        result->tv_sec--;
        result->tv_usec+=1000000;
    }

    return   0;

}


/****************************************
* cg_upnpav_dmr_avtransport_seek
****************************************/
BOOL cg_upnpav_dmr_avtransport_seek(CgUpnpAvRenderer *dmr, char* rel_time)
{
    BOOL bActionFlag = TRUE;
    double percentage = 0.0; 
    int currentMediaduration = 0;
    int seekTime = 0;
    char *seekResponse = NULL;


    struct timeval  start,stop,diff;

    if(dmr == NULL || rel_time == NULL)
        return FALSE;

    cg_mutex_lock(dmr->transportMutex);

    switch (transport_state) 
    {
        case TRANSPORT_PLAYING:

            /*pause */
            if (output_pause() != 0) 
            {
                cg_drm_av_debug("pause fail!!!!!!!!!!!!!!!!!", 1);
                bActionFlag = FALSE;
                break;
            } 
            else 
            {
                transport_state = TRANSPORT_PAUSED_PLAYBACK;
                cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE, "PAUSED_PLAYBACK");   
                cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "Stop,Play");
            
                cg_upnpav_dmr_avtransport_change_lastchange(dmr);
            }


            /*seek*/
            seekTime = cg_time_to_int(rel_time);
            currentMediaduration = cg_time_to_int(cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTMEDIADURATION));
        
            if( (seekTime<=currentMediaduration) && (currentMediaduration>0))
            {
                percentage = (double)seekTime / currentMediaduration;
                gettimeofday(&start,0);
                output_seek(percentage);
                gettimeofday(&stop,0);

                tim_subtract(&diff,&start,&stop);
                printf("seek diff: %d \n",diff.tv_usec);
                cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_RELATIVETIMEPOSITION, 
                                                            rel_time);
            }
            else
            {
                cg_drm_av_debug("seekTime error", 0);
                bActionFlag = FALSE;
                break;
            }
        break;
            
        case TRANSPORT_PAUSED_PLAYBACK:
            seekTime = cg_time_to_int(rel_time);
            currentMediaduration = cg_time_to_int(cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTMEDIADURATION));
        
            if( (seekTime<=currentMediaduration) && (currentMediaduration>0))
            {
                percentage = (double)seekTime / currentMediaduration;
                gettimeofday(&start,0);
                output_seek(percentage);
                gettimeofday(&stop,0);

                tim_subtract(&diff,&start,&stop);
                printf("seek diff: %d \n",diff.tv_usec);
                cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_RELATIVETIMEPOSITION, 
                                                            rel_time);
            }
            else
            {
                cg_drm_av_debug("seekTime error", 0);
                bActionFlag = FALSE;
                break;
            }

        break;
        
        case TRANSPORT_STOPPED:     
        
        case TRANSPORT_NO_MEDIA_PRESENT:
        case TRANSPORT_TRANSITIONING:
        case TRANSPORT_PAUSED_RECORDING:
        case TRANSPORT_RECORDING:
               /* action not allowed in these states - error 701 */
                
        break;
    }

    cg_mutex_unlock(dmr->transportMutex);

    if(bActionFlag)
    {
       // printf("Set UdpSeekResponse value...................\n");
        asprintf(&seekResponse, "ACK:%s", cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_UDPSESSION));
        cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_UDPSEEKRESPONSE, seekResponse);
        free(seekResponse);
    }
    return bActionFlag;
    
}
/***************************************
* cg_upnpav_dmr_avtransport_test
****************************************/
BOOL cg_upnpav_dmr_avtransport_test(CgUpnpAvRenderer *dmr)
{
    BOOL bActionFlag = TRUE;
    char *seekResponse = NULL;
    
    cg_mutex_lock(dmr->transportMutex);

    
    if (output_udp_pause(dmr) != 0) 
    {
        cg_drm_av_debug("pause fail!!!", 1);
        bActionFlag = FALSE;
    } 
    else 
    {
        transport_state = TRANSPORT_PAUSED_PLAYBACK;
        cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE, "PAUSED_PLAYBACK");   
        cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "Stop,Play");

        cg_upnpav_dmr_avtransport_change_lastchange(dmr);
    }

    cg_mutex_unlock(dmr->transportMutex);

    if(bActionFlag)
    {
        asprintf(&seekResponse, "ACK:%s", cg_upnpav_dmr_avtransport_getstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_UDPSESSION));
        cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_UDPSEEKRESPONSE, seekResponse);
        free(seekResponse);
    }
    
    return bActionFlag;
}


#if 0
/****************************************
* cg_upnpav_dmr_sudp_play_receive
****************************************/
BOOL cg_upnpav_dmr_sudp_play_receive(CgUpnpSUDPPacket *sudpPkt)
{
    CgUpnpAvRenderer *dmr;
    CgUpnpDevice *dev;

    dev = (CgUpnpDevice *)cg_upnp_sudp_packet_getuserdata(sudpPkt);
    if (!dev)
    {
        cg_drm_av_debug("dev is null", 1);
        return FALSE;
    }
    dmr = (CgUpnpAvRenderer *)cg_upnp_device_getuserdata(dev);
    if (!dmr)
    {
        cg_drm_av_debug("dmr is null", 1);
        return FALSE;
    }
        

    return cg_upnpav_dmr_avtransport_play(dmr);
}

/****************************************
* cg_upnpav_dmr_sudp_seek_receive
****************************************/
BOOL cg_upnpav_dmr_sudp_seek_receive(CgUpnpSUDPPacket *sudpPkt)
{
    CgUpnpAvRenderer *dmr;
    CgUpnpDevice *dev;
    char *rel_time;

    dev = (CgUpnpDevice *)cg_upnp_sudp_packet_getuserdata(sudpPkt);
    if (!dev)
        return FALSE;

    dmr = (CgUpnpAvRenderer *)cg_upnp_device_getuserdata(dev);
    if (!dmr)
        return FALSE;

    rel_time = cg_upnp_sudp_packet_getreltime(sudpPkt);
    if(rel_time == NULL)
    {
        cg_drm_av_debug("sudp REL_TIME contain nothing", 1);
        return FALSE;
    }
    
    return cg_upnpav_dmr_avtransport_seek(dmr, rel_time);
}


/****************************************
* cg_upnpav_dmr_sudp_test_receive
****************************************/
BOOL cg_upnpav_dmr_sudp_test_receive(CgUpnpSUDPPacket *sudpPkt)
{
    CgUpnpAvRenderer *dmr;
    CgUpnpDevice *dev;
    char *rel_time;

    dev = (CgUpnpDevice *)cg_upnp_sudp_packet_getuserdata(sudpPkt);
    if (!dev)
        return FALSE;

    dmr = (CgUpnpAvRenderer *)cg_upnp_device_getuserdata(dev);
    if (!dmr)
        return FALSE;

    
    return cg_upnpav_dmr_avtransport_test(dmr);
}


/****************************************
* cg_upnpav_dmr_sudp_pause_receive
****************************************/
BOOL cg_upnpav_dmr_sudp_pause_receive(CgUpnpSUDPPacket *sudpPkt)
{
    CgUpnpAvRenderer *dmr;
    CgUpnpDevice *dev;

    dev = (CgUpnpDevice *)cg_upnp_sudp_packet_getuserdata(sudpPkt);
    if (!dev)
    {
        cg_drm_av_debug("dev is null", 1);
        return FALSE;
    }
    dmr = (CgUpnpAvRenderer *)cg_upnp_device_getuserdata(dev);
    if (!dmr)
    {
        cg_drm_av_debug("dmr is null", 1);
        return FALSE;
    }
        

    return cg_upnpav_dmr_avtransport_pause(dmr);
}

#endif






