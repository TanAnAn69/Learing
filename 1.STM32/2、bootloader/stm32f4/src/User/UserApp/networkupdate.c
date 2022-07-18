#include "rtthread.h"
#include "finsh.h"
#include "bsp_common.h"
#include "networkupdate.h"
#include "finsh.h"
#include "networkcfg.h"

#define FLASH_USER_START_ADDR  ((uint32_t)0x08020000)
/* Base address of the Flash sectors */ 
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base address of Sector 0, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base address of Sector 1, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base address of Sector 2, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base address of Sector 3, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base address of Sector 4, 64 Kbytes   */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base address of Sector 5, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base address of Sector 6, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base address of Sector 7, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base address of Sector 8, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base address of Sector 9, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base address of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base address of Sector 11, 128 Kbytes */

uint32_t CodeTotalLength=0; //���볤��
uint32_t CodeDownloadLength=0; //��д��flash�Ĵ��볤��
uint32_t CodeUpperCheckSum=0; //��λ������Ĵ���У���
uint32_t CodeLowerCheckSum=0; //��λ������Ĵ���У���
uint8_t CheckSumResult=0;////0:��ʼ״̬ 1��������  2��У��ֵ����  3��У��ֵ��ȷ
uint8_t EraseFlashResult=0; // �����BYTE����0����ʼ״̬  1��������  2������ʧ��  3�������ɹ�
uint8_t CurrentProgress=0; //��ǰ����

struct rt_semaphore UpdateSem;
sFileBuf FileBuf[FILE_BUFFER_NUM];

/*�������ļ����浽��������*/
uint8_t SaveFileToBuf(uint16_t iSeq,uint8_t *iData,uint16_t iDataLen)
{
    uint8_t i;
    uint32_t tick=rt_tick_get();
    ATOMIC_VAR;
    
    //�ظ���ֱ�ӷ���
    for(i=0;i<FILE_BUFFER_NUM;i++)
    {
        if((FileBuf[i].Seq==iSeq) && (FileBuf[i].State!=FILE_STATE_INIT)) 
        {
            rt_kprintf("%s,%d : Seq=%d already saved \n",__FUNCTION__,__LINE__,iSeq);
            return 1;
        }
    }
    
    while(1)
    {
        if(iSeq<FILE_BUFFER_NUM) //��ǰ�ļ������С�ڻ���������ʱ��ֱ�Ӵ�ż���
        {
            if(FileBuf[iSeq].State==FILE_STATE_INIT)
            {
                FileBuf[iSeq].Seq=iSeq;
                FileBuf[iSeq].DataLen=iDataLen;
                memcpy(FileBuf[iSeq].Data,iData,iDataLen);
                ATOMIC(FileBuf[iSeq].State=FILE_STATE_RECEIVE_OK;)
                return 1;
            }
            else
            {
                rt_kprintf("%s,%d : state not equal FILE_STATE_INIT, Seq=%d\n",__FUNCTION__,__LINE__,iSeq);
                return 0;
            }
        }
        else
        {
            for(i=0;i<FILE_BUFFER_NUM;i++)
            {
                if(FileBuf[i].Seq==(iSeq-1)) //�ҵ���һ���ļ�����ŵ�λ�ã���ǰ�ļ���ֻ��������󼴿�
                {
                    if(i==FILE_BUFFER_NUM-1) //��һ���ļ������������һ��������������һ���ļ�����Ҫ���ڵ�һ��������
                    {
                        if(FileBuf[0].State==FILE_STATE_DOWNLOAD_OK)
                        {
                            FileBuf[0].Seq=iSeq;
                            FileBuf[0].DataLen=iDataLen;
                            memcpy(FileBuf[0].Data,iData,iDataLen);
                            ATOMIC(FileBuf[0].State=FILE_STATE_RECEIVE_OK;)
                            return 1;                              
                        }
                    }
                    else
                    {
                        if(FileBuf[i+1].State==FILE_STATE_DOWNLOAD_OK)
                        {
                            FileBuf[i+1].Seq=iSeq;
                            FileBuf[i+1].DataLen=iDataLen;
                            memcpy(FileBuf[i+1].Data,iData,iDataLen);
                            ATOMIC(FileBuf[i+1].State=FILE_STATE_RECEIVE_OK;) 
                            return 1;
                        }                            
                    }
                }
            }
        }
        if(rt_tick_get()-tick>MSec(5000)) 
        {
            rt_kprintf("%s,%d : SaveFileToBuf error , Seq=%d\n",__FUNCTION__,__LINE__,iSeq);
            return 0;
        }
        rt_thread_delay(MSec(100));
    }
}

/*�ӻ������л�ȡָ���ļ���������*/
uint8_t GetFileIndexFromBuf(uint16_t iSeq)
{
    uint8_t i;
    uint32_t tick=rt_tick_get();
    
    while(1)
    {
        for(i=0;i<FILE_BUFFER_NUM;i++)
        {
            if((FileBuf[i].State == FILE_STATE_RECEIVE_OK) && (FileBuf[i].Seq==iSeq)) return i;
        }
        if(rt_tick_get()-tick>MSec(5000)) 
        {
            rt_kprintf("%s,%d : GetFileIndexFromBuf error , Seq=%d\n",__FUNCTION__,__LINE__,iSeq);
            return FILE_BUFFER_NUM;
        }
        rt_thread_delay(MSec(100));
    }
}


/**
  * @brief  Gets the sector of a given address
  * @param  None
  * @retval The sector of a given address
  */
static uint32_t GetSector(uint32_t Address)
{
    uint32_t sector = 0;

    if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
    {
        sector = FLASH_Sector_0;
    }
    else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
    {
        sector = FLASH_Sector_1;
    }
    else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
    {
        sector = FLASH_Sector_2;
    }
    else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
    {
        sector = FLASH_Sector_3;
    }
    else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
    {
        sector = FLASH_Sector_4;
    }
    else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
    {
        sector = FLASH_Sector_5;
    }
    else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
    {
        sector = FLASH_Sector_6;
    }
    else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
    {
        sector = FLASH_Sector_7;
    }
    else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
    {
        sector = FLASH_Sector_8;
    }
    else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
    {
        sector = FLASH_Sector_9;
    }
    else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
    {
        sector = FLASH_Sector_10;
    }
    else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11))*/
    {
        sector = FLASH_Sector_11;
    }
    return sector;
}


uint8_t Download(void)
{
    uint32_t i,j;
    uint16_t fileseq=0; //�ļ������
    uint8_t bufindex=0; //�ļ�����������
    uint16_t worddatanum=0;//Word�������ݸ���
    FLASH_Status status;
    uint32_t data;
    ATOMIC_VAR;
    
    CurrentProgress=0;   
    for(i=0;i<CodeTotalLength;)
    {
        bufindex=GetFileIndexFromBuf(fileseq);
        if(bufindex>=FILE_BUFFER_NUM)
        {
            rt_kprintf("%s,%d : bufindex error , Seq=%d\n",__FUNCTION__,__LINE__,fileseq);
            return 0;
        }
        else
        {
            //���㱾���ļ����ж��ٸ�Word�͵�����
            if(FileBuf[bufindex].DataLen%4)
            {
                worddatanum=(FileBuf[bufindex].DataLen/4) + 1 ;
                memset(&(FileBuf[bufindex].Data[FileBuf[bufindex].DataLen]),0,(1024-FileBuf[bufindex].DataLen)); //������������ֵȫ������Ϊ0
            }
            else
            {
                worddatanum=(FileBuf[bufindex].DataLen/4);
            }
            
            //�������������ص�flash��
            for(j=0;j<worddatanum;j++)
            {
                memcpy((uint8_t *)(&data),&(FileBuf[bufindex].Data[4*j]),4);
                status=FLASH_ProgramWord(FLASH_USER_START_ADDR + CodeDownloadLength + 4 * j, data);
                if(status != FLASH_COMPLETE)
                {
                    rt_kprintf("Flash Promgram word error (addr=0x%x)! \n",FLASH_USER_START_ADDR + CodeDownloadLength + 4 * j);
                    return 0;
                }
            }
            ATOMIC(FileBuf[bufindex].State=FILE_STATE_DOWNLOAD_OK;)
            i+=FileBuf[bufindex].DataLen;
        }
        CodeDownloadLength=i;
        fileseq++;
        CurrentProgress=(uint8_t)(100.0f*CodeDownloadLength/CodeTotalLength);
    }
    CurrentProgress=100;
    return 1;
}

/*����flash����*/
uint8_t EraseFlash(uint32_t iDataLen)
{
    uint32_t sector_start = 0;
    uint32_t sector_end = 0;
    uint32_t sector_counter = 0;
    
    EraseFlashResult=1;
    /* Unlock the Flash Program Erase controller */
    FLASH_Unlock();

    /* Clear pending flags (if any) */
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                    FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    /* Get the number of the start and end sectors */
    sector_start = GetSector(FLASH_USER_START_ADDR);
    sector_end = GetSector(FLASH_USER_START_ADDR + iDataLen);

    if(sector_end > FLASH_Sector_11)
    {
        rt_kprintf("��ַ�������flash��Χ(0x%08x)\n", FLASH_USER_START_ADDR + iDataLen);
        EraseFlashResult = 2;
        return 0;
    }
    /* Strat the erase operation */
    sector_counter = sector_start;
    while (sector_counter <= sector_end)
    {
        /* Device voltage range supposed to be [2.7V to 3.6V], the operation will be done by word */
        if (FLASH_EraseSector(sector_counter, VoltageRange_3) != FLASH_COMPLETE)
        {
            rt_kprintf("error : Sector 0x%02x erase faild", sector_counter);
            EraseFlashResult = 2;
            return 0;
        }
        /* jump to the next sector */
        sector_counter += 8;
    }
    EraseFlashResult = 3;
    return 1;
}

uint8_t CalChecksum(void)
{
    uint32_t i=0;
    CodeLowerCheckSum=0;
    
    CheckSumResult=1;
    for(i=0;i<CodeTotalLength;i++) 
    {
        CodeLowerCheckSum+=(uint8_t)(*((uint8_t *)(FLASH_USER_START_ADDR+i)));
    }
    if(CodeLowerCheckSum != CodeUpperCheckSum)
    {
        rt_kprintf("error : lower check sum = %d ,upper check sum =%d\n",CodeLowerCheckSum,CodeUpperCheckSum);
        CheckSumResult=2;
        return 0;
    }
    CheckSumResult=3;
    return 1;   
}

void EnterUser(void)
{
    uint32_t user_app_addr = FLASH_USER_START_ADDR;//BootLoaderռ��128K�Ŀռ�
	void (*user_app)(void) = (void (*)(void)) (*((uint32_t *) (user_app_addr + 4))); 
        
	if ((*(__IO uint32_t*)user_app_addr)<=0x20020000 )
	{
		rt_kprintf("enter user Program \n");
		__set_MSP(*(__IO uint32_t*) user_app_addr); 
		user_app();
	}
	else
	{
		rt_kprintf("no user Program \n");
	}
}

/*����ͨ�ţ������߳�*/
void update_thread_entry(void* parameter)
{
    rt_err_t err;
    uint32_t tick;
    uint8_t result=1;
	err = rt_sem_init(&UpdateSem,"nwsem",0,RT_IPC_FLAG_FIFO);
	if(err != RT_EOK)
	{
		rt_kprintf("UpdateSem init fail !\n");
        RT_ASSERT(err == RT_EOK);
	}

    while(1)
    {
        result=1;
        rt_sem_take(&UpdateSem,RT_WAITING_FOREVER);
        
        rt_kprintf("erase start\n");
        result=EraseFlash(CodeTotalLength);
        if(result==1)rt_kprintf("erase ok\n");
        else rt_kprintf("erase error\n");
        
        //����
        if(result==1)
        {
            tick=rt_tick_get();
            result=Download();
            if(result==1) rt_kprintf("download ok , time=%d ms\n",(rt_tick_get()-tick)*10);
            else rt_kprintf("download error \n");
        }
        
        //����У���
        if(result==1)
        {
            result=CalChecksum();
            if(result==1) rt_kprintf("cal checksum ok , time=%d ms\n",(rt_tick_get()-tick)*10);
            else rt_kprintf("cal checksum error\n");
        }
        
        //���ý���user����ı�־λ
        if(result==1)
        {
            //�ȴ���λ����ȡУ����
            tick = rt_tick_get();
            while(1)
            {
                if(rt_tick_get()-tick>MSec(1000)) break;
                if(CheckSumResult==0) break; //��λ���ɹ���ȡ��У�����󣬻ὫУ��������
                rt_thread_delay(MSec(20));
            }
            
            //������λ�����´ν����û�����
            NetLocalAddr.BootFlag = 0;
            NET_LocalSave();
            NVIC_SystemReset();
        }
        while(rt_sem_trytake(&UpdateSem)==RT_EOK);
    }
}

void buf(void)
{
    uint8_t i;
    for(i=0;i<FILE_BUFFER_NUM;i++)
    {
        rt_kprintf("seq=%d,state=%d,len=%d\n",FileBuf[i].Seq,FileBuf[i].State,FileBuf[i].DataLen);
    }
}
void flashdata(uint32_t startaddr,uint32_t num)
{
    uint32_t i;
    for(i=0;i<num;i++)
    {
        rt_kprintf("%02x ",(uint8_t)(*((uint8_t *)(FLASH_USER_START_ADDR+i))));
    }
}
FINSH_FUNCTION_EXPORT(buf,buf)
FINSH_FUNCTION_EXPORT(flashdata,flashdata)
