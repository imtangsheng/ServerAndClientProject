#pragma once
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QString>
#include <QDateTime>
#include <QDebug>

#include "iFaro.h"

// 定义结果数据结构
struct Mileage {
	long long id;
	double mileage;
	long long time;
	double mileage_revision;        // 修正后的里程
	double mileage_align;
	double mileage_auxiliary;
	double time_raw;
};


// 定义里程数据结构
struct MileageData {
	long long ID;
	double LeftMileage;
	double LeftTime;
	long long LeftTimeRaw;
	double RightMileage;
	double RightTime;
	long long RightTimeRaw;
};

//倾角计 lsw 24/10/29 add
struct Clinometer {
	int x;
	int y;
	long long time;
};

// 创建JSON数据
QJsonObject createTaskJson();
// 保存JSON到文件
bool saveJsonToFile(const QJsonObject& jsonObj, const QString& filePath);
// 从文件读取JSON
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

