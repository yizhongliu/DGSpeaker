noinst_PROGRAMS = upnpavdump

INCLUDES = -I$(top_srcdir)/include -I../ -I../../../common

noinst_HEADERS = 

upnpavdump_SOURCES = \
	../upnpavdump.c

if HAVE_LIBTOOL
upnpavdump_LDADD = -lclinkc  
else
upnpavdump_LDADD = -lclinkc 
endif

#../../../../../lib/unix/libclinkc.a

