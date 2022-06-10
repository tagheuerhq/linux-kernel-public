/************************************************************************
* Copyright (C) 2012-2020, Focaltech Systems (R), All Rights Reserved.
*
* File Name: Focaltech_test_ft2388.c
*
* Author: Focaltech Driver Team
*
* Created: 2020-04-23
*
* Abstract:
*
************************************************************************/

/*****************************************************************************
* Included header files
*****************************************************************************/
#include "../focaltech_test.h"

/*****************************************************************************
* Private constant and macro definitions using #define
*****************************************************************************/

/*****************************************************************************
* Private constant and macro definitions using #define
*****************************************************************************/

/*****************************************************************************
* Private enumerations, structures and unions using typedef
*****************************************************************************/

/*****************************************************************************
* Static variables
*****************************************************************************/

/*****************************************************************************
* Global variable or extern global variabls/functions
*****************************************************************************/

/*****************************************************************************
* Static function prototypes
*****************************************************************************/

/*
 * get_cb_incell - get cb data for incell IC
 */
static int get_cb_incell_ft2388(u16 saddr, int byte_num, int *cb_buf)
{
    int ret = 0;
    int i = 0;
    u8 cb_addr = 0;
    u8 addr_h = 0;
    u8 addr_l = 0;
    int read_num = 0;
    int packet_num = 0;
    int packet_remainder = 0;
    int offset = 0;
    int addr = 0;
    u8 *data = NULL;

    data = (u8 *)fts_malloc(byte_num * sizeof(u8));
    if (NULL == data) {
        FTS_TEST_SAVE_ERR("cb buffer malloc fail\n");
        return -ENOMEM;
    }

    packet_num = byte_num / BYTES_PER_TIME;
    packet_remainder = byte_num % BYTES_PER_TIME;
    if (packet_remainder)
        packet_num++;
    read_num = BYTES_PER_TIME;

    FTS_TEST_INFO("cb packet:%d,remainder:%d", packet_num, packet_remainder);
    cb_addr = FACTORY_REG_CB_ADDR;
    for (i = 0; i < packet_num; i++) {
        offset = read_num * i;
        addr = saddr + offset / 2;
        addr_h = (addr >> 8) & 0xFF;
        addr_l = addr & 0xFF;
        if ((i == (packet_num - 1)) && packet_remainder) {
            read_num = packet_remainder;
        }

        ret = fts_test_write_reg(FACTORY_REG_CB_ADDR_H, addr_h);
        if (ret) {
            FTS_TEST_SAVE_ERR("write cb addr high fail\n");
            goto TEST_CB_ERR;
        }
        ret = fts_test_write_reg(FACTORY_REG_CB_ADDR_L, addr_l);
        if (ret) {
            FTS_TEST_SAVE_ERR("write cb addr low fail\n");
            goto TEST_CB_ERR;
        }

        ret = fts_test_read(cb_addr, data + offset, read_num);
        if (ret) {
            FTS_TEST_SAVE_ERR("read cb fail\n");
            goto TEST_CB_ERR;
        }
    }

    for (i = 0; i < byte_num; i++) {
        cb_buf[i] = data[i];
    }

TEST_CB_ERR:
    fts_free(data);
    return ret;
}


static int ft2388_short_test(struct fts_test *tdata, bool *test_result)
{
    int ret = 0;
    int i = 0;
    bool tmp_result = false;
    int byte_num = 0;
    int ch_num = 0;
    int min = 0;
    int max = 0;
    int tmp_adc = 0;
    int *adcdata = NULL;
    struct incell_threshold *thr = &tdata->ic.incell.thr;

    FTS_TEST_FUNC_ENTER();
    FTS_TEST_SAVE_INFO("\n============ Test Item: Short Circuit Test\n");
    memset(tdata->buffer, 0, tdata->buffer_length);
    adcdata = tdata->buffer;

    ret = enter_factory_mode();
    if (ret < 0) {
        FTS_TEST_SAVE_ERR("enter factory mode fail,ret=%d\n", ret);
        goto test_err;
    }

    byte_num = tdata->node.node_num * 2;
    ch_num = tdata->node.rx_num;
    ret = short_get_adcdata_incell(TEST_RETVAL_AA, ch_num, byte_num, adcdata);
    if (ret < 0) {
        FTS_TEST_SAVE_ERR("get adc data fail\n");
        goto test_err;
    }

    /* calculate resistor */
    for (i = 0; i < tdata->node.node_num; i++) {
        tmp_adc = adcdata[i];
        if ((1023 - tmp_adc) <= 0) {
            adcdata[i] = thr->basic.short_res_min;
            continue;
        }
        adcdata[i] = (((205 + tmp_adc) / (1023 - tmp_adc)) * 100);
    }

    /* save */
    show_data(adcdata, false);

    /* compare */
    min = thr->basic.short_res_min;
    max = TEST_SHORT_RES_MAX * 10;
    tmp_result = compare_data(adcdata, min, max, min, max, false);

    ret = 0;
test_err:
    ret = fts_test_write_reg(FACTORY_REG_SHORT_TEST_STATE, 0x03);
    if (ret < 0) {
        FTS_TEST_SAVE_ERR("write idle to short test state fail\n");
    }

    if (tmp_result) {
        *test_result = true;
        FTS_TEST_SAVE_INFO("------ Short Circuit Test PASS\n");
    } else {
        *test_result = false;
        FTS_TEST_SAVE_INFO("------ Short Circuit Test NG\n");
    }

    /* save test data */
    fts_test_save_data("Short Circuit Test", CODE_SHORT_TEST,
                       adcdata, 0, false, false, *test_result);

    FTS_TEST_FUNC_EXIT();
    return ret;
}

static int ft2388_cb_test(struct fts_test *tdata, bool *test_result)
{
    int ret = 0;
    int i = 0;
    bool tmp_result = false;
    int byte_num = 0;
    int *cbdata = NULL;
    struct incell_threshold *thr = &tdata->ic.incell.thr;

    FTS_TEST_FUNC_ENTER();
    FTS_TEST_SAVE_INFO("\n============ Test Item: CB Test\n");
    memset(tdata->buffer, 0, tdata->buffer_length);
    cbdata = tdata->buffer;

    if (!thr->cb_min || !thr->cb_max) {
        FTS_TEST_SAVE_ERR("cb_min/max is null\n");
        ret = -EINVAL;
        goto test_err;
    }

    ret = enter_factory_mode();
    if (ret < 0) {
        FTS_TEST_SAVE_ERR("enter factory mode fail,ret=%d\n", ret);
        goto test_err;
    }

    /* cb test enable */
    ret = fts_test_write_reg(FACTORY_REG_CB_TEST_EN, 0x01);
    if (ret < 0) {
        FTS_TEST_SAVE_ERR("cb test enable fail\n");
        goto test_err;
    }

    /* auto clb */
    ret = chip_clb();
    if (ret < 0) {
        FTS_TEST_SAVE_ERR("auto clb fail\n");
        goto test_err;
    }


    ret = fts_test_write_reg(FACTORY_REG_CLB, 0x05);
    if (ret < 0) {
        FTS_TEST_SAVE_ERR("cb test enable fail\n");
        goto test_err;
    }

    byte_num = tdata->node.node_num * 2;
    ret = get_cb_incell_ft2388(0, byte_num, cbdata);
    if (ret < 0) {
        FTS_TEST_SAVE_ERR("get cb fail\n");
        goto test_err;
    }

    for (i = 0; i < byte_num; i = i + 2) {
        cbdata[i >> 1] = (int)(short)((cbdata[i] << 8) + cbdata[i + 1]);
    }

    /* save */
    show_data(cbdata, false);

    /* compare */
    tmp_result = compare_array(cbdata, thr->cb_min, thr->cb_max, false);

test_err:
    /* cb test disable */
    ret = fts_test_write_reg(FACTORY_REG_CB_TEST_EN, 0x00);
    if (ret < 0) {
        FTS_TEST_SAVE_ERR("cb test disable fail\n");
    }

    if (tmp_result) {
        *test_result = true;
        FTS_TEST_SAVE_INFO("------ CB Test PASS\n");
    } else {
        *test_result = false;
        FTS_TEST_SAVE_INFO("------ CB Test NG\n");
    }

    /*save test data*/
    fts_test_save_data("CB Test", CODE_CB_TEST,
                       cbdata, 0, false, false, *test_result);
    FTS_TEST_FUNC_EXIT();
    return ret;
}

static int ft2388_rawdata_test(struct fts_test *tdata, bool *test_result)
{
    int ret = 0;
    bool tmp_result = false;
    int *rawdata = NULL;
    struct incell_threshold *thr = &tdata->ic.incell.thr;

    FTS_TEST_FUNC_ENTER();
    FTS_TEST_SAVE_INFO("\n============ Test Item: RawData Test\n");
    memset(tdata->buffer, 0, tdata->buffer_length);
    rawdata = tdata->buffer;

    if (!thr->rawdata_min || !thr->rawdata_max) {
        FTS_TEST_SAVE_ERR("rawdata_min/max is null\n");
        ret = -EINVAL;
        goto test_err;
    }

    ret = enter_factory_mode();
    if (ret < 0) {
        FTS_TEST_SAVE_ERR("enter factory mode fail,ret=%d\n", ret);
        goto test_err;
    }

    /* read rawdata */
    ret = get_rawdata(rawdata);
    if (ret < 0) {
        FTS_TEST_SAVE_ERR("get RawData fail,ret=%d\n", ret);
        goto test_err;
    }

    /* save */
    show_data(rawdata, false);

    /* compare */
    tmp_result = compare_array(rawdata,
                               thr->rawdata_min,
                               thr->rawdata_max,
                               false);
    ret = 0;

test_err:
    /* rawdata test disble */

    if (tmp_result) {
        *test_result = true;
        FTS_TEST_SAVE_INFO("------ RawData Test PASS\n");
    } else {
        *test_result = false;
        FTS_TEST_SAVE_INFO("------ RawData Test NG\n");
    }

    /*save test data*/
    fts_test_save_data("RawData Test", CODE_RAWDATA_TEST,
                       rawdata, 0, false, false, *test_result);
    FTS_TEST_FUNC_EXIT();
    return ret;
}

static int ft2388_panel_differ_test(struct fts_test *tdata, bool *test_result)
{
	int ret = 0;
	int i = 0;
	int j = 0;
	u8 value = 0;
	u8 vref = 0;
	bool tmp_result = false;
	int *rawdata = NULL;
	int *base_raw = NULL;
	int *rawdata_avg = NULL;
	struct incell_threshold *thr = &tdata->ic.incell.thr;

	FTS_TEST_FUNC_ENTER();
	FTS_TEST_SAVE_INFO("\n============ Test Item: Panel Diff Test\n");
	memset(tdata->buffer, 0, tdata->buffer_length);
	rawdata = tdata->buffer;

	base_raw = (int *)fts_malloc(tdata->buffer_length);
	if (NULL == base_raw) {
        FTS_TEST_ERROR("base_raw malloc fail");
        goto test_err;
    }
	rawdata_avg = (int *)fts_malloc(tdata->buffer_length);
	if (NULL == rawdata_avg) {
        FTS_TEST_ERROR("rawdata_avg malloc fail");
       goto test_err;
    }

	ret = enter_factory_mode();
	if (ret < 0) {
		FTS_TEST_SAVE_ERR("enter factory mode fail,ret=%d\n", ret);
		goto test_err;
	}

	/* read rawdata */
	for(i = 0; i < 10; i++){
		memset(tdata->buffer, 0, tdata->buffer_length);
		ret = get_rawdata(rawdata);
		if (ret < 0) {
			FTS_TEST_SAVE_ERR("get RawData fail,ret=%d\n", ret);
			goto test_err;
		}
		for(j = 0; j < tdata->node.node_num; j++){
			base_raw[j] += rawdata[j];
		}
		
	}
	for(i = 0; i < tdata->node.node_num; i++){
			base_raw[i] = base_raw[i] / 10;
	}
	fts_test_read_reg(0x86, &value);
	vref = value - thr->basic.delta_vol;
	fts_test_write_reg(0x86, vref);

	for(i = 0; i < 10; i++){
		ret = get_rawdata(rawdata);
		if (ret < 0) {
			FTS_TEST_SAVE_ERR("get RawData fail,ret=%d\n", ret);
			goto test_err;
		}
		for(j = 0; j < tdata->node.node_num; j++){
			rawdata_avg[j] += rawdata[j];
		}
		
	}
	for(i = 0; i < tdata->node.node_num; i++){
			rawdata_avg[i] = rawdata_avg[i] / 10;
	}
	memset(tdata->buffer, 0, tdata->buffer_length);

	for(i = 0; i < tdata->node.node_num; i++){
			rawdata[i] = base_raw[i] - rawdata_avg[i];
	}

	/* save */
	show_data(rawdata, false);

	/* compare */
	tmp_result = compare_array(rawdata,
                               thr->panel_differ_min,
                               thr->panel_differ_max,
                               false);
	ret = 0;

test_err:
	/* rawdata test disble */

	if (tmp_result) {
		*test_result = true;
		FTS_TEST_SAVE_INFO("------ Panel Diff Test PASS\n");
	} else {
		*test_result = false;
		FTS_TEST_SAVE_INFO("------ Panel Diff Test NG\n");
	}

	fts_free(base_raw);
	fts_free(rawdata_avg);
	if(value){
		fts_test_read_reg(0x86, &value);
	}
	/*save test data*/
	fts_test_save_data("Panel Diff Test", CODE_PANEL_DIFFER_TEST,
					   rawdata, 0, false, false, *test_result);
	FTS_TEST_FUNC_EXIT();
	return ret;
}



static int ft2388_lcdnoise_test(struct fts_test *tdata, bool *test_result)
{
    int ret = 0;
    int i = 0;
    bool tmp_result = false;
    u8 old_mode = 0;
    u8 status = 0;
    int byte_num = 0;
    int frame_num = 0;
    int max = 0;
    int *lcdnoise = NULL;
    struct incell_threshold *thr = &tdata->ic.incell.thr;

    FTS_TEST_FUNC_ENTER();
    FTS_TEST_SAVE_INFO("\n============ Test Item: LCD Noise Test\n");
    memset(tdata->buffer, 0, tdata->buffer_length);
    lcdnoise = tdata->buffer;

    ret = enter_factory_mode();
    if (ret < 0) {
        FTS_TEST_SAVE_ERR("enter factory mode fail,ret=%d\n", ret);
        goto test_err;
    }

    ret = fts_test_read_reg(FACTORY_REG_DATA_SELECT, &old_mode);
    if (ret < 0) {
        FTS_TEST_SAVE_ERR("read reg06 fail\n");
        goto test_err;
    }

    ret =  fts_test_write_reg(FACTORY_REG_DATA_SELECT, 0x01);
    if (ret < 0) {
        FTS_TEST_SAVE_ERR("write 1 to reg06 fail\n");
        goto test_err;
    }

    ret =  fts_test_write_reg(FACTORY_REG_LINE_ADDR, 0xE1);
    if (ret < 0) {
        FTS_TEST_SAVE_ERR("write reg01 fail\n");
        goto test_err;
    }

    ret =  fts_test_write_reg(FACTORY_REG_LINE_ADDR, 0xAD);
    if (ret < 0) {
        FTS_TEST_SAVE_ERR("write reg01 fail\n");
        goto test_err;
    }

    frame_num = thr->basic.lcdnoise_frame;
    ret = fts_test_write_reg(FACTORY_REG_LCD_NOISE_FRAME, frame_num / 4);
    if (ret < 0) {
        FTS_TEST_SAVE_INFO("write frame num fail\n");
        goto test_err;
    }

    /* start test */
    ret = fts_test_write_reg(FACTORY_REG_LCD_NOISE_START, 0x01);
    if (ret < 0) {
        FTS_TEST_SAVE_INFO("start lcdnoise test fail\n");
        goto test_err;
    }

    /* check test status */
    sys_delay(frame_num * FACTORY_TEST_DELAY / 2);
    for (i = 0; i < FACTORY_TEST_RETRY; i++) {
        status = 0xFF;
        ret = fts_test_read_reg(FACTORY_REG_LCD_NOISE_TEST_STATE, &status);
        if ((ret >= 0) && (TEST_RETVAL_AA == status)) {
            break;
        } else {
            FTS_TEST_DBG("reg%x=%x,retry:%d\n",
                         FACTORY_REG_LCD_NOISE_TEST_STATE, status, i);
        }
        sys_delay(FACTORY_TEST_RETRY_DELAY);
    }
    if (i >= FACTORY_TEST_RETRY) {
        FTS_TEST_SAVE_ERR("lcdnoise test timeout\n");
        goto test_err;
    }

    /* read lcdnoise */
    byte_num = tdata->node.node_num * 2;
    ret = read_mass_data(FACTORY_REG_RAWDATA_ADDR, byte_num, lcdnoise);
    if (ret < 0) {
        FTS_TEST_SAVE_ERR("read rawdata fail\n");
        goto test_err;
    }

    /* save */
    show_data(lcdnoise, false);

    /* compare */
    max = thr->basic.lcdnoise_coefficient * tdata->va_touch_thr * 32 / 100;
    FTS_TEST_DBG("touch thr:%d, max:%d", tdata->va_touch_thr, max);
    tmp_result = compare_data(lcdnoise, 0, max, 0, 0, false);

test_err:
    ret = fts_test_write_reg(FACTORY_REG_LCD_NOISE_START, 0x00);
    if (ret < 0) {
        FTS_TEST_SAVE_ERR("write 0 to reg11 fail\n");
    }

    ret = fts_test_write_reg(FACTORY_REG_LINE_ADDR, 0xE0);
    if (ret < 0) {
        FTS_TEST_SAVE_ERR("write 0 to reg01 fail\n");
    }

    ret = fts_test_write_reg(FACTORY_REG_DATA_SELECT, old_mode);
    if (ret < 0) {
        FTS_TEST_SAVE_ERR("restore reg06 fail\n");
    }

    ret = fts_test_write_reg(FACTORY_REG_LCD_NOISE_TEST_STATE, 0x03);
    if (ret < 0) {
        FTS_TEST_SAVE_ERR("write idle to lcdnoise test state fail\n");
    }

    if (tmp_result) {
        *test_result = true;
        FTS_TEST_SAVE_INFO("------ LCD Noise Test PASS\n");
    } else {
        *test_result = false;
        FTS_TEST_SAVE_INFO("------ LCD Noise Test NG\n");
    }

    /*save test data*/
    fts_test_save_data("LCD Noise Test", CODE_LCD_NOISE_TEST,
                       lcdnoise, 0, false, false, *test_result);
    FTS_TEST_FUNC_EXIT();
    return ret;
}

static int start_test_ft2388(void)
{
    int ret = 0;
    struct fts_test *tdata = fts_ftest;
    struct incell_testitem *test_item = &tdata->ic.incell.u.item;
    bool temp_result = false;
    bool test_result = true;

    FTS_TEST_FUNC_ENTER();
    FTS_TEST_INFO("test item:0x%x", fts_ftest->ic.incell.u.tmp);
    if (!tdata || !tdata->testresult || !tdata->buffer) {
        FTS_TEST_ERROR("tdata is null");
        return -EINVAL;
    }
    /* cb test */
    if (true == test_item->cb_test) {
        ret = ft2388_cb_test(tdata, &temp_result);
        if ((ret < 0) || (false == temp_result)) {
            test_result = false;
        }
    }

    /* rawdata test */
    if (true == test_item->rawdata_test) {
        ret = ft2388_rawdata_test(tdata, &temp_result);
        if ((ret < 0) || (false == temp_result)) {
            test_result = false;
        }
    }

	/* panel differ test */
	if (true == test_item->panel_differ_test) {
        ret = ft2388_panel_differ_test(tdata, &temp_result);
        if ((ret < 0) || (false == temp_result)) {
            test_result = false;
        }
    }

    /* short test */
    if (true == test_item->short_test) {
        ret = ft2388_short_test(tdata, &temp_result);
        if ((ret < 0) || (false == temp_result)) {
            test_result = false;
        }
    }

    /* lcd noise test */
    if (true == test_item->lcdnoise_test) {
        ret = ft2388_lcdnoise_test(tdata, &temp_result);
        if ((ret < 0) || (false == temp_result)) {
            test_result = false;
        }
    }
    return test_result;
}

static int param_init_ft2388(void)
{
    struct incell_threshold *thr = &fts_ftest->ic.incell.thr;

    get_value_basic("PanelDifferTest_DifferThreshold", &thr->basic.panel_differ);
	get_value_basic("PanelDifferTest_DeltaVol", &thr->basic.delta_vol);
	
    return 0;
}


struct test_funcs test_func_ft2388 = {
    .ctype = {0x1E},
    .hwtype = IC_HW_INCELL,
    .key_num_total = 0,
    .start_test = start_test_ft2388,
    .param_init = param_init_ft2388,
};

