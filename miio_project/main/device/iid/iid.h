/**
* Copyright (C) 2013-2015
*
* @author ouyangchengfeng@xiaomi.com
* @date   2018-11-1
*
* @file   IID.h
*
* @remark
*
*/

#ifndef __IID_H__
#define __IID_H__


#define IID_1_DeviceInformation                                               1
#define IID_1_1_Manufacturer                                                  1
#define IID_1_2_Model                                                         2
#define IID_1_3_SerialNumber                                                  3
#define IID_1_4_FirmwareRevision                                              4
#define IID_1_5_SerialNo		                                              5

#define IID_2_Light                                                           2
#define IID_2_1_On                                                            1
#define IID_2_2_Brightness                                                    2
#define IID_2_3_ColorTemperature                                              3
#define IID_2_4_RGBColor                                           			  4
#define IID_2_5_RGBMode                                          			  5

#define IID_2_6_FlexSwitch                                        			  6
#define IID_2_7_SleepMode                                          			  7
#define IID_2_8_WeakupMode                                         			  8
#define IID_2_9_WLduration                                         			  9
#define IID_2_10_WeakpEbr                                          			  10
#define IID_2_11_SleepSbr		                                   			  11
#define IID_2_12_LightPONDefault											  12

#define AID_2_1_GetWeakup													  1
#define AID_2_2_GetSleep													  2
#define AID_2_3_SetWeakup													  3
#define AID_2_4_SetSleep													  4

#define IID_3_Scens                                                           3
#define IID_3_1_OnOff                                                         1
#define IID_3_2_BrAdd                                                         2
#define IID_3_3_BrDec	                                                      3
#define IID_3_4_BrSw                                                    	  4
#define IID_3_5_OnBrSw														  5
#define IID_3_6_TmAdd                                                         6
#define IID_3_7_TmDec	                                                      7
#define IID_3_8_TmSw                                                    	  8
#define IID_3_9_OnTmSw														  9
#define IID_3_10_RGBSw                                                    	  10
#define IID_3_11_OnRGBSw												      11
#define IID_3_12_OnDynamic                                                    12
#define IID_3_13_OnMusic												      13

#define IID_3_Fan                                                             3
#define IID_3_1_On                                                            1
#define IID_3_2_FanLevel                                                      2
#define IID_3_3_FanPositiveNegative											  3
#define IID_3_4_FanNatural													  4

#define IID_4_UserSet                                                         4
#define IID_4_1_Defualt														  1
#define IID_4_2_UserMode													  2
#define IID_4_3_FadeOn													  	  3
#define IID_4_4_FadeOff													  	  4
#define IID_4_5_FadeScen													  5
#define IID_4_6_NightSwitch													  6
#define IID_4_7_TimerOff													  7
#define IID_4_8_LowestBrightness    										  8
#define IID_4_9_LockState		    										  9

#define IID_5_Show                                                            5
#define IID_5_1_Dynamic														  1
#define IID_5_2_Music   													  2

//Ò£¿ØÆ÷
#define IID_8_Remote	                                                      8
#define IID_8_1_RemoteSwitch												  1

#define IID_8_2_RemoteAdd													  2
#define IID_8_3_RemoteDel													  3
#define IID_8_4_LType														  4
#define IID_8_5_HType													  	  5
#define IID_8_6_Remote1Mac													  6
#define IID_8_7_Remote2Mac													  7
#define IID_8_8_Remote3Mac													  8
#define IID_8_9_Remote4Mac													  9
#define IID_8_10_Remote5Mac													  10
#define IID_8_11_Remote6Mac													  11
#define IID_8_12_Remote7Mac													  12
#define IID_8_13_Remote8Mac													  13
#define IID_8_14_RemoteCmd													  14



#endif /* __IID_H__ */
