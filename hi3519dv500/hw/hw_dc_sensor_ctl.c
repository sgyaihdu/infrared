/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef OT_GPIO_I2C
#include "gpioi2c_ex.h"
#else
#include "ot_i2c.h"
#endif
#include "securec.h"

#include "hw_dc_cmos.h"

#define I2C_DEV_FILE_NUM     16
#define I2C_BUF_NUM          8

static int g_fd[OT_ISP_MAX_PIPE_NUM] = {[0 ...(OT_ISP_MAX_PIPE_NUM - 1)] = -1};

int hw_dc_i2c_init(ot_vi_pipe vi_pipe)
{
    if (g_fd[vi_pipe] >= 0) {
        return TD_SUCCESS;
    }
#ifdef OT_GPIO_I2C
    g_fd[vi_pipe] = open("/dev/gpioi2c_ex", O_RDONLY, S_IRUSR);
    if (g_fd[vi_pipe] < 0) {
        isp_err_trace("Open gpioi2c_ex error!\n");
        return TD_FAILURE;
    }
#else
    int ret;
    char dev_file[I2C_DEV_FILE_NUM] = {0};
    td_u8 dev_num;
    ot_isp_sns_commbus *hw_dcbusinfo = TD_NULL;
    hw_dcbusinfo = hw_dc_get_bus_info(vi_pipe);
    dev_num = hw_dcbusinfo->i2c_dev;
    (td_void)snprintf_s(dev_file, sizeof(dev_file), sizeof(dev_file) - 1, "/dev/i2c-%u", dev_num);

    g_fd[vi_pipe] = open(dev_file, O_RDWR, S_IRUSR | S_IWUSR);
    if (g_fd[vi_pipe] < 0) {
        isp_err_trace("Open /dev/ot_i2c_drv-%u error!\n", dev_num);
        return TD_FAILURE;
    }

    ret = ioctl(g_fd[vi_pipe], OT_I2C_SLAVE_FORCE, (HW_DC_I2C_ADDR >> 1));
    if (ret < 0) {
        isp_err_trace("I2C_SLAVE_FORCE error!\n");
        close(g_fd[vi_pipe]);
        g_fd[vi_pipe] = -1;
        return ret;
    }
#endif

    return TD_SUCCESS;
}

int hw_dc_i2c_exit(ot_vi_pipe vi_pipe)
{
    if (g_fd[vi_pipe] >= 0) {
        close(g_fd[vi_pipe]);
        g_fd[vi_pipe] = -1;
        return TD_SUCCESS;
    }
    return TD_FAILURE;
}

td_s32 hw_dc_read_register(ot_vi_pipe vi_pipe, td_u32 addr)
{
    ot_unused(vi_pipe);
    ot_unused(addr);
    return TD_SUCCESS;
}

td_s32 hw_dc_write_register(ot_vi_pipe vi_pipe, td_u32 addr, td_u32 data)
{
        return TD_SUCCESS;
    td_s32 ret;
    if (g_fd[vi_pipe] < 0) {
        return TD_SUCCESS;
    }

#ifdef OT_GPIO_I2C
    i2c_data.dev_addr = HW_DC_I2C_ADDR;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = HW_DC_ADDR_BYTE;
    i2c_data.data = data;
    i2c_data.data_byte_num = HW_DC_DATA_BYTE;

    ret = ioctl(g_fd[vi_pipe], GPIO_I2C_WRITE, &i2c_data);
    if (ret) {
        isp_err_trace("GPIO-I2C write failed!\n");
        return ret;
    }
#else
    td_u32 idx = 0;
    td_u8 buf[I2C_BUF_NUM];

    if (HW_DC_ADDR_BYTE == 2) {  /* 2 byte */
        buf[idx] = (addr >> 8) & 0xff;  /* shift 8 */
        idx++;
        buf[idx] = addr & 0xff;
        idx++;
    } else {
    }

    if (HW_DC_DATA_BYTE == 2) {  /* 2 byte */
    } else {
        buf[idx] = data & 0xff;
        idx++;
    }

    ret = write(g_fd[vi_pipe], buf, HW_DC_ADDR_BYTE + HW_DC_DATA_BYTE);
    if (ret < 0) {
        isp_err_trace("I2C_WRITE error!\n");
        return TD_FAILURE;
    }

#endif
    return TD_SUCCESS;
}

static void delay_ms(int ms)
{
    usleep(ms * 1000); /* 1ms: 1000us */
    return;
}

void hw_dc_prog(ot_vi_pipe vi_pipe, const td_u32 *rom)
{
    ot_unused(vi_pipe);
    ot_unused(rom);
}


void hw_dc_standby(ot_vi_pipe vi_pipe)
{
        return ;
    td_s32 ret = TD_SUCCESS;
    ret += hw_dc_write_register(vi_pipe, 0x3000, 0x01);  /* STANDBY */
    ret += hw_dc_write_register(vi_pipe, 0x3002, 0x01);  /* XTMSTA */
    if (ret != TD_SUCCESS) {
        isp_err_trace("write register failed!\n");
    }
    return;
}

void hw_dc_restart(ot_vi_pipe vi_pipe)
{
    td_s32 ret = TD_SUCCESS;
    ret += hw_dc_write_register(vi_pipe, 0x3000, 0x00);  /* standby */
    delay_ms(20); /* 20ms */
    ret += hw_dc_write_register(vi_pipe, 0x3002, 0x00);  /* master mode start */
    if (ret != TD_SUCCESS) {
        isp_err_trace("write register failed!\n");
    }
    return;
}

void hw_dc_mirror_flip(ot_vi_pipe vi_pipe, ot_isp_sns_mirrorflip_type sns_mirror_flip)
{
    switch (sns_mirror_flip) {
        case ISP_SNS_NORMAL:
            hw_dc_write_register(vi_pipe, 0x3030, 0x00);
            break;
        case ISP_SNS_MIRROR:
            hw_dc_write_register(vi_pipe, 0x3030, 0x01);
            break;
        case ISP_SNS_FLIP:
            hw_dc_write_register(vi_pipe, 0x3030, 0x02);
            break;
        case ISP_SNS_MIRROR_FLIP:
            hw_dc_write_register(vi_pipe, 0x3030, 0x03);
            break;
        default:
            break;
    }
    return;
}

td_void hw_dc_blc_clamp(ot_vi_pipe vi_pipe, ot_isp_sns_blc_clamp blc_clamp)
{
        return ;
    td_s32 ret = TD_SUCCESS;

    hw_dc_set_blc_clamp_value(vi_pipe, blc_clamp.blc_clamp_en);

    if (blc_clamp.blc_clamp_en == TD_TRUE) {
        ret += hw_dc_write_register(vi_pipe, 0x300e, 0x01);
    } else {
        ret += hw_dc_write_register(vi_pipe, 0x300e, 0x00);
    }

    if (ret != TD_SUCCESS) {
        isp_err_trace("write register failed!\n");
    }
    return;
}

void hw_dc_comm_init(ot_vi_pipe vi_pipe);
void hw_dc_linear_8m_init(ot_vi_pipe vi_pipe);

void hw_dc_default_reg_init(ot_vi_pipe vi_pipe)
{
    td_u32 i;
    td_s32 ret = TD_SUCCESS;
    ot_isp_sns_state *pasthw_dc = TD_NULL;
    pasthw_dc = hw_dc_get_ctx(vi_pipe);
    for (i = 0; i < pasthw_dc->regs_info[0].reg_num; i++) {
        ret += hw_dc_write_register(vi_pipe,
                                     pasthw_dc->regs_info[0].i2c_data[i].reg_addr,
                                     pasthw_dc->regs_info[0].i2c_data[i].data);
    }
    if (ret != TD_SUCCESS) {
        isp_err_trace("write register failed!\n");
    }
    return;
}

void hw_dc_init(ot_vi_pipe vi_pipe)
{

    return;
    ot_wdr_mode wdr_mode;
    td_bool          init;
    td_s32 ret;
    ot_isp_sns_state *pasthw_dc = TD_NULL;
    pasthw_dc = hw_dc_get_ctx(vi_pipe);
    init       = pasthw_dc->init;
    wdr_mode   = pasthw_dc->wdr_mode;

    ret = hw_dc_i2c_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("i2c init failed!\n");
        return;
    }
    /* When sensor first init, config all registers */
    if (init == TD_FALSE) {
        if (OT_WDR_MODE_2To1_LINE == wdr_mode) {
        } else {
            hw_dc_linear_8m_init(vi_pipe);
        }
    } else {
        /* When sensor switch mode(linear<->WDR or resolution), config different registers(if possible) */
        if (OT_WDR_MODE_2To1_LINE == wdr_mode) {
        } else {
            hw_dc_linear_8m_init(vi_pipe);
        }
    }

    pasthw_dc->init = TD_TRUE;
    return;
}

void hw_dc_exit(ot_vi_pipe vi_pipe)
{
    return;
    td_s32 ret;
    ret = hw_dc_i2c_exit(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("HW_DC exit failed!\n");
    }
    return;
}

static td_s32 hw_dc_linear_2160p30_init_part1(ot_vi_pipe vi_pipe)
{
    td_s32 ret = TD_SUCCESS;
    ret += hw_dc_write_register(vi_pipe, 0x3008, 0x7F);
    ret += hw_dc_write_register(vi_pipe, 0x300A, 0x5B);
    ret += hw_dc_write_register(vi_pipe, 0x3024, 0xE4);
    ret += hw_dc_write_register(vi_pipe, 0x3025, 0x0C);
    ret += hw_dc_write_register(vi_pipe, 0x3028, 0xEE);
    ret += hw_dc_write_register(vi_pipe, 0x3033, 0x08);
    ret += hw_dc_write_register(vi_pipe, 0x3050, 0x06);
    ret += hw_dc_write_register(vi_pipe, 0x3051, 0x09);
    ret += hw_dc_write_register(vi_pipe, 0x3090, 0x14);
    ret += hw_dc_write_register(vi_pipe, 0x30C1, 0x00);
    ret += hw_dc_write_register(vi_pipe, 0x3116, 0x24);
    ret += hw_dc_write_register(vi_pipe, 0x3118, 0xA0);
    ret += hw_dc_write_register(vi_pipe, 0x311E, 0x24);
    ret += hw_dc_write_register(vi_pipe, 0x32D4, 0x21);
    ret += hw_dc_write_register(vi_pipe, 0x32EC, 0xA1);
    ret += hw_dc_write_register(vi_pipe, 0x344C, 0x2B);
    ret += hw_dc_write_register(vi_pipe, 0x344D, 0x01);
    ret += hw_dc_write_register(vi_pipe, 0x344E, 0xED);
    ret += hw_dc_write_register(vi_pipe, 0x344F, 0x01);
    ret += hw_dc_write_register(vi_pipe, 0x3450, 0xF6);
    ret += hw_dc_write_register(vi_pipe, 0x3451, 0x02);
    ret += hw_dc_write_register(vi_pipe, 0x3452, 0x7F);
    ret += hw_dc_write_register(vi_pipe, 0x3453, 0x03);
    ret += hw_dc_write_register(vi_pipe, 0x358A, 0x04);
    ret += hw_dc_write_register(vi_pipe, 0x35A1, 0x02);
    ret += hw_dc_write_register(vi_pipe, 0x35EC, 0x27);
    ret += hw_dc_write_register(vi_pipe, 0x35EE, 0x8D);
    ret += hw_dc_write_register(vi_pipe, 0x35F0, 0x8D);
    ret += hw_dc_write_register(vi_pipe, 0x35F2, 0x29);
    ret += hw_dc_write_register(vi_pipe, 0x36BC, 0x0C);
    ret += hw_dc_write_register(vi_pipe, 0x36CC, 0x53);
    ret += hw_dc_write_register(vi_pipe, 0x36CD, 0x00);
    ret += hw_dc_write_register(vi_pipe, 0x36CE, 0x3C);
    ret += hw_dc_write_register(vi_pipe, 0x36D0, 0x8C);
    ret += hw_dc_write_register(vi_pipe, 0x36D1, 0x00);
    ret += hw_dc_write_register(vi_pipe, 0x36D2, 0x71);
    ret += hw_dc_write_register(vi_pipe, 0x36D4, 0x3C);
    ret += hw_dc_write_register(vi_pipe, 0x36D6, 0x53);
    ret += hw_dc_write_register(vi_pipe, 0x36D7, 0x00);
    ret += hw_dc_write_register(vi_pipe, 0x36D8, 0x71);
    ret += hw_dc_write_register(vi_pipe, 0x36DA, 0x8C);
    ret += hw_dc_write_register(vi_pipe, 0x36DB, 0x00);

    return ret;
}

static td_s32 hw_dc_linear_2160p30_init_part2(ot_vi_pipe vi_pipe)
{
    td_s32 ret = TD_SUCCESS;

    ret += hw_dc_write_register(vi_pipe, 0x3720, 0x00);
    ret += hw_dc_write_register(vi_pipe, 0x3724, 0x02);
    ret += hw_dc_write_register(vi_pipe, 0x3726, 0x02);
    ret += hw_dc_write_register(vi_pipe, 0x3732, 0x02);
    ret += hw_dc_write_register(vi_pipe, 0x3734, 0x03);
    ret += hw_dc_write_register(vi_pipe, 0x3736, 0x03);
    ret += hw_dc_write_register(vi_pipe, 0x3742, 0x03);
    ret += hw_dc_write_register(vi_pipe, 0x3862, 0xE0);
    ret += hw_dc_write_register(vi_pipe, 0x38CC, 0x30);
    ret += hw_dc_write_register(vi_pipe, 0x38CD, 0x2F);
    ret += hw_dc_write_register(vi_pipe, 0x395C, 0x0C);
    ret += hw_dc_write_register(vi_pipe, 0x39A4, 0x07);
    ret += hw_dc_write_register(vi_pipe, 0x39A8, 0x32);
    ret += hw_dc_write_register(vi_pipe, 0x39AA, 0x32);
    ret += hw_dc_write_register(vi_pipe, 0x39AC, 0x32);
    ret += hw_dc_write_register(vi_pipe, 0x39AE, 0x32);
    ret += hw_dc_write_register(vi_pipe, 0x39B0, 0x32);
    ret += hw_dc_write_register(vi_pipe, 0x39B2, 0x2F);
    ret += hw_dc_write_register(vi_pipe, 0x39B4, 0x2D);
    ret += hw_dc_write_register(vi_pipe, 0x39B6, 0x28);
    ret += hw_dc_write_register(vi_pipe, 0x39B8, 0x30);
    ret += hw_dc_write_register(vi_pipe, 0x39BA, 0x30);
    ret += hw_dc_write_register(vi_pipe, 0x39BC, 0x30);
    ret += hw_dc_write_register(vi_pipe, 0x39BE, 0x30);
    ret += hw_dc_write_register(vi_pipe, 0x39C0, 0x30);
    ret += hw_dc_write_register(vi_pipe, 0x39C2, 0x2E);
    ret += hw_dc_write_register(vi_pipe, 0x39C4, 0x2B);
    ret += hw_dc_write_register(vi_pipe, 0x39C6, 0x25);
    ret += hw_dc_write_register(vi_pipe, 0x3A42, 0xD1);
    ret += hw_dc_write_register(vi_pipe, 0x3A4C, 0x77);
    ret += hw_dc_write_register(vi_pipe, 0x3AE0, 0x02);
    ret += hw_dc_write_register(vi_pipe, 0x3AEC, 0x0C);
    ret += hw_dc_write_register(vi_pipe, 0x3B00, 0x2E);
    ret += hw_dc_write_register(vi_pipe, 0x3B06, 0x29);
    ret += hw_dc_write_register(vi_pipe, 0x3B98, 0x25);
    ret += hw_dc_write_register(vi_pipe, 0x3B99, 0x21);
    ret += hw_dc_write_register(vi_pipe, 0x3B9B, 0x13);
    ret += hw_dc_write_register(vi_pipe, 0x3B9C, 0x13);
    ret += hw_dc_write_register(vi_pipe, 0x3B9D, 0x13);
    ret += hw_dc_write_register(vi_pipe, 0x3B9E, 0x13);

    return ret;
}

static td_s32 hw_dc_linear_2160p30_init_part3(ot_vi_pipe vi_pipe)
{
    td_s32 ret = TD_SUCCESS;

    ret += hw_dc_write_register(vi_pipe, 0x3BA1, 0x00);
    ret += hw_dc_write_register(vi_pipe, 0x3BA2, 0x06);
    ret += hw_dc_write_register(vi_pipe, 0x3BA3, 0x0B);
    ret += hw_dc_write_register(vi_pipe, 0x3BA4, 0x10);
    ret += hw_dc_write_register(vi_pipe, 0x3BA5, 0x14);
    ret += hw_dc_write_register(vi_pipe, 0x3BA6, 0x18);
    ret += hw_dc_write_register(vi_pipe, 0x3BA7, 0x1A);
    ret += hw_dc_write_register(vi_pipe, 0x3BA8, 0x1A);
    ret += hw_dc_write_register(vi_pipe, 0x3BA9, 0x1A);
    ret += hw_dc_write_register(vi_pipe, 0x3BAC, 0xED);
    ret += hw_dc_write_register(vi_pipe, 0x3BAD, 0x01);
    ret += hw_dc_write_register(vi_pipe, 0x3BAE, 0xF6);
    ret += hw_dc_write_register(vi_pipe, 0x3BAF, 0x02);
    ret += hw_dc_write_register(vi_pipe, 0x3BB0, 0xA2);
    ret += hw_dc_write_register(vi_pipe, 0x3BB1, 0x03);
    ret += hw_dc_write_register(vi_pipe, 0x3BB2, 0xE0);
    ret += hw_dc_write_register(vi_pipe, 0x3BB3, 0x03);
    ret += hw_dc_write_register(vi_pipe, 0x3BB4, 0xE0);
    ret += hw_dc_write_register(vi_pipe, 0x3BB5, 0x03);
    ret += hw_dc_write_register(vi_pipe, 0x3BB6, 0xE0);
    ret += hw_dc_write_register(vi_pipe, 0x3BB7, 0x03);
    ret += hw_dc_write_register(vi_pipe, 0x3BB8, 0xE0);
    ret += hw_dc_write_register(vi_pipe, 0x3BBA, 0xE0);
    ret += hw_dc_write_register(vi_pipe, 0x3BBC, 0xDA);
    ret += hw_dc_write_register(vi_pipe, 0x3BBE, 0x88);
    ret += hw_dc_write_register(vi_pipe, 0x3BC0, 0x44);
    ret += hw_dc_write_register(vi_pipe, 0x3BC2, 0x7B);
    ret += hw_dc_write_register(vi_pipe, 0x3BC4, 0xA2);
    ret += hw_dc_write_register(vi_pipe, 0x3BC8, 0xBD);
    ret += hw_dc_write_register(vi_pipe, 0x3BCA, 0xBD);
    ret += hw_dc_write_register(vi_pipe, 0x4004, 0x48);
    ret += hw_dc_write_register(vi_pipe, 0x4005, 0x09);
    ret += hw_dc_write_register(vi_pipe, 0x4018, 0xA7);
    ret += hw_dc_write_register(vi_pipe, 0x401A, 0x57);
    ret += hw_dc_write_register(vi_pipe, 0x401C, 0x5F);
    ret += hw_dc_write_register(vi_pipe, 0x401E, 0x97);
    ret += hw_dc_write_register(vi_pipe, 0x4020, 0x5F);
    ret += hw_dc_write_register(vi_pipe, 0x4022, 0xAF);
    ret += hw_dc_write_register(vi_pipe, 0x4024, 0x5F);
    ret += hw_dc_write_register(vi_pipe, 0x4026, 0x9F);
    ret += hw_dc_write_register(vi_pipe, 0x4028, 0x4F);

    return ret;
}

void hw_dc_linear_8m_init(ot_vi_pipe vi_pipe)
{
    td_s32 ret = TD_SUCCESS;

    ret += hw_dc_linear_2160p30_init_part1(vi_pipe);
    ret += hw_dc_linear_2160p30_init_part2(vi_pipe);
    ret += hw_dc_linear_2160p30_init_part3(vi_pipe);

    hw_dc_default_reg_init(vi_pipe);

    ret += hw_dc_write_register(vi_pipe, 0x3000, 0x00);  /* standby */
    delay_ms(20); /* 20ms */
    ret += hw_dc_write_register(vi_pipe, 0x3002, 0x00);  /* master mode start */
    if (ret != TD_SUCCESS) {
        isp_err_trace("hw_dc write register failed!\n");
        return;
    }
    printf("===HW_DC 8M 30fps 12bit LINE Init OK!===\n");
    return;
}
