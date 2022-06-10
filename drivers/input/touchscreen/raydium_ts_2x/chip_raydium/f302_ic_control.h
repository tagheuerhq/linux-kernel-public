#include "ic_drv_global.h"

#define DEVICE_ID_2X             				 0xF302
#define HEADER_LENGTH                            (4)//length + checksum

extern unsigned char enable_ic_block_2x(void);
extern unsigned char stop_mcu_2x(unsigned char u8_is_tp_reset);
extern unsigned char hardware_reset_2x(unsigned char u8_enable_ic_block);
extern unsigned char check_dev_id_2x(unsigned short u16_dev_id);
extern unsigned char check_dev_sub_version_2x(unsigned char u8_version);
extern unsigned char set_fw_system_cmd_2x(unsigned int u32_sysm_cmd);
extern unsigned char  wait_fw_state_2x(unsigned int u32_addr, unsigned int u32_state, unsigned short u16_delay, unsigned short u16_retry);

