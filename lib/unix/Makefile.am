if HAVE_LIBTOOL
lib_LTLIBRARIES = libclinkcav.la
else
lib_LIBRARIES = libclinkcav.a
endif


#lib_LTLIBRARIES = libclinkcav.a

PWD=$(shell pwd)

#GSTREAMER_ENV=$(PWD)/../../../../dep_lib
#GST_CFLAGS = -I$(GSTREAMER_ENV)/include/gstreamer-0.10 -I$(GSTREAMER_ENV)/include/glib-2.0 -I$(GSTREAMER_ENV)/lib/glib-2.0/include -I$(GSTREAMER_ENV)/include/libxml2 -L$(GSTREAMER_ENV)/lib



#GST_CFLAGS = -I/usr/include/gstreamer-0.10 -I/usr/include/glib-2.0 -I/usr/lib/i386-linux-gnu/glib-2.0/include -I/usr/include/libxml2


INCLUDES = -I$(top_srcdir)/include 

########################################
# Sources
########################################

clinkcav_source = ../../src/cybergarage/upnp/std/av/ccontent.c \
	../../src/cybergarage/upnp/std/av/ccontent_list.c \
	../../src/cybergarage/upnp/std/av/cdidl.c \
	../../src/cybergarage/upnp/std/av/cmd5.c \
	../../src/cybergarage/upnp/std/av/cresource.c \
	../../src/cybergarage/upnp/std/av/cresource_list.c \
	../../src/cybergarage/upnp/std/av/cresource_data.c \
	../../src/cybergarage/upnp/std/av/server/cconnectionmgr_service.c \
	../../src/cybergarage/upnp/std/av/server/ccontentdir_service.c \
	../../src/cybergarage/upnp/std/av/server/cmediaserver_device.c \
	../../src/cybergarage/upnp/std/av/cprotocolinfo_list.c \
	../../src/cybergarage/upnp/std/av/cprotocolinfo.c \
	../../src/cybergarage/upnp/std/av/ctypeconver.c \
	../../src/cybergarage/upnp/std/av/xmlescape.c \
	../../src/cybergarage/upnp/std/av/ctrackinfo_list.c \
	../../src/cybergarage/upnp/std/av/ctrackinfo.c \
	../../src/cybergarage/upnp/std/av/renderer/cavtransport_service.c \
	../../src/cybergarage/upnp/std/av/renderer/crenderingcontrol_service.c \
	../../src/cybergarage/upnp/std/av/renderer/cconnectionmgrr_service.c \
	../../src/cybergarage/upnp/std/av/renderer/cmediarenderer_device.c \
	../../src/gstreamer_0.10/output_gstreamer.c 

########################################
# Library
########################################

libclinkcavincludedir = $(includedir)/cybergarage 
nobase_libclinkcavinclude_HEADERS =  \
        $(clinkcav_header) 

if HAVE_LIBTOOL
libclinkcav_la_SOURCES = \
        $(clinkcav_source)
libclinkcav_a_CFLAGS = $(AM_CFLAGS) 
libclinkcav_la_LIBADD =
else
libclinkcav_a_SOURCES = \
        $(clinkcav_source)
libclinkcav_a_CFLAGS = $(AM_CFLAGS) 
libclinkcav_a_LIBADD =
endif
