
//#include "Config.h"

#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/unistd.h>
#include <linux/string.h>
#include <linux/of_gpio.h>
#include "chip_raydium/ic_drv_global.h"
#include "chip_raydium/ic_drv_interface.h"
#include "drv_interface.h"
#include "raydium_selftest.h"
#include "raydium_driver.h"

struct timeval timer;
unsigned char g_u8_m_buf[2][128];
unsigned char g_u8_ini_flash[0x400];
struct raydium_ts_data *ts;

STATUS i2c_burst_read_pda2(unsigned char u8_addr, unsigned short u16ReadLen, unsigned char *p_u8_output_buf)
{
	return ERROR;
}
STATUS i2c_burst_write_pda2(unsigned char u8_addr, unsigned char bWriteLen, unsigned char *bValue)
{
	return ERROR;
}

unsigned char spi_write_pda(unsigned int u32_addr, unsigned char u8_write_len, unsigned char *bValue, unsigned char u8_trans_mode)
{
	return ERROR;
}
unsigned char spi_read_pda(unsigned int u32_addr, unsigned char u8_read_len, unsigned char *p_u8_output_buf) 
{
	return ERROR;
}

unsigned char fw_upgrade(unsigned char type)
{
	int i32_ret = ERROR; 
	i32_ret = raydium_burn_fw(g_raydium_ts->client);
		if (i32_ret < 0)
			pr_err("[touch]FW update fail:%d\n", i32_ret);
	return i32_ret;
	
}
unsigned char read_flash_data(unsigned int u32_addr, unsigned short u16_lenth)
{
	unsigned int u32_data_offset;

	if (g_u16_dev_id == DEVICE_ID_2X) {
		u32_data_offset = u32_addr - 0x800;

		if (u32_addr < 0x8000) {
			if (u32_data_offset >= 0x6200 && u32_addr < DONGLE_FLASH_INI_ADDR) {
				u32_data_offset = u32_data_offset - 0x6200;
				memcpy(g_u8_data_buf, g_rad_para_image + u32_data_offset, u16_lenth);
			} else if (u32_addr >= DONGLE_FLASH_INI_ADDR) {
				u32_data_offset = u32_addr - DONGLE_FLASH_INI_ADDR;
				memcpy(g_u8_data_buf, g_u8_ini_flash + u32_data_offset, u16_lenth);
			}  else
				memcpy(g_u8_data_buf, g_rad_fw_image + u32_data_offset, u16_lenth);
		} else {
			u32_data_offset -= 0x8000;
			if (u32_data_offset >= 0x6200 && u32_addr < DONGLE_FLASH_INI_ADDR) {
				u32_data_offset = u32_data_offset - 0x6200;
				memcpy(g_u8_data_buf, g_rad_testpara_image + u32_data_offset, u16_lenth);
			} else if (u32_addr >= DONGLE_FLASH_INI_ADDR) {
				u32_data_offset = u32_addr - DONGLE_FLASH_INI_ADDR;
				memcpy(g_u8_data_buf, g_u8_ini_flash + u32_data_offset, u16_lenth);
			} else {
				memcpy(g_u8_data_buf, g_rad_testfw_image + u32_data_offset, u16_lenth);
			}
		}
		return TRUE;
	}
	return FALSE;
}

unsigned int get_system_time(void)
{
	unsigned int u32_timer;
	do_gettimeofday(&timer);
	u32_timer = (timer.tv_sec % 1000) * 1000 + (timer.tv_usec / 1000);
	return u32_timer;
}

unsigned char gpio_touch_int_pin_state_access(void)
{	
	return gpio_get_value(ts->irq_gpio);
}


unsigned char sysfs_burn_cc_bl(void)
{
	unsigned char ret = ERROR;
	DEBUGOUT("start sysfs_burn_cc_bl\r\n");
	ret = raydium_burn_comp(ts->client);
	return ret;
}

unsigned char raydium_upgrade_test_fw_2x(unsigned long ul_fw_addr)
{
	int ret = ERROR; 
	unsigned char u8_retry = 2;
	unsigned int u32_read;

RETRY:
	gpio_touch_reset_pin_control(0);
	delay_ms(10);
	gpio_touch_reset_pin_control(1);
	delay_ms(2);
	handle_write_word(0x50000918, 0x00000030);
	
	
	if (raydium_load_test_fw(ts->client) == SUCCESS) {
		ret = SUCCESS;
		DEBUGOUT("### Raydium Load test FW SUCCESS ###\n");
	}

	handle_read_word(0x6A04, &u32_read);
	if (u32_read != g_st_test_para_resv.u32_test_fw_version) {
		DEBUGOUT("Read FW version NG=0x%x:0x%x!!\r\n", u32_read, g_st_test_para_resv.u32_test_fw_version);
		goto ERROR_EXIT;
	}

	return ret;

ERROR_EXIT:
	if (u8_retry) {
		u8_retry--;
		goto RETRY;
	}

	return ERROR;
}

void gpio_touch_reset_pin_control(unsigned char u8_high)
{

	if (u8_high)
		gpio_set_value(ts->rst_gpio, 1);
	else
		gpio_set_value(ts->rst_gpio, 0);

	return;

}


void gpio_touch_hw_reset(void)
{
	gpio_touch_reset_pin_control(0);
	delay_ms(10);
	gpio_touch_reset_pin_control(1);

}
void set_raydium_ts_data(struct raydium_ts_data *ts_old)
{
	ts = ts_old;
}

unsigned char i2c_burst_read(unsigned int u32_addr, unsigned char u8_read_len, unsigned char *p_u8_output_buf)
{

	STATUS bResult = ERROR;
	unsigned char u8_rx_len = u8_read_len;
	unsigned char *p_u8_i2c_read_buf;
	unsigned char u8_len = 0;

	if (p_u8_output_buf == NULL)
		p_u8_i2c_read_buf = g_u8_m_buf[M_RX_BUF];
	else
		p_u8_i2c_read_buf = p_u8_output_buf;

	if (u8_read_len == 0) {
		DEBUGOUT("[I2CBR] Length(%d) of I2C read is error!!\r\n", u8_read_len);
		return ERROR;
	}


	while (u8_len < u8_read_len) {
		if ((u8_len + MAX_READ_PACKET_SIZE) < u8_read_len)
			u8_rx_len = MAX_READ_PACKET_SIZE;
		else
			u8_rx_len = (u8_read_len - u8_len);
		if (raydium_i2c_pda_read(ts->client, u32_addr + u8_len, g_u8_m_buf[M_RX_BUF], u8_rx_len) == ERROR) {
			bResult = ERROR;
			break;
		} else {
			memcpy(&p_u8_i2c_read_buf[u8_len], g_u8_m_buf[M_RX_BUF], u8_rx_len);
			//DEBUGOUT("[I2CBR]read len%d data0x%2x%2x%2x%2x \r\n",u8_rx_len,p_u8_i2c_read_buf[u8_len],p_u8_i2c_read_buf[u8_len+1],p_u8_i2c_read_buf[u8_len+2],p_u8_i2c_read_buf[u8_len+3]);
			u8_len += u8_rx_len;
			bResult = SUCCESS;
		}
	}



	return bResult;
}


unsigned char i2c_burst_write(unsigned int u32_addr, unsigned char u8_write_len, unsigned char *bValue)
{

	STATUS bResult = ERROR;
	unsigned char u8_tx_len = u8_write_len;
	unsigned char u8_len = 0;

	if (u8_write_len == 0) {
		DEBUGOUT("[I2CBW] Length(%d) of I2C write is error!!\r\n", u8_write_len);
		return ERROR;
	}

	while (u8_len < u8_write_len) {
		if ((u8_len + MAX_WRITE_PACKET_SIZE) < u8_write_len)
			u8_tx_len = MAX_WRITE_PACKET_SIZE;
		else
			u8_tx_len = (u8_write_len - u8_len);
		if (raydium_i2c_pda_write(ts->client, u32_addr + u8_len, bValue + u8_len, u8_tx_len) == ERROR) {
			bResult = ERROR;
			break;
		} else {
			u8_len += u8_tx_len;
			//DEBUGOUT("[I2CBW]write len%d data0x%2x%2x%2x%2x \r\n",u8_tx_len,bValue[u8_len],bValue[u8_len +1], bValue[u8_len +2] ,bValue[u8_len + 3]);
			
			bResult = SUCCESS;
		}
	}


	return bResult;
}

unsigned char handle_read_word(unsigned int u32_addr, unsigned int *p_u32_data)
{

	if (u32_addr  & 0x00000003) {
		DEBUGOUT("[HRW] Handle Read Word ADDR Not Word Align!!\r\n");
		return ERROR;
	}

	if (raydium_i2c_pda_read(ts->client, u32_addr, (unsigned char *)p_u32_data, 4) == ERROR) {
		*p_u32_data = 0;
		DEBUGOUT("[I2CRW] I2C Read Word NG (0x%x)\r\n", u32_addr);
		return ERROR;
	}

	return SUCCESS;
}


unsigned char handle_write_word(unsigned int u32_addr, unsigned int u32_data)
{

	if (u32_addr  & 0x00000003) {
		DEBUGOUT("[I2CRW] Handle Write Word ADDR Not Word Align!!\r\n");
		return ERROR;
	}

	if ( raydium_i2c_pda_write(ts->client, u32_addr, (unsigned char *)&u32_data, 4) == ERROR) {
		DEBUGOUT("[I2CWW] Write I2C NG (0x%x:0x%x) \r\n", u32_addr, u32_data);
		return ERROR;
	}
	return SUCCESS;
}


unsigned char handle_read_pda(unsigned int u32_addr, unsigned char u8_read_len, 
				unsigned char *p_u8_output_buf)
{
	if (i2c_burst_read(u32_addr, u8_read_len, p_u8_output_buf) == ERROR) {
		DEBUGOUT("[HRP] handle_read_pda I2C NG!\r\n");
		return ERROR;
	}

	return SUCCESS;
}

unsigned char handle_write_pda(unsigned int u32_addr, unsigned char u8_write_len ,
				unsigned char *bValue, unsigned char u8_trans_mode)
{	
	if (i2c_burst_write(u32_addr, u8_write_len, bValue) == ERROR) {
		DEBUGOUT("[HWP] handle_write_pda I2C NG!\r\n");
		return ERROR;
	}

	return SUCCESS;
}

/******************************************************************************
** Function name:		handle_ic_read
**
** Descriptions:		handle read data from ic
**
** parameters:			u32_addr,  			Address
						u8_read_len, 		Read data length
						p_u8_output_buf,	Data buffer
						u8_interface,		SPI or I2C
						u8_trans_mode		PDA2_MODE, PDA_WORD_MODE, PDA_BYTE_MODE, MCU_MODE

** Returned value:		ERROR, SUCCESS
**
******************************************************************************/
unsigned char handle_ic_read(
	unsigned int 	u32_addr,
	unsigned short 	u8_read_len,
	unsigned char 	*p_u8_output_buf,
	unsigned char	u8_interface,
	unsigned char 	u8_trans_mode)
{
	if (u8_trans_mode == I2C_PDA2_MODE) {
		//PDA2 MODE
		if (raydium_i2c_pda2_read(g_raydium_ts->client, (unsigned char)u32_addr, p_u8_output_buf, u8_read_len) == ERROR) {
			DEBUGOUT("[HWP] handle_ic_read PDA2 I2C NG!\r\n");
			return ERROR;
		}
	} else {
		//PDA MODE
		if ((u8_trans_mode == I2C_WORD_MODE) && (u32_addr & 0x00000003)) {
			DEBUGOUT("[HRW] Handle Read Word ADDR Not Word Align!!\r\n");
			return ERROR;
		}

		if (u8_interface == SPI_INTERFACE) {
			if (spi_read_pda(u32_addr, u8_read_len, p_u8_output_buf) == ERROR) {
				DEBUGOUT("[HRP] handle_ic_read SPI NG!\r\n");
				return ERROR;
			}
		} else {
			if (i2c_burst_read(u32_addr, u8_read_len, p_u8_output_buf) == ERROR) {
				DEBUGOUT("[HRP] handle_ic_read I2C NG!\r\n");
				return ERROR;
			}
		}
	}

	return SUCCESS;
}

/******************************************************************************
** Function name:		handle_ic_write
**
** Descriptions:		handle write data to ic
**
** parameters:			u32_addr,  			Address
						u8_write_len, 		data length
						bValue,				Data
						u8_interface,		SPI or I2C
						u8_trans_mode		PDA2_MODE, PDA_WORD_MODE, PDA_BYTE_MODE, MCU_MODE

** Returned value:		ERROR, SUCCESS
**
******************************************************************************/
unsigned char handle_ic_write(
	unsigned int 	u32_addr,
	unsigned char 	u8_write_len,
	unsigned char 	*bValue,
	unsigned char	u8_interface,
	unsigned char 	u8_trans_mode)
{
	if (u8_trans_mode == I2C_PDA2_MODE) {
		//PDA2 MODE
		if (raydium_i2c_pda2_write(g_raydium_ts->client, (unsigned char)u32_addr, bValue, u8_write_len) == ERROR) {
			DEBUGOUT("[HWP] handle_ic_write PDA2 I2C NG!\r\n");
			return ERROR;
		}
	} else {
		//PDA MODE
		if ((u8_trans_mode == I2C_WORD_MODE) && (u32_addr & 0x00000003)) {
			DEBUGOUT("[I2CRW] Handle Write Word ADDR Not Word Align!!\r\n");
			return ERROR;
		}

		if (u8_interface == SPI_INTERFACE) {
			switch (u8_trans_mode) {
			case I2C_BYTE_MODE:
				u8_trans_mode = SPI_BYTE_MODE;
				break;
			case I2C_WORD_MODE:
				u8_trans_mode = SPI_WORD_MODE;
				break;
//			case I2C_MCU_MODE:
//				u8_trans_mode = SPI_MCU_MODE;
//				break;
			case SPI_BYTE_MODE:
			case SPI_WORD_MODE:
				break;
			default:
				DEBUGOUT("[HWP] handle_ic_write Trans mode NG! %d\r\n", u8_trans_mode);
				return ERROR;
			}

			if (spi_write_pda(u32_addr, u8_write_len, bValue, u8_trans_mode) == ERROR) {
				DEBUGOUT("[HWP] handle_ic_write SPI NG!\r\n");
				return ERROR;
			}
		} else {
			if (i2c_burst_write(u32_addr, u8_write_len, bValue) == ERROR) {
				DEBUGOUT("[HWP] handle_ic_write I2C NG!\r\n");
				return ERROR;
			}
		}
	}

	return SUCCESS;
}
