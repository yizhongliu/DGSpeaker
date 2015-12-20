/* output_gstreamer.c - Output module for GStreamer
 *
 * Copyright (C) 2005-2007   Ivo Clarysse
 *
 * Adapted to gstreamer-0.10 2006 David Siorpaes
 *
 * This file is part of GMediaRender.
 *
 * GMediaRender is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GMediaRender is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GMediaRender; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
 * MA 02110-1301, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>



#include <gstreamer_0.10/logging.h>
#include <gstreamer_0.10/output_gstreamer.h>
#include <cybergarage/upnp/std/av/cmediarenderer.h> //add by llm



#define AUDIO_MPA_CAPS "application/x-rtp,media=(string)audio, clock-rate=(int)90000, encoding-name=(string)MPA, payload=(int)96" 




/* dmr */
static CgUpnpAvRenderer *sdmr = NULL;


/* unicast pipeline varialbe*/
static GMainLoop *playbinLoop = NULL;
static GstElement *play = NULL;
static char *gsuri = NULL;

/* multicast pipeline variable */
static GstElement *rtpPipeline = NULL;
static GMainLoop *rtpLoop = NULL;

static MULTICAST_ATTR multicast_attr = {NULL, 6666};

static guint bus_watch_id;


extern BOOL bGetDuration;
extern BOOL buniPipeline;
extern BOOL bmultiPipeline;


pthread_mutex_t pipelineMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t pipelineCond = PTHREAD_COND_INITIALIZER;



static gboolean show_version = FALSE;
static gboolean show_devicedesc = FALSE;
static gboolean show_connmgr_scpd = FALSE;
static gboolean show_control_scpd = FALSE;
static gboolean show_transport_scpd = FALSE;
static gchar *ip_address = NULL;
static gchar *uuid = "BA2E90A0-3669-401a-B249-F85196ADFC44";
static gchar *friendly_name = PACKAGE_NAME;

static GOptionEntry cmd_option_entries[] = {
        { "version", 0, 0, G_OPTION_ARG_NONE, &show_version,
          "Output version information and exit", NULL },
        { "ip-address", 'I', 0, G_OPTION_ARG_STRING, &ip_address,
          "IP address on which to listen", NULL },
        { "uuid", 'u', 0, G_OPTION_ARG_STRING, &uuid,
          "UUID to advertise", NULL },
        { "friendly-name", 'f', 0, G_OPTION_ARG_STRING, &friendly_name,
          "Friendly name to advertise", NULL },
        { "dump-devicedesc", 0, 0, G_OPTION_ARG_NONE, &show_devicedesc,
          "Dump device descriptor XML and exit", NULL },
        { "dump-connmgr-scpd", 0, 0, G_OPTION_ARG_NONE, &show_connmgr_scpd,
          "Dump Connection Manager service description XML and exit", NULL },
        { "dump-control-scpd", 0, 0, G_OPTION_ARG_NONE, &show_control_scpd,
          "Dump Rendering Control service description XML and exit", NULL },
        { "dump-transport-scpd", 0, 0, G_OPTION_ARG_NONE, &show_transport_scpd,
          "Dump A/V Transport service description XML and exit", NULL },
        { NULL }
};

int process_cmdline(int argc, char **argv)
{
        int result = -1;
        GOptionContext *ctx;
        GError *err = NULL;
        int rc;

        ctx = g_option_context_new("- GMediaRender");
        g_option_context_add_main_entries(ctx, cmd_option_entries, NULL);

        rc = output_gstreamer_add_options(ctx);
        if (rc != 0) {
                goto out;
        }

        if (!g_option_context_parse (ctx, &argc, &argv, &err)) {
                g_print ("Failed to initialize: %s\n", err->message);
                g_error_free (err);
                goto out;
        }

        result = 0;
out:
        LEAVE();
        return result;
}




/*add by llm 20130127*/
/************************************************
 * check_dlna_profieID
 ************************************************/
static int audio_mpeg = 0;
static void check_dlna_additionainfo(const char *cmime_type)
{

    if (strcmp("audio/mpeg", cmime_type) == 0)
    {
       audio_mpeg = 1;
    }
}



/************************************************
 * register_dlna_profieID
 ************************************************/
static void register_dlna_additionainfo(CgUpnpAvRenderer *dmr)
{
    CgUpnpAvProtocolInfo * protocolInfo;

    if (audio_mpeg == 1)
    {
        protocolInfo = cg_upnpav_protocolinfo_new();
        cg_upnpav_protocolinfo_setprotocol(protocolInfo, "http-get");
        cg_upnpav_protocolinfo_setnetwork(protocolInfo, "*");
        cg_upnpav_protocolinfo_setmimetype(protocolInfo, "audio/mpeg");
        cg_upnpav_protocolinfo_setadditionainfo(protocolInfo, "DLNA.ORG_PN=MP3");
        cg_list_insert((CgList *)dmr->protocolInfoList, (CgList *)protocolInfo);

        protocolInfo = cg_upnpav_protocolinfo_new();
        cg_upnpav_protocolinfo_setprotocol(protocolInfo, "http-get");
        cg_upnpav_protocolinfo_setnetwork(protocolInfo, "*");
        cg_upnpav_protocolinfo_setmimetype(protocolInfo, "audio/mpeg");
        cg_upnpav_protocolinfo_setadditionainfo(protocolInfo, "DLNA.ORG_PN=MP3X");
        cg_list_insert((CgList *)dmr->protocolInfoList, (CgList *)protocolInfo);
    }
    
    /*add audio/L16 for test*/
    protocolInfo = cg_upnpav_protocolinfo_new();
    cg_upnpav_protocolinfo_setprotocol(protocolInfo, "http-get");
    cg_upnpav_protocolinfo_setnetwork(protocolInfo, "*");
    cg_upnpav_protocolinfo_setmimetype(protocolInfo, "audio/L16");
    cg_upnpav_protocolinfo_setadditionainfo(protocolInfo, "*");  
    cg_upnpav_protocolinfolist_add(dmr->protocolInfoList, protocolInfo); 

    protocolInfo = cg_upnpav_protocolinfo_new();
    cg_upnpav_protocolinfo_setprotocol(protocolInfo, "http-get");
    cg_upnpav_protocolinfo_setnetwork(protocolInfo, "*");
    cg_upnpav_protocolinfo_setmimetype(protocolInfo, "audio/L16");
    cg_upnpav_protocolinfo_setadditionainfo(protocolInfo, "DLNA.ORG_PN=LPCM");  
    cg_list_insert((CgList *)dmr->protocolInfoList, (CgList *)protocolInfo);
    /*end test*/
}

 
/****************************************
 * register_mime_type
 ****************************************/
void register_mime_type(const char *cmime_type, CgUpnpAvRenderer* dmr)
{
     CgUpnpAvProtocolInfo * protocolInfo;

     check_dlna_additionainfo(cmime_type);
     
     protocolInfo = cg_upnpav_protocolinfo_new();
     
     cg_upnpav_protocolinfo_setprotocol(protocolInfo, "http-get");
     cg_upnpav_protocolinfo_setnetwork(protocolInfo, "*");
     cg_upnpav_protocolinfo_setmimetype(protocolInfo, cmime_type);
     cg_upnpav_protocolinfo_setadditionainfo(protocolInfo, "*");
     
     cg_upnpav_protocolinfolist_add(dmr->protocolInfoList, protocolInfo); 

     if (strcmp("audio/mpeg", cmime_type) == 0) 
     {
         protocolInfo = cg_upnpav_protocolinfo_new();
         
         cg_upnpav_protocolinfo_setprotocol(protocolInfo, "http-get");
         cg_upnpav_protocolinfo_setnetwork(protocolInfo, "*");
         cg_upnpav_protocolinfo_setmimetype(protocolInfo, "audio/x-mpeg");
         cg_upnpav_protocolinfo_setadditionainfo(protocolInfo, "*");
         
         cg_upnpav_protocolinfolist_add(dmr->protocolInfoList, protocolInfo);
     }

     
    
}

/*end llm 20130127*/

static void scan_caps(const GstCaps * caps, CgUpnpAvRenderer* dmr)
{
    guint i;

    g_return_if_fail(caps != NULL);

    if (gst_caps_is_any(caps)) {
        return;
    }
    if (gst_caps_is_empty(caps)) {
        return;
    }

    for (i = 0; i < gst_caps_get_size(caps); i++) {
        GstStructure *structure = gst_caps_get_structure(caps, i);                 
    register_mime_type(gst_structure_get_name(structure), dmr);  //modify by llm
    }

    /*for test*/
//  register_mime_type("audio/x-mpegurl");

}

static void scan_pad_templates_info(GstElement * element,
                    GstElementFactory * factory, CgUpnpAvRenderer* dmr)
{
    const GList *pads;
    GstPadTemplate *padtemplate;
    GstPad *pad;
    GstElementClass *class;

    class = GST_ELEMENT_GET_CLASS(element);

    if (!class->numpadtemplates) {
        return;
    }

    pads = class->padtemplates;
    while (pads) {
        padtemplate = (GstPadTemplate *) (pads->data);
        pad = (GstPad *) (pads->data);
        pads = g_list_next(pads);

        if ((padtemplate->direction == GST_PAD_SINK) &&
            ((padtemplate->presence == GST_PAD_ALWAYS) ||
             (padtemplate->presence == GST_PAD_SOMETIMES) ||
             (padtemplate->presence == GST_PAD_REQUEST)) &&
            (padtemplate->caps)) {
            scan_caps(padtemplate->caps, dmr);
        }
    }

}

#if 0
/****************************************
 * scan_mime_list
 ****************************************/
void scan_mime_list(CgUpnpAvRenderer* dmr)
{
    GList *plugins;
    GstRegistry *registry = gst_registry_get_default();

    CgString *protocolInfos;             //add by llm
    CgUpnpAvProtocolInfo *protocolInfo;  //add by llm

    ENTER();

    if(dmr == NULL)
    {
        printf("%s: input arg is NULL\n", __FUNCTION__);
        return;
    }

    plugins = gst_default_registry_get_plugin_list();

    while (plugins) {
        GList *features;
        GstPlugin *plugin;

        plugin = (GstPlugin *) (plugins->data);
        plugins = g_list_next(plugins);

        features =
            gst_registry_get_feature_list_by_plugin(registry,
                                gst_plugin_get_name
                                (plugin));

        while (features) {
            GstPluginFeature *feature;

            feature = GST_PLUGIN_FEATURE(features->data);

            if (GST_IS_ELEMENT_FACTORY(feature)) {
                GstElementFactory *factory;
                GstElement *element;
                factory = GST_ELEMENT_FACTORY(feature);
                element =
                    gst_element_factory_create(factory,
                                   NULL);
                if (element) {
                    scan_pad_templates_info(element,
                                factory, dmr);
                    gst_object_unref (GST_OBJECT (element)); //add by llm 20130527
                }

                
            }

            features = g_list_next(features);
        }
    }

    /*add by llm*/
    register_dlna_additionainfo(dmr);//dlna 需要在一些媒体类型后面加上profidId,要将这些选项放在一起并且在最前面显示
    
    protocolInfos = cg_string_new();
    for (protocolInfo = cg_upnpav_dmr_getprotocolinfos(dmr); protocolInfo; protocolInfo = cg_upnpav_protocolinfo_next(protocolInfo)) 
    {
        if (0 < cg_string_length(protocolInfos))
            cg_string_addvalue(protocolInfos, ",");
        cg_string_addvalue(protocolInfos, cg_upnpav_protocolinfo_getstring(protocolInfo));
    }

    cg_upnpav_dmr_setsinkprotocolinfo(dmr, protocolInfos->value);

    cg_string_delete(protocolInfos);

    /*end llm*/

    LEAVE();
}

#endif

/*add by llm*/

gchar curTime[9];
gchar totalTime[9];

void get_duration(GstElement *pipeline)
{
    int errCount = 0;

    /*It may need a long time to detect the duration*/
    for(errCount=0; errCount<5; errCount++)
    {
        if(cb_get_duration(pipeline) == TRUE)
        {
                break;
        }
        
        usleep(2000*100);
    }
}

gboolean cb_get_duration(GstElement *pipeline)
{
    gint64 len;
    GstFormat m_format = GST_FORMAT_TIME;

    ENTER();
  
    if (gst_element_query_duration (pipeline, m_format, &len))
    {
          
          g_snprintf(totalTime, 24, "%02u:%02u:%02u", GST_TIME_ARGS(len));

          printf("*******************totalTime = %s \n",(char *)totalTime);

          if(sdmr != NULL)
          {       
              cg_upnpav_dmr_avtransport_setstatevariable(sdmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKDURATION, (char *)totalTime);
              cg_upnpav_dmr_avtransport_setstatevariable(sdmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTMEDIADURATION, (char *)totalTime);
              cg_upnpav_dmr_avtransport_change_lastchange(sdmr);
          } 
          else
          {
              printf("sdmr is null_____________\n");
          }
    }
    else
    {
        printf(" query duartion error.......\n");
        return FALSE;
    }

    LEAVE();

    return TRUE;   
}

gboolean cb_get_position(GstElement *pipeline)
{
    gint64 pos;
    gint64 len;
    GstFormat m_format = GST_FORMAT_TIME;
    
    #if 1
    if (gst_element_query_position (pipeline, m_format, &pos)
        && gst_element_query_duration (pipeline, m_format, &len))
    {
          //g_print ("Time: %" GST_TIME_FORMAT " / %" GST_TIME_FORMAT "\r",
          //         GST_TIME_ARGS (pos), GST_TIME_ARGS (len));
          g_snprintf(curTime, 24, "%02u:%02u:%02u", GST_TIME_ARGS(pos));
          
          if(sdmr != NULL)
          {
              #if 0
              cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTMEDIADURATION,
                                                         (char *)totalTime);
              cg_upnpav_dmr_avtransport_setstatevariable(dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKDURATION,
                                                         (char *)totalTime);
              #endif
              cg_upnpav_dmr_avtransport_setstatevariable(sdmr,CG_UPNPAV_DMR_AVTRANSPORT_ABSOLUTETIMEPOSITION, (char *)curTime);
              cg_upnpav_dmr_avtransport_setstatevariable(sdmr,CG_UPNPAV_DMR_AVTRANSPORT_RELATIVETIMEPOSITION, (char *)curTime);

    
          }       
    }
    #endif

    return TRUE;
}


/*end llm*/

void output_set_uri(const char *uri)
{
    ENTER();
    printf("%s: setting uri to '%s'\n", __FUNCTION__, uri);
    if (gsuri != NULL)
    {
        free(gsuri);
    }
    gsuri = strdup(uri);
    LEAVE();
}


/****************************************
 * output_play
 ****************************************/
int output_play(CgUpnpAvRenderer *dmr, int index)
{
    int result = -1;
    CgUpnpAvTrackInfoList *trackInfoList; 
    CgUpnpAvTrackInfo *trackInfo;

    /* unicast */   
    trackInfoList = dmr->trackInfoList;
    trackInfo = cg_upnpav_trackinfolist_gettrack(trackInfoList, index);
    if(trackInfo == NULL)
    {
        printf("getting trackInfo failed\n");
        goto out;
    }
    #if 1
    ENTER();
    if (gst_element_set_state(play, GST_STATE_READY) ==
        GST_STATE_CHANGE_FAILURE) {
        printf("setting play ready state failed\n");
                goto out;
    }
    g_object_set(G_OBJECT(play), "uri", trackInfo->track->value, NULL);
    printf("======uri:%s\n",trackInfo->track->value);
    FILE *fp = fopen("/mnt/rtsp","r");
    if(NULL != fp){
        char buf[256];
         if(NULL != fgets(buf,sizeof(buf),fp)){
             if('\n'== buf[strlen(buf)-1]){
                 buf[strlen(buf)-1] = '\0';
             }
             g_object_set(G_OBJECT(play), "uri", buf,  NULL);
             printf("======rtsp:%s\n", buf);
         }
//          g_object_set(G_OBJECT(play), "uri", trackInfo->track->value, NULL);
//          g_object_set(G_OBJECT(play), "uri", "rtsp://192.168.1.106:8555/mnt/sdcard/TCL_Demo/Music/89467.mp3",  NULL);
        //  g_object_set(G_OBJECT(play), "uri", "rtsp://192.168.0.115:8555/mnt/sdcard/TCL_Demo/Music/89467.mp3",  NULL);
//          printf("====================rtsp://192.168.1.106:8555/mnt/sdcard/TCL_Demo/Music/89467.mp3\n");
        //  g_object_set(G_OBJECT(play), "uri", "rtsp://10.128.118.57:6668/testStream",  NULL);
        //  g_object_set(G_OBJECT(play), "uri", "rtsp://192.168.0.112/testStream",  NULL);
        //  g_object_set(G_OBJECT(play), "uri", "rtsp://10.84.77.70:554/qq.mp3",  NULL);
         fclose(fp);
    }


    if (gst_element_set_state(play, GST_STATE_PLAYING) ==
        GST_STATE_CHANGE_FAILURE) {
        printf("setting play state failed\n");
        goto out;
    }

    #if 1
    if (gst_element_get_state (play, NULL, NULL, 10 * GST_SECOND) == GST_STATE_CHANGE_FAILURE) {
      // g_error ("Failed to go into PLAYING state");
       printf ("Failed to go into PLAYING state");
       gst_element_set_state(play, GST_STATE_READY);
       goto out;
    }
    #endif

    /*add by llm*/
    g_timeout_add(1000, (GSourceFunc)cb_get_position, play);

    /*get duration through gstreamer query*/
    if(bGetDuration == FALSE)
    {
        get_duration(play);
    }
    /*end llm*/
    #endif
    result = 0;
out:
    LEAVE();
    return result;
}

int output_play2(char *uri)
{
    int result = -1;
    printf("output_play uri= %s\n", uri);
    #if 1
    ENTER();
    if (gst_element_set_state(play, GST_STATE_READY) ==
        GST_STATE_CHANGE_FAILURE) {
        printf("setting play state failed\n");
                goto out;
    }
    g_object_set(G_OBJECT(play), "uri", uri, NULL);
    if (gst_element_set_state(play, GST_STATE_PLAYING) ==
        GST_STATE_CHANGE_FAILURE) {
        printf("setting play state failed\n");
        goto out;
    }

    #if 1
    if (gst_element_get_state (play, NULL, NULL, -1) == GST_STATE_CHANGE_FAILURE) {
      // g_error ("Failed to go into PLAYING state");
       printf ("Failed to go into PLAYING state");
       goto out;
    }
    #endif

    /*add by llm*/
    g_timeout_add(1000, (GSourceFunc)cb_get_position, play);

    /*get duration through gstreamer query*/
    if(bGetDuration == FALSE)
    {
        get_duration(play);
    }
    /*end llm*/
    #endif
out:
    LEAVE();
    return result;
}


/****************************************
 * output_stop
 ****************************************/
int output_stop(void)
{
    ENTER();
        
    if (gst_element_set_state(play, GST_STATE_READY) ==
        GST_STATE_CHANGE_FAILURE) {
        printf ("Failed to set STOPPED state\n");
        return -1;
    } 

    if (gst_element_get_state (play, NULL, NULL, 10*GST_SECOND) == GST_STATE_CHANGE_FAILURE) 
    {
        printf ("Failed to go into STOPPED state\n");
        return -1;
    }

    LEAVE();
        
    return 0;

}


/****************************************
 * output_pause
 ****************************************/
int output_pause(void)
{
    GstStateChangeReturn ret;
    ENTER();
    
    if (gst_element_set_state(play, GST_STATE_PAUSED) ==
        GST_STATE_CHANGE_FAILURE) 
    {
        printf ("Failed to go into PAUSE state1\n");
        return -1;
    } 
    else
    {
        ret = gst_element_get_state (play, NULL, NULL, 5*GST_SECOND);
        
        if (ret == GST_STATE_CHANGE_FAILURE) 
        {
            printf ("Failed to go into PAUSE state2\n");
            return -1;
        }

        if (ret == GST_STATE_CHANGE_ASYNC) 
        {
            printf ("audio sink is in GST_STATE_CHANGE_ASYNC");
            output_stop();
            
            return -2;
        }
    }

    LEAVE();
    
    return 0;

}

/****************************************
 * output_udp_pause
 ****************************************/
int output_udp_pause(CgUpnpAvRenderer *dmr)
{
    CgUpnpAvTrackInfoList *trackInfoList; 
    CgUpnpAvTrackInfo *trackInfo;

    /*get uri from dmr*/
    trackInfoList = dmr->trackInfoList;
    trackInfo = cg_upnpav_trackinfolist_gettrack(trackInfoList, 1);
    if(trackInfo == NULL)
    {
        printf("getting trackInfo failed\n");
        return -1;
    }

    if (gst_element_set_state(play, GST_STATE_READY) ==
        GST_STATE_CHANGE_FAILURE) {
        printf("setting play state failed\n");
        return -1;
    }

    /*set uri*/
    g_object_set(G_OBJECT(play), "uri", trackInfo->track->value, NULL);

    /*change playbin to PAUSED state*/
    if (gst_element_set_state(play, GST_STATE_PAUSED) ==
        GST_STATE_CHANGE_FAILURE) {
        return -1;
    } 
    else
    {
        if (gst_element_get_state (play, NULL, NULL, -1) == GST_STATE_CHANGE_FAILURE) 
        {
            printf ("Failed to go into PAUSE state");
            return -1;
        }
    }

    /*set callback funtion get position*/
    g_timeout_add(1000, (GSourceFunc)cb_get_position, play);

    /*get duration through gstreamer query*/
    if(bGetDuration == FALSE)
    {
        get_duration(play);
    }

    return 0;

}








int output_loop()
{
    GMainLoop *loop;
    
    /* Create a main loop that runs the default GLib main context */
    loop = g_main_loop_new(NULL, FALSE);

    g_main_loop_run(loop);
    return 0;
}


/****************************************
 * output_seek
 ****************************************/
int output_seek(gdouble percentage)
{
    GstFormat fmt = GST_FORMAT_TIME;
    gint64 length;
    gint64 target;
    
    if(gst_element_query_duration(play, fmt, &length))
    {
        target = ((gdouble)length * percentage);
    
        if (!gst_element_seek (play, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
                 GST_SEEK_TYPE_SET, target,
                 GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE)) 
        {
             g_print ("Seek failed!\n");
             return -1;
        }
    }
    else
    {
        g_print("can't query duration\n");
        return -1;
    }

    if (gst_element_get_state (play, NULL, NULL, -1) == GST_STATE_CHANGE_FAILURE) 
    {
        printf ("Failed to go into Seeking PLAYING state");
        return -1;
    }
    
    return 0;
}


/****************************************
 * output_playback
 ****************************************/
int output_playback(void)
{

    if (gst_element_set_state(play, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) 
    {
        return -1;
    } 
    else 
    {
        if (gst_element_get_state (play, NULL, NULL, -1) == GST_STATE_CHANGE_FAILURE) 
        {
            printf ("Failed to go into PLAYING state");
            return -1;
        }
        
        return 0;
    }   
    
}


static const char *gststate_get_name(GstState state)
{
    switch(state) {
    case GST_STATE_VOID_PENDING:
        return "VOID_PENDING";
    case GST_STATE_NULL:
        return "NULL";
    case GST_STATE_READY:
        return "READY";
    case GST_STATE_PAUSED:
        return "PAUSED";
    case GST_STATE_PLAYING:
        return "PLAYING";
    default:
        return "Unknown";
    }
}

static gboolean my_bus_callback(GstBus * bus, GstMessage * msg,
                gpointer data)
{
    GstMessageType msgType;
    GstObject *msgSrc;
    gchar *msgSrcName;

    msgType = GST_MESSAGE_TYPE(msg);
    msgSrc = GST_MESSAGE_SRC(msg);
    msgSrcName = GST_OBJECT_NAME(msgSrc);


    switch (msgType) {
    case GST_MESSAGE_EOS:
            g_print("GStreamer: %s: End-of-stream\n", msgSrcName);

            cg_mutex_lock(sdmr->transportMutex);

            output_stop();
            cg_upnpav_dmr_avtransport_set_transportstate(TRANSPORT_STOPPED);
            
            cg_upnpav_dmr_avtransport_setstatevariable(sdmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE, "STOPPED");
            cg_upnpav_dmr_avtransport_setstatevariable(sdmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "Play");

            printf("set time :%s \n", cg_upnpav_dmr_avtransport_getstatevariable(sdmr,CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKDURATION));

            cg_upnpav_dmr_avtransport_setstatevariable(sdmr,CG_UPNPAV_DMR_AVTRANSPORT_ABSOLUTETIMEPOSITION, 
                                            cg_upnpav_dmr_avtransport_getstatevariable(sdmr,CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKDURATION));
            cg_upnpav_dmr_avtransport_setstatevariable(sdmr,CG_UPNPAV_DMR_AVTRANSPORT_RELATIVETIMEPOSITION, 
                                            cg_upnpav_dmr_avtransport_getstatevariable(sdmr,CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKDURATION));
        
            cg_upnpav_dmr_avtransport_change_lastchange(sdmr);

            cg_mutex_unlock(sdmr->transportMutex);

        break;
    case GST_MESSAGE_ERROR:{
            gchar *debug;
            GError *err;

            gst_message_parse_error(msg, &err, &debug);
            g_free(debug);

            g_print("GStreamer: %s: Error: %s\n", msgSrcName, err->message);
            g_error_free(err);

            output_stop();
            cg_upnpav_dmr_avtransport_set_transportstate(TRANSPORT_STOPPED);
            
            cg_upnpav_dmr_avtransport_setstatevariable(sdmr, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE, "STOPPED");
            cg_upnpav_dmr_avtransport_setstatevariable(sdmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS, "Play");


            cg_upnpav_dmr_avtransport_change_lastchange(sdmr);

            break;
        }
    case GST_MESSAGE_STATE_CHANGED:{
        #if 0
            GstState oldstate, newstate, pending;
            gst_message_parse_state_changed(msg, &oldstate, &newstate, &pending);
            g_print("GStreamer: %s: State change: OLD: '%s', NEW: '%s', PENDING: '%s'\n",
                    msgSrcName,
                    gststate_get_name(oldstate),
                    gststate_get_name(newstate),
                    gststate_get_name(pending));
        #endif
            break;
        }
    #if 0
    case GST_MESSAGE_BUFFERING:{
        gint percent;
        /* no state management needed for live pipelines */
        if (is_live)
            break;
        gst_message_parse_buffering (message, &percent);
        if (percent == 100) {
        /* a 100% message means buffering is done */
            buffering = FALSE;
        /* if the desired state is playing, go back */
            if (target_state == GST_STATE_PLAYING) {
                gst_element_set_state (pipeline, GST_STATE_PLAYING);
            }
        } else {
        /* buffering busy */
            if (buffering == FALSE && target_state == GST_STATE_PLAYING) {
            /* we were not buffering but PLAYING, PAUSE the pipeline. */
            gst_element_set_state (pipeline, GST_STATE_PAUSED);
            }
            buffering = TRUE;
        }
    break;

    #endif
    
    default:
        #if 1
        g_print("GStreamer: %s: unhandled message type %d (%s)\n",
                msgSrcName, msgType, gst_message_type_get_name(msgType));
        #endif
        break;
    }

    return TRUE;
}

static gchar *audiosink = "alsasink";
static gchar *videosink = NULL;

/* Options specific to output_gstreamer */
static GOptionEntry option_entries[] = {
        { "gstout-audiosink", 0, 0, G_OPTION_ARG_STRING, &audiosink,
          "GStreamer audio sink to use "
      "(autoaudiosink, alsasink, osssink, esdsink, ...)",
      NULL },
        { "gstout-videosink", 0, 0, G_OPTION_ARG_STRING, &videosink,
          "GStreamer video sink to use "
      "(autovideosink, xvimagesink, ximagesink, ...)",
      NULL },
        { NULL }
};


int output_gstreamer_add_options(GOptionContext *ctx)
{
    GOptionGroup *option_group;
    ENTER();
    option_group = g_option_group_new("gstout", "GStreamer Output Options",
                                      "Show GStreamer Output Options",
                                      NULL, NULL);
    g_option_group_add_entries(option_group, option_entries);

    g_option_context_add_group (ctx, option_group);
    
    g_option_context_add_group (ctx, gst_init_get_option_group ());
    
    LEAVE();
    return 0;
}

/****************************************
 * output_creat_playbinloop
 ****************************************/
int output_creat_playbinloop()
{     
    GstBus *bus;    
    guint playbin_bus_watch_id;
    
    ENTER();    
    
    play = gst_element_factory_make("playbin", "play");

    bus = gst_pipeline_get_bus(GST_PIPELINE(play));
    playbin_bus_watch_id = gst_bus_add_watch(bus, my_bus_callback, NULL);
    gst_object_unref(bus);

    if(audiosink != NULL){
        GstElement *sink = NULL;
        printf("Setting audio sink to %s\n", audiosink);
        sink = gst_element_factory_make (audiosink, "sink");
        g_object_set (G_OBJECT (play), "audio-sink", sink, NULL);
    }
    if(videosink != NULL){
        GstElement *sink = NULL;
        printf("Setting video sink to %s\n", videosink);
        sink = gst_element_factory_make (videosink, "sink");
        g_object_set (G_OBJECT (play), "video-sink", sink, NULL);
    }

    if (gst_element_set_state(play, GST_STATE_READY) ==
        GST_STATE_CHANGE_FAILURE) {
        fprintf(stderr,
            "Error: pipeline doesn't want to get ready\n");
    }

    pthread_cond_signal(&pipelineCond);

    
    printf("start playbinloop ......\n");

    /* Create a main loop that runs the default GLib main context */
    playbinLoop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(playbinLoop);

    /*clean up*/
    gst_element_set_state (play, GST_STATE_NULL);
    gst_object_unref (play);
    g_source_remove (playbin_bus_watch_id);
    g_main_loop_unref (playbinLoop);

    LEAVE();
    
    return 0;
}

/****************************************
 * output_close_playbinloop
 ****************************************/
int output_close_playbinloop()
{
    g_main_loop_quit(playbinLoop);
    return 0;
}

/****************************************
 * output_creat_rtploop
 ****************************************/
int output_creat_rtploop()
{ 
    
    GstElement *rtpbin,*rtpsrc,*buffer,*rtppay,*audiodecode, *audioconver, *audioresamp, *audiosink,*jitterbuf;
    GstCaps *caps;
    GstPadLinkReturn lres;
    GstPad *srcpad,*sinkpad;

    printf("creat rtp pipeline...\n");

    /* set up pipeline */
    rtpPipeline=gst_pipeline_new(NULL);
    g_assert (rtpPipeline);

    printf("multicastPort = %d\n", multicast_attr.multicastPort);
    printf("multicast-group = %s\n", multicast_attr.multicastIp);
    
    rtpsrc=gst_element_factory_make("udpsrc","rtpsrc"); 
    g_assert (rtpsrc);
    g_object_set (rtpsrc,"port",multicast_attr.multicastPort, NULL);
    g_object_set (rtpsrc,"multicast-group",multicast_attr.multicastIp,NULL);

    caps=gst_caps_from_string(AUDIO_MPA_CAPS);
    g_object_set(rtpsrc,"caps",caps,NULL);
    gst_caps_unref(caps);

    printf("setup rtpsrc...\n");

    printf("make gstrtpjitterbuffer...\n");
    gst_bin_add_many(GST_BIN (rtpPipeline),rtpsrc,NULL);

//    jitterbuf = gst_element_factory_make("gstrtpjitterbuffer", "rtpjitterbuf");
//    g_assert (jitterbuf);

    rtppay=gst_element_factory_make("rtpmpadepay","rtppay");
    g_assert (rtppay);

    audiodecode = gst_element_factory_make("mad","audiomad");
    g_assert (audiodecode);

    audioconver=gst_element_factory_make("audioconvert","audioconver");
    g_assert (audioconver);

    audioresamp=gst_element_factory_make("audioresample","audioresamp");
    g_assert (audioresamp);

    audiosink=gst_element_factory_make("alsasink","audiosink");
    g_assert (audiosink);

    printf("add many...\n");

//    gst_bin_add_many (GST_BIN(rtpPipeline),jitterbuf,rtppay,audiodecode,audioconver,audioresamp,audiosink,NULL);
    gst_bin_add_many (GST_BIN(rtpPipeline),rtppay,audiodecode,audioconver,audioresamp,audiosink,NULL);
    printf("link_many...\n");
  //  gst_bin_add_many (GST_BIN(pipeline),rtppay,audiosink,NULL); 
//    gboolean res=gst_element_link_many(rtpsrc,jitterbuf,rtppay,audiodecode,audioconver,audioresamp,audiosink,NULL);
    gboolean res=gst_element_link_many(rtpsrc,rtppay,audiodecode,audioconver,audioresamp,audiosink,NULL);

    
 //  gboolean res=gst_element_link_many(rtppay,audiodecode,audioconver,audioresamp,audiosink,NULL);
     printf("g_assert...\n");
   // gboolean res=gst_element_link_many(rtppay,audiosink,NULL);  
    g_assert(res==TRUE);

    printf("sync...\n");
    g_object_set (audiosink, "sync", FALSE, NULL);

    printf("set pipeline to ready\n");

    gst_element_set_state (rtpPipeline, GST_STATE_READY);

    pthread_cond_signal(&pipelineCond);

    printf("start rtploop ......\n");
    
    /* Create a main loop that runs the default GLib main context */
    rtpLoop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(rtpLoop);

    /*clean up*/
    gst_element_set_state (rtpPipeline, GST_STATE_NULL);
    gst_object_unref (rtpPipeline);
    g_main_loop_unref (rtpLoop);
    
    return 0;
}

/****************************************
 * output_close_rtploop
 ****************************************/
int output_close_rtploop()
{
    g_main_loop_quit(rtpLoop);
    return 0;
}

/****************************************
 * output_loop_cb
 ****************************************/
void* output_loop_cb(void* arg)
{
    ENTER();
    
    if(sdmr == NULL)
    {
        g_error("dmr do not init/n");
        return;
    }
    
    if(sdmr->uriMode == UNICAST)
    {
        output_creat_playbinloop();
        printf("Main loop for playbin leave.....\n");
    }
    else if(sdmr->uriMode == MULTICAST)
    {
       output_creat_rtploop();
       printf("Main loop for rtpPipeline leave.....\n");
    }

    
    LEAVE();
    
}



/****************************************
 * output_gstreamer_playbin_init
 ****************************************/
int output_gstreamer_playbin_init(CgUpnpAvRenderer *dmr)
{
    pthread_t thread_id;    
    pthread_attr_t thread_attr;

    ENTER();
    
    sdmr = dmr;

//  scan_mime_list();

    /*start main_loop*/
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
    if(-1==pthread_create(&thread_id, &thread_attr, output_loop_cb, NULL))
    {       
        fprintf(stderr, "Create  checkIpThread failed !\n");
        pthread_attr_destroy(&thread_attr);
        return -1;
    }

    pthread_attr_destroy(&thread_attr);

    sleep(3);
    
    pthread_mutex_lock(&pipelineMutex);
    printf("playbin waiting for signal........\n");
    pthread_cond_wait (&pipelineCond, &pipelineMutex);
    pthread_mutex_unlock (&pipelineMutex);

    LEAVE();

    return 0;
}

 
int output_gstreamer_rtpbin_init(CgUpnpAvRenderer *dmr, char* multicastIp, char* multicastPort)
{
    pthread_t thread_id;    
    pthread_attr_t thread_attr;


    if( (dmr==NULL) || (multicastIp==NULL) || (multicastPort==NULL) )
    {
        printf("%s: input arg is NULL!!!!!!\n", __FUNCTION__);
        return -1;
    }

    ENTER();

    /* set sdmr */
    sdmr = dmr;

    /* set value of multicast_attr */
    if(multicast_attr.multicastIp ==NULL){
        multicast_attr.multicastIp = (char *)malloc( 16 * sizeof(char) );//IP max  length is 16,just need mallac once
    }
    
    if(multicast_attr.multicastIp == NULL)
    {
        printf("%s:can't malloc memory for multicastIp\n", __FUNCTION__);
        return -1;
    }
    strcpy(multicast_attr.multicastIp, multicastIp);
    multicast_attr.multicastIp[strlen(multicastIp)] = '\0';

    multicast_attr.multicastPort = (gint)atoi(multicastPort);

    printf("multicastip = %s\n", multicast_attr.multicastIp);
    
    /*start multicast received pipeline */
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
    if(-1==pthread_create(&thread_id, &thread_attr, output_loop_cb, NULL))
    {       
        fprintf(stderr, "Create  checkIpThread failed !\n");
        pthread_attr_destroy(&thread_attr);
        return -1;
    }

    pthread_attr_destroy(&thread_attr);

    pthread_mutex_lock(&pipelineMutex);
    pthread_cond_wait (&pipelineCond, &pipelineMutex);
    pthread_mutex_unlock (&pipelineMutex);

    LEAVE();

    return 0;

}


/****************************************
 * output_gstreamer_playbin_init
 ****************************************/
int output_gstreamer_pipeline_init(CgUpnpAvRenderer *dmr)
{
    
    GstBus *bus;    
    
    if(dmr == NULL)
    {
        printf("%s: input arg is NULL!!!!!!\n", __FUNCTION__);
        return -1;
    }

    ENTER();

    sdmr = dmr;
    
    play = gst_element_factory_make("playbin", "play");

    bus = gst_pipeline_get_bus(GST_PIPELINE(play));
    bus_watch_id = gst_bus_add_watch(bus, my_bus_callback, NULL);
    gst_object_unref(bus);

    if(audiosink != NULL){
        GstElement *sink = NULL;
        printf("Setting audio sink to %s\n", audiosink);
        sink = gst_element_factory_make (audiosink, "sink");
        g_object_set (G_OBJECT (play), "audio-sink", sink, NULL);
    }
    if(videosink != NULL){
        GstElement *sink = NULL;
        printf("Setting video sink to %s\n", videosink);
        sink = gst_element_factory_make (videosink, "sink");
        g_object_set (G_OBJECT (play), "video-sink", sink, NULL);
    }

    if (gst_element_set_state(play, GST_STATE_READY) ==
        GST_STATE_CHANGE_FAILURE) {
        fprintf(stderr,
            "Error: pipeline doesn't want to get ready\n");
    }

    buniPipeline = TRUE;
    dmr->uriMode = UNICAST;
    
    LEAVE();
        
    return 0;   
}

/****************************************
 *  output_pipeline_loop
 ****************************************/
int output_pipeline_loop()
{
    ENTER();
    /* Create a main loop that runs the default GLib main context */
    playbinLoop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(playbinLoop);

    /*clean up*/
    gst_element_set_state (play, GST_STATE_NULL);
    gst_object_unref (play);
    g_source_remove (bus_watch_id);
    g_main_loop_unref (playbinLoop);

    LEAVE();
    
    return 0;
}
    


int output_gstreamer_rtpbin_mpa_test(CgUpnpAvRenderer *dmr)
{
#if 1
    GstElement *rtpbin,*rtpsrc,*buffer,*rtppay,*audiodecode, *audioconver, *audioresamp, *audiosink;
    GstElement *pipeline;
    GMainLoop *loop;
    GstCaps *caps;
    GstPadLinkReturn lres;
    GstPad *srcpad,*sinkpad;

    pipeline=gst_pipeline_new(NULL);
    g_assert (pipeline);
    rtpsrc=gst_element_factory_make("udpsrc","rtpsrc");
    g_assert (rtpsrc);
    g_object_set (rtpsrc,"port",6666, NULL);
    g_object_set (rtpsrc,"multicast-group","239.255.42.42",NULL);

    printf("%s\n", AUDIO_MPA_CAPS);
    caps=gst_caps_from_string(AUDIO_MPA_CAPS);
    g_object_set(rtpsrc,"caps",caps,NULL);
    gst_caps_unref(caps);

    gst_bin_add_many(GST_BIN (pipeline),rtpsrc,NULL);

    rtppay=gst_element_factory_make("rtpmpadepay","rtppay");
    g_assert (rtppay);

    audiodecode = gst_element_factory_make("mad","audiomad");
    g_assert (audiodecode);

    audioconver=gst_element_factory_make("audioconvert","audioconver");
    g_assert (audioconver);

    audioresamp=gst_element_factory_make("audioresample","audioresamp");
    g_assert (audioresamp);

    audiosink=gst_element_factory_make("alsasink","audiosink");
    g_assert (audiosink);

    gst_bin_add_many (GST_BIN(pipeline),rtppay,audiodecode,audioconver,audioresamp,audiosink,NULL);

  //  gst_bin_add_many (GST_BIN(pipeline),rtppay,audiosink,NULL); 
    gboolean res=gst_element_link_many(rtpsrc,rtppay,audiodecode,audioconver,audioresamp,audiosink,NULL);

   // gboolean res=gst_element_link_many(rtppay,audiosink,NULL);  
    g_assert(res==TRUE);
    g_object_set (audiosink, "sync", FALSE, NULL);

    g_print ("starting receiver pipeline\n");
    gst_element_set_state (pipeline, GST_STATE_PLAYING);

    loop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (loop);

    g_print ("stopping receiver pipeline\n");
    gst_element_set_state (pipeline, GST_STATE_NULL);

    gst_object_unref (pipeline);
#endif
    return 0;

}


