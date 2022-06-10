//****************************************************************************
//;***************************************************************************
//; Raydium Semiconductor Corporation.
//; Design Division V.,
//; Touch System and Firmware Design Depr II.
//; Raydium Application Software.
//;***************************************************************************
//; Project Name : T009
//; File Name      : ic_drv_global.h
//; Built Date      : 2012/02/22
//;
//; Copyright 2006 Raydium Semiconductor Corporation.
//; 2F, No.23, Li-Hsin Rd.,
//; Hsinchu Science Park, Hsinchu 300,
//; Taiwan, R.O.C.
//; Email : @rad-ic.com
//; Website : www.rad-ic.com
//; All rights are reserved. Reproduction in whole or part is prohibited
//; without the prior written consent of the copyright owner.
//;
//; COMPANY CONFIDENTIAL
//;***************************************************************************
//
// Revision History:
// Rev     Date        Author
//____________________________________________________________________________
//
// Build.
//
// Includes Sensor IC Scan and Get Raw Data Related Function Header Files
//;---------------------------------------------------------------------------
//****************************************************************************

#ifndef _DRVGLOBAL_H_
#define _DRVGLOBAL_H_
#include <Config.h>
#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif
#include "f302_ic_control.h"

#if !SELFTEST
#include "f301_ic_control.h"
#include "usb.h"
#endif
//********* Basic Parameter Definition ***********//
/*****************************************************************************
**
**                  Declared Global Variable
**
*****************************************************************************/


#define MAX_IMAGE_BUFFER_SIZE       36
#define NA_P				   	    36
#define DATA_BUFFER_SIZE            (0x100)

#define DONGLE_FLASH_INI_ADDR       0xF000
#define INI_THRESHOLD_ADDR          DONGLE_FLASH_INI_ADDR + 16
#define INI_PARA_ADDR               INI_THRESHOLD_ADDR + 36
#define INI_RAW_DATA2_BL_ADDR		INI_PARA_ADDR+48
#define INI_RAW_DATA_3_CC_ADDR      INI_RAW_DATA2_BL_ADDR+72
#define INI_UC_CC_ADDR              INI_RAW_DATA_3_CC_ADDR + 72// INI_RAW_DATA2_ADDR

#define PRINT_DEBUG_MSG_TYPE_1 		0x01
#define PRINT_DEBUG_MSG_TYPE_2 		0x02
#define PRINT_DEBUG_MSG_TYPE_3 		0x04
#define PRINT_DEBUG_MSG_TYPE_4 		0x08
#define PRINT_DEBUG_MSG_TYPE_5 		0x10
#define PRINT_DEBUG_MSG_TYPE_6 		0x20
#define PRINT_DEBUG_MSG_TYPE_7 		0x40
#define PRINT_DEBUG_MSG_TYPE_8 		0x80

#define FW_SYS_CMD_ADDR         	0x20000288
#define FW_TP_SEQ_NUM_ADDR          0x20000290
#define FW_TP_POINT_DATA_ADDR       0x20000294
#define REG_SYSCON_MISCIER_ADDR		0x40000014
#define FW_FT_IMG_ADDR          	0x2000019C
#define RM_DATAMEM0_BASE			0x20000000
//======================= Basic Hardware Define ==============================


//==================== End of Basic Hardware Define ==========================

#if SELFTEST

#ifdef __KERNEL__
typedef enum {
	error = -1,
	success = 1
} STATUS; 

#else
typedef enum {
	ERROR = 0,
	SUCCESS = 1
} STATUS;
typedef enum {
	DISABLE = 0,
	ENABLE = 1
} FunctionalState;

typedef enum {
	false = 0,
	true = 1
} bool;
#endif
#endif

//========== Basic Parameter Information ==========
//********************* Global *********************//

// Global variable for dongle
extern unsigned char g_u8_drv_interface;

extern unsigned char g_u8_gpio_irq_trigger;
extern unsigned short g_u16_dev_id;


extern unsigned char g_u8_raw_data_buffer[MAX_IMAGE_BUFFER_SIZE * 2];
extern unsigned short g_u16_raw_data_tmp[MAX_IMAGE_BUFFER_SIZE];

extern short g_i16_raw_data_1_short_buf[MAX_IMAGE_BUFFER_SIZE];
extern short g_i16_raw_data_2_open_buf[MAX_IMAGE_BUFFER_SIZE];
extern unsigned short g_u16_raw_data3_cc_buf[MAX_IMAGE_BUFFER_SIZE + 1];
extern unsigned short g_u16_uc_buf[MAX_IMAGE_BUFFER_SIZE + 1];
extern short g_i16_ub_buf[MAX_IMAGE_BUFFER_SIZE + 1];
extern short g_i16_open_bl_data[MAX_IMAGE_BUFFER_SIZE];
extern short g_i16_raw_data2_golden_bl_buf[MAX_IMAGE_BUFFER_SIZE + 1];
extern unsigned short g_u16_raw_data3_golden_cc_buf[MAX_IMAGE_BUFFER_SIZE + 1];
extern unsigned short g_u16_uc_golden_cc_buf[MAX_IMAGE_BUFFER_SIZE];
extern unsigned int g_u32_test_result[MAX_IMAGE_BUFFER_SIZE]; // each node test result (open ng, short ng, uniformity ng..etc)
extern unsigned char g_u8_wearable_pin_map[MAX_IMAGE_BUFFER_SIZE];

extern unsigned short g_u16_test_items_host_cmd;
extern unsigned short g_u16_test_items_tool_cmd;
extern volatile unsigned short g_u16_panel_jig_set_test_items;

extern unsigned int g_u32_wearable_test_result;
extern unsigned char g_u8_channel_x;
extern unsigned char g_u8_channel_y;
extern unsigned char g_u8_is_normal_fw;
extern unsigned char g_u8_data_buf[DATA_BUFFER_SIZE];
extern unsigned char g_u8_mipi_read_buf[56];

extern unsigned int g_u32_fw_cc_version;
extern unsigned char g_u8_print_debug_msg;

extern char g_i8_test_baseline_msg[30];

#if ENABLE_TEST_TIME_MEASURMENT || ENABLE_TEST_TIME_MEASURMENT_CC
extern unsigned int g_u32_spend_time;
#endif


#if !SELFTEST
extern unsigned char g_u8_ic_power_on_ng;
#endif

extern void ic_drv_init(void);

#endif  /* end _DRVGLOBAL_H_*/

/******************************************************************************
**                            End Of File
******************************************************************************/
