#pragma once
#include "iFaro.h"

extern	size_t g_num_rows;        ///< 扫描行数
extern	size_t g_num_cols;        ///< 扫描列数
extern double g_points_per_millisecond; // 976 kHz; ≈ 1.024 毫秒 / 点 数据记录时间

//加载fls文件
bool LoadFaroFlsFile(const std::string& fileName);

/**
 * @brief 解析FLS文件并提取点云数据
 * @param flsFile FLS文件路径
 * @param pointRes [out] 输出的点云数据集合
 * @param direction 正向需要x轴翻转,默认为true,即反向为false,即不反转x轴.扫描仪与小车方向相反
 * @return 解析是否成功
 */
bool ReadFaroFileData(const std::string& fileName, std::vector< std::vector<PointCloud>>& pointsRes,bool direction = true);

bool GetFaroFileStartAndEndTimestamp(const std::string& fileName, unsigned __int64& start, unsigned __int64& end);
/**
 * @brief 解析FLS文件并提取某一个面,即列的极坐标系中的R值数据
 * 三维球坐标(r,ω,θ)r: 点到原点的距离。ω: 点与z-轴的夹角（极角或天顶角）。θ: 点在xy-平面上的投影与x-轴的夹角（方位角）。
 * θ:仪器下端有60度扫描死角 范围-60°至90°,90°至-60° 对应的范围是30~330
 * @param radius [out] 极坐标系中的R值数据,
 * @param col 指定的列
 * @return 提取是否成功
 */
bool GetRadius(std::vector<double>& out_radius, const int &col = 0);
bool GetRadiusAtThetas(const std::vector<double>& input_radius, const std::vector<double>& thetas, std::vector<double>& out_radius);
bool GetRadiusAtAngles(const std::vector<double>& input_radius, const std::vector<double>& angles, std::vector<double>& out_radius);
