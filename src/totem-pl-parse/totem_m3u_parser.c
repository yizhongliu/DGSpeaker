#include <totem-pl-parser/1/plparser/totem-pl-parser.h>
#include <totem-pl-parse/totem_m3u_parser.h>
#include <stdlib.h>



static void  entry_parsed (TotemPlParser *parser,
	     const char *uri,
	     GHashTable *metadata,
	     gpointer data)
{

    CgUpnpAvRenderer *render = (CgUpnpAvRenderer *) data;
	CgUpnpAvTrackInfo *trackInfo = NULL;
	gchar *metaData = NULL;

	if(uri == NULL)
	{
	    printf("%s %d %s uri is NULL\n",__FILE__, __LINE__, __PRETTY_FUNCTION__);
		return;
	}
			
	metaData = g_hash_table_lookup (metadata, TOTEM_PL_PARSER_FIELD_TITLE);
	if (metaData != NULL)
		printf ("Entry title: %s\n", metaData);
	else
		printf ("Entry (URI: %s) has no title.\n", uri);

	trackInfo = cg_upnpav_trackinfo_new();
    if(trackInfo == NULL)
	{
	    printf("%s %d %s can't malloc trackInfo\n",__FILE__, __LINE__, __PRETTY_FUNCTION__);
		return;
	}
	cg_upnpav_trackinfo_settrack(trackInfo, uri);
	cg_upnpav_trackinfolist_add(render->trackInfoList,trackInfo);
	
   

}


void totem_m3u_parse_uri(char *uri, CgUpnpAvRenderer *dmr)
{
   	CgUpnpAvRenderer *render = dmr;
	TotemPlParser *parser;
	TotemPlParserResult res;
	int iNumberOfTracks = 0;
	char *currentTrackUri = NULL;
	char strNumberOfTracks[5];

	CgUpnpAvTrackInfo *trackInfo;
	
    if( uri==NULL || dmr==NULL)
    {
        printf("parse argument is NULL!!!!!!!!!!!!!!!\n");
		return;
    }

   
	parser = totem_pl_parser_new ();
	g_object_set (parser, "force", TRUE,
		      "disable-unsafe", TRUE,
		      NULL);
	g_signal_connect (parser, "entry-parsed", G_CALLBACK (entry_parsed), render);
	res = totem_pl_parser_parse_with_base (parser, uri, NULL, FALSE);

	g_object_unref (parser);

	if (res != TOTEM_PL_PARSER_RESULT_SUCCESS) {

		printf("error!!!!!!!\n");
		//FIXME show a proper error message
		switch (res) {
		case TOTEM_PL_PARSER_RESULT_UNHANDLED:
			g_print ("url '%s' unhandled\n", uri);
			break;
		case TOTEM_PL_PARSER_RESULT_ERROR:
			g_print ("error handling url '%s'\n", uri);
			break;
		case TOTEM_PL_PARSER_RESULT_IGNORED:
			g_print ("ignored url '%s'\n", uri);
			break;
		default:
			g_assert_not_reached ();
			;;
		}
	}

//	trackInfo = cg_upnpav_trackinfolist_gettrack(render->trackInfoList, 2);
//	printf("get trackInfo = %s\n", trackInfo->track->value);
	

	iNumberOfTracks = cg_upnpav_trackinfolist_size(render->trackInfoList);
	if(iNumberOfTracks == 0)
	{
	    printf("%s %d %s playlist is empty\n",__FILE__, __LINE__, __PRETTY_FUNCTION__);
	    return;
	}
	sprintf(strNumberOfTracks, "%d", iNumberOfTracks);
//	itoa(iNumberOfTracks, strNumberOfTracks, 10);

	cg_upnpav_dmr_avtransport_setstatevariable(render, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKURI, 
		                                       cg_upnpav_trackinfolist_gets(render->trackInfoList));
    cg_upnpav_dmr_avtransport_change_var(render, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACKURI,
			  	                         cg_upnpav_trackinfolist_gets(render->trackInfoList));
	
	cg_upnpav_dmr_avtransport_setstatevariable(render, CG_UPNPAV_DMR_AVTRANSPORT_NUMBEROFTRACKS, 
		                                       strNumberOfTracks);
	cg_upnpav_dmr_avtransport_change_var(render, CG_UPNPAV_DMR_AVTRANSPORT_NUMBEROFTRACKS, 
		                                       strNumberOfTracks);
	
	cg_upnpav_dmr_avtransport_setstatevariable(render, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACK, 
		                                       "1");
	cg_upnpav_dmr_avtransport_change_var(render, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRACK, 
		                                       "1");
	             
}




