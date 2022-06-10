/*
 * File:    raydium_selftest.c
 * Author:  Valentine <valentine.hsu@rad-ic.com>
 * Brief:   rm31080 touch screen test tool.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/unistd.h> 
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include "drv_interface.h"
#include "chip_raydium/ic_drv_interface.h"
#include "raydium_selftest.h"
#include "chip_raydium/ic_drv_global.h"
#include "tpselftest_20.h"
#if defined(FW_MAPPING_EN)
#include "tpselftest_21.h"
#endif

#define RM_SELF_TEST_CUSTOMER_VERSION	0x02
#define RM_SELF_TEST_PLATFORM_VERSION	0x16
#define RM_SELF_TEST_PROJECT_VERSION		0x40
#define RM_SELF_TEST_MAIN_VERSION		1
#define RM_SELF_TEST_SUB_VERSION		0

#define RESULT_SUCCESS 					1
#define RESULT_NG 					0

#define RELATIVE_PATH	 				0

unsigned char g_u8_normal_fw_version_buf[4];
char str_ini_path[100];

static int self_test_all(void)
{
	int ret = 0;
	
	g_u8_raydium_flag |= ENG_MODE;
	handle_ic_test();
	ret = g_u32_wearable_test_result;

	//g_u8_raydium_flag &= ~ENG_MODE;
	DEBUGOUT("self_test_all end \r\n");

	return ret;
}

int self_test_save_to_file(char *file_name, char *p_string, short len)
{
	struct file *filp = NULL;
	mm_segment_t old_fs;
	filp = filp_open(file_name, O_RDWR | O_CREAT | O_APPEND, 0666);
	if (IS_ERR(filp)) {
		DEBUGOUT("can't open file:%s\n", RM_SELF_TEST_LOGFILE);
		return 0;
	}
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	filp->f_op->write(filp, p_string, len, &filp->f_pos);
	set_fs(old_fs);
	filp_close(filp, NULL);

	return 1;
}

#if 1
static int raydium_check_ini_version(void)
{
	int ret = 0;
	unsigned int u32_test_version;
	memcpy(&u32_test_version, &g_rad_testpara_image[4], 4);

	if (u32_test_version != g_st_test_para_resv.u32_test_fw_version) {
		DEBUGOUT("test fw versio 0x%X != ini version 0x%X\n"
			 , u32_test_version, g_st_test_para_resv.u32_test_fw_version);
		ret = WEARABLE_FT_TEST_RESULT_TEST_FW_VER_NG;
	}
	return ret;
}
#else
static int raydium_check_ini_version(void)
{
	int ret = 0;
	unsigned int u32_test_version, u32_version_20; 
	memcpy(&u32_test_version, &g_st_test_para_resv.u32_test_fw_version, 4);

	if (g_u16_dev_id == DEVICE_ID_2X) {
		memcpy(&u32_version_20, &u8_rad_testpara_20[4], 4);
		DEBUGOUT("ini version 0x%X, 20 version 0x%X\n"
			 , u32_test_version, u32_version_20);

		if (u32_test_version == u32_version_20) {
			DEBUGOUT("map version= 0x%X\r\n", u32_version_20);
		}  else
			ret = WEARABLE_FT_TEST_RESULT_TEST_FW_VER_NG;
	}
	 
	return ret;
}
#endif

static int self_test_init(void)
{
	int ret = 0;
	unsigned int u32_dev_id; 

	g_u8_drv_interface = I2C_INTERFACE;
	
	if (handle_read_pda(0x500009BC, 4, (unsigned char *)&u32_dev_id) == ERROR) {
		ret = WEARABLE_FT_TEST_RESULT_SYSFS_NG;
		return ret;
	}
	g_u16_dev_id = ((u32_dev_id & 0xFFFF0000) >> 16);
	
	if (g_u16_dev_id == DEVICE_ID_2X) {
		handle_read_pda(0x00006a04, 4, g_u8_normal_fw_version_buf);
		DEBUGOUT("FW Version=0x%.2X%.2X%.2X%.2X\n", g_u8_normal_fw_version_buf[0], g_u8_normal_fw_version_buf[1],
			 g_u8_normal_fw_version_buf[3], g_u8_normal_fw_version_buf[2]);
	} else {
		DEBUGOUT("read ic namd fail \n");
		ret = WEARABLE_FT_TEST_RESULT_TEST_INIT_NG;
		return ret;
	}	
	
	if (raydium_check_ini_version() != 0) {
		ret = WEARABLE_FT_TEST_RESULT_TEST_FW_VER_NG;
	}

	return ret;
}

int self_test_save_test_raw_data_to_file(int i32_ng_type)
{
	//struct tm *time_infor;
	//time_t raw_time;
	char write_string[1000]; 
	unsigned char u8_i, u8_j;
#if 0	
	//Date
	time(&raw_time);
	time_infor = localtime(&raw_time);
	memset(write_string, 0, strlen(write_string));
	sprintf(write_string, "Date=%s\n", asctime(time_infor));
	self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
#endif
	//FW Version
	memset(write_string, 0, strlen(write_string));
	if (g_u16_dev_id == DEVICE_ID_2X) {
		sprintf(write_string, "FW Version=0x%.2X%.2X%.2X%.2X\n", g_u8_normal_fw_version_buf[0], g_u8_normal_fw_version_buf[1],
			g_u8_normal_fw_version_buf[3], g_u8_normal_fw_version_buf[2]);
	}
	self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));

	//Version
	memset(write_string, 0, strlen(write_string));
	sprintf(write_string, "Selftest Version=%x.%x.%x.%x.%x\n\n", RM_SELF_TEST_CUSTOMER_VERSION, RM_SELF_TEST_PLATFORM_VERSION, 
		RM_SELF_TEST_PROJECT_VERSION, RM_SELF_TEST_MAIN_VERSION, RM_SELF_TEST_SUB_VERSION);
	self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));

	//Test result
	memset(write_string, 0, strlen(write_string));
	sprintf(write_string, "Test Result = 0x%08X\n\n", i32_ng_type);
	self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));

	if (i32_ng_type == 0) {
		memset(write_string, 0, strlen(write_string));
		sprintf(write_string, "All pass\n\n\n");
		self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
	} else {
		memset(write_string, 0, strlen(write_string));

		if (i32_ng_type & WEARABLE_FT_TEST_RESULT_SYSFS_NG) {
			sprintf(write_string + strlen(write_string), "System NG ");
		}

		if (i32_ng_type & WEARABLE_FT_TEST_RESULT_I2C_NG) {
			sprintf(write_string + strlen(write_string), "I2C NG ");
		}

		if (i32_ng_type & WEARABLE_FT_TEST_RESULT_INT_NG) {
			sprintf(write_string + strlen(write_string), "INT NG ");
		}

		if (i32_ng_type & WEARABLE_FT_TEST_RESULT_RESET_NG) {
			sprintf(write_string + strlen(write_string), "RESET NG ");
		}

		if (i32_ng_type & WEARABLE_FT_TEST_RESULT_PRAM_NG) {
			sprintf(write_string + strlen(write_string), "PRAM NG ");
		}

		if (i32_ng_type & WEARABLE_FT_TEST_RESULT_NORMAL_FW_NG) {
			sprintf(write_string + strlen(write_string), "NORMAL_FW_NG ");
		}

		if (i32_ng_type & WEARABLE_FT_TEST_RESULT_OPEN_NG) {
			sprintf(write_string + strlen(write_string), "OPEN NG ");
		}

		if (i32_ng_type & WEARABLE_FT_TEST_RESULT_SHORT_NG) {
			sprintf(write_string + strlen(write_string), "SHORT NG ");
		}		

		if (i32_ng_type & WEARABLE_FT_TEST_RESULT_BURN_CC_NG) {
			sprintf(write_string + strlen(write_string), "BURN CC NG ");
		}

		if (i32_ng_type & WEARABLE_FT_TEST_RESULT_GET_DATA_NG) {
			sprintf(write_string + strlen(write_string), "GET DATA NG ");
		}

		if (i32_ng_type & WEARABLE_FT_TEST_RESULT_FLASH_ID_NG) {
			sprintf(write_string + strlen(write_string), "FLASH ID NG ");
		}

		if (i32_ng_type & WEARABLE_FT_TEST_RESULT_NORMAL_FW_VER_NG) {
			sprintf(write_string + strlen(write_string), "NORMAL FW VER NG ");
		}

		if (i32_ng_type & WEARABLE_FT_TEST_RESULT_TEST_FW_VER_NG) {
			sprintf(write_string + strlen(write_string), "TEST FW VER NG ");
		}

		if (i32_ng_type & WEARABLE_FT_TEST_RESULT_TEST_INIT_NG) {
			sprintf(write_string + strlen(write_string), "TEST INIT NG ");
		}

		if (i32_ng_type & WEARABLE_FT_TEST_RESULT_LOAD_TESTFW_NG) {
			sprintf(write_string + strlen(write_string), "LOAD TESTFW NG ");
		}

		if (i32_ng_type & WEARABLE_FT_TEST_RESULT_BURN_FW_NG) {
			sprintf(write_string + strlen(write_string), "BURN FW NG ");
		}

		if (i32_ng_type & WEARABLE_FT_TEST_RESULT_SINGLE_CC_OPEN_NG) {
			sprintf(write_string + strlen(write_string), "Open NG (Single Pin CC) ");
		}

		if (i32_ng_type & WEARABLE_FT_TEST_RESULT_SINGLE_CC_SHORT_NG) {
			sprintf(write_string + strlen(write_string), "Short NG (Single Pin CC) ");
		}

		if (i32_ng_type & WEARABLE_FT_TEST_RESULT_UB_NG) {
			sprintf(write_string + strlen(write_string), "Uniformity Baseline NG ");
		}

		if (i32_ng_type & WEARABLE_FT_TEST_RESULT_UC_NG) {
			sprintf(write_string + strlen(write_string), "Uniformity CC NG ");
		}

		sprintf(write_string + strlen(write_string), "\n");
		self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
	}

	//Threshold
	memset(write_string, 0, strlen(write_string));
	sprintf(write_string, "0x%02X, 0x%02X\n0x%02X, 0x%02X\n0x%02X, 0x%02X\n0x%02X, 0x%02X\n0x%02X, 0x%02X\n0x%02X, 0x%02X\n0x%02X, 0x%02X\n0x%02X, 0x%02X\n0x%02X, 0x%02X\n",
		(g_st_test_thd.i16_ft_test_open_lower_thd >> 8) & 0xFF, (g_st_test_thd.i16_ft_test_open_lower_thd & 0xFF),
		(g_st_test_thd.i16_ft_test_short_upper_thd >> 8) & 0xFF, (g_st_test_thd.i16_ft_test_short_upper_thd & 0xFF),
		(g_st_test_thd.i16_ft_test_short_lower_thd >> 8) & 0xFF, (g_st_test_thd.i16_ft_test_short_lower_thd & 0xFF),
		(g_st_test_thd.i16_ft_test_single_cc_upper_thd >> 8) & 0xFF, (g_st_test_thd.i16_ft_test_single_cc_upper_thd & 0xFF),
		(g_st_test_thd.i16_ft_test_single_cc_lower_thd >> 8) & 0xFF, (g_st_test_thd.i16_ft_test_single_cc_lower_thd & 0xFF), 
		(g_st_test_thd.i16_ft_test_uniformity_bl_upper_thd >> 8) & 0xFF, (g_st_test_thd.i16_ft_test_uniformity_bl_upper_thd & 0xFF),
		(g_st_test_thd.i16_ft_test_uniformity_bl_lower_thd >> 8) & 0xFF, (g_st_test_thd.i16_ft_test_uniformity_bl_lower_thd & 0xFF),
		(g_st_test_thd.i16_ft_test_uniformity_cc_upper_thd >> 8) & 0xFF, (g_st_test_thd.i16_ft_test_uniformity_cc_upper_thd & 0xFF),
		(g_st_test_thd.i16_ft_test_uniformity_cc_lower_thd >> 8) & 0xFF, (g_st_test_thd.i16_ft_test_uniformity_cc_lower_thd & 0xFF));
	self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));

	for (u8_i = 0; u8_i < MAX_IMAGE_BUFFER_SIZE; u8_i++) {
		memset(write_string, 0, strlen(write_string));
		sprintf(write_string, "0x%2X,", g_u16_uc_golden_cc_buf[u8_i]);
		self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
	}

	memset(write_string, 0, strlen(write_string));
	sprintf(write_string, "\n");
	self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));

	for (u8_i = 0; u8_i < MAX_IMAGE_BUFFER_SIZE; u8_i++) {
		memset(write_string, 0, strlen(write_string));
		sprintf(write_string, "0x%2X,", g_u16_raw_data3_golden_cc_buf[u8_i]);
		self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
	}

	memset(write_string, 0, strlen(write_string));
	sprintf(write_string, "\n\n\n\n\n\n\n\n\n\n\n\n\n");
	self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));

	if ((i32_ng_type & 0xFFF8FBF8) < 4) {

		if ((g_st_test_info.u16_ft_test_item & (IC_TEST_ITEMS_SHORT)) != 0) {
			//Raw data
			//Raw data slow
			memset(write_string, 0, strlen(write_string));
			sprintf(write_string, "\r\n\n\n");
			self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
			memset(write_string, 0, strlen(write_string));
			sprintf(write_string, "Raw Data 1\n");
			self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
#if DATA_MAP_5_5
			for (u8_i = 0; u8_i < 25; u8_i++) {
				if (u8_i % 5 == 0 && u8_i != 0) {
					memset(write_string, 0, strlen(write_string));
					sprintf(write_string, "\r\n");
					self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
				}

				if (/*u8_i == 0 || u8_i == 5 || u8_i == 30 || u8_i == 35*/0) {
					memset(write_string, 0, strlen(write_string));
					sprintf(write_string, "%05d,", 0);
					self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
				} else {
					for (u8_j = 0; u8_j < MAX_IMAGE_BUFFER_SIZE; u8_j++) {
						if (g_u8_wearable_pin_map[u8_j] != NA_P && g_u8_wearable_pin_map[u8_j] == u8_i) {
							memset(write_string, 0, strlen(write_string));
							sprintf(write_string, "%05d,", g_i16_raw_data_1_short_buf[u8_j]);
							self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
							break;
						}
					}

					if (u8_j == MAX_IMAGE_BUFFER_SIZE) {
						memset(write_string, 0, strlen(write_string));
						sprintf(write_string, "%05d,", 0);
						self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
					}
				}
			}
#else
			for (u8_i = 0; u8_i < 36; u8_i++) {
				if (u8_i % 6 == 0 && u8_i != 0) {
					memset(write_string, 0, strlen(write_string));
					sprintf(write_string, "\r\n");
					self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
				}

				if (u8_i == 0 || u8_i == 5 || u8_i == 30 || u8_i == 35) {
					memset(write_string, 0, strlen(write_string));
					sprintf(write_string, "%05d,", 0);
					self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
				} else {
					for (u8_j = 0; u8_j < MAX_IMAGE_BUFFER_SIZE; u8_j++) {
						if (g_u8_wearable_pin_map[u8_j] != NA_P && g_u8_wearable_pin_map[u8_j] == u8_i) {
							memset(write_string, 0, strlen(write_string));
							sprintf(write_string, "%05d,", g_i16_raw_data_1_short_buf[u8_j]);
							self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
							break;
						}
					}

					if (u8_j == MAX_IMAGE_BUFFER_SIZE) {
						memset(write_string, 0, strlen(write_string));
						sprintf(write_string, "%05d,", 0);
						self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
					}
				}
			}
#endif			
		}

		if ((g_st_test_info.u16_ft_test_item & (IC_TEST_ITEMS_OPEN)) != 0) {
			memset(write_string, 0, strlen(write_string));
			sprintf(write_string, "\r\n\n\n");
			self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));

			//Raw data quick
			memset(write_string, 0, strlen(write_string));
			sprintf(write_string, "Raw Data 2\n");
			self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
#if DATA_MAP_5_5
			for (u8_i = 0; u8_i < 25; u8_i++) {
				if (u8_i % 5 == 0 && u8_i != 0) {
					memset(write_string, 0, strlen(write_string));
					sprintf(write_string, "\r\n");
					self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
				}

				if (/*u8_i == 0 || u8_i == 5 || u8_i == 30 || u8_i == 35*/0) {
					memset(write_string, 0, strlen(write_string));
					sprintf(write_string, "%05d,", 0);
					self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
				} else {
					for (u8_j = 0; u8_j < MAX_IMAGE_BUFFER_SIZE; u8_j++) {
						if (g_u8_wearable_pin_map[u8_j] != NA_P && g_u8_wearable_pin_map[u8_j] == u8_i) {
							memset(write_string, 0, strlen(write_string));
							sprintf(write_string, "%05d,", g_i16_raw_data_2_open_buf[u8_j]);
							self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
							break;
						}
					}

					if (u8_j == MAX_IMAGE_BUFFER_SIZE) {
						memset(write_string, 0, strlen(write_string));
						sprintf(write_string, "%05d,", 0);
						self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
					}
				}
			}
#else
			for (u8_i = 0; u8_i < 36; u8_i++) {

				if (u8_i % 6 == 0 && u8_i != 0) {
					memset(write_string, 0, strlen(write_string));
					sprintf(write_string, "\r\n");
					self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
				}

				if (u8_i == 0 || u8_i == 5 || u8_i == 30 || u8_i == 35) {
					memset(write_string, 0, strlen(write_string));
					sprintf(write_string, "%05d,", 0);
					self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
				} else {
					for (u8_j = 0; u8_j < MAX_IMAGE_BUFFER_SIZE; u8_j++) {
						if (g_u8_wearable_pin_map[u8_j] != NA_P && g_u8_wearable_pin_map[u8_j] == u8_i) {
							memset(write_string, 0, strlen(write_string));
							sprintf(write_string, "%05d,", g_i16_raw_data_2_open_buf[u8_j]);
							self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
							break;
						}
					}

					if (u8_j == MAX_IMAGE_BUFFER_SIZE) {
						memset(write_string, 0, strlen(write_string));
						sprintf(write_string, "%05d,", 0);
						self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
					}
				}
			}
#endif			
		}		
		if ((g_st_test_info.u16_ft_test_item & (IC_TEST_ITEMS_OPEN | IC_TEST_ITEMS_SHORT)) != 0) {
			memset(write_string, 0, strlen(write_string));
			sprintf(write_string, "\r\n\n\n");
			self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));

			//Raw data 3
			memset(write_string, 0, strlen(write_string));
			sprintf(write_string, "Raw Data 3\n");
			self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
#if DATA_MAP_5_5
			for (u8_i = 0; u8_i < 25; u8_i++) {
				if (u8_i % 5 == 0 && u8_i != 0) {
					memset(write_string, 0, strlen(write_string));
					sprintf(write_string, "\r\n");
					self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
				}

				if (/*u8_i == 0 || u8_i == 5 || u8_i == 30 || u8_i == 35*/0) {
					memset(write_string, 0, strlen(write_string));
					sprintf(write_string, "%05d,", 0);
					self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
				} else {
					for (u8_j = 0; u8_j < MAX_IMAGE_BUFFER_SIZE; u8_j++) {
						if (g_u8_wearable_pin_map[u8_j] != NA_P && g_u8_wearable_pin_map[u8_j] == u8_i) {
							memset(write_string, 0, strlen(write_string));
							sprintf(write_string, "%05d,", g_u16_raw_data3_cc_buf[u8_j]);
							self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
							break;
						}
					}

					if (u8_j == MAX_IMAGE_BUFFER_SIZE) {
						memset(write_string, 0, strlen(write_string));
						sprintf(write_string, "%05d,", 0);
						self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
					}
				}
			}
#else
			for (u8_i = 0; u8_i < 36; u8_i++) {

				if (u8_i % 6 == 0 && u8_i != 0) {
					memset(write_string, 0, strlen(write_string));
					sprintf(write_string, "\r\n");
					self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
				}

				if (u8_i == 0 || u8_i == 5 || u8_i == 30 || u8_i == 35) {
					memset(write_string, 0, strlen(write_string));
					sprintf(write_string, "%05d,", 0);
					self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
				} else {
					for (u8_j = 0; u8_j < MAX_IMAGE_BUFFER_SIZE; u8_j++) {
						if (g_u8_wearable_pin_map[u8_j] != NA_P && g_u8_wearable_pin_map[u8_j] == u8_i) {
							memset(write_string, 0, strlen(write_string));
							sprintf(write_string, "%05d,", g_u16_raw_data3_cc_buf[u8_j]);
							self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
							break;
						}
					}

					if (u8_j == MAX_IMAGE_BUFFER_SIZE) {
						memset(write_string, 0, strlen(write_string));
						sprintf(write_string, "%05d,", 0);
						self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
					}
				}
			}
#endif			
		}

		if ((g_st_test_info.u16_ft_test_item & (IC_TEST_ITEMS_UC)) != 0) {
			memset(write_string, 0, strlen(write_string));
			sprintf(write_string, "\r\n\n\n");
			self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));

			//Raw data Uniformity CC
			memset(write_string, 0, strlen(write_string));
			sprintf(write_string, "Raw Data_UC\n");
			self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
#if DATA_MAP_5_5
			for (u8_i = 0; u8_i < 25; u8_i++) {
				if (u8_i % 5 == 0 && u8_i != 0) {
					memset(write_string, 0, strlen(write_string));
					sprintf(write_string, "\r\n");
					self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
				}

				if (/*u8_i == 0 || u8_i == 5 || u8_i == 30 || u8_i == 35*/0) {
					memset(write_string, 0, strlen(write_string));
					sprintf(write_string, "%05d,", 0);
					self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
				} else {
					for (u8_j = 0; u8_j < MAX_IMAGE_BUFFER_SIZE; u8_j++) {
						if (g_u8_wearable_pin_map[u8_j] != NA_P && g_u8_wearable_pin_map[u8_j] == u8_i) {
							memset(write_string, 0, strlen(write_string));
							sprintf(write_string, "%05d,", g_u16_uc_buf[u8_j]);
							self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
							break;
						}
					}

					if (u8_j == MAX_IMAGE_BUFFER_SIZE) {
						memset(write_string, 0, strlen(write_string));
						sprintf(write_string, "%05d,", 0);
						self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
					}
				}
			}
#else
			for (u8_i = 0; u8_i < 36; u8_i++) {
				if (u8_i % 6 == 0 && u8_i != 0) {
					memset(write_string, 0, strlen(write_string));
					sprintf(write_string, "\r\n");
					self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
				}

				if (u8_i == 0 || u8_i == 5 || u8_i == 30 || u8_i == 35) {
					memset(write_string, 0, strlen(write_string));
					sprintf(write_string, "%05d,", 0);
					self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
				} else {
					for (u8_j = 0; u8_j < MAX_IMAGE_BUFFER_SIZE; u8_j++) {
						if (g_u8_wearable_pin_map[u8_j] != NA_P && g_u8_wearable_pin_map[u8_j] == u8_i) {
							memset(write_string, 0, strlen(write_string));
							sprintf(write_string, "%05d,", g_u16_uc_buf[u8_j]);
							self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
							break;
						}
					}

					if (u8_j == MAX_IMAGE_BUFFER_SIZE) {
						memset(write_string, 0, strlen(write_string));
						sprintf(write_string, "%05d,", 0);
						self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
					}
				}
			}
#endif			
		}

		if ((g_st_test_info.u16_ft_test_item & (IC_TEST_ITEMS_UB)) != 0) {
			memset(write_string, 0, strlen(write_string));
			sprintf(write_string, "\r\n\n\n");
			self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));

			//Raw data Uniformity BL
			memset(write_string, 0, strlen(write_string));
			sprintf(write_string, "Raw Data_UB\n");
			self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
#if DATA_MAP_5_5
			for (u8_i = 0; u8_i < 25; u8_i++) {
				if (u8_i % 5 == 0 && u8_i != 0) {
					memset(write_string, 0, strlen(write_string));
					sprintf(write_string, "\r\n");
					self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
				}

				if (/*u8_i == 0 || u8_i == 5 || u8_i == 30 || u8_i == 35*/0) {
					memset(write_string, 0, strlen(write_string));
					sprintf(write_string, "%05d,", 0);
					self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
				} else {
					for (u8_j = 0; u8_j < MAX_IMAGE_BUFFER_SIZE; u8_j++) {
						if (g_u8_wearable_pin_map[u8_j] != NA_P && g_u8_wearable_pin_map[u8_j] == u8_i) {
							memset(write_string, 0, strlen(write_string));
							sprintf(write_string, "%05d,", g_i16_ub_buf[u8_j]);
							self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
							break;
						}
					}

					if (u8_j == MAX_IMAGE_BUFFER_SIZE) {
						memset(write_string, 0, strlen(write_string));
						sprintf(write_string, "%05d,", 0);
						self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
					}
				}
			}
#else
			for (u8_i = 0; u8_i < 36; u8_i++) {

				if (u8_i % 6 == 0 && u8_i != 0) {
					memset(write_string, 0, strlen(write_string));
					sprintf(write_string, "\r\n");
					self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
				}

				if (u8_i == 0 || u8_i == 5 || u8_i == 30 || u8_i == 35) {
					memset(write_string, 0, strlen(write_string));
					sprintf(write_string, "%05d,", 0);
					self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
				} else {
					for (u8_j = 0; u8_j < MAX_IMAGE_BUFFER_SIZE; u8_j++) {
						if (g_u8_wearable_pin_map[u8_j] != NA_P && g_u8_wearable_pin_map[u8_j] == u8_i) {
							memset(write_string, 0, strlen(write_string));
							sprintf(write_string, "%05d,", g_i16_ub_buf[u8_j]);
							self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
							break;
						}
					}

					if (u8_j == MAX_IMAGE_BUFFER_SIZE) {
						memset(write_string, 0, strlen(write_string));
						sprintf(write_string, "%05d,", 0);
						self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));
					}
				}
			}
#endif			
		}
	}

	memset(write_string, 0, strlen(write_string));
	sprintf(write_string, "\r\n\n\n");
	self_test_save_to_file(RM_SELF_TEST_LOGFILE, write_string, strlen(write_string));

	return 1;
}

int self_test_read_setting_from_file(void)
{
	unsigned short u16_offset = 0;
	
	switch (g_raydium_ts->id) {
	case RAD_SELFTEST_20:	
		u16_offset = 0;
		memcpy(&g_u8_ini_flash[u16_offset], &u8_test_info_20, sizeof(u8_test_info_20));
		u16_offset += sizeof(g_st_test_info);
		memcpy(&g_u8_ini_flash[u16_offset], &i8_ft_test_thd_20, sizeof(i8_ft_test_thd_20));
		u16_offset += 36;
		memcpy(&g_u8_ini_flash[u16_offset], &u8_test_para_20, sizeof(u8_test_para_20));
		u16_offset += sizeof(u8_test_para_20);	
		memcpy(&g_u8_ini_flash[u16_offset], &u8_raw_data_2_bl_20, sizeof(u8_raw_data_2_bl_20));
		u16_offset += 72;
		memcpy(&g_u8_ini_flash[u16_offset], &u8_raw_data_3_cc_20, sizeof(u8_raw_data_3_cc_20));
		u16_offset += 72;
		memcpy(&g_u8_ini_flash[u16_offset], &u8_raw_uc_cc_20, sizeof(u8_raw_uc_cc_20));
		u16_offset += 72;

		memcpy((void *)(&g_st_test_para_resv) , &u8_test_para_20[0], sizeof(g_st_test_para_resv));	
		DEBUGOUT("ini length = %d\r\n", u16_offset);
		break;
#if defined(FW_MAPPING_EN)		
	case RAD_SELFTEST_21:	
		u16_offset = 0;
		memcpy(&g_u8_ini_flash[u16_offset], &u8_test_info_21, sizeof(u8_test_info_21));
		u16_offset += sizeof(g_st_test_info);
		memcpy(&g_u8_ini_flash[u16_offset], &i8_ft_test_thd_21, sizeof(i8_ft_test_thd_21));
		u16_offset += 36;
		memcpy(&g_u8_ini_flash[u16_offset], &u8_test_para_21, sizeof(u8_test_para_21));
		u16_offset += sizeof(u8_test_para_21);	
		memcpy(&g_u8_ini_flash[u16_offset], &u8_raw_data_2_bl_21, sizeof(u8_raw_data_2_bl_21));
		u16_offset += 72;
		memcpy(&g_u8_ini_flash[u16_offset], &u8_raw_data_3_cc_21, sizeof(u8_raw_data_3_cc_21));
		u16_offset += 72;
		memcpy(&g_u8_ini_flash[u16_offset], &u8_raw_uc_cc_21, sizeof(u8_raw_uc_cc_21));
		u16_offset += 72;

		memcpy((void *)(&g_st_test_para_resv) , &u8_test_para_21[0], sizeof(g_st_test_para_resv));	
		DEBUGOUT("ini length = %d\r\n", u16_offset);
		break;
#endif		
	}	

	return 0;	
}

//return value: 0 succeed
//              1 open NG
//              2 short NG
//              3 uniformtiy NG
//              4 burn fw NG
//              5 I2C NG
//              6 INT NG
//              7 Reset pin NG
int raydium_do_selftest(struct raydium_ts_data *ts)
{
	int ret = RESULT_SUCCESS;
	unsigned int time_start, time_end, time_start2, time_end2;
	time_start = get_system_time();

	pr_info("Selftest Version=%x.%x.%x.%x.%x\n", RM_SELF_TEST_CUSTOMER_VERSION, RM_SELF_TEST_PLATFORM_VERSION, 
		RM_SELF_TEST_PROJECT_VERSION, RM_SELF_TEST_MAIN_VERSION, RM_SELF_TEST_SUB_VERSION);

	self_test_read_setting_from_file();
	ic_drv_init();
	set_raydium_ts_data(ts);

	ret = self_test_init();
	if (ret != 0) {
		DEBUGOUT("mapping ic fw fail \n");
	} else {
		DEBUGOUT("Test all\n");
		ret |= self_test_all();
	}
#if 1
	if(ret != WEARABLE_FT_TEST_RESULT_SYSFS_NG) {
		gpio_touch_hw_reset();
		g_u8_raydium_flag &= ~ENG_MODE;
	}
	
	raydium_i2c_mode_control(ts->client, ENABLE_TOUCH_INT);
#if ENABLE_TIME_MEASURMENT
	time_start2 = get_system_time();
#endif
	self_test_save_test_raw_data_to_file(ret);

#if ENABLE_TIME_MEASURMENT
	time_end2 = get_system_time();
	DEBUGOUT("Write log Finish(%ums)\n", time_end2 - time_start2);
#endif
	if (ret != 0) {
		DEBUGOUT("Selftest Test Result=0x%x\n", ret);
		ret = RESULT_NG;
		DEBUGOUT("Selftest Result=%d\n", ret);
	} else {
		DEBUGOUT("Selftest Pass ^_^!!!\n");
		ret = RESULT_SUCCESS;
	}

	time_end = get_system_time();
	DEBUGOUT("All Test Finish(%ums)\n", time_end - time_start);

#endif
	return ret;
}
