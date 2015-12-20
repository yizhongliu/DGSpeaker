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
*	File: ctrackinfo.c
*
*	Revision:
*
*	07/27/09
*		- first revision
*
******************************************************************/

#include <cybergarage/upnp/std/av/ctrackinfo.h>
#include <cybergarage/util/clog.h>



/****************************************
* cg_upnpav_trackinfo_new
****************************************/

CgUpnpAvTrackInfo *cg_upnpav_trackinfo_new()
{
	CgUpnpAvTrackInfo *info;

	cg_log_debug_l4("Entering...\n");

	info = (CgUpnpAvTrackInfo *)malloc(sizeof(CgUpnpAvTrackInfo));

	if  ( NULL != info ) {
		cg_list_node_init((CgList *)info);

		info->track = cg_string_new();
		cg_upnpav_trackinfo_settrack(info, "0");
    }
	cg_log_debug_l4("Leaving...\n");

	return info;
}

/****************************************
* cg_upnpav_trackinfo_delete
****************************************/

void cg_upnpav_trackinfo_delete(CgUpnpAvTrackInfo *info)
{
	cg_log_debug_l4("Entering...\n");

	cg_list_remove((CgList *)info);

	if (info->track)
		cg_string_delete(info->track);

	free(info);

	cg_log_debug_l4("Leaving...\n");
}


