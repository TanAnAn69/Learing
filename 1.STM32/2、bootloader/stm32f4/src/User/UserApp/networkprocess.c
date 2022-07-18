#include "rtthread.h"
#include "ethernetif.h"
#include "networkcfg.h"
#include "networkprocess.h"
#include "limitcfg.h"
#include "bsp_88e6060.h"
#include "finsh.h"
#include "bsp_common.h"
#include "networkupdate.h"

extern void lwip_sys_init(void);
extern void lwip_sys_initip(uint8_t ip1, uint8_t ip2, uint8_t ip3, uint8_t ip4);
extern void NET_LocalSave(void);

/*UDP相关变量定义*/
const char BootVersion[5]="V1.2"; 
uint8_t  UDPRecvBuf[UDP_RECV_BUF_LEN]; //UDP缓冲区

/*UDP数据包处理*/
void UDPPacketProcess(void)
{
    uint16_t cmd;
    sUDPPacketFrame *precvbuf=(sUDPPacketFrame *)UDPRecvBuf;
    uint16_t sendindex=0;
    uint8_t *sendbuf;
    uint8_t stringlen=0;
    
    sendbuf=rt_malloc(256);
    RT_ASSERT(sendbuf != RT_NULL);
    
    cmd=(precvbuf->MajorCmd<<8) + (precvbuf->MinorCmd); 
    
    /*处理来自于测试上位机的数据包*/
    switch (cmd)
    {
        case CMD_BROADCAST:
        {
            sendindex=0;
            
            //元素1：当前程序(String)：当前是boot程序还是user程序
            stringlen=strlen("boot");
            sendbuf[sendindex++]=stringlen;
            memcpy(&sendbuf[sendindex],"boot",stringlen);
            sendindex+=stringlen;
            
            //元素2：boot程序版本(Sting): “V1.0” , “V1.1”
            stringlen=strlen(BootVersion);
            sendbuf[sendindex++]=stringlen;
            memcpy(&sendbuf[sendindex],BootVersion,stringlen);
            sendindex+=stringlen;

            //元素3：CodeType(String)：下位机程序类别
            stringlen=strlen(NetLocalAddr.UserCodeType);
            sendbuf[sendindex++]=stringlen;
            memcpy(&sendbuf[sendindex],NetLocalAddr.UserCodeType,stringlen);
            sendindex+=stringlen;
            
            //元素4：FWVersion(String)：下位机程序版本号
            stringlen=strlen(NetLocalAddr.UserCodeVersion);
            sendbuf[sendindex++]=stringlen;
            memcpy(&sendbuf[sendindex],NetLocalAddr.UserCodeVersion,stringlen);
            sendindex+=stringlen;

            //元素5：BootKey(String)：boot按键
            stringlen=strlen(NetLocalAddr.BootKey);
            sendbuf[sendindex++]=stringlen;
            memcpy(&sendbuf[sendindex],NetLocalAddr.BootKey,stringlen);
            sendindex+=stringlen;
            
            UDPSendAck(CMD_BROADCAST,sendbuf,sendindex,IS_END_PACKET,&UDPRecvInfo.SockAddr);
            break;
        }
        
        case CMD_ENTER_BOOT:
        {
            sendindex=0;
            
            rt_kprintf("receive enter boot cmd\n");
            //元素1：当前程序(String)：当前是boot程序还是user程序
            stringlen=strlen("boot");
            sendbuf[sendindex++]=stringlen;
            memcpy(&sendbuf[sendindex],"boot",stringlen);
            sendindex+=stringlen;

            UDPSendAck(CMD_ENTER_BOOT,sendbuf,sendindex,IS_END_PACKET,&UDPRecvInfo.SockAddr);
            break;
        }
        
        case CMD_ENTER_USER:
        {
            sendindex=0;
            rt_kprintf("receive enter user cmd\n");
            
            //元素1：当前程序(String)：当前是boot程序还是user程序
            stringlen=strlen("boot");
            sendbuf[sendindex++]=stringlen;
            memcpy(&sendbuf[sendindex],"boot",stringlen);
            sendindex+=stringlen;

            UDPSendAck(CMD_ENTER_USER,sendbuf,sendindex,IS_END_PACKET,&UDPRecvInfo.SockAddr);
            NetLocalAddr.BootFlag=0;
            NET_LocalSave();
            NVIC_SystemReset();
            break;
        }
        case CMD_ERASE_FLASH:
        {
            CodeTotalLength=0; //代码长度
            CodeDownloadLength=0; //已写入flash的代码长度
            CodeUpperCheckSum=0; //上位机计算的代码校验和
            CodeLowerCheckSum=0; //下位机计算的代码校验和
            CurrentProgress=0; //当前进度
            EraseFlashResult=0;
            CheckSumResult=0;
            memset(FileBuf,0,sizeof(FileBuf));
            
            sendindex=0;
            UDPSendAck(CMD_ERASE_FLASH,sendbuf,sendindex,IS_END_PACKET,&UDPRecvInfo.SockAddr);
            memcpy(&CodeTotalLength,&precvbuf->Content[0],4);
            memcpy(&CodeUpperCheckSum,&precvbuf->Content[4],4);
            rt_kprintf("CodeTotalLength=%d,CodeUpperCheckSum=%d\n",CodeTotalLength,CodeUpperCheckSum);
            rt_sem_release(&UpdateSem);

            break;
        }
        case CMD_UQARY_ERASE_FLASH_RESULT:
        {
            sendindex=0;
            sendbuf[sendindex++]=EraseFlashResult;
            UDPSendAck(CMD_UQARY_ERASE_FLASH_RESULT,sendbuf,sendindex,IS_END_PACKET,&UDPRecvInfo.SockAddr);
            break;
        }
               
        case CMD_DOWNLOAD:
        {
            uint16_t datalen=0;
            uint16_t packetseq=(precvbuf->PacketIndexH<<8)+precvbuf->PacketIndexL;
            sendindex=0;
            
            memcpy(&datalen,precvbuf->Content,2);
            if(datalen<=1024)
            {                
                //元素1：结果（BYTE）：0：失败 1：成功
                sendbuf[sendindex++]=SaveFileToBuf(packetseq,&(precvbuf->Content[2]),datalen);
                
                //元素2：当前进度(BYTE)：0~100
                sendbuf[sendindex++]=CurrentProgress;
                UDPSendAck(CMD_DOWNLOAD,sendbuf,sendindex,IS_END_PACKET,&UDPRecvInfo.SockAddr);
            }
            else
            {
                rt_kprintf("error:len=%d\n",datalen);
            }
            break;
        }
        case CMD_QUARY_CHECK_SUM:
        {
            sendindex=0;
            sendbuf[sendindex++]=CheckSumResult;
            UDPSendAck(CMD_QUARY_CHECK_SUM,sendbuf,sendindex,IS_END_PACKET,&UDPRecvInfo.SockAddr);
            if(CheckSumResult>0) CheckSumResult=0;  //上位机成功获取校验结果后，将校验结果清空
            break;
        }
        default :
        {
            
            rt_kprintf("invalid udp cmd (%04x)\n",cmd);
            break;
        }
    }
    rt_free(sendbuf);
}

///todo:待注释:此函数一般不需要修改
void NetworkInit(void)
{
    sUDPCfgInfo udpcfg;
    uint16_t addr,size;
    
    PRINT_INIT_START();
    
    /*网络驱动初始化*/
    EEPB_GetAddr("IP",&addr,&size);
    NET_CfgInit(addr,size);
    eth_system_device_init();
    rt_hw_88e6060_init(NetLocalAddr.Mac);
    lwip_sys_init();
    lwip_sys_initip(NetLocalAddr.IP[0],NetLocalAddr.IP[1],NetLocalAddr.IP[2],NetLocalAddr.IP[3]);

    /*UDP配置信息*/
    udpcfg.pRecvBuf=UDPRecvBuf;
    udpcfg.RecvBufLen=sizeof(UDPRecvBuf);
        
    /*网络通信初始化*/
    NetworkCommuInit(&udpcfg);
    
    PRINT_INIT_OK();
}

