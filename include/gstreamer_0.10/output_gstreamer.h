/* output_gstreamer.h - Definitions for GStreamer output module
 *
 * Copyright (C) 2005-2007   Ivo Clarysse
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

#ifndef _OUTPUT_GSTREAMER_H
#define _OUTPUT_GSTREAMER_H

#include <cybergarage/upnp/std/av/cmediarenderer.h>
#include <gst/gst.h>



/*add by llm 20130127*/
typedef struct _mime_type
{
	const char *cmime_type;
	struct _mime_type *next;
}mime_type;

typedef struct _MULTICAST_ATTR
{
	char *multicastIp;
	gint  multicastPort;
}MULTICAST_ATTR;

/*end*/

int output_gstreamer_add_options(GOptionContext *ctx);
int output_gstreamer_playbin_init(CgUpnpAvRenderer *dmr); //modify by llm
void output_set_uri(const char *uri);
int output_play(CgUpnpAvRenderer *dmr, int index); //modify by llm 20130321
int output_play2(char *uri);
int output_stop(void);
int output_pause(void);
int output_loop(void);
int output_seek(gdouble percentage);  //add by llm
int output_playback(void); //add by llm 20130508 change pause to play

int output_udp_pause(CgUpnpAvRenderer *dmr);



int process_cmdline(int argc, char **argv);
void scan_mime_list(CgUpnpAvRenderer* dmr);
/*add by llm*/ 
mime_type* get_mime_type();  
int output_gstreamer_rtpbin_mpa_test(CgUpnpAvRenderer *dmr);
int output_creat_playbinloop();
int output_close_playbinloop();
int output_creat_rtploop();
int output_close_rtploop();


int output_gstreamer_pipeline_init(CgUpnpAvRenderer *dmr);
int output_pipeline_loop(void);







#endif /*  _OUTPUT_GSTREAMER_H */
