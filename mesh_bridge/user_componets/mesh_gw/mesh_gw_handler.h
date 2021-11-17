#ifndef _MESH_GW_HANDLER_H
#define _MESH_GW_HANDLER_H

#include <hap.h>


/*MESH_GW_TASK_MASK 与mesh gw模块相关的任务*/
#if 0
#define MESH_GW_ENABLE_DEVICE_JION_TASK		0X0001	/*<允许子设备入网>*/
#define MESH_GW_DISABLE_DEVICE_JION_TASK	0X0002	/*<关闭子设备入网>*/
#define MESH_GW_ALLOW_DEVICE_JION_TASK		0X0004	/*<是否允许子设备入网>*/
#define MESH_GW_DETECT_DEVICE_ONLINE_TASK	0X0008	/*<子设备心跳检测>*/
#define MESH_GW_QUERY_DEVICE_STATUS_TASK	0X0010	/*<子设备状态查询>*/
#define MESH_GW_CONTROL_DEVICE_TASK			0X0020	/*<子设备控制>*/
#define MESH_GW_UPDATE_DEVICE_STATUS_TASK	0X0040	/*<子设备状态上报>*/
#endif

#define TASK_SUB_DEVICE_ADD_MASK			0X01
#define	TASK_SUB_DEVICE_DELETE_MASK			0X02
#define	TASK_SUB_DEVICE_ONLINE_MASK			0X04
#define TASK_SUB_DEVICE_OFFLINE_MASK		0X08
#define TASK_SUB_DEVICE_UPDATE_MASK			0X10
#define TASK_SUB_DEVICE_QUERY_MASK			0X20


//灯设备状态更新上报任务
#define LIGHTBULB_NOTIFY_SW_MASK			0X01
#define	LIGHTBULB_NOTIFY_BRIGHTNESS_MASK	0X02
#define	LIGHTBULB_NOTIFY_COLOR_TEMP_MASK	0X04
#define LIGHTBULB_NOTIFY_RGB_MASK			0X08
#define LIGHTBULB_NOTIFY_WY_MODE_MASK		0X10

/*<子设备ID中高两位字节表示的类型>*/
#define SUB_TYPE_SINGLE_LIGHTBULB	((uint16_t)0x3035)/*<单色灯>*/
#define SUB_TYPE_DOUBLE_LIGHTBULB	((uint16_t)0x3031)/*<双色灯>*/
#define SUB_TYPE_RGB_LIGHTBULB		((uint16_t)0x3033)/*<RGB灯>*/
#define SUB_TYPE_WYRGB_LIGHTBULB	((uint16_t)0x3038)/*<WYRGB灯>*/
#define SUB_TYPE_SW_CONTROLLER		((uint16_t)0x6231)/*<通断器>*/
#define SUB_TYPE_SWITCH_KEY			((uint16_t)0x6232)/*<86开关>*/
#define SUB_TYPE_SMART_OUTLET		((uint16_t)0x6234)/*<智能插座>*/
#define SUB_TYPE_CURTAIN			((uint16_t)0x6230)/*<窗帘>*/
//#define SUB_TYPE_GAUZE				((uint16_t)0x6231)/*<窗纱>*/


/*数据点序号 datapoint      dpid*/
/*-----------灯------------*/
#define DPID_LIGHT_SW				((uint8_t)20)
#define DPID_LIGHT_BRIGHTNESS		((uint8_t)22)
#define DPID_LIGHT_COLOR_TEMP		((uint8_t)23)
#define DPID_LIGHT_HSV				((uint8_t)24)
#define DPID_LIGHT_WYRGB_MODE		((uint8_t)21)
#define DPID_LIGHT_AUXILIARY1_SW	((uint8_t)102)
#define DPID_LIGHT_AUXILIARY2_SW	((uint8_t)103)

/*-----------开关------------*/
#define DPID_SWITCH_KEY1_SW			((uint8_t)1)
#define DPID_SWITCH_KEY2_SW			((uint8_t)2)
#define DPID_SWITCH_KEY3_SW			((uint8_t)3)
#define DPID_SWITCH_KEY4_SW			((uint8_t)4)

/*-----------插座------------*/
#define DPID_SOCKET1_SW 			((uint8_t)1) //插座开关-布尔变量 
#define DPID_SOCKET2_SW 			((uint8_t)2) //插座开关-布尔变量
#define DPID_SOCKET3_SW 			((uint8_t)3) //插座开关-布尔变量 
#define DPID_SOCKET4_SW 			((uint8_t)4) //插座开关-布尔变量

/*-----------窗帘------------*/
#define DPID_CURTAIN_SW				((uint8_t)1)
#define DPID_CURTAIN_CURRENT_STATE	((uint8_t)2)
#define DPID_CURTAIN_TARGET_STATE	((uint8_t)3)

/*-----------照度------------*/
#define DPID_BRINGHT_VALUE 			((uint8_t)2) //环境照度上报-数值-【0-20000】

/*-----------门磁------------*/
#define DPID_DOORSENSOR 			((uint8_t)1) //门磁 
#define DPID_DOORBATTERY 			((uint8_t)2) //电量

/*-----------红外------------*/
#define DPID_IR_STATE 				((uint8_t)1) //人来=1,人走=2 
#define DPID_IR_BATTERY1 			((uint8_t)4)//电量

/*-----------水浸------------*/
#define DPID_WATER_STATE 			((uint8_t)1) //1 水浸报警 2 水浸报警解除 
#define DPID_WATER_BATTERY 			((uint8_t)4) //电量

/*-----------烟雾------------*/
#define DPID_SMOKE_STATE 			((uint8_t)1) //1 烟雾报警 2 烟雾报警解除 
#define DPID_SMOKE_BATTERY 			((uint8_t)15) //电量 
#define DPID_CLEAR_WARNING 			((uint8_t)101) //清除报警

/*-----------煤气------------*/
#define DPID_GAS_STATE 				((uint8_t)1) //1 煤气报警 2 煤气报警解除
#define DPID_GAS_BATTERY 			((uint8_t)15) //电量-百分比 
#define DPID_GAS_CLEAR_WARNING 		((uint8_t)101) //清除报警-布尔变量

/*-----------灯------------*/
#define DPTYPE_RAW				((uint8_t)0)
#define DPTYPE_BOOL				((uint8_t)1)
#define DPTYPE_VALUE			((uint8_t)2)
#define DPTYPE_STRING			((uint8_t)3)
#define DPTYPE_ENUM				((uint8_t)4)
#define DPTYPE_BITMAP			((uint8_t)5)

#pragma pack(1)
/*<子设备类型>*/
typedef enum{
	MESH_SUB_TYPE_SINGLE_LIGHTBULB_E = 1,			/*<单色灯>*/
	MESH_SUB_TYPE_DOUBLE_LIGHTBULB_E,				/*<双色灯>*/
	MESH_SUB_TYPE_RGB_LIGHTBULB_E,					/*<RGB灯>*/
	MESH_SUB_TYPE_WYRGB_LIGHTBULB_E,				/*<WYRGB灯>*/
	MESH_SUB_TYPE_DOUBLE_PLUS_LIGHTBULB_E,			/*<双色灯+辅灯>*/
	MESH_SUB_TYPE_SINGLE_PLUS_LIGHTBULB_E,			/*<单色灯+辅灯>*/
	MESH_SUB_TYPE_WRGB_LIGHTBULB_E,					/*<WRGB灯>*/
	MESH_SUB_TYPE_SW_CONTROLLER_E, 					/*<开关控制器/通断器>*/
	MESH_SUB_TYPE_CLASSROOM_SINGLE_LIGHTBULB_E,		/*<教室单色灯>*/
	MESH_SUB_TYPE_ILLUMINANCE_SENSOR_E, 			/*<照度传感器>*/
	MESH_SUB_TYPE_ELECTRIC_CURTAINS_E, 				/*<电动窗帘>*/
	MESH_SUB_TYPE_SWITCH_1KEY_E,					/*<1开开关>*/
	MESH_SUB_TYPE_SWITCH_2KEY_E,					/*<2开开关>*/
	MESH_SUB_TYPE_SWITCH_3KEY_E,					/*<3开开关>*/
	MESH_SUB_TYPE_SWITCH_4KEY_E,					/*<4开开关>*/
	MESH_SUB_TYPE_AUDIBLE_VISUAL_ALARM_E,			/*<声光报警器>*/
	MESH_SUB_TYPE_ELECTRIC_VALVE_E,					/*<电动阀门>*/
	MESH_SUB_TYPE_SMART_OUTLET_E, 					/*<智能插座>*/
	MESH_SUB_TYPE_MAGNETIC_DOOR_SENSOR_E,			/*<门磁传感器>*/
	MESH_SUB_TYPE_IR_SENSOR_E,						/*<红外传感器&门锁?>*/
	MESH_SUB_TYPE_UNDER_WATER_SENSOR_E,				/*<水浸传感器>*/
	MESH_SUB_TYPE_SMOKE_SENSOR_E,					/*<烟雾传感器>*/
	MESH_SUB_TYPE_GAS_SENSOR_E,						/*<燃气传感器>*/
	MESH_SUB_TYPE_SCENE_1KEY_E,						/*<1键情景面板>*/
	MESH_SUB_TYPE_SCENE_2KEY_E, 					/*<2键情景面板>*/
	MESH_SUB_TYPE_SCENE_3KEY_E,						/*<3键情景面板>*/
	MESH_SUB_TYPE_SCENE_4KEY_E, 					/*<4键情景面板>*/
	MESH_SUB_TYPE_SCENE_5KEY_E,						/*<5键情景面板>*/
	MESH_SUB_TYPE_SCENE_6KEY_E,						/*<6键情景面板>*/
}MESH_SUB_DEVICE_TYPE_e;

typedef struct{
	uint16_t task_pending_flag;//任务挂起标志位@refer MESH_GW_TASK_MASK	

}mesh_gw_task_t;

/*datapoint 结构体*/
typedef struct{
	uint8_t d_pid;			/*<数据点序号>*/
	uint8_t d_type;			/*<数据点类型 0-raw 1-bool 2-value/int 3-string 4-enum 5-bitmap>*/
	uint8_t d_len[2];		/*<数据长度>*/
	uint8_t* d_value;		/*<数据内容>*/
}datapoint_info_t;

/*lightbulb 设备特征值*/
typedef struct{
	hap_char_t* hap_char_on_handle;					/*<characteristic 'on'的句柄>*/
	hap_char_t* hap_char_brightness_handle;			/*<characteristic 'brightness'的句柄>*/
	hap_char_t* hap_char_aturation_handle;			/*<characteristic 'aturation'的句柄>*/
	hap_char_t* hap_char_hue_handle;				/*<characteristic 'hue'的句柄>*/
	hap_char_t* hap_char_color_temp_handle;			/*<characteristic 'color temperature'的句柄>*/
	hap_char_t* hap_char_auxiliary1_on_handle;		/*<characteristic 'on'的句柄,对应辅灯1的开关状态>*/
	hap_char_t* hap_char_auxiliary2_on_handle;		/*<characteristic 'on'的句柄,对应辅灯2的开关状态>*/
	bool lamp_on_off;								/*<开关状态 true-ON false-OFF>*/
	uint8_t lamp_brightness;						/*<亮度值 0-100>*/
	uint8_t lamp_saturation;						/*<饱和度 0-100>*/
	uint16_t lamp_hue;								/*<色调 0-360>*/ 
	uint16_t lamp_color_temp;						/*<色温 50-400>*/
	uint8_t auxiliary_lamp1_sw;						/*<辅灯1开关状态 true-ON false-OFF>*/
	uint8_t auxiliary_lamp2_sw;						/*<辅灯2开关状态 true-ON false-OFF>*/
}mesh_lightbulb_t;

/*switch 设备特征值*/
typedef struct{
	hap_char_t* hap_char_on_handle1;					/*<characteristic 'on'的句柄,对应sw1>*/
	hap_char_t* hap_char_on_handle2;					/*<characteristic 'on'的句柄,对应sw2>*/
	hap_char_t* hap_char_on_handle3;					/*<characteristic 'on'的句柄,对应sw3>*/
	hap_char_t* hap_char_on_handle4;					/*<characteristic 'on'的句柄,对应sw4>*/
	bool sw1_on_ff;									/*<第1路开关状态>*/
	bool sw2_on_ff;									/*<第2路开关状态>*/
	bool sw3_on_ff;									/*<第3路开关状态>*/
	bool sw4_on_ff;									/*<第4路开关状态>*/
}mesh_switch_t;

/*二进制型传感器 设备特征值*/
typedef struct{
	hap_char_t* hap_char_sensor_detect_handle;		/*<characteristic ' Carbon Dioxide Detected'/' Carbon Monoxide Detected'
													' Contact Sensor State'/'Smoke Detected'的句柄>*/
	uint8_t sensor_detect_sta;
}mesh_binary_sensor_t;

/*其它数据型传感器 设备特征值*/
typedef struct{
	hap_char_t* hap_char_sensor_data_handle;			/*<characteristic ' Current Ambient Light Level'的句柄*/
	float sensor_detect_data;						/*<' 传感器检测到的数据'>*/
	uint32_t additiona_data;
}mesh_data_sensor_t;

/*窗帘设备 特征值*/
typedef struct{
	hap_char_t* hap_char_target_position_handle;		/*<characteristic 'target position'的句柄>*/
	hap_char_t* hap_char_current_position_handle;	/*<characteristic 'current position'的句柄>*/
	hap_char_t* hap_char_position_state_handle;		/*<characteristic 'position state'的句柄>*/
	uint8_t target_position;/*<目标位置 0-100,uint:百分比 0-关 100-完全开>*/
	uint8_t current_position;/*<当前位置 0-100,uint:百分比 0-关 100-完全开>*/
	uint8_t position_state;/*<位置状态 0-正在向最小值移动 1-正在向最大值移动 2-停止>*/
}mesh_curtain_t;

/*子设备的特征值集合*/
typedef union
{
	mesh_lightbulb_t sub_lightbulb;
	mesh_switch_t sub_switch;
	mesh_binary_sensor_t sub_binary_sensor;
	mesh_data_sensor_t sub_data_sensor;
	mesh_curtain_t sub_curtain;
}sub_device_t;

/*mesh网关下子设备信息*/
typedef struct{
	MESH_SUB_DEVICE_TYPE_e sub_type;/*<子设备类型>*/
	uint8_t sub_register_id;/*<子设备注册id,用于HAP命名编号>*/
	uint8_t sub_id[10];/*<子设备id>*/
	sub_device_t sub_device;/*<子设备特征值>*/
	uint8_t task_pending_f;
	uint8_t is_notify_f;
	uint8_t is_control_f;
}mesh_device_t;
#pragma pack()

void mesh_gw_init(void);
void mesh_gw_start(void);
//void mesh_gw_task_handle(void);
void add_sub_device(uint8_t sub_type,uint8_t sub_id);
void delete_sub_device(uint8_t sub_type, uint8_t sub_id);
void update_sub_device_status(uint8_t* sub_device_info,uint8_t info_len);

#endif
