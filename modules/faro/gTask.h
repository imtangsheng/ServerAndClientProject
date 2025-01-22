#pragma once
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QString>

#include "Common_Structs.h"
using namespace CommonStructs;

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
//// ���������ݽṹ
//struct Mileage {
//    long long id;
//    double mileage;
//    long long time;
//    double mileage_revision;        // ����������
//    double mileage_align;
//    double mileage_auxiliary;
//    double time_raw;
//};

//��Ǽ� lsw 24/10/29 add
struct clinometer {
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

class Task
{
public:
	Task() = default;
	~Task() = default;
	QJsonObject taskObj;

private:

};