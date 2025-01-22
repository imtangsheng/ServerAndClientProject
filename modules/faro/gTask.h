#pragma once
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QString>

#include "Common_Structs.h"
using namespace CommonStructs;

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
//// 定义结果数据结构
//struct Mileage {
//    long long id;
//    double mileage;
//    long long time;
//    double mileage_revision;        // 修正后的里程
//    double mileage_align;
//    double mileage_auxiliary;
//    double time_raw;
//};

//倾角计 lsw 24/10/29 add
struct clinometer {
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

class Task
{
public:
	Task() = default;
	~Task() = default;
	QJsonObject taskObj;

private:

};