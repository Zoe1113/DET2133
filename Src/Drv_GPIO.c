/**************************************************************************
文件名称：	Drv_GPIO.c
说    明：	GPIO初始化函数集合（驱动层）
作    者:	Liaoy
版    本：	V1.0
芯	  片:	SN8P29XX
修订记录：
**************************************************************************/
#include "Include.h"
uint8 g_MotorSystick = 0;		//振动马达的剩余时间
// bit ET_mode_buf = 0;		//模式备份标志(具备耳额物产品需要)
/**************************************************************************
函数名称：	Cal_Inspect_Detect()
函数功能：	校准模式、绑定模式入口检测
输入参数：	Port_Debug、Port_Cal
输出参数：	uSetFlag
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Cal_Inspect_Detect(void)
{
	//绑定检测模式、校温检测模式、系数调整模式判断
	if( !Port_Debug )
	{
		if ( eTestmode_num == Insptectmode )
		{
			#if ParamModif
				LED_CloseAll();
				eMain_Task = Task_ParamModifymode;	//生产模式短路debug口则进入系数调整模式
			#endif
		}
		else
		{
			F_FirstEnter_SetMode = 0;
			eMain_Task = Task_BondTestmode;	//用户模式下短路debug进入绑定检测模式
		}
	}

	// //设置态决不允许进入校温，汇编和C的程序结构不一样，未初始化adc
	// if( !Port_Debug && eMain_Task == Task_ReadyMode )
	// {
	// 	#if Func_Ble
    //         F_Ble_En = Disable;         //进入校准关闭蓝牙
    //         Port_Ble_En = 0;            //蓝牙失能
    //     #endif
	// 	eMain_Task = Task_Calimode;		//用户模式下短路debug和cal进入校温模式
	// }
}

/**************************************************************************
函数名称：	CF_Check()
函数功能：	单位状态、单位可切换状态查询
输入参数：	Port_CF、Port_Change_CF
输出参数：	uSetFlag
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void CF_Check(void)
{
	//单位状态检测
	#if !Func_unit
		uSetFlag.bits.Unit = Unit_C;
	#else
		uSetFlag.bits.Unit = Unit_F;
	#endif

	//单位可切换状态检测
	#if Func_Change_CF
		uSetFlag.bits.Unit_Change = Unit_Change_En;
	#else
		uSetFlag.bits.Unit_Change = Unit_Change_Dis;
	#endif

}

/**************************************************************************
函数名称：	Forehead_Ear_Check()
函数功能：	模式切换查询
输入参数：	无
输出参数：	uSetFlag.bits.VoiceEnable
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Forehead_Ear_Check(void)
{

	static bit Frist_Earmode = 0;
	static bit Frist_Foreheadmode = 0;
	static bit Frist_Mode = 0;
    if( eTestmode_num != Insptectmode && eTestmode_num != Blackbodymode && !uErrFlag.bits.Er2 ) 
    {
		#if Func_Hall
		if(Port_Hall)	//霍尔检测
		#else
        if(Port_Rod)	//检测杆检测
		#endif
		{
			if(!Frist_Earmode)
			{
				eTestmode_num = Earmode;	//默认耳温模式
				Frist_Earmode = 1;
				Frist_Mode = 1;
				Frist_Foreheadmode = 0;
			}

   
		}              
        else
        {
            if(!Frist_Foreheadmode)
			{
				eTestmode_num = Foreheadmode;	//默认额温模式
				Frist_Foreheadmode = 1;
				Frist_Mode = 1;
				Frist_Earmode = 0;	
			}
            
        }

		if(uSetFlag.bits.Ready_First)
		{
			Frist_Mode = 0;
		}

		if(Frist_Mode)
		{
			Frist_Mode = 0;
        	eMain_Task = Task_ReadyMode;
			eReadyTask_Sta = Ready_Refresh;
			#if !Have_Voice_Func //蜂鸣款的在模式切换的时候，由于蜂鸣操作时延时。所以清屏和显示会有延时，为了解决这一问题把清屏的显示提前
				Clr_Disp888();
				Disp_Ready();	//首次开机必须显示_ _._
				Disp_Unit();	//显示单位
				Disp_VoiceSign(uSetFlag.bits.VoiceEnable);
				LED_CloseAll();
				#if Func_White
					LED_White_En();
				#else
					LED_Green_En();		//开启绿光
				#endif
			#endif
			Disp_ModeSign();	//显示模式符号
			Er1_Display_Sound(RESET);		//清除Er1相关标志位
			Adc_Channel_Init(TPTONTC);
			Auto_TurnOff_Time_Sel();
			if( uSetFlag.bits.VoiceEnable == 1)// && ET_mode_buf
			{
				
				#if Have_Voice_Func
					PlayStatueParam(2 , 0 , 0);
					PlayStatueParam(1 , Play_Stop,0);
					PlayStatueParam(1 , Play_Di,0);
				#else
					if(eTestmode_num == Foreheadmode)
					{
						BZ_Beep50();
						BZ_Beep50();		//蜂鸣2声
					}
				#endif
			}
			#if Have_Motor
				if( (uSetFlag.bits.VoiceEnable == 0) &&(eTestmode_num != Earmode))
				{
					g_MotorSystick = 25;
				}			 	
				else
				{
					g_MotorSystick = 0;
				}
			#endif
		}
		
    }
}

/**************************************************************************
函数名称：	Voice_Change()
函数功能：	语音开关状态切换
输入参数：	无
输出参数：	无
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Voice_Change(void)
{
    uSetFlag.bits.VoiceEnable = !uSetFlag.bits.VoiceEnable ;  
}

/**************************************************************************
函数名称：	GPIO_Init()
函数功能：	IO口初始化设置
输入参数：	P0、P1、P2、P3、P4、P5
输出参数：	P0、P1、P2、P3、P4、P5
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void GPIO_Init(void)
{
	//SDA/SCL输出为0(因为采用的是硬体I2C),按键输入上拉高电平,XIN/XOUT输入不上拉低
	//0: input mode, 1: output mode
	P0M = 0x0C;			//0b0000 1100
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P0UR = 0x33;		//0b0011 0011
	//0: low level, 1: high level
	P0 = 0x3F;			//0b0011 1111

#if Have_Voice_Func
	//白灯输出不上拉高电平、MOSI输出不上拉高电平，MISO输入不上拉高电平,SCK输出不上拉高电平,TX输入不上拉高电平（防止向BLE供电）,RX输入不上拉,VOC_EN输出上拉低,BLE_LINK输入不上拉高
	//0: input mode, 1: output mode
	P1M = 0x4B;			//0b0100 1011
	//0: LCD functon pin, 1: IO pin
	P1SEG = 0xFF;		//均为普通IO口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P1UR = 0x40;		//0b0100 0000
	//0: low level, 1: hFFigh level
	P1 = 0xBF;			//0b1011 1111
#else
	//白灯输出不上拉高电平,MOSI输出不上拉高电平,MISO输入不上拉高电平,SCK输出不上拉高电平,TX输入不上拉高电平（防止向BLE供电）,RX输入不上拉,VOC_EN,BLE_LINK输入不上拉
	//0: input mode, 1: output mode
	P1M = 0x4F;			//0b0100 1111
	//0: LCD functon pin, 1: IO pin
	P1SEG = 0xFF;		//均为普通IO口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P1UR = 0x40;		//0b0100 0000
	//0: low level, 1: high lDevel
	P1 = 0xBB;			//0b1011 1011
#endif

	//MOTOR输出不上拉低电平,G320_EN输出不上拉高、检测杆输入上拉高、HALL输入上拉高、Debug输入上拉高电平,RGB输出不上拉高电平
	//0: input mode, 1: output mode
	P5M = 0xE3;			//0b1110 0011
	//0: LCD functon pin, 1: IO pin
	P5SEG = 0xFF;		//0b1111 1111
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P5UR = 0x1C;		//0b0001 1100
	//0: low level, 1: high level
	P5 = 0xFE;			//0b1111 1110

	//P22-P27为seg口，LINK脚输入 不上拉 高,EN输出 上拉 低，额温盖检测脚输入上拉 高
	//0: input mode, 1: output mode
	P2M = 0x01;			//0b0000 0001
	//0: LCD functon pin, 1: IO pin
	P2SEG = 0x03;		//0b0000 0011,P23-P27均为LCD口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P2UR = 0x00;		//0b0000 0000
	//0: low level, 1: high level
	P2 = 0x03;			//0b0000 0011

	//0: input mode, 1: output mode
	P3M = 0x00;			//均为LCD口
	//0: LCD functon pin, 1: IO pin
	P3SEG = 0x00;		//均为LCD口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P3UR = 0x00;		//均为LCD口
	//0: low level, 1: high level
	P3 = 0x00;			//均为LCD口

	//0: input mode, 1: output mode
	P4M = 0x00;			//均为LCD口
	//0: LCD functon pin, 1: IO pin
	P4SEG = 0x00;		//均为LCD口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P4UR = 0x00;		//均为LCD口
	//0: low level, 1: high level
	P4 = 0x00;			//均为LCD口
}


/**************************************************************************
函数名称：	GPIO_PowerDown()
函数功能：	IO口初始化设置
输入参数：	P0、P1、P2、P3、P4、P5
输出参数：	P0、P1、P2、P3、P4、P5
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void GPIO_PowerDown( void )
{
	//按键输入上拉高电平,XIN/XOUT输入不上拉0,E2供电口输出上拉搞
	//0: input mode, 1: output mode
	P0M = 0x3C;			//0b0011 1100
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P0UR = 0x33;		//0b0011 0011
	//0: low level, 1: high level
	P0 = 0x3F;			//0b0011 1111

#if !Func_Ble
	//白灯输入上拉高电平、MISO/MOSI/SCK/TX均输出不上拉低电平,VOC_EN输出上拉低,BLE_LINK输入不上拉0
	//0: input mode, 1: output mode
	P1M = 0xFE;			//0b1111 1110
	//0: LCD functon pin, 1: IO pin
	P1SEG = 0xFF;		//均为普通IO口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P1UR = 0x41;		//0b0100 0001
	//0: low level, 1: high level
	P1 = 0x01;			//0b0000 0001

	//P23-P27为seg口，BLE_Link、BLE_EN、额温盖检测脚输出 上拉 高,其他均为输入上拉高电平
	//0: input mode, 1: output mode
	P2M = 0x07;			//0b 0000 0111
	//0: LCD functon pin, 1: IO pin
	P2SEG = 0x07;		//0b00000111,P23-P27均为LCD口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P2UR = 0x00;		//0b 0000 0000
	//0: low level, 1: high level
	P2 = 0x00;			//0b 0000 0000
#else
	//BLE_BUSY输出0,BZ不上拉输出0,MISO/MOSI/SCK/TX均输出不上拉低电平,VOC_EN输出上拉低,BLE_LINK输入不上拉0
	//0: input mode, 1: output mode
	P1M = 0xFE;			//0b 1111 1110
	//0: LCD functon pin, 1: IO pin
	P1SEG = 0xFF;		//均为普通IO口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P1UR = 0x41;		//0b 0100 0001
	//0: low level, 1: high level
	P1 = 0x01;			//0b 0000 0001

	//P23-P27为seg口，BLE_Link、BLE_EN、额温盖检测脚输出 上拉 高,其他均为输入上拉高电平
	//0: input mode, 1: output mode
	P2M = 0x07;			//0b 0000 0111
	//0: LCD functon pin, 1: IO pin
	P2SEG = 0x07;		//0b00000111,P23-P27均为LCD口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P2UR = 0x00;		//0b 0000 0000
	//0: low level, 1: high level
	P2 = 0x00;			//0b 0000 0000
#endif

    //MOTOR输出不上拉低,CF/C/F输出不上拉0，CAL/Debug输入上拉高电平,RGB输入上拉高
    //0: input mode, 1: output mode
    P5M = 0x05;			//0b0000 0101
    //0: LCD functon pin, 1: IO pin
    P5SEG = 0xFF;		//0b11111111
    //0: disable pullup, 1: enable pullup, pull resistor = 200k
    P5UR = 0xFA;		//0b1111 1010
    //0: low level, 1: high level
    P5 = 0xFA;			//0b1111 1010


	//0: input mode, 1: output mode
	P3M = 0x00;			//均为LCD口
	//0: LCD functon pin, 1: IO pin
	P3SEG = 0x00;		//均为LCD口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P3UR = 0x00;		//均为LCD口
	//0: low level, 1: high level
	P3 = 0x00;			//均为LCD口

	//0: input mode, 1: output mode
	P4M = 0x00;			//均为LCD口
	//0: LCD functon pin, 1: IO pin
	P4SEG = 0x00;		//均为LCD口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P4UR = 0x00;		//均为LCD口
	//0: low level, 1: high level
	P4 = 0x00;			//均为LCD口
}