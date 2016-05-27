#include "main.h"

void system_Init(void);
void led_Manage(SYS_Status stat);
void recordData(void);
void system_Failure(void);
	
u32 flash_DataCacheAddress=DATA_START_ADDRESS;
u32 flash_DataCacheSize=0;

extern u16 data_page0[ADS_BUFF_MAX];
extern u16 data_page1[ADS_BUFF_MAX];
extern u8 selectChannel;
extern u8 selectPage;
extern u16 data_index;
extern u32 data_count;

u8 recordingTime = 1;

#define DATA_START_ADDR 0x0
u32 data_addr = DATA_START_ADDR;

u8 myRtcInit(void);
RTC_DateTypeDef d;
RTC_TimeTypeDef t;

extern u8 writeFlag;

u32 mount;
u32 lasttime;

const u8 renCommand[]="@echo Change File Name\n@ren data.txt data.csv\n@pause\n";
FATFS fs;
FIL fnew;
FRESULT res_flash;
UINT fnum;
BYTE ReadBuffer[512]={ 0 };
DWORD fsize=0;
u32 id;
u8 i,j;

SYS_Status systemStatus = SYS_INIT;
u32 startTime=0;

//#define MEM_TEST
#ifdef MEM_TEST
#define WRITE_BUFF_SIZE 256
#define SPIFLASH_DMA/////////////////////////////////
u8 toWrite[WRITE_BUFF_SIZE];
u8 toRead[WRITE_BUFF_SIZE];
u16 indexWrite;
u16 buff_size[10]={1,2,4,8,16,32,64,128,256,512};
#endif

INPUT_TYPE input=INPUT_NONE;
u8 rxdata[4];
extern u8 prepareFlag;

int main(void)
{
	system_Init();
	RX_Mode();
	systemStatus = SYS_INIT;
#ifdef MEM_TEST
	for(indexWrite=0;indexWrite<WRITE_BUFF_SIZE;indexWrite++)
	{
		toWrite[indexWrite]=indexWrite%64;
	}
	EF_spiFlashEraseBlock(0x00);
	__nop();
	
#ifdef SPIFLASH_DMA	
	TP1_L;
	TP2_L;
	EF_spiFlashDmaPageWrite(0x00,toWrite,WRITE_BUFF_SIZE);
	TP1_H;
#else
	TP1_L;
	TP2_L;
	EF_spiFlashWrite(0x00,toWrite,WRITE_BUFF_SIZE);
	TP1_H;
#endif	
	EF_spiFlashRead(0x00,toRead,WRITE_BUFF_SIZE);
	__nop();
#endif

	for(i=0;i<5;i++)
	{
		for(j=0;j<3;j++)
		{
			led_Off(LED_All);
			led_On(LED_Blue<<j);
			DelayMs(80);	
		}
	}
	res_flash = f_mount(&fs,"1:",1);
	if(res_flash == FR_NO_FILESYSTEM)
	{
		led_Off(LED_All);
		for(i=0;i<12;i++)
		{
			led_Toggle(LED_All);
			DelayMs(250);
		}
		led_Off(LED_All);
		led_On(LED_Red);
		res_flash=f_mkfs("1:",1,0);
		DelayMs(1000);
		if(res_flash == FR_OK)
		{
			res_flash = f_setlabel("1:ENIGMO-II");
			res_flash = f_mount(0,"1:",1);
			DelayMs(10);
			led_Off(LED_Red);
		}
		else
		{
			system_Failure();
		}
	}
	else if(res_flash==FR_OK)
	{
		led_Off(LED_All);
	}
	else if(res_flash !=FR_OK)
	{
		system_Failure();
	}
	systemStatus = SYS_IDLE;
	EF_OpenMassStorage();
	
	while(1)
	{
		led_Manage(systemStatus);
		if( systemStatus != SYS_USB_CONNECTED )
		{
			input=INPUT_NONE;
			if(NRF24L01_RxPacket(rxdata))
			{
					switch(rxdata[0])
					{
					case 'T':
						input=INPUT_SETTIME;
						//SELFON_Enable();
						break;
					case 'S':
						input=INPUT_START;
						break;
					case 'R':
						//input=INPUT_REQDATA;
						//SELFON_Disable();
						break;
					case 'C':
						input=INPUT_CLEAR;
						//SELFON_Enable();
						break;
					case 'P':
						input=INPUT_OFF;
						//SELFON_Disable();
						break;
					}
					rxdata[0]='Z';		
			}
		}
		switch (systemStatus)
		{
			case	SYS_USB_CONNECTED:
				__nop();
				break;
			
			case SYS_PREPARE:
				led_Off(LED_All);
				DelayMs(500);
				for(i=0;i<(((ADS_FREQ_H*ADS_CH_NUM)*2*recordingTime/64)+2);i++)
				{
					EF_spiFlashEraseBlock(DATA_START_ADDR+i*MX66_Block_Size);
					led_Toggle(LED_Blue);
				}
				led_On(LED_Blue);
				data_addr=DATA_START_ADDR;
				led_Off(LED_All);
				if( prepareFlag != 1)
					systemStatus=SYS_IDLE;
				else
					systemStatus=SYS_RECORDING;
				break;
			
			case SYS_IDLE:
				if( input == INPUT_SETTIME )
				{
						recordingTime=(u16)(rxdata[2]-'0')*10+(u16)(rxdata[3]-'0');
						led_Flash(LED_Red,3);			
				}
				else if( input == INPUT_CLEAR )
				{
						systemStatus = SYS_PREPARE;
						led_Flash(LED_Red,3);	
				}				
				else if( input == INPUT_START )
				{
						systemStatus=SYS_RECORDING;
				}
				else if( input == INPUT_OFF )
				{
						led_Flash(LED_Red,10);
				}
				__nop();
				break;
				
			case SYS_FORMAT:
				res_flash = f_mount(&fs,"1:",1);
				if(res_flash == FR_OK)
				{
					led_Off(LED_All);
					led_On(LED_Red);
					res_flash=f_mkfs("1:",1,0);
					DelayMs(1000);
					if(res_flash == FR_OK)
					{
						res_flash = f_setlabel("1:ENIGMO-II");
						res_flash = f_mount(0,"1:",1);
						DelayMs(10);
						led_Off(LED_Red);
						led_Flash(LED_Green,5);
					}
					else
					{
						led_Flash(LED_Red,5);
					}
					res_flash = f_mount(0,"1:",1);
				}
				systemStatus = SYS_IDLE;
				break;
				
			case SYS_RECORDING:
				//blocking thread to ensure high sapling frequency
				led_Off(LED_All);
				led_On(LED_Red);
			  lasttime = EF_TickGetTimeMs();
				mount=(u32)(ADS_FREQ_H*ADS_CH_NUM)*recordingTime*1000+512;
				writeFlag = 0;
				selectChannel=0;
				selectPage=0;
				ADS_Start();
				while( data_count < mount )
				{
					if( writeFlag == 1)
					{
						TP3_L;
						writeFlag = 0;
						if( selectPage == 1)
						{
							EF_spiFlashDmaPageWrite(data_addr,(u8*)data_page0,ADS_BUFF_MAX*2);
						}
						else
						{
							EF_spiFlashDmaPageWrite(data_addr,(u8*)data_page1,ADS_BUFF_MAX*2);
						}
						data_addr += ADS_BUFF_MAX_DUAL;
						if(EF_TickGetTimeMs()-lasttime > 50 )
						{
							led_Toggle(LED_Red);
							lasttime = EF_TickGetTimeMs();
						}
						TP3_H;
					}
				}
				ADS_Stop();
				led_Off(LED_All);
				led_On(LED_Green);
				DelayMs(200);
				led_Off(LED_Green);
				saveDataToFlash();
				prepareFlag = 0;
				systemStatus=SYS_IDLE;
				break;
				
			default:
				systemStatus=SYS_IDLE;
		}
		DelayMs(20);
	}
}

const u16 pData[15] = {1875,1923,1097,1201,2103,1023,1781,1031,901,832,1312,231,1193,1212,3123} ;

#define SAVE_DATA_TO_FLASH
#ifdef SAVE_DATA_TO_FLASH
void myitoa(int num,char *str,int radix)   
{    
    char index[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";   
    unsigned unum;  
    int i=0,j,k;   
	  char temp; 
    if(radix==10&&num<0)  
    {   
        unum=(unsigned)-num;   
        str[i++]='-';   
    }   
    else unum=(unsigned)num; 
    do    
    {   
        str[i++]=index[unum%(unsigned)radix];   
        unum/=radix;   
    }while(unum);   
    str[i]='\0';   
    if(str[0]=='-') k=1; 
    else k=0;   
  
    for(j=k;j<=(i-k-1)/2.0;j++)   
    {   
        temp=str[j];   
        str[j]=str[i-j-1];   
        str[i-j-1]=temp;   
    }   
    //return str;   
}  

void saveDataToFlash(void)
{
	u32 i;
	int filePathTmp=0;
	char filePath[10]="";
	char dirPathFull[50]="1:";
	char filePathFull[256]="";
	char *nullString="";
	int result=0;
	
	RTC_DateTypeDef date;
	RTC_TimeTypeDef time;
	
	RTC_GetDate(RTC_Format_BIN,&date);
	RTC_GetTime(RTC_Format_BIN,&time);
	
	filePathTmp=date.RTC_Year+2000;
	myitoa(filePathTmp,filePath,10);
	strcat(dirPathFull,filePath);

	filePathTmp=date.RTC_Month;
	myitoa(filePathTmp,filePath,10);
	if(filePathTmp<10)
		strcat(dirPathFull,"0");
	strcat(dirPathFull,filePath);

	filePathTmp=date.RTC_Date;
	myitoa(filePathTmp,filePath,10);
	if(filePathTmp<10)
		strcat(dirPathFull,"0");
	strcat(dirPathFull,filePath);
	strcat(dirPathFull,"_");
	
	filePathTmp=time.RTC_Hours;
	myitoa(filePathTmp,filePath,10);
	if(filePathTmp<10)
		strcat(dirPathFull,"0");
	strcat(dirPathFull,filePath);
	
	filePathTmp=time.RTC_Minutes;
	myitoa(filePathTmp,filePath,10);
	if(filePathTmp<10)
		strcat(dirPathFull,"0");
	strcat(dirPathFull,filePath);
	
	filePathTmp=time.RTC_Seconds;
	myitoa(filePathTmp,filePath,10);
	if(filePathTmp<10)
		strcat(dirPathFull,"0");
	strcat(dirPathFull,filePath);
	
	led_Off(LED_All);
	led_On(LED_Blue);
	res_flash=f_mount(&fs,"1:",1);
	
	if(res_flash == FR_OK)
	{
		res_flash = f_mkdir(dirPathFull);
		
		strcpy(filePathFull,nullString);
		strcat(filePathFull,dirPathFull);
		strcat(filePathFull,"/");
		strcat(filePathFull,"acc.csv");

		res_flash = f_open(&fnew,filePathFull,FA_CREATE_ALWAYS | FA_WRITE );
		if( res_flash == FR_OK)
		{
			data_count = 0;
			data_addr = DATA_START_ADDR+256;
			mount=(u32)((ADS_FREQ_H*ADS_CH_NUM))*recordingTime*1000;
			while( data_count < mount )
			{
				EF_spiFlashRead(data_addr,(u8*)&data_page0[0],ADS_BUFF_MAX*2);
				for(i=0;i<ADS_BUFF_MAX;i=i+ADS_CH_NUM*ADS_FREQ_FACTOR)
				{
					result = f_printf(&fnew,"%d\n",data_page0[i]);
					if(result<0)
						break;
					led_Toggle(LED_Blue);
				}
				data_count = data_count+ADS_BUFF_MAX;
				data_addr += ADS_BUFF_MAX*2;
			}
			led_Off(LED_Blue);
			f_close(&fnew);
		}
//////////////////////////////////////////////////////////////////////////////////////
		strcpy(filePathFull,nullString);
		strcat(filePathFull,dirPathFull);
		strcat(filePathFull,"/");
		strcat(filePathFull,"uls.csv");

		res_flash = f_open(&fnew,filePathFull,FA_CREATE_ALWAYS | FA_WRITE );
		if( res_flash == FR_OK)
		{
			data_count = 0;
			data_addr = DATA_START_ADDR+256;
			mount=(u32)((ADS_FREQ_H*ADS_CH_NUM))*recordingTime*1000;
			while( data_count < mount )
			{
				EF_spiFlashRead(data_addr,(u8*)&data_page0[0],ADS_BUFF_MAX*2);
				for(i=1;i<ADS_BUFF_MAX;i=i+ADS_CH_NUM)
				{
					result = f_printf(&fnew,"%d\n",data_page0[i]);
					if(result<0)
						break;
					led_Toggle(LED_Blue);
				}
				data_count = data_count+ADS_BUFF_MAX;
				data_addr += ADS_BUFF_MAX*2;
			}
			led_Off(LED_Blue);
			f_close(&fnew);
		}
		else
		{
				led_Off(LED_Blue);	//write success
				__nop();
				led_On(LED_Red); //open error
				DelayMs(500);
				__nop();
		}
	}
	f_mount(0,"1:",1);
}
#endif

////////////////////////////////////////////////
void led_Manage(SYS_Status stat)
{
	static u8 ledState = 0;
	static u32 lastSwitchTime=0;
	static u32 timeSinceLastSwitch;
	static u16 timeOn=500;
	static u16 timeOff=500;
	static u16 ledColor = LED_Green;
	switch(stat)
	{
		case SYS_IDLE:
			ledColor = LED_Green;
			timeOn = 	1000;
			timeOff = 1000;
			break;
		
		case SYS_RECORDING:
			ledColor = LED_Green;
			timeOn = 	100;
			timeOff = 100;
			break;
		
		case SYS_USB_CONNECTED:
			ledColor = LED_Blue;
			timeOn = 	300;
			timeOff = 300;
			break;
		
		default:
			ledColor = LED_All;
			timeOn = 500;
			timeOff = 500;
			
	}	
	
	timeSinceLastSwitch = EF_TickGetTimeMs()-lastSwitchTime;
	if (ledState==0)
	{
		if (timeSinceLastSwitch>=timeOff)
		{
			led_Off(LED_All);
			led_On(ledColor);
			ledState = 1;
			lastSwitchTime = EF_TickGetTimeMs();
		}
	}
	else
	{
		if (timeSinceLastSwitch>=timeOn)
		{
			led_Off(LED_All);
			ledState = 0;
			lastSwitchTime = EF_TickGetTimeMs();
		}
	}
			
}

void system_Init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	EF_TickInit();
	led_Init();
	led_On(LED_All);
	EF_buttonInit();
	//analog_Init();
	MAGNET_Init();
	NRF24L01_Init();
	ADS_Init();
	myRtcInit();
	DelayMs(5);
	EF_spiFlashInit();
}

u8 myRtcInit(void)
{
	RTC_InitTypeDef RTC_InitStructure;
	RTC_TimeTypeDef Time;
	RTC_DateTypeDef Date;
	u16 retry = 0x1FFF;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
	PWR_BackupAccessCmd(ENABLE);
	if(RTC_ReadBackupRegister(RTC_BKP_DR0)!=0x5050)
	{
		RCC_LSEConfig(RCC_LSE_ON);
		while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET && retry>0)
		{
			retry--;
			DelayMs(10);
		}
		if(retry == 0) return 0;
	}
		
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
		RCC_RTCCLKCmd(ENABLE);
		
		RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
		RTC_InitStructure.RTC_SynchPrediv = 0xFF;
		RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
		RTC_Init(&RTC_InitStructure);
		
		Time.RTC_H12=RTC_H12_PM;
		Time.RTC_Hours=12;
		Time.RTC_Minutes=59;
		Time.RTC_Seconds=50;
		Date.RTC_Year=16;
		Date.RTC_Month=4;
		Date.RTC_Date=13;
		Date.RTC_WeekDay=RTC_Weekday_Wednesday;
		RTC_SetTime(RTC_Format_BIN,&Time);
		RTC_SetDate(RTC_Format_BIN,&Date);
		RTC_WriteBackupRegister(RTC_BKP_DR0,0x5050);
		return 1;
}

void system_Failure(void)
{
	led_Off(LED_All);
	while(1)
	{
		led_On(LED_Red);
		DelayMs(100);
		led_Off(LED_Red);
		DelayMs(400);
	}
}
