#pragma once
/**
 * @brief 法如命名空间,防止算法使用中避免重名
 */
#include <string>
#include <vector>


#define FARO_LICENSE_KEY "J3CW4PNRTCTXFJ7T6KZUARUPL"

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif


//constexpr double M_PI = 3.14159265358979323846;
//#include <numbers>
//using std::numbers::pi; // pi是一个inline constexpr变量

 /**
  * @brief FLS文件解析错误枚举
 */
enum class FlsParseError {
	kInitError,
	kLicenseError,    ///< 许可证验证失败
	kFlsFormatError   ///< FLS文件格式错误
};

// 点云展开的速度，枚举值代表展开跨度多少圈
// 一圈为5000个点
enum Speed {
	kFastest = 50,
	kFaster = 35,
	kFast = 25,
	kSlow = 15,
	kSlower = 5
};

//隧道点结构
//PointTunnel


struct PointFaro
{
	double x{ 0.0 };
	double y{0.0};
	double z{ 0.0 };
	unsigned short int color{ 0 };
	unsigned __int64 time{0};
};

struct PointCloudXYZCTXD : public PointFaro
{
	double xe = 0.0;
	double ds = 0.0;
};

struct PointCloudXYZCTXDF : public PointCloudXYZCTXD
{
	int flag = 0;
};

struct PointCloudXYZCTRGB : public PointFaro
{
	unsigned int R;
	unsigned int G;
	unsigned int B;
};

// 点云截面信息结构
struct PointCloud : public PointFaro
{
	double xe{ 0.0 };          //展开后横坐标
	double error{ 0.0 };		//误差 0
	double mileage{ 0.0 };     //里程
	double angle{ 0.0 };		//角度 0
	double area{ 0.0 };        //扫过的面积 0
	double depth{ 0.0 };       //每点的深度 0
};

