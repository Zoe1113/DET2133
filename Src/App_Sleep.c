/**************************************************************************
文件名称：	App_Sleep.c
说    明：	关机模式（应用层）
作    者:	Liaoy
版    本：	V1.0
芯	  片:	SN8P29XX
修订记录：
**************************************************************************/
#include "Include.h"

eSleep_Status eSleepTask_Sta;

void App_Sleep(void)
{
	static uint8 l_buf;

	switch ( eSleepTask_Sta )
	{
		//关机初始化
		case Sleep_false:
			//请养成习惯，还原主任务相关变量
			//如果进入过生产模式则需要清除记忆
			Time_CountDown_5s_timeout(RESET);		//清除倒计时相关标志位
			Er1_Display_Sound(RESET);
			Er2_Display_Sound(RESET);
			//如果进入过检验模式或黑体模式，则需初始化
			if(eTestmode_num == Insptectmode )
			{
				TimeAndMode_Set();
				Clr_All_Memory();
				CF_Check();
				eTestmode_num = Earmode;
			}
			//如果进入过检验模式或黑体模式，则需初始化
			if(eTestmode_num == Blackbodymode )
			{
				eTestmode_num = Earmode;
			}

			//主要变量初始化（错误清0）

			
			//各任务状态初始化
			eReadyTask_Sta = Ready_Init;
			eInitTask_Sta = Init_Set;
			eBle_Sta = Ble_Disable;	

            if(  uSetFlag.bits.VoiceEnable ==1 )
            {
                #if Have_Voice_Func
                    PlayStatueParam(2 , 0 , 0);
                    AM5BA_Send_Cmd(LL_Stop, SS_Stop, Nodata);
					Delay10ms(1);
                #endif				
            }

			//记忆状态初始化
			F_Mem_FirstEnter = 0;
			F_FirstEnter_SetMode = 0;

            #if !Have_Voice_Func
			    Drv_BZ_Disable();
            #endif
			LED_CloseAll();	//关背光

			// 判断是否为上电关机，为上电关机则不显示OFF
            if(uSetFlag.bits.PowerFirst)
			{
				Disp_OFF();
				Delay10ms(50);
			}
			else
			{
				uSetFlag.bits.PowerFirst = 1;
			}
			Clr_Disp();
			#if Func_Reverse_Disp
				LED_Reverse_Disp_Dis();
			#endif
			eSleepTask_Sta = Sleep_waitkey;
			break;

		//等待按键释放
		case Sleep_waitkey:
			//如果关机时一直长按则进入黑体模式(此处与原汇编不一样)
			if(uKeyPress.bits.OKeyPress && !uStaFlag.bits.LowBat)
			{
                // L_ModePressTime++;
                //  if(uKeyHold.bits.OKeyHold)
				// {
				// 	Port_Ble_En = 0;
				// 	lcd_point_en();	//显示小数点
				// 	#if Func_Reverse_Disp
				// 		LED_Reverse_Disp_En();
				// 	#endif
                //     if( uSetFlag.bits.VoiceEnable )
                //     {
                //         #if Have_Voice_Func
				// 			AM5BA_Send_Cmd(LL_Broadcast, SS_Di, 0x00);
                //         #else
                //             BZ_Beep400();
                //             BZ_Beep400();
                //             BZ_Beep400();
                //         #endif
                //     }

				// 	while( !Port_On )//等待释放进入
				// 	{
				// 		//喂狗
		        //         WDTR = 0x5A;
				// 	}	
				// 	Disp_CAL();	//显示CAL表示进入黑体模式
				// 	Delay10ms(100);
				// 	Disp_Code(Soft_Code);	//显示程序编码
				// 	Disp_Version(Soft_External_Version);	//显示对外程序版本
				// 	Delay10ms(100);
				// 	HalKey_KeyClr();
				// 	Adc_Channel_Init(TPTONTC);	//某些模式会只采集tp故必须切回ntc采集，ADC初始化和通道切换
                //     Er2_Display_Sound(RESET);//复位Er2错误
				// 	#if Func_Ble
                //     	eBle_Sta = Ble_Disable;	
                //     	F_Ble_En = Disable;
			    //     	F_Ble_Blink = Disable;	//关闪烁
                //     #endif
				// 	eTestmode_num = Blackbodymode;	//代表进入黑体模式
				// 	eSleepTask_Sta = Sleep_false;	//设置当前任务状态为初始状态
				// 	eMain_Task = Task_ReadyMode;
				// 	eReadyTask_Sta = Ready_Init;	//进入ready模式
				// }
			}
			else
			{
				eSleepTask_Sta = Sleep_true;
				uStaFlag.g_StatusFlag &= 0x22;
			}
			break;

		//完全关机状态
		case Sleep_true:
			Drv_UartTX_Disable();	//关UART功能
			I2C_Disable();		//关I2C
			GPIO_PowerDown();	//IO口省电设置

			FCLKMD = 1;			//切到Slow mode
			NOP(2);
			FSTPHX = 1;			//关IHRC
			NOP(2);
			FDA1EN = 0;			//关DAC1
			FDA2EN = 0;			//关DAC2
			FOPA1EN = 0;		//关OP1
			FOPA2EN = 0;		//关OP2
			FAMPEN = 0 ;		//关PGA
			FPCHPEN = 0 ;		//关PGA chopper
			FACHPEN = 0 ;		//关ADC chopper
			FADC1EN = 0 ;		//关ADC1
			FADC2EN = 0;		//关ADC2
			FAVEN = 0 ;			//关AVE电压
			FACMEN = 0 ;		//关ACM电压
			FAVDDREN = 0 ;		//关avddr电压
			FBGCHP = 0;			//关bandgap chooper
			FBGREN = 0 ;		//关bandgap电压
			FLCDBNK = 1 ;		//All of the LCD dots off
			FLCDEN = 0 ;		//关LCD
			FLCDMOD0 = 1 ;
			FLCDMOD1 = 1 ;		//LCD Mode All OFF
			FLBTEN = 0 ;		//关低电压检测
			FTC0ENB = 0;		//关TC0 timer
			FTC1ENB = 0;		//关TC1 timer
			FTC2ENB = 0;		//关TC2 timer
			INTEN0 = 0;			//所有中断除能
			INTEN1 = 0;			//所有中断除能
			FGIE = 0 ;			//关总中断
			FCPUM1 = 1;		//Green mode（不能关T0模块）
			NOP(2);

			//无按键按下
			l_buf = 1;
			while(l_buf)
			{
				 while( Port_On && Port_Mode )
				 {
					//喂狗
		            WDTR = 0x5A;
					if( FT0IRQ )
					{
						FT0IRQ = 0;
						Update_SysTime();	//系统时间更新
						FCPUM1 = 1;		//Green mode（不能关T0模块）
						NOP(2);
					}
				 }
				 Delay50us(4);	//green mode fcpu=32768/4=8k，是normal模式的250分之一，故实际延迟去抖为50ms
				 if( !Port_On || !Port_Mode )
				 {
					l_buf = 0;
				 }
			}

			FSTPHX = 0;		//开IHRC
			NOP(2);
			FCLKMD = 0;		//关Slow模式
			NOP(2);

			GPIO_Init();	//IO口设置
			Lcd_Init();		//Lcd设置
			TC1Init();		//TC1设置（10ms）
			FGIE = 1;

			HalKey_KeyClr();	//清所有按键信息
			HalKey_Set_KeyMode(Func_Long, &sModeKey);	//设置键设为长按（ET05)
			l_buf = 0;
			eSleepTask_Sta = Sleep_wakeup;
			break;

		//按键唤醒等待
		case Sleep_wakeup:
			l_buf ++;
			//去抖80ms，保证扫键程序执行
			if( l_buf > 8 )
			{
				//设置键长按3s进设置态（设置不全显的机型适用，一般独立按键，非开机键）
				if( uKeyPress.bits.VoiceKeyPress )
				{
					if( uKeyHold.bits.VoiceKeyHold )
					{
						//进入设置态和快进为同一按键适用（当然非同一个按键也适用这样的写法）
						if( uStaFlag.bits.LowBat )
						{
							Clr_Disp();
							lcd_bat_en();	//显示低电压符号；
							Auto_TurnOff_Time_Sel();	//关机时间选择
							eSleepTask_Sta = Sleep_false;
							eMain_Task = Task_InitMode;
							eInitTask_Sta = Init_Err;

							#if Func_Reverse_Disp
								LED_Reverse_Disp_En();
							#endif
						}
						else
						{
							HalKey_KeyClr();	//清除所有按键信息
							App_SetMode();
							while(!Port_Mode)
							{
								//喂狗
								WDTR = 0x5A;
							}
							eSleepTask_Sta = Sleep_false;	//还原当前任务状态
							eMain_Task = Task_Setmode;
						}
					}
				}
				else
				{
					eSleepTask_Sta = Sleep_true;	//默认关机
					if( uKeyPress.bits.OKeyPress )
					{
						eSleepTask_Sta = Sleep_End;	//只有开机键运行开机
					}
				}
			}
			break;

		//结束睡眠
		case Sleep_End:
			Port_Ble_En = 0; //蓝牙使能准备
			//F_Disp_Clean = Disable;
			eSleepTask_Sta = Sleep_false;
			eMain_Task = Task_InitMode;
			#if Func_Hall
			if(Port_Hall)	//霍尔检测
			#else
			if(Port_Rod)	//检测杆检测
			#endif
			{
				eTestmode_num = Earmode;
			}
			else
			{
				eTestmode_num = Foreheadmode;	//默认额温模式
			}	
            uSetFlag.bits.Ready_First = 1;	//默认开启已就绪提示
			HalKey_Set_KeyMode(Func_Long, &sOnKey);   //开机&测量键设为短长按
			HalKey_Set_KeyMode(Func_Long, &sMemKey);	//记忆键为短长按
			HalKey_Set_KeyMode(Func_Short_Long, &sModeKey);	//模式键为长按
			HalKey_Set_KeyMode(Func_Long, &sVoiceKey);	//声音键为短长按
			#if Have_Voice_Func
				// AM5BA_SPI_Init();
				// AM5BA_Power_Enable();
				// PlayStatueParam(2 , 0 , 0);
                // PlayStatueParam(1 , Play_Stop,0);
			#endif
			break;
		default:
			break;
	}
}
