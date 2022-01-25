/**
 * Copyright (C) 2013-2015
 *
 * @author ouyangchengfeng@xiaomi.com
 * @date   2018-11-14
 *
 * @file   S_2_Light_doChange.h
 *
 * @remark
 *
 */

#ifndef __S_2_Light_doChange_H__
#define __S_2_Light_doChange_H__

#include "miio_define.h"
#include "../typedef/property_operation.h"


void P_2_1_Group_doChange(miio_handle_t handle, int newValue);
void P_2_1_Group_doChange_notify(int newValue);

void P_2_2_Name_doChange(miio_handle_t handle, char* newValue);
void P_2_2_Name_doChange_notify(char* newValue);
void P_2_3_Name_doChange_notify(char* newValue);
void P_2_4_Name_doChange_notify(char* newValue);
void P_2_5_Name_doChange_notify(char* newValue);
void P_2_6_Name_doChange_notify(char* newValue);
void P_2_7_Name_doChange_notify(char* newValue);
void P_2_8_Name_doChange_notify(char* newValue);
void P_2_9_Name_doChange_notify(char* newValue);
void P_2_10_Name_doChange_notify(char* newValue);
void P_2_11_Name_doChange_notify(char* newValue);
void P_2_12_Name_doChange_notify(char* newValue);
void P_2_13_Name_doChange_notify(char* newValue);
void P_2_14_Name_doChange_notify(char* newValue);
void P_2_15_Name_doChange_notify(char* newValue);
void P_2_16_Name_doChange_notify(char* newValue);
void P_2_17_Name_doChange_notify(char* newValue);
void P_2_18_NotifyGroupName_doChange_notify(int newValue);
void P_2_19_NotifyNum_doChange_notify(int newValue);

void P_2_3_ColorTemperature_doChange(miio_handle_t handle, int newValue);
void P_2_3_ColorTemperature_doChange_notify(int newValue);

void P_2_4_RGBColor_doChange(miio_handle_t handle, int newValue);
void P_2_4_RGBColor_doChange_notify(int newValue);

void P_2_5_RGBMode_doChange(miio_handle_t handle, int newValue);
void P_2_5_RGBMode_doChange_notify(int newValue);

void P_2_6_FlexSwitch_doChange(miio_handle_t handle, bool newValue);
void P_2_6_FlexSwitch_doChange_notify(bool newValue);

void P_2_7_SleepMode_doChange(miio_handle_t handle, bool newValue);
void P_2_7_SleepMode_doChange_notify(bool newValue);

void P_2_8_WeakupMode_doChange(miio_handle_t handle, bool newValue);
void P_2_8_WeakupMode_doChange_notify(bool newValue);

void P_2_9_WLduration_doChange(miio_handle_t handle, int newValue);
void P_2_9_WLduration_doChange_notify(int newValue);

void P_2_10_WeakpEbr_doChange(miio_handle_t handle, int newValue);
void P_2_10_WeakpEbr_doChange_notify(int newValue);

void P_2_11_SleepSbr_doChange(miio_handle_t handle, int newValue);
void P_2_11_SleepSbr_doChange_notify(int newValue);

void P_2_12_LightPONDefault_doChange(miio_handle_t handle, int newValue);
void P_2_12_LightPONDefault_doChange_notify(int newValue);

//3
void P_3_1_OnOff_doChange(miio_handle_t handle, bool newValue);
void P_3_1_OnOff_doChange_notify(bool newValue);

void P_3_2_BrAdd_doChange(miio_handle_t handle, int newValue);
void P_3_2_Mode_doChange_notify(int newValue);

void P_3_3_BrDec_doChange(miio_handle_t handle, int newValue);
void P_3_3_Fault_doChange_notify(int newValue);

void P_3_4_BrSw_doChange(miio_handle_t handle, int newValue);
void P_3_4_SetTemp_doChange_notify(int newValue);

void P_3_5_OnBrSw_doChange(miio_handle_t handle, bool newValue);
void P_3_5_OnBrSw_doChange_notify(bool newValue);

void P_3_6_TmAdd_doChange(miio_handle_t handle, bool newValue);
void P_3_6_TmAdd_doChange_notify(bool newValue);

void P_3_7_TmDec_doChange(miio_handle_t handle, bool newValue);
void P_3_7_TmDec_doChange_notify(bool newValue);

void P_3_8_TmSw_doChange(miio_handle_t handle, bool newValue);
void P_3_8_TmSw_doChange_notify(bool newValue);


void P_3_9_OnTmSw_doChange(miio_handle_t handle, bool newValue);
void P_3_9_OnTmSw_doChange_notify(bool newValue);

void P_3_10_RGBSw_doChange(miio_handle_t handle, bool newValue);
void P_3_10_RGBSw_doChange_notify(bool newValue);

void P_3_11_OnRGBSw_doChange(miio_handle_t handle, bool newValue);
void P_3_11_OnRGBSw_doChange_notify(bool newValue);


void P_4_1_Default_doChange(miio_handle_t handle, int newValue);
void P_4_2_FanSpeed_doChange_notify(int newValue);

void P_4_2_UserMode_doChange(miio_handle_t handle, int newValue);
void P_4_2_UserMode_doChange_notify(int newValue);

void P_4_3_FadeOn_doChange(miio_handle_t handle, int newValue);
void P_4_3_FadeOn_doChange_notify(int newValue);

void P_4_4_FadeOff_doChange(miio_handle_t handle, int newValue);
void P_4_4_FadeOff_doChange_notify(int newValue);

void P_4_5_FadeScen_doChange(miio_handle_t handle, int newValue);
void P_4_5_FadeScen_doChange_notify(int newValue);

void P_4_6_NightSwitch_doChange(miio_handle_t handle, int newValue);
void P_4_6_NightSwitch_doChange_notify(int newValue);

void P_4_7_TimerOff_doChange(miio_handle_t handle, int newValue);
void P_4_7_TimerOff_doChange_notify(int newValue);

void P_4_8_LowestBrightness_doChange(miio_handle_t handle, int newValue);
void P_4_8_LowestBrightness_doChange_notify(int newValue);

void P_4_9_LockState_doChange(miio_handle_t handle, int newValue);
void P_4_9_LockState_doChange_notify(int newValue);

//S8
void P_8_1_RemoteSwitch_doChange(miio_handle_t handle, int newValue);
void P_8_1_RemoteSwitch_doChange_notify(int newValue);

void P_8_2_RemoteAdd_doChange(miio_handle_t handle, int newValue);
void P_8_2_RemoteAdd_doChange_notify(int newValue);

void P_8_3_RemoteDel_doChange(miio_handle_t handle, int newValue);
void P_8_3_RemoteDel_doChange_notify(int newValue);

void P_8_4_RemoteLType_doChange(miio_handle_t handle, int newValue);
void P_8_4_RemoteLType_doChange_notify(int newValue);

void P_8_5_RemoteHType_doChange(miio_handle_t handle, int newValue);
void P_8_5_RemoteHType_doChange_notify(int newValue);

void P_8_6_Remote1Mac_doChange(miio_handle_t handle, int newValue);
void P_8_6_Remote1Mac_doChange_notify(int newValue);

void P_8_7_Remote2Mac_doChange(miio_handle_t handle, int newValue);
void P_8_7_Remote2Mac_doChange_notify(int newValue);

void P_8_8_Remote3Mac_doChange(miio_handle_t handle, int newValue);
void P_8_8_Remote3Mac_doChange_notify(int newValue);

void P_8_9_Remote4Mac_doChange(miio_handle_t handle, int newValue);
void P_8_9_Remote4Mac_doChange_notify(int newValue);

void P_8_10_Remote5Mac_doChange(miio_handle_t handle, int newValue);
void P_8_10_Remote5Mac_doChange_notify(int newValue);

void P_8_11_Remote6Mac_doChange(miio_handle_t handle, int newValue);
void P_8_11_Remote6Mac_doChange_notify(int newValue);

void P_8_12_Remote7Mac_doChange(miio_handle_t handle, int newValue);
void P_8_12_Remote7Mac_doChange_notify(int newValue);

void P_8_13_Remote8Mac_doChange(miio_handle_t handle, int newValue);
void P_8_13_Remote8Mac_doChange_notify(int newValue);

//S8 CMD
void P_8_14_RemoteCmd_doChange(miio_handle_t handle, int newValue);
void P_8_14_RemoteCmd_doChange_notify(int newValue);



#endif /* __S_2_Light_doChange_H__ */

