/******************************************************************
*
*	CyberLink for C
*
*	Copyright (C) Liu Liming 2013
*
*       Copyright (C) 2013 TCL Corporation. All rights reserved.
*
*       This is licensed under BSD-style license,
*       see file COPYING.
*
*	File: ctrackinfolist.c
*
*	Revision:
*
*	13/03/18
*		- first revision
*
******************************************************************/

#include <cybergarage/upnp/std/av/ctrackinfo.h>
#include <cybergarage/util/clog.h>

/****************************************
* cg_upnp_trackinflist_new
****************************************/

CgUpnpAvTrackInfoList *cg_upnpav_trackinfolist_new()
{
	CgUpnpAvTrackInfo *trackinfoList;

	cg_log_debug_l4("Entering...\n");

	trackinfoList = (CgUpnpAvTrackInfo *)malloc(sizeof(CgUpnpAvTrackInfo));

	if ( NULL != trackinfoList )
		cg_list_header_init((CgList *)trackinfoList);

	return trackinfoList;

	cg_log_debug_l4("Leaving...\n");
}

/****************************************
* cg_upnpav_trackinfor_delete
****************************************/

void cg_upnpav_trackinfolist_delete(CgUpnpAvTrackInfoList *trackinfoList)
{
	cg_log_debug_l4("Entering...\n");

	cg_list_remove((CgList *)trackinfoList);
	free(trackinfoList);

	cg_log_debug_l4("Leaving...\n");
}

/****************************************
* cg_upnpav_trackinfolist_gettrack
****************************************/
CgUpnpAvTrackInfo *cg_upnpav_trackinfolist_gettrack(CgUpnpAvTrackInfoList *trackinfoList, int trackNo)
{
    CgUpnpAvTrackInfo *trackInfo;
	int currentTrack = 1;
		
    cg_log_debug_l4("Entering...\n");

	if(trackinfoList == NULL)
	{
	     printf("%s %d %s trackInfoList is empty\n",__FILE__, __LINE__, __PRETTY_FUNCTION__);
		 return NULL;
	}
	if(trackNo > cg_upnpav_trackinfolist_size(trackinfoList))
	{
	     printf("%s %d %s trackNo too large trackNo:%d, size:%d\n",__FILE__, __LINE__, __PRETTY_FUNCTION__,trackNo, cg_upnpav_trackinfolist_size(trackinfoList));
		 return NULL;
	}

	for (trackInfo = cg_upnpav_trackinfolist_gets(trackinfoList); currentTrack<trackNo; currentTrack++)
	{
	    trackInfo = cg_upnpav_trackinfo_next(trackInfo);
	}

	return trackInfo;

	cg_log_debug_l4("Leaving...\n");
}


