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
#include "f302_ic_reg.h"
//nclude "f302_ic_test_ft.h"

unsigned char check_dev_id_2x(unsigned short u16_dev_id)
{
	unsigned int u32_read;

	if (handle_ic_read(REG_FLASHCTL_DEVID_ADDR, 4, (unsigned char *)(&u32_read), g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}
	if (((u32_read & 0xFFFF0000) >> 16) == u16_dev_id) {
		g_u16_dev_id = DEVICE_ID_2X;
		return SUCCESS;
	} else {
		DEBUGOUT("Device ID NG! 0x%x:0x%x\r\n", ((u32_read & 0xFFFF0000) >> 16), u16_dev_id);
	}
	return ERROR;
}

unsigned char check_dev_sub_version_2x(unsigned char u8_version)
{
	unsigned int u32_read = 0;

	if (handle_ic_read(REG_FLASHCTL_DEVID_ADDR, 4, (unsigned char *)(&u32_read), g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}
	DEBUGOUT("Device Sub Version 0x%x\r\n", u32_read);
	if ((u32_read & 0x000000FF) == u8_version) {
		return SUCCESS;
	} else {
		DEBUGOUT("Device Sub Version NG! 0x%x:0x%x\r\n", (u32_read & 0x000000FF), u8_version);
	}
	return ERROR;
}

unsigned char enable_ic_block_2x()
{
	unsigned int u32_read = 0;

	if (handle_ic_read(REG_SYSCON_BLKEN_ADDR, 4, (unsigned char *)(&u32_read), g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}

	u32_read |= 0xc8000000;
	//u32_read |= (BLKEN_FIC_RB_EN | BLKEN_GPIO_RB_EN | BLKEN_SYS_RB_EN);
	if (handle_ic_write(REG_SYSCON_BLKEN_ADDR, 4, (unsigned char *)&u32_read, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}

	if (handle_ic_read(REG_SYSCON_MISCIER_ADDR, 4, (unsigned char *)(&u32_read), g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}

	u32_read |= 0x00000404;
	//u32_read |= (MISCIER_RB_MCU_INTO | MISCIER_RB_MCU_INT_EN);
	if (handle_ic_write(REG_SYSCON_MISCIER_ADDR, 4, (unsigned char *)&u32_read, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}
	return SUCCESS;
}

unsigned char stop_mcu_2x(unsigned char u8_is_tp_reset)
{
	unsigned short u16_time_out = 100;
	unsigned int u32_read_data = 0;
	unsigned int u32_write_data = 0;

	if (u8_is_tp_reset) {
		gpio_touch_hw_reset();
		delay_ms(2);
	}

	//Stop MCU
	u32_write_data = MCU_HOLD | SKIP_LOAD;
	if (handle_ic_write(REG_FLASHCTL_FLASH_STATE_REG_ADDR, 4, (unsigned char *)&u32_write_data, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}

	u32_write_data = BLKRST_SW_RST;
	if (handle_ic_write(REG_SYSCON_BLKRST_ADDR, 4, (unsigned char *)&u32_write_data, g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}

	delay_ms(2);

	if (handle_ic_read(REG_FLASHCTL_FLASH_STATE_REG_ADDR, 4, (unsigned char *)(&u32_read_data), g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
		return ERROR;
	}

	while ((u32_read_data & MCU_HOLD_STATUS) == 0 && u16_time_out-- > 0) {
		delay_ms(10);
		if (handle_ic_read(REG_FLASHCTL_FLASH_STATE_REG_ADDR, 4, (unsigned char *)(&u32_read_data), g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
			return ERROR;
		}
	}

	DEBUGOUT("Stop MCU=0x%X(0x%x)(%d)!!\r\n", u32_read_data, (u32_read_data & MCU_HOLD_STATUS), u16_time_out);

	if ((u32_read_data & MCU_HOLD_STATUS) == 0) {
		return ERROR;
	}

	return SUCCESS;
}

unsigned char hardware_reset_2x(unsigned char u8_enable_ic_block)
{
	unsigned char u8_time_out = 30;
	DEBUGOUT("HW Reseting...\r\n");


	if (g_u8_drv_interface == SPI_INTERFACE) {
		//flashless
#if !SELFTEST
		if (burn_pram_from_dongle_flash_2x() == ERROR) {
			return ERROR;
		}
#else
		if (fw_upgrade(RAD_CMD_START_BURN) == ERROR) {
			DEBUGOUT("### Raydium Reload FW NG ###\n");
			return ERROR;
		}
#endif
	} else {
		gpio_touch_hw_reset();
		delay_ms(20);
	}

	if (u8_enable_ic_block) {
		if (enable_ic_block_2x() == ERROR) {
			DEBUGOUT("HW Reset NG!!\r\n");
			return ERROR;
		}
	}

	while (u8_time_out--) {
		if (gpio_touch_int_pin_state_access()) {
			break;
		}
		if (u8_time_out == 0) {
			DEBUGOUT("TP INT state access NG!!\r\n");
			return ERROR;
		}
		delay_ms(1);
	}

	return SUCCESS;
}

unsigned char set_fw_system_cmd_2x(unsigned int u32_sysm_cmd)
{
	unsigned short u16_time_out = 300;
	unsigned char u8_value;

	if (handle_ic_write(FW_SYS_CMD_ADDR, 4, (unsigned char *)&u32_sysm_cmd, g_u8_drv_interface, I2C_WORD_MODE) == ERROR)
		return ERROR;

	// Wait Test Command ready
	while (--u16_time_out) {
		delay_ms(1);
		if (handle_ic_read(FW_SYS_CMD_ADDR, 1, &u8_value, g_u8_drv_interface, I2C_BYTE_MODE) == ERROR) {
			return ERROR;
		} else if (u8_value == 0) {
			break;
		}
	}

	if (u16_time_out == 0) {
		DEBUGOUT("ERROR!! timeout!!! \r\n");
		return ERROR;
	}

	return SUCCESS;
}

unsigned char wait_fw_state_2x(unsigned int u32_addr, unsigned int u32_state, unsigned short u16_delay, unsigned short u16_retry)
{
	unsigned int u32_read_data = 0;
	do {
		if (handle_ic_read(u32_addr, 4, (unsigned char *)(&u32_read_data), g_u8_drv_interface, I2C_WORD_MODE) == ERROR) {
			return ERROR;
		}

		delay_ms(u16_delay);
		u16_retry--;
	} while ((u32_read_data != u32_state) && (u16_retry != 0));

	if (u32_read_data != u32_state) {
		return ERROR;
	}
	return SUCCESS;
}

