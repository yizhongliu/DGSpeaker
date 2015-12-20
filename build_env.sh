#!/bin/bash

DEP_PATH=/home/llm/project/deplib/x86
UPNP_PATH=/home/llm/project/protocol/clinkc-code

GSTREMERE_INCLUDE_PATH=$DEP_PATH/include/gstreamer-1.0
GSTREMERE_LIB_PATH=$DEP_PATH/lib/gstreamer-1.0

export PATH=$PATH:$DEP_PATH/bin
export PKG_CONFIG_PATH=$DEP_PATH/lib/pkgconfig
export LDFLAGS="-L$DEP_PATH/lib/ -L$UPNP_PATH/lib/unix/ -L$GSTREMERE_LIB_PATH/"
export CFLAGS="-g -I$DEP_PATH/include -I$UPNP_PATH/include -I$DEP_PATH/include/glib-2.0 -I$DEP_PATH/lib/glib-2.0/include -I$GSTREMERE_INCLUDE_PATH -I$GSTREMERE_LIB_PATH/include"
export LIBS="-lpthread -lexpat -lgstreamer-1.0 -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lrt -lglib-2.0 -lxml2"
