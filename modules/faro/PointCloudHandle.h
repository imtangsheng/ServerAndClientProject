#pragma once
#include "gFaro.h"
using namespace Faro;


// 平移和全局参数结构
struct ShiftAndGlobal
{
    double x, y, z, p;
};

// 椭圆拟合参数结构
struct FittingEllipse
{
    double A;    //长半轴
    double B;    //短半轴
    double x;    //拟合椭圆的中心x
    double y;    //拟合椭圆的中心y
    double angle;//偏移角度
};

// 每50圈将隧道点云分成若干块，每一块的信息
// 里程椭圆信息结构
struct MileageEllipse
{
    double mileage;  //起始里程
    double dm;       //圈间距
    int col;         //用来拟合椭圆的圈数
    FittingEllipse ep;   //拟合椭圆的参数
};

// 椭圆基础参数结构
struct EllipseParams {
    double A{ 0.0 };  // 长半轴
    double B{ 0.0 };  // 短半轴 
    double X0{ 0.0 }; // 中心x
    double Y0{ 0.0 }; // 中心y
    double Angle{ 0.0 }; // 椭圆长轴与x轴正向的夹角
};

// 点云截面信息结构
struct PointCloudSection : public PointCloudXYZCT
{
    double xe{ 0.0 };          //展开后横坐标
    double error{ 0.0 };		// 误差
    double mileage{ 0.0 };     //里程
    double angle{ 0.0 };		// 角度
    double area{ 0.0 };        //扫过的面积
    double depth{ 0.0 };       //每点的深度
};

// 图像数据
struct Image {
    std::vector<std::vector<unsigned char>> data;
    int width;
    int height;
    unsigned char& at(int y, int x) {
        return data[y][x];
    }

    template<typename T>
    T& at(int y, int x) {
        return reinterpret_cast<T&>(data[y][x]);
    }

    Image(int w, int h) : width(w), height(h) {
        data.resize(h, std::vector<unsigned char>(w));
    }

    void save(const std::string& path);
    static void save(std::vector<std::vector<unsigned char>> color, int width, int height, std::string savepath);
};

/**
 * @brief Faro点云数据处理类
 * 用于解析和处理来自Faro扫描仪的点云数据
 */
class PointCloudImage
{
public:

    PointCloudImage();
    ~PointCloudImage();


    //Algorithm image ImagePointCloud
    /**
     * @brief 生成fls对应的深度图与灰度图
     * @param lme 里程及点间距信息
     * @param tunnel 隧道点云
     * @param savephotopathGray 保存的灰度图路径
     * @param savephotopathDepth 保存的深度图路径
     * @param start_mileage 起始里程
     * @param end_mileage 终止里程
     * @param expand 展开的宽度
     * @param dDesignSecRadius 隧道设计半径
     * @param dScannerHeight 扫描仪安装高度
     * @return
    */
    bool generateImages(
        std::vector<MileageEllipse>& lme,
        std::vector<std::vector<PointCloudSection>>& tunnel,
        const std::string& savephotopathGray,
        const std::string& savephotopathDepth,
        double start_mileage,
        double end_mileage,
        int expand,
        double dDesignSecRadius,
        double dScannerHeight = 0.68
    );

private:
    EllipseParams m_ellipse;// 用于展开影像图的椭圆参数，用于后面求出纵坐标xe
};
