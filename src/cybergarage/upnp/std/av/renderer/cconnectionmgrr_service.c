/************************************************************
*
*	CyberLink for C
*
*	Copyright (C) Satoshi Konno 2009
*
*	File: cconnectionmgr_service.c
*
*	Revision:
*		2009/06/11
*        - first release.
*
************************************************************/

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <cybergarage/upnp/std/av/cmediarenderer.h>
#include <cybergarage/upnp/std/av/ccontent.h>
#include <cybergarage/upnp/std/av/cresource.h>
#include <cybergarage/upnp/std/av/cprotocolinfo.h>  //add by llm 20130123
#include <gstreamer_0.10/output_gstreamer.h>   //add by llm 20130411

/****************************************
* Service Description (Connection Manager)
****************************************/


/****************************************
* cg_upnpav_dmr_conmgr_actionreceived
****************************************/

BOOL cg_upnpav_dmr_conmgr_actionreceived(CgUpnpAction *action)
{
	CgUpnpAvRenderer *dmr;
	CgUpnpDevice *dev;
	char *actionName;
	CgUpnpArgument *arg;
	CgString *protocolInfos;
	CgUpnpAvProtocolInfo *protocolInfo;
	
	actionName = cg_upnp_action_getname(action);
	if (cg_strlen(actionName) <= 0)
		return FALSE;

	printf("--------------------------------------\n");
	printf("[conmgr] actionName : %s\n",actionName);

	dev = (CgUpnpDevice *)cg_upnp_service_getdevice(cg_upnp_action_getservice(action));
	if (!dev) 
		return FALSE;

	dmr = (CgUpnpAvRenderer *)cg_upnp_device_getuserdata(dev);
	if (!dmr)
		return FALSE;
	
	/* GetProtocolInfo*/
	if (cg_streq(actionName, CG_UPNPAV_DMR_CONNECTIONMANAGER_GET_PROTOCOL_INFO)) 
	{
	    //sink
		arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_CONNECTIONMANAGER_SINK);
		if (!arg)
			return FALSE;
		cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_connectionmgrr_getstatevariable(dmr, CG_UPNPAV_DMR_CONNECTIONMANAGER_SINKPROTOCOLINFO));

		return TRUE;
	}

	/*GetCurrentConnectionIDs*/
	if (cg_streq(actionName, CG_UPNPAV_DMR_CONNECTIONMANAGER_GET_CURRENT_CONNECTION_IDS)) 
	{
	    arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_CONNECTIONMANAGER_CONNECTIONIDS);
		if (!arg)
			return FALSE;
		cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_connectionmgrr_getstatevariable(dmr, CG_UPNPAV_DMR_CONNECTIONMANAGER_CURRENTCONNECTIONIDS));

		return TRUE;
	}

	/*GetCurrentConnectionInfo*/
	if (cg_streq(actionName, CG_UPNPAV_DMR_CONNECTIONMANAGER_GET_CURRENT_CONNECTION_INFO)) 
	{
	    arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_CONNECTIONMANAGER_CONNECTIONID);
		if (!arg)
			return FALSE;

		/*RcsID*/
		arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_CONNECTIONMANAGER_RCSID);
		if (!arg)
			return FALSE;
		cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_connectionmgrr_getstatevariable(dmr, CG_UPNPAV_DMR_CONNECTIONMANAGER_AAT_RCSID));

        /*AVTransportID*/
		arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_CONNECTIONMANAGER_AVTRANSPORTID);
		if (!arg)
			return FALSE;
		cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_connectionmgrr_getstatevariable(dmr, CG_UPNPAV_DMR_CONNECTIONMANAGER_AAT_AVTRANSPORTID));

        /*ProtocolInfo*/
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_CONNECTIONMANAGER_PROTOCOLINFO);
		if (!arg)
			return FALSE;
		cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_connectionmgrr_getstatevariable(dmr, CG_UPNPAV_DMR_CONNECTIONMANAGER_AAT_PROTOCOLINFO));

		/*PeerConnectionManager*/
		#if 0
		arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_CONNECTIONMANAGER_PEERCONNECTIONMANAGER);
		if (!arg)
			return FALSE;
		cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_getpeerconnectionmanager(dmr));
        #endif

		/*PeerConnectionID*/
		arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_CONNECTIONMANAGER_PEERCONNECTIONID);
		if (!arg)
			return FALSE;
		cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_connectionmgrr_getstatevariable(dmr, CG_UPNPAV_DMR_CONNECTIONMANAGER_AAT_CONNECTIONID));

        /*Direction*/
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_CONNECTIONMANAGER_DIRECTION);
		if (!arg)
			return FALSE;
		cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_connectionmgrr_getstatevariable(dmr, CG_UPNPAV_DMR_CONNECTIONMANAGER_AAT_DIRECTION));

        /*Status*/
	    arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_CONNECTIONMANAGER_STATUS);
		if (!arg)
			return FALSE;
		cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_connectionmgrr_getstatevariable(dmr, CG_UPNPAV_DMR_CONNECTIONMANAGER_AAT_STATUS));
		
		return TRUE;
	}
	

	return FALSE;
}

/****************************************
 * cg_upnpav_dmr_conmgr_queryreceived
 ****************************************/

BOOL cg_upnpav_dmr_conmgr_queryreceived(CgUpnpStateVariable *statVar)
{
	return FALSE;
}

/****************************************
* cg_upnpav_dmr_conmgr_init
****************************************/

BOOL cg_upnpav_dmr_conmgr_init(CgUpnpAvRenderer *dmr)
{
	CgUpnpDevice *dev;
	CgUpnpService *service;
	CgUpnpAction *action;

	dev = cg_upnpav_dmr_getdevice(dmr);
	if (!dev)
		return FALSE;

	service = cg_upnp_device_getservicebytype(dev, CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_TYPE);
	if (!service)
		return FALSE;

    if(cg_upnp_service_loaddescriptionfile(service, CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_DESCRIPTION) == FALSE)
    {
        printf("parase cconetionManager error !!! \n");
		return FALSE;
    }
	#if 0
	if (cg_upnp_service_parsedescription(service, CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_DESCRIPTION, cg_strlen(CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_DESCRIPTION)) == FALSE)
		return FALSE;
    #endif
	
	cg_upnp_service_setuserdata(service, dmr);
	for (action=cg_upnp_service_getactions(service); action; action=cg_upnp_action_next(action))
		cg_upnp_action_setuserdata(action, dmr);
    
	/*inti variable*/ 
    cg_upnpav_dmr_connectionmgrr_setstatevariable(dmr, 
                  CG_UPNPAV_DMR_CONNECTIONMANAGER_CURRENTCONNECTIONIDS, "0");
	cg_upnpav_dmr_connectionmgrr_setstatevariable(dmr, 
		          CG_UPNPAV_DMR_CONNECTIONMANAGER_AAT_RCSID, "0");
	cg_upnpav_dmr_connectionmgrr_setstatevariable(dmr, 
		          CG_UPNPAV_DMR_CONNECTIONMANAGER_AAT_AVTRANSPORTID, "0");
	cg_upnpav_dmr_connectionmgrr_setstatevariable(dmr, 
		          CG_UPNPAV_DMR_CONNECTIONMANAGER_AAT_PROTOCOLINFO, "");
	cg_upnpav_dmr_connectionmgrr_setstatevariable(dmr, 
		          CG_UPNPAV_DMR_CONNECTIONMANAGER_AAT_CONNECTIONMANAGER, "/");
	cg_upnpav_dmr_connectionmgrr_setstatevariable(dmr, 
		          CG_UPNPAV_DMR_CONNECTIONMANAGER_AAT_CONNECTIONID, "-1");
	cg_upnpav_dmr_connectionmgrr_setstatevariable(dmr, 
		          CG_UPNPAV_DMR_CONNECTIONMANAGER_AAT_DIRECTION, "Input");
	cg_upnpav_dmr_connectionmgrr_setstatevariable(dmr, 
		          CG_UPNPAV_DMR_CONNECTIONMANAGER_AAT_STATUS, "Unknown");
	
	return TRUE;
}


/****************************************
 * cg_upnpav_dmr_setsinkprotocolinfo
 ****************************************/

void cg_upnpav_dmr_setsinkprotocolinfo(CgUpnpAvRenderer *dmr, char *value)
{
	CgUpnpService *service;
	CgUpnpStateVariable *stateVar;
	
	service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_TYPE);
	stateVar = cg_upnp_service_getstatevariablebyname(service, CG_UPNPAV_DMR_CONNECTIONMANAGER_SINKPROTOCOLINFO);
	cg_upnp_statevariable_setvalue(stateVar, value);
}

/****************************************
 * cg_upnpav_dmr_getsinkprotocolinfo
 ****************************************/

char *cg_upnpav_dmr_getsinkprotocolinfo(CgUpnpAvRenderer *dmr)
{
	CgUpnpService *service;
	CgUpnpStateVariable *stateVar;
	
	service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_TYPE);
	stateVar = cg_upnp_service_getstatevariablebyname(service, CG_UPNPAV_DMR_CONNECTIONMANAGER_SINKPROTOCOLINFO);
	return cg_upnp_statevariable_getvalue(stateVar);
}

/****************************************
 * cg_upnpav_dmr_setsourceprotocolinfo
 ****************************************/

void cg_upnpav_dmr_setsourceprotocolinfo(CgUpnpAvRenderer *dmr, char *value)
{
	CgUpnpService *service;
	CgUpnpStateVariable *stateVar;
	
	service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_TYPE);
	stateVar = cg_upnp_service_getstatevariablebyname(service, CG_UPNPAV_DMR_CONNECTIONMANAGER_SOURCEPROTOCOLINFO);
	cg_upnp_statevariable_setvalue(stateVar, value);
}

/****************************************
 * cg_upnpav_dmr_getsourceprotocolinfo
 ****************************************/

char *cg_upnpav_dmr_getsourceprotocolinfo(CgUpnpAvRenderer *dmr)
{
	CgUpnpService *service;
	CgUpnpStateVariable *stateVar;
	
	service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_TYPE);
	stateVar = cg_upnp_service_getstatevariablebyname(service, CG_UPNPAV_DMR_CONNECTIONMANAGER_SOURCEPROTOCOLINFO);
	return cg_upnp_statevariable_getvalue(stateVar);
}


 
/****************************************
 * cg_upnpav_dmr_setcurrentconnectionids
 ****************************************/

void cg_upnpav_dmr_setcurrentconnectionids(CgUpnpAvRenderer *dmr, char *value)
{
	CgUpnpService *service;
	CgUpnpStateVariable *stateVar;

	service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_TYPE);
	stateVar = cg_upnp_service_getstatevariablebyname(service, CG_UPNPAV_DMR_CONNECTIONMANAGER_CURRENTCONNECTIONIDS);
	cg_upnp_statevariable_setvalue(stateVar, value);
}

/****************************************
 * cg_upnpav_dmr_getcurrentconnectionids
 ****************************************/

char *cg_upnpav_dmr_getcurrentconnectionids(CgUpnpAvRenderer *dmr)
{
	CgUpnpService *service;
	CgUpnpStateVariable *stateVar;

	service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_TYPE);
	stateVar = cg_upnp_service_getstatevariablebyname(service, CG_UPNPAV_DMR_CONNECTIONMANAGER_CURRENTCONNECTIONIDS);
	return cg_upnp_statevariable_getvalue(stateVar);
}





/****************************************
 * cg_upnpav_dmr_setprotocolinfo
 ****************************************/
void cg_upnpav_dmr_setprotocolinfo(CgUpnpAvRenderer *dmr, char *value)
{
    CgUpnpService *service;
	CgUpnpStateVariable *stateVar;

	service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_TYPE);
	stateVar = cg_upnp_service_getstatevariablebyname(service, CG_UPNPAV_DMR_CONNECTIONMANAGER_AAT_PROTOCOLINFO);
	cg_upnp_statevariable_setvalue(stateVar, value);
}

/****************************************
 * cg_upnpav_dmr_getprotocolinfo
 ****************************************/
char *cg_upnpav_dmr_getprotocolinfo(CgUpnpAvRenderer *dmr)
{
    CgUpnpService *service;
	CgUpnpStateVariable *stateVar;

	service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_TYPE);
	stateVar = cg_upnp_service_getstatevariablebyname(service, CG_UPNPAV_DMR_CONNECTIONMANAGER_AAT_PROTOCOLINFO);
	return cg_upnp_statevariable_getvalue(stateVar);
}

/****************************************
 * cg_upnpav_dmr_connectionmgrr_variable_init
 ****************************************/
void cg_upnpav_dmr_connectionmgrr_variable_init(CgUpnpAvRenderer *dmr)
{ 

	cg_upnpav_dmr_connectionmgrr_setstatevariable(dmr, 
		CG_UPNPAV_DMR_CONNECTIONMANAGER_AAT_PROTOCOLINFO, "");
	cg_upnpav_dmr_connectionmgrr_setstatevariable(dmr, 
		CG_UPNPAV_DMR_CONNECTIONMANAGER_AAT_STATUS, "Unknown");
	
}


/****************************************
 * cg_upnpav_dmr_connectionmgrr_setstatevariable
 ****************************************/
void cg_upnpav_dmr_connectionmgrr_setstatevariable(CgUpnpAvRenderer *dmr,char *statevariable, char *value)
{
	CgUpnpService *service;
	CgUpnpStateVariable *stateVar;

	service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_TYPE);
	stateVar = cg_upnp_service_getstatevariablebyname(service, statevariable);
	cg_upnp_statevariable_setvalue(stateVar, value);
}

/****************************************
 * cg_upnpav_dmr_connectionmgrr_getstatevariable
 ****************************************/
char *cg_upnpav_dmr_connectionmgrr_getstatevariable(CgUpnpAvRenderer *dmr, char *statevariable)
{
	CgUpnpService *service;
	CgUpnpStateVariable *stateVar;

	service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_TYPE);
	stateVar = cg_upnp_service_getstatevariablebyname(service, statevariable);
	return cg_upnp_statevariable_getvalue(stateVar);
}




