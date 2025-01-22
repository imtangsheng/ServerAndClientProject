#include "RealtimeSolving.h"
#include "TaskHandle.h"
#include "FaroHandle.h"
//#include "PointCloudHandle.h"

#include "generate_image_map.h"
static void save_baseinfo_file(const std::string& taskPath, const std::string& targetName,
	const std::vector<pointcloudDef::MileageEllipse>& lme,
	const std::vector<std::vector<pointcloudDef::PointCloudSection>>& pointcloudResTemp)
{
	QString baseinfo_path = QString::fromStdString(taskPath + "/TempVar/" + targetName + "_baseInf.json");
	QFile file(baseinfo_path);

	if (file.open(QIODevice::WriteOnly))
	{
		// 创建主JSON对象
		QJsonObject root;

		// 处理椭圆信息
		QJsonArray ellipseInf;
		for (const auto& it : lme)
		{
			QJsonObject ellipseItem;
			ellipseItem["mileage"] = it.mileage;
			ellipseItem["col"] = it.col;
			ellipseItem["dm"] = it.dm;
			ellipseItem["ep_A"] = it.ep.A;
			ellipseItem["ep_angle"] = it.ep.angle;
			ellipseItem["ep_B"] = it.ep.B;
			ellipseItem["ep_x"] = it.ep.x;
			ellipseItem["ep_y"] = it.ep.y;
			ellipseInf.append(ellipseItem);
		}

		// 处理点云数量信息
		QJsonArray pointNumInf;
		for (const auto& it : pointcloudResTemp)
		{
			pointNumInf.append(static_cast<int>(it.size()));
		}

		// 添加到根对象
		root["ellipse_info"] = ellipseInf;
		root["point_num"] = pointNumInf;

		// 创建JSON文档
		QJsonDocument doc(root);

		// 写入文件
		file.write(doc.toJson(QJsonDocument::Indented));
		file.close();
	}
}
RealtimeSolving::RealtimeSolving()
{
}

RealtimeSolving::~RealtimeSolving()
{
}

static std::string baseName(const std::string& filePath)
{
	// 找到最后一个目录分隔符（/ 或 \）
	size_t pos = filePath.find_last_of("/\\");
	std::string fileName = (pos != std::string::npos) ? filePath.substr(pos + 1) : filePath;

	// 找到文件扩展名的起始位置
	pos = fileName.find_last_of('.');
	if (pos != std::string::npos) {
		fileName = fileName.substr(0, pos);  // 删除扩展名
	}

	return fileName;
}

void RealtimeSolving::test()
{
	static QDateTime startTime = QDateTime::currentDateTime();
	std::string fls_path = "E:\\Test\\test\\PointCloud\\Scan001.fls";
	std::string task_path = "E:\\Test\\test\\Task";
	//#1 执行点云解析
	std::vector<Mileage> mileage_vec;
	if(!get_mileage_from_file(task_path, mileage_vec)) return;
	bool direction = mileage_vec[0].mileage_revision < mileage_vec.back().mileage_revision;
	qDebug() << "#1 读取mileage文件" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
	//从文件读取JSON
	QString task_info_path = QString::fromStdString(task_path) + "/../task_info.json";
	QJsonObject taskParams = loadJsonFromFile(task_info_path);
	qDebug() << "ProjectName:" <<  taskParams["ProjectName"].toString();
	//#1-1 读取点云数据
	FaroHandle faroHandle;
	std::vector< std::vector<PointCloudXYZCT>> pointcloudRes;
	if(!faroHandle.read_pointcloud_from_file(fls_path, pointcloudRes,direction)) return;
	qDebug() << "#1 读取点云数据" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
	//读取 倾角计文件和扫描仪小车时间文件 "/Inclinometer.txt" "/scannerTime.txt"
	std::vector<clinometer>clinometerVecs; bool hasClinoData = false;
	if(get_clinometer_from_file(task_path, clinometerVecs)) hasClinoData = true;
	if(clinometerVecs.empty()) hasClinoData = false;
	qDebug() << "#1 读取倾角计文件" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
	std::map<double, double> mileageWithTime;
	for (auto& mVecsIt : mileage_vec)
	{
		mileageWithTime.insert(std::make_pair(std::abs(mVecsIt.mileage_revision), mVecsIt.time));
	}
	std::map<double, double> clinometerWithTime;
	for (auto& clinometerVecsIt : clinometerVecs)
	{
		clinometerWithTime.insert(std::make_pair(clinometerVecsIt.time, clinometerVecsIt.x));
	}
	qDebug() << "#1 扫描仪小车时间与倾角计数据匹配<" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
	//点云解析展开
	Speed flag = kFastest;
	if (hasClinoData) {
		expandPointcloud(pointcloudRes, mileageWithTime, clinometerWithTime, flag, direction);
	}
	else {
		expandPointcloud(pointcloudRes, mileageWithTime, flag);
	}
	qDebug() << "#1 点云解析展开<" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
	//#2 图像生成
	QString accuracy = taskParams["MissionContent"].toObject()["Accuracy"].toString("5");
	int resolving = 1000 / accuracy.toInt();
	double diameter = taskParams["MissionContent"].toObject()["Diameter"].toDouble(8.0);
	std::string fls_base_name = baseName(fls_path);
	std::string grayPath = task_path + "/TempVar/" + fls_base_name + "_gray.tiff";
	std::string depthPath = task_path + "/TempVar/" + fls_base_name + "_depth.tiff";
	
	std::vector<pointcloudDef::MileageEllipse> lme;
	//PointCloudImage map;
	Generate_image_map map;
	int mergeType = std::stoi("1");
	// 2024-6-19 fix:通缝使用0.19计算，错缝使用0.09计算
	if (mergeType == 0)
		map.holeDepthLimit = 0.09;
	else if (mergeType == 1)
		map.holeDepthLimit = 0.19;

	qDebug() << "#2 图像生成" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
	std::vector<std::vector<pointcloudDef::PointCloudSection> > tunnel;
	tunnel.resize(pointcloudRes.size());
	for (size_t i = 0; i < pointcloudRes.size(); i++)
	{
		tunnel[i].resize(pointcloudRes[i].size());
		size_t index = 0; //使用索引赋值
		for (const auto& point : pointcloudRes[i]) {
			pointcloudDef::PointCloudSection newPoint;
			// 复制基类的数据
			static_cast<PointCloudXYZCT&>(newPoint) = point;
			// 如果有额外的成员变量需要初始化，在这里设置
			tunnel[i][index++] = newPoint;
		}
	}
	qDebug() << "#2 图像生成点云数据转为图像点云数据" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
	//图像生成
	map.GenerateDepthAndGrayImage(
		lme,
		tunnel,
		grayPath,
		depthPath,
		_StartMileage,
		_EndMileage,
		resolving,
		diameter / 2
	);
	qDebug() << "#2 图像生成 深度图和灰度图:" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
	// 写椭圆和点数信息
	save_baseinfo_file(task_path, fls_base_name, lme, tunnel);

	return;
}

