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
*	File: ctrackinfo.h
*
*	Revision:
*
*	13/03/18
*		- first revision
*
******************************************************************/

#ifndef _CG_CLINKCAV_TRACKINFO_H_
#define _CG_CLINKCAV_TRACKINFO_H_

#include <cybergarage/util/cstring.h>
#include <cybergarage/util/clist.h>

#ifdef  __cplusplus
extern "C" {
#endif

/****************************************
* Constant
****************************************/

#define CG_UPNPAV_TRACKINFO_MAXSIZE 512


/****************************************
* Data Type
****************************************/
/**
 * The generic trackinfo struct
 */
typedef struct _CgUpnpAvTrackInfo {
		BOOL headFlag;
		struct _CgUpnpAvTrackInfo *prev;
		struct _CgUpnpAvTrackInfo *next;
		CgString *track;
} CgUpnpAvTrackInfo, CgUpnpAvTrackInfoList;


/****************************************
* Function (TracklInfo)
****************************************/

/**
 * Create a new trackinfo struct
 */
CgUpnpAvTrackInfo *cg_upnpav_trackinfo_new();

/**
 * Delete an trackinfo struct
 *
 * \param dev TrackInfo to delete
 */
void cg_upnpav_trackinfo_delete(CgUpnpAvTrackInfo *trackinfo);

/**
 * Get the next trackinfo in list. Use as an iterator.
 *
 * \param trackinfo Node in a list of trackinfos
 */
#define cg_upnpav_trackinfo_next(trackinfo) (CgUpnpAvTrackInfo *)cg_list_next((CgList *)trackinfo)

/*****************************************************************************
 * Track
 *****************************************************************************/
/**
 * Set the trackinfo's track
 *
 * \param trackinfo TrackInfo in question
 * \param value Track
 */
#define cg_upnpav_trackinfo_settrack(trackinfo, value) cg_string_setvalue(trackinfo->track, value)

/**
 * Get the trackinfo's track
 *
 */
#define cg_upnpav_trackinfo_gettrack(trackinfo) cg_string_getvalue(trackinfo->track)



/*****************************************************************************
 * Function (TrackInfoList)
 *****************************************************************************/

/**
 * Create a new list of trackinfos
 *
 */
CgUpnpAvTrackInfoList *cg_upnpav_trackinfolist_new();

/**
 * Delete a list of trackinfos
 *
 * \param trackinfoList List of trackinfos
 */
void cg_upnpav_trackinfolist_delete(CgUpnpAvTrackInfoList *trackinfoList);

/**
 * \todo Get the  trackinfo from the list for iteration
 *
 * \param trackinfoList List of trackinfos
 * \param trackNo Serial No. of trackinfo
 */
CgUpnpAvTrackInfo *cg_upnpav_trackinfolist_gettrack(CgUpnpAvTrackInfoList *trackinfoList, int trackNo);



/**
 * Clear the contents of an trackinfo list
 *
 * \param trackinfoList List of protocolinfos
 */
#define cg_upnpav_trackinfolist_clear(trackinfoList) cg_list_clear((CgList *)trackinfoList, (CG_LIST_DESTRUCTORFUNC)cg_upnpav_trackinfo_delete)

/**
 * Get the size (number of elements) of an trackinfo list
 *
 * \param trackinfoList List of trackinfos
 */
#define cg_upnpav_trackinfolist_size(trackinfoList) cg_list_size((CgList *)trackinfoList)

/**
 * \todo Get the first trackinfo from the list for iteration
 *
 * \param trackinfoList List of trackinfos
 */
#define cg_upnpav_trackinfolist_gets(trackinfoList) (CgUpnpAvTrackInfo *)cg_list_next((CgList *)trackinfoList)

/**
 * Add an trackinfo to an trackinfo list
 *
 * \param trackinfoList List of trackinfos
 * \param trackinfo TrackInfo to add
 */
#define cg_upnpav_trackinfolist_add(trackinfoList, trackinfo) cg_list_add((CgList *)trackinfoList, (CgList *)trackinfo)

#ifdef  __cplusplus

} /* extern "C" */

#endif

#endif
