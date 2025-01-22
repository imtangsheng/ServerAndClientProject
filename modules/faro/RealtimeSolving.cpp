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
		// ������JSON����
		QJsonObject root;

		// ������Բ��Ϣ
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

		// �������������Ϣ
		QJsonArray pointNumInf;
		for (const auto& it : pointcloudResTemp)
		{
			pointNumInf.append(static_cast<int>(it.size()));
		}

		// ��ӵ�������
		root["ellipse_info"] = ellipseInf;
		root["point_num"] = pointNumInf;

		// ����JSON�ĵ�
		QJsonDocument doc(root);

		// д���ļ�
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
	// �ҵ����һ��Ŀ¼�ָ�����/ �� \��
	size_t pos = filePath.find_last_of("/\\");
	std::string fileName = (pos != std::string::npos) ? filePath.substr(pos + 1) : filePath;

	// �ҵ��ļ���չ������ʼλ��
	pos = fileName.find_last_of('.');
	if (pos != std::string::npos) {
		fileName = fileName.substr(0, pos);  // ɾ����չ��
	}

	return fileName;
}

void RealtimeSolving::test()
{
	static QDateTime startTime = QDateTime::currentDateTime();
	std::string fls_path = "E:\\Test\\test\\PointCloud\\Scan001.fls";
	std::string task_path = "E:\\Test\\test\\Task";
	//#1 ִ�е��ƽ���
	std::vector<Mileage> mileage_vec;
	if(!get_mileage_from_file(task_path, mileage_vec)) return;
	bool direction = mileage_vec[0].mileage_revision < mileage_vec.back().mileage_revision;
	qDebug() << "#1 ��ȡmileage�ļ�" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
	//���ļ���ȡJSON
	QString task_info_path = QString::fromStdString(task_path) + "/../task_info.json";
	QJsonObject taskParams = loadJsonFromFile(task_info_path);
	qDebug() << "ProjectName:" <<  taskParams["ProjectName"].toString();
	//#1-1 ��ȡ��������
	FaroHandle faroHandle;
	std::vector< std::vector<PointCloudXYZCT>> pointcloudRes;
	if(!faroHandle.read_pointcloud_from_file(fls_path, pointcloudRes,direction)) return;
	qDebug() << "#1 ��ȡ��������" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
	//��ȡ ��Ǽ��ļ���ɨ����С��ʱ���ļ� "/Inclinometer.txt" "/scannerTime.txt"
	std::vector<clinometer>clinometerVecs; bool hasClinoData = false;
	if(get_clinometer_from_file(task_path, clinometerVecs)) hasClinoData = true;
	if(clinometerVecs.empty()) hasClinoData = false;
	qDebug() << "#1 ��ȡ��Ǽ��ļ�" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
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
	qDebug() << "#1 ɨ����С��ʱ������Ǽ�����ƥ��<" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
	//���ƽ���չ��
	Speed flag = kFastest;
	if (hasClinoData) {
		expandPointcloud(pointcloudRes, mileageWithTime, clinometerWithTime, flag, direction);
	}
	else {
		expandPointcloud(pointcloudRes, mileageWithTime, flag);
	}
	qDebug() << "#1 ���ƽ���չ��<" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
	//#2 ͼ������
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
	// 2024-6-19 fix:ͨ��ʹ��0.19���㣬���ʹ��0.09����
	if (mergeType == 0)
		map.holeDepthLimit = 0.09;
	else if (mergeType == 1)
		map.holeDepthLimit = 0.19;

	qDebug() << "#2 ͼ������" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
	std::vector<std::vector<pointcloudDef::PointCloudSection> > tunnel;
	tunnel.resize(pointcloudRes.size());
	for (size_t i = 0; i < pointcloudRes.size(); i++)
	{
		tunnel[i].resize(pointcloudRes[i].size());
		size_t index = 0; //ʹ��������ֵ
		for (const auto& point : pointcloudRes[i]) {
			pointcloudDef::PointCloudSection newPoint;
			// ���ƻ��������
			static_cast<PointCloudXYZCT&>(newPoint) = point;
			// ����ж���ĳ�Ա������Ҫ��ʼ��������������
			tunnel[i][index++] = newPoint;
		}
	}
	qDebug() << "#2 ͼ�����ɵ�������תΪͼ���������" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
	//ͼ������
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
	qDebug() << "#2 ͼ������ ���ͼ�ͻҶ�ͼ:" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
	// д��Բ�͵�����Ϣ
	save_baseinfo_file(task_path, fls_base_name, lme, tunnel);

	return;
}

