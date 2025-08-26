/**
 * @file iFaro.h
 * @brief Faro公有头文件,定义数据类型
 * @version 0.1
 * @date 2025-08
 */

#pragma once
#include "api/iQOpen.tlh"
namespace faro
{

#define FARO_LICENSE_KEY "J3CW4PNRTCTXFJ7T6KZUARUPL"
    // Faro许可证验证 不能省略说明,否则报错
    const BSTR LicenseCode = _bstr_t("FARO Open Runtime License\n"
        "Key:" FARO_LICENSE_KEY "\n"
        "\n"
        "The software is the registered property of "
        "FARO Scanner Production GmbH, Stuttgart, Germany.\n"
        "All rights reserved.\n"
        "This software may only be used with written permission "
        "of FARO Scanner Production GmbH, Stuttgart, Germany.");
    constexpr double InvalidValue = 0.01;//无效值,判断小于0的值

    inline double gCameraCenterToLens = 0.250; //相机中心到相机镜头的距离250mm
    enum ErrorNumbers :int {// 在头文件中定义错误信息枚举
        OK = 0,
        BUSY = 1,
        TIMEOUT = 2,
        NOT_CONNECTED = 3,
        FAILED = 4,
        NO_WORKSPACE = 11,
        NO_SCAN = 13,
        CANNOT_OPEN_READ = 14,
        CANNOT_OPEN_WRITE = 15,
        CANNOT_FIND_SCAN_FILE = 16,
        CANNOT_FIND_SCAN_DATA = 17,
        UNKNOWN_SCAN_VERSION = 21,
        UNKNOWN_KEY = 22,
        OUT_OF_MEMORY = 26,
        DATA_MISSING = 27,
        SCAN_STILL_ACTIVE = 81,
        SCANNER_OPERATION_FAILURE = 83,
        OUT_OF_BOUNDARY = 94,
        SCANNER_BUSY = 116,
        NOT_FOUND = 169
    };
    /*扫描仪的局部笛卡尔坐标系统XYZScanPoints in the local cartesian coordinate  system  of  the  scanner.
    * 每个点将 3 个值（x、y、z）存储到数组 pos 中，将一个值存储到数组 refl 中
    * @brief color:
    * For grey scan points: Mapped reflection value is in the range 0 (black) to 255 (white).
    * For color: the lightness of the color in the range 0 to 255.
    * @bruef time 内部时间
    */
    struct ScanPoint {
        double x{ 0.0 };
        double y{ 0.0 };
        double z{ 0.0 };
        unsigned short int color{ 0 };//refl
        unsigned __int64 time{ 0 };
    };

    /*坐标系统解释PolarScanPoints 球形Spherical坐标系
   r(radius) : 表示距离，即点到原点的距离 径向距离
   phi azimuth(φ) : 方位角，在水平面内的角度方位角 [0, 2π) 或 [-π, π]
   theta elevation(θ) :仰角 [-π/2, π/2]，正值向上，负值向下
   */
    struct ScanPointPolar {
        double r{ 0 }, phi{ 0 }, theta{ 0 };
    };

    inline static BSTR FaroSring(QString str) {
        return _bstr_t(str.toStdString().c_str());
    }
    //Nc = 2f * t / r 线数Number of San Lines= 2*频率(Hz)*时间(秒)*分辨率(1/10倍数)
    inline static int GetNumColumns(int t, int r, int quality = 8) {
        double f;
        switch (quality) {
        case 1:f = 3.05; break;
        case 2:f = 6.10; break;
        case 4:f = 12.2; break;
        case 8:f = 24.4; break;
        }
        return 2 * f * t * r;
    }
}//end namespace faro
