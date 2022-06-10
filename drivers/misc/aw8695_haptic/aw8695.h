#ifndef _AW8695_H_
#define _AW8695_H_

/*********************************************************
 *
 * kernel version
 *
 ********************************************************/
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 4, 1)
#define TIMED_OUTPUT
#endif

#define INPUT_DEV
//#define TEST_RTP
#define TEST_CONT_TO_RAM
/*********************************************************
 *
 * aw8695.h
 *
 ********************************************************/
#include <linux/regmap.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/hrtimer.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
#include <linux/atomic.h>
#ifdef TIMED_OUTPUT
#include <../../../drivers/staging/android/timed_output.h>
#else
#include <linux/leds.h>
#endif

/*********************************************************
 *
 * marco
 *
 ********************************************************/
#define AW8695_CHIPID                      0x95

#define MAX_I2C_BUFFER_SIZE                 65536

#define AW8695_SEQUENCER_SIZE               8
#define AW8695_SEQUENCER_LOOP_SIZE          4

#define AW8695_RTP_I2C_SINGLE_MAX_NUM       512

#define HAPTIC_MAX_TIMEOUT                  10000

#define AW8695_VBAT_REFER                   4200
#define AW8695_VBAT_MIN                     3000
#define AW8695_VBAT_MAX                     4500
#define ENABLE_PIN_CONTROL

/* trig config */
#define AW8695_TRIG_NUM                     3
#define AW8695_TRG1_ENABLE                  1
#define AW8695_TRG2_ENABLE                  1
#define AW8695_TRG3_ENABLE                  1
#define HAP_BRAKE_PATTERN_MAX       4
#define HAP_WAVEFORM_BUFFER_MAX     8
#define HAP_PLAY_RATE_US_DEFAULT    5715
#define HAP_PLAY_RATE_US_MAX        20475
#define FF_EFFECT_COUNT_MAX     32

#define AW8695_CONT_PLAYBACK_MODE       AW8695_BIT_CONT_CTRL_CLOSE_PLAYBACK
static int wf_repeat[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };
static int wf_s_repeat[4] = { 1, 2, 4, 8 };

/*
 * trig default high level
 * ___________           _________________
 *           |           |
 *           |           |
 *           |___________|
 *        first edge
 *                   second edge
 *
 *
 * trig default low level
 *            ___________
 *           |           |
 *           |           |
 * __________|           |_________________
 *        first edge
 *                   second edge
 */
#define AW8695_TRG1_DEFAULT_LEVEL     1	/* 1: high level; 0: low level */
#define AW8695_TRG2_DEFAULT_LEVEL     1	/* 1: high level; 0: low level */
#define AW8695_TRG3_DEFAULT_LEVEL     1	/* 1: high level; 0: low level */

#define AW8695_TRG1_DUAL_EDGE         1	/* 1: dual edge; 0: first edge */
#define AW8695_TRG2_DUAL_EDGE         1	/* 1: dual edge; 0: first edge */
#define AW8695_TRG3_DUAL_EDGE         1	/* 1: dual edge; 0: first edge */

#define AW8695_TRG1_FIRST_EDGE_SEQ    1	/* trig1: first edge waveform seq */
#define AW8695_TRG1_SECOND_EDGE_SEQ   2	/* trig1: second edge waveform seq */
#define AW8695_TRG2_FIRST_EDGE_SEQ    1	/* trig2: first edge waveform seq */
#define AW8695_TRG2_SECOND_EDGE_SEQ   2	/* trig2: second edge waveform seq */
#define AW8695_TRG3_FIRST_EDGE_SEQ    1	/* trig3: first edge waveform seq */
#define AW8695_TRG3_SECOND_EDGE_SEQ   2	/* trig3: second edge waveform seq */

#if AW8695_TRG1_ENABLE
#define AW8695_TRG1_DEFAULT_ENABLE          AW8695_BIT_TRGCFG2_TRG1_ENABLE
#else
#define AW8695_TRG1_DEFAULT_ENABLE          AW8695_BIT_TRGCFG2_TRG1_DISABLE
#endif

#if AW8695_TRG2_ENABLE
#define AW8695_TRG2_DEFAULT_ENABLE          AW8695_BIT_TRGCFG2_TRG2_ENABLE
#else
#define AW8695_TRG2_DEFAULT_ENABLE          AW8695_BIT_TRGCFG2_TRG2_DISABLE
#endif

#if AW8695_TRG3_ENABLE
#define AW8695_TRG3_DEFAULT_ENABLE          AW8695_BIT_TRGCFG2_TRG3_ENABLE
#else
#define AW8695_TRG3_DEFAULT_ENABLE          AW8695_BIT_TRGCFG2_TRG3_DISABLE
#endif

#if AW8695_TRG1_DEFAULT_LEVEL
#define AW8695_TRG1_DEFAULT_POLAR           AW8695_BIT_TRGCFG1_TRG1_POLAR_POS
#else
#define AW8695_TRG1_DEFAULT_POLAR           AW8695_BIT_TRGCFG1_TRG1_POLAR_NEG
#endif

#if AW8695_TRG2_DEFAULT_LEVEL
#define AW8695_TRG2_DEFAULT_POLAR           AW8695_BIT_TRGCFG1_TRG2_POLAR_POS
#else
#define AW8695_TRG2_DEFAULT_POLAR           AW8695_BIT_TRGCFG1_TRG2_POLAR_NEG
#endif

#if AW8695_TRG3_DEFAULT_LEVEL
#define AW8695_TRG3_DEFAULT_POLAR           AW8695_BIT_TRGCFG1_TRG3_POLAR_POS
#else
#define AW8695_TRG3_DEFAULT_POLAR           AW8695_BIT_TRGCFG1_TRG3_POLAR_NEG
#endif

#if AW8695_TRG1_DUAL_EDGE
#define AW8695_TRG1_DEFAULT_EDGE            AW8695_BIT_TRGCFG1_TRG1_EDGE_POS_NEG
#else
#define AW8695_TRG1_DEFAULT_EDGE            AW8695_BIT_TRGCFG1_TRG1_EDGE_POS
#endif

#if AW8695_TRG2_DUAL_EDGE
#define AW8695_TRG2_DEFAULT_EDGE            AW8695_BIT_TRGCFG1_TRG2_EDGE_POS_NEG
#else
#define AW8695_TRG2_DEFAULT_EDGE            AW8695_BIT_TRGCFG1_TRG2_EDGE_POS
#endif

#if AW8695_TRG3_DUAL_EDGE
#define AW8695_TRG3_DEFAULT_EDGE            AW8695_BIT_TRGCFG1_TRG3_EDGE_POS_NEG
#else
#define AW8695_TRG3_DEFAULT_EDGE            AW8695_BIT_TRGCFG1_TRG3_EDGE_POS
#endif

enum aw8695_flags {
	AW8695_FLAG_NONR = 0,
	AW8695_FLAG_SKIP_INTERRUPTS = 1,
};

enum aw8695_haptic_read_write {
	AW8695_HAPTIC_CMD_READ_REG = 0,
	AW8695_HAPTIC_CMD_WRITE_REG = 1,
};

enum aw8695_haptic_work_mode {
	AW8695_HAPTIC_STANDBY_MODE = 0,
	AW8695_HAPTIC_RAM_MODE = 1,
	AW8695_HAPTIC_RTP_MODE = 2,
	AW8695_HAPTIC_TRIG_MODE = 3,
	AW8695_HAPTIC_CONT_MODE = 4,
	AW8695_HAPTIC_RAM_LOOP_MODE = 5,
};

enum aw8695_haptic_bst_mode {
	AW8695_HAPTIC_BYPASS_MODE = 0,
	AW8695_HAPTIC_BOOST_MODE = 1,
};

enum aw8695_haptic_activate_mode {
	AW8695_HAPTIC_ACTIVATE_RAM_MODE = 0,
	AW8695_HAPTIC_ACTIVATE_CONT_MODE = 1,
	AW8695_HAPTIC_ACTIVATE_RTP_MODE = 2,
	AW8695_HAPTIC_ACTIVATE_RAM_LOOP_MODE = 3,
};

enum aw8695_haptic_cont_vbat_comp_mode {
	AW8695_HAPTIC_CONT_VBAT_SW_COMP_MODE = 0,
	AW8695_HAPTIC_CONT_VBAT_HW_COMP_MODE = 1,
};

enum aw8695_haptic_ram_vbat_comp_mode {
	AW8695_HAPTIC_RAM_VBAT_COMP_DISABLE = 0,
	AW8695_HAPTIC_RAM_VBAT_COMP_ENABLE = 1,
};

enum aw8695_haptic_f0_flag {
	AW8695_HAPTIC_LRA_F0 = 0,
	AW8695_HAPTIC_CALI_F0 = 1,
};

enum aw8695_haptic_pwm_mode {
	AW8695_PWM_48K = 0,
	AW8695_PWM_24K = 1,
	AW8695_PWM_12K = 2,
};

enum aw8695_haptic_play {
	AW8695_HAPTIC_PLAY_NULL = 0,
	AW8695_HAPTIC_PLAY_ENABLE = 1,
	AW8695_HAPTIC_PLAY_STOP = 2,
	AW8695_HAPTIC_PLAY_GAIN = 8,
};

enum aw8695_haptic_cmd {
	AW8695_HAPTIC_CMD_NULL = 0,
	AW8695_HAPTIC_CMD_ENABLE = 1,
	AW8695_HAPTIC_CMD_HAPTIC = 0x0f,
	AW8695_HAPTIC_CMD_TP = 0x10,
	AW8695_HAPTIC_CMD_SYS = 0xf0,
	AW8695_HAPTIC_CMD_STOP = 255,
};

enum haptics_custom_effect_param {
	CUSTOM_DATA_EFFECT_IDX,
	CUSTOM_DATA_TIMEOUT_SEC_IDX,
	CUSTOM_DATA_TIMEOUT_MSEC_IDX,
	CUSTOM_DATA_LEN,
};
enum aw8695_haptic_strength {
	AW8695_LIGHT_MAGNITUDE = 0x3fff,
	AW8695_MEDIUM_MAGNITUDE = 0x5fff,
	AW8695_STRONG_MAGNITUDE = 0x7fff,
};
/*********************************************************
 *
 * struct
 *
 ********************************************************/
struct fileops {
	unsigned char cmd;
	unsigned char reg;
	unsigned char ram_addrh;
	unsigned char ram_addrl;
};

struct ram {
	unsigned int len;
	unsigned int check_sum;
	unsigned int base_addr;
	unsigned char version;
	unsigned char ram_shift;
	unsigned char baseaddr_shift;
};

struct haptic_ctr {
	unsigned char cnt;
	unsigned char cmd;
	unsigned char play;
	unsigned char wavseq;
	unsigned char loop;
	unsigned char gain;
	struct list_head list;
};

struct haptic_audio {
	struct mutex lock;
	struct hrtimer timer;
	struct work_struct work;
	int delay_val;
	int timer_val;
	struct haptic_ctr ctr;
	struct list_head ctr_list;
	/* struct tp tp; */
	struct list_head list;
	/*  struct haptic_audio_tp_size tp_size; */
	/*   struct trust_zone_info output_tz_info[10]; */
	int tz_num;
	int tz_high_num;
	int tz_cnt_thr;
	int tz_cnt_max;
	unsigned int uevent_report_flag;
	unsigned int hap_cnt_outside_tz;
	unsigned int hap_cnt_max_outside_tz;
};

struct trig {
	unsigned char enable;
	unsigned char default_level;
	unsigned char dual_edge;
	unsigned char frist_seq;
	unsigned char second_seq;
};

struct aw8695_dts_info {
	unsigned int mode;
	unsigned int f0_pre;
	unsigned int f0_cali_percen;
	unsigned int cont_drv_lvl;
	unsigned int cont_drv_lvl_ov;
	unsigned int cont_td;
	unsigned int cont_zc_thr;
	unsigned int cont_num_brk;
	unsigned int f0_coeff;
	unsigned int f0_trace_parameter[4];
	unsigned int bemf_config[4];
	unsigned int sw_brake;
	unsigned int tset;
	unsigned int r_spare;
	unsigned int bstdbg[6];
	
	unsigned int parameter1;
   	unsigned int gain_flag;
	unsigned int effect_id_boundary;
	unsigned int effect_max;
	unsigned int rtp_time[175];
	unsigned int trig_config[3][5];
};


#ifdef INPUT_DEV
enum actutor_type {
	ACT_LRA,
	ACT_ERM,
};

enum lra_res_sig_shape {
	RES_SIG_SINE,
	RES_SIG_SQUARE,
};

enum lra_auto_res_mode {
	AUTO_RES_MODE_ZXD,
	AUTO_RES_MODE_QWD,
};

enum wf_src {
	INT_WF_VMAX,
	INT_WF_BUFFER,
	EXT_WF_AUDIO,
	EXT_WF_PWM,
};

enum own_cali {
	NORMAL_CALI,
	F0_CALI,
	OSC_CALI,
};

struct qti_hap_effect {
	int id;
	u8 *pattern;
	int pattern_length;
	u16 play_rate_us;
	u16 vmax_mv;
	u8 wf_repeat_n;
	u8 wf_s_repeat_n;
	u8 brake[HAP_BRAKE_PATTERN_MAX];
	int brake_pattern_length;
	bool brake_en;
	bool lra_auto_res_disable;
};

struct qti_hap_play_info {
	struct qti_hap_effect *effect;
	u16 vmax_mv;
	int length_us;
	int playing_pos;
	bool playing_pattern;
};

struct qti_hap_config {
	enum actutor_type act_type;
	enum lra_res_sig_shape lra_shape;
	enum lra_auto_res_mode lra_auto_res_mode;
	enum wf_src ext_src;
	u16 vmax_mv;
	u16 play_rate_us;
	bool lra_allow_variable_play_rate;
	bool use_ext_wf_src;
};
#endif

#ifdef ENABLE_PIN_CONTROL
const char *const pctl_names[] = {
	"aw8695_reset_reset",
	"aw8695_reset_active",
	"aw8695_interrupt_active",
};
#endif
struct aw8695 {
	//struct regmap *regmap;
	struct i2c_client *i2c;
	//struct device *dev;
	//struct input_dev *input;

	struct mutex lock;
    //struct mutex rtp_lock;
	//struct hrtimer timer;
	
#ifdef ENABLE_PIN_CONTROL
	struct pinctrl *aw8695_pinctrl;
	struct pinctrl_state *pinctrl_state[ARRAY_SIZE(pctl_names)];
#endif

	int enable_pin_control;
	struct work_struct vibrator_work;
	struct work_struct rtp_work;
	struct work_struct set_gain_work;
	struct delayed_work ram_work;
	struct delayed_work stop_work;
	
#ifdef TIMED_OUTPUT
	struct timed_output_dev to_dev;
#else
	struct led_classdev cdev;
#endif

	struct fileops fileops;
	struct ram ram;
	bool haptic_ready;
	bool audio_ready;
	int pre_haptic_number;
	struct timeval current_time;
	struct timeval pre_enter_time;
	struct timeval start, end;
	unsigned int timeval_flags;
	unsigned int osc_cali_flag;
	unsigned long int microsecond;
	unsigned int sys_frequency;
	unsigned int rtp_len;
	unsigned int lra_calib_data;
	unsigned int f0_calib_data;

	int reset_gpio;
	int irq_gpio;

	unsigned char hwen_flag;
	unsigned char flags;
	unsigned char chipid;
	unsigned char chipid_flag;

	unsigned char play_mode;

	unsigned char activate_mode;

	unsigned char auto_boost;

	int state;
	int duration;
	int amplitude;
	int index;
	int vmax;
	int gain;
	u16 new_gain;
	int f0_value;
	unsigned char level;

	unsigned char seq[AW8695_SEQUENCER_SIZE];
	unsigned char loop[AW8695_SEQUENCER_SIZE];

	unsigned int rtp_cnt;
	unsigned int rtp_file_num;

	unsigned char rtp_init;
	unsigned char ram_init;
	unsigned char rtp_routine_on;

	unsigned int f0;
	unsigned int cont_f0;
	unsigned char max_pos_beme;
	unsigned char max_neg_beme;
	unsigned char f0_cali_flag;
	unsigned int theory_time;
	unsigned int osc_cali_run;

	unsigned char ram_vbat_comp;
	unsigned int vbat;
	unsigned int lra;
	unsigned int interval_us;

	struct trig trig[AW8695_TRIG_NUM];

	struct haptic_audio haptic_audio;
	struct aw8695_dts_info info;
	atomic_t is_in_rtp_loop;
	atomic_t exit_in_rtp_loop;
	wait_queue_head_t wait_q;	//wait queue for exit irq mode
	wait_queue_head_t stop_wait_q;	//wait queue for stop rtp mode
	atomic_t is_in_irq;
	atomic_t exit_in_irq;
	struct workqueue_struct *work_queue;
	unsigned int ramupdate_flag;
	unsigned int rtpupdate_flag;
#ifdef INPUT_DEV
	struct platform_device *pdev;
	struct device *dev;
	struct regmap *regmap;
	struct input_dev *input_dev;
	struct pwm_device *pwm_dev;
	struct qti_hap_config config;
	struct qti_hap_play_info play;
	struct qti_hap_effect *predefined;
	struct qti_hap_effect constant;
	struct regulator *vdd_supply;
	struct hrtimer stop_timer;
	struct hrtimer hap_disable_timer;
	struct hrtimer timer;	/*test used  ,del */
	struct mutex rtp_lock;
	spinlock_t bus_lock;
	ktime_t last_sc_time;
	int play_irq;
	int sc_irq;
	int effects_count;
	int sc_det_count;
	u16 reg_base;
	bool perm_disable;
	bool play_irq_en;
	bool vdd_enabled;
	int effect_type;
	int effect_id;
	int test_val;
#endif
};

/*2019.12.19 longcheer zhangjunwei1 start*/
/*achieve the debug function*/
#define VIB_DEBUG_EN  0
#if VIB_DEBUG_EN
#define VIB_DEBUG(fmt, args...) do { \
    printk("[AWINIC_HAPTIC]%s:"fmt"\n", __func__, ##args); \
} while (0)

#define VIB_FUNC_ENTER() do { \
    printk("[AWINIC_HAPTIC]%s: Enter\n", __func__); \
} while (0)

#define VIB_FUNC_EXIT() do { \
    printk("[AWINIC_HAPTIC]%s: Exit(%d)\n", __func__, __LINE__); \
} while (0)
#else
#define VIB_DEBUG(fmt, args...)
#define VIB_FUNC_ENTER()
#define VIB_FUNC_EXIT()
#endif

#define VIB_INFO(fmt, args...) do { \
    printk(KERN_INFO "[AWINIC_HAPTIC/I]%s:"fmt"\n", __func__, ##args); \
} while (0)

#define VIB_ERROR(fmt, args...) do { \
    printk(KERN_ERR "[AWINIC_HAPTIC/E]%s:"fmt"\n", __func__, ##args); \
} while (0)
/*2019.12.19 longcheer zhangjunwei1 end*/
struct aw8695_container {
	int len;
	unsigned char data[];
};

/*********************************************************
 *
 * ioctl
 *
 ********************************************************/
struct aw8695_seq_loop {
	unsigned char loop[AW8695_SEQUENCER_SIZE];
};

struct aw8695_que_seq {
	unsigned char index[AW8695_SEQUENCER_SIZE];
};

#define AW8695_HAPTIC_IOCTL_MAGIC         'h'

#define AW8695_HAPTIC_SET_QUE_SEQ         _IOWR(AW8695_HAPTIC_IOCTL_MAGIC, 1, struct aw8695_que_seq*)
#define AW8695_HAPTIC_SET_SEQ_LOOP        _IOWR(AW8695_HAPTIC_IOCTL_MAGIC, 2, struct aw8695_seq_loop*)
#define AW8695_HAPTIC_PLAY_QUE_SEQ        _IOWR(AW8695_HAPTIC_IOCTL_MAGIC, 3, unsigned int)
#define AW8695_HAPTIC_SET_BST_VOL         _IOWR(AW8695_HAPTIC_IOCTL_MAGIC, 4, unsigned int)
#define AW8695_HAPTIC_SET_BST_PEAK_CUR    _IOWR(AW8695_HAPTIC_IOCTL_MAGIC, 5, unsigned int)
#define AW8695_HAPTIC_SET_GAIN            _IOWR(AW8695_HAPTIC_IOCTL_MAGIC, 6, unsigned int)
#define AW8695_HAPTIC_PLAY_REPEAT_SEQ     _IOWR(AW8695_HAPTIC_IOCTL_MAGIC, 7, unsigned int)

#endif
