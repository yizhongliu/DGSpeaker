/******************************************************************
*
*
*	Copyright (C) Liu Liming 2013
*
*       Copyright (C) 2013 TCL Corporation. All rights reserved.
*
*       This is licensed under BSD-style license,
*       see file COPYING.
*
*	File: totem_m3u_parser.h
*
*	Revision:
*
*	13/03/18
*		- first revision
*
*
******************************************************************/

#ifndef _TOTEM_M3U_PARSER_H_
#define _TOTEM_M3U_PARSER_H_

#include <cybergarage/util/cstring.h>
#include <cybergarage/util/clist.h>
#include <cybergarage/upnp/std/av/cmediarenderer.h>



#ifdef  __cplusplus
extern "C" {
#endif

/****************************************
* Data Type
****************************************/
/**
 * The generic Trackinfo struct
 */






/****************************************
* Function
****************************************/


/**
 * parse a .m3u file
 *
 * @param uri the path of file
 */
void totem_m3u_parse_uri(char *uri, CgUpnpAvRenderer *dmr);



#ifdef  __cplusplus
}
#endif

#endif








