/*********************************************************************
 * INCLUDES
 */

#include <stdio.h>
#include <string.h>
#include "AF.h"
#include "OnBoard.h"
#include "OSAL_Tasks.h"
#include "SampleApp.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"

#include "hal_drivers.h"
#include "hal_key.h"
#if defined ( LCD_SUPPORTED )
  #include "hal_lcd.h"
#endif
#include "hal_led.h"
#include "hal_uart.h"
#include "hal_adc.h"
#include "ds18b20.h"
#include "sapi.h"


/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// General UART frame offsets
#define FRAME_SOF_OFFSET                    0
#define FRAME_LENGTH_OFFSET                 1 
#define FRAME_CMD0_OFFSET                   2
#define FRAME_CMD1_OFFSET                   3
#define FRAME_DATA_OFFSET                   4

// ZB_RECEIVE_DATA_INDICATION offsets
#define ZB_RECV_SRC_OFFSET                  0
#define ZB_RECV_CMD_OFFSET                  2
#define ZB_RECV_LEN_OFFSET                  4
#define ZB_RECV_DATA_OFFSET                 6
#define ZB_RECV_FCS_OFFSET                  8

// ZB_RECEIVE_DATA_INDICATION frame length
#define ZB_RECV_LENGTH                      15

// PING response frame length and offset
#define SYS_PING_RSP_LENGTH                 7 
#define SYS_PING_CMD_OFFSET                 1

// Stack Profile
#define ZIGBEE_2007                         0x0040
#define ZIGBEE_PRO_2007                     0x0041

#ifdef ZIGBEEPRO
#define STACK_PROFILE                       ZIGBEE_PRO_2007             
#else 
#define STACK_PROFILE                       ZIGBEE_2007
#endif

#define CPT_SOP                             0xFE
#define SYS_PING_REQUEST                    0x0021
#define SYS_PING_RESPONSE                   0x0161
#define ZB_RECEIVE_DATA_INDICATION          0x8746

#define RX_BUF_LEN                        128



#if !defined( SAMPLE_APP_PORT )
#define SAMPLE_APP_PORT  0
#endif

#if !defined( SAMPLE_APP_BAUD )
  #define SAMPLE_APP_BAUD  HAL_UART_BR_115200
#endif

// When the Rx buf space is less than this threshold, invoke the Rx callback.
#if !defined( SAMPLE_APP_THRESH )
#define SAMPLE_APP_THRESH  64
#endif

#if !defined( SAMPLE_APP_RX_SZ )
#define SAMPLE_APP_RX_SZ  128
#endif

#if !defined( SAMPLE_APP_TX_SZ )
#define SAMPLE_APP_TX_SZ  128
#endif

// Millisecs of idle time after a byte is received before invoking Rx callback.
#if !defined( SAMPLE_APP_IDLE )
#define SAMPLE_APP_IDLE  6
#endif

// Loopback Rx bytes to Tx for throughput testing.
#if !defined( SAMPLE_APP_LOOPBACK )
#define SAMPLE_APP_LOOPBACK  FALSE
#endif

// This is the max byte count per OTA message.
#if !defined( SAMPLE_APP_TX_MAX )
#define SAMPLE_APP_TX_MAX  80
#endif

#define SAMPLE_APP_RSP_CNT  4

// This list should be filled with Application specific Cluster IDs.
const cId_t SampleApp_ClusterList[SAMPLE_MAX_CLUSTERS] =
{
  SAMPLEAPP_P2P_CLUSTERID,
  SAMPLEAPP_PERIODIC_CLUSTERID,
};

const SimpleDescriptionFormat_t SampleApp_SimpleDesc =
{
  SAMPLEAPP_ENDPOINT,              //  int   Endpoint;
  SAMPLEAPP_PROFID,                //  uint16 AppProfId[2];
  SAMPLEAPP_DEVICEID,              //  uint16 AppDeviceId[2];
  SAMPLEAPP_DEVICE_VERSION,        //  int   AppDevVer:4;
  SAMPLEAPP_FLAGS,                 //  int   AppFlags:4;
  SAMPLE_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)SampleApp_ClusterList,  //  byte *pAppInClusterList;
  SAMPLE_MAX_CLUSTERS,          //  byte  AppNumOutClusters;
  (cId_t *)SampleApp_ClusterList   //  byte *pAppOutClusterList;
};

endPointDesc_t SampleApp_epDesc =
{
  SAMPLEAPP_ENDPOINT,
 &SampleApp_TaskID,
  (SimpleDescriptionFormat_t *)&SampleApp_SimpleDesc,
  noLatencyReqs
};

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
devStates_t SampleApp_NwkState;   
uint8 SampleApp_TaskID;           // Task ID for internal task/event processing.

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

static uint8 SampleApp_MsgID;

afAddrType_t SampleApp_Periodic_DstAddr; //广播
afAddrType_t SampleApp_Flash_DstAddr;    //组播
afAddrType_t SampleApp_P2P_DstAddr;      //点播


static afAddrType_t SampleApp_TxAddr;
static uint8 SampleApp_TxSeq;
static uint8 SampleApp_TxBuf[SAMPLE_APP_TX_MAX+1];
static uint8 SampleApp_TxLen;

static afAddrType_t SampleApp_RxAddr;
static uint8 SampleApp_RxSeq;
static uint8 SampleApp_RspBuf[SAMPLE_APP_RSP_CNT];

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void SampleApp_ProcessMSGCmd( afIncomingMSGPacket_t *pkt );
void SampleApp_CallBack(uint8 port, uint8 event); 
static void SampleApp_Send_P2P_Message( void );
static void packDataAndSend(uint8 fc, uint8* data, uint8 len);


/*********************************************************************
 * @fn      SampleApp_Init
 *
 * @brief   This is called during OSAL tasks' initialization.
 *
 * @param   task_id - the Task ID assigned by OSAL.
 *
 * @return  none
 */
#define LED P1_0
void SampleApp_Init( uint8 task_id )
{
  halUARTCfg_t uartConfig;

  SampleApp_TaskID = task_id;
  SampleApp_RxSeq = 0xC3;
  SampleApp_NwkState = DEV_INIT;       

  MT_UartInit();                  //串口初始化
  MT_UartRegisterTaskID(task_id); //注册串口任务
  afRegister( (endPointDesc_t *)&SampleApp_epDesc );
  RegisterForKeys( task_id );

#ifdef ZDO_COORDINATOR
  
  //协调器初始化蜂鸣器
  P0SEL &= ~0x80;       
  P0DIR |= 0x80;  
  P0_7=1;
  //P0SEL &= ~0x02; //设置P0.1为普通IO口
  //P0DIR &= ~0X02; //按键接在P0.1口上，设P0.1为输入模式
    P1DIR |= 0x01;//led p1.0输出
  LED = 0;
  
#else
  //因为温度传感器在读取时会进行多次初始化，所以终端初始化时并没有初始化温度传感器引脚
  //P15是光敏传感器的DO引脚
 //gas or light
  P1SEL &= ~0x20;                 //设置P1.5为普通IO口
  P1DIR &= ~0x20;                 //P1.5定义为输入口
#endif
  

  SampleApp_Periodic_DstAddr.addrMode = (afAddrMode_t)AddrBroadcast;//广播
  SampleApp_Periodic_DstAddr.endPoint = SAMPLEAPP_ENDPOINT;
  SampleApp_Periodic_DstAddr.addr.shortAddr = 0xFFFF;

  // Setup for the flash command's destination address - Group 1
  SampleApp_Flash_DstAddr.addrMode = (afAddrMode_t)afAddrGroup;//组播
  SampleApp_Flash_DstAddr.endPoint = SAMPLEAPP_ENDPOINT;
  SampleApp_Flash_DstAddr.addr.shortAddr = SAMPLEAPP_FLASH_GROUP;
  
  SampleApp_P2P_DstAddr.addrMode = (afAddrMode_t)Addr16Bit; //点播 
  SampleApp_P2P_DstAddr.endPoint = SAMPLEAPP_ENDPOINT; 
  SampleApp_P2P_DstAddr.addr.shortAddr = 0x0000;            //发给协调器

  
}

/*********************************************************************
 * @fn      SampleApp_ProcessEvent
 *
 * @brief   Generic Application Task event processor.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events   - Bit map of events to process.
 *
 * @return  Event flags of all unprocessed events.
 */
uint8 d[2] = {0,0};
uint8 m = 0;
UINT16 SampleApp_ProcessEvent( uint8 task_id, UINT16 events )
{
  (void)task_id;  // Intentionally unreferenced parameter
  
  if ( events & SYS_EVENT_MSG )
  {
    afIncomingMSGPacket_t *MSGpkt;

    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( SampleApp_TaskID )) )
    {
      switch ( MSGpkt->hdr.event )
      {
      case AF_INCOMING_MSG_CMD:
        SampleApp_ProcessMSGCmd( MSGpkt );
        uint8 debugStr[50];
        if(d[0]>=5||d[1]>=70){P0_7=0;m=1;LED=1;}
        else if(d[0]<5&&d[1]<70){P0_7=1;m=0;LED=0;}
        
        sprintf(debugStr, "GAS:%d LIGHT:%d, BuzzerOn:%d", d[0],d[1],m);
        HalUARTWrite(0, debugStr, osal_strlen(debugStr));
        HalUARTWrite(0, "\r\n", 2);
        break;
      case ZDO_STATE_CHANGE:
        SampleApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
        if ( //(SampleApp_NwkState == DEV_ZB_COORD)||
            (SampleApp_NwkState == DEV_ROUTER)
            || (SampleApp_NwkState == DEV_END_DEVICE) )
        {
            
            osal_start_timerEx( SampleApp_TaskID,
                              SAMPLEAPP_SEND_PERIODIC_MSG_EVT,
                              SAMPLEAPP_SEND_PERIODIC_MSG_TIMEOUT );
        }
        else
        {
          // Device is no longer in the network
        }
        break;

      default:
        break;
      }
      osal_msg_deallocate( (uint8 *)MSGpkt );
    }

    return ( events ^ SYS_EVENT_MSG );
  }

  if ( events & SAMPLEAPP_SEND_PERIODIC_MSG_EVT )
  {
    // 
    SampleApp_Send_P2P_Message();

    // Setup to send message again in normal period (+ a little jitter)
    osal_start_timerEx( SampleApp_TaskID, SAMPLEAPP_SEND_PERIODIC_MSG_EVT,
        (SAMPLEAPP_SEND_PERIODIC_MSG_TIMEOUT + (osal_rand() & 0x00FF)) );

    // return unprocessed events
    return (events ^ SAMPLEAPP_SEND_PERIODIC_MSG_EVT);
  }


  return ( 0 );  // Discard unknown events.
}

static uint8 calcFCS(uint8 *pBuf, uint8 len)
{
  uint8 rtrn = 0;

  while (len--)
  {
    rtrn ^= *pBuf++;
  }

  return rtrn;
}


//取光照值,接在P06上
uint8 GetLight()
{
    uint8 temp=0;//百分比的整数值
    float vol=0.0; //adc采样电压  
    uint16 adc= HalAdcRead(HAL_ADC_CHANNEL_6, HAL_ADC_RESOLUTION_14); //ADC 采样值 P06口

    //最大采样值8192(因为最高位是符号位)
    if(adc>=8192)
    {
        return 0;
    }
    
    adc=8192-adc;//反相一下，因为低湿度时AO口输出较高电平
                   //          高湿度时AO口输出较低电平   

    //转化为百分比
    vol=(float)((float)adc)/8192.0;
       
    //取百分比两位数字
    temp=vol*100;

    return temp;
}

static void sendGtwReport(uint16 source, uint16 parent, uint8 temp, uint8 voltage)
{
  uint8 pFrame[ZB_RECV_LENGTH];
  
  // Start of Frame Delimiter
  pFrame[FRAME_SOF_OFFSET] = CPT_SOP; // Start of Frame Delimiter
  
  // Length
  pFrame[FRAME_LENGTH_OFFSET] = 10;
  
  // Command type
  pFrame[FRAME_CMD0_OFFSET] = LO_UINT16(ZB_RECEIVE_DATA_INDICATION);   
  pFrame[FRAME_CMD1_OFFSET] = HI_UINT16(ZB_RECEIVE_DATA_INDICATION); 
  
  // Source address
  pFrame[FRAME_DATA_OFFSET+ ZB_RECV_SRC_OFFSET] = LO_UINT16(source); 
  pFrame[FRAME_DATA_OFFSET+ ZB_RECV_SRC_OFFSET+ 1] = HI_UINT16(source);
  
  // Command ID
  pFrame[FRAME_DATA_OFFSET+ ZB_RECV_CMD_OFFSET] = LO_UINT16(SAMPLEAPP_P2P_CLUSTERID); 
  pFrame[FRAME_DATA_OFFSET+ ZB_RECV_CMD_OFFSET+ 1] = HI_UINT16(SAMPLEAPP_P2P_CLUSTERID);
  
  // Length
  pFrame[FRAME_DATA_OFFSET+ ZB_RECV_LEN_OFFSET] = LO_UINT16(4); 
  pFrame[FRAME_DATA_OFFSET+ ZB_RECV_LEN_OFFSET+ 1] = HI_UINT16(4);
  
  // Data
  pFrame[FRAME_DATA_OFFSET+ ZB_RECV_DATA_OFFSET] = temp;
  pFrame[FRAME_DATA_OFFSET+ ZB_RECV_DATA_OFFSET+ 1] = voltage; 
  pFrame[FRAME_DATA_OFFSET+ ZB_RECV_DATA_OFFSET+ 2] = LO_UINT16(parent); 
  pFrame[FRAME_DATA_OFFSET+ ZB_RECV_DATA_OFFSET+ 3] = HI_UINT16(parent);
  
  // Frame Check Sequence
  pFrame[ZB_RECV_LENGTH - 1] = calcFCS(&pFrame[FRAME_LENGTH_OFFSET], (ZB_RECV_LENGTH - 2) );
  
  // Write report to UART
  HalUARTWrite(HAL_UART_PORT_0,pFrame, ZB_RECV_LENGTH);
}



void SampleApp_ProcessMSGCmd( afIncomingMSGPacket_t *pkt )
{
  uint8 buff[20]={0};
  uint8 debugStr[50];
  switch ( pkt->clusterId )
  {
  // 接收终端上传的温度数据
  case SAMPLEAPP_P2P_CLUSTERID: 
    {
      uint16 nwkAddr = BUILD_UINT16(pkt->cmd.Data[0],pkt->cmd.Data[1]);
      uint16 parentShortAddr = BUILD_UINT16(pkt->cmd.Data[2],pkt->cmd.Data[3]);
      uint8 t[3];
      t[0] = pkt->cmd.Data[4];//temp
      t[1] = pkt->cmd.Data[5];//lightofgas
      t[2] = pkt->cmd.Data[6];//pc pc=1:gas
      if(t[2]==1)
      {
        d[0]=t[1];
        sprintf(debugStr, "gas:%3d,temp:%d", t[1],t[0]);
        HalLcdWriteString((char*)debugStr, HAL_LCD_LINE_3 );
      }
      else
      {
        d[1]=t[1];
        sprintf(debugStr, "li:%3d,temp:%d", t[1], t[0]);
        HalLcdWriteString((char*)debugStr, HAL_LCD_LINE_4 );
      }
    }
    break;

  case SAMPLEAPP_PERIODIC_CLUSTERID:

    break;

    default:
      break;
  }
}


static void sysPingRsp(void)
{
  uint8 pBuf[SYS_PING_RSP_LENGTH];
  
  // Start of Frame Delimiter
  pBuf[FRAME_SOF_OFFSET] = CPT_SOP;
  
  // Length
  pBuf[FRAME_LENGTH_OFFSET] = 2; 
  
  // Command type
  pBuf[FRAME_CMD0_OFFSET] = LO_UINT16(SYS_PING_RESPONSE); 
  pBuf[FRAME_CMD1_OFFSET] = HI_UINT16(SYS_PING_RESPONSE);
  
  // Stack profile
  pBuf[FRAME_DATA_OFFSET] = LO_UINT16(STACK_PROFILE);
  pBuf[FRAME_DATA_OFFSET+ 1] = HI_UINT16(STACK_PROFILE);
  
  // Frame Check Sequence
  pBuf[SYS_PING_RSP_LENGTH - 1] = calcFCS(&pBuf[FRAME_LENGTH_OFFSET], (SYS_PING_RSP_LENGTH - 2));
  
  // Write frame to UART
  HalUARTWrite(HAL_UART_PORT_0,pBuf, SYS_PING_RSP_LENGTH);
}


void SampleApp_CallBack(uint8 port, uint8 event)
{
  uint8 pBuf[RX_BUF_LEN];
  uint16 cmd;
  uint16 len;
  
  if ( event != HAL_UART_TX_EMPTY ) 
  {
  
    // Read from UART
    len = HalUARTRead( HAL_UART_PORT_0, pBuf, RX_BUF_LEN );
    
    if ( len>0 ) 
    {
      cmd = BUILD_UINT16(pBuf[SYS_PING_CMD_OFFSET+ 1], pBuf[SYS_PING_CMD_OFFSET]);
  
      if( (pBuf[FRAME_SOF_OFFSET] == CPT_SOP) && (cmd == SYS_PING_REQUEST) ) 
      {
        sysPingRsp();
      }
    }
  }
}

uint8 GetGasPercentage(void)
{
    uint16 adc = HalAdcRead(HAL_ADC_CHANNEL_6, HAL_ADC_RESOLUTION_14);
    const uint16 ADC_MAX = 8192;
    
    if(adc > ADC_MAX) adc = ADC_MAX;
    
    // 转换为百分比（0-100%）
    return (adc * 100) / ADC_MAX;
}

/*********************************************************************
 * @fn      SampleApp_Send_P2P_Message
 *
 * @brief   point to point.
 *
 * @param   none
 *
 * @return  none
 */
uint8 pc = 2;   //1/2
void SampleApp_Send_P2P_Message( void )
{
  uint8 str[7]={0};
  uint16 temp[2];          
  uint16 nwkAddr = NLME_GetShortAddr();
  uint16 parentShortAddr=NLME_GetCoordShortAddr();
 
  temp[0] = ReadDs18B20();//temp
  if(pc==1)
    temp[1] = GetGasPercentage();//gas
  else
    temp[1] = GetLight();//light
  str[0] = LO_UINT16(nwkAddr);
  str[1] = HI_UINT16(nwkAddr);
  str[2] = LO_UINT16(parentShortAddr);
  str[3] = HI_UINT16(parentShortAddr);
  
  str[4] = temp[0];//temp
  str[5] = temp[1];//gasorlight
  str[6] = pc;//pc
  
  uint8 debugStr[50];
  if(str[6]==1)
    sprintf(debugStr, "GAS:%d",temp[1]);
  else
    sprintf(debugStr, "Light:%d",temp[1]);
  HalLcdWriteString((char*)debugStr, HAL_LCD_LINE_3 );
  sprintf(debugStr, "TEMP:%d  %d", temp[0],pc);
  HalLcdWriteString((char*)debugStr, HAL_LCD_LINE_4 );
  
  if(str[6]==1)
    sprintf(debugStr, "GAS:%d TEMP:%d %d", temp[1],temp[0],pc);
  else
    sprintf(debugStr, "Light:%d TEMP:%d %d", temp[1],temp[0],pc);
  
  HalUARTWrite(0, debugStr, osal_strlen(debugStr));
  HalUARTWrite(0, "\r\n", 2);
  
  SampleApp_TxAddr.addrMode = (afAddrMode_t)Addr16Bit;
  SampleApp_TxAddr.endPoint = SAMPLEAPP_ENDPOINT;
  SampleApp_TxAddr.addr.shortAddr = 0x0;




  //无线发送到协调器
  if ( AF_DataRequest( &SampleApp_P2P_DstAddr, &SampleApp_epDesc,
                       SAMPLEAPP_P2P_CLUSTERID,
                       7,
                       str,
                       &SampleApp_MsgID,
                       AF_DISCV_ROUTE,
                       AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
  {
  }
  else
  {
    // Error occurred in request to send.
  }
}

uint8 CheckSum(uint8 *pdata, uint8 len)
{
	uint8 i;
	uint8 check_sum=0;

	for(i=0; i<len; i++)
	{
		check_sum += pdata[i];
	}
	return check_sum;
}

//数据打包发送
/**
*fc:功能码
*data:上传的数据
*len:数据长度
格式:len,校验,fc,内容,$,@,
*/
void packDataAndSend(uint8 fc, uint8* data, uint8 len)
{
    osal_memset(SampleApp_TxBuf, 0, SAMPLE_APP_TX_MAX+1);


    //数据包长度
    SampleApp_TxBuf[0]=3+len;

    //功能码
    SampleApp_TxBuf[2]=fc;

    //发送的数据
    if(len>0)
    {
        osal_memcpy(SampleApp_TxBuf+3, data, len);
    }

    //校验和,从fc开始，
    SampleApp_TxBuf[1]=CheckSum(SampleApp_TxBuf+2, len+1);

    //数据结尾
    SampleApp_TxBuf[3+len]='$';
    SampleApp_TxBuf[4+len]='@';

    //发送长度
    SampleApp_TxLen=5+len;

    //接着发数据包
    HalUARTWrite(0,SampleApp_TxBuf, SampleApp_TxLen);
}
