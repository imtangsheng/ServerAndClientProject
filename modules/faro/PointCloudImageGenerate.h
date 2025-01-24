#pragma once
#include "iFaro.h"

//namespace poiontcloud_image_generate
//{
extern int smoothStep;
extern double g_first_thinning;
extern double g_second_thinning;
extern double g_pipe_wall_limit;
extern double g_tunnel_face_value;
extern double g_depth_standard;
extern double g_depth_variance_standard;
extern double g_pipe_wall_start_and_end_init_value;
extern double g_hole_depth_limit;//螺栓孔深度过滤阈值
//} // namespace poiontcloud_image_generate

//拟合椭圆的参数
struct FittingEllipse
{
	double A;    //长半轴
	double B;    //短半轴
	double x;    //拟合椭圆的中心x
	double y;    //拟合椭圆的中心y
	double angle;//偏移角度
};

/// <summary>
/// 每50圈将隧道点云分成若干块，每一块的信息
/// </summary>
struct MileageEllipse
{
	double mileage;  //起始里程
	double dm;       //圈间距
	int col;         //用来拟合椭圆的圈数
	FittingEllipse ep;   //拟合椭圆的参数
};

//椭圆参数结构体
struct EllipseParams
{
	double A;
	double B;
	double X0;
	double Y0;
	double Angle;   //椭圆长轴与x轴正向的夹角
	EllipseParams() :A(0.0), B(0.0), X0(0.0), Y0(0.0), Angle(0.0) {}
	EllipseParams(double a, double b, double x0, double y0, double angle) :
		A(a), B(b), X0(x0), Y0(y0), Angle(angle) {}
};


class Image {
public:
	std::vector<std::vector<unsigned char>> data;
	int width{ 0 };
	int height{ 0 };
	Image() {}
	Image(int w, int h) : width(w), height(h) {
		data.resize(h, std::vector<unsigned char>(w));
	}
	// resize 直接映射到 data
	void resize(int w, int h) {
		width = w;
		height = h;
		data.resize(h, std::vector<unsigned char>(w));
	}
	// 添加 operator[] 重载
	std::vector<unsigned char>& operator[](int index) {
		return data[index];
	}

	// 添加 const operator[] 重载
	const std::vector<unsigned char>& operator[](int index) const {
		return data[index];
	}

	unsigned char& at(int y, int x) {
		return data[y][x];
	}

	template<typename T>
	T& at(int y, int x) {
		return reinterpret_cast<T&>(data[y][x]);
	}
};

/**
 * @brief 生成fls对应的深度图与灰度图
 * @param lme 里程及点间距信息
 * @param points 隧道点云
 * @param savephotopathGray 保存的灰度图路径
 * @param savephotopathDepth 保存的深度图路径
 * @param start_mileage 起始里程
 * @param end_mileage 终止里程
 * @param expand 展开的宽度
 * @param dDesignSecRadius 隧道设计半径
 * @param dScannerHeight 扫描仪安装高度
 * @return
*/
bool GetPointCloudDepthAndGrayImage(
	std::vector<MileageEllipse>& lme,	std::vector<std::vector<PointCloud> >& points,
	Image& gray_data, std::vector<std::vector<unsigned char>>& depth_data,
	double start_mileage,	double end_mileage,
	int expand,	double dDesignSecRadius,
	double dScannerHeight = 0.68
);