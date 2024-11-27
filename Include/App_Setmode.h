#ifndef _App_SetMode_H
#define _App_SetMode_H

#if Have_Voice_Func

//有语音设置态任务
#if Have_VoiceLang_Change

//有语种切换的设置态任务
typedef enum
{
	Set_Unit = 0,		//设置单位
	Set_TimeFormate ,//设置时制
	Set_Hour ,			//设置小时
	Set_Minute ,		//设置分钟
	Set_Year ,			//设置年份
	Set_Month ,			//设置月份
	Set_Day ,			//设置天
	Set_VoiceLang ,		//设置语种
	//Set_VoiceOnOff ,	//设置语音开关
	
	Set_Emission ,		//设置发射率
	Set_HumanRatio ,	//设置人体系数
	#if Func_Probecover
		Set_PcRatio ,		//设置耳套系数
	#endif
	Set_TableNum ,		//设置黑体表格
	Set_End 			//设置退出保存
}eSetModeTask;

#else

//无语种切换的设置态任务
typedef enum
{
	Set_Unit = 0,		//设置单位
	Set_TimeFormate ,//设置时制
	Set_Hour ,			//设置小时
	Set_Minute ,		//设置分钟
	Set_Year ,			//设置年份
	Set_Month ,			//设置月份
	Set_Day ,			//设置天
	// Set_VoiceLang ,		//设置语种
	//Set_VoiceOnOff ,	//设置语音开关
	
	Set_Emission ,		//设置发射率
	Set_HumanRatio ,	//设置人体系数
	#if Func_Probecover
	Set_PcRatio ,		//设置耳套系数
	#endif
	Set_TableNum ,		//设置黑体表格
	Set_End 			//设置退出保存
}eSetModeTask;

#endif

#else

//无语音设置态任务
typedef enum
{
	Set_Unit = 0,		//设置单位
	Set_TimeFormate ,	//设置时制
	Set_Hour ,			//设置小时
	Set_Minute ,		//设置分钟
	Set_Year ,			//设置年份
	Set_Month ,			//设置月份
	Set_Day ,			//设置天
	// Set_VoiceLang ,		//设置语种
	// Set_VoiceOnOff ,	//设置语音开关
	Set_Emission ,		//设置发射率
	Set_HumanRatio ,	//设置人体系数
	#if Func_Probecover
	Set_PcRatio ,		//设置耳套系数
	#endif
	Set_TableNum ,		//设置黑体表格
	Set_End 			//设置退出保存
}eSetModeTask;

#endif

extern eSetModeTask eSetTask;
extern bit F_FirstEnter_SetMode;

void App_SetMode(void);
void Time_Blink(void);
void Data_Handle(void);

#endif
