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
#endif
#include "ic_drv_interface.h"
#include "drv_interface.h"
#include "f302_ic_control.h"
#include "f302_ic_test.h"

#include "ic_drv_global.h"
#if !SELFTEST
#include "main.h"
#include "f301_ic_control.h"
#include "f301_ic_test.h"
#include "f302_ic_test_ft.h"
#endif

st_test_threshold g_st_test_thd;
st_test_info g_st_test_info;
st_test_para_resv g_st_test_para_resv;
unsigned char g_u8_ic_test_state;

void wearable_ic_test_read_info(void)
{
	if (read_flash_data(DONGLE_FLASH_INI_ADDR, 16) != ERROR) {
		memcpy(&g_st_test_info, g_u8_data_buf, sizeof(g_st_test_info));
	}
}

void wearable_ic_test_info_init(void)
{
#if SELFTEST
	g_u16_test_items_host_cmd = IC_TEST_ITEMS_SYSTEM | IC_TEST_ITEMS_OPEN | IC_TEST_ITEMS_SHORT |  IC_TEST_ITEMS_UC | IC_TEST_ITEMS_UB | IC_TEST_ITEMS_BURN_CC;
#endif

	wearable_ic_test_read_info();

	if (g_u16_test_items_host_cmd != 0)
		g_st_test_info.u16_ft_test_item &= g_u16_test_items_host_cmd;
	g_u16_panel_jig_set_test_items |= g_st_test_info.u16_ft_test_item;

	DEBUGOUT("TestItem:0x%x:0x%x\r\n", g_st_test_info.u16_ft_test_item, g_st_test_info.u16_ft_eng_item);
	DEBUGOUT("g_u16_panel_jig_set_test_items:0x%x\r\n", g_u16_panel_jig_set_test_items);
	if (read_flash_data(INI_THRESHOLD_ADDR, 36))
		memcpy(&g_st_test_thd, g_u8_data_buf, sizeof(g_st_test_thd));

	DEBUGOUT("THD:\r\n%d,%d,%d,%d,\r\n%d,%d,%d,%d,\r\n%d,%d,%d,%d\r\n",
		 g_st_test_thd.i16_ft_test_open_lower_thd,
		 g_st_test_thd.i16_ft_test_short_upper_thd,
		 g_st_test_thd.i16_ft_test_short_lower_thd,
		 g_st_test_thd.i16_ft_test_single_cc_upper_thd,
		 g_st_test_thd.i16_ft_test_single_cc_lower_thd,
		 g_st_test_thd.i16_ft_test_uniformity_bl_upper_thd,
		 g_st_test_thd.i16_ft_test_uniformity_bl_lower_thd,
		 g_st_test_thd.i16_ft_test_uniformity_cc_upper_thd,
		 g_st_test_thd.i16_ft_test_uniformity_cc_lower_thd,
		 g_st_test_thd.i16_ft_test_panel_test_1_thd,
		 g_st_test_thd.i16_ft_test_panel_test_3_thd,
		 g_st_test_thd.i16_ft_test_panel_test_2_thd);

	if (read_flash_data(INI_PARA_ADDR, 48))
		memcpy(&g_st_test_para_resv, g_u8_data_buf, sizeof(g_st_test_para_resv));
#if 0
	DEBUGOUT(" g_st_test_para_resv.u32_normal_fw_version = %X ,g_st_test_para_resv.u32_test_fw_version= %X \r\n",
		 g_st_test_para_resv.u32_normal_fw_version,
		 g_st_test_para_resv.u32_test_fw_version
		);
#endif

	if (read_flash_data(INI_RAW_DATA_3_CC_ADDR, 72))
		memcpy(g_u16_raw_data3_golden_cc_buf, g_u8_data_buf, sizeof(g_u16_raw_data3_golden_cc_buf));
#if 0
	DEBUGOUT(" g_u16_raw_data3_golden_cc_buf[0] = %d,g_u16_raw_data3_golden_cc_buf[1 =%d g_u16_raw_data3_golden_cc_buf[2]=%d,\r\n",
		 g_u16_raw_data3_golden_cc_buf[0],
		 g_u16_raw_data3_golden_cc_buf[1],
		 g_u16_raw_data3_golden_cc_buf[2]
		);
#endif
	if (read_flash_data(INI_UC_CC_ADDR, 72))
		memcpy(g_u16_uc_golden_cc_buf, g_u8_data_buf, sizeof(g_u16_uc_golden_cc_buf));

	if (read_flash_data(INI_RAW_DATA2_BL_ADDR, 72))
		memcpy(g_i16_raw_data2_golden_bl_buf, g_u8_data_buf, sizeof(g_i16_raw_data2_golden_bl_buf));
}

void wearable_ic_test_init(void)
{
	wearable_ic_test_info_init();
	if (!(g_st_test_info.u16_ft_eng_item & IC_TEST_ENG_ITEMS_PANEL_TEST_JIG)) {
		wearable_ic_test_init_buffer();
	}
}

void wearable_ic_test_init_buffer(void)
{
	g_u32_wearable_test_result = WEARABLE_FT_TEST_RESULT_IC_INIT_STATE;
	memset(g_u32_test_result, 0, sizeof(g_u32_test_result));
	g_u8_is_normal_fw = FALSE;
}
#if !SELFTEST
STATUS handle_burn_log_to_flash(void)
{
	STATUS u8_status = ERROR;

	if (g_u16_dev_id == DEVICE_ID_2X) {
		u8_status = burn_log_to_flash_2x();
	}

	return u8_status;
}
#endif
void handle_ic_test(void)
{
	if (g_u16_dev_id == DEVICE_ID_2X) {
		do_ic_test_2x();
	}
#if !SELFTEST
	else if (g_u16_dev_id == DEVICE_ID_1X) {
		do_ic_test_1x();
	}
#endif
}

