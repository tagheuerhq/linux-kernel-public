/* drivers/input/touchscreen/raydium_burn_ts.c
 *
 * Raydium TouchScreen driver.
 *
 * Copyright (c) 2010  Raydium tech Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/unistd.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/device.h>
#include <asm/traps.h>
#include <linux/firmware.h>
#include "raydium_driver.h"
#include "rad_fw_image_20.h"
#if defined(FW_MAPPING_EN)
#include "rad_fw_image_21.h"
#endif
/* jianghan@20210519 add for tp_info Begin*/
extern int init_tp_fm_info(u16 version_info_num, char* version_info_str, char *name);
/* jianghan@20210519 add for tp_info End*/
void raydium_mem_table_init(unsigned short u16_id)
{
	pr_info("[touch]Raydium table init 0x%x\n", u16_id);

	g_rad_boot_image = kzalloc(RAD_BOOT_2X_SIZE, GFP_KERNEL);
	g_rad_init_image = kzalloc(RAD_INIT_2X_SIZE, GFP_KERNEL);
	g_rad_fw_image = kzalloc(RAD_FW_2X_SIZE, GFP_KERNEL);
	g_rad_para_image = kzalloc(RAD_PARA_2X_SIZE + 4, GFP_KERNEL);
	g_rad_testfw_image = kzalloc(RAD_TESTFW_2X_SIZE, GFP_KERNEL);
	g_rad_testpara_image = kzalloc(RAD_PARA_2X_SIZE + 4,
							GFP_KERNEL);
	g_u8_table_init = SUCCESS;
}

int raydium_mem_table_setting(void)
{
	int i32_ret = SUCCESS;
	pr_info("[touch]Raydium ID is 0x%x\n", g_raydium_ts->id);

	switch (g_raydium_ts->id) {
	case RAD_20:
		memcpy(g_rad_boot_image, u8_rad_boot_20, RAD_BOOT_2X_SIZE);
		memcpy(g_rad_init_image, u8_rad_init_20, RAD_INIT_2X_SIZE);
		memcpy(g_rad_fw_image, u8_rad_fw_20, RAD_FW_2X_SIZE);
		memcpy(g_rad_testfw_image, u8_rad_testfw_20, RAD_FW_2X_SIZE);
		memcpy(g_rad_testfw_image + RAD_FW_2X_SIZE, u8_rad_testpara_20
			, RAD_PARA_2X_SIZE + 4);
		memcpy(g_rad_para_image, u8_rad_para_20, RAD_PARA_2X_SIZE + 4);
		memcpy(g_rad_testpara_image, u8_rad_testpara_20,
			RAD_PARA_2X_SIZE + 4);
		break;
#if defined(FW_MAPPING_EN)
	case RAD_21:
		memcpy(g_rad_boot_image, u8_rad_boot_21, RAD_BOOT_2X_SIZE);
		memcpy(g_rad_init_image, u8_rad_init_21, RAD_INIT_2X_SIZE);
		memcpy(g_rad_fw_image, u8_rad_fw_21, RAD_FW_2X_SIZE);
		memcpy(g_rad_testfw_image, u8_rad_testfw_21, RAD_FW_2X_SIZE);
		memcpy(g_rad_testfw_image + RAD_FW_2X_SIZE, u8_rad_testpara_21,
			RAD_PARA_2X_SIZE + 4);
		memcpy(g_rad_para_image, u8_rad_para_21, RAD_PARA_2X_SIZE + 4);
		memcpy(g_rad_testpara_image, u8_rad_testpara_21, 
			RAD_PARA_2X_SIZE + 4);
		break;
#endif
	default:
		pr_info("[touch]mapping ic setting use default fw\n");
		memcpy(g_rad_boot_image, u8_rad_boot_20, RAD_BOOT_2X_SIZE);
		memcpy(g_rad_init_image, u8_rad_init_20, RAD_INIT_2X_SIZE);
		memcpy(g_rad_fw_image, u8_rad_fw_20, RAD_FW_2X_SIZE);
		memcpy(g_rad_testfw_image, u8_rad_testfw_20, RAD_FW_2X_SIZE);
		memcpy(g_rad_testfw_image + RAD_FW_2X_SIZE,
			u8_rad_testpara_20, RAD_PARA_2X_SIZE + 4);
		memcpy(g_rad_para_image, u8_rad_para_20, RAD_PARA_2X_SIZE + 4);
		memcpy(g_rad_testpara_image, u8_rad_testpara_20,
			RAD_PARA_2X_SIZE + 4);
		i32_ret = SUCCESS;
		break;
	}

	g_u8_table_setting = 0;
	return i32_ret;
}

int raydium_id_init(unsigned char u8_type)
{
	int i32_ret = ERROR;
	i32_ret = 0;

	switch (u8_type) {
	case 0:
		g_raydium_ts->id = RAD_20;
		i32_ret = SUCCESS;
		break;
#if defined(FW_MAPPING_EN)
	case 1:
		g_raydium_ts->id = RAD_21;
		i32_ret = SUCCESS;
		break;
#endif
	}
	return i32_ret;
}

unsigned int bits_reverse(unsigned int u32_num, unsigned int bit_num)
{
	unsigned int reverse = 0, u32_i;

	for (u32_i = 0; u32_i < bit_num; u32_i++) {
		if (u32_num & (1 << u32_i))
			reverse |= 1 << ((bit_num - 1) - u32_i);
	}
	return reverse;
}

unsigned int rc_crc32(const char *buf, unsigned int u32_len,
			     unsigned int u32_crc)
{
	unsigned int u32_i;
	unsigned char u8_flash_byte, u8_current, u8_j;

	for (u32_i = 0; u32_i < u32_len; u32_i++) {
		u8_flash_byte = buf[u32_i];
		u8_current = (unsigned char)bits_reverse(u8_flash_byte, 8);
		for (u8_j = 0; u8_j < 8; u8_j++) {
			if ((u32_crc ^ u8_current) & 0x01)
				u32_crc = (u32_crc >> 1) ^ 0xedb88320;
			else
				u32_crc >>= 1;
			u8_current >>= 1;
		}
	}
	return u32_crc;
}

int wait_fw_state(struct i2c_client *client, unsigned int u32_addr,
			 unsigned int u32_state, unsigned long u32_delay_us,
			 unsigned short u16_retry)
{
	unsigned char u8_buf[4];
	unsigned int u32_read_data;
	unsigned int u32_min_delay_us = u32_delay_us - 500;
	unsigned int u32_max_delay_us = u32_delay_us + 500;

	do {
		if (raydium_i2c_pda_read(client, u32_addr, u8_buf, 4) == ERROR)
			return ERROR;

		memcpy(&u32_read_data, u8_buf, 4);
		u16_retry--;
		usleep_range(u32_min_delay_us, u32_max_delay_us);
	} while ((u32_read_data != u32_state) && (u16_retry != 0));

	if (u32_read_data != u32_state) {
		pr_err("[touch]confirm data error : 0x%x\n", u32_read_data);
		return ERROR;
	}

	return SUCCESS;
}

int wait_irq_state(struct i2c_client *client, unsigned int retry_time,
				unsigned int u32_delay_us)
{
	int i32_ret = SUCCESS;
	unsigned int u32_retry;
	unsigned int u32_irq_value;
	unsigned int u32_min_delay_us = u32_delay_us - 500;
	unsigned int u32_max_delay_us = u32_delay_us + 500;

	u32_retry = retry_time;
	u32_irq_value = 0;
	while (u32_retry != 0 && u32_irq_value != 1) {
		u32_irq_value = gpio_get_value(g_raydium_ts->irq_gpio);
		usleep_range(u32_min_delay_us, u32_max_delay_us);
		u32_retry--;
	}
	pr_info("[touch]irq_value is %d\n", u32_irq_value);

	if (u32_retry == 0) {
		pr_err("[touch]%s, FW not ready, retry error!\n", __func__);
		i32_ret = ERROR;
	}

	return i32_ret;
}

int raydium_do_software_reset(struct i2c_client *client)
{
	int i32_ret = SUCCESS;

	unsigned char u8_buf[4];

	/*SW reset*/
	g_u8_resetflag = true;
	memset(u8_buf, 0, sizeof(u8_buf));
	u8_buf[0] = 0x01;
	pr_info("[touch]SW reset\n");
	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_BLKRST, u8_buf, 4);
	if (i32_ret < 0)
		goto exit;

	msleep(25);
exit:
	return i32_ret;
}

int set_skip_load(struct i2c_client *client)
{
	int i32_ret = SUCCESS;
	unsigned int u32_retry_time = 1000;
	unsigned char u8_buf[4];

	/*Skip load*/
	memset(u8_buf, 0, sizeof(u8_buf));
	u8_buf[0] = 0x10;
	u8_buf[1] = 0x08;
	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_BOOTREG, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;
	i32_ret = raydium_do_software_reset(client);
	if (i32_ret < 0)
		pr_err("[touch]%s, SW reset error!\n", __func__);

	i32_ret = wait_fw_state(client, RAYDIUM_PDA_BOOTSTATE, 0x82, 2000, u32_retry_time);
	if (i32_ret < 0)
		pr_err("[touch]%s, wait_fw_state error!\n", __func__);

exit_upgrade:
	return i32_ret;
}

/*check pram crc32*/
static int raydium_check_pram_crc_2X(struct i2c_client *client,
		unsigned int u32_addr,
		unsigned int u32_len)
{
	int i32_ret = SUCCESS;
	unsigned int u32_crc_addr = u32_addr + u32_len;
	unsigned int u32_end_addr = u32_crc_addr - 1;
	unsigned int u32_crc_result, u32_read_data;
	unsigned int u32_retry = 400;
	unsigned char u8_buf[4], u8_retry = 3;

	memset(u8_buf, 0, sizeof(u8_buf));
	u8_buf[0] = (unsigned char)(u32_addr & 0xFF);
	u8_buf[1] = (unsigned char)((u32_addr & 0xFF00) >> 8);
	u8_buf[2] = (unsigned char)(u32_end_addr & 0xFF);
	u8_buf[3] = (unsigned char)((u32_end_addr & 0xFF00) >> 8);

	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_PRGCHKSUMADDR, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;
	i32_ret = raydium_i2c_pda_read(client, RAYDIUM_PDA_PRGCHKSUMENG, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;
	u8_buf[3] |= 0x81;
	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_PRGCHKSUMENG, u8_buf, 4);

	while (u8_buf[3] != 0x80 && u32_retry != 0) {
		i32_ret = raydium_i2c_pda_read(client, RAYDIUM_PDA_PRGCHKSUMENG, u8_buf, 4);
		if (i32_ret < 0)
			goto exit_upgrade;

		u32_retry--;
		usleep_range(4500, 5500);
	}
	if (u32_retry == 0) {
		pr_err("[touch]%s, Cal CRC not ready, retry error!\n",
			__func__);
		i32_ret = ERROR;
	}

	i32_ret = raydium_i2c_pda_read(client, RAYDIUM_PDA_PRGCHKSUMRESULT, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;
	memcpy(&u32_crc_result, u8_buf, 4);
	i32_ret = raydium_i2c_pda_read(client, u32_crc_addr, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;
	memcpy(&u32_read_data, u8_buf, 4);

	while (u32_read_data != u32_crc_result && u8_retry > 0) {
		i32_ret = raydium_i2c_pda_read(client, RAYDIUM_PDA_PRGCHKSUMRESULT, u8_buf, 4);
		if (i32_ret < 0)
			goto exit_upgrade;
		memcpy(&u32_crc_result, u8_buf, 4);
		usleep_range(1500, 2500);
		u8_retry--;
	}
	if (u32_read_data != u32_crc_result) {
		pr_err("[touch]check pram crc fail!!\n");
		pr_err("[touch]u32_read_data 0x%x\n", u32_read_data);
		pr_err("[touch]u32_crc_result 0x%x\n", u32_crc_result);
		i32_ret = ERROR;
		goto exit_upgrade;
	} else if (u8_retry != 3) {
		pr_err("[touch]check pram crc pass!!\n");
		pr_err("[touch]u8_retry : %d\n", u8_retry);
		pr_err("[touch]u32_read_data 0x%x\n", u32_read_data);
		pr_err("[touch]u32_crc_result 0x%x\n", u32_crc_result);
		i32_ret = ERROR;
		goto exit_upgrade;
	}

exit_upgrade:
	return i32_ret;
}

/* upgrade firmware with image file */
static int raydium_write_to_pram_2X(struct i2c_client *client,
		unsigned int u32_fw_addr,
		unsigned char u8_type)
{
	int i32_ret = ERROR;
	unsigned int u32_fw_size = 0;
	unsigned char *p_u8_firmware_data = NULL;
	unsigned int u32_write_offset = 0;
	unsigned short u16_write_length = 0;

	switch (u8_type) {
	case RAYDIUM_PARA:
		u32_fw_size = RAYDIUM_PDA_PARALENGTH;
		p_u8_firmware_data = g_rad_para_image;
		break;

	case RAYDIUM_FIRMWARE:
		u32_fw_size = RAYDIUM_PDA_FIRMWARELENGTH;
		p_u8_firmware_data = g_rad_fw_image;
		break;
	case RAYDIUM_TEST_FW:
		u32_fw_size = 0x6360;
		p_u8_firmware_data = g_rad_testfw_image;
		break;

	default:
		goto exit_upgrate;
	}

	u32_write_offset = 0;
	while (u32_write_offset < u32_fw_size) {
		if ((u32_write_offset + MAX_WRITE_PACKET_SIZE) < u32_fw_size)
			u16_write_length = MAX_WRITE_PACKET_SIZE;
		else
			u16_write_length =
			(unsigned short)(u32_fw_size - u32_write_offset);

		i32_ret = raydium_i2c_pda_write(
			      client,
			      (u32_fw_addr + u32_write_offset),
			      (p_u8_firmware_data + u32_write_offset),
			      u16_write_length);
		if (i32_ret < 0)
			goto exit_upgrate;

		u32_write_offset += (unsigned long)u16_write_length;
	}
	u32_fw_addr += u32_write_offset;

exit_upgrate:
	if (i32_ret < 0) {
		pr_err("[touch]upgrade failed\n");
		return i32_ret;
	}
	pr_info("[touch]upgrade success\n");
	return SUCCESS;
}

/* Raydium fireware upgrade flow */
static int raydium_fw_upgrade_2X(struct i2c_client *client,
			      unsigned char u8_type,
			      unsigned char u8_check_crc)
{
	int i32_ret = 0;
	unsigned char u8_buf[4];
	unsigned short u16_retry = 1000;
	
	/*##### wait for boot-loader start #####*/
	pr_info("[touch]Type is %x\n", u8_type);

	/*read Boot version*/
	if (raydium_i2c_pda_read(client, RAYDIUM_PDA_BOOTVERSION, u8_buf, 4) == ERROR)
		return ERROR;
	pr_info("[touch]Boot version is %x\n", u8_buf[2]);

	if (u8_type != RAYDIUM_COMP) {
		/*set mcu hold*/
		memset(u8_buf, 0, sizeof(u8_buf));
		u8_buf[0] = 0x20;
		i32_ret = raydium_i2c_pda_write(client,
					RAYDIUM_PDA_BOOTREG,
					u8_buf, 4);
		if (i32_ret < 0)
			goto exit_upgrade;
		u8_buf[0] = 0x01;
		i32_ret = raydium_i2c_pda_write(client,
					RAYDIUM_PDA_BLKRST,
					u8_buf, 4);
		if (i32_ret < 0)
			goto exit_upgrade;
		msleep(25);
	}

	/*#start write data to PRAM*/
	if (u8_type == RAYDIUM_FIRMWARE) {
		/* unlock PRAM */
		u8_buf[0] = 0x27;
		i32_ret = raydium_i2c_pda_write(client,
					RAYDIUM_PDA_PRAMLOCK,
					u8_buf, 4);
		if (i32_ret < 0)
			goto exit_upgrade;
		i32_ret = raydium_write_to_pram_2X(client, RAYDIUM_PDA_FIRMWAREADDR,
						    RAYDIUM_FIRMWARE);
		if (i32_ret < 0)
			goto exit_upgrade;

		i32_ret = raydium_write_to_pram_2X(client, RAYDIUM_PDA_PARAADDR,
						    RAYDIUM_PARA);
		if (i32_ret < 0)
			goto exit_upgrade;

		i32_ret = raydium_check_pram_crc_2X(client, RAYDIUM_PDA_FIRMWAREADDR,
			RAYDIUM_PDA_CRCLENGTH);
		if (i32_ret < 0)
			goto exit_upgrade;

	}

	if (u8_type != RAYDIUM_COMP) {
		/*release mcu hold*/
		/*Skip load*/
		i32_ret = set_skip_load(client);
		if (i32_ret < 0)
			pr_err("[touch]%s, set skip_load error!\n",
				__func__);
	}

	/*#setting burning mode*/
	memset(u8_buf, 0, sizeof(u8_buf));
	u8_buf[0] = 0x01;
	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_BOOTENG1, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;

	memset(u8_buf, 0, sizeof(u8_buf));
	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_BOOTENG2, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;

	memset(u8_buf, 0, sizeof(u8_buf));
	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_BOOTENG3, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;

	memset(u8_buf, 0, sizeof(u8_buf));
	u8_buf[0] = 0x01;
	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_BOOTMODE, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;

	/*#confirm in burn mode*/
	if (wait_fw_state(client, RAYDIUM_PDA_BOOTSTATE, 255,
			 2000, u16_retry) == ERROR) {
		pr_err("[touch]Error, confirm in burn mode\n");
		i32_ret = ERROR;
		goto exit_upgrade;
	}

	/* burning setting */
	memset(u8_buf, 0, sizeof(u8_buf));
	u8_buf[0] = 0x10;
	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_BOOTREG, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;

	memset(u8_buf, 0, sizeof(u8_buf));
	u8_buf[0] = u8_type;
	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_PRAMTYPE, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;

	/*#set PRAM length (at 'h5000_090C)*/
	if (u8_type == RAYDIUM_COMP) {
		memset(u8_buf, 0, sizeof(u8_buf));
		u8_buf[0] = 0x60;
		u8_buf[1] = 0x6b;
		i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_PRAMADDR,
						 u8_buf, 4);
		if (i32_ret < 0)
			goto exit_upgrade;

		memset(u8_buf, 0, sizeof(u8_buf));
		u8_buf[0] = 0x9c;
		u8_buf[1] = 0x02;
		i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_PRAMLENGTH,
						 u8_buf, 4);
		if (i32_ret < 0)
			goto exit_upgrade;

	} else if (u8_type == RAYDIUM_FIRMWARE) {
		memset(u8_buf, 0, sizeof(u8_buf));
		u8_buf[0] = 0x00;
		u8_buf[1] = 0x08;
		i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_PRAMADDR,
						 u8_buf, 4);
		if (i32_ret < 0)
			goto exit_upgrade;

		memset(u8_buf, 0, sizeof(u8_buf));
		u8_buf[0] = 0x5c;
		u8_buf[1] = 0x63;
		i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_PRAMLENGTH,
						 u8_buf, 4);
		if (i32_ret < 0)
			goto exit_upgrade;

	}

	/*#set sync_data(RAYDIUM_PDA_SYNCDATA) = 0 as WRT data finish*/
	memset(u8_buf, 0, sizeof(u8_buf));
	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_SYNCDATA, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;

	/*#wait for sync_data finish*/
	if (wait_fw_state(client, RAYDIUM_PDA_BOOTENG4, 168, 1000,
			 u16_retry) == ERROR) {
		pr_err("[touch]Error, wait for input unlock key\n");
		i32_ret = ERROR;
		goto exit_upgrade;
	}

	/*#flash unlock key*/
	memset(u8_buf, 0, sizeof(u8_buf));
	u8_buf[0] = 0xd7;
	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_FLKEY2, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;

	memset(u8_buf, 0, sizeof(u8_buf));
	u8_buf[0] = 0xa5;
	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_FLKEY1, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;

	memset(u8_buf, 0, sizeof(u8_buf));
	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_FLKEY1, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;

	memset(u8_buf, 0, sizeof(u8_buf));
	u8_buf[0] = 0xa5;
	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_FLKEY1, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;

	memset(u8_buf, 0, sizeof(u8_buf));
	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_FLKEY2, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;

	memset(u8_buf, 0, sizeof(u8_buf));
	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_FLASHPRO, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;

	/*#ready to burn flash*/
	memset(u8_buf, 0, sizeof(u8_buf));
	u8_buf[0] = 0xa8;
	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_BOOTSTATE, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;

	/*#clr sync_data(RAYDIUM_PDA_SYNCDATA) = 0 as finish*/
	memset(u8_buf, 0, sizeof(u8_buf));
	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_SYNCDATA, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;

	/* wait erase/wrt finish
	 * confirm burning_state result (gu8I2CSyncData.burning_state =
	 * BURNING_WRT_FLASH_FINISH at RAYDIUM_PDA_BOOTENG3)
	 */
	if (wait_fw_state(client, RAYDIUM_PDA_BOOTENG3, 6, 8000,
			 u16_retry) == ERROR) {
		pr_err("[touch]Error, wait erase/wrt finish\n");
		i32_ret = ERROR;
		goto exit_upgrade;
	}
	pr_info("[touch]Burn flash ok\n");

	if (u8_check_crc) {
		memset(u8_buf, 0, sizeof(u8_buf));
		i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_SYNCDATA,
						 u8_buf, 4);
		if (i32_ret < 0)
			goto exit_upgrade;

		/*#wait software reset finish*/
		msleep(25);

		/* wait sw reset finished RAYDIUM_PDA_BOOTSTATE = 0x82 */
		if (wait_fw_state(client, RAYDIUM_PDA_BOOTSTATE, 0x82, 2000,
				 u16_retry) == ERROR) {
			pr_err("[touch]Error, wait sw reset finished\n");
			i32_ret = ERROR;
			goto exit_upgrade;
		}

		/*#set test_mode = 1 start to check crc*/
		memset(u8_buf, 0, sizeof(u8_buf));
		u8_buf[0] = 0x01;
		i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_BOOTMODE,
						 u8_buf, 4);
		if (i32_ret < 0)
			goto exit_upgrade;

		/*#wait crc check finish*/
		if (wait_fw_state(client, RAYDIUM_PDA_BOOTENG2, 2,
				 2000, u16_retry)
				== ERROR) {
			pr_err("[touch]Error, wait crc check finish\n");
			i32_ret = ERROR;
			goto exit_upgrade;
		}

		/*#crc check pass RAYDIUM_PDA_BOOTSTATE = 0x81*/
		if (wait_fw_state(client, RAYDIUM_PDA_BOOTSTATE, 0x81,
				 2000, u16_retry)
				== ERROR) {
			pr_err("[touch]Error, confirm crc result\n");
			i32_ret = ERROR;
			goto exit_upgrade;
		}
		/*#run to next step*/
		pr_info("[touch]Type 0x%x => Pass\n", u8_type);

		/* sw rest */
		i32_ret = raydium_do_software_reset(client);
		if (i32_ret < 0)
			goto exit_upgrade;
		
		g_u8_i2c_mode = PDA2_MODE;

		pr_info("[touch]Burn FW finish!\n");		
	}

exit_upgrade:
	return i32_ret;
}

int raydium_burn_fw(struct i2c_client *client)
{
	int i32_ret = ERROR;

	g_u8_resetflag = true;
	if ((g_raydium_ts->id & 0x2000) != 0) {
		pr_info("[touch]start burn function!\n");

		i32_ret = raydium_fw_upgrade_2X(client, RAYDIUM_FIRMWARE, ENABLE);
		if (i32_ret < 0)
			goto exit_upgrade;
	} else {
		pr_info("[touch]FW ID ERROR!\n");
	}

exit_upgrade:
	return i32_ret;
}

int raydium_fw_update_check(unsigned short u16_i2c_data)
{

	unsigned char u8_rbuffer[4];

	unsigned int u32_fw_version, u32_image_version;
	int i32_ret = ERROR;

#ifdef SUPPORT_READ_TP_VERSION
	char tp_version[50] = {0};
#endif
	mutex_lock(&g_raydium_ts->lock);
	i32_ret = raydium_i2c_pda2_set_page(g_raydium_ts->client,
				g_raydium_ts->is_suspend,
				RAYDIUM_PDA2_PAGE_0);
	if (i32_ret < 0)
		goto exit_error;

	i32_ret = raydium_i2c_pda2_read(g_raydium_ts->client,
				    RAYDIUM_PDA2_FW_VERSION_ADDR,
				    u8_rbuffer,
				    4);
	if (i32_ret < 0)
		goto exit_error;

	mutex_unlock(&g_raydium_ts->lock);

	u32_fw_version = (u8_rbuffer[0] << 24) | (u8_rbuffer[1] << 16) |
		(u8_rbuffer[2] << 8) | u8_rbuffer[3];
	pr_info("[touch]RAD FW ver 0x%.8x\n", u32_fw_version);

	g_raydium_ts->fw_version = u32_fw_version;

	g_raydium_ts->id = ((u16_i2c_data & 0xF) << 12) |
		((u8_rbuffer[0] & 0xF) << 8) | u8_rbuffer[1];

	raydium_mem_table_init(g_raydium_ts->id);
	if (raydium_mem_table_setting() == SUCCESS) {

		u32_image_version = (g_rad_para_image[PARA_FW_VERSION_OFFSET] << 24) |
			(g_rad_para_image[PARA_FW_VERSION_OFFSET + 1] << 16) |
			(g_rad_para_image[PARA_FW_VERSION_OFFSET + 2] << 8) |
			g_rad_para_image[PARA_FW_VERSION_OFFSET + 3];

		pr_info("[touch]RAD Image FW ver : 0x%x\n", u32_image_version);
	} else {
		pr_info("[touch]Mem setting failed, Stop fw upgrade!\n");
		return FAIL;
	}

#ifdef FW_UPDATE_EN
	if (u32_fw_version != u32_image_version) {
		pr_info("[touch]FW need update.\n");
 		g_u8_raydium_flag |= ENG_MODE; 

 		i32_ret = raydium_burn_fw(g_raydium_ts->client);
		if (i32_ret < 0)
			pr_err("[touch]FW update fail:%d\n", i32_ret);

		g_u8_raydium_flag &= ~ENG_MODE;
 		mutex_lock(&g_raydium_ts->lock);
		i32_ret = raydium_i2c_pda2_set_page(g_raydium_ts->client,
					g_raydium_ts->is_suspend,
					RAYDIUM_PDA2_PAGE_0);
		if (i32_ret < 0)
			goto exit_error;

		i32_ret = raydium_i2c_pda2_read(g_raydium_ts->client,
					    RAYDIUM_PDA2_FW_VERSION_ADDR,
					    u8_rbuffer,
					    4);
		if (i32_ret < 0)
			goto exit_error;

		mutex_unlock(&g_raydium_ts->lock);
		u32_fw_version = (u8_rbuffer[0] << 24) |
			     (u8_rbuffer[1] << 16) |
			     (u8_rbuffer[2] << 8) |
			     u8_rbuffer[3];
		pr_info("[touch]RAD FW ver is 0x%x\n",
			 u32_fw_version);
		g_raydium_ts->fw_version = u32_fw_version;
	} else
		pr_info("[touch]FW is the latest version.\n");
#endif

#ifdef SUPPORT_READ_TP_VERSION
	memset(tp_version, 0, sizeof(tp_version));
	sprintf(tp_version, "[FW]0x%X,[IC]RM6D020",u32_fw_version);
	init_tp_fm_info(0, tp_version, "Raydium");
#endif

	return i32_ret;

exit_error:
	mutex_unlock(&g_raydium_ts->lock);
	return i32_ret;
}
int raydium_burn_comp(struct i2c_client *client)
{
	int i32_ret = ERROR;

	i32_ret = set_skip_load(client);
	if (i32_ret < 0)
		goto exit_upgrade;


	i32_ret = raydium_fw_upgrade_2X(client, RAYDIUM_COMP, 1);
	if (i32_ret < 0)
		goto exit_upgrade;

	i32_ret = SUCCESS;

exit_upgrade:
	return i32_ret;
}

int raydium_check_fw_ready(struct i2c_client *client)
{
	int i32_ret = SUCCESS;
	unsigned int u32_retry = 400;
	unsigned char u8_buf[4];

	u8_buf[1] = 0;
	while (u8_buf[1] != 0x40 && u32_retry != 0) {
		i32_ret = raydium_i2c_pda_read(client, RAYDIUM_PDA_BOOTREG, u8_buf, 4);
		if (i32_ret < 0)
			goto exit;

		u32_retry--;
		usleep_range(4500, 5500);
	}

	if (u32_retry == 0) {
		pr_err("[touch]%s, FW not ready, retry error!\n", __func__);
		i32_ret = ERROR;
	} else {
		pr_info("[touch]%s, FW is ready!!\n", __func__);
		usleep_range(4500, 5500);
	}

exit:
	return i32_ret;
}
/* upgrade firmware with image file */
int raydium_fw_upgrade_with_image(struct i2c_client *client,
		unsigned int u32_fw_addr,
		unsigned char u8_type)
{
	int i32_ret = ERROR;
	unsigned int u32_fw_size = 0;
	unsigned char *p_u8_firmware_data = NULL;
	unsigned int u32_write_offset = 0;
	unsigned short u16_write_length = 0;
	unsigned int u32_checksum = 0xFFFFFFFF;

	switch (u8_type) {
		case RAYDIUM_INIT:
			u32_fw_size = 0x1fc;
			p_u8_firmware_data = g_rad_init_image;
			break;
		case RAYDIUM_PARA:
			u32_fw_size = 0x158;
			p_u8_firmware_data = g_rad_para_image;
			break;
		case RAYDIUM_FIRMWARE:
			u32_fw_size = 0x61fc;
			p_u8_firmware_data = g_rad_fw_image;
			break;
		case RAYDIUM_BOOTLOADER:
			u32_fw_size = 0x7FC;
			p_u8_firmware_data = g_rad_boot_image;
			break;
		case RAYDIUM_TEST_FW:
			u32_fw_size = 0x635C;
			p_u8_firmware_data = g_rad_testfw_image;
			break;
	}

#if 1
	pr_info("[touch]CRC 0x%08X\n",
		*(unsigned int *)(p_u8_firmware_data + u32_fw_size));

	u32_checksum = rc_crc32(p_u8_firmware_data,
		u32_fw_size, u32_checksum);
	u32_checksum = bits_reverse(u32_checksum, 32);
	memcpy((p_u8_firmware_data + u32_fw_size), &u32_checksum, 4);
	pr_info("[touch]CRC result 0x%08X\n", u32_checksum);
#endif	
	u32_fw_size += 4;

	u32_write_offset = 0;
	while (u32_write_offset < u32_fw_size) {
		if ((u32_write_offset + MAX_WRITE_PACKET_SIZE) < u32_fw_size)
			u16_write_length = MAX_WRITE_PACKET_SIZE;
		else
			u16_write_length =
				(unsigned short)
				(u32_fw_size - u32_write_offset);

		i32_ret = raydium_i2c_pda_write(
			      client,
			      (u32_fw_addr + u32_write_offset),
			      (p_u8_firmware_data + u32_write_offset),
			      u16_write_length);
		if (i32_ret < 0)
			goto exit_upgrate;

		u32_write_offset += (unsigned long)u16_write_length;
	}
	u32_fw_addr += u32_write_offset;

exit_upgrate:
	if (i32_ret < 0) {
		pr_err("[touch]upgrade failed\n");
		return i32_ret;
	}
	pr_info("[touch]upgrade success\n");
	return SUCCESS;
}


int raydium_load_test_fw(struct i2c_client *client)
{
	int i32_ret = SUCCESS;
	unsigned char u8_buf[4];
	unsigned int u32_crc_result, u32_read_data;

	/*set mcu hold*/
	memset(u8_buf, 0, sizeof(u8_buf));
	u8_buf[0] = 0x20;
	raydium_i2c_pda_write(client, RAYDIUM_PDA_BOOTREG, u8_buf, 4);
	raydium_i2c_pda_read(client, RAYDIUM_PDA_BLKRST, u8_buf, 4);
	u8_buf[0] |= 0x01;
	raydium_i2c_pda_write(client, RAYDIUM_PDA_BLKRST, u8_buf, 4);
	msleep(25);


	i32_ret = raydium_i2c_pda_read(client, RAYDIUM_PDA_BLKEN, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;

	u8_buf[3] |= 0x40;
	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_BLKEN, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;

	i32_ret = raydium_i2c_pda_read(client, RAYDIUM_PDA_MISCIER, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;

	u8_buf[0] |= 0x04;
	u8_buf[1] |= 0x04;
	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_MISCIER, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;

	memset(u8_buf, 0, sizeof(u8_buf));
	pr_info("[touch]Raydium WRT test_fw to PRAM\n");

	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_PRAMLOCK, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;

	/*Sending test fw*/
	i32_ret = raydium_fw_upgrade_with_image(client,
		0x800, RAYDIUM_TEST_FW);
	if (i32_ret < 0)
		goto exit_upgrade;

	/*check pram crc data*/

	memset(u8_buf, 0, sizeof(u8_buf));
	u8_buf[1] = 0x08;
	u8_buf[2] = 0x5B;
	u8_buf[3] = 0x6B;
	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_PRGCHKSUMADDR, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;
	i32_ret = raydium_i2c_pda_read(client, RAYDIUM_PDA_PRGCHKSUMENG, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;
	u8_buf[3] |= 0x81;
	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_PRGCHKSUMENG, u8_buf, 4);
	usleep_range(9500, 10500);
	i32_ret = raydium_i2c_pda_read(client, RAYDIUM_PDA_PRGCHKSUMRESULT, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;
	memcpy(&u32_crc_result, u8_buf, 4);
	i32_ret = raydium_i2c_pda_read(client, 0x6B5C, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;
	memcpy(&u32_read_data, u8_buf, 4);
	if (u32_read_data != u32_crc_result) {
		pr_err("[touch]check pram fw crc fail!!\n");
		pr_err("[touch]u32_crc_result 0x%x\n", u32_crc_result);
		i32_ret = ERROR;
		goto exit_upgrade;
	}

	memset(u8_buf, 0, sizeof(u8_buf));
	u8_buf[0] = 0x60;
	u8_buf[1] = 0x6B;
	u8_buf[2] = 0xFB;
	u8_buf[3] = 0x6D;
	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_PRGCHKSUMADDR, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;
	i32_ret = raydium_i2c_pda_read(client, RAYDIUM_PDA_PRGCHKSUMENG, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;
	u8_buf[3] |= 0x81;
	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_PRGCHKSUMENG, u8_buf, 4);
	usleep_range(1000, 2000);
	i32_ret = raydium_i2c_pda_read(client, RAYDIUM_PDA_PRGCHKSUMRESULT, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;
	memcpy(&u32_crc_result, u8_buf, 4);
	i32_ret = raydium_i2c_pda_read(client, 0x6DFC, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;
	memcpy(&u32_read_data, u8_buf, 4);
	if (u32_read_data != u32_crc_result) {
		pr_err("[touch]check pram CB crc fail!!\n");
		pr_err("[touch]u32_crc_result 0x%x\n", u32_crc_result);
		i32_ret = ERROR;
		goto exit_upgrade;
	}

	i32_ret = raydium_i2c_pda_read(client, 0x80, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;


	pr_err("[touch]bootloader version %x,!!\n", u8_buf[2]);
	memset(u8_buf, 0, sizeof(u8_buf));

	u8_buf[1] = 0x04;
	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_BOOTREG,
					 u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;

	/*Skip load*/
	pr_info("[touch]Raydium skip load\n");
	i32_ret = raydium_i2c_pda_read(client, RAYDIUM_PDA_BOOTREG, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;
	u8_buf[0] = 0x10;
	i32_ret = raydium_i2c_pda_write(client, RAYDIUM_PDA_BOOTREG, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;
	i32_ret = raydium_do_software_reset(client);
	if (i32_ret < 0)
		goto exit_upgrade;
	i32_ret = raydium_i2c_pda_read(client, RAYDIUM_PDA_BOOTREG, u8_buf, 4);
	if (i32_ret < 0)
		goto exit_upgrade;
	pr_info("[touch]RAYDIUM_PDA_BOOTREG = 0x%x, 0x%x, 0x%x, 0x%x\n",
		u8_buf[0], u8_buf[1], u8_buf[2], u8_buf[3]);
	i32_ret = raydium_check_fw_ready(client);

exit_upgrade:
	return i32_ret;
}
