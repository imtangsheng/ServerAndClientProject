//FaroHandle.cpp
#include <qdebug.h>
#include <qdatetime.h>
#include "FaroHandle.h"
#include "iQOpen.tlh" // 类型库头文件
#include "iQOpen.tli" // 类型库实现文件

//未使用作用域解析运算符明确指定要使用的变量
size_t g_num_rows{ 0 };        ///< 扫描行数
size_t g_num_cols{ 0 };        ///< 扫描列数
static bool gs_is_faro_initialized = false;//法如初始化标志
double g_points_per_millisecond{ 1.024 }; // 976 kHz; ≈ 1.024 毫秒 / 点 数据记录时间

QString g_file_name{""};
unsigned __int64 g_time_start{ 0 }, g_time_end{0};

inline static BSTR FaroString(QString str) {
	return SysAllocString(reinterpret_cast<const OLECHAR*>(str.utf16()));
	//return _bstr_t(str.toStdString().c_str());//不支持中文
}

/**
 * @brief import directive of iQOpen.SDK
 * 声明法如扫描的解析变量,参考FARO自动化手册
*/
static IiQLicensedInterfaceIfPtr liPtr{ nullptr }; //许可
static IiQLibIfPtr libRef{ nullptr }; //reference 指向变量或对象的引用

static void FaroCleanup() {
	if (gs_is_faro_initialized) {
		if (libRef) {
			//libRef->Release();
			libRef = nullptr;
		}
		if (liPtr) {
			//liPtr->Release();
			liPtr = nullptr;
		}
		CoUninitialize();
		gs_is_faro_initialized = false;
	}
}

static bool FaroInitialized() {
	if (gs_is_faro_initialized) {
		return true;
	}
	try
	{
		// 初始化COM组件
		if (FAILED(CoInitialize(nullptr))) {
			throw FlsParseError::kInitError;
		}
		// Faro许可证验证
		const std::string licenseTemplate =
			"FARO Open Runtime License\n"
			"Key:" FARO_LICENSE_KEY "\n"
			"\n"
			"The software is the registered property of "
			"FARO Scanner Production GmbH, Stuttgart, Germany.\n"
			"All rights reserved.\n"
			"This software may only be used with written permission "
			"of FARO Scanner Production GmbH, Stuttgart, Germany.";

		const BSTR licenseCode = _bstr_t(licenseTemplate.c_str());
		// 创建并验证许可接口
		liPtr = IiQLicensedInterfaceIfPtr(__uuidof(iQLibIf));
		if (!liPtr) {
			throw FlsParseError::kLicenseError;
		}
		liPtr->License = licenseCode;
		libRef = static_cast<IiQLibIfPtr>(liPtr);
		if (!libRef) {
			throw FlsParseError::kLicenseError;
		}
		gs_is_faro_initialized = true;
		return true;
	}
	catch (const FlsParseError& error)
	{
		qWarning() << QObject::tr("#Faro:法如扫描初始化错误: %1").arg(static_cast<int>(error));
		FaroCleanup();
		return false;
	}
	catch (...)
	{
		qWarning() << QObject::tr("#Faro:法如扫描仪初始化发生未知错误");
		FaroCleanup();
		return false;
	}
}
//The performance to retrieve the scan points of a scan with getScanPoint depends on the number of scans in the workspace.The more scans the workspace has, the slower the access will be.The arrays in getXYZScanPoints and getPolarScanPoints are not managed!Therefore these functions can only be used in C++!
//使用 getScanPoint 检索扫描的扫描点的性能取决于工作区中的扫描数量。工作区的扫描次数越多，访问速度就越慢。getXYZScanPoints 和 getPolarScanPoints 中的数组不受管理！因此，这些函数只能在 C++ 中使用！
static bool FaroLoadFlsFile(QString fileName) {
	if (!FaroInitialized()) {
		return false;
	}
	try {
		// 加载FLS文件 行 列  4268 5000 13722 ms
		qDebug() << "#Faro:加载FLS文件:"<< fileName;
		static QDateTime startTime = QDateTime::currentDateTime();
		BSTR filepathBstr = FaroString(fileName);
		// 加载文件不支持多个实例多线程,有锁
		int ret = libRef->load(filepathBstr);//Return Values: 	0, 11, 12, 13, 25
		if (ret != 0) {
			QObject::tr("[#Faro]文件格式错误,错误码%1").arg(ret);//11 没有工作空间 No workspace 
			throw FlsParseError::kFlsFormatError;
		}
		qDebug() << "#Faro:加载FLS文件:" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
		return true;
	}
	catch (const FlsParseError& error)
	{
		qWarning() << QObject::tr("#Faro:FLS解析错误: %1").arg(static_cast<int>(error));
		FaroCleanup();
		return false;
	}
	catch (...) {
		qWarning() << QObject::tr("#Faro:FLS解析发生未知错误");
		FaroCleanup();
		return false;
	}
}

// Function to calculate nanoseconds based on measurement rate
static size_t FaroCalculatePointsPerNanoseconds(int measurementRate = 8) {
	double pointsPerSecond;
	switch (measurementRate) {
	case 1:
		pointsPerSecond = 122000;
		break;
	case 2:
		pointsPerSecond = 244000;
		break;
	case 4:
		pointsPerSecond = 488000;
		break;
	case 8:
		pointsPerSecond = 976000;
		break;
	default:
		pointsPerSecond = 976000; // Default to highest rate
	}
	// Calculate microseconds: (1/pointsPerSecond) * 1,000,000
	return  (1.0 / pointsPerSecond) * 1000000 * 1000;
}

bool LoadFaroFlsFile(const QString& fileName)
{
	if (!FaroLoadFlsFile(fileName)) {
		return false;
	}
	// 获取扫描维度
	g_num_rows = libRef->getScanNumRows(0);
	g_num_cols = libRef->getScanNumCols(0);

	libRef->getAutomationTimeOfScanPoint(0, g_num_rows - 1, 0, &g_time_start);
	libRef->getAutomationTimeOfScanPoint(0, 0, g_num_cols - 1, &g_time_end);
	qDebug() << "#Faro:获取一个fls文件的开始和结束时间:" << g_time_start << g_time_end;
	return true;
}

//Helical Mode 螺旋模式
bool ReadFaroFileData(const QString& fileName, std::vector< std::vector<PointCloud>>& pointsRes, bool direction)
{
	if (!LoadFaroFlsFile(fileName)) {
		return false;
	}
	const size_t halfCols = g_num_cols / 2; //扫描数据 前半部分列是从下往上读 后半部分列是从上往下读

	static QDateTime startTime = QDateTime::currentDateTime();
	std::vector<PointCloud> dataCache(g_num_cols * g_num_rows);// 先按列批量读取数据到缓存
	// 避免使用单点getScanPoint,改用批量读取
	double* positions = new double[g_num_rows * 3];
	int* reflections = new int[g_num_rows];
	unsigned __int64 atime{ 0 };
	//for (int col = 0; col < numCols; col++) { 
	//	iQLibIfPtr->getXYZScanPoints(0, 0, col, numRows, positions, reflections);
	//	for (int row = 0; row < numRows; ++row)
	//	{
	//		PointXYZCT& item = dataCache[col * numRows + row];
	//		item.x = positions[3 * row + 0]; item.y = positions[3 * row + 1]; item.z = positions[3 * row + 2];
	//		item.color = reflections[row];
	//		//item.time = atime++;
	//		//iQLibIfPtr->getScanPoint(0, row, col, &item.x, &item.y, &item.z, &item.color); //60726 ms
	//		iQLibIfPtr->getAutomationTimeOfScanPoint(0, row, col, &item.time);//31910 ms
	//	}
	//}
	// 183525278 183535763 183546248 183556734 与第一点差868ms
	//qDebug() << "getAutomationTimeOfSyncPulse:";
	for (int col = 0; col < g_num_cols; col++) {
		libRef->getXYZScanPoints(0, 0, col, g_num_rows, positions, reflections);
		//iQLibIfPtr->getAutomationTimeOfScanPoint(0, 0, col, &atime); // 每个点差 (1 / 976000)秒 * 1, 000, 000 = 1.024微秒
		// 前半部分 时间开始递减
		for (int row = 0; row < g_num_rows; ++row) {
			PointCloud& item = dataCache[col * g_num_rows + row];
			item.x = positions[3 * row + 0]; item.y = positions[3 * row + 1]; item.z = positions[3 * row + 2];
			item.color = reflections[row];
		}
	}
	delete[] positions;
	delete[] reflections;
	qDebug() << "#Faro:先按列批量读取数据到缓存:" << g_num_cols << g_num_rows << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
	// 然后并行处理数据重排和坐标变换
	startTime = QDateTime::currentDateTime();

	double timeStep = 0;
	size_t resultIndex = 0;
	pointsRes.resize(halfCols);//直接分配正确大小 以及跟踪结果vector的填充位置直接赋值
	for (int c = 0; c < halfCols; ++c) {
		std::vector<PointCloud> pointsRows(2 * g_num_rows);
		resultIndex = 0;
		libRef->getAutomationTimeOfScanPoint(0, g_num_rows - 1, c, &atime);// 同步的起始时间 (ms)
		timeStep = 0;// 同步的起始时间差值
		for (size_t r = g_num_rows; r-- > 0;)// 前半部分：从下往上
		{
			PointCloud& Point = dataCache[r + c * g_num_rows];
			if (direction) Point.x = -Point.x;// 小车方向和扫描仪朝向相反，故将X置反，下同
			Point.time = atime + timeStep;
			timeStep += g_points_per_millisecond; //下一次数据记录时间
			pointsRows[resultIndex++] = Point;// 直接赋值，没有push_back的开销
		}
		for (size_t r = 0; r < g_num_rows; ++r) // 后半部分：从上往下
		{
			PointCloud& Point = dataCache[r + (c + halfCols) * g_num_rows];
			if (direction) Point.x = -Point.x;
			Point.time = atime + timeStep;
			timeStep += g_points_per_millisecond;
			pointsRows[resultIndex++] = Point;
		}
		pointsRes[c] = pointsRows;
	}
	qDebug() << "#Faro:然后并行处理数据重排和坐标变换:" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
	return true;
}

bool GetFaroFileStartAndEndTimestamp(const QString& fileName, unsigned __int64& start, unsigned __int64& end)
{
	static QDateTime startTime = QDateTime::currentDateTime();
	if (!LoadFaroFlsFile(fileName)) {
		return false;
	}
	libRef->getAutomationTimeOfScanPoint(0, g_num_rows - 1, 0, &start);
	libRef->getAutomationTimeOfScanPoint(0, 0, g_num_cols - 1, &end);
	qDebug() << "#Faro:获取一个fls文件的开始和结束时间:" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
	return true;
}

bool GetRadius(std::vector<double>& out_radius, const int& col)
{
	if (col < 0 || col >= g_num_cols || !libRef) {
		return false;
	}

	out_radius.reserve(g_num_rows * 2);  // 预分配足够的空间，避免多次重新分配
	//访问极坐标中每列的所有点列
	double* positions = new double[g_num_rows * 3];
	int* reflections = new int[g_num_rows];
	// 位置数组pos(存储r,ω,θ)，颜色值color r: 米(m) ω, θ : 弧度(rad)
	// 获取第一部分数据
	libRef->getPolarScanPoints(0, 0, col, g_num_rows, positions, reflections);
	for (size_t row = g_num_rows; row-- > 0;) {
		out_radius.push_back(positions[3 * row + 0]);
	}
	// 获取第二部分数据
	libRef->getPolarScanPoints(0, 0, col + g_num_cols / 2, g_num_rows, positions, reflections);
	for (size_t row = 0; row < g_num_rows; ++row) {
		out_radius.push_back(positions[3 * row + 0]);
	}
	delete[] positions, reflections;
	return true;
}

bool GetRadiusAtThetas(const std::vector<double>& input_radius, const std::vector<double>& thetas, std::vector<double>& out_radius)
{
	const int poinstNum = input_radius.size();
	out_radius.reserve(thetas.size());
	for (auto theta : thetas) {
		// 检查theta是否在有效范围内
		if (theta < 30.0 || theta > 330.0) {
			out_radius.push_back(0.0);  // 超出范围返回0
			continue;
		}
		size_t it = (theta - 30) / 330.0 * (poinstNum - 1);
		out_radius.push_back(input_radius[it]);
	}
	return true;
}

bool GetRadiusAtAngles(const std::vector<double>& input_radius, const std::vector<double>& angles, std::vector<double>& out_radius)
{
	double distanceFromPoint = 1.0;
	const double& L = distanceFromPoint;// The distance from point p1 to p2
	const int poinstNum = input_radius.size();
	out_radius.reserve(angles.size());
	for (auto angle : angles) {
		// 检查angle是否在有效范围内 标准象限
		if (angle > 240 && angle < 300) {
			out_radius.push_back(0.0);  // 超出范围返回0
			continue;
		}
		//转换到对应法如的角度值
		auto theta = 270.0 - angle;
		if (theta < 0) theta += 360.0;
		//计算对应的R值
		size_t it = (theta - 30) / 330.0 * (poinstNum - 1);
		out_radius.push_back(input_radius[it]);
		double R1 = input_radius[it];
		// 计算 R2
		double R2 = std::sqrt(R1 * R1 + L * L - 2 * R1 * L * std::sin(theta * M_PI / 180.0));
	}
	return true;
}
