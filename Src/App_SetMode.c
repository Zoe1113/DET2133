/**************************************************************************
文件名称：	App_SetMode.c
说    明：	设置模式（应用层）
作    者:	Liaoy
版    本：	V1.0
芯	  片:	SN8P29XX
备    注:
修订记录：
**************************************************************************/
#include "Include.h"

#define left 0
#define right 1

uint8 g_Blink;	//消隐
uint8 g_Blink_count;	//消隐时间(10ms时间基准)

eSetModeTask eSetTask;

bit F_UpdateValue;		//更新设置值
bit F_UpdateMenu;		//更新设置项
bit F_FirstEnter_SetMode;	//首次进入设置态
uint8 L_MaxDay;
void App_SetMode(void)
{
	
	uint8 L_Buf;

	//第一次进入必须刷新，原RatioModify和unit模式合并到设置态
	//单位切换能否合并关键在于按键的功能是否一样
	if( !F_FirstEnter_SetMode )
	{
		F_DispNtc_Time_Date_En = 0;
		F_FirstEnter_SetMode = 1;
		F_UpdateMenu = 1;	//首次需显示
		g_Blink_count = 0;
		High_Precision_Flag = 0;

		//初始化开机键为短按
 		HalKey_Set_KeyMode(Func_Short, &sSetKey);
		//初始化设置键为加速按
 		HalKey_Set_KeyMode(Func_Short_Continue, &sOnKey);
		Auto_TurnOff_Time_Sel();	//按下关机时间清0

		/*** 请特别注意不同型号此处设定 ***/
		HalKey_KeyClr();	//清除所有按键信息
		// //保证首次设置键长按不快进
		// uKeyRelease.bits.SKeyRelease = 1;
		// //保证首次开机键长按不切换（开机长按进设置态适用）
		// uKeyPress.bits.OKeyPress = 0;
		/*******************************/
        #if Func_Ble
            F_Ble_En = 0;
            Port_Ble_En = 0; //蓝牙不使能准备
        #endif

		#if Func_Reverse_Disp
			LED_Reverse_Disp_En();
		#endif

		//上电复位由于未使能，故无法播报（设计就是无法播报）
		Tone_Init();//语音功能
		
		#if ParamModif 
			if( eMain_Task == Task_ParamModifymode )
			{
				
					#if ET_FT !=1                               //额温，发射率不可调
						eSetTask = Set_Emission;
					#else
						eSetTask = Set_TableNum;                //黑体表格
					#endif
			
			}
		#endif

			if(eMain_Task == Task_Unitmode && uSetFlag.bits.Unit_Change == Unit_Change_En)
			{
				eSetTask = Set_Unit;
                Clr_Disp();
				Disp_Unit();
				LVD_Display();
		
			}

			while (!Port_Set)
			{
				//喂狗
				WDTR = 0x5A;
			}	

		//绑定检测判断，放这里的作用(只允许绑定厂在设置态进入，逼员工必须开机全显后进入)
		// Cal_Inspect_Detect();
	}

	//开机键按下并抬起后才触发切换设置项并更新显示(ET05/ETH)
	if( uKeyPress.bits.SetKeyPress && eMain_Task == Task_Unitmode)
	{
		uKeyPress.bits.SetKeyPress = 0;
		Auto_TurnOff_Time_Sel();	//按下关机时间清0
		F_UpdateMenu = 1;
		eSetTask ++;
		if (eSetTask > Set_Unit)
		{
			eSetTask = Set_End;
		}
	
	}

	if( uKeyPress.bits.SetKeyPress && eMain_Task == Task_Setmode)
	{
		uKeyPress.bits.SetKeyPress = 0;
		Auto_TurnOff_Time_Sel();	//按下关机时间清0
		F_UpdateMenu = 1;
		eSetTask ++;
		//注意不同模式结束任务号不同
		
		#if Have_Voice_Func
			#if Have_VoiceLang_Change
				if (eSetTask > Set_VoiceLang)
				{
					eSetTask = Set_End;
				}
			#else
				if (eSetTask > Set_Day)
				{
					eSetTask = Set_End;
				}
			#endif
		#else

				if (eSetTask > Set_Day)
				{
					eSetTask = Set_End;
				}


		#endif
	}

#if ParamModif
	if( uKeyPress.bits.SetKeyPress && eMain_Task == Task_ParamModifymode)
	{
		uKeyPress.bits.SetKeyPress = 0;
		Auto_TurnOff_Time_Sel();	//按下关机时间清0
		F_UpdateMenu = 1;
		eSetTask ++;
		//注意不同模式结束任务号不同
		if (eSetTask > Set_TableNum)
		{
			eSetTask = Set_End;
		}
	}
#endif

	if( uKeyPress.bits.OKeyPress )
	{
		uKeyRelease.bits.OKeyRelease = 0;
		Auto_TurnOff_Time_Sel();	//按下关机时间清0
	}

	if( uKeyPress.bits.SetKeyPress )
	{
		uKeyRelease.bits.SetKeyRelease = 0;
		//Auto_TurnOff_Time_Sel();	//按下关机时间清0
	}

	//设置键按下则立即触发累加并更新显示（ET215）
	if( !uKeyRelease.bits.OKeyRelease )
	{
		if( uKeyPress.bits.OKeyPress )
		{
			uKeyPress.bits.OKeyPress = 0;
			F_UpdateValue = 1;
		}

		if( uKeyHold.bits.OKeyHold )
		{
			F_UpdateValue = 1;
			uKeyHold.bits.OKeyHold = 0;
		}
	}

	//当前设置项处理(短按递增/长按快进递增)
	if( F_UpdateValue )
	{
		F_UpdateValue = 0;
		F_UpdateMenu = 1;		//修改参数后必须立刻更新显示
		switch( eSetTask )
		{
			#if No_Time
			//时制
			case Set_TimeFormate:
				uSetFlag.bits.TimeFormat = ~uSetFlag.bits.TimeFormat;
				break;

  			//时
			case Set_Hour:
				g_Hour ++;
				if( g_Hour > 23 )
					g_Hour = 0;
				break;

  			//分
			case Set_Minute:
				g_Minute ++;
				if( g_Minute > 59 )
					g_Minute = 0;
				break;

  			//年
			case Set_Year:
				g_Year ++;
				if( g_Year > 2099 )
					g_Year = Default_Year;
				//判定日是否要更新
				Data_Handle();
				break;

  			//月
			case Set_Month:
				g_Month ++;
				if( g_Month > 12 )
					g_Month = 1;
				Data_Handle();
				break;

   			//日
			case Set_Day:
				g_Day ++;
				Data_Handle();
				break;
			#endif
            
            //单位
			case Set_Unit:
				uSetFlag.bits.Unit = ~uSetFlag.bits.Unit;
				break;
			
  			//语种
			#if Have_Voice_Func
				#if Have_VoiceLang_Change
				case Set_VoiceLang:
					if( g_LLCode == lang_En)
						g_LLCode = lang_Ch;
					else
						g_LLCode = lang_En;
					break;
				#endif
			#endif

  			
	#if ParamModif
  			//发射率
			case Set_Emission:
				g_Emission ++;
				if( g_Emission > 100 )
					g_Emission = 96;
				break;

			//人体系数
		  	case Set_HumanRatio1:
			  g_HumanRatio1 ++;
			  if( g_HumanRatio1 > 60 )
				  g_HumanRatio1 = 0;
			  break;
		  	//人体系数
		  	case Set_HumanRatio2:
			  g_HumanRatio2 ++;
			  if( g_HumanRatio2 > 60 )
				  g_HumanRatio2 = 0;
			  break;

  			//耳套系数
			#if Func_Probecover
			case Set_Earcap10:
				g_Earcap10 ++;
				if( g_Earcap10 > 200 )
					g_Earcap10 = 0;
				break;

			//耳套系数
			case Set_Earcap15:
				g_Earcap15 ++;
				if( g_Earcap15 > 200 )
					g_Earcap15 = 0;
				break;

			//耳套系数
			case Set_Earcap20:
				g_Earcap20 ++;
				if( g_Earcap20 > 200 )
					g_Earcap20 = 0;
				break;

			//耳套系数
			case Set_Earcap25:
				g_Earcap25 ++;
				if( g_Earcap25 > 200 )
					g_Earcap25 = 0;
				break;

			//耳套系数
			case Set_Earcap30:
				g_Earcap30 ++;
				if( g_Earcap30 > 200 )
					g_Earcap30 = 0;
				break;

			//耳套系数
			case Set_Earcap35:
				g_Earcap35 ++;
				if( g_Earcap35 > 200 )
					g_Earcap35 = 0;
				break;

			//耳套系数
			case Set_Earcap40:
				g_Earcap40 ++;
				if( g_Earcap40 > 200 )
					g_Earcap40 = 0;
				break;
			#endif
			#if OffsetTable2open
 			//表格选项
			case Set_TableNum:
				uStaFlag.bits.TableNum = ~uStaFlag.bits.TableNum;
				break;
			#endif
	#endif
  			//关机
			default:
				break;
		}
	}

	//设置项切换处理(更新显示)
	if( F_UpdateMenu )
	{
		F_UpdateMenu = 0;
		switch( eSetTask )
		{
			#if No_Time
			//时制
			case Set_TimeFormate:
				if( uSetFlag.bits.TimeFormat )
					Disp_12H();
				else
					Disp_24H();
				break;

  			//时
			case Set_Hour:
				Disp_Time(1,uSetFlag.bits.TimeFormat,g_Hour,g_Minute);
				g_Blink = left;		//左消隐
				break;

  			//分
			case Set_Minute:
				Disp_Time(1,uSetFlag.bits.TimeFormat,g_Hour,g_Minute);
				g_Blink = right;	//右消隐
				break;

  			//年
			case Set_Year:
				Disp_Year(g_Year);
				g_Blink = right;	//右消隐
				break;

  			//月
			case Set_Month:
				Disp_Time(0,uSetFlag.bits.TimeFormat,g_Month,g_Day);
				g_Blink = left;		//左消隐
				break;

   			//日
			case Set_Day:
				Disp_Time(0,uSetFlag.bits.TimeFormat,g_Month,g_Day);
				g_Blink = right;	//右消隐
				break;
            #endif
            //单位
			case Set_Unit:
                Clr_Disp();
				Disp_Unit();
				LVD_Display();
				break;

  			//语种
			#if Have_Voice_Func
				#if Have_VoiceLang_Change
				case Set_VoiceLang:
					if( g_LLCode == lang_En)
						Disp_En();
					else
						Disp_Ch();
					lcd_voice_en();
					break;
				#endif
			#endif
	#if ParamModif
  			//发射率
			case Set_Emission:
				Clr_Disp();
				Disp_Temp(0,0,0, (uint16)g_Emission*10);
				break;

  			//人体系数
			case Set_HumanRatio1:   //无耳套
				lcd_ear_en();
				Disp_Temp(0,0,0, (uint16)g_HumanRatio1*10);
			  break;
		  	//人体系数
		  	case Set_HumanRatio2:   //有耳套
				lcd_pc_en();
				lcd_ear_en();
				Disp_Temp(0,0,0, (uint16)g_HumanRatio2*10);
			  break;

			//耳套系数
			#if Func_Probecover
			case Set_Earcap10:
                Disp_Temp(0,0,0,(uint16)100);
				Delay10ms(50);
                lcd_ear_clr();
                lcd_pc_en();;
				Disp_Temp(0,0,0,(uint16)g_Earcap10*10);
				break;
            //耳套系数
			case Set_Earcap15:
                Disp_Temp(0,0,0,(uint16)150);
				Delay10ms(50);
                lcd_pc_en();
				Disp_Temp(0,0,0,(uint16)g_Earcap15*10);
				break;

  			//耳套系数
			case Set_Earcap20:
                Disp_Temp(0,0,0,(uint16)200);
				Delay10ms(50);
                lcd_pc_en();
				Disp_Temp(0,0,0,(uint16)g_Earcap20*10);
				break;

  			//耳套系数
			case Set_Earcap25:
                Disp_Temp(0,0,0,(uint16)250);
				Delay10ms(50);
                lcd_pc_en();
				Disp_Temp(0,0,0,(uint16)g_Earcap25*10);
				break;

  			//耳套系数
			case Set_Earcap30:
			    Disp_Temp(0,0,0,(uint16)300);
				Delay10ms(50);
                lcd_pc_en();
				Disp_Temp(0,0,0,(uint16)g_Earcap30*10);
				break;

  			//耳套系数
			case Set_Earcap35:
                Disp_Temp(0,0,0,(uint16)350);
				Delay10ms(50);
                lcd_pc_en();
				Disp_Temp(0,0,0,(uint16)g_Earcap35*10);
				break;

  			//耳套系数
			case Set_Earcap40:
			    Disp_Temp(0,0,0,(uint16)400);
				Delay10ms(50);
                lcd_pc_en();
				Disp_Temp(0,0,0,(uint16)g_Earcap40*10);
				break;
			#endif

			#if OffsetTable2open
 			//表格选项
			case Set_TableNum:
				if( uStaFlag.bits.TableNum )
					Disp_Table2();
				else
					Disp_Table1();
				break;
			#endif
	#endif
  			//关机保存参数，并还原当前任务变量
			default:
			#if ParamModif
				
				
				I2C_masterInit();
				if(eMain_Task == Task_ParamModifymode)
				{
					#if OffsetTable2open
						L_Buf = (uStaFlag.g_StatusFlag >> 2) & 0x01;
						I2C_Byte_W(I2C_Add_Table, L_Buf);				//保存黑体修正表格
						Delay1ms(5);
					#endif



					I2C_Byte_W(I2C_Add_Emission, g_Emission);		//保存人体发射率
					Delay1ms(5);

					I2C_Byte_W(I2C_Add_HumanRatio1, g_HumanRatio1);	//保存人体系数
					Delay1ms(5);

					I2C_Byte_W(I2C_Add_HumanRatio2, g_HumanRatio2);	//保存人体系数
					Delay1ms(5);

					I2C_Byte_W(I2C_Add_Earcap10, g_Earcap10);		//g_Earcap10=100
                    Delay1ms(5);

                    I2C_Byte_W(I2C_Add_Earcap15, g_Earcap15);		//g_Earcap15=100
                    Delay1ms(5);

                    I2C_Byte_W(I2C_Add_Earcap20, g_Earcap20);		//g_Earcap15=100
                    Delay1ms(5);

                    I2C_Byte_W(I2C_Add_Earcap25, g_Earcap25);		//g_Earcap15=100
                    Delay1ms(5);

                    I2C_Byte_W(I2C_Add_Earcap30, g_Earcap30);		//g_Earcap15=100
                    Delay1ms(5);

                    I2C_Byte_W(I2C_Add_Earcap35, g_Earcap35);		//g_Earcap15=100
                    Delay1ms(5);

                    I2C_Byte_W(I2C_Add_Earcap40, g_Earcap40);		//g_Earcap15=100
                    Delay1ms(5);
				}
				I2C_Disable();
			#endif
				//语种
				#if Have_Voice_Func & Have_VoiceLang_Change
					I2C_masterInit();
					if( g_LLCode == lang_En)
						I2C_Byte_W(I2C_Add_Voice, lang_En);		//保存当前语音语言
					else
						I2C_Byte_W(I2C_Add_Voice, lang_Ch);		//保存当前语音语言
					Delay1ms(5);
					AM5BA_Power_Disable();
					I2C_Disable();
				#endif


				
			

				//还原变量，保证下次进入首先初始化起始设置项
				F_FirstEnter_SetMode = 0;

				//进入关机
				eMain_Task = Task_Sleepmode;
				OFF_Flag = 1;
				break;
		}
	}
#if No_Time
	//非加速按时启动消隐
	if( !uKeyContinue.bits.OKeyContinue )
		Time_Blink();
#endif
}

/**************************************************************************
函数名称：	void Time_Blink(void)
函数功能：	消隐、显示当前设置项
输入参数：	g_Blink
输出参数：	lcd
返回值  ：	无
占用空间：	TBD
备    注：	20ms作为时基
**************************************************************************/
		#if No_Time
void Time_Blink(void)
{
	g_Blink_count ++;

	//消隐
	if( g_Blink_count == 1 )
	{
		//年月日时分使能消隐
		switch( eSetTask )
		{
			case Set_Hour:
			case Set_Minute:
			case Set_Year:
			case Set_Month:
			case Set_Day:
				Clr_SetTime(g_Blink);	//消隐当前设置项
				break;
			default:
				break;
		}
	}
	//显示
	if( g_Blink_count == 51 )
	{
		switch( eSetTask )
		{
			#if No_Time
  			//时
			case Set_Hour:
  			//分
			case Set_Minute:
				Disp_Time(1,uSetFlag.bits.TimeFormat,g_Hour,g_Minute);
				break;

  			//年
			case Set_Year:
				Disp_Year(g_Year);
				break;

  			//月
			case Set_Month:
   			//日
			case Set_Day:
				Disp_Time(0,uSetFlag.bits.TimeFormat,g_Month,g_Day);
				break;
			#endif

			default:
				break;
		}
	}
	if(	g_Blink_count == 102 )
		g_Blink_count = 0;
}
#endif

/**
 * 判断日期是否超出月份的最大值
*/
#if No_Time
void Data_Handle(void)
{
	L_MaxDay = MonthTable[g_Month-1];
	if( g_Month == 2)
	{
		if( LeapYear_Judge(g_Year) )
			L_MaxDay ++;	//闰年2月份加1天
	}
	if( g_Day > L_MaxDay )
		g_Day = 1;
}
#endif
