/************************************************************
*
*	CyberLink for C
*
*	Copyright (C) Satoshi Konno 2009
*
*	File: crenderingcontrol_service.c
*
*	Revision:
*       2008/06/16
*               - first release.
*
************************************************************/

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <cybergarage/upnp/std/av/cmediarenderer.h>

/****************************************
* Service Description (Rendering Control)
****************************************/



CG_UPNPAV_VOLUME_LISTNER setVolumeListner = NULL;
CG_UPNPAV_VOLUME_GETLISTNER getVolumeListner = NULL;

CG_UPNPAV_MUTE_LISTNER setMuteListner = NULL;
CG_UPNPAV_UNMUTE_LISTNER setUnMuteListner = NULL;




/****************************************
* cg_upnpav_dmr_renderingctrl_actionreceived
****************************************/

BOOL cg_upnpav_dmr_renderingctrl_actionreceived(CgUpnpAction *action)
{
	CgUpnpAvRenderer *dmr;
	CgUpnpDevice *dev;
	char *actionName;
	CgUpnpArgument *arg;
	int iVolume;
	char *lastChange;

	actionName = cg_upnp_action_getname(action);
	if (cg_strlen(actionName) <= 0)
		return FALSE;

	printf("--------------------------------------\n");
	printf("[renderingctrl] actionName : %s\n",actionName);

	dev = (CgUpnpDevice *)cg_upnp_service_getdevice(cg_upnp_action_getservice(action));
	if (!dev) 
		return FALSE;

	dmr = (CgUpnpAvRenderer *)cg_upnp_device_getuserdata(dev);
	if (!dmr)
		return FALSE;
#if 1
	/* GetMute*/
	if (cg_streq(actionName, CG_UPNPAV_DMR_RENDERINGCONTROL_GETMUTE)) 
	{
		arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_RENDERINGCONTROL_CURRENTMUTE);
		if (!arg)
			return FALSE;
		
		cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_rendercontrol_getstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_MUTE));

		return TRUE;
	}
#endif

#if 1
	/* SetMute*/
	if (cg_streq(actionName, CG_UPNPAV_DMR_RENDERINGCONTROL_SETMUTE)) 
	{

	    //Channel
	    arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_RENDERINGCONTROL_CHANNEL);
		if (!arg)
			return FALSE;

		cg_upnpav_dmr_rendercontrol_setstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_AAT_CHANNEL, arg->value->value);

		//DesiredMute
		arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_RENDERINGCONTROL_DESIREDMUTE);
		if (!arg)
			return FALSE;

		printf("desired Mute : %s\n", arg->value->value);

        /*set mute*/
		if(cg_streq(arg->value->value, "0"))
		{
		    if(setUnMuteListner != NULL)
		    {
		         printf("turn Mute on\n");
		         setUnMuteListner();
		    }
		}
        else if(cg_streq(arg->value->value, "1"))
        {
            if(setMuteListner != NULL)
		    {
		         printf("turn Mute off\n");
		         setMuteListner();
		    }
        }
		

		cg_upnpav_dmr_rendercontrol_setstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_MUTE, arg->value->value);

        
		/*set lastchange*/
		#if 0
		asprintf(&lastChange,
		 "<Event xmlns = \"urn:schemas-upnp-org:metadata-1-0/AVT/\"><InstanceID val=\"0\"><%s val=\"%s\"/><%s val=\"%s\"/></InstanceID></Event>",
		 CG_UPNPAV_DMR_RENDERINGCONTROL_AAT_CHANNEL, cg_upnpav_dmr_rendercontrol_getstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_AAT_CHANNEL),
		 CG_UPNPAV_DMR_RENDERINGCONTROL_MUTE, cg_upnpav_dmr_rendercontrol_getstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_MUTE));
	    cg_upnpav_dmr_setrenderingcontrollastchange(dmr, lastChange);
	    free(lastChange);
		#endif
		cg_upnpav_dmr_rendercontrol_change_lastchange(dmr);

		return TRUE;
	}
#endif	

	/* GetVolume */
	if (cg_streq(actionName, CG_UPNPAV_DMR_RENDERINGCONTROL_GETVOLUME)) 
    {
		arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_RENDERINGCONTROL_CURRENTVOLUME);
		if (!arg)
			return FALSE;
		cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_rendercontrol_getstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_VOLUME));
        printf("get volume = %s\n", cg_upnpav_dmr_rendercontrol_getstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_VOLUME)); //llm debug
		return TRUE;
	}
	
	/* SetVolume */
	if (cg_streq(actionName, CG_UPNPAV_DMR_RENDERINGCONTROL_SETVOLUME)) 
    {
        //Channel
        arg = cg_upnp_action_getargumentbyname(action, "Channel");
		if (!arg)
			return FALSE;

		cg_upnpav_dmr_rendercontrol_setstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_AAT_CHANNEL, arg->value->value);

		
		//DesiredVolume
		arg = cg_upnp_action_getargumentbyname(action, "DesiredVolume");
		if (!arg)
			return FALSE;
		cg_upnpav_dmr_rendercontrol_setstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_VOLUME, arg->value->value);

        /*set volume*/
		iVolume = cg_atoi(arg->value->value);
		iVolume = iVolume * 30 / 100;

		if(setVolumeListner != NULL)
		{
		    setVolumeListner((unsigned char)iVolume);
		}
		/**/

        
#if 0
        /*set lastchange*/
		asprintf(&lastChange,
		 "<Event xmlns = \"urn:schemas-upnp-org:metadata-1-0/AVT/\"><InstanceID val=\"0\"><Volume channel=\"%s\"  val=\"%s\"/></InstanceID></Event>",
		 cg_upnpav_dmr_rendercontrol_getstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_AAT_CHANNEL), 
		 cg_upnpav_dmr_rendercontrol_getstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_VOLUME));
	    cg_upnpav_dmr_setrenderingcontrollastchange(dmr, lastChange);
	    free(lastChange);
#endif
        cg_upnpav_dmr_rendercontrol_change_lastchange(dmr);
		

		return TRUE;
		
	}

	/*ListPresets*/
	if (cg_streq(actionName, CG_UPNPAV_DMR_RENDERINGCONTROL_LISTPRESETS)) 
	{
	
		//CurrentPresetNameList
		arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_RENDERINGCONTROL_CURRENTPRESETNAMELIST);
		if (!arg)
			return FALSE;
		cg_upnp_argument_setvalue(arg, cg_upnpav_dmr_rendercontrol_getstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_PRESETNAMELIST));
		
		return TRUE;
	}

	/*SelectPreset*/
	if (cg_streq(actionName, CG_UPNPAV_DMR_RENDERINGCONTROL_SELECTPRESET)) 
	{	

	    arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_RENDERINGCONTROL_PRESETNAME);
		if (!arg)
			return FALSE;

		cg_upnpav_dmr_rendercontrol_setstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_AAT_PRESETNAME, arg->value->value);
        #if 0
		asprintf(&lastChange,
		 "<Event xmlns = \"urn:schemas-upnp-org:metadata-1-0/AVT/\"><InstanceID val=\"0\"><A_ARG_TYPE_PresetName val=\"%s\"/></InstanceID></Event>",
		 cg_upnpav_dmr_rendercontrol_getstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_AAT_PRESETNAME));
	    cg_upnpav_dmr_setrenderingcontrollastchange(dmr, lastChange);
		free(lastChange);
		#endif
        cg_upnpav_dmr_rendercontrol_change_lastchange(dmr);
		
		return TRUE;
	}
}

/****************************************
 * cg_upnpav_dmr_renderingctrl_actionreceived
 ****************************************/

BOOL cg_upnpav_dmr_renderingctrl_queryreceived(CgUpnpStateVariable *statVar)
{
	return FALSE;
}

/****************************************
* cg_upnpav_dmr_renderingctrl_init
****************************************/

BOOL cg_upnpav_dmr_renderingctrl_init(CgUpnpAvRenderer *dmr)
{
	CgUpnpDevice *dev;
	CgUpnpService *service;
	CgUpnpAction *action;
	int iVolume;
	char strVolume[3];

	dev = cg_upnpav_dmr_getdevice(dmr);
	if (!dev)
		return FALSE;

	service = cg_upnp_device_getservicebytype(dev, CG_UPNPAV_DMR_RENDERINGCONTROL_SERVICE_TYPE);
	if (!service)
		return FALSE;
	
	if(cg_upnp_service_loaddescriptionfile(service, CG_UPNPAV_DMR_RENDERINGCONTROL_SERVICE_DESCRIPTION) == FALSE)
    {
        printf("parase cavtransport error !!! \n");
		return FALSE;
    }

	#if 0
	if (cg_upnp_service_parsedescription(service, CG_UPNPAV_DMR_RENDERINGCONTROL_SERVICE_DESCRIPTION, cg_strlen(CG_UPNPAV_DMR_RENDERINGCONTROL_SERVICE_DESCRIPTION)) == FALSE)
		return FALSE;
    #endif
	
	cg_upnp_service_setuserdata(service, dmr);
	for (action=cg_upnp_service_getactions(service); action; action=cg_upnp_action_next(action))
		cg_upnp_action_setuserdata(action, dmr);

    /*init volume*/ 
	if(getVolumeListner != NULL)
	{
	    iVolume = getVolumeListner();
	    iVolume = iVolume * 100 /30;

	    cg_int2str(iVolume, strVolume, sizeof(strVolume));

	    cg_upnpav_dmr_rendercontrol_setstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_VOLUME, strVolume);
	}
	else
	{
	    cg_upnpav_dmr_rendercontrol_setstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_VOLUME, "30");
	}
	/**/
	
    cg_upnpav_dmr_rendercontrol_setstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_AAT_CHANNEL, "Master");
	cg_upnpav_dmr_rendercontrol_setstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_PRESETNAMELIST, "FactoryDefaults");   
	cg_upnpav_dmr_rendercontrol_setstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_AAT_PRESETNAME, "FactoryDefaults");
	cg_upnpav_dmr_rendercontrol_setstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_MUTE, "0");



    cg_upnpav_dmr_rendercontrol_change_lastchange(dmr);
	
	return TRUE;
}

/****************************************
 * cg_upnpav_dmr_setrenderingcontrollastchange
 ****************************************/

void cg_upnpav_dmr_setrenderingcontrollastchange(CgUpnpAvRenderer *dmr, char *value)
{
	CgUpnpService *service;
	CgUpnpStateVariable *stateVar;
	
	service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_RENDERINGCONTROL_SERVICE_TYPE);
	stateVar = cg_upnp_service_getstatevariablebyname(service, CG_UPNPAV_DMR_RENDERINGCONTROL_LASTCHANGE);
	cg_upnp_statevariable_setvalue(stateVar, value);
}

/****************************************
 * cg_upnpav_dmr_getrenderingcontrollastchange
 ****************************************/

char *cg_upnpav_dmr_getrenderingcontrollastchange(CgUpnpAvRenderer *dmr)
{
	CgUpnpService *service;
	CgUpnpStateVariable *stateVar;
	
	service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_AVTRANSPORT_SERVICE_TYPE);
	stateVar = cg_upnp_service_getstatevariablebyname(service, CG_UPNPAV_DMR_AVTRANSPORT_LASTCHANGE);
	return cg_upnp_statevariable_getvalue(stateVar);
}


/****************************************
 * cg_upnpav_dmr_rendercontrol_setstatevariable
 ****************************************/
void cg_upnpav_dmr_rendercontrol_setstatevariable(CgUpnpAvRenderer *dmr,char *statevariable, char *value)
{
	CgUpnpService *service;
	CgUpnpStateVariable *stateVar;

	service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_RENDERINGCONTROL_SERVICE_TYPE);
	stateVar = cg_upnp_service_getstatevariablebyname(service, statevariable);
	cg_upnp_statevariable_setvalue(stateVar, value);
}

/****************************************
 * cg_upnpav_dmr_rendercontrol_getstatevariable
 ****************************************/
char *cg_upnpav_dmr_rendercontrol_getstatevariable(CgUpnpAvRenderer *dmr, char *statevariable)
{
	CgUpnpService *service;
	CgUpnpStateVariable *stateVar;

	service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_RENDERINGCONTROL_SERVICE_TYPE);
	stateVar = cg_upnp_service_getstatevariablebyname(service, statevariable);
	return cg_upnp_statevariable_getvalue(stateVar);
}

/****************************************
 * cg_upnpav_dmr_rendercontrol_change_lastchange
 ****************************************/
void cg_upnpav_dmr_rendercontrol_change_lastchange(CgUpnpAvRenderer *dmr)
{
	char *lastChange = NULL;

	asprintf(&lastChange,
		 "<Event xmlns = \"urn:schemas-upnp-org:metadata-1-0/RCS/\">\
		 <InstanceID val=\"0\">\
		 <Volume channel=\"%s\"  val=\"%s\"/>\
		 <Mute channel=\"%s\"  val=\"%s\"/>\
		 <PresetNameList val=\"%s\"/>\
		 <A_ARG_TYPE_PresetName val=\"%s\"/>\
		 </InstanceID></Event>",
		 cg_upnpav_dmr_rendercontrol_getstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_AAT_CHANNEL),
		 cg_upnpav_dmr_rendercontrol_getstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_VOLUME),
		 cg_upnpav_dmr_rendercontrol_getstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_AAT_CHANNEL),
		 cg_upnpav_dmr_rendercontrol_getstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_MUTE),
		 cg_upnpav_dmr_rendercontrol_getstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_PRESETNAMELIST),
		 cg_upnpav_dmr_rendercontrol_getstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_AAT_PRESETNAME));
	
	    cg_upnpav_dmr_setrenderingcontrollastchange(dmr, lastChange);
		free(lastChange);
	
}

/****************************************
 * cg_upnpav_dmr_rendercontrol_variable_init
 ****************************************/
void cg_upnpav_dmr_rendercontrol_variable_init(CgUpnpAvRenderer *dmr)
{
    int iVolume;
	char strVolume[3];
	
     /*init volume*/ 
	if(getVolumeListner != NULL)
	{
	    iVolume = getVolumeListner();
	    iVolume = iVolume * 100 /30;

	    cg_int2str(iVolume, strVolume, sizeof(strVolume));

	    cg_upnpav_dmr_rendercontrol_setstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_VOLUME, strVolume);
	}
	else
	{
	    cg_upnpav_dmr_rendercontrol_setstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_VOLUME, "30");
	}
	/**/
	
    cg_upnpav_dmr_rendercontrol_setstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_AAT_CHANNEL, "Master");
	cg_upnpav_dmr_rendercontrol_setstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_PRESETNAMELIST, "FactoryDefaults");   
	cg_upnpav_dmr_rendercontrol_setstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_AAT_PRESETNAME, "FactoryDefaults");
	cg_upnpav_dmr_rendercontrol_setstatevariable(dmr, CG_UPNPAV_DMR_RENDERINGCONTROL_MUTE, "0");

	cg_upnpav_dmr_rendercontrol_change_lastchange(dmr);
}



