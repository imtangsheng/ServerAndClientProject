#pragma once
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QString>
#include <QDateTime>
#include <QDebug>

#include "iFaro.h"

// ���������ݽṹ
struct Mileage {
	long long id;
	double mileage;
	long long time;
	double mileage_revision;        // ����������
	double mileage_align;
	double mileage_auxiliary;
	double time_raw;
};


// ����������ݽṹ
struct MileageData {
	long long ID;
	double LeftMileage;
	double LeftTime;
	long long LeftTimeRaw;
	double RightMileage;
	double RightTime;
	long long RightTimeRaw;
};

//��Ǽ� lsw 24/10/29 add
struct Clinometer {
	int x;
	int y;
	long long time;
};

// ����JSON����
QJsonObject createTaskJson();
// ����JSON���ļ�
bool saveJsonToFile(const QJsonObject& jsonObj, const QString& filePath);
// ���ļ���ȡJSON
QJsonObject loadJsonFromFile(const QString& filePath);
//
struct TaskFaroPart
{
	QString task_dir{""};
	QString faro_file{ "" };
	QString mileage_file{ "" };
	QString clinometer_file{ "" };

	bool direction{true};
	double start_mileage{0};
	double end_mileage{0};
	Speed flag{ kFastest };

	int resolving{200};
	double diameter{8.0};

	std::map<double, double> mileageWithTime;
	std::map<double, double> clinometerWithTime;

	std::vector<Mileage> mileage;
	std::vector<Clinometer> clinometer;
	std::vector< std::vector<PointCloud>> points;
};

