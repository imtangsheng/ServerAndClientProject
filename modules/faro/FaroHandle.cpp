//FaroHandle.cpp
#include "FaroHandle.h"
#include "iQOpen.tlh" // 类型库头文件
#include "iQOpen.tli" // 类型库实现文件

#include <QDebug>
#include <QDateTime>
#include <omp.h>

#define FARO_LICENSE_KEY "J3CW4PNRTCTXFJ7T6KZUARUPL"


/**
 * @brief FLS文件解析错误枚举
*/
enum class FlsParseError {
	kInitError,
	kLicenseError,    ///< 许可证验证失败
	kFlsFormatError   ///< FLS文件格式错误
};
static bool isFaroInitialized = false;//法如初始化标志
IiQLicensedInterfaceIfPtr iQLicensedInterfaceIfPtr{ nullptr };
IiQLibIfPtr iQLibIfPtr{ nullptr };

void FaroCleanup() {
	if (isFaroInitialized) {
		if (iQLibIfPtr) {
			iQLibIfPtr->Release();
			iQLibIfPtr = nullptr;
		}
		if (iQLicensedInterfaceIfPtr) {
			iQLicensedInterfaceIfPtr->Release();
			iQLicensedInterfaceIfPtr = nullptr;
		}
		CoUninitialize();
		isFaroInitialized = false;
	}
}

bool FaroInitialized() {
	if (isFaroInitialized) {
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
		iQLicensedInterfaceIfPtr = IiQLicensedInterfaceIfPtr(__uuidof(iQLibIf));
		if (!iQLicensedInterfaceIfPtr) {
			throw FlsParseError::kLicenseError;
		}
		iQLicensedInterfaceIfPtr->License = licenseCode;
		iQLibIfPtr = static_cast<IiQLibIfPtr>(iQLicensedInterfaceIfPtr);
		if (!iQLibIfPtr) {
			throw FlsParseError::kLicenseError;
		}
		isFaroInitialized = true;
		return true;
	}
	catch (const FlsParseError& error)
	{
		qWarning() << QObject::tr("法如扫描初始化错误: %1").arg(static_cast<int>(error));
		FaroCleanup();
		return false;
	}
	catch (...)
	{
		qWarning() << QObject::tr("法如扫描仪初始化发生未知错误");
		FaroCleanup();
		return false;
	}
}
//The performance to retrieve the scan points of a scan with getScanPoint depends on the number of scans in the workspace.The more scans the workspace has, the slower the access will be.The arrays in getXYZScanPoints and getPolarScanPoints are not managed!Therefore these functions can only be used in C++!
//使用 getScanPoint 检索扫描的扫描点的性能取决于工作区中的扫描数量。工作区的扫描次数越多，访问速度就越慢。getXYZScanPoints 和 getPolarScanPoints 中的数组不受管理！因此，这些函数只能在 C++ 中使用！
bool FaroLoadFlsFile(const char* fileName) {
	if (!FaroInitialized()) {
		return false;
	}
	try {
		// 加载FLS文件 行 列  4268 5000 13722 ms
		static QDateTime startTime = QDateTime::currentDateTime();
		_bstr_t filepathBstr(fileName);
		// 加载文件不支持多个实例多线程,有锁
		if (iQLibIfPtr->load(filepathBstr) != 0) {
			throw FlsParseError::kFlsFormatError;
		}
		qDebug() << "加载FLS文件:" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
		return true;
	}
	catch (const FlsParseError& error)
	{
		qWarning() << QObject::tr("FLS解析错误: %1").arg(static_cast<int>(error));
		FaroCleanup();
		return false;
	}
	catch (...) {
		qWarning() << QObject::tr("FLS解析发生未知错误");
		FaroCleanup();
		return false;
	}
}


FaroHandle::FaroHandle() = default;
FaroHandle::~FaroHandle() {
	FaroCleanup();
}

bool FaroHandle::init()
{
	return FaroInitialized();
}

//Helical Mode 螺旋模式
bool FaroHandle::parseFaroFlsFile(const char* fileName, std::vector<PointXYZCT>& pointRes)
{
	if (!loadFaroFlsFile(fileName)) {
		return false;
	}
	const size_t halfCols = numCols / 2; //扫描数据 前半部分列是从下往上读 后半部分列是从上往下读
	pointRes.resize(numRows * numCols); size_t resultIndex = 0; // 直接分配正确大小 以及跟踪结果vector的填充位置直接赋值

	static QDateTime startTime = QDateTime::currentDateTime();
	std::vector<PointXYZCT> dataCache(numCols * numRows);// 先按列批量读取数据到缓存
	// 避免使用单点getScanPoint,改用批量读取
	double* positions = new double[numRows * 3];
	int* reflections = new int[numRows];
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
	for (int col = 0; col < numCols; col++) {
		iQLibIfPtr->getXYZScanPoints(0, 0, col, numRows, positions, reflections);
		//iQLibIfPtr->getAutomationTimeOfScanPoint(0, 0, col, &atime); // 每个点差 (1 / 976000)秒 * 1, 000, 000 = 1.024微秒
		// 前半部分 时间开始递减
		for (int row = 0; row < numRows; ++row) {
			PointXYZCT& item = dataCache[col * numRows + row];
			item.x = positions[3 * row + 0]; item.y = positions[3 * row + 1]; item.z = positions[3 * row + 2];
			item.color = reflections[row];
		}
	}
	delete[] positions;
	delete[] reflections;
	qDebug() << "先按列批量读取数据到缓存:" << numCols << numRows << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
	// 然后并行处理数据重排和坐标变换
	startTime = QDateTime::currentDateTime();
	const double points_millisecond_increment = 1.024;// 1.0e6 / 976000.0;  // 976 kHz; ≈ 1.024 微秒/点
	double timeStep{ 0 };
	for (int c = 0; c < halfCols; ++c) {
		iQLibIfPtr->getAutomationTimeOfScanPoint(0, numRows-1, c, &atime);
		for (size_t r = numRows; r-- > 0;)// 前半部分：从下往上
		{
			PointXYZCT& Point = dataCache[r + c * numRows];
			Point.x = -Point.x;// 小车方向和扫描仪朝向相反，故将X置反，下同
			Point.time = atime + timeStep;
			timeStep += timeStep + points_millisecond_increment;
			pointRes[resultIndex++] = Point;// 直接赋值，没有push_back的开销
		}
		for (size_t r = 0; r < numRows; ++r) // 后半部分：从上往下
		{
			PointXYZCT& Point = dataCache[r + (c + halfCols) * numRows];
			Point.x = -Point.x;
			Point.time = atime + timeStep;
			timeStep += timeStep + points_millisecond_increment;
			pointRes[resultIndex++] = Point;
		}
	}
	qDebug() << "然后并行处理数据重排和坐标变换:" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
	return true;
}

bool FaroHandle::getFaroFlsFileStartAndEndTime(const char* fileName, unsigned __int64& start, unsigned __int64& end)
{
	static QDateTime startTime = QDateTime::currentDateTime();
	if (!loadFaroFlsFile(fileName)) {
		return false;
	}
	iQLibIfPtr->getAutomationTimeOfScanPoint(0, numRows - 1, 0, &start);
	iQLibIfPtr->getAutomationTimeOfScanPoint(0, 0, numCols - 1, &end);
	qDebug() << "获取一个fls文件的开始和结束时间:" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
	return true;
}

bool FaroHandle::loadFaroFlsFile(const char* fileName)
{
	if (!FaroLoadFlsFile(fileName)) {
		return false;
	}
	// 获取扫描维度
	numRows = iQLibIfPtr->getScanNumRows(0);
	numCols = iQLibIfPtr->getScanNumCols(0);
	return true;
}

/**
Number of San Lines

nc = 2f * t / r
With the factor f, the dependency on the measurement rate and the maximum
resolution of the scanner is covered. The factor f is classified as:
122 kHz: f = 3.05 Hz
244 kHz: f = 6.10 Hz
488 kHz: f = 12.2 Hz
976 kHz: f = 24.4 Hz
The 2 in the formula reflects the fact that one turn of the mirror creates two lines
in the scan.
Example: For a scan duration of 1h with 1/10 resolution and Quality 4x, the
expected number of columns is:
nc = 2*3.05Hz * 3600s * 10 = 219600


	ScanMode scanMode = ScanMode::HelicalCanGrey;
	StorageMode storageMode = StorageMode::SMRemote;
	int scanFileNum = 1;
	QString scanBaseName = "Scan";
	QString remoteScanStoragePath = "D://Scans//Default ";
	int resolution = 4;
	int measurementRate = 8;
	int noiseCompression = 1;
	int vertAngleMin = -60;
	int vertAngleMax = 90;
	int numCols = 2000000;
	int splitAfterLines = 5000;
Property int MeasurementRate (Read/Write)
The measurement rate or speed used to record the scans.
Possible values: 1, 2, 4, 8.
1 = 122.000 points per second
2 = 244.000 points per second
4 = 488.000 points per second
8 = 976.000 points per second
5000 = 2*24.4Hz * 25.614s * 4 = 219600

(1/976000)秒 * 1,000,000 = 1.024微秒

“扫描仪内部延迟”通常为10毫秒。
Sdk扫描控制扫描参数的选择核心是三个变量：速率，分辨率，噪声压缩。首先速率确定仪器每秒的出点数，固定1, 2, 4, 8几个固定选项。分辨率决定每圈扫描的点数（竖直角角度分辨率），有1, 2, 4, 5, 8, 10, 16, 20, 32几个固定选项。噪声压缩为数据降噪，但使用会减少点数，有1, 2, 4几个固定选项，1无压缩，2压缩4倍，4压缩16倍。上诉三个参数使用时不是任意组合，只有部分组合是可以被使用的。详细描述参考faro的sdk开发文档。

仪器的sdk控制响应，目前看来不算非常稳定流畅，经常会出现卡顿响应时间长的情况。Sdk控制仪器采集数据时，有以下一些注意事项：
1.螺旋扫描模式下需要输入扫描线数，扫描仪转一圈获取两条线。扫描线数输入时应为偶数，输入奇数时数量会被默认减1。
2.扫描线数小于100时，扫描数据可能会出现问题，比如后半部分数据全为0.
3.出现某次扫描未正常完成，下一次扫描的数据会接着存在上一个扫描文件中。
4.扫描文件的命名不正确会影响后期操作，一些特殊字符可能不可以输入，比如"："号

通过sdk从法如原始扫描数据（.fls）文件中获取扫描数据的过程相对比较耗时和占资源。可能因为.fls文件本身有很高的压缩率，故解析时比较麻烦。这种情况其实不适用在实时处理的应用场景中。TSD软件在实时处理数据的过程中有近半的时间耗在单纯的.fls文件的数据获取中。个人认为.fls文件或者法如的数据存储方案不适用实时处理的场景。

Sdk扫描控制扫描参数的选择核心是三个变量：速率，分辨率，噪声压缩。首先速率确定仪器每秒的出点数，固定1, 2, 4, 8几个固定选项。分辨率决定每圈扫描的点数（竖直角角度分辨率），有1, 2, 4, 5, 8, 10, 16, 20, 32几个固定选项。噪声压缩为数据降噪，但使用会减少点数，有1, 2, 4几个固定选项，1无压缩，2压缩4倍，4压缩16倍。上诉三个参数使用时不是任意组合，只有部分组合是可以被使用的。详细描述参考faro的sdk开发文档。

仪器的sdk控制响应，目前看来不算非常稳定流畅，经常会出现卡顿响应时间长的情况。Sdk控制仪器采集数据时，有以下一些注意事项：
1.螺旋扫描模式下需要输入扫描线数，扫描仪转一圈获取两条线。扫描线数输入时应为偶数，输入奇数时数量会被默认减1。
2.扫描线数小于100时，扫描数据可能会出现问题，比如后半部分数据全为0.
3.出现某次扫描未正常完成，下一次扫描的数据会接着存在上一个扫描文件中。
4.扫描文件的命名不正确会影响后期操作，一些特殊字符可能不可以输入，比如"："号

通过sdk从法如原始扫描数据（.fls）文件中获取扫描数据的过程相对比较耗时和占资源。可能因为.fls文件本身有很高的压缩率，故解析时比较麻烦。这种情况其实不适用在实时处理的应用场景中。TSD软件在实时处理数据的过程中有近半的时间耗在单纯的.fls文件的数据获取中。个人认为.fls文件或者法如的数据存储方案不适用实时处理的场景。

Sdk扫描控制扫描参数的选择核心是三个变量：速率，分辨率，噪声压缩。首先速率确定仪器每秒的出点数，固定1, 2, 4, 8几个固定选项。分辨率决定每圈扫描的点数（竖直角角度分辨率），有1, 2, 4, 5, 8, 10, 16, 20, 32几个固定选项。噪声压缩为数据降噪，但使用会减少点数，有1, 2, 4几个固定选项，1无压缩，2压缩4倍，4压缩16倍。上诉三个参数使用时不是任意组合，只有部分组合是可以被使用的。详细描述参考faro的sdk开发文档。

仪器的sdk控制响应，目前看来不算非常稳定流畅，经常会出现卡顿响应时间长的情况。Sdk控制仪器采集数据时，有以下一些注意事项：
1.螺旋扫描模式下需要输入扫描线数，扫描仪转一圈获取两条线。扫描线数输入时应为偶数，输入奇数时数量会被默认减1。
2.扫描线数小于100时，扫描数据可能会出现问题，比如后半部分数据全为0.
3.出现某次扫描未正常完成，下一次扫描的数据会接着存在上一个扫描文件中。
4.扫描文件的命名不正确会影响后期操作，一些特殊字符可能不可以输入，比如"："号

通过sdk从法如原始扫描数据（.fls）文件中获取扫描数据的过程相对比较耗时和占资源。可能因为.fls文件本身有很高的压缩率，故解析时比较麻烦。这种情况其实不适用在实时处理的应用场景中。TSD软件在实时处理数据的过程中有近半的时间耗在单纯的.fls文件的数据获取中。个人认为.fls文件或者法如的数据存储方案不适用实时处理的场景。

**/
