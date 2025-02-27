#ifndef _DEMO_CONFIG_H_
#define _DEMO_CONFIG_H_

/* TODO: 替换为自己的接入信息*/
#define INSTANCE_ID      "iot-06z00a7dl4ogq6o"
#define HOST             INSTANCE_ID".mqtt.iothub.aliyuncs.com"
#define PORT1             1883

/* TODO: 替换直连设备认证信息:产品名、设备名、设备密钥、产品密�? */
#define PRODUCT_KEY       "k1m42UY3eJf"
#define DEVICE_NAME       "rk3568_client_control"
#define DEVICE_SECRET     "95d9a44a1d43a3f001bab7142bce3304"
#define PRODUCT_SECRET    "h4IQZ5****L075Zi"          /* 仅动态注册功能需要使�? */

/* 网关子设备demo TODO: 替换网关设备认证信息 */
#define GW_PRODUCT_KEY     "a1kt****O8H"
#define GW_DEVICE_NAME     "GATEWAY_****_001"
#define GW_DEVICE_SECRET   "d414****c32e2"

/* 网关子设备demo TODO: 替换子设备认证信�? */
#define SUBDEV_LIST {                                                    \
    {                                                                    \
        .product_key = "a1o****qvg7",                                    \
        .device_name = "SUBDEV_****_001",                                \
        .device_secret = "af1e********55a822931af372cbc6e7",             \
    },                                                                   \
    {                                                                    \
        .product_key = "a1o****qvg7",                                    \
        .device_name = "SUBDEV_****_002",                                \
        .device_secret = "f15*********688b7dd5e9d7e8ee0b51",             \
    },                                                                   \
    {                                                                    \
        .product_key = "a1o****qvg7",                                    \
        .device_name = "SUBDEV_****_003",                                \
        .device_secret = "eca3*******b3f056f3b6abe06538648",             \
    },                                                                   \
}

/* TODO: 网关与子设备动态注册demo: 子设备的信息列表 */
#define SUBDEV_DYNREG_LIST {                                             \
    {                                                                    \
        .product_key = "gb80****8MP",                                    \
        .device_name = "STAND_********_d001",                            \
        .product_secret = "rqml*******l2oS2",                            \
    },                                                                   \
    {                                                                    \
        .product_key = "gb80****8MP",                                    \
        .device_name = "STAND_********_d002",                            \
        .product_secret = "rqml*******l2oS2",                            \
    },                                                                   \
    {                                                                    \
        .product_key = "gb80****8MP",                                    \
        .device_name = "STAND_********_d003",                            \
        .product_secret = "rqml*******l2oS2",                            \
    },                                                                   \
}

#endif

