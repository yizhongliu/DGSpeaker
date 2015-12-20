/******************************************************************
*
*	
*
*	Copyright (C) 2013 TCL Corporation
*
*       Copyright (C) 2013 TCL Corporation. All rights reserved.
*
*       This is licensed under BSD-style license,
*       see file COPYING.
*
*	File: upnpavrender.h
*
*	Revision:
*
*	18-Apr-13
*		- first revision
*
******************************************************************/

#ifndef _UPNPAVRENDER_H_
#define _UPNPAVRENDER_H_

#include <cybergarage/upnp/std/av/cmediarenderer.h>
#include <gstreamer_0.10/output_gstreamer.h>


#ifdef  __cplusplus
extern "C" {
#endif

#define TCL_DMR_START 0
#define TCL_DMR_STOP  1

int dmr_init(int argc, char **argv);
void dmr_handle(char cmd);

#ifdef  __cplusplus

} /* extern "C" */

#endif

#endif

