//****************************************************************************
//;***************************************************************************
//; Raydium Semiconductor Corporation.
//; Design Division V.,
//; Touch System and Firmware Design Depr II.
//; Raydium Application Software.
//;***************************************************************************
//; Project Name : SDK3.0
//; File Name      : ic_drv_global.c
//; Built Date      :
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
// 0.1
// Build.
//
// Sensor IC Scan and Get Raw Data Related Functions
//;---------------------------------------------------------------------------
//****************************************************************************

#include <Config.h>
#include "ic_drv_global.h"
#include "ic_drv_interface.h"
#ifdef __KERNEL__
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/unistd.h> 
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#endif

#include "drv_interface.h"



#if !SELFTEST
#include "usb.h"
#include "Descriptors.h"
#endif
/*****************************************************************************
**
**                  Declared Global Variable
**
*****************************************************************************/

//********************* Global *********************//


unsigned short g_u16_dev_id;

unsigned char g_u8_drv_interface;

unsigned char g_u8_gpio_irq_trigger;

unsigned char g_u8_raw_data_buffer[MAX_IMAGE_BUFFER_SIZE * 2];
unsigned short g_u16_raw_data_tmp[MAX_IMAGE_BUFFER_SIZE];

// IC Test
short g_i16_raw_data_1_short_buf[MAX_IMAGE_BUFFER_SIZE];
short g_i16_raw_data_2_open_buf[MAX_IMAGE_BUFFER_SIZE];
unsigned short g_u16_raw_data3_cc_buf[MAX_IMAGE_BUFFER_SIZE + 1];
unsigned short g_u16_uc_buf[MAX_IMAGE_BUFFER_SIZE + 1];//CC
short g_i16_ub_buf[MAX_IMAGE_BUFFER_SIZE + 1];//Baseline
short g_i16_open_bl_data[MAX_IMAGE_BUFFER_SIZE];
short g_i16_raw_data2_golden_bl_buf[MAX_IMAGE_BUFFER_SIZE + 1];
unsigned short g_u16_raw_data3_golden_cc_buf[MAX_IMAGE_BUFFER_SIZE + 1];
unsigned short g_u16_uc_golden_cc_buf[MAX_IMAGE_BUFFER_SIZE];

unsigned int g_u32_test_result[MAX_IMAGE_BUFFER_SIZE]; // each node test result (bit[0]:open ng, bit[1]:short ng, bit[2]:uniformity ng..etc)


unsigned int g_u32_wearable_test_result;
unsigned char g_u8_wearable_pin_map[MAX_IMAGE_BUFFER_SIZE];


unsigned char g_u8_data_buf[DATA_BUFFER_SIZE];
unsigned char g_u8_mipi_read_buf[56];

unsigned char g_u8_channel_x;
unsigned char g_u8_channel_y;

unsigned char g_u8_is_normal_fw;

unsigned short g_u16_test_items_host_cmd;
unsigned short g_u16_test_items_tool_cmd;
unsigned char g_u8_ic_power_on_ng;
char g_i8_test_baseline_msg[30];
volatile unsigned short g_u16_panel_jig_set_test_items;

#if ENABLE_TEST_TIME_MEASURMENT || ENABLE_TEST_TIME_MEASURMENT_CC
unsigned int g_u32_spend_time;
#endif

unsigned int g_u32_fw_cc_version;
unsigned char g_u8_print_debug_msg;

void ic_drv_init(void)
{
	g_u16_test_items_host_cmd = 0xFFFF;
	g_u16_test_items_tool_cmd = 0;
	g_u16_dev_id = DEVICE_ID_2X;
	g_u8_print_debug_msg = 0;
#if !SELFTEST
	g_u8_ic_power_on_ng = FALSE;
#endif
}

/*****************************************************************************
**                            End Of File
******************************************************************************/
