#pragma once
#pragma execution_character_set("utf-8") 
#include <string>
#include <vector>

/**
 * @brief 法如命名空间,防止算法使用中避免重名
 */
namespace Faro {

}//namespace Faro 

/**
 * @brief 点云数据结构体,存储单个点的信息
 */
struct PointXYZCT {
	double x{ 0.0 };      ///< X坐标
	double y{ 0.0 };      ///< Y坐标
	double z{ 0.0 };      ///< Z坐标
	int color{ 0 };   ///< 颜色值
	unsigned __int64 time{ 0 };   ///< 时间戳
};

/**
 * @brief Faro点云数据处理类
 * 用于解析和处理来自Faro扫描仪的点云数据
 */
class FaroHandle
{
public:
	/**
	 * @brief 点云展开速度枚举
	 * 数值表示展开跨度的圈数(一圈=5000点)
	 */
	enum class Speed {
		kFastest = 50,   ///< 最快速度
		kFaster = 35,   ///< 较快速度
		kFast = 25,   ///< 快速
		kSlow = 15,   ///< 慢速
		kSlower = 5     ///< 较慢速度
	};

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
	 * @return 解析是否成功
	 */
	bool parseFaroFlsFile(const char* fileName, std::vector<PointXYZCT>& pointRes);
	bool getFaroFlsFileStartAndEndTime(const char* fileName, unsigned __int64& start, unsigned __int64& end);

	bool loadFaroFlsFile(const char* fileName);
	//bool parseFaroFls(std::vector<PointXYZCT>& pointRes);
	//bool getFaroFlsStartAndEndTime(unsigned __int64 &start, unsigned __int64 &end);

private:
	size_t numRows{ 0 };        ///< 扫描行数
	size_t numCols{ 0 };        ///< 扫描列数
};
