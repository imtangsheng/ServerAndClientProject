#pragma once
/**
 * @brief 法如命名空间,防止算法使用中避免重名
 */
#pragma execution_character_set("utf-8") 
#include <string>
#include <vector>
#include <QDebug>
#include <QDateTime>

#define FARO_LICENSE_KEY "J3CW4PNRTCTXFJ7T6KZUARUPL"

#include "Common_Structs.h"
using namespace CommonStructs;

 // 点云展开的速度，枚举值代表展开跨度多少圈
 // 一圈为5000个点


namespace Faro {

/**
 * @brief FLS文件解析错误枚举
*/
enum class FlsParseError {
	kInitError,
	kLicenseError,    ///< 许可证验证失败
	kFlsFormatError   ///< FLS文件格式错误
};



/**
* @brief 点云数据结构体,存储单个点的信息
*/
//struct PointCloudXYZCT {
//	double x{ 0.0 };      ///< X坐标
//	double y{ 0.0 };      ///< Y坐标
//	double z{ 0.0 };      ///< Z坐标
//	int color{ 0 };   ///< 颜色值
//	unsigned __int64 time{ 0 };   ///< 时间戳
//};



}//namespace Faro 