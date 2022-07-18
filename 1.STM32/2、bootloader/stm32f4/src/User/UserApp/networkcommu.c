#include "string.h"
#include "mb.h"
#include "networkcommu.h"
#include "bsp_common.h"

/*NetWorkRecvSem:当网络线程接收到数据时会发送此信号量：
  说明：无论是Modbus/UDP/Shell收到数据都会发送此信号量，网络通信处理设计为一个线程，
       当收到此信号量之后，再逐个做相应处理*/
struct rt_semaphore NetWorkRecvSem;
static struct rt_semaphore NetWorkCommuSafeSem; 


/*UDP相关*/
sUDPRecvInfo UDPRecvInfo;
static int UDPSocket;
static sUDPCfgInfo UDPCfgInfo;
static char UDPRecvBuf[1500];

///todo:待编写:增加注释
void NetworkCommuInit(sUDPCfgInfo *iUDPCfg)
{
    rt_err_t err;
    
    ///todo:待编写:参数检查
    memcpy(&UDPCfgInfo,iUDPCfg,sizeof(sUDPCfgInfo));    
    
    /*NetWorkRecvSem init*/
	err = rt_sem_init(&NetWorkRecvSem,"nwsem",0,RT_IPC_FLAG_FIFO);
	if(err != RT_EOK)
	{
		rt_kprintf("NetWorkRecvSem init fail !\n");
        RT_ASSERT(err == RT_EOK);
	}

    /*NetWorkCommuSafeSem init*/
	err = rt_sem_init(&NetWorkCommuSafeSem,"nwcsem",1,RT_IPC_FLAG_FIFO);
	if(err != RT_EOK)
	{
		rt_kprintf("NetWorkCommuSafeSem init fail !\n");
        RT_ASSERT(err == RT_EOK);
	}

    /*UDPRecvInfo.RecvSem init*/
	err = rt_sem_init(&(UDPRecvInfo.RecvSem),"udprs",0,RT_IPC_FLAG_FIFO);
	if(err != RT_EOK)
	{
		rt_kprintf("UDPRecvInfo.RecvSem init fail !\n");
        RT_ASSERT(err == RT_EOK);
	}

}

void network_commu_thread_entry(void* parameter)
{
    uint8_t i;
    int maxsock=0,ret=0,size=0,error=0;
    struct sockaddr_in address, remotehost;
    fd_set rfd;	//字符集
    struct timeval  timeout;
    timeout.tv_sec=0;
    timeout.tv_usec=100000;
    size = sizeof(remotehost);
    
    UDPSocket=socket(AF_INET, SOCK_DGRAM, 0);
    if(UDPSocket<0)
    {
        rt_kprintf("UDPSocket error =%d \n",UDPSocket);
        return ;
    }
    else 
    {
        rt_kprintf("UDPSocket ok =%d \n",UDPSocket);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(7000);
    if(bind(UDPSocket, (struct sockaddr *)&address, sizeof (address)) == -1)
    {
        rt_kprintf("udp bind error !\n");
    }
    else
    {
        rt_kprintf("udp bind ok !\n");
    }
  
    while(1)
    {
        /*设置总网络连接数*/
        maxsock=0;
        FD_ZERO(&rfd);
        
        FD_SET(UDPSocket, &rfd);/*UDP服务器socket*/
        if(maxsock<UDPSocket) maxsock=UDPSocket;
        
        /*选择链接*/
        error = select(maxsock + 1, &rfd, RT_NULL, RT_NULL,&timeout);
        if(error == -1)
        {
            rt_kprintf("select return = -1\n");
            break;
        }
        else if(error == 0)
        {
            continue;
        }
        else
        {
            /*UDP链接*/
            if(FD_ISSET(UDPSocket,&rfd))
            {
                rt_sem_take(&NetWorkCommuSafeSem,RT_WAITING_FOREVER);
                ret = recvfrom(UDPSocket, UDPRecvBuf, sizeof(UDPRecvBuf), 0,(struct sockaddr *)&remotehost, (socklen_t *)&size);
                rt_sem_release(&NetWorkCommuSafeSem);
                if(ret == 0)	                        //断开连接
                {
                    rt_kprintf("UDPSocket recv return 0 \n");
                    lwip_close(UDPSocket);
                    continue;
                } 
                else if(ret > 0)  
                {
                    UDPRecvInfo.RecvLen=ret;
                    if(ret<UDPCfgInfo.RecvBufLen)
                    {
                        memcpy(UDPCfgInfo.pRecvBuf,UDPRecvBuf,ret);
                        memcpy(&(UDPRecvInfo.SockAddr),&remotehost,size);
                        rt_sem_release(&UDPRecvInfo.RecvSem);
                        rt_sem_release(&NetWorkRecvSem);
                    }
                    else
                    {
                        rt_kprintf("err : recv len=%d > UDPCfgInfo.pRecvBuf=%d\n",ret,UDPCfgInfo.RecvBufLen);
                    }
                    ///todo:else语句
                }
                else
                {
                    rt_kprintf("UDPSocket recv return -1 \n");
                    lwip_close(UDPSocket);
                    break;
                }
            } 
           
        }
    }
    rt_kprintf("Exit network_commu_thread_entry\n");
}


/*网络通信：处理线程*/
void network_process_thread_entry(void* parameter)
{
    uint32_t tick;
    while(1)
    {
        rt_sem_take(&NetWorkRecvSem,RT_WAITING_FOREVER);
        tick=rt_tick_get();

        /*udp数据处理*/
        if(RT_EOK == rt_sem_take(&UDPRecvInfo.RecvSem,0))
        {
            UDPPacketProcess();
        }

    }
}


void UDPSendAck(uint16_t iCmd,uint8_t *iBuf,uint16_t iDataLen,uint8_t iEndFlag,struct sockaddr_in *iSockAddr)
{
    uint8_t *pbuf;
    
    pbuf=rt_malloc(iDataLen+19);
    RT_ASSERT(pbuf != RT_NULL);
    
    pbuf[0]=0x3E;
    pbuf[1]=(iDataLen+19)&0xff;
    pbuf[2]=((iDataLen+19)&0xff00)>>8;
    pbuf[3]=(iCmd&0xff00)>>8;
    pbuf[4]=iCmd&0xff;
    pbuf[5]=UDPCfgInfo.pRecvBuf[5];
    pbuf[6]=UDPCfgInfo.pRecvBuf[6];
    pbuf[7]=UDPCfgInfo.pRecvBuf[7];
    pbuf[8]=UDPCfgInfo.pRecvBuf[8];
    pbuf[9]=iEndFlag;
    pbuf[10]=IS_ACK;
    pbuf[11]=NOT_NEED_ACK;
    pbuf[12]=0;
    pbuf[13]=0;
    pbuf[14]=0;
    pbuf[15]=0;
    memcpy(&pbuf[16],iBuf,iDataLen);
    pbuf[iDataLen+16]=0;
    pbuf[iDataLen+17]=0;
    pbuf[iDataLen+18]=0xE3;
    UDPSend(pbuf,iDataLen+19,iSockAddr);
   
    rt_free(pbuf);
}

void UDPSend(uint8_t *iBuf,uint16_t iLen,struct sockaddr_in *iSockAddr)
{
    rt_sem_take(&NetWorkCommuSafeSem,RT_WAITING_FOREVER);
    sendto(UDPSocket, iBuf,iLen, 0,(struct sockaddr *)iSockAddr,sizeof(struct sockaddr));    
    rt_sem_release(&NetWorkCommuSafeSem);
}
