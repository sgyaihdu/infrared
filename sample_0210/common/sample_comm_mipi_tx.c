/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include "sample_comm.h"
#include "hi_mipi_tx.h"

#include "ot_i2c.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if VO_MIPI_SUPPORT

#ifdef OT_FPGA
#define SAMPLE_COMM_MIPI_TX_MAX_PHY_DATA_RATE 594
#else
#define SAMPLE_COMM_MIPI_TX_MAX_PHY_DATA_RATE 945
#endif


#define I2C_ADDR_LEN 2
#define I2C_DATA_LEN 2
#define I2C_M_RD 0x0001
#define I2C_RDWR 0x0707
#define I2C_SLAVE_FORCE	0x0706
static hi_s32 g_sample_comm_mipi_fd = HI_INVALID_VALUE;

static const combo_dev_cfg_t g_sample_comm_mipi_tx_720x576_50_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 64,
        .hbp  = 68,
        .hact = 720,
        .hfp  = 12,
        .vpw   = 5,
        .vbp   = 39,
        .vact  = 576,
        .vfp   = 5,
    },
    .phy_data_rate = 162,
    .pixel_clk = 27000,
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_1024x768_60_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 136,
        .hbp  = 160,
        .hact = 1024,
        .hfp  = 24,
        .vpw   = 6,
        .vbp   = 29,
        .vact  = 768,
        .vfp   = 3,
    },
    .phy_data_rate = 390,
    .pixel_clk = 65000,
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_1280x720_50_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 40,
        .hbp  = 220,
        .hact = 1280,
        .hfp  = 440,
        .vpw   = 5,
        .vbp   = 20,
        .vact  = 720,
        .vfp   = 5,
    },
    .phy_data_rate = 446,
    .pixel_clk = 74250,
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_1280x720_60_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 40,
        .hbp  = 220,
        .hact = 1280,
        .hfp  = 110,
        .vpw   = 5,
        .vbp   = 20,
        .vact  = 720,
        .vfp   = 5,
    },
    .phy_data_rate = 446,
    .pixel_clk = 74250,
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_1280x1024_60_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 112,
        .hbp  = 248,
        .hact = 1280,
        .hfp  = 48,
        .vpw   = 3,
        .vbp   = 38,
        .vact  = 1024,
        .vfp   = 1,
    },
    .phy_data_rate = 648, /* 486 */
    .pixel_clk = 108000,
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_1920x1080_24_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 44,
        .hbp  = 148,
        .hact = 1920,
        .hfp  = 638,
        .vpw   = 5,
        .vbp   = 36,
        .vact  = 1080,
        .vfp   = 4,
    },
    .phy_data_rate = 446,
    .pixel_clk = 74250,
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_1920x1080_25_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 44,
        .hbp  = 148,
        .hact = 1920,
        .hfp  = 528,
        .vpw   = 5,
        .vbp   = 36,
        .vact  = 1080,
        .vfp   = 4,
    },
    .phy_data_rate = 446,
    .pixel_clk = 74250,
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_1920x1080_30_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 44,
        .hbp  = 148,
        .hact = 1920,
        .hfp  = 88,
        .vpw   = 5,
        .vbp   = 36,
        .vact  = 1080,
        .vfp   = 4,
    },
    .phy_data_rate = 446,
    .pixel_clk = 74250,
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_1920x1080_50_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 44,
        .hbp  = 148,
        .hact = 1920,
        .hfp  = 528,
        .vpw   = 5,
        .vbp   = 36,
        .vact  = 1080,
        .vfp   = 4,
    },
    .phy_data_rate = 891,
    .pixel_clk = 148500,
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_1920x1080_60_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 44,
        .hbp  = 148,
        .hact = 1920,
        .hfp  = 88,
        .vpw   = 5,
        .vbp   = 36,
        .vact  = 1080,
        .vfp   = 4,
    },
    .phy_data_rate = 891,
    .pixel_clk = 148500,
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_3840x2160_24_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 88,
        .hbp  = 296,
        .hact = 3840,
        .hfp  = 1276,
        .vpw   = 10,
        .vbp   = 72,
        .vact  = 2160,
        .vfp   = 8,
    },
    .phy_data_rate = 1782,
    .pixel_clk = 297000,
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_3840x2160_25_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 88,
        .hbp  = 296,
        .hact = 3840,
        .hfp  = 1056,
        .vpw   = 10,
        .vbp   = 72,
        .vact  = 2160,
        .vfp   = 8,
    },
    .phy_data_rate = 1782,
    .pixel_clk = 297000,
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_3840x2160_30_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 88,
        .hbp  = 296,
        .hact = 3840,
        .hfp  = 176,
        .vpw   = 10,
        .vbp   = 72,
        .vact  = 2160,
        .vfp   = 8,
    },
    .phy_data_rate = 1782, /* lt9611's max data rate < 2000Mbps */
    .pixel_clk = 297000,
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_3840x2160_50_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_CSI,
    .out_format = OUT_FORMAT_RAW_16BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 88,
        .hbp  = 296,
        .hact = 3840,
        .hfp  = 1056,
        .vpw   = 10,
        .vbp   = 72,
        .vact  = 2160,
        .vfp   = 8,
    },
    .phy_data_rate = 2259, /* 2376 is larger than the max 2259, set to 2259 */
    .pixel_clk = 594000,
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_3840x2160_60_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_CSI,
    .out_format = OUT_FORMAT_RAW_16BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 88,
        .hbp  = 296,
        .hact = 3840,
        .hfp  = 176,
        .vpw   = 10,
        .vbp   = 72,
        .vact  = 2160,
        .vfp   = 8,
    },
    .phy_data_rate = 2259, /* 2376 is larger than the max 2259, set to 2259 */
    .pixel_clk = 594000,
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_720x1280_60_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 24,
        .hbp  = 99,
        .hact = 720,
        .hfp  = 99,
        .vpw   = 4,
        .vbp   = 20,
        .vact  = 1280,
        .vfp   = 8,
    },
    .phy_data_rate = 459,
    .pixel_clk = 74250,
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_1080x1920_60_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw = 8,
        .hbp = 20,
        .hact = 1080,
        .hfp = 130,
        .vpw = 10,
        .vbp = 26,
        .vact = 1920,
        .vfp = 16,
    },
    .phy_data_rate = 891,
    .pixel_clk = 148500,
};

typedef struct {
    mipi_tx_intf_sync index;
    const combo_dev_cfg_t *mipi_tx_combo_dev_cfg;
} mipi_tx_intf_sync_cfg;

typedef struct {
    hi_vo_intf_sync intf_sync;
    mipi_tx_intf_sync mipi_tx_sync;
} vo_mst_sync_mipi_tx;

static const mipi_tx_intf_sync_cfg g_sample_mipi_tx_timing[HI_MIPI_TX_OUT_USER] = {
    {HI_MIPI_TX_OUT_576P50,       &g_sample_comm_mipi_tx_720x576_50_config},
    {HI_MIPI_TX_OUT_1024X768_60,  &g_sample_comm_mipi_tx_1024x768_60_config},
    {HI_MIPI_TX_OUT_720P50,       &g_sample_comm_mipi_tx_1280x720_50_config},
    {HI_MIPI_TX_OUT_720P60,       &g_sample_comm_mipi_tx_1280x720_60_config},
    {HI_MIPI_TX_OUT_1280X1024_60, &g_sample_comm_mipi_tx_1280x1024_60_config},
    {HI_MIPI_TX_OUT_1080P24,      &g_sample_comm_mipi_tx_1920x1080_24_config},
    {HI_MIPI_TX_OUT_1080P25,      &g_sample_comm_mipi_tx_1920x1080_25_config},
    {HI_MIPI_TX_OUT_1080P30,      &g_sample_comm_mipi_tx_1920x1080_30_config},
    {HI_MIPI_TX_OUT_1080P50,      &g_sample_comm_mipi_tx_1920x1080_50_config},
    {HI_MIPI_TX_OUT_1080P60,      &g_sample_comm_mipi_tx_1920x1080_60_config},
    {HI_MIPI_TX_OUT_3840X2160_24, &g_sample_comm_mipi_tx_3840x2160_24_config},
    {HI_MIPI_TX_OUT_3840X2160_25, &g_sample_comm_mipi_tx_3840x2160_25_config},
    {HI_MIPI_TX_OUT_3840X2160_30, &g_sample_comm_mipi_tx_3840x2160_30_config},
    {HI_MIPI_TX_OUT_3840X2160_50, &g_sample_comm_mipi_tx_3840x2160_50_config},
    {HI_MIPI_TX_OUT_3840X2160_60, &g_sample_comm_mipi_tx_3840x2160_60_config},

    {HI_MIPI_TX_OUT_720X1280_60,  &g_sample_comm_mipi_tx_720x1280_60_config},
    {HI_MIPI_TX_OUT_1080X1920_60, &g_sample_comm_mipi_tx_1080x1920_60_config},
    {0},
};

static const combo_dev_cfg_t *sample_mipi_tx_get_combo_dev_config(mipi_tx_intf_sync mipi_intf_sync)
{
    hi_u32 loop;
    size_t loop_num = sizeof(g_sample_mipi_tx_timing) / sizeof(mipi_tx_intf_sync_cfg);

    for (loop = 0; loop < loop_num; loop++) {
        if (g_sample_mipi_tx_timing[loop].index == mipi_intf_sync) {
            return g_sample_mipi_tx_timing[loop].mipi_tx_combo_dev_cfg;
        }
    }
    return HI_NULL;
}

static hi_s32 vo_mst_mipi_tx_send_one_cmd(cmd_info_t *cmd_info)
{
    hi_s32 ret;
    ret = ioctl(g_sample_comm_mipi_fd, HI_MIPI_TX_SET_CMD, cmd_info);
    if (ret != HI_SUCCESS) {
        printf("MIPI_TX SET CMD failed\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}
static void delay_ms(int ms)
{
    usleep(ms * 1000); /* 1ms: 1000us */
    return;
}
static hi_s32 vo_i2c_mipi_tx_send_one_cmd(hi_s32 i2c_fd,mipi_tx_i2c_cmd_info *i2c_cmd_info)   
{
    hi_s32 ret = 0;
    td_u8 buf[I2C_ADDR_LEN+I2C_DATA_LEN] = {0};
    td_s8 len = I2C_ADDR_LEN+I2C_DATA_LEN;

    char tmp[I2C_DATA_LEN] = {0};
    buf[0] = i2c_cmd_info->reg_addr_high;
    buf[1] = i2c_cmd_info->reg_addr_low;
    buf[2] = 0x00;
    buf[3] = i2c_cmd_info->reg_value;
    if((i2c_cmd_info->delay_en == HI_TRUE) && buf[3] == 0x00)
    {
        len = I2C_ADDR_LEN;
        buf[3] == 0x00;
    }
    ret = write(i2c_fd,buf, len);
    // printf("i2c write ret = %d\n",ret);
    if(ret < 0)
    {
        printf("i2c write error!\n");
        return HI_FAILURE;
    }
    if(i2c_cmd_info->delay_en == HI_TRUE)
    {
        delay_ms(100);
    }


    return HI_SUCCESS;
}

static hi_s32 vo_i2c_mipi_tx_init_screen(const sample_mipi_tx_config *tx_config)
{
    printf("vo_i2c_mipi_tx_init_screen\n");
    printf("vo_i2c_bus = %x\n vo_i2c_addr = %x\n",tx_config->i2c_bus,tx_config->i2c_addr);
    hi_s32 ret;
    char dev_file[16] = {0};
    int loop = 0;
    hi_u8 i2c_bus_num = tx_config->i2c_bus;
    (hi_void) snprintf_s(dev_file, sizeof(dev_file), sizeof(dev_file) - 1, "/dev/i2c-%u", i2c_bus_num);
    hi_s32 i2c_fd = open(dev_file, O_RDWR,S_IRUSR | S_IWUSR);
    if (i2c_fd < 0) {
        printf("Open /dev/ot_i2c_drv-%s error!\n", dev_file);
        return HI_FAILURE;
    }
    ret = ioctl(i2c_fd,OT_I2C_SLAVE_FORCE,tx_config->i2c_addr);
    if(ret < 0){
        printf("OT_I2C_SLAVE_FORCE error!\n");
        close(i2c_fd);
        return HI_FAILURE;
    }
    for(loop = 0;(hi_u32)loop < tx_config->i2c_cmd_count;loop++)
    {
        ret = vo_i2c_mipi_tx_send_one_cmd(i2c_fd,&tx_config->i2c_cmd_info[loop]);
        if(ret != HI_SUCCESS)
        {
            printf("loop(%u): MIPI_TX I2C SET CMD failed\n", loop);
            close(i2c_fd);
            return HI_FAILURE;
        }
    }
    printf("MIPI_TX I2C SET CMD success\n");
    close(i2c_fd);
    return HI_SUCCESS;

    
}
static hi_s32 vo_i2c_mipi_tx_send_one_cmd_LT_write(hi_s32 i2c_fd_LT,mipi_tx_i2c_cmd_info_LT *i2c_cmd_info_LT)
{
    int retval = 0;
    unsigned char buf[4];  // 最大4字节缓冲区(2字节地址+2字节数据)
    int index = 0;
    if (i2c_cmd_info_LT->reg_width == 2) {
        // 2字节地址
        buf[index] = (i2c_cmd_info_LT->reg_addr >> 8) & 0xff;  // 高字节
        index++;
        buf[index] = (i2c_cmd_info_LT->reg_addr) & 0xff;         // 低字节
        index++;
    } else {
        // 1字节地址
        buf[index] = i2c_cmd_info_LT->reg_addr & 0xff;
        index++;
    }
     if (i2c_cmd_info_LT->reg_value_width == 2) {
        // 2字节数据
        buf[index] = (i2c_cmd_info_LT->reg_value >> 8) & 0xff;  // 高字节
        index++;
        buf[index] = i2c_cmd_info_LT->reg_value & 0xff;         // 低字节
        index++;
    } else {
        // 1字节数据
        buf[index] = i2c_cmd_info_LT->reg_value & 0xff;
        index++;
    }
    printf(" 0x%02X  0x%02X  0x%02X\n",buf[0],buf[1],buf[2]);
    retval = write(i2c_fd_LT, buf, (i2c_cmd_info_LT->reg_value_width + i2c_cmd_info_LT->reg_width));
    printf("%d\n",retval);
    if(retval < 0) {
        printf("i2c write fail!\n");
        return  -1;
    }


}

struct i2c_rdwr_ioctl_data {
    struct i2c_msg *msgs;    // 指向消息数组的指针
    int nmsgs;            // 消息数量
};
static struct i2c_msg {

    hi_u16 addr;    // 从机地址(7位或10位)
    hi_u16 flags;   // 传输标志
    hi_u16 len;     // 消息长度
    hi_u8 *buf;     // 数据缓冲区指针
};
struct i2c_msg msg[2];  // 两个消息：写地址和读数据 
struct i2c_rdwr_ioctl_data rdwr;
static hi_s32 vo_i2c_mipi_tx_send_one_cmd_LT_read(hi_s32 i2c_fd_LT,mipi_tx_i2c_cmd_info_LT *i2c_cmd_info_LT)
{
    int retval;
    unsigned int val = 0;
    unsigned char buf[4];  // 数据缓冲区
    static struct i2c_rdwr_ioctl_data rdwr;
    struct i2c_msg msg[2];  // 两个消息：写地址和读数据
    unsigned int data;
    msg[0].addr = 0x5a>1;
    msg[0].flags = 0;           // 写标志
    msg[1].addr = 0x5a>>1;
    msg[0].len = 2;     // 地址长度
    msg[0].buf = 0;       
    msg[1].flags =1;
    msg[1].len = 1;    // 数据长度
    msg[1].buf = &buf[2]; 
    buf[0] = 0x81;
    buf[1] = 0x01;           // 数据缓冲区
    /*if (i2c_cmd_info_LT->reg_value == 2) {
        // 2字节地址
        buf[0] = (i2c_cmd_info_LT->reg_addr >> 8) & 0xFF;
        buf[1] = i2c_cmd_info_LT->reg_addr & 0xFF;
    } else {
        // 1字节地址
        buf[0] = i2c_cmd_info_LT->reg_addr & 0xFF;
    }*/
    printf("!!!!!!!!!!!!!!\n");
    rdwr.msgs = &msg[0];
    rdwr.nmsgs = 2;
    printf("I2C Messages:\n");
    for (int i = 0; i < rdwr.nmsgs; i++) {
        printf("Message[%d]:\n", i);
        printf("  Address: 0x%02X\n", msg[i].addr);
        printf("  Flags: 0x%02X\n", msg[i].flags);
        printf("  Length: %d\n", msg[i].len);
        printf("  Buffer: ");
        
        
        printf("\n");
    }
    retval = ioctl(i2c_fd_LT, I2C_RDWR, &rdwr);
    if (retval < 0) {
        printf("I2C read failed: %s\n", strerror(errno));
        return -1;
    }
    printf("%d\n",retval);
    if (i2c_cmd_info_LT->reg_width == 2) {
        data = (buf[2] << 8) | buf[3];  // 组合高低字节
    } else {
        data = buf[2];
    }
    printf("Read Reg[0x%04X] = 0x%04X\n", i2c_cmd_info_LT->reg_addr, data);


}

static hi_s32 vo_i2c_mipi_tx_send_one_cmd_LT(hi_s32 i2c_fd_LT,mipi_tx_i2c_cmd_info_LT *i2c_cmd_info_LT)   
{
    if(i2c_cmd_info_LT->reg_value_width != 0)
    {
        printf("111111111111111111111\n");
        printf("%d\n",i2c_cmd_info_LT->reg_value_width);
        vo_i2c_mipi_tx_send_one_cmd_LT_write(i2c_fd_LT,i2c_cmd_info_LT);
    }
    else
    {
        printf("222222222222222222\n");
        vo_i2c_mipi_tx_send_one_cmd_LT_read(i2c_fd_LT,i2c_cmd_info_LT);
    }
    return HI_SUCCESS;
}
static hi_s32 vo_i2c_mipi_tx_init_LT9211(const sample_mipi_tx_config *tx_config)
{
    sample_mipi_LT_config LT9211 = {0};
    LT9211.i2c_addr = tx_config->lt9211_i2c_addr1;
    LT9211.i2c_bus = tx_config->lt9211_i2c_bus1;
    LT9211.i2c_cmd_count = tx_config->lt9211i2c_cmd_count1;
    LT9211.i2c_cmd_en = tx_config->lt9211_i2c_cmd_en1;
    LT9211.i2c_cmd_info = tx_config->lt9211_i2c_cmd_info1;
    printf("vo_i2c_mipi_tx_init_LT9211\n");
    printf("vo_i2c_bus = %x\n vo_i2c_addr = %x\n",LT9211.i2c_bus,LT9211.i2c_addr);
    hi_s32 ret;
    char dev_file[16] = {0};
    int loopS = 0;
    hi_u8 i2c_bus_num_LT = LT9211.i2c_bus;
    (hi_void) snprintf_s(dev_file, sizeof(dev_file), sizeof(dev_file) - 1, "/dev/i2c-%u", i2c_bus_num_LT);
    hi_s32 i2c_fd_LT = open(dev_file, O_RDWR,S_IRUSR | S_IWUSR);
    if (i2c_fd_LT < 0) {
        printf("Open /dev/ot_i2c_drv-%s error!\n", dev_file);
        return HI_FAILURE;
    }
    unsigned int dev_addr = LT9211.i2c_addr >> 1;
    if ( ioctl(i2c_fd_LT, I2C_SLAVE_FORCE, dev_addr)) {
			printf("CMD_I2C_READ error!\n");
			return -1;
		}
    for(loopS = 0;(hi_u32)loopS < LT9211.i2c_cmd_count;loopS++)
    {
        ret = vo_i2c_mipi_tx_send_one_cmd_LT(i2c_fd_LT,&LT9211.i2c_cmd_info[loopS]);
        if(ret != HI_SUCCESS)
        {
            printf("loopS(%u): MIPI_TX I2C SET CMD failed\n", loopS);
            close(i2c_fd_LT);
            return HI_FAILURE;
        }
    }
    printf("MIPI_TX I2C SET CMD success\n");
    close(i2c_fd_LT);
    return HI_SUCCESS;

    
}

static hi_s32 vo_mst_mipi_tx_init_screen(const sample_mipi_tx_config *tx_config)
{
    hi_s32 ret;
    cmd_info_t ci = { 0 };
    hi_u32 loop;
    hi_u32 loop_num = tx_config->cmd_count;

    for (loop = 0; loop < loop_num; loop++) {
        (hi_void)memcpy_s(&ci, sizeof(cmd_info_t), &(tx_config->cmd_info[loop].cmd_info), sizeof(cmd_info_t));
        ret = vo_mst_mipi_tx_send_one_cmd(&ci);
        if (ret != HI_SUCCESS) {
            printf("loop(%u): MIPI_TX SET CMD failed\n", loop);

            return HI_FAILURE;
        }
        usleep(tx_config->cmd_info[loop].usleep_value);
    }

    return HI_SUCCESS;
}

static hi_s32 sample_comm_mipi_tx_check_config(const sample_mipi_tx_config *tx_config)
{
    mipi_tx_intf_sync mipi_intf_sync;

    if (tx_config == NULL) {
        sample_print("tx_config is null\n");
        return HI_FAILURE;
    }

    mipi_intf_sync = tx_config->intf_sync;
    if ((mipi_intf_sync >= HI_MIPI_TX_OUT_BUTT)) {
        sample_print("mipi tx sync illegal\n");
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

static hi_s32 sample_comm_mipi_tx_get_config(const sample_mipi_tx_config *tx_config,
    const combo_dev_cfg_t **mipi_tx_config)
{
    mipi_tx_intf_sync mipi_intf_sync;
    mipi_intf_sync = tx_config->intf_sync;

    if (mipi_intf_sync == HI_MIPI_TX_OUT_USER) {
        *mipi_tx_config = &tx_config->combo_dev_cfg;
        if ((*mipi_tx_config)->phy_data_rate == 0) {
            printf("error: not set mipi tx user config\n");
            return HI_FAILURE;
        }
    } else {
        *mipi_tx_config = sample_mipi_tx_get_combo_dev_config(mipi_intf_sync);
        if (*mipi_tx_config == HI_NULL) {
            sample_print("error: mipi tx combo config is null\n");
            return HI_FAILURE;
        }
    }
    return HI_SUCCESS;
}

static hi_void sample_comm_mipi_tx_do_close_fd(hi_void)
{
    if (g_sample_comm_mipi_fd != HI_INVALID_VALUE) {
        close(g_sample_comm_mipi_fd);
        g_sample_comm_mipi_fd = HI_INVALID_VALUE;
    }
}

hi_s32 sample_comm_start_mipi_tx(const sample_mipi_tx_config *tx_config)
{
    mipi_tx_intf_sync mipi_intf_sync;
    const combo_dev_cfg_t *combo_config = HI_NULL;
    hi_s32 ret;

    ret = sample_comm_mipi_tx_check_config(tx_config);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    g_sample_comm_mipi_fd = open(MIPI_TX_DEV_NAME, O_RDONLY);
    if (g_sample_comm_mipi_fd < 0) {
        printf("open mipi dev file (%s) fail\n", MIPI_TX_DEV_NAME);
        return HI_FAILURE;
    }

    mipi_intf_sync = tx_config->intf_sync;
    printf("mipi intf sync = %d\n", mipi_intf_sync);

    ret = sample_comm_mipi_tx_get_config(tx_config, &combo_config);
    if (ret != HI_SUCCESS) {
        printf("%s,%d, get mipi tx config fail\n", __FUNCTION__, __LINE__);
        sample_comm_mipi_tx_do_close_fd();
        return ret;
    }

    /* step1 */
    ret = ioctl(g_sample_comm_mipi_fd, HI_MIPI_TX_DISABLE, NULL);
    if (ret != HI_SUCCESS) {
        printf("%s,%d, ioctl mipi tx (%s) fail at ret(%d)\n", __FUNCTION__, __LINE__, MIPI_TX_DEV_NAME, ret);
        sample_comm_mipi_tx_do_close_fd();
        return ret;
    }

    /* step2 */
    ret = ioctl(g_sample_comm_mipi_fd, HI_MIPI_TX_SET_DEV_CFG, combo_config);
    if (ret != HI_SUCCESS) {
        printf("%s,%d, ioctl mipi tx (%s) fail at ret(%d)\n", __FUNCTION__, __LINE__, MIPI_TX_DEV_NAME, ret);
        sample_comm_mipi_tx_do_close_fd();
        return ret;
    }

    /* step3 */
    if(tx_config->i2c_cmd_en == HI_TRUE) {
        ret = vo_i2c_mipi_tx_init_screen(tx_config);
        ret = vo_i2c_mipi_tx_init_LT9211(tx_config);
    }else{
        ret = vo_mst_mipi_tx_init_screen(tx_config);
    }
        if (ret != HI_SUCCESS) {
            printf("%s,%d, init screen failed\n", __FUNCTION__, __LINE__);
            sample_comm_mipi_tx_do_close_fd();
            return ret;
        }

    /* step4 */
    ret = ioctl(g_sample_comm_mipi_fd, HI_MIPI_TX_ENABLE, NULL);
    if (ret != HI_SUCCESS) {
        printf("%s,%d, ioctl mipi tx (%s) fail at ret(%d)\n", __FUNCTION__, __LINE__, MIPI_TX_DEV_NAME, ret);
    }

    sample_comm_mipi_tx_do_close_fd();
    return ret;
}

hi_void sample_comm_stop_mipi_tx(hi_vo_intf_type intf_type)
{
    if (!((intf_type & HI_VO_INTF_MIPI) ||
        (intf_type & HI_VO_INTF_MIPI_SLAVE))) {
        sample_print("intf is not mipi\n");
        return;
    }

    g_sample_comm_mipi_fd = open(MIPI_TX_DEV_NAME, O_RDONLY);
    if (g_sample_comm_mipi_fd < 0) {
        printf("open mipi dev file (%s) fail\n", MIPI_TX_DEV_NAME);
        return;
    }

    if (ioctl(g_sample_comm_mipi_fd, HI_MIPI_TX_DISABLE, NULL) < 0) {
        printf("ioctl mipi tx (%s) fail\n", MIPI_TX_DEV_NAME);
        return;
    }

    close(g_sample_comm_mipi_fd);
    g_sample_comm_mipi_fd = HI_INVALID_VALUE;
}

#else

hi_void sample_comm_mipi_tx_set_intf_sync(mipi_tx_intf_sync intf_sync)
{
}

hi_void sample_comm_start_mipi_tx(hi_vo_pub_attr *pub_attr)
{
}

hi_void sample_comm_stop_mipi_tx(hi_vo_intf_type intf_type)
{
}
#endif /* end of #if VO_MIPI_SUPPORT */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */
