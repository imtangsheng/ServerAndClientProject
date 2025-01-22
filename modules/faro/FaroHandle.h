#pragma once
#include "gFaro.h"
using namespace Faro;


/**
 * @brief Faro点云数据处理类
 * 用于解析和处理来自Faro扫描仪的点云数据
 */
class FaroHandle
{
public:
	FaroHandle();
	~FaroHandle();

	/**
	 * @brief 初始化解析器
	 */
	bool init();

	/**
	 * @brief 解析FLS文件并提取点云数据
	 * @param flsFile FLS文件路径
	 * @param pointRes [out] 输出的点云数据集合
	 * @param direction 正向需要x轴翻转,默认为true,即反向为false,即不反转x轴.扫描仪与小车方向相反
	 * @return 解析是否成功
	 */
	bool read_pointcloud_from_file(const std::string& fileName, std::vector< std::vector<PointCloudXYZCT>>& pointsRes, bool direction = true);

	bool getFaroFlsFileStartAndEndTime(const std::string& fileName, unsigned __int64& start, unsigned __int64& end);
	/**
	 * @brief 解析FLS文件并提取某一个面,即列的极坐标系中的R值数据
	 * 三维球坐标(r,ω,θ)r: 点到原点的距离。ω: 点与z-轴的夹角（极角或天顶角）。θ: 点在xy-平面上的投影与x-轴的夹角（方位角）。
	 * θ:仪器下端有60度扫描死角 范围-60°至90°,90°至-60° 对应的范围是30~330
	 * @param radius [out] 极坐标系中的R值数据,
	 * @param col 指定的列
	 * @return 提取是否成功
	 */
	bool getPolarScanPointsRadiusByColumn(std::vector<double>& radius,const int col = 0);
	bool getScanPointsRadiuByThetas(const std::vector<double>& PointsRadius, const std::vector<double> thetas, std::vector<double>& radius);
	bool getPointsRadiuByScanPointsRadiu(const std::vector<double>& PointsRadius, const std::vector<double> angles, std::vector<double>& radius);

	bool loadFaroFlsFile(const std::string& fileName);
	//bool parseFaroFls(std::vector<PointXYZCT>& pointRes);
	//bool getFaroFlsStartAndEndTime(unsigned __int64 &start, unsigned __int64 &end);
	void test();


private:


public:

	double pointsPerMillisecond{ 1.024 }; // 976 kHz; ≈ 1.024 毫秒 / 点 数据记录时间
	size_t numRows{ 0 };        ///< 扫描行数
	size_t numCols{ 0 };        ///< 扫描列数


};
