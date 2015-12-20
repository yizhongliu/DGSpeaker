noinst_PROGRAMS = upnpavrender


PWD=$(shell pwd)

#GSTREAMER_ENV=$(PWD)/../../../../../dep_lib

#GST_CFLAGS = -I$(GSTREAMER_ENV)/include/gstreamer-0.10 -I$(GSTREAMER_ENV)/include/glib-2.0 -I$(GSTREAMER_ENV)/lib/glib-2.0/include -I$(GSTREAMER_ENV)/include/libxml2 -L$(GSTREAMER_ENV)/lib

#GST_CFLAGS = -I/opt/buildroot-gcc342/include/gstreamer-0.10 -I/opt/buildroot-gcc342/include/glib-2.0 -I/opt/buildroot-gcc342/include/glib-2.0/glib -I/opt/buildroot-gcc342/lib/glib-2.0/include -I/opt/buildroot-gcc342/include/libxml2

#GST_CFLAGS = -I/usr/include/gstreamer-0.10 -I/usr/include/glib-2.0 -I/usr/lib/i386-linux-gnu/glib-2.0/include -I/usr/include/libxml2
#GST_LIBS = -lpthread -lgstreamer-0.10 -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lrt -lglib-2.0 -lxml2
# -ltotem-plparser

#INCLUDES = -I$(top_srcdir)/include -I../ -I../../common  $(GST_CFLAGS)

INCLUDES = -I$(top_srcdir)/include -I../ -I../../../common 
noinst_HEADERS = 
#        ../logging.h  
#	../../common/ctypeconver.h

upnpavrender_SOURCES = \
	../upnpavrender.c \
	../upnpavrender_main.c 
#        ../cmediarenderer_device.c 
#	../../../src/gstreamer_0.10/output_gstreamer.c 
#        ../../../src/cybergarage/upnp/std/av/renderer/cavtransport_service.c 
#        ../../../src/cybergarage/upnp/std/av/renderer/cconnectionmgrr_service.c 
#        ../../../src/cybergarage/upnp/std/av/renderer/crenderingcontrol_service.c 
#        ../../../src/cybergarage/upnp/std/av/cprotocolinfo_list.c 
#        ../../../src/cybergarage/upnp/std/av/cprotocolinfo.c 
#        ../../../src/cybergarage/upnp/std/av/ctrackinfo_list.c 
#        ../../../src/cybergarage/upnp/std/av/ctrackinfo.c 
#        ../../../src/cybergarage/upnp/std/av/cresource.c 
#        ../../../src/cybergarage/upnp/std/av/cresource_list.c 
#        ../../../src/cybergarage/upnp/std/av/cresource_data.c 
#	../../common/ctypeconver.c
 
#	../../../src/totem-pl-parse/totem_m3u_parser.c 
if HAVE_LIBTOOL
upnpavrender_LDADD = -lclinkc ../../../lib/unix/.libs/libclinkcav.a $(LIBS) 
else
upnpavrender_LDADD = -lclinkc ../../../lib/unix/libclinkcav.a $(LIBS)
endif

#../../../lib/unix/libclinkcav.la
