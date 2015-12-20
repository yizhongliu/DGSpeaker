/*************************************************
  Copyright (C), 2013-2014, TCL AV 
  File name:      upnpavrender.c
  Author:   llm    Version:        Date: 
  Description:    dmr初始化, dmr启动关闭消息处理
  Others:         
  Function List:  
    1. dmr_init_cb
    2. dmr_init
    3. dmr_handle_cb
    4. dmr_handle
    
  History:        
                  
    1. Date:   2013/03/18
       Author: llm 
       Modification:  first release

*************************************************/
#include <string.h>
#include <stdio.h>
//#include "cmd.h"
#include "upnpavrender.h"
//#include "dsp_interface.h"
//#include "share_variable.h"


/*for network test*/
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <stdlib.h> 
#include <netdb.h> 
#include <errno.h> 

#define BUFLEN 255
/*****************/



CgUpnpAvRenderer *dmr = NULL;
int dmr_state = TCL_DMR_START;

pthread_mutex_t dmrstate_mutex;


/*设置音量回调函数*/
extern CG_UPNPAV_VOLUME_LISTNER setVolumeListner;
extern CG_UPNPAV_VOLUME_GETLISTNER getVolumeListner;
/*mute on/off 回调函数*/
extern CG_UPNPAV_MUTE_LISTNER setMuteListner;
extern CG_UPNPAV_UNMUTE_LISTNER setUnMuteListner;




void recvUdp(void* arg);
void recvUdpTest(void* arg);
BOOL recvUdp2();
void recvUdp3(void* arg);





/*************************************************
  Function:       getUpnpAvRender
  Description:    获取DMR结构体
  Input:          
  Output:         
  Return:         DMR结构体
*************************************************/
CgUpnpAvRenderer * getUpnpAvRender(void)
{
    return dmr;
}

/*************************************************
  Function:       dmr_init_cb
  Description:    启动upnp线程 和 gstreamer main loop线程
  Input:          
  Output:         
  Return:         
*************************************************/
void dmr_init_cb(void *arg)
{
    int ret;
    BOOL bret;

	pthread_t checkIpId;	
	pthread_attr_t checkIpThreadAttr;

	ret = output_gstreamer_pipeline_init(dmr);
	if(ret != 0)
	{
		fprintf(stderr, "pipeline create failed !\n");
		return;
	}


    bret = cg_upnpav_dmr_start(dmr);
    if(bret == FALSE)
    {
        fprintf(stderr, "dmr start failed !\n");
		return;
    }

	

    /*start a thread to check ip*/
	pthread_attr_init(&checkIpThreadAttr);
	pthread_attr_setdetachstate(&checkIpThreadAttr, PTHREAD_CREATE_DETACHED);
    if(-1==pthread_create(&checkIpId, &checkIpThreadAttr, checkIpThread, dmr->dev))
    {       
        fprintf(stderr, "Create  checkIpThread failed !\n");
		pthread_attr_destroy(&checkIpThreadAttr);
        return;
    }

	pthread_attr_destroy(&checkIpThreadAttr); 

//	recvUdp2();

	output_pipeline_loop();

	return;
}

/*************************************************
  Function:       dmr_init
  Description:    dmr初始化
  Input:          
  Output:         
  Return:         
*************************************************/
int dmr_init(int argc, char **argv)
{
    int ret;
	BOOL bret;
    pthread_t dmrInitId;	
	pthread_attr_t thread_attr;
//	pthread_t checkIpId;	
//	pthread_attr_t checkIpThreadAttr;

//    pthread_t recvUdpId;	
//	pthread_attr_t recvUdpAttr;

//	setVolumeListner = TASSetVolume;    // 设置音量回调函数
//	getVolumeListner = TASReadVolume;   //  获取音量回调函输
//	setMuteListner = TASMute;  //设置mute 回调函数
//	setUnMuteListner = TASUMute;    //设置unmute 回调函数

	pthread_mutex_init (&dmrstate_mutex,NULL); 

	/*gstreamer core init*/
    ret = process_cmdline(argc, argv);
    if (ret != 0) 
    {
        printf("gst init error ...\n");
        return ret;
    }

    /*malloc memory for struct CgUpnpAvRenderer */
    dmr = cg_upnpav_dmr_new();
    if(dmr == NULL)
    {
        printf("dmr malloc fail ...\n");
        return -1;
    }

	printf("dmr creat success###___________\n");

	//scan_mime_list(dmr);

    /*start upnp thread */
    pthread_attr_init(&thread_attr);
	pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
    if(-1==pthread_create(&dmrInitId, &thread_attr, dmr_init_cb, NULL))
    {       
        fprintf(stderr, "Create  checkIpThread failed !\n");
		pthread_attr_destroy(&thread_attr);
        return -1;
    }

	pthread_attr_destroy(&thread_attr);




    return 0;

}


/*************************************************
  Function:       dmr_handle_cb
  Description:    dmr开启关闭消息处理函数
  Input:          arg 接收到的消息
  Output:         
  Return:         
*************************************************/
void dmr_handle_cb(void *arg)
{
    int *cmd;
    BOOL bret;
    #if 0
	pthread_mutex_lock (&dmrstate_mutex);

    cmd = (int *)arg; 
    switch((*cmd))
    {
        case TCL_BTP463_DMR_START:
			printf("DMR_STRAT\n");
		//	output_close_playbinloop();
		//	output_gstreamer_rtpbin_mpa_test(dmr);
			#if 1
            output_play2("rtsp://192.168.1.4:8555/mnt/sdcard/Music/baihu.mp3");
       //     cg_upnpav_dmr_avtransport_setmulticasturi_test(dmr, "rtp://232.255.42.42:6666/mnt/sdcard/Music/baihu.mp3");
      //      dmr->uriMode = MULTICAST;
	//		output_gstreamer_rtpbin_init(dmr, "232.255.42.42", "6666");
     //       cg_upnpav_dmr_avtransport_play_test(dmr);
			#endif
			#if 0
            if(dmr_state == TCL_DMR_STOP)
            {
                cg_upnpav_dmr_connectionmgrr_variable_init(dmr);
                cg_upnpav_dmr_avtransport_variable_init(dmr);
                cg_upnpav_dmr_rendercontrol_variable_init(dmr);
			
                bret = cg_upnpav_dmr_start(dmr);
                if(bret == FALSE)
                {
                    printf("dmr start fail!!!!!!\n");
                    break;
                }
  
                dmr_state = TCL_DMR_START;
                printf("dmr start......\n");
            }
            #endif
        break;

        case TCL_BTP463_DMR_STOP:
            printf("DRM_STOP\n");
//			output_stop();
//			output_gstreamer_rtpbin_mpa_test(dmr);
            #if 0
			printf("DRM_STOP\n");
            if(dmr_state == TCL_DMR_START)
            {
                cg_mutex_lock(dmr->transportMutex);
			    output_stop();
			    cg_mutex_unlock(dmr->transportMutex);

				printf("gstreamer stop\n");
				
                bret = cg_upnpav_dmr_stop(dmr);
                if(bret == FALSE)
                {
                    printf("dmr stop fail!!!!!!\n");
                    break;
                }

                dmr_state = TCL_DMR_STOP;
                printf("dmr stop......\n");

            }
        #endif
        break;
           
        default:
        break;
    }

	pthread_mutex_unlock (&dmrstate_mutex);
    #endif	
    printf("leave %d....\n", (*cmd));
    
}

/*************************************************
  Function:       dmr_handle
  Description:    dmr开启关闭消息接收，启动线程处理
  Input:          cmd 接收到的消息
  Output:         
  Return:         
*************************************************/
int gcmd = 0;
void dmr_handle(char cmd)
{
    pthread_t dmrHandleId;
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

    gcmd = cmd;

    if(-1==pthread_create(&dmrHandleId, &thread_attr, dmr_handle_cb, &gcmd))
    {
        fprintf(stderr, "Create  dmr_handle failed !\n");
        pthread_attr_destroy(&thread_attr);
        return;
    }

	pthread_attr_destroy(&thread_attr);

}

void recvUdp(void* arg)
{
    int sin_len;
    char message[256];

    int socket_descriptor;
    struct sockaddr_in sin;
	int port = 8080;
    printf("Waiting for data form sender \n");

	sleep(10);

    bzero(&sin,sizeof(sin));
    sin.sin_family=AF_INET;
    sin.sin_addr.s_addr=htonl("239.1.1.1");
    sin.sin_port=htons(port);
    sin_len=sizeof(sin);

    socket_descriptor=socket(AF_INET,SOCK_DGRAM,0);
    bind(socket_descriptor,(struct sockaddr *)&sin,sizeof(sin));

    while(1)
    {
       recvfrom(socket_descriptor,message,sizeof(message),0,(struct sockaddr *)&sin,&sin_len);
       printf("Response from server:%s\n",message);
   
    }

    close(socket_descriptor);
    exit(0);
}

void recvUdpTest(void* arg)
{
    struct sockaddr_in peeraddr;
    struct in_addr ia;
    int sockfd;
    char recmsg[BUFLEN + 1];
    unsigned int socklen, n;
    struct hostent *group;
    struct ip_mreq mreq;

    /* 创建 socket 用于UDP通讯 */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket\n");
        exit(errno);
    }
    printf("socket created success!!!\n");
    /* 设置要加入组播的地址 */
    bzero(&mreq, sizeof(struct ip_mreq));
   
    if ((group = gethostbyname("239.1.1.1")) == (struct hostent *) 0) {
            perror("gethostbyname");
            exit(errno);
    }
   

    bcopy((void *) group->h_addr, (void *) &ia, group->h_length);
    printf("组播地址：%s\n",inet_ntoa(ia));
    /* 设置组地址 */
    bcopy(&ia, &mreq.imr_multiaddr.s_addr, sizeof(struct in_addr));

    /* 设置发送组播消息的源主机的地址信息 */
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    /* 把本机加入组播地址，即本机网卡作为组播成员，只有加入组才能收到组播消息 */
    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq,sizeof(struct ip_mreq)) ==-1) {
        perror("setsockopt");
        exit(errno);
    }

    socklen = sizeof(struct sockaddr_in);
    memset(&peeraddr, 0, socklen);
    peeraddr.sin_family = AF_INET;
    peeraddr.sin_port = htons(8080);
	  
   
    if (inet_pton(AF_INET, "239.1.1.1", &peeraddr.sin_addr) <= 0) {
          printf("Wrong dest IP address!\n");
          exit(0);
    }
   

    /* 绑定组播地址的端口和IP信息到socket上 */
    if (bind(sockfd, (struct sockaddr *) &peeraddr,sizeof(struct sockaddr_in)) == -1) {
        printf("Binded failure\n");
        exit(0);
    }else
        printf("binded success!!!\n");

    /* 循环接收网络上来的组播消息 */
    for (;;) {
        bzero(recmsg, BUFLEN + 1);
        n = recvfrom(sockfd, recmsg, BUFLEN, 0,
        (struct sockaddr *) &peeraddr, &socklen);
        if (n < 0) {
            printf("recvfrom error in udptalk!\n");
            exit(errno);
        } else {
    /* 成功接收到数据报 */
            recmsg[n] = 0;
            printf("peer info:%s", recmsg);
        }
    }
}

CgSocket *sock;

void cg_upnp_udp_thread(CgThread *thread)
{
    int recvLen = 0;
	char recvBuf[512+1];
	struct sockaddr_storage from;
	socklen_t fromLen = sizeof(from);

	printf("enter  cg_upnp_udp_thread ......\n");
	
    while (cg_thread_isrunnable(thread) == TRUE) 
	{			   
	   recvLen = recvfrom(sock->id, recvBuf, sizeof(recvBuf)-1, 0, (struct sockaddr *)&from, &fromLen);
	   printf("receive Len %d\n", recvLen);
	}
}

BOOL recvUdp2()
{
    
	CgThread *recvThread;

	char* mcastAddr = "239.1.1.1";
	char* bindAddr = "0.0.0.0";
	int port = 8085;
	
    sock = cg_socket_new(CG_NET_SOCKET_DGRAM);

    if (cg_socket_bind(sock, port, bindAddr, FALSE, TRUE) == FALSE)
		return FALSE;
		
	if (cg_socket_joingroup(sock, mcastAddr, bindAddr) == FALSE) {
		cg_socket_close(sock);
		return FALSE;
	}

	recvThread = cg_thread_new();
	
	cg_thread_setaction(recvThread, cg_upnp_udp_thread);

	if (cg_thread_start(recvThread) == FALSE) {	
		cg_thread_delete(recvThread);
		recvThread = NULL;
		return FALSE;
	}
	
}

struct sockaddr_in localSock;
struct ip_mreq group;
int sd;
int datalen;
char databuf[1024];
 
void recvUdp3(void* arg)
{
sleep(5);
/* Create a datagram socket on which to receive. */
sd = socket(AF_INET, SOCK_DGRAM, 0);
if(sd < 0)
{
perror("Opening datagram socket error");
exit(1);
}
else
printf("Opening datagram socket....OK.\n");
 
/* Enable SO_REUSEADDR to allow multiple instances of this */
/* application to receive copies of the multicast datagrams. */
{
int reuse = 1;
if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
{
perror("Setting SO_REUSEADDR error");
close(sd);
exit(1);
}
else
printf("Setting SO_REUSEADDR...OK.\n");
}
 
/* Bind to the proper port number with the IP address */
/* specified as INADDR_ANY. */
memset((char *) &localSock, 0, sizeof(localSock));
localSock.sin_family = AF_INET;
localSock.sin_port = htons(8080);
localSock.sin_addr.s_addr = INADDR_ANY;
if(bind(sd, (struct sockaddr*)&localSock, sizeof(localSock)))
{
perror("Binding datagram socket error");
close(sd);
exit(1);
}
else
printf("Binding datagram socket...OK.\n");
 
/* Join the multicast group 226.1.1.1 on the local 203.106.93.94 */
/* interface. Note that this IP_ADD_MEMBERSHIP option must be */
/* called for each local interface over which the multicast */
/* datagrams are to be received. */
group.imr_multiaddr.s_addr = inet_addr("239.1.1.1");
group.imr_interface.s_addr = inet_addr("0.0.0.0");
if(setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0)
{
perror("Adding multicast group error");
close(sd);
exit(1);
}
else
printf("Adding multicast group...OK.\n");
 
/* Read from the socket. */
datalen = sizeof(databuf);
while(1)
{
    if(read(sd, databuf, datalen) < 0)
    {
		perror("Reading datagram message error");
		close(sd);
		exit(1);
	}
	else
	{
    	printf("Reading datagram message...OK.\n");
    	printf("The message from multicast server is: \"%s\"\n", databuf);
		output_seek(0.5);
	}
}
return ;
}

