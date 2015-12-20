/******************************************************************
*
*	CyberLink for C
*
*	Copyright (C) Liu Liming 2013
*
*		Copyright (C) 2013 TCL Corporation. All rights reserved.
*
*		This is licensed under BSD-style license,
*		see file COPYING.
*
*	File: ctypeconver.h
*
*	Revision:
*
*	13/03/18
*		- first revision
*
******************************************************************/


#ifndef _CG_UPNPAV_TYPECONVER_H_
#define _CG_UPNPAV_TYPECONVER_H_

#ifdef  __cplusplus
extern "C" {
#endif

/****************************************
* Function
****************************************/


/**
 * Transform time format "00:00:00" to int
 *
 * \param time value of time in string type
 * \return The time value in int type
 */
int cg_time_to_int(char *time);


/**
 * Transform string to int
 *
 */
int cg_atoi(char *str);

/**
 * parse multicast ip and port from inUri
 *
 * \param inUri  input uri
 * \param outMulticastIP output ip
 * \param outPort output  port
 * \return 0:succeed  other: fail
 */
int cg_parse_multicastUri(const char* inUri, char* outMulticastIP, char* outPort);



#ifdef  __cplusplus
}
#endif

#endif
