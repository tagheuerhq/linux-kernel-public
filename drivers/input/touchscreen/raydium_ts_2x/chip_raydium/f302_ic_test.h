#include "ic_drv_global.h"

//****************************************************************************
//                            Defined Const Value
//*****************************************************************************
#define FT_CMD_DO_FT_TEST			0x14
#define FT_CMD_ALWAYS_ACTIVE		0x34

extern STATUS turn_on_flash_2x(void);
extern STATUS read_fpc_flash_2x(unsigned int u32_addr, unsigned int *p_u32_data);
extern STATUS set_test_info_thd_para_2x(void);
extern STATUS check_test_fw_status_2x(unsigned char u8_target_status, unsigned char *p_u8_result);
extern STATUS ft_test_do_fw_test_2x(unsigned short u16_test_items);
extern STATUS enter_fw_test_mode_2x(void);
extern STATUS system_test_2x(void);
extern STATUS ft_test_ctrl_mbist_fun_2x(unsigned char u8_enable);
extern STATUS ft_test_ram_test_2x(unsigned char u8_is_stop_mcu);
extern STATUS ft_test_connect_test_2x(void);
extern STATUS ft_test_reset_pin_test_2x(void);
extern STATUS ft_raw_data_checksum_check_2x(unsigned short *u16_buffer);
extern STATUS ft_test_result_checksum_check_2x(unsigned int *u32_buffer);
extern STATUS burn_cc_to_ic_flash_2x(void);
extern STATUS check_cc_bl_flag_2x(void);
extern STATUS read_test_fw_data_2x(unsigned short u16_test_items);
extern STATUS load_test_fw_2x(void);

extern void dump_image_data_2x(short *p_image_buf, unsigned char u8_remap);
extern void dump_image_hex_data_2x(short *p_image_buf);
extern STATUS ft_test_read_used_pin_infor_2x(unsigned char *p_u8_infor);
extern void ft_raw_data_checksum_cal_2x(unsigned short *u16_buffer);
extern void ft_test_result_checksum_cal_2x(unsigned int *u32_buffer);
extern STATUS baseline_update_control_2x(bool b_enable_baseline_update);
extern STATUS enter_normal_fw_2x(void);
extern STATUS do_calibration_2x(unsigned char u8_do_calibration_cmd, unsigned char u8_burn_flash);
extern STATUS hw_int_pin_Test_2x(void);
extern void test_item_message_2x(void);
extern STATUS burn_cc_2x(unsigned short u16_test_items);
extern void do_ic_test_2x(void);

//-----------------------------extern FT function ------------------------------------------
extern STATUS load_test_fw_ft_2x(void);
extern void do_ic_panel_test_2x(void);
extern STATUS burn_fw_2x(void);
extern STATUS burn_to_ic_flash_2x(unsigned char u8_type, unsigned char u8_is_check_cc_bl);
extern unsigned char notify_panel_jig_start_test_2x(unsigned char u8_cmd);

