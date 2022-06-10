#ifdef __KERNEL__
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/unistd.h> 
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#else
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#endif
#include "drv_interface.h"
#include "ic_drv_global.h"

#include "ic_drv_interface.h"
#include "f302_ic_control.h"
#include "f302_ic_test.h"
#include "f302_ic_reg.h"
#if !SELFTEST
#include "f302_ic_test_ft.h"
#include "ic_drv_interface_ft.h"
#include "ic_drv_global_ft.h"
#endif

STATUS turn_on_flash_2x(void)
{
	unsigned int u32_read = 0;
	//Turn on Flash
	if (handle_ic_write(REG_I2C_I2CFLASHPRO, 4, (unsigned char *)&u32_read, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}

	u32_read = FLH_RELEASE_PD;
	if (handle_ic_write(REG_FLASHCTL_FLASH_ISPCTL, 4, (unsigned char *)&u32_read, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}
	return SUCCESS;
}

STATUS read_fpc_flash_2x(unsigned int u32_addr, unsigned int *p_u32_data)
{
	unsigned int u32_read = 0;
	unsigned int u32_write = 0;

	//Turn on Flash
	if (turn_on_flash_2x() == ERROR) {
		return ERROR;
	}

	if (handle_ic_read(REG_SYSCON_BLKEN_ADDR, 4, (unsigned char *)(&u32_read), g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}

	u32_read |= 0x40;
	if (handle_ic_write(REG_SYSCON_BLKEN_ADDR, 4, (unsigned char *)&u32_read, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}

	u32_write = 0;
	if (handle_ic_write(REG_I2C_I2CFLASHPRO, 4, (unsigned char *)&u32_write, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}

	if (handle_ic_write(REG_FLASHCTL_FLASH_ADDR, 4, (unsigned char *)&u32_addr, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}

	u32_write = 0x40;
	if (handle_ic_write(REG_FLASHCTL_FLASH_ISPCTL, 4, (unsigned char *)&u32_write, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}
	delay_ms(1);

	if (handle_ic_read(REG_FLASHCTL_FLASH_DATA, 4, (unsigned char *)(&u32_read), g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}

	*p_u32_data = u32_read;
	return SUCCESS;
}

STATUS set_test_info_thd_para_2x(void)
{
	unsigned char u8_read_buf[64], u8_i;
	unsigned char *p_u8_input_buf;

	if (handle_ic_write(FT_TEST_INFO_ADDR, sizeof(g_st_test_info), (unsigned char *)&g_st_test_info, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}
	if (handle_ic_read(FT_TEST_INFO_ADDR, sizeof(g_st_test_info), (unsigned char *)u8_read_buf, g_u8_drv_interface, I2C_BYTE_MODE) == ERROR) {
		return ERROR;
	} else {
		p_u8_input_buf = (unsigned char *)&g_st_test_info;
		for (u8_i = 0; u8_i < sizeof(g_st_test_info); u8_i++) {
			if (p_u8_input_buf[u8_i] != u8_read_buf[u8_i]) {
				DEBUGOUT("Set INFO NG:[%d]=%d->%d\r\n", u8_i, p_u8_input_buf[u8_i], u8_read_buf[u8_i]);
				return ERROR;
			}
		}
	}
	if (handle_ic_write(FT_TEST_THD_ADDR, sizeof(g_st_test_thd), (unsigned char *)&g_st_test_thd, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}
	if (handle_ic_read(FT_TEST_THD_ADDR, sizeof(g_st_test_thd), (unsigned char *)u8_read_buf, g_u8_drv_interface, I2C_BYTE_MODE) == ERROR) {
		return ERROR;
	} else {
		p_u8_input_buf = (unsigned char *)&g_st_test_thd;
		for (u8_i = 0; u8_i < sizeof(g_st_test_thd); u8_i++) {
			if (p_u8_input_buf[u8_i] != u8_read_buf[u8_i]) {
				DEBUGOUT("Set THD NG:[%d]=%d->%d\r\n", u8_i, p_u8_input_buf[u8_i], u8_read_buf[u8_i]);
				return ERROR;
			}
		}
	}
	if (handle_ic_write(FT_TEST_PARA_ADDR, sizeof(g_st_test_para_resv), (unsigned char *)&g_st_test_para_resv, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}
	if (handle_ic_read(FT_TEST_PARA_ADDR, sizeof(g_st_test_para_resv), (unsigned char *)u8_read_buf, g_u8_drv_interface, I2C_BYTE_MODE) == ERROR) {
		return ERROR;
	} else {
		p_u8_input_buf = (unsigned char *)&g_st_test_para_resv;
		for (u8_i = 0; u8_i < sizeof(g_st_test_para_resv); u8_i++) {
			if (p_u8_input_buf[u8_i] != u8_read_buf[u8_i]) {
				DEBUGOUT("Set Para NG:[%d]=%d->%d\r\n", u8_i, p_u8_input_buf[u8_i], u8_read_buf[u8_i]);
				return ERROR;
			}
		}
	}

	if (handle_ic_write(SRAM_FT_UC_CC_ADDR, 60, (unsigned char *)g_u16_uc_golden_cc_buf, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}

	if (handle_ic_read(SRAM_FT_UC_CC_ADDR, 60, (unsigned char *)u8_read_buf, g_u8_drv_interface, I2C_BYTE_MODE) == ERROR) {
		return ERROR;
	} else {
		p_u8_input_buf = (unsigned char *)g_u16_uc_golden_cc_buf;
		for (u8_i = 0; u8_i < 60; u8_i++) {
			if (p_u8_input_buf[u8_i] != u8_read_buf[u8_i]) {
				DEBUGOUT("Set PRAM_FT_FW_GOLDEN_CC_ADDR NG:[%d]=%d->%d\r\n", u8_i, p_u8_input_buf[u8_i], u8_read_buf[u8_i]);
				return ERROR;
			}
		}
	}

	if (handle_ic_write(SRAM_FT_RAWDATA_3_CC_ADDR, 60, (unsigned char *)g_u16_raw_data3_golden_cc_buf, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}

	if (handle_ic_read(SRAM_FT_RAWDATA_3_CC_ADDR, 60, (unsigned char *)u8_read_buf, g_u8_drv_interface, I2C_BYTE_MODE) == ERROR) {
		return ERROR;
	} else {
		p_u8_input_buf = (unsigned char *)g_u16_raw_data3_golden_cc_buf;
		for (u8_i = 0; u8_i < 60; u8_i++) {
			if (p_u8_input_buf[u8_i] != u8_read_buf[u8_i]) {
				DEBUGOUT("Set PRAM_FT_OPEN_GOLDEN_CC_ADDR NG:[%d]=%d->%d\r\n", u8_i, p_u8_input_buf[u8_i], u8_read_buf[u8_i]);
				return ERROR;
			}
		}
	}

	if (handle_ic_write(SRAM_FT_RAWDATA_2_BL_ADDR, 60, (unsigned char *)g_i16_raw_data2_golden_bl_buf, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}
	if (handle_ic_read(SRAM_FT_RAWDATA_2_BL_ADDR, 60, (unsigned char *)u8_read_buf, g_u8_drv_interface, I2C_BYTE_MODE) == ERROR) {
		return ERROR;
	} else {
		p_u8_input_buf = (unsigned char *)g_i16_raw_data2_golden_bl_buf;
		for (u8_i = 0; u8_i < 60; u8_i++) {
			if (p_u8_input_buf[u8_i] != u8_read_buf[u8_i]) {
				DEBUGOUT("Set SRAM_FT_RAWDATA_2_BL_ADDR NG:[%d]=%d->%d\r\n", u8_i, p_u8_input_buf[u8_i], u8_read_buf[u8_i]);
				return ERROR;
			}
		}
	}

	return SUCCESS;
}

void dump_image_data_2x(short *p_image_buf, unsigned char u8_remap)
{
	short *p_i16_buf = (short *)g_u16_raw_data_tmp;
	unsigned char u8_i, u8_j;

	memset(p_i16_buf, 0, MAX_IMAGE_BUFFER_SIZE * 2);
	if (u8_remap) {
		for (u8_i = 0; u8_i < MAX_IMAGE_BUFFER_SIZE; u8_i++) {
			if (g_u8_wearable_pin_map[u8_i] != NA_P) {
				p_i16_buf[g_u8_wearable_pin_map[u8_i]] = p_image_buf[u8_i];
			}
		}
	} else {
		memcpy(p_i16_buf, p_image_buf, MAX_IMAGE_BUFFER_SIZE << 1);
	}

	for (u8_j = 0; u8_j < g_u8_channel_y; u8_j++) {
		for (u8_i = 0; u8_i < g_u8_channel_x; u8_i++) {
			DEBUGOUT("%4d, ", p_i16_buf[u8_i + u8_j * g_u8_channel_x]);
		}
		DEBUGOUT("\r\n");
	}
}

void dump_image_hex_data_2x(short *p_image_buf)
{

	short *p_i16_buf = (short *)g_u16_raw_data_tmp;
	unsigned char u8_i, u8_j;

	memset(p_i16_buf, 0, MAX_IMAGE_BUFFER_SIZE * 2);
	for (u8_i = 0; u8_i < MAX_IMAGE_BUFFER_SIZE; u8_i++) {
		if (g_u8_wearable_pin_map[u8_i] != NA_P) {
			p_i16_buf[g_u8_wearable_pin_map[u8_i]] = p_image_buf[u8_i];
		}
	}

	for (u8_j = 0; u8_j < g_u8_channel_y; u8_j++) {
		for (u8_i = 0; u8_i < g_u8_channel_x; u8_i++) {
			DEBUGOUT("%4X, ", (p_i16_buf[u8_i + u8_j * g_u8_channel_x] & 0x7ff));
		}
		DEBUGOUT("\r\n");
	}
}

STATUS check_test_fw_status_2x(unsigned char u8_target_status, unsigned char *p_u8_result)
{
	unsigned int u32_read;

	if (handle_ic_read(FW_FT_ARG1_ADDR, 4, (unsigned char *)(&u32_read), g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}

	if ((u32_read & FT_TEST_STUATUS_PATTERN) == FT_TEST_STUATUS_PATTERN) {
		if (((unsigned char)(u32_read & 0x00FF)) == u8_target_status)
			*p_u8_result = 1;
		else {
			DEBUGOUT("[CTFS] Test STATUS NG!! (%x:%x)\r\n", u8_target_status, u32_read);
			*p_u8_result = 0;
			return ERROR;
		}
		DEBUGOUT("[CTFS] Test FW STATUS (%x:%x)\r\n", u8_target_status, u32_read);
	} else {
		DEBUGOUT("[CTFS] Test STATUS Not Ready!! (%x:%x)\r\n", u8_target_status, u32_read);
		*p_u8_result = 0;
	}
	return SUCCESS;
}

STATUS ft_test_do_fw_test_2x(unsigned short u16_test_items)
{
	unsigned char u8_write_data[5], u8_result;
	unsigned int u32_read_data;
	short i16_time_out = 0;
#if ENABLE_TEST_TIME_MEASURMENT
	unsigned int u32_fun_time = 0;
	u32_fun_time = get_system_time();
	DEBUGOUT("ft_test_do_fw_test Start Time= %d\r\n", u32_fun_time);
#endif

	g_u32_wearable_test_result &= ~WEARABLE_FT_TEST_RESULT_TEST_INIT_NG;

	if ((u16_test_items & (IC_TEST_ITEMS_OPEN | IC_TEST_ITEMS_SHORT | IC_TEST_ITEMS_UC | IC_TEST_ITEMS_UB)) == 0) {
		return SUCCESS;
	}
	memset(u8_write_data, 0, sizeof(u8_write_data));
	u8_write_data[1] = FT_CMD_DO_FT_TEST | 0x80;
	u8_write_data[2] = (unsigned char) u16_test_items;
	u8_write_data[3] = (unsigned char)(u16_test_items >> 8);

	DEBUGOUT("Star FW Test!!\r\n");

	if (handle_ic_write(FW_SYS_CMD_ADDR, 4, u8_write_data, g_u8_drv_interface, I2C_WORD_MODE) == ERROR)
		return ERROR;

	//Wait test fw finish INT //Wait 20 Sec
	i16_time_out = 1000;

	while (i16_time_out--) {
		delay_ms(20);
#if ENABLE_WDT
		// reload the Watchdog timer
		Chip_WWDT_Feed(LPC_WWDT);
#endif
		if (!gpio_touch_int_pin_state_access() || (i16_time_out < 10)) {
			if (check_test_fw_status_2x(FT_CMD_DO_FT_TEST, &u8_result) == ERROR) {
				DEBUGOUT("[FWT] Wait Test CMD NG (0x%x:%d)\r\n", FT_CMD_DO_FT_TEST, i16_time_out);
				return ERROR;
			} else if (u8_result == TRUE) {
				break;
			}
		}
	}

	if (handle_ic_read(FW_SYS_CMD_ADDR, 4, (unsigned char *)(&u32_read_data), g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}
	DEBUGOUT("[FWT]SysCMD1:0x%x (%d)\r\n", u32_read_data, i16_time_out);

	if (i16_time_out == IC_TEST_TIME_OUT) {
		DEBUGOUT("[FWT] Wait Test CMD Timeout!! (0x%x)\r\n", FT_CMD_DO_FT_TEST);
		return ERROR;
	}

#if ENABLE_TEST_TIME_MEASURMENT
	DEBUGOUT("[FWT] FW Test Time(%d)\r\n", get_system_time() - u32_fun_time);
#endif

	return SUCCESS;
}

STATUS enter_fw_test_mode_2x(void)
{
	unsigned char u8_result = 0;
	short i16_time_out = 0;

#if ENABLE_TEST_TIME_MEASURMENT
	unsigned int u32_fun_time = 0;
	u32_fun_time = get_system_time();
#endif

	g_u32_wearable_test_result &= ~WEARABLE_FT_TEST_RESULT_TEST_INIT_NG;

	DEBUGOUT("[EFTM] Start\r\n");

	if ((g_st_test_info.u16_ft_test_item & (IC_TEST_ITEMS_OPEN | IC_TEST_ITEMS_SHORT |
						IC_TEST_ITEMS_UC | IC_TEST_ITEMS_UB)) == 0) {
		return SUCCESS;
	}

	g_u8_is_normal_fw = FALSE;

	i16_time_out = 300;
	u8_result = 0;
	while (i16_time_out--) {
		if (!gpio_touch_int_pin_state_access() || (i16_time_out < 20)) {
			if (check_test_fw_status_2x(FT_CMD_INIT, &u8_result) == ERROR) {
				goto IC_INIT_NG;
			} else if (u8_result == TRUE) {
				break;
			}
		}
		delay_ms(5);
	}

	if (i16_time_out == IC_TEST_TIME_OUT) {
		DEBUGOUT("[EFTM] FW State Check NG\r\n");
		goto IC_INIT_NG;
	}

	DEBUGOUT("[EFTM] Enter FT mode\r\n");

	if (set_test_info_thd_para_2x() == ERROR) {
		DEBUGOUT("[RUPI] Set test info, thd, para NG!\r\n");
		return ERROR;
	}

#if ENABLE_TEST_TIME_MEASURMENT
	DEBUGOUT("[EFTM] (%d)\r\n", get_system_time() - u32_fun_time);
#endif
	return SUCCESS;

IC_INIT_NG:
	g_u32_wearable_test_result = WEARABLE_FT_TEST_RESULT_TEST_INIT_NG;
	return ERROR;
}

STATUS ft_test_read_used_pin_infor_2x(unsigned char *p_u8_infor)
{
	unsigned char u8_r_buf[2];

	//get Pin remap
	if (handle_ic_read(FW_FT_PIN_ADDR, 36, p_u8_infor, g_u8_drv_interface, I2C_BYTE_MODE) == ERROR) {
		DEBUGOUT("[RUPI] NG!\r\n");
		return ERROR;
	}

	if (handle_ic_read(FW_FT_CHANNEL_X_ADDR, 2, u8_r_buf, g_u8_drv_interface, I2C_BYTE_MODE) == ERROR) {
		DEBUGOUT("[RUPI] NG!\r\n");
		return ERROR;
	}
	g_u8_channel_x = u8_r_buf[0];
	g_u8_channel_y = u8_r_buf[1];
	DEBUGOUT("[RUPI] X:%d,Y:%d\r\n", g_u8_channel_x, g_u8_channel_y);


	return SUCCESS;
}

STATUS ft_test_ctrl_mbist_fun_2x(unsigned char u8_enable)
{
	unsigned int u32_read;
	if (u8_enable) {
		if (handle_ic_read(REG_SYSCON_BLKEN_ADDR, 4, (unsigned char *)(&u32_read), g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
			DEBUGOUT("Data RAM Test NG !!!\r\n");
			return ERROR;
		}

		u32_read |= (1 << 29);
		if (handle_ic_write(REG_SYSCON_BLKEN_ADDR, 4, (unsigned char *)&u32_read, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
			DEBUGOUT("Data RAM Test NG !!!\r\n");
			return ERROR;
		}
	} else {
		if (handle_ic_read(REG_SYSCON_BLKEN_ADDR, 4, (unsigned char *)(&u32_read), g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
			DEBUGOUT("Data RAM Test NG !!!\r\n");
			return ERROR;
		}

		u32_read &= ~(1 << 29);
		if (handle_ic_write(REG_SYSCON_BLKEN_ADDR, 4, (unsigned char *)&u32_read, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
			DEBUGOUT("Data RAM Test NG !!!\r\n");
			return ERROR;
		}
	}
	return SUCCESS;
}

STATUS ft_test_ram_test_2x(unsigned char u8_is_stop_mcu)
{
	unsigned int u32_read = 0;
	unsigned int u32_write = 0;
	unsigned char u8_retry_times = 2;

	if ((g_st_test_info.u16_ft_test_info_1) == WEARBLE_FT_PRAM_SYSTEM_NG_CASE) {
		g_u32_wearable_test_result = WEARABLE_FT_TEST_RESULT_PRAM_NG;
		return ERROR;
	}

RETRY_RAM_TEST:

	if (u8_is_stop_mcu) {
		stop_mcu_2x(TRUE);
	}

	if (ft_test_ctrl_mbist_fun_2x(ENABLE) == ERROR) {
		goto EXIT_ERROR;
	}

	u32_write = 0x04000080;
	if (handle_ic_write(RAM_WRITE_TEST_ADDR1, 4, (unsigned char *)&u32_write, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		goto EXIT_ERROR;
	}

	u32_write = 0x00FE90FE;
	if (handle_ic_write(RAM_WRITE_TEST_ADDR2, 4, (unsigned char *)&u32_write, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		goto EXIT_ERROR;
	}

	u32_write = 0x0A800080;
	if (handle_ic_write(RAM_WRITE_TEST_ADDR3, 4, (unsigned char *)&u32_write, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		goto EXIT_ERROR;
	}

	delay_ms(5);

	if (handle_ic_read(RAM_READ_TEST_ADDR1, 4, (unsigned char *)(&u32_read), g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		goto EXIT_ERROR;
	}

	if ((u32_read & 0xFE00) != 0) {
		DEBUGOUT("RAM Test NG 954[0x%x]!!!\r\n", u32_read);
		goto EXIT_ERROR;
	}

	if (handle_ic_read(RAM_READ_TEST_ADDR2, 4, (unsigned char *)(&u32_read), g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		goto EXIT_ERROR;
	}

	if (u32_read != 0x19007F00) {
		DEBUGOUT("RAM Test NG B04[0x%x]!!!\r\n", u32_read);
		goto EXIT_ERROR;
	}

	if (handle_ic_read(RAM_READ_TEST_ADDR3, 4, (unsigned char *)(&u32_read), g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		goto EXIT_ERROR;
	}

	if (u32_read != 0x40007F00) {
		DEBUGOUT("RAM Test NG B08[0x%x]!!!\r\n", u32_read);
		goto EXIT_ERROR;
	}

	if (ft_test_ctrl_mbist_fun_2x(DISABLE) == ERROR) {
		goto EXIT_ERROR;
	} else {
		DEBUGOUT("RAM Test Pass\r\n");
		return SUCCESS;
	}

EXIT_ERROR:
	if ((u8_retry_times-- > 0)) {
		goto RETRY_RAM_TEST;
	}

	ft_test_ctrl_mbist_fun_2x(DISABLE);
	g_u32_wearable_test_result = WEARABLE_FT_TEST_RESULT_PRAM_NG;
	DEBUGOUT("PRAM Test NG !!!\r\n");
	return ERROR;
}

STATUS ft_test_connect_test_2x(void)
{
	unsigned char u8_retry = IC_TEST_RETRY_TIME;
	unsigned int u32_w_buf, u32_r_buf;
	unsigned int u32_test_addr = RM_DATAMEM0_BASE;
	char i8_str[8];

	if ((g_st_test_info.u16_ft_test_info_1) == WEARBLE_FT_I2C_SYSTEM_NG_CASE) {
		g_u32_wearable_test_result = WEARABLE_FT_TEST_RESULT_I2C_NG;
		return ERROR;
	}

RETRY_CONNECT_TEST:

	if (g_u8_drv_interface == SPI_INTERFACE)
		sprintf(i8_str, "SPI");
	else
		sprintf(i8_str, "I2C");

	u32_w_buf = 0x55aa00ff;

	if (handle_ic_write(u32_test_addr, 4, (unsigned char *)&u32_w_buf, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		DEBUGOUT("%s Test Write NG!!\r\n", i8_str);
		goto NG_CASE;
	}

	if (handle_ic_read(u32_test_addr, 4, (unsigned char *)&u32_r_buf, g_u8_drv_interface, I2C_BYTE_MODE) == ERROR) {
		DEBUGOUT("%s Test Read NG!!\r\n", i8_str);
		goto NG_CASE;
	}


	if (u32_w_buf != u32_r_buf) {
		DEBUGOUT("%s Test Compare NG [%d], W=0x%x,R=0x%x\r\n", i8_str, 0, u32_w_buf, u32_r_buf);
		goto NG_CASE;
	}


#if ENABLE_WDT
	Chip_WWDT_Feed(LPC_WWDT);// reload the Watchdog timer
#endif


	DEBUGOUT("%s Test Pass\r\n", i8_str);
	return SUCCESS;

NG_CASE:
	if (--u8_retry > 0) {
		DEBUGOUT("%s Test Retry=%d\r\n", i8_str, u8_retry);
		if (hardware_reset_2x(TRUE) == ERROR)
			goto NG_CASE2;
		//Stop MCU
		stop_mcu_2x(FALSE);
		goto RETRY_CONNECT_TEST;
	}

NG_CASE2:
	g_u32_wearable_test_result = WEARABLE_FT_TEST_RESULT_I2C_NG;
	DEBUGOUT("%s Test NG !!!\r\n", i8_str);
	return ERROR;
}

STATUS ft_test_reset_pin_test_2x()
{
	unsigned int u32_write = 0;

	if ((g_st_test_info.u16_ft_test_info_1) == WEARBLE_FT_RESET_PIN_SYSTEM_NG_CASE) {
		g_u32_wearable_test_result = WEARABLE_FT_TEST_RESULT_RESET_NG;
		return ERROR;
	}

	u32_write = 0x00000404;
	if (handle_ic_write(REG_SYSCON_MISCIER_ADDR, 4, (unsigned char *)&u32_write, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}

	if (!gpio_touch_int_pin_state_access()) { // check INT in high state
		g_u32_wearable_test_result = WEARABLE_FT_TEST_RESULT_INT_NG;
		return ERROR;
	}
	gpio_touch_reset_pin_control(FALSE);//Low
	delay_ms(1);
	if (gpio_touch_int_pin_state_access()) {
		g_u32_wearable_test_result = WEARABLE_FT_TEST_RESULT_RESET_NG;
		DEBUGOUT("Reset Pin Test NG(Not High) !!!\r\n");
		return ERROR;
	}
	gpio_touch_reset_pin_control(TRUE);//High
	delay_ms(25);
	DEBUGOUT("Reset Pin Test Pass !!!\r\n");
	return SUCCESS;
}

STATUS ft_test_panel_model_check_2x(unsigned short u16_version)
{
	unsigned int u32_read;
	unsigned short u16_model_version;

	if (handle_ic_read(FW_FT_SRAM_FW_VERSION, 4, (unsigned char *)(&u32_read), g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}
	u16_model_version = (u32_read & 0x0000FFFF);
	DEBUGOUT("Panel Model Test! 0x%08X:0x%08X\r\n", u32_read, u16_version);
	DEBUGOUT("Panel Model 0x%08X\r\n", u16_model_version);
	if (u16_model_version == u16_version) {
		return SUCCESS;
	} else {
		DEBUGOUT("Panel Model NG! 0x%x:0x%x\r\n", u16_model_version, u16_version);
	}
	return ERROR;
}

void ft_raw_data_checksum_cal_2x(unsigned short *u16_buffer)
{
	unsigned char u8_i;

	u16_buffer[34] = 0x55AA;
	u16_buffer[35] = 0;
	for (u8_i = 0; u8_i < (MAX_IMAGE_BUFFER_SIZE - 1); u8_i++) {
		u16_buffer[35] += u16_buffer[u8_i];
	}

	DEBUGOUT("[RDCSA] %x:%x\r\n", u16_buffer[34], u16_buffer[35]);
}

void ft_test_result_checksum_cal_2x(unsigned int *u32_buffer)
{
	unsigned char u8_i;
	u32_buffer[34] = 0x55AA00FF;
	u32_buffer[35] = 0;
	for (u8_i = 0; u8_i < (MAX_IMAGE_BUFFER_SIZE - 1); u8_i++) {
		u32_buffer[35] += u32_buffer[u8_i];
	}

	DEBUGOUT("[TRCSA] %x:%x\r\n", u32_buffer[34], u32_buffer[35]);
}

STATUS ft_raw_data_checksum_check_2x(unsigned short *u16_buffer)
{

	unsigned char u8_i;
	unsigned short u16_sum = 0;

	if (u16_buffer[34] != 0x55AA) {
		DEBUGOUT("u16_buffer[34]:%x\r\n", u16_buffer[34]);
		DEBUGOUT("[RDCSC] Pattern NG! [0x%x](0x%x)\r\n", (int)u16_buffer, u16_buffer[34]);
		return ERROR;
	}

	//u16_buffer[35]=0;
	for (u8_i = 0; u8_i < (MAX_IMAGE_BUFFER_SIZE - 1); u8_i++) {
		u16_sum += u16_buffer[u8_i];
	}

	if (u16_buffer[35] != u16_sum) {
		DEBUGOUT("[RDCSC] Check SUM NG! [0x%x](0x%x:0x%x)\r\n", (int)u16_buffer, u16_sum, u16_buffer[35]);
		return ERROR;
	}

	DEBUGOUT("[RDCSC] PASS (0x%x:0x%x)\r\n", u16_buffer[34], u16_buffer[35]);

	return SUCCESS;
}

STATUS ft_test_result_checksum_check_2x(unsigned int *u32_buffer)
{

	unsigned char u8_i;
	unsigned int u32_sum = 0;

	if (u32_buffer[34] != 0x55AA00FF) {
		DEBUGOUT("[TRCSC] Pattern NG! [0x%x](0x%x:0x%x)\r\n", (int)u32_buffer, u32_buffer[34], u32_buffer[35]);
		return ERROR;
	}

	//u32_buffer[35]=0;
	for (u8_i = 0; u8_i < (MAX_IMAGE_BUFFER_SIZE - 1); u8_i++) {
		u32_sum += u32_buffer[u8_i];
	}

	if (u32_buffer[35] != u32_sum) {
		DEBUGOUT("[TRCSC] Check SUM NG! [0x%x](0x%x:0x%x)\r\n", (int)u32_buffer, u32_sum, u32_buffer[35]);
		return ERROR;
	}

	DEBUGOUT("[TRCSC] PASS (0x%x:0x%x)\r\n", u32_buffer[34], u32_buffer[35]);

	return SUCCESS;
}

STATUS baseline_update_control_2x(bool b_enable_baseline_update)
{
	unsigned int u32_write_buf;
	unsigned char u8_read;
	short i16_time_out = 100;

	if (b_enable_baseline_update) {
		u32_write_buf = (SYS_CMD_FUNC_DIS_BS_UPDATE);
		DEBUGOUT("[BUC] Enable Baseline update\r\n");
	} else {
		u32_write_buf = (DIS_BASELINE_UPDATE | SYS_CMD_FUNC_DIS_BS_UPDATE);
		DEBUGOUT("[BUC] Disable Baseline update\r\n");
	}

	if (handle_ic_write(FW_SYS_CMD_ADDR, 4, (unsigned char *)&u32_write_buf, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}
	delay_ms(5);
	while (i16_time_out--) {
		if (handle_ic_read(FW_SYS_CMD_ADDR, 1, &u8_read, g_u8_drv_interface, I2C_BYTE_MODE) == SUCCESS) {
			if (u8_read == 0x00) {
				break;
			}
		} else {
			DEBUGOUT("[BUC] I2C Read NG\r\n");
			return ERROR;
		}
		delay_ms(2);
	}

	if (i16_time_out == IC_TEST_TIME_OUT) {
		DEBUGOUT("[BUC] Baseline Update Control (%d) NG\r\n", b_enable_baseline_update);
		return ERROR;
	}

	return SUCCESS;
}

STATUS wait_fw_init_ready_2x(void)
{
	unsigned short u16_time_out;
	unsigned char u8_r_buf[2];

	u16_time_out = 100;
	//Check FW Ready ?
	do {
		if (handle_ic_read(FW_FT_ARG0_ADDR, 2, u8_r_buf, g_u8_drv_interface, I2C_BYTE_MODE) == ERROR) {
			return ERROR;
		} else {
			DEBUGOUT("FW Flag = 0x%x,0x%x\r\n", u8_r_buf[0], u8_r_buf[1]);
			if (u8_r_buf[0] == 0xAA
			    && u8_r_buf[1] == 0x55) {
				return SUCCESS;
			}
			delay_ms(2);
		}

	} while (u16_time_out-- > 0);

	return ERROR;
}


STATUS enter_normal_fw_2x(void)
{
	unsigned char u8_retry = IC_TEST_RETRY_TIME;
#if ENABLE_TEST_TIME_MEASURMENT
	unsigned int u32_fun_time = 0;
	u32_fun_time = get_system_time();
#endif

	if (g_u8_is_normal_fw)
		return SUCCESS;

	DEBUGOUT("[EFNM] Start\r\n");
	g_u32_wearable_test_result &= ~(WEARABLE_FT_TEST_RESULT_NORMAL_FW_NG);

RETRY:
#if !SELFTEST
	//Set INT Falling Triggle
	gpio_touch_int_trigger_control(FALSE);
#endif

	g_u32_wearable_test_result &= ~WEARABLE_FT_TEST_RESULT_NORMAL_FW_RESET_NG;
//	if (g_u8_drv_interface == SPI_INTERFACE) {
//		if (hardware_reset_2x(TRUE) == ERROR) {
//			DEBUGOUT("[ENF]No INT, no OTP!!\r\n");
//			goto ERROR_EXIT;
//		}

//#if !SELFTEST
//		if (burn_pram_from_dongle_flash_2x() == ERROR) {
//			goto ERROR_EXIT;
//		}
//#endif
//	} else {
	if (hardware_reset_2x(FALSE) == ERROR) {
		DEBUGOUT("[ENF]No INT, no OTP!!\r\n");
		goto ERROR_EXIT;
	}
//	}

	if (wait_fw_init_ready_2x() == ERROR) {
		goto ERROR_EXIT;
	}

	if (ft_test_read_used_pin_infor_2x(g_u8_wearable_pin_map) == ERROR) {
		goto ERROR_EXIT;
	}

#if ENABLE_TEST_TIME_MEASURMENT
	DEBUGOUT("[ENF] (%d)\r\n", get_system_time() - u32_fun_time);
#endif

	g_u8_is_normal_fw = TRUE;
	return SUCCESS;

ERROR_EXIT:
	g_u32_wearable_test_result |= WEARABLE_FT_TEST_RESULT_NORMAL_FW_RESET_NG;
	if (u8_retry--) {
		goto RETRY;
	}

	return ERROR;
}

STATUS check_cc_bl_flag_2x(void)
{
	unsigned char u8_read_buf[4];
	unsigned char u8_is_cc_ready = 0, u8_is_bl_ready = 0;
	short i16_time_out;

	DEBUGOUT("[CCBF] Check CC BL Flag\r\n");

	i16_time_out = 20;
	while (i16_time_out--) {
		if (!u8_is_cc_ready) {
			if (handle_ic_read(PRAM_ADDR_CC_INFO, 4, u8_read_buf, g_u8_drv_interface, I2C_BYTE_MODE) == ERROR)
				return ERROR;
			if (u8_read_buf[0] >= 1) {
				u8_is_cc_ready = 1;
			}
		}

		if (!u8_is_bl_ready) {
			if (handle_ic_read(PRAM_ADDR_BL_INFO, 4, u8_read_buf, g_u8_drv_interface, I2C_BYTE_MODE) == ERROR)
				return ERROR;
			if (u8_read_buf[0] >= 1) {
				u8_is_bl_ready = 1;
			}
		}

		if (u8_is_cc_ready && u8_is_bl_ready)
			break;
		delay_ms(1);
	}

	//Check flag
	if (i16_time_out == IC_TEST_TIME_OUT) {
		DEBUGOUT("CC Flag=%d,BL Flag=%d NG!!\r\n", u8_is_cc_ready, u8_is_bl_ready);
		return ERROR;
	}

	return SUCCESS;
}

STATUS burn_cc_to_ic_flash_2x()
{
	short i16_time_out = 1000;
	unsigned int u32_cc_table;
	unsigned char u8_fw_value[4];
	unsigned int u32_write = 0;
#if SELFTEST
	unsigned int u32_read;
#endif

#if ENABLE_TEST_TIME_MEASURMENT_CC
	DEBUGOUT("[BCTF] Before Switch to Bootloader\r\n");
	g_u32_spend_time = get_system_time();
#endif

	//Set Skip_Load = 1
	u32_write = BL_CRC_CHK | SKIP_LOAD;
	if (handle_ic_write(REG_FLASHCTL_FLASH_STATE_REG_ADDR, 4, (unsigned char *)&u32_write, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}

	//wait sw rst finish
	u32_write = BLKRST_SW_RST;
	if (handle_ic_write(REG_SYSCON_BLKRST_ADDR, 4, (unsigned char *)&u32_write, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}
	delay_ms(2);

	if (wait_fw_state_2x(BOOT_RET_DATA_ADDR, WAIT_TEST_MODE, 1, i16_time_out) == ERROR) {
		DEBUGOUT("[BCTF] Check Burn CC & BL Fail\r\n");
		return ERROR;
	}

#if ENABLE_TEST_TIME_MEASURMENT_CC
	DEBUGOUT("[BCTF] End of Switch to Bootloader = %d\r\n", get_system_time() - g_u32_spend_time);
	g_u32_spend_time = get_system_time();
	DEBUGOUT("TICK=%d\r\n", get_system_time());
#endif

#if SELFTEST
	if (sysfs_burn_cc_bl()) {
		//Read Flash CC Table
		if (read_fpc_flash_2x(FLASH_NORMAL_FW_CC_TABLE_ADDR, &u32_cc_table) == ERROR) {
			return ERROR;
		}
		DEBUGOUT("Flash Do CC FW Version=0x%04X\r\n", (u32_cc_table & 0xFFFF0000) >> 16);
		DEBUGOUT("Flash Do CC Flag=0x%X\r\n", (u32_cc_table & 0x0000FFFF));
		g_u32_fw_cc_version = u32_cc_table;

		//Read PRAM FW Version
		if (handle_ic_read(FW_FT_SRAM_FW_VERSION, 4, u8_fw_value, g_u8_drv_interface, I2C_BYTE_MODE) == ERROR) {
			return ERROR;
		}
		DEBUGOUT("SRAM FW Version=0x%02X,0x%02X\r\n", u8_fw_value[3], u8_fw_value[2]);

		if (((u32_cc_table & 0xFF000000) >> 24) != u8_fw_value[3] || ((u32_cc_table & 0x00FF0000) >> 16) != u8_fw_value[2]) {
			DEBUGOUT("Flash CC Table FW Version is not match!\r\n");
			if (handle_ic_read(REG_FLASHCTL_FLASH_STATE_REG_ADDR, 4, (unsigned char *)(&u32_read), g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
				DEBUGOUT("read 5000 0918 error\r\n");
			}
			DEBUGOUT("Flash 0x50000918=0x%08X\r\n", u32_read);
			return ERROR;
		}

		return SUCCESS;
	} else
		return ERROR;
#else
	if (burn_to_ic_flash_2x(COMP_AREA, 1) == ERROR) {
		DEBUGOUT("[BCTF] Burn CC & BL Fail\r\n");
		return ERROR;
	}

	u32_write = 0x00000000;
	if (handle_ic_write(BOOT_TEST_MODE_ADDR, 4, (unsigned char *)&u32_write, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}

	// clr sync_data(0x20000200) = 0 as finish
	u32_write = 0x00000000;
	if (handle_ic_write(BOOT_SYNC_DATA_ADDR, 4, (unsigned char *)&u32_write, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}
	delay_ms(10);

	if (wait_fw_state_2x(REG_FLASHCTL_FLASH_STATE_REG_ADDR, 0x4000, 2, i16_time_out) == ERROR) {
		DEBUGOUT("[BCTF] Check Burn CC & BL Fail\r\n");
		return ERROR;
	}

#if ENABLE_AUO_VERIFY_LOG
	if (read_fpc_flash_2x((unsigned int)(FLASH_FW_START + PRAM_FW_LENGTH - 4), &u32_i2c_read_data)) {
		DEBUGOUT("Normal FW CRC=0x%08X\r\n", u32_i2c_read_data);
	}
#endif

#if ENABLE_TEST_TIME_MEASURMENT_CC
	DEBUGOUT("BCTF] Burn to flash finish[Tick=%d]!\r\n", get_system_time() - g_u32_spend_time);
#endif

	//Read Flash CC Table
	if (read_fpc_flash_2x(FLASH_NORMAL_FW_CC_TABLE_ADDR, &u32_cc_table) == ERROR) {
		return ERROR;
	}
	DEBUGOUT("Flash Do CC FW Version=0x%04X\r\n", (u32_cc_table & 0xFFFF0000) >> 16);
	DEBUGOUT("Flash Do CC Flag=0x%X\r\n", (u32_cc_table & 0x0000FFFF));

	//Read PRAM FW Version
	if (handle_ic_read(FW_FT_SRAM_FW_VERSION, 4, u8_fw_value, g_u8_drv_interface, I2C_BYTE_MODE) == ERROR) {
		return ERROR;
	}
	DEBUGOUT("SRAM FW Version=0x%02X,0x%02X\r\n", u8_fw_value[3], u8_fw_value[2]);

	if (((u32_cc_table & 0xFF000000) >> 24) != u8_fw_value[3] || ((u32_cc_table & 0x00FF0000) >> 16) != u8_fw_value[2]) {
		DEBUGOUT("Flash CC Table FW Version is not match!\r\n");
		return ERROR;
	}

	return SUCCESS;
#endif
}

STATUS do_calibration_2x(
	unsigned char u8_do_calibration_cmd,
	unsigned char u8_burn_flash)
{
	unsigned char u8_value[4];
	short u8_time_out = 100;
	unsigned char u8_retry = IC_TEST_RETRY_TIME;

	g_u32_wearable_test_result &= ~(WEARABLE_FT_TEST_RESULT_CC_CALIBRATION_NG | WEARABLE_FT_TEST_RESULT_BURN_CC_NG);

	if (u8_do_calibration_cmd == TRUE) {

#if ENABLE_TEST_TIME_MEASURMENT_CC
		DEBUGOUT("[DOCC] Start do calibration (%d)\r\n", get_system_time());
		g_u32_spend_time = get_system_time();
#endif
		while (u8_retry--) {
			//Trigger Calibration
			u8_value[0] = SYS_CMD_DO_CC_CAL;
			if (handle_ic_write(FW_SYS_CMD_ADDR, 1, u8_value, g_u8_drv_interface, I2C_BYTE_MODE) == ERROR) {
				return ERROR;
			}

			u8_time_out = 200;

			// Wait calibration ready
			while (u8_time_out--) {
				delay_ms(10);//20
				if (handle_ic_read(FW_SYS_CMD_ADDR, 4, u8_value, g_u8_drv_interface, I2C_BYTE_MODE) == ERROR) {
					return ERROR;
				} else if (u8_value[0] == 0) {
					break;
				}
#if ENABLE_WDT
				Chip_WWDT_Feed(LPC_WWDT);// reload the Watchdog timer
#endif
			}
			if (u8_time_out == IC_TEST_TIME_OUT) {
				if (u8_retry) {
					DEBUGOUT("[DOCC] Do CC Retry (%d)\r\n", u8_retry);
					hardware_reset_2x(TRUE);
					delay_ms(20);
					continue;
				} else {
					g_u32_wearable_test_result |= WEARABLE_FT_TEST_RESULT_CC_CALIBRATION_NG;
					DEBUGOUT("[DOCC] SYS CMD CC NG\r\n");
					return ERROR;
				}
			} else {
				if (check_cc_bl_flag_2x() == ERROR) {
					if (u8_retry) {
						DEBUGOUT("[DOCC] CC Flag Retry (%d)", u8_retry);
						hardware_reset_2x(TRUE);
						delay_ms(20);
						continue;
					} else {
						g_u32_wearable_test_result |= WEARABLE_FT_TEST_RESULT_CC_CALIBRATION_NG;
						DEBUGOUT("[DOCC] CC Flag NG\r\n");
						return ERROR;
					}
				} else //Calibration OK
					break;
			}
		}
#if ENABLE_TEST_TIME_MEASURMENT_CC
		DEBUGOUT("[DOCC] Do CC Finish! [%d:%d]\r\n", get_system_time() - g_u32_spend_time, u8_time_out);
		g_u32_spend_time = get_system_time();
#endif
	}

	if (u8_burn_flash == FALSE) {
		DEBUGOUT("[DOCC] Do Calibration Finish \r\n");
		return SUCCESS;
	}

	if (burn_cc_to_ic_flash_2x() == ERROR) {
		DEBUGOUT("[DOCC] Burn to flash NG!\r\n");
		hardware_reset_2x(TRUE);
		g_u32_wearable_test_result |= WEARABLE_FT_TEST_RESULT_BURN_CC_NG;
		return ERROR;
	}

#if ENABLE_TEST_TIME_MEASURMENT_CC
	DEBUGOUT("[DOCC] Burn CC Finish (%d)\r\n", get_system_time() - g_u32_spend_time);
#endif

	DEBUGOUT("[DOCC] Burn CC Finish \r\n");
	return SUCCESS;
}

STATUS hw_int_pin_Test_2x(void)
{
	unsigned int u32_write = 0;

	if ((g_st_test_info.u16_ft_test_info_1) == WEARBLE_FT_INT_SYSTEM_NG_CASE) {
		g_u32_wearable_test_result = WEARABLE_FT_TEST_RESULT_INT_NG;
		return ERROR;
	}

	//Trigger INT to Low
	u32_write = 0x00000004;
	if (handle_ic_write(REG_SYSCON_MISCIER_ADDR, 4, (unsigned char *)&u32_write, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}

	//Read Pin state
	if (!gpio_touch_int_pin_state_access()) {
		//DEBUGOUT("INT is LOW\r\n");
	} else {
		DEBUGOUT("INT Test NG!\r\n");
		g_u32_wearable_test_result = WEARABLE_FT_TEST_RESULT_INT_NG;
		return ERROR;
	}

	//Trigger INT to High
	u32_write = 0x00000404;
	if (handle_ic_write(REG_SYSCON_MISCIER_ADDR, 4, (unsigned char *)&u32_write, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}

	//Read Pin state
	if (gpio_touch_int_pin_state_access()) {
		//DEBUGOUT("INT is High\r\n");
	} else {
		DEBUGOUT("INT Test NG!\r\n");
		g_u32_wearable_test_result = WEARABLE_FT_TEST_RESULT_INT_NG;
		return ERROR;
	}
	//gpio_touch_int_access(TRUE);
	DEBUGOUT("INT Test Pass\r\n");
	return SUCCESS;
}

STATUS read_test_fw_data_2x(unsigned short u16_test_items)
{

	unsigned char u8_retry = IC_TEST_RETRY_TIME;
	unsigned int u32_test_item_result;
#if ENABLE_TEST_TIME_MEASURMENT
	unsigned int u32_fun_time = 0;
	u32_fun_time = get_system_time();
#endif


	g_u32_wearable_test_result &= ~(WEARABLE_FT_TEST_RESULT_GET_DATA_NG | WEARABLE_FT_TEST_RESULT_OPEN_NG
					| WEARABLE_FT_TEST_RESULT_SHORT_NG | WEARABLE_FT_TEST_RESULT_UB_NG
					| WEARABLE_FT_TEST_RESULT_UC_NG | WEARABLE_FT_TEST_RESULT_SINGLE_CC_OPEN_NG
					| WEARABLE_FT_TEST_RESULT_SINGLE_CC_SHORT_NG);
	if ((g_st_test_info.u16_ft_test_item & (IC_TEST_ITEMS_OPEN | IC_TEST_ITEMS_SHORT |
						IC_TEST_ITEMS_UC | IC_TEST_ITEMS_UB)) == 0) {
		return SUCCESS;
	}

RETRY:
	memset(g_i16_raw_data_1_short_buf, 0 , sizeof(g_i16_raw_data_1_short_buf));
	memset(g_i16_raw_data_2_open_buf, 0 , sizeof(g_i16_raw_data_2_open_buf));
	memset(g_u16_raw_data3_cc_buf, 0 , sizeof(g_u16_raw_data3_cc_buf));
	memset(g_i16_ub_buf, 0 , sizeof(g_i16_ub_buf));
	memset(g_u16_uc_buf, 0 , sizeof(g_u16_uc_buf));
	memset(g_i16_open_bl_data, 0 , sizeof(g_i16_open_bl_data));
	memset(g_u8_wearable_pin_map, 0 , sizeof(g_u8_wearable_pin_map));

	if (handle_ic_read(FT_RAWDATA1_SHORT_BUF_ADDR, 72, (unsigned char *)g_i16_raw_data_1_short_buf, g_u8_drv_interface, I2C_BYTE_MODE) == ERROR) {
		goto ERROR_EXIT;
	}

	if (handle_ic_read(FT_RAWDATA2_OPEN_BUF_ADDR, 72, (unsigned char *)g_i16_raw_data_2_open_buf, g_u8_drv_interface, I2C_BYTE_MODE) == ERROR) {
		goto ERROR_EXIT;
	}

	if (handle_ic_read(FT_RAWDATA3_CC_BUF_ADDR, 72, (unsigned char *)(g_u16_raw_data3_cc_buf), g_u8_drv_interface, I2C_BYTE_MODE) == ERROR) {
		goto ERROR_EXIT;
	}

	if (handle_ic_read(FT_UC_BUF_ADDR, 72, (unsigned char *)g_u16_uc_buf, g_u8_drv_interface, I2C_BYTE_MODE) == ERROR) {
		goto ERROR_EXIT;
	}

	if (handle_ic_read(FT_UB_BUF_ADDR, 72, (unsigned char *)g_i16_ub_buf, g_u8_drv_interface, I2C_BYTE_MODE) == ERROR) {
		goto ERROR_EXIT;
	}

	if (handle_ic_read(FT_TEST_RESULT_BUF_ADDR, 72, (unsigned char *)(g_u32_test_result), g_u8_drv_interface, I2C_BYTE_MODE) == ERROR) {
		goto ERROR_EXIT;
	}

	if (handle_ic_read(FT_TEST_RESULT_BUF_ADDR + 72, 72, (unsigned char *)(g_u32_test_result) + 72, g_u8_drv_interface, I2C_BYTE_MODE) == ERROR) {
		goto ERROR_EXIT;
	}

	if (handle_ic_read(FT_TEST_ITEM_RESULT, 4, (unsigned char *)(&u32_test_item_result), g_u8_drv_interface, I2C_BYTE_MODE) == ERROR) {
		goto ERROR_EXIT;
	}

	if (handle_ic_read(FT_OPEN_BL_BUF_ADDR, 72, (unsigned char *)g_i16_open_bl_data, g_u8_drv_interface, I2C_BYTE_MODE) == ERROR) {
		goto ERROR_EXIT;
	}

	if (ft_raw_data_checksum_check_2x((unsigned short *)g_i16_raw_data_1_short_buf) == ERROR) {
		goto ERROR_EXIT;
	}

	if (ft_raw_data_checksum_check_2x((unsigned short *)g_i16_raw_data_2_open_buf) == ERROR) {
		goto ERROR_EXIT;
	}

	if (ft_raw_data_checksum_check_2x((unsigned short *)g_u16_raw_data3_cc_buf) == ERROR) {
		goto ERROR_EXIT;
	}

	if (ft_raw_data_checksum_check_2x(g_u16_uc_buf) == ERROR) {
		goto ERROR_EXIT;
	}

	if (ft_raw_data_checksum_check_2x((unsigned short *)g_i16_ub_buf) == ERROR) {
		goto ERROR_EXIT;
	}

	if (ft_raw_data_checksum_check_2x((unsigned short *)g_i16_open_bl_data) == ERROR) {
		goto ERROR_EXIT;
	}

	if (ft_test_result_checksum_check_2x(g_u32_test_result) == ERROR) {
		goto ERROR_EXIT;
	}

	if (ft_test_result_checksum_check_2x(g_u32_test_result) == ERROR) {
		goto ERROR_EXIT;
	}

	if (ft_test_read_used_pin_infor_2x(g_u8_wearable_pin_map) == ERROR) {
		goto ERROR_EXIT;
	}

#if ENABLE_TEST_RSU_DATA_SHOW
	DEBUGOUT("Slow Data:\r\n");
	dump_image_data_2x(g_i16_raw_data_1_short_buf, TRUE);
	DEBUGOUT("Quick Data:\r\n");
	dump_image_data_2x(g_i16_raw_data_2_open_buf, TRUE);
	DEBUGOUT("P CC:\r\n");
	dump_image_hex_data_2x((short *)g_u16_raw_data3_cc_buf);
	DEBUGOUT("Open Golden CC:\r\n");
	dump_image_hex_data_2x((short *)g_u16_raw_data3_golden_cc_buf);
	DEBUGOUT("fw cc:\r\n");
	dump_image_data_2x((short *)g_u16_uc_golden_cc_buf, TRUE);
	DEBUGOUT("test item result:0x%x\r\n", u32_test_item_result);
#endif

	if (g_u8_print_debug_msg & PRINT_DEBUG_MSG_TYPE_4) {
		DEBUGOUT("Slow Data:\r\n");
		dump_image_data_2x(g_i16_raw_data_1_short_buf, TRUE);
		DEBUGOUT("Quick Data:\r\n");
		dump_image_data_2x(g_i16_raw_data_2_open_buf, TRUE);
		DEBUGOUT("P CC:\r\n");
		dump_image_data_2x((short *)g_u16_raw_data3_cc_buf , TRUE);

	}
	if (g_u8_print_debug_msg & PRINT_DEBUG_MSG_TYPE_5) {
		DEBUGOUT("UC Data:\r\n");
		dump_image_data_2x((short *)g_u16_uc_buf, TRUE);
		DEBUGOUT("UB Data:\r\n");
		dump_image_data_2x((short *)g_i16_ub_buf, TRUE);
	}

#if ENABLE_TEST_TIME_MEASURMENT
	DEBUGOUT("#read_test_data end=%d\r\n", get_system_time() - u32_fun_time);
#endif
	g_u32_wearable_test_result |= u32_test_item_result;
	DEBUGOUT("Read Test FW Result Finish\r\n");
	return SUCCESS;

ERROR_EXIT:
	if (u8_retry) {
		u8_retry--;
		DEBUGOUT("Read Test FW Result Retry:%d", u8_retry);
		goto RETRY;
	}

	g_u32_wearable_test_result |= WEARABLE_FT_TEST_RESULT_GET_DATA_NG;
	DEBUGOUT("Read Test FW Result NG\r\n");
	return ERROR;
}

void test_item_message_2x(void)
{
	unsigned short u16_test_item = g_st_test_info.u16_ft_test_item;
	DEBUGOUT("================================\r\n");
	DEBUGOUT("Enter 2x IC Test,(%d)\r\n", get_system_time());
	DEBUGOUT("IC Test Items=0x%X\r\n", u16_test_item);

	if (u16_test_item & IC_TEST_ITEMS_SYSTEM) {
		DEBUGOUT("Enable System Test\r\n");
	}

	if (u16_test_item & IC_TEST_ITEMS_OPEN) {
		DEBUGOUT("Enable Open Test\r\n");
	}

	if (u16_test_item & IC_TEST_ITEMS_SHORT) {
		DEBUGOUT("Enable Short Test\r\n");
	}

	if (u16_test_item & IC_TEST_ITEMS_UC) {
		DEBUGOUT("Enable Uniformity CC test\r\n");
	}

	if (u16_test_item & IC_TEST_ITEMS_UB) {
		DEBUGOUT("Enable Uniformity BL test\r\n");
	}

	if (u16_test_item & IC_TEST_ITEMS_BURN_FW) {
		DEBUGOUT("Enable Burn FW\r\n");
	}

	if (u16_test_item & IC_TEST_ITEMS_BURN_CC) {
		DEBUGOUT("Enable Burn CC\r\n");
	}
}

STATUS burn_cc_2x(unsigned short u16_test_items)
{
#if ENABLE_AUO_VERIFY_LOG
	unsigned int u32_flash_crc = 0;
#endif
#if ENABLE_TEST_TIME_MEASURMENT
	unsigned int u32_fun_time = 0;
	u32_fun_time = get_system_time();
#endif
	g_u32_wearable_test_result &= ~(WEARABLE_FT_TEST_RESULT_BURN_CC_NG | WEARABLE_FT_TEST_RESULT_CC_CALIBRATION_NG);

	if ((g_st_test_info.u16_ft_test_info_1) == WEARBLE_FT_BURN_CC_SYSTEM_NG_CASE) {
		g_u32_wearable_test_result = WEARABLE_FT_TEST_RESULT_BURN_CC_NG;
		return ERROR;
	}

	if ((g_st_test_info.u16_ft_test_item & (IC_TEST_ITEMS_BURN_CC)) == 0) {
		return SUCCESS;
	}

	if (enter_normal_fw_2x() == ERROR) {
		g_u32_wearable_test_result |= WEARABLE_FT_TEST_RESULT_NORMAL_FW_NG;
		DEBUGOUT("Enter Normal FW NG\r\n");
		return ERROR;
	}

	//Do CC
	DEBUGOUT("[BC] Burn CC Start!!\r\n");
	if (g_u8_drv_interface == SPI_INTERFACE) {
		//flashless, bypass
		DEBUGOUT("[BC]Flashless Burn CC ByPass!!\r\n");
		return SUCCESS;
	}

	if (do_calibration_2x(TRUE, TRUE) == SUCCESS) {
		DEBUGOUT("[BC] Burn CC Pass!!\r\n");
	} else {
		DEBUGOUT("[BC] Burn CC NG!!\r\n");
		return ERROR;
	}

#if ENABLE_TEST_TIME_MEASURMENT
	DEBUGOUT("[BC] Burn CC Time(%d)\r\n", get_system_time() - u32_fun_time);
#endif

	return SUCCESS;
}

/*Use dongle ext.Flash to read test fw and write to pram*/
STATUS load_test_fw_2x(void)
{

	STATUS u8_ret = ERROR;
#if ENABLE_TEST_TIME_MEASURMENT
	unsigned int u32_fun_time = 0;
	u32_fun_time = get_system_time();
	DEBUGOUT("load_test_fw Start Time= %d\r\n", u32_fun_time);
#endif
	DEBUGOUT("[LTFW] start load test fw\r\n");
	g_u32_wearable_test_result &= ~(WEARABLE_FT_TEST_RESULT_LOAD_TESTFW_NG);

	if (g_st_test_info.u16_ft_test_info_1 == WEARBLE_FT_LOAD_TEST_FW_SYSTEM_NG_CASE)
		return ERROR;

	if ((g_st_test_info.u16_ft_test_item & (IC_TEST_ITEMS_OPEN | IC_TEST_ITEMS_SHORT | IC_TEST_ITEMS_UC |
						IC_TEST_ITEMS_UB)) == 0) {
		return SUCCESS;
	}

#if SELFTEST
	u8_ret = raydium_upgrade_test_fw_2x(0);
#else
	u8_ret = load_test_fw_ft_2x();
#endif
	DEBUGOUT("[LTFW] Load test fw finish!!\r\n");
#if ENABLE_TEST_TIME_MEASURMENT
	DEBUGOUT("load_test_fw Spend Time= %d\r\n", get_system_time() - u32_fun_time);
#endif

	return u8_ret;
}

STATUS check_ext_flash_fw_version(void)
{
	unsigned int u32_addr, u32_normal_fw_version, u32_test_fw_version;
	unsigned short u16_data_length = 0;
	u16_data_length = 4;

	if ((g_st_test_info.u16_ft_test_info_1) == WEARBLE_FT_FW_VER_SYSTEM_NG_CASE)
		return ERROR;

	//get normal FW version from ext flash
	u32_addr = FLASH_NORMAL_FW_FW_VERSION_ADDR;
	if (read_flash_data(u32_addr, u16_data_length) == ERROR) {
		DEBUGOUT("Read Ext Flash NG [0x%08X] \r\n", u32_addr);
		return ERROR;
	}
	u32_normal_fw_version = 0;
	u32_normal_fw_version |= g_u8_data_buf[0];
	u32_normal_fw_version |= g_u8_data_buf[1] << 8;
	u32_normal_fw_version |= g_u8_data_buf[2] << 16;
	u32_normal_fw_version |= g_u8_data_buf[3] << 24;
	//DEBUGOUT("SPI Read_addr = 0x%08X, u16DataLength = %d \r\n", u32_normal_fw_version, u16_data_length);

	// Test FW Version
	u32_addr = FLASH_TEST_FW_FW_VERSION_ADDR;
	if (read_flash_data(u32_addr, u16_data_length) == ERROR) {
		DEBUGOUT("Read Ext Flash NG [0x%08X] \r\n", u32_addr);
		return ERROR;
	}
	u32_test_fw_version = 0;
	u32_test_fw_version |= g_u8_data_buf[0];
	u32_test_fw_version |= g_u8_data_buf[1] << 8;
	u32_test_fw_version |= g_u8_data_buf[2] << 16;
	u32_test_fw_version |= g_u8_data_buf[3] << 24;
	//DEBUGOUT("SPI Read_addr = 0x%08X, u16DataLength = %d \r\n", u32_test_fw_version, u16_data_length);
	if (g_st_test_para_resv.u32_normal_fw_version != u32_normal_fw_version
	    || g_st_test_para_resv.u32_test_fw_version != u32_test_fw_version) {
		DEBUGOUT("INI FW Version NG ,0x%08X VS 0x%08X \r\n", g_st_test_para_resv.u32_normal_fw_version, u32_normal_fw_version);
		DEBUGOUT("INI Test FW Version NG ,0x%08X VS 0x%08X \r\n", g_st_test_para_resv.u32_test_fw_version, u32_test_fw_version);
		return ERROR;
	}

	return SUCCESS;
}

STATUS system_test_2x(void)
{
	STATUS u8_test_result = SUCCESS;
	unsigned short u16_panel_mode;
#if ENABLE_TEST_TIME_MEASURMENT
	unsigned int u32_fun_time;
#endif
	//Clear Test Item Test result
	g_u32_wearable_test_result &= ~(WEARABLE_FT_TEST_RESULT_I2C_NG | WEARABLE_FT_TEST_RESULT_INT_NG
					| WEARABLE_FT_TEST_RESULT_RESET_NG | WEARABLE_FT_TEST_RESULT_PRAM_NG
					| WEARABLE_FT_TEST_RESULT_IC_SUB_VERSION_NG | WEARABLE_FT_TEST_RESULT_IC_FW_VERIFY_NG);

	if ((g_st_test_info.u16_ft_test_item & IC_TEST_ITEMS_SYSTEM) == FALSE) {
		return SUCCESS;
	}
	if (enable_ic_block_2x() == ERROR) {
		DEBUGOUT("enable_ic_block NG!!!\r\n");
		//return ERROR;
	}

	stop_mcu_2x(1);

	//Check Dongle Ext Flash FW/Test FW version PK INI
	/*if (check_ext_flash_fw_version() == ERROR) {
		u8_test_result = ERROR;
		g_u32_wearable_test_result = WEARABLE_FT_TEST_RESULT_IC_FW_VERIFY_NG;
		goto ERROR_EXIT;
	}*/

#if ENABLE_TEST_TIME_MEASURMENT
	u32_fun_time = get_system_time();
#endif

	//I2C or SPI Test
	if (ft_test_connect_test_2x() == ERROR) {
		u8_test_result = ERROR;
		goto ERROR_EXIT;
	}
#if ENABLE_TEST_TIME_MEASURMENT
	DEBUGOUT("connect test Spend Time= %d\r\n", get_system_time() - u32_fun_time);
#endif

	if ((g_st_test_info.u16_ft_eng_item & IC_TEST_ENG_ITEMS_RESV_12)) {
		DEBUGOUT("INI Model = 0x%08X\r\n", g_st_test_para_resv.u32_normal_fw_version);
		u16_panel_mode = (g_st_test_para_resv.u32_normal_fw_version & 0x0000FFFF);
		if (ft_test_panel_model_check_2x(u16_panel_mode) == ERROR) {
			u8_test_result = ERROR;
			g_u32_wearable_test_result = WEARABLE_FT_TEST_RESULT_IC_FW_VERIFY_NG;
			goto ERROR_EXIT;
		}
	}

#if ENABLE_TEST_TIME_MEASURMENT
	u32_fun_time = get_system_time();
#endif
	//INT Pin Test
	if (hw_int_pin_Test_2x() == ERROR) {
		u8_test_result = ERROR;
		goto ERROR_EXIT;
	}
#if ENABLE_TEST_TIME_MEASURMENT
	DEBUGOUT("INT Spend Time= %d\r\n", get_system_time() - u32_fun_time);
#endif

#if ENABLE_TEST_TIME_MEASURMENT
	u32_fun_time = get_system_time();
#endif

	//RAM Test
	if (ft_test_ram_test_2x(0) == ERROR) {
		u8_test_result = ERROR;
		goto ERROR_EXIT;
	}

#if ENABLE_TEST_TIME_MEASURMENT
	DEBUGOUT("RAM Spend Time= %d\r\n", get_system_time() - u32_fun_time);
#endif

	if ((g_st_test_info.u16_ft_eng_item & IC_TEST_ENG_ITEMS_RESV_11)) {
		if (check_dev_sub_version_2x(g_st_test_para_resv.u8_para_resv_0[21]) == ERROR) {
			u8_test_result = ERROR;
			g_u32_wearable_test_result = WEARABLE_FT_TEST_RESULT_IC_SUB_VERSION_NG;
			goto ERROR_EXIT;
		}
	}

#if ENABLE_TEST_TIME_MEASURMENT
	u32_fun_time = get_system_time();
#endif

	//Reset Pin Test
	if (ft_test_reset_pin_test_2x() == ERROR) {
		u8_test_result = ERROR;
		goto ERROR_EXIT;
	}

#if ENABLE_TEST_TIME_MEASURMENT
	DEBUGOUT("Reset Spend Time= %d\r\n", get_system_time() - u32_fun_time);
#endif

ERROR_EXIT:

	if (u8_test_result == ERROR)
		DEBUGOUT("System Test NG!!\r\n");
	else
		DEBUGOUT("System Test PASS\r\n");

	return u8_test_result;
}

void do_ic_test_2x(void)
{
	unsigned short u16_test_items;

#if ENABLE_TEST_TIME_MEASURMENT
	unsigned char u8_ic_test_state;
#endif

#if ENABLE_TEST_TIME_MEASURMENT
	unsigned int u32_fun_time = 0;
#endif

#if ENABLE_CONTROL_OPENSHORT_WDT
	Chip_WWDT_SetTimeOut(LPC_WWDT, (WDT_OSC / 10) * 150);
#endif

	wearable_ic_test_init();
	test_item_message_2x();
	if (g_st_test_info.u16_ft_test_item == 0) {
		return;
	}
	if (g_st_test_info.u8_device_id != 2) {
		g_u32_wearable_test_result = WEARABLE_FT_TEST_RESULT_IC_VERSION_NG;
		return;
	}

	g_u8_ic_test_state = IC_TEST_INIT;

	u16_test_items = g_st_test_info.u16_ft_test_item;

	g_u32_wearable_test_result &= ~(WEARABLE_FT_TEST_RESULT_POWER_ON_NG | WEARABLE_FT_TEST_RESULT_IC_VERSION_NG
					| WEARABLE_FT_TEST_RESULT_MCU_HOLD_NG | WEARABLE_FT_TEST_RESULT_EXT_FLASH_EMPTY_NG
					| WEARABLE_FT_TEST_RESULT_FLASH_ID_NG | WEARABLE_FT_TEST_RESULT_NORMAL_FW_VER_NG
					| WEARABLE_FT_TEST_RESULT_PANEL_TEST_3_NG | WEARABLE_FT_TEST_RESULT_CB_NG | WEARABLE_FT_TEST_RESULT_TEST_FW_VER_NG

					| WEARABLE_FT_TEST_RESULT_AUO_JIG_NG);

	while (u16_test_items) {
#if ENABLE_TEST_TIME_MEASURMENT
		u8_ic_test_state = g_u8_ic_test_state;
		g_u32_spend_time = get_system_time();
#endif

		switch (g_u8_ic_test_state) {
		case IC_TEST_INIT:
#if ENABLE_TEST_TIME_MEASURMENT
			u32_fun_time = get_system_time();
#endif
			g_u8_ic_test_state = IC_TEST_SYSTEM;
#if ENABLE_TEST_TIME_MEASURMENT
			DEBUGOUT("IC_TEST_INIT Spend Time= %d\r\n", get_system_time() - u32_fun_time);
#endif

			break;
		case IC_TEST_SYSTEM:
			if (system_test_2x() == SUCCESS ||
			    (g_st_test_info.u16_ft_eng_item & IC_TEST_ENG_ITEMS_TEST_ALL)) {

				g_u8_ic_test_state = IC_TEST_BURN_FW;
			} else {
				g_u8_ic_test_state = IC_TEST_EXIT;
			}
			break;
		case IC_TEST_BURN_FW:
#if !SELFTEST
			if (burn_fw_2x() == ERROR) {
				g_u32_wearable_test_result = WEARABLE_FT_TEST_RESULT_BURN_FW_NG;
				g_u8_ic_test_state = IC_TEST_EXIT;
			} else {
				g_u8_ic_test_state = IC_TEST_LOAD_TEST_FW;
			}
#else
			g_u32_wearable_test_result &= ~(WEARABLE_FT_TEST_RESULT_BURN_FW_NG);
			g_u8_ic_test_state = IC_TEST_LOAD_TEST_FW;
#endif
			break;
		case IC_TEST_LOAD_TEST_FW:
			if (load_test_fw_2x() == ERROR) {
				DEBUGOUT("Load Test FW NG!!\r\n");
				g_u32_wearable_test_result = WEARABLE_FT_TEST_RESULT_LOAD_TESTFW_NG;
				g_u8_ic_test_state = IC_TEST_EXIT;
			} else {
				g_u8_ic_test_state = IC_TEST_ENTER_FW_TEST_MODE;
			}
			break;
		case IC_TEST_ENTER_FW_TEST_MODE:
			if (enter_fw_test_mode_2x() == ERROR) {
				DEBUGOUT("Enter FW Test Mode NG!!\r\n");
				g_u8_ic_test_state = IC_TEST_EXIT;
			} else {
				g_u8_ic_test_state = IC_TEST_OPEN_SHORT;
			}
			break;
		case IC_TEST_OPEN_SHORT:
			if (ft_test_do_fw_test_2x(u16_test_items) == SUCCESS) {
				DEBUGOUT("FW Test Finish!!\r\n");
				g_u8_ic_test_state = IC_TEST_READ_TEST_FW_DATA;
			} else {
				DEBUGOUT("FW Test NG!!\r\n");
				g_u32_wearable_test_result = WEARABLE_FT_TEST_RESULT_TEST_INIT_NG;
				g_u8_ic_test_state = IC_TEST_EXIT;
			}
			break;
		case IC_TEST_READ_TEST_FW_DATA:
			if (read_test_fw_data_2x(u16_test_items) == SUCCESS) {
//				g_u8_ic_test_state = IC_TEST_BURN_CC;
				g_u8_ic_test_state = IC_ENTER_NORMAL_FW;
			} else {
				DEBUGOUT("Read Test FW Data NG!!\r\n");
				g_u8_ic_test_state = IC_TEST_EXIT;
			}
			break;
		case IC_ENTER_NORMAL_FW:
			if (enter_normal_fw_2x() == ERROR) {
				g_u32_wearable_test_result |= WEARABLE_FT_TEST_RESULT_NORMAL_FW_NG;
				DEBUGOUT("Enter Normal FW NG\r\n");
				g_u8_ic_test_state = IC_TEST_EXIT;
			} else {
				g_u8_ic_test_state = IC_TEST_BURN_CC;
			}
			break;
		case IC_TEST_BURN_CC:
			if (burn_cc_2x(u16_test_items) == ERROR) {
				DEBUGOUT("Burn CC NG!!\r\n");
				g_u32_wearable_test_result |= WEARABLE_FT_TEST_RESULT_BURN_CC_NG;
				g_u8_ic_test_state = IC_TEST_PANEL_PATTERN_TEST;
			} else {
				g_u8_ic_test_state = IC_TEST_PANEL_PATTERN_TEST;
			}
			break;
		case IC_TEST_PANEL_PATTERN_TEST:
			DEBUGOUT("B g_u32_wearable_test_result = 0x%08x\r\n", g_u32_wearable_test_result);

#if !SELFTEST
			do_ic_panel_test_2x();
#endif
			g_u8_ic_test_state = IC_TEST_EXIT;
			break;
		case IC_TEST_EXIT:
#if ENABLE_TEST_TIME_MEASURMENT
			u32_fun_time = get_system_time();
			DEBUGOUT("Final action Start Time= %d\r\n", u32_fun_time);
#endif

			DEBUGOUT("g_u32_wearable_test_result = 0x%08x\r\n", g_u32_wearable_test_result);
			DEBUGOUT("Exit IC Test!\r\n");
#if !SELFTEST
			gpio_touch_int_trigger_control(FALSE);
#endif

			g_u8_ic_test_state = IC_TEST_INIT;
			if ((g_st_test_info.u16_ft_test_item & (IC_TEST_ITEMS_OPEN | IC_TEST_ITEMS_SHORT)) == 0) {
				ft_raw_data_checksum_cal_2x((unsigned short *)g_i16_raw_data_1_short_buf);
				ft_raw_data_checksum_cal_2x((unsigned short *)g_i16_raw_data_2_open_buf);
				ft_raw_data_checksum_cal_2x(g_u16_raw_data3_cc_buf);
			}
			ft_raw_data_checksum_cal_2x((unsigned short *)g_i16_ub_buf);
			ft_raw_data_checksum_cal_2x(g_u16_uc_buf);

			ft_raw_data_checksum_cal_2x((unsigned short *)g_u16_raw_data3_golden_cc_buf);
			ft_raw_data_checksum_cal_2x((unsigned short *)g_u16_uc_golden_cc_buf);
			ft_test_result_checksum_cal_2x(g_u32_test_result);
#if !SELFTEST
			DEBUGOUT("g_u8_panel_jig_test_status= %d\r\n", g_u8_panel_jig_test_status);
			if (g_u8_panel_jig_test_status == STATUS_DATA_PANEL_JIG_FINISH && (g_st_test_info.u16_ft_eng_item & IC_TEST_ENG_ITEMS_RESV_16)) {
				if (!burn_log_to_flash_2x())
					DEBUGOUT("[burn_log_to_flash ERROR!]\r\n");
			}
#endif
#if ENABLE_CONTROL_OPENSHORT_WDT
			//Chip_WWDT_SetTimeOut(LPC_WWDT, (WDT_OSC / 10) * 50);
#endif
			u16_test_items = 0;
#if ENABLE_TEST_TIME_MEASURMENT
			DEBUGOUT("Final action Spend Time= %d\r\n", get_system_time() - u32_fun_time);
#endif
			break;
		}

#if !SELFTEST
		update_dongle_test_status(g_u8_ic_test_state);
#endif

#if ENABLE_WDT
		Chip_WWDT_Feed(LPC_WWDT);// reload the Watchdog timer
#endif

#if !SELFTEST
		if (g_u8_ic_power_on_ng) {
			g_u32_wearable_test_result |= WEARABLE_FT_TEST_RESULT_POWER_ON_NG;
			g_u8_ic_test_state = IC_TEST_EXIT;
		}
#endif

#if ENABLE_TEST_TIME_MEASURMENT
		DEBUGOUT("Test Item [%d] End (%d)\r\n", u8_ic_test_state, get_system_time() - g_u32_spend_time);
#endif
	}
}

