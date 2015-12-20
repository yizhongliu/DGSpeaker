/************************************************************
*
 
*	CyberLink for C
*
*	Copyright (C) Satoshi Konno 2005
*
*	File: cmediarenderer.h
*
*	Revision:
*		2009/06/11
*        - first release.
*       2013/03/18 Liu Liming
*        - add struct CgUpnpAvTrackInfoList *trackInfoList
*          in struct CgUpnpAvRenderer
*        - add enum _transport_state 
*        - add macro-definitions
*        - add functions to handle statevariable
*
************************************************************/

#ifndef _CG_CLINKCAV_MEDIARENDERER_H_
#define _CG_CLINKCAV_MEDIARENDERER_H_

#include <cybergarage/upnp/cupnp.h>
#include <cybergarage/upnp/std/av/cupnpav.h>
//#include <cybergarage/upnp/std/av/cprotocolinfo.h>
#include <cybergarage/upnp/std/av/ctrackinfo.h>

#ifdef  __cplusplus
extern "C" {
#endif

#undef CG_CLINKCAV_USE_UPNPSTD_XML 

/****************************************
* Struct
****************************************/
typedef enum _CgUriMode {
    UNICAST,
    MULTICAST   
} CgUriMode;


typedef struct _CgUpnpAvRenderer {
CgMutex *mutex;
CgUpnpDevice *dev;
CG_UPNPAV_HTTP_LISTENER httplistener;
CG_UPNPAV_ACTION_LISTNER actionListner;
CG_UPNPAV_STATEVARIABLE_LISTNER queryListner;
CgUpnpAvProtocolInfoList *protocolInfoList;
CgUpnpAvTrackInfoList *trackInfoList;  //add by llm 20130319
CgMutex *transportMutex;  //add by llm 20130508
CgUriMode uriMode; //add by llm 20130802 
//CgMutex *uniMutex;  //add by llm 20130802 mutex for pipeline that play unicast streams
//CgMutex *multiMutex;  //add by llm 20130802  mutex for pipeline that play multicast streams

void *userData;
} CgUpnpAvRenderer;

typedef enum _transport_state {
	TRANSPORT_STOPPED,
	TRANSPORT_PLAYING,
	TRANSPORT_TRANSITIONING,	/* optional */
	TRANSPORT_PAUSED_PLAYBACK,	/* optional */
	TRANSPORT_PAUSED_RECORDING,	/* optional */
	TRANSPORT_RECORDING,	/* optional */
	TRANSPORT_NO_MEDIA_PRESENT	/* optional */
}TRANSPORTSTATE;

typedef void (*CG_UPNPAV_VOLUME_LISTNER)(unsigned char);
typedef int (*CG_UPNPAV_VOLUME_GETLISTNER)();

typedef void (*CG_UPNPAV_MUTE_LISTNER)();
typedef void (*CG_UPNPAV_UNMUTE_LISTNER)();



/****************************************
* Constants 
****************************************/

#define FILE_READ_CHUNK_SIZE    1024


#define CG_UPNPAV_DMR_DEVICE_DESCRIPTION "/home/llm/project/DMR/clinkc_upnpavrender/res/description.xml"
#define CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_DESCRIPTION "/home/llm/project/DMR/clinkc_upnpavrender/res/connectionmanager.xml"
#define CG_UPNPAV_DMR_AVTRANSPORT_SERVICE_DESCRIPTION "/home/llm/project/DMR/clinkc_upnpavrender/res/avtransport.xml"
#define CG_UPNPAV_DMR_RENDERINGCONTROL_SERVICE_DESCRIPTION "/home/llm/project/DMR/clinkc_upnpavrender/res/renderingcontrol.xml"

/****************************************
* Constants (Media Server)
****************************************/

#define CG_UPNPAV_DMR_DEVICE_TYPE "urn:schemas-upnp-org:device:MediaRenderer:1"
#define CG_UPNPAV_DMR_DEFAULT_HTTP_PORT 38520

/****************************************
* Constants (Rendering Control)
****************************************/

#define CG_UPNPAV_DMR_RENDERINGCONTROL_SERVICE_TYPE "urn:schemas-upnp-org:service:RenderingControl:1"
#define CG_UPNPAV_DMR_RENDERINGCONTROL_GETVOLUME "GetVolume"
#define CG_UPNPAV_DMR_RENDERINGCONTROL_GETMUTE "GetMute"
#define CG_UPNPAV_DMR_RENDERINGCONTROL_SETVOLUME "SetVolume"
#define CG_UPNPAV_DMR_RENDERINGCONTROL_SETMUTE "SetMute"
#define CG_UPNPAV_DMR_RENDERINGCONTROL_CURRENTMUTE "CurrentMute"
#define CG_UPNPAV_DMR_RENDERINGCONTROL_DESIREDMUTE "DesiredMute"
#define CG_UPNPAV_DMR_RENDERINGCONTROL_CURRENTVOLUME "CurrentVolume"
#define CG_UPNPAV_DMR_RENDERINGCONTROL_DESIREDVOLUME "DesiredVolume"
#define CG_UPNPAV_DMR_RENDERINGCONTROL_LASTCHANGE "LastChange"

/*add by llm*/
#define CG_UPNPAV_DMR_RENDERINGCONTROL_LISTPRESETS    "ListPresets"
#define CG_UPNPAV_DMR_RENDERINGCONTROL_SELECTPRESET   "SelectPreset"

//arg
#define CG_UPNPAV_DMR_RENDERINGCONTROL_DESIREDVOLUME  "DesiredVolume"
#define CG_UPNPAV_DMR_RENDERINGCONTROL_CHANNEL        "Channel"
#define CG_UPNPAV_DMR_RENDERINGCONTROL_DESIREDMUTE    "DesiredMute"
#define CG_UPNPAV_DMR_RENDERINGCONTROL_CURRENTPRESETNAMELIST "CurrentPresetNameList"
#define CG_UPNPAV_DMR_RENDERINGCONTROL_PRESETNAME "PresetName"



//var
#define CG_UPNPAV_DMR_RENDERINGCONTROL_VOLUME  "Volume"
#define CG_UPNPAV_DMR_RENDERINGCONTROL_PRESETNAMELIST  "PresetNameList"
#define CG_UPNPAV_DMR_RENDERINGCONTROL_AAT_CHANNEL  "A_ARG_TYPE_Channel"
#define CG_UPNPAV_DMR_RENDERINGCONTROL_MUTE  "Mute"
#define CG_UPNPAV_DMR_RENDERINGCONTROL_AAT_PRESETNAME   "A_ARG_TYPE_PresetName"




/****************************************
* Constants (Connection Manager)
****************************************/

#define CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_TYPE "urn:schemas-upnp-org:service:ConnectionManager:1"

#define CG_UPNPAV_DMR_CONNECTIONMANAGER_HTTP_GET "http-get"
	
#define CG_UPNPAV_DMR_CONNECTIONMANAGER_GET_PROTOCOL_INFO "GetProtocolInfo"
#define CG_UPNPAV_DMR_CONNECTIONMANAGER_SOURCE "Source"
#define CG_UPNPAV_DMR_CONNECTIONMANAGER_SINK "Sink"
#define CG_UPNPAV_DMR_CONNECTIONMANAGER_CURRENTCONNECTIONIDS "CurrentConnectionIDs"
#define CG_UPNPAV_DMR_CONNECTIONMANAGER_SOURCEPROTOCOLINFO "SourceProtocolInfo"
#define CG_UPNPAV_DMR_CONNECTIONMANAGER_SINKPROTOCOLINFO "SinkProtocolInfo"

/*add by llm*/
#define CG_UPNPAV_DMR_CONNECTIONMANAGER_LASTCHANGE                  "LastChange"
#define CG_UPNPAV_DMR_CONNECTIONMANAGER_GET_CURRENT_CONNECTION_IDS  "GetCurrentConnectionIDs"
#define CG_UPNPAV_DMR_CONNECTIONMANAGER_CONNECTIONIDS               "ConnectionIDs"
#define CG_UPNPAV_DMR_CONNECTIONMANAGER_GET_CURRENT_CONNECTION_INFO "GetCurrentConnectionInfo"
#define CG_UPNPAV_DMR_CONNECTIONMANAGER_CONNECTIONID                "ConnectionID"
#define CG_UPNPAV_DMR_CONNECTIONMANAGER_RCSID                       "RcsID"
#define CG_UPNPAV_DMR_CONNECTIONMANAGER_AVTRANSPORTID               "AVTransportID"
#define CG_UPNPAV_DMR_CONNECTIONMANAGER_PROTOCOLINFO                "ProtocolInfo"
#define CG_UPNPAV_DMR_CONNECTIONMANAGER_PEERCONNECTIONMANAGER       "PeerConnectionManager"
#define CG_UPNPAV_DMR_CONNECTIONMANAGER_PEERCONNECTIONID            "PeerConnectionID"	
#define CG_UPNPAV_DMR_CONNECTIONMANAGER_DIRECTION                   "Direction"
#define CG_UPNPAV_DMR_CONNECTIONMANAGER_STATUS                      "Status"
#define CG_UPNPAV_DMR_CONNECTIONMANAGER_AAT_CONNECTIONID            "A_ARG_TYPE_ConnectionID"
#define CG_UPNPAV_DMR_CONNECTIONMANAGER_AAT_RCSID                   "A_ARG_TYPE_RcsID"
#define CG_UPNPAV_DMR_CONNECTIONMANAGER_AAT_AVTRANSPORTID           "A_ARG_TYPE_AVTransportID"
#define CG_UPNPAV_DMR_CONNECTIONMANAGER_AAT_PROTOCOLINFO            "A_ARG_TYPE_ProtocolInfo"
#define CG_UPNPAV_DMR_CONNECTIONMANAGER_AAT_CONNECTIONMANAGER       "A_ARG_TYPE_ConnectionManager"
#define CG_UPNPAV_DMR_CONNECTIONMANAGER_AAT_DIRECTION               "A_ARG_TYPE_Direction"
#define CG_UPNPAV_DMR_CONNECTIONMANAGER_AAT_STATUS                  "A_ARG_TYPE_ConnectionStatus"

/****************************************
* Constants (AVTransport)
****************************************/

#define CG_UPNPAV_DMR_AVTRANSPORT_SERVICE_TYPE "urn:schemas-upnp-org:service:AVTransport:1"


#define CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE "CurrentTransportState"
#define CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATUS "CurrentTransportStatus"
#define CG_UPNPAV_DMR_AVTRANSPORT_CURRENTSPEED "CurrentSpeed"

#define CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE_NOMEDIAPRESENT "NO_MEDIA_PRESENT"
#define CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE_STOPPED "STOPPED"
#define CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATUS_OK "OK"

/*add by llm 20130127*/

//Var
#define CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATE                "TransportState"
#define CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTSTATUS               "TransportStatus"
#define CG_UPNPAV_DMR_AVTRANSPORT_CURRENTMEDIACATEGORY          "CurrentMediaCategory"
#define CG_UPNPAV_DMR_AVTRANSPORT_PLAYBACKSTORAGEMEDIUM         "PlaybackStorageMedium"
#define CG_UPNPAV_DMR_AVTRANSPORT_RECORDSTORAGEMEDIUM           "RecordStorageMedium"
#define CG_UPNPAV_DMR_AVTRANSPORT_POSSIBLEPLAYBACKSTORAGEMEDIA  "PossiblePlaybackStorageMedia"
#define CG_UPNPAV_DMR_AVTRANSPORT_POSSIBLERECORDSTORAGEMEDIA    "PossibleRecordStorageMedia"
#define CG_UPNPAV_DMR_AVTRANSPORT_CURRENTPALYMODE               "CurrentPlayMode"
#define CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORTPLAYSTATE            "TransportPlaySpeed"
#define CG_UPNPAV_DMR_AVTRANSPORT_RECORDMEDIUMWRITESTATUS       "RecordMediumWriteStatus"
#define CG_UPNPAV_DMR_AVTRANSPORT_CURRENTRECORDQUALITYMODE      "CurrentRecordQualityMode"
#define CG_UPNPAV_DMR_AVTRANSPORT_POSSIBLERECORDQUALITYMODES    "PossibleRecordQualityModes"
#define CG_UPNPAV_DMR_AVTRANSPORT_NUMBEROFTRACKS                "NumberOfTracks"
#define CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACK                  "CurrentTrack"
#define CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKDURATION          "CurrentTrackDuration"
#define CG_UPNPAV_DMR_AVTRANSPORT_CURRENTMEDIADURATION          "CurrentMediaDuration"
#define CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKMETADATA          "CurrentTrackMetaData"
#define CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKURI               "CurrentTrackURI"
#define CG_UPNPAV_DMR_AVTRANSPORT_AVTRANSPORTURI                "AVTransportURI"
#define CG_UPNPAV_DMR_AVTRANSPORT_AVTRANSPORTURIMETADATA        "AVTransportURIMetaData"
#define CG_UPNPAV_DMR_AVTRANSPORT_NEXTAVTRANSPORTURI            "NextAVTransportURI"
#define CG_UPNPAV_DMR_AVTRANSPORT_NEXTAVTRANSPORTURIMETADATA    "NextAVTransportURIMetaData"
#define CG_UPNPAV_DMR_AVTRANSPORT_RELATIVETIMEPOSITION          "RelativeTimePosition"
#define CG_UPNPAV_DMR_AVTRANSPORT_ABSOLUTETIMEPOSITION          "AbsoluteTimePosition"
#define CG_UPNPAV_DMR_AVTRANSPORT_RELATIVECOUNTERPOSITION       "RelativeCounterPosition"
#define CG_UPNPAV_DMR_AVTRANSPORT_ABSOLUTECOUNTERPOSITION       "AbsoluteCounterPosition"
#define CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTACTIONS       "CurrentTransportActions"
#define CG_UPNPAV_DMR_AVTRANSPORT_LASTCHANGE                    "LastChange"
#define CG_UPNPAV_DMR_AVTRANSPORT_AAT_SEEKMODE                  "A_ARG_TYPE_SeekMode"
#define CG_UPNPAV_DMR_AVTRANSPORT_AAT_SEEKTARGET                "A_ARG_TYPE_SeekTarget"
#define CG_UPNPAV_DMR_AVTRANSPORT_AAT_INSTANCEID                "A_ARG_TYPE_InstanceID"
#define CG_UPNPAV_DMR_AVTRANSPORT_MULTICASTIP                   "MulticastIp"            /*private*/
#define CG_UPNPAV_DMR_AVTRANSPORT_MULTICASTPORT                 "MulticastPort"          /*private*/
#define CG_UPNPAV_DMR_AVTRANSPORT_CURRENTCONTROLPOINT           "X_CurrentControlPoint"    /*private*/
#define CG_UPNPAV_DMR_AVTRANSPORT_CURRENTWIFIINTES              "X_CurrentWifiIntes"    /*private*/
#define CG_UPNPAV_DMR_AVTRANSPORT_UDPSESSION                    "UdpSession"
#define CG_UPNPAV_DMR_AVTRANSPORT_UDPSEEKRESPONSE               "UdpSeekResponse"



//arg
#define CG_UPNPAV_DMR_AVTRANSPORT_X_WIFIINTES                   "X_WifiIntes"     /*private*/
#define CG_UPNPAV_DMR_AVTRANSPORT_X_ControlPoint                "X_ControlPoint"     /*private*/
#define CG_UPNPAV_DMR_AVTRANSPORT_INSTANCEID                    "InstanceID"
#define CG_UPNPAV_DMR_AVTRANSPORT_CURRENTURI                    "CurrentURI"
#define CG_UPNPAV_DMR_AVTRANSPORT_CURRENTURIMETADATA            "CurrentURIMetaData"

#define CG_UPNPAV_DMR_AVTRANSPORT_NEXTURI                       "NextURI"
#define CG_UPNPAV_DMR_AVTRANSPORT_NEXTURIMETADATA               "NextURIMetaData"

#define CG_UPNPAV_DMR_AVTRANSPORT_NRTRACKS                      "NrTracks"
#define CG_UPNPAV_DMR_AVTRANSPORT_NUMBEROFTRACKS                "NumberOfTracks"
#define CG_UPNPAV_DMR_AVTRANSPORT_MEDIADURATION                 "MediaDuration"

#define CG_UPNPAV_DMR_AVTRANSPORT_PLAYMEDIUM                    "PlayMedium"
#define CG_UPNPAV_DMR_AVTRANSPORT_PLAYBACKSTORAGEMEDIUM         "PlaybackStorageMedium"
#define CG_UPNPAV_DMR_AVTRANSPORT_RECORDMEDIUM                  "RecordMedium"
#define CG_UPNPAV_DMR_AVTRANSPORT_WRITESTATUS                   "WriteStatus"

#define CG_UPNPAV_DMR_AVTRANSPORT_TRACK                         "Track"
#define CG_UPNPAV_DMR_AVTRANSPORT_TRACKDURATION                 "TrackDuration"
#define CG_UPNPAV_DMR_AVTRANSPORT_TRACKMETADATA                 "TrackMetaData"
#define CG_UPNPAV_DMR_AVTRANSPORT_TRACKURI                      "TrackURI"
#define CG_UPNPAV_DMR_AVTRANSPORT_RELTIME                       "RelTime"
#define CG_UPNPAV_DMR_AVTRANSPORT_ABSTIME                       "AbsTime"
#define CG_UPNPAV_DMR_AVTRANSPORT_RELCOUNT                      "RelCount"
#define CG_UPNPAV_DMR_AVTRANSPORT_ABSCOUNT                      "AbsCount"

#define CG_UPNPAV_DMR_AVTRANSPORT_PLAYMEDIA                     "PlayMedia"
#define CG_UPNPAV_DMR_AVTRANSPORT_RECMEDIA                      "RecMedia"
#define CG_UPNPAV_DMR_AVTRANSPORT_RECQUALITYMODES               "RecQualityModes"

#define CG_UPNPAV_DMR_AVTRANSPORT_UNIT                          "Unit"
#define CG_UPNPAV_DMR_AVTRANSPORT_TARGET                        "Target"

#define CG_UPNPAV_DMR_AVTRANSPORT_ACTIONS                       "Actions"

#define CG_UPNPAV_DMR_AVTRANSPORT_PLAYMODE                      "PlayMode"
#define CG_UPNPAV_DMR_AVTRANSPORT_RECQUALITYMODE                "RecQualityMode"

#define CG_UPNPAV_DMR_AVTRANSPORT_SPEED                         "Speed"
#define CG_UPNPAV_DMR_AVTRANSPORT_SESSION                       "Session"







//Action
#define CG_UPNPAV_DMR_AVTRANSPORT_SETAVTRANSPORTURI               "SetAVTransportURI"
#define CG_UPNPAV_DMR_AVTRANSPORT_SETNEXTAVTRANSPORTURI           "SetNextAVTransportURI" /*Option*/
#define CG_UPNPAV_DMR_AVTRANSPORT_GETMEDIAINFO                    "GetMediaInfo"
#define CG_UPNPAV_DMR_AVTRANSPORT_GETMEDIAINFO_EXT                "GetMediaInfo_Ext"
#define CG_UPNPAV_DMR_AVTRANSPORT_GETTRANSPORTINFO                "GetTransportInfo"
#define CG_UPNPAV_DMR_AVTRANSPORT_GETPOSITIONINFO                 "GetPositionInfo"
#define CG_UPNPAV_DMR_AVTRANSPORT_GETDEVICECAPABILITIES           "GetDeviceCapabilities"
#define CG_UPNPAV_DMR_AVTRANSPORT_GETTRANSPORTSETTINGS            "GetTransportSettings"
#define CG_UPNPAV_DMR_AVTRANSPORT_PLAY                            "Play"
#define CG_UPNPAV_DMR_AVTRANSPORT_STOP                            "Stop"
#define CG_UPNPAV_DMR_AVTRANSPORT_NEXT                            "Next"
#define CG_UPNPAV_DMR_AVTRANSPORT_PREVIOUS                        "Previous"
#define CG_UPNPAV_DMR_AVTRANSPORT_SEEK                            "Seek"
#define CG_UPNPAV_DMR_AVTRANSPORT_PAUSE                           "Pause"
#define CG_UPNPAV_DMR_AVTRANSPORT_GETCURRENTTRANSPORTACTIONS      "GetCurrentTransportActions"
#define CG_UPNPAV_DMR_AVTRANSPORT_SETMULTICASTURI                 "SetMulticastURI"  /*private*/
#define CG_UPNPAV_DMR_AVTRANSPORT_X_GETCONTROLPOINT               "X_GetControlPoint"   /*private*/
#define CG_UPNPAV_DMR_AVTRANSPORT_X_SETCONTROLPOINT               "X_SetControlPoint"   /*private*/
#define CG_UPNPAV_DMR_AVTRANSPORT_X_GETWIFIINTES                  "X_GetWifiIntes"   /*private*/
#define CG_UPNPAV_DMR_AVTRANSPORT_PLAYPREPARE                     "PlayPrepare"   /*private*/

//error code
#define CG_UPNPAV_TRANSPORT_E_TRANSITION_NA	        701
#define CG_UPNPAV_TRANSPORT_E_NO_CONTENTS	        702
#define CG_UPNPAV_TRANSPORT_E_READ_ERROR	        703
#define CG_UPNPAV_TRANSPORT_E_PLAY_FORMAT_NS	    704
#define CG_UPNPAV_TRANSPORT_E_TRANSPORT_LOCKED	    705
#define CG_UPNPAV_TRANSPORT_E_WRITE_ERROR	        706
#define CG_UPNPAV_TRANSPORT_E_REC_MEDIA_WP	        707
#define CG_UPNPAV_TRANSPORT_E_REC_FORMAT_NS	        708
#define CG_UPNPAV_TRANSPORT_E_REC_MEDIA_FULL	    709
#define CG_UPNPAV_TRANSPORT_E_SEEKMODE_NS	        710
#define CG_UPNPAV_TRANSPORT_E_ILL_SEEKTARGET	    711
#define CG_UPNPAV_TRANSPORT_E_PLAYMODE_NS	        712
#define CG_UPNPAV_TRANSPORT_E_RECQUAL_NS	        713
#define CG_UPNPAV_TRANSPORT_E_ILLEGAL_MIME	        714
#define CG_UPNPAV_TRANSPORT_E_CONTENT_BUSY	        715
#define CG_UPNPAV_TRANSPORT_E_RES_NOT_FOUND	        716
#define CG_UPNPAV_TRANSPORT_E_PLAYSPEED_NS	        717
#define CG_UPNPAV_TRANSPORT_E_INVALID_IID	        718



/****************************************
* Public Functions
****************************************/

CgUpnpAvRenderer *cg_upnpav_dmr_new();
void cg_upnpav_dmr_delete(CgUpnpAvRenderer *dmr);

#define cg_upnpav_dmr_getdevice(dmr) (dmr->dev)

#define cg_upnpav_dmr_start(dmr) cg_upnp_device_start(dmr->dev)
#define cg_upnpav_dmr_stop(dmr) cg_upnp_device_stop(dmr->dev)

#define cg_upnpav_dmr_lock(dmr) cg_mutex_lock(dmr->mutex)
#define cg_upnpav_dmr_unlock(dmr) cg_mutex_unlock(dmr->mutex)

#define cg_upnpav_dmr_setfriendlyname(dmr, value) cg_upnp_device_setfriendlyname(dmr->dev, value)
#define cg_upnpav_dmr_getfriendlyname(dmr) cg_upnp_device_getfriendlyname(dmr->dev)

#define cg_upnpav_dmr_setudn(dmr, value) cg_upnp_device_setudn(dmr->dev, value)
#define cg_upnpav_dmr_getudn(dmr) cg_upnp_device_getudn(dmr->dev)

#define cg_upnpav_dmr_sethttplistener(dmr,func) (dmr->httplistener = func)
#define cg_upnpav_dmr_gethttplistener(dmr) (dmr->httplistener)

#define cg_upnpav_dmr_setactionlistener(dmr,func) (dmr->actionListner = func)
#define cg_upnpav_dmr_getactionlistener(dmr) (dmr->actionListner)

#define cg_upnpav_dmr_setquerylistener(dmr,func) (dmr->queryListner = func)
#define cg_upnpav_dmr_getquerylistener(dmr) (dmr->queryListner)

#define cg_upnpav_dmr_setuserdata(dmr,data) (dmr->userData = data)
#define cg_upnpav_dmr_getuserdata(dmr) (dmr->userData)

void cg_upnpav_dmr_addprotocolinfo(CgUpnpAvRenderer *dmr, CgUpnpAvProtocolInfo *info);
#define cg_upnpav_dmr_getprotocolinfos(dmr) cg_upnpav_protocolinfolist_gets(dmr->protocolInfoList)

void cg_upnpav_dmr_setsinkprotocolinfo(CgUpnpAvRenderer *dmr, char *value);
char *cg_upnpav_dmr_getsinkprotocolinfo(CgUpnpAvRenderer *dmr);

void cg_upnpav_dmr_setsourceprotocolinfo(CgUpnpAvRenderer *dmr, char *value);
char *cg_upnpav_dmr_getsourceprotocolinfo(CgUpnpAvRenderer *dmr);

void cg_upnpav_dmr_setcurrentconnectionids(CgUpnpAvRenderer *dmr, char *value);
char *cg_upnpav_dmr_getcurrentconnectionids(CgUpnpAvRenderer *dmr);

void cg_upnpav_dmr_setavtransportlastchange(CgUpnpAvRenderer *dmr, char *value);
char *cg_upnpav_dmr_getavtransportlastchange(CgUpnpAvRenderer *dmr);

void cg_upnpav_dmr_setrenderingcontrollastchange(CgUpnpAvRenderer *dmr, char *value);
char *cg_upnpav_dmr_getrenderingcontrollastchange(CgUpnpAvRenderer *dmr);

void cg_upnpav_dmr_setprotocolinfo(CgUpnpAvRenderer *dmr, char *value);
char *cg_upnpav_dmr_getprotocolinfo(CgUpnpAvRenderer *dmr);


/*add by llm*/
void cg_upnpav_dmr_connectionmgrr_variable_init(CgUpnpAvRenderer *dmr);

void cg_upnpav_dmr_connectionmgrr_setstatevariable(CgUpnpAvRenderer *dmr,char *statevariable, char *value);
char *cg_upnpav_dmr_connectionmgrr_getstatevariable(CgUpnpAvRenderer *dmr, char *statevariable);


void cg_upnpav_dmr_avtransport_variable_init(CgUpnpAvRenderer *dmr);

void cg_upnpav_dmr_avtransport_setstatevariable(CgUpnpAvRenderer *dmr,char *statevariable, char *value);
char *cg_upnpav_dmr_avtransport_getstatevariable(CgUpnpAvRenderer *dmr,char *statevariable);

void cg_upnpav_dmr_avtransport_change_var(CgUpnpAvRenderer *dmr, char *stateVariable, char *newValue);

void cg_upnpav_dmr_avtransport_set_transportstate(TRANSPORTSTATE state);

void cg_upnpav_dmr_avtransport_change_lastchange(CgUpnpAvRenderer *dmr);


void cg_upnpav_dmr_rendercontrol_variable_init(CgUpnpAvRenderer *dmr);

void cg_upnpav_dmr_rendercontrol_setstatevariable(CgUpnpAvRenderer *dmr,char *statevariable, char *value);
char *cg_upnpav_dmr_rendercontrol_getstatevariable(CgUpnpAvRenderer *dmr, char *statevariable);

void cg_upnpav_dmr_rendercontrol_change_lastchange(CgUpnpAvRenderer *dmr);



void* checkIpThread (CgUpnpDevice *device);


BOOL cg_upnpav_dmr_avtransport_setmulticasturi_test(CgUpnpAvRenderer *dmr, char* mulUri);
BOOL cg_upnpav_dmr_avtransport_play_test(CgUpnpAvRenderer *dmr);

BOOL cg_upnp_service_loaddescriptionfile(CgUpnpService *service, char *fileName);
BOOL cg_upnp_device_loaddescriptionfile(CgUpnpDevice *dev, char *fileName);

/*
 *I don't remember what this for
 */
#if 0
BOOL cg_upnpav_dmr_sudp_seek_receive(CgUpnpSUDPPacket *sudpPkt);
BOOL cg_upnpav_dmr_sudp_play_receive(CgUpnpSUDPPacket *sudpPkt);
BOOL cg_upnpav_dmr_sudp_pause_receive(CgUpnpSUDPPacket *sudpPkt);

BOOL cg_upnpav_dmr_sudp_test_receive(CgUpnpSUDPPacket *sudpPkt);
#endif






#ifdef  __cplusplus
}
#endif

#endif
