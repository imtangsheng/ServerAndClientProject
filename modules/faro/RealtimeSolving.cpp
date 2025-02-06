#include "RealtimeSolving.h"
#include <QDir>
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

bool RealtimeSolving::writeFaroImage(TaskFaroPart& task, const QString& imagePath)
{
	static QDateTime startTime = QDateTime::currentDateTime();
	//#1-1 ִ�е��ƽ��� ��ȡ���
	if (!get_mileage_from_file(task.task_dir.toStdString(), task.mileage)) return false;
	task.direction = task.mileage[0].mileage_revision < task.mileage.back().mileage_revision;
	qDebug() << "#1 ��ȡmileage�ļ�" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";

	//#1-2 ִ�е��ƽ��� ��Ǽ��ļ���ɨ����С��ʱ���ļ� "/Inclinometer.txt" "/scannerTime.txt"
	bool hasClinoData = false;
	if (get_clinometer_from_file(task.task_dir.toStdString(), task.clinometer)) hasClinoData = true;
	if (task.clinometer.empty()) hasClinoData = false;
	qDebug() << "#1 ��ȡ��Ǽ��ļ�" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
	for (auto& mVecsIt : task.mileage){
		task.mileageWithTime.insert(std::make_pair(std::abs(mVecsIt.mileage_revision), mVecsIt.time));
	}
	for (auto& clinometerVecsIt : task.clinometer){
		task.clinometerWithTime.insert(std::make_pair(clinometerVecsIt.time, clinometerVecsIt.x));
	}
	qDebug() << "#1 ɨ����С��ʱ������Ǽ�����ƥ��" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
	//#1-��ȡ��������
	if (!ReadFaroFileData(task.faro_file.toStdString(), task.points, task.direction)) return false;
	qDebug() << "#1 ��ȡ��������" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";

	//#2-���ƽ���չ��
	if (!PointCloudExpand(hasClinoData,task)) return false;


	qDebug() << "#2 ���ƽ���չ��" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";

	//#2 ͼ������
	QString fls_base_name = baseName(task.faro_file.toStdString()).data();
	QString grayPath = QDir(imagePath).filePath(fls_base_name + "_gray.jpg");//�ø�ʽ������,��С1566KB tiffΪ��ѹ��ͼ���ʽ:6704KB opencv:Ϊ5786KB
	QString depthPath = QDir(imagePath).filePath(fls_base_name + "_depth.jpg");//181KB tiff:6709KB opencv:371KB
	task.resolving = 1000 / 5;
	task.diameter = 5.4;
	if(!WritePointCloudImage(task,grayPath, depthPath)) return false;
	qDebug() << "#2 ͼ������ ���ͼ�ͻҶ�ͼ" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
	return true;
}

void RealtimeSolving::test()
{
	TaskFaroPart task;
	task.faro_file = "E:/Test/test/PointCloud/Scan001.fls";
	task.mileage_file = "E:/Test/test/Task/mileage.txt";
	task.clinometer_file = "E:/Test/test/Task/Inclinometer.txt";
	task.task_dir = "E:/Test/test/Task";

	QString imagePath = "E:/Test/test/PointCloudImage/";
	qDebug() << "#ͼ������ ���ͼ�ͻҶ�ͼ ���" << writeFaroImage(task, imagePath);
	return;
}

