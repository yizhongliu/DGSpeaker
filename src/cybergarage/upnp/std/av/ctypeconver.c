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
*	File: ctypeconver.c
*
*	Revision:
*
*	13/03/18
*		- first revision
*
******************************************************************/




#include<stdlib.h>
//#include "ctypeconver.h"
#include <cybergarage/upnp/std/av/ctypeconver.h>

/****************************************
* cg_time_to_int
****************************************/
int cg_time_to_int(char *time)
{
    int tmp = 0;
    int result = 0;

    int i = 0;

    if(time == NULL)
    {
        return 0;
    }

	printf("cg_time_to_int inputtime = %s\n", time);

	for(i=0; time[i]==' '; i++)
		;

    while( (time[i] != '\0') && (time[i] != '.') )
    {
        if(time[i] == ':')
        {
            result += tmp;
            result *= 60;
            tmp = 0;
            i++;
        }
        else
        {
            tmp = tmp*10 + (time[i] - '0');
			i++;
        }
		
    }
    
    result += tmp;

    return result;
}


/****************************************
* cg_atoi
****************************************/
int cg_atoi(char *str)
{
    int i;
	int result;

    if(str == NULL)
    {
        return 0;
    }
	
	for(i=0; str[i]==' '; i++)
	    ;

	for(result=0; str[i]!='\0'; i++)
	{
	    result = 10 * result + (str[i] - '0');
	}

	return result;
}

/****************************************
* cg_parse_multicastUri
****************************************/
int cg_parse_multicastUri(const char* inUri, char* outMulticastIP, char* outPort)
{
    char* tempS;
	char* tempE;
	
    if(inUri == NULL)
    {
        printf("%s: input arg is NULL",__FUNCTION__);
        return -1;
    }

    /*parse ip*/
	tempS = strstr(inUri, "rtp://");
	if(tempS == NULL)
	{
	    printf("%s: uri is invalid",__FUNCTION__);
        return -1;
	}

	tempS += 6;
	
	tempE = strchr(tempS, ':');
    if(tempE == NULL)
    {
        printf("%s: uri is invalid",__FUNCTION__);
        return -1;
    }

	if(tempE-tempS > 20)
	{
	   printf("%s: length of ip invalid",__FUNCTION__);
       return -1;
	}

	strncpy(outMulticastIP, tempS, tempE-tempS);
	outMulticastIP[tempE-tempS] = '\0';

	printf("multicalIp = %s\n", outMulticastIP);

	/*parse port*/
	tempS = tempE + 1;
	
	tempE = strchr(tempS, '/');
	if(tempE == NULL)
    {
        printf("%s: uri is invalid\n",__FUNCTION__);
        return -1;
    }

	if(tempE-tempS > 10)
	{
	   printf("%s: length of port invalid\n",__FUNCTION__);
       return -1;
	}
	
    strncpy(outPort, tempS, tempE-tempS);
	outPort[tempE-tempS] = '\0';

	printf("multicalPort = %s\n", outPort);

	return 0;
}

