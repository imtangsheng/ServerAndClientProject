#include <iostream>
#include <sstream>
#include <fstream>
#include "TaskHandle.h"
#include <mutex>

double _StartMileage = 0.0;
double _EndMileage = 0.0;

// ����JSON����
QJsonObject createTaskJson()
{
    QJsonObject missionObj;
    missionObj["Accuracy"] = 5.0;
    missionObj["Author"] = "";
    missionObj["BetweenName"] = "";
    missionObj["CameraPostion"] = "";
    missionObj["CreateTime"] = "";
    missionObj["DeviceModel"] = "MS201";
    missionObj["Diameter"] = 5.4;
    missionObj["ExposeTime"] = 600;
    missionObj["LastWorkTime"] = "";
    missionObj["LineName"] = "";
    missionObj["LineType"] = "";
    missionObj["OverlapRate"] = 20;
    missionObj["RingWidth"] = 1.2;
    missionObj["SegmentWidth"] = 1.5;
    missionObj["Speed"] = 1500;
    missionObj["TrolleyDirection"] = "";
    missionObj["jobCount"] = 1;
    missionObj["note"] = "";

    QJsonObject taskObj;
    taskObj["DeviceType"] = "MS201";
    taskObj["JobName"] = "";
    taskObj["MissionContent"] = missionObj;
    taskObj["ProjectName"] = "";

    return taskObj;
}

// ����JSON���ļ�
bool saveJsonToFile(const QJsonObject& jsonObj, const QString& filePath)
{
    QJsonDocument doc(jsonObj);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    file.write(doc.toJson(QJsonDocument::Indented)); // Indented ��ʽ�����
    file.close();
    return true;
}

// ���ļ���ȡJSON
QJsonObject loadJsonFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QJsonObject();
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        return QJsonObject();
    }

    return doc.object();
}

static bool GetMile(const std::string& path, std::vector<MileageData>& data) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Failed to open file: " << path << std::endl;
        return false;
    }

    std::string line;
    std::getline(file, line); // ����������

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        MileageData entry;

        // ���ո���Ʊ������ÿһ��
        if (iss >> entry.ID >> entry.LeftMileage >> entry.LeftTime >> entry.LeftTimeRaw
            >> entry.RightMileage >> entry.RightTime >> entry.RightTimeRaw) {
            data.push_back(entry);
        }
        else {
            std::cerr << "Error: Failed to parse line: " << line << std::endl;
        }
    }

    file.close();
    return true;
}

static void Interp1(const std::vector<long long>& Time, const std::vector<MileageData>& data, std::vector<MileageData>& Mixdata) {
    size_t num = Time.size();
    size_t numIn = data.size();
    Mixdata.resize(num);

    for (size_t i = 0; i < num; ++i) {
        Mixdata[i].RightTimeRaw = Time[i];
        Mixdata[i].LeftTimeRaw = Time[i];
    }
    //����̲�ֵ
    int i = 1; int j; double ratio, ratio1, ratio2, Mile; long long MileRaw;
    for (j = 1; j < numIn; j++) {
        while (Time[i] <= data[j].LeftTimeRaw) {
            ratio = static_cast<double>(Time[i] - data[j - 1].LeftTimeRaw) / static_cast<double>(data[j].LeftTimeRaw - data[j - 1].LeftTimeRaw);
            Mile = data[j].LeftMileage - data[j - 1].LeftMileage;//ĳ�������������
            MileRaw = data[j].LeftTime - data[j - 1].LeftTime;//ĳ������ʱ������
            Mixdata[i].LeftMileage = Mile * ratio + data[j - 1].LeftMileage;
            Mixdata[i].LeftTime = MileRaw * ratio + data[j - 1].LeftTime;
            i++;
            if (i >= num) {
                break;
            }
        }
        if (i >= num) {
            break;
        }
    }

    if (Time[num - 1] > data[numIn - 1].LeftTimeRaw) {
        Mixdata[num - 1].LeftMileage = data[numIn - 1].LeftMileage;
        Mixdata[num - 1].LeftTime = Time[num - 1] - data[numIn - 1].LeftTimeRaw + data[numIn - 1].LeftTime;
    }
    Mixdata[0].LeftMileage = 0.0; Mixdata[0].LeftTime = Mixdata[1].LeftTime - 1e5;
    //����̲�ֵ
    i = 1;
    for (j = 1; j < numIn; j++) {
        while (Time[i] <= data[j].RightTimeRaw) {
            ratio = static_cast<double>(Time[i] - data[j - 1].RightTimeRaw) / static_cast<double>(data[j].RightTimeRaw - data[j - 1].RightTimeRaw);
            Mile = data[j].RightMileage - data[j - 1].RightMileage;//ĳ�������������
            MileRaw = data[j].RightTime - data[j - 1].RightTime;//ĳ������ʱ������
            Mixdata[i].RightMileage = Mile * ratio + data[j - 1].RightMileage;
            Mixdata[i].RightTime = MileRaw * ratio + data[j - 1].RightTime;
            i++;
            if (i >= num) {
                break;
            }
        }
        if (i >= num) {
            break;
        }
    }

    if (Time[num - 1] > data[numIn - 1].RightTimeRaw) {
        Mixdata[num - 1].RightMileage = data[numIn - 1].RightMileage;
        Mixdata[num - 1].RightTime = Time[num - 1] - data[numIn - 1].RightTimeRaw + data[numIn - 1].RightTime;
    }
    Mixdata[0].RightMileage = 0.0; Mixdata[0].RightTime = Mixdata[1].RightTime - 1e5;
}

static bool LeftRightMileageFusion(const std::string& path, std::vector<Mileage>& result) {
    std::vector<MileageData> data, Mixdata;

    // ��ȡ����
    if (!GetMile(path, data)) {
        std::cerr << "Error: Failed to read data from file: " << path << std::endl;
        return false;
    }
    size_t num = data.size();
    if (num == 0) {
        std::cerr << "Error: No data found in file: " << path << std::endl;
        return false;
    }
    // �����������ʹ���
    long long TimeStart = 0, TimeEnd = 0;
    if (data[num - 1].RightTimeRaw < data[num - 1].LeftTimeRaw) {
        TimeStart = data[1].RightTimeRaw;
        TimeEnd = data[num - 1].RightTimeRaw;
    }
    else {
        TimeStart = data[1].LeftTimeRaw;
        TimeEnd = data[num - 1].LeftTimeRaw;
    }

    size_t Timenum = std::ceil((TimeEnd - TimeStart) / 1e5);
    std::vector<long long> Time(Timenum + 2);
    long long interval = 1e5; // ��� 0.1s
    // ����ʱ������
    for (size_t i = 1; i < (Timenum + 1); ++i) {
        Time[i] = TimeStart + (i - 1) * interval;
    }
    Time[0] = Time[1] - 1e5;
    Time[Timenum + 1] = TimeEnd;
    // ��ֵ
    Interp1(Time, data, Mixdata);
    size_t Resnum = Timenum + 2; // Mixdata.size()
    result.resize(Resnum);
    // ��ʼ����һ�����
    result[0].id = 1;
    result[0].mileage = data[0].RightMileage;
    result[0].time = data[0].RightTime;
    result[0].mileage_align = result[0].mileage;
    result[0].mileage_revision = result[0].mileage;
    result[0].time_raw = data[0].RightTimeRaw;
    // ����ʣ����
    double dMLeft = 0.0, dMRight = 0.0, maxAbsValue = 0.0, dMileage = 0.0;
    for (size_t i = 1; i < Resnum; ++i) {
        result[i].id = static_cast<long long>(i + 1);
        // ����������̲�ֵֵ
        dMLeft = Mixdata[i].LeftMileage - Mixdata[i - 1].LeftMileage;
        dMRight = Mixdata[i].RightMileage - Mixdata[i - 1].RightMileage;
        maxAbsValue = (std::abs(dMLeft) > std::abs(dMRight)) ? dMLeft : dMRight;
        // �жϲ�ֵ�Ƿ񳬹���ֵ
        const double LeftAndRightMileageMaximumDifferenceValue = 0.3;
        if (std::abs((dMLeft - dMRight) / maxAbsValue) >= 0.3) {
            dMileage = maxAbsValue;
        }
        else {
            dMileage = (dMLeft + dMRight) / 2;
        }
        // ���½��
        result[i].mileage = result[i - 1].mileage + dMileage;
        result[i].time = (Mixdata[i].LeftTime + Mixdata[i].RightTime) / 2;
        result[i].mileage_revision = result[i].mileage;
        result[i].mileage_align = result[i].mileage;
        result[i].time_raw = Mixdata[i].RightTimeRaw;
    }
    return true;
}

bool get_mileage_from_file(const  std::string& taskPath, std::vector<Mileage>& mileage)
{
    std::string srcMileage = taskPath + "/mileage.txt";
    std::ifstream f(srcMileage);
    if (!f.is_open())
    {
        return false;
    }
    f.close();
    if (LeftRightMileageFusion(srcMileage, mileage)) {
        return true;
    }
    return false;
}


bool get_clinometer_from_file(const std::string& taskPath, std::vector<clinometer>& vec)
{
    vec.clear();
    std::string srcPath = taskPath + "/Inclinometer.txt";
    std::string sctimePath = taskPath + "/scannerTime.txt";

    //24/11/1
    std::vector<long long> trolleyTimes;
    std::vector<long long> scannerTimes;

    // ����ļ������Բ����Դ��ļ�
    std::ifstream fc(sctimePath), f(srcPath);
    if (!fc.is_open() || !f.is_open()) {
        return false;
    }

    // ������ͷ��
    char tableHeader[1024] = { 0 };
    fc.getline(tableHeader, 1024);
    // ����ļ��Ƿ��ڱ�ͷ��ֱ�ӵ���β��
    if (fc.eof()) {
        fc.close();
        f.close();
        return false;  // �ļ�ֻ�б�ͷ��û�������У�ֱ�ӷ��� false
    }

    // ���ж�ȡ�ļ�����
    while (!fc.eof()) {
        char data[128] = { 0 };
        fc.getline(data, 128);
        std::string lineData = data;

        std::vector<std::string> datas;
        std::string item = "";
        const char& ch = '\t';
        for (size_t i = 0; i < lineData.size(); ++i)
        {
            if (lineData[i] == ch)
            {
                datas.push_back(item);
                item.clear();
                continue;
            }
            item.append(1, lineData[i]);
        }
        
        if (datas.size() < 3) {
            fc.close();
            f.close();
            return false; // �����и�ʽ����ȷ
        }
        long long trolleyTime = std::stoll(datas[1]);
        long long scannerTime = std::stoll(datas[2]);
        trolleyTimes.push_back(trolleyTime);
        scannerTimes.push_back(scannerTime);
    }
    fc.close();

    // ��ʼ���� clinometer �ļ�
    int i = 0; double dt = 1.0;
    f.getline(tableHeader, 1024); // ������ͷ��
    while (!f.eof()) {
        char data[128] = { 0 };
        f.getline(data, 128);
        std::string lineData = data;
        clinometer temp;
        std::vector<std::string> datas;
        std::string item = "";
        const char& ch = '\t';
        for (size_t i = 0; i < lineData.size(); ++i)
        {
            if (lineData[i] == ch)
            {
                datas.push_back(item);
                item.clear();
                continue;
            }
            item.append(1, lineData[i]);
        }
        temp.x = std::stoi(datas[0]);
        temp.y = std::stoi(datas[1]);
        long long currentTrolleyTime = std::stoll(datas[2]);

        if (currentTrolleyTime < trolleyTimes.front() || currentTrolleyTime > trolleyTimes.back()) {
            dt = 1.0;
        }
        else {
            // Ѱ����ȷ�� i ֵ��ʹ currentTrolleyTime �� trolleyTimes[i] �� trolleyTimes[i+1] ֮��
            while (i < trolleyTimes.size() - 2 && currentTrolleyTime > trolleyTimes[i + 1]) {
                i++;
            }
            // �������Բ�ֵ��ʱ�����
            dt = static_cast<double>(scannerTimes[i + 1] - scannerTimes[i]) / (trolleyTimes[i + 1] - trolleyTimes[i]);
        }
        temp.time = scannerTimes[i] + (currentTrolleyTime - trolleyTimes[i]) * dt;
        //temp.time = datas[2].toLongLong() + sctime;
        vec.push_back(temp);
    }
    f.close();
    return true;
}

template<class T>
std::pair<int, int> binarySearch(const std::vector<T>& dataMap, const T& target)
{
    int left = 0, right = dataMap.size() - 1;
    while (left <= right)
    {
        int mid = (left + right) / 2;
        if (mid > 0 && target >= dataMap[mid - 1] && target <= dataMap[mid])
        {
            return std::pair<int, int>(mid - 1, mid); // �ҵ�Ŀ��ֵ����������
        }

        if (target < dataMap[mid])
        {
            right = mid - 1; // Ŀ��ֵ�����ߣ���С�Ҷ˵�
        }
        else
        {
            left = mid + 1; // Ŀ��ֵ���Ұ�ߣ�������˵�
        }
    }
    return std::pair<int, int>(0, 1); // δ�ҵ�Ŀ��ֵ�����ص�һ��
}


bool expandPointcloud(std::vector<std::vector<PointCloudXYZCT> >& pointcloudRes, const std::map<double, double>& mileageWithTime, Speed flag)
{
    std::vector<double> timeSet;
    for (auto iter = mileageWithTime.begin(); iter != mileageWithTime.end(); iter++)
    {
        timeSet.push_back(iter->second);
    }

    std::vector<std::thread> threads;
    int span = flag;

    for (int i = 0; i < pointcloudRes.size(); i += span)
    {
        {
            double t = pointcloudRes[i].front().time;
            if (t < timeSet.front())
            {
                pointcloudRes[i].front().y = mileageWithTime.begin()->first;
            }
            else if (t > timeSet.back())
            {
                pointcloudRes[i].front().y = (--mileageWithTime.end())->first;
            }
            else
            {
                std::pair<int, int> pos = binarySearch<double>(timeSet, t);
                std::map<double, double>::const_iterator it = mileageWithTime.begin();
                std::advance(it, pos.first);  // ��λ���� x �Եĵ�����
                double t1 = it->second;
                double s1 = it->first;
                it = mileageWithTime.begin();
                std::advance(it, pos.second);  // ��λ���� x �Եĵ�����
                double t2 = it->second;
                double s2 = it->first;
                double s = ((s2 - s1) / (t2 - t1)) * (t - t1) + s1;
                pointcloudRes[i].front().y = s;
            }
        }
        int num;
        // second
        {
            if ((i + span - 1) >= pointcloudRes.size())
                num = pointcloudRes.size() - 1;
            else
                num = i + span - 1;

            double t = pointcloudRes[num].back().time;
            if (t < timeSet.front())
            {
                pointcloudRes[num].back().y = mileageWithTime.begin()->first;
            }
            else if (t > timeSet.back())
            {
                pointcloudRes[num].back().y = (--mileageWithTime.end())->first;
            }
            else
            {
                std::pair<int, int> pos = binarySearch<double>(timeSet, t);
                std::map<double, double>::const_iterator it = mileageWithTime.begin();
                std::advance(it, pos.first);  // ��λ���� x �Եĵ�����
                double t1 = it->second;
                double s1 = it->first;
                it = mileageWithTime.begin();
                std::advance(it, pos.second);  // ��λ���� x �Եĵ�����
                double t2 = it->second;
                double s2 = it->first;
                double s = ((s2 - s1) / (t2 - t1)) * (t - t1) + s1;
                pointcloudRes[num].back().y = s;
            }
        }

        // average
        double tempSpeed = (pointcloudRes[num].back().y - pointcloudRes[i].front().y) / (pointcloudRes[num].back().time - pointcloudRes[i].front().time);
        auto func = [&](int _idx, int _span, double _speed, time_t beginT, double beginS)
            {
                for (int j = _idx; j <= _span; j++)
                {
                    for (int k = 0; k < pointcloudRes[j].size(); k++)
                    {
                        time_t deltaT = pointcloudRes[j][k].time - beginT; // ���һ�����ʱ���
                        double d_mileage = deltaT * _speed + beginS;
                        pointcloudRes[j][k].y = d_mileage;
                    }
                }
            };
        std::thread t(func, i, num, tempSpeed, pointcloudRes[i].front().time, pointcloudRes[i].front().y);
        threads.push_back(std::move(t));
    }

    for (auto& t : threads)
    {
        t.join();
    }

    _EndMileage = pointcloudRes.back().back().y;
    _StartMileage = pointcloudRes[0][0].y;

    return true;
}

//24/10/30 ����չ�� ��̼�+��Ǽ� 
bool expandPointcloud(std::vector<std::vector<PointCloudXYZCT> >& pointcloudRes, const std::map<double, double>& mileageWithTime, const std::map<double, double>& clinometerWithTime, Speed flag, bool Resverse)
{
    std::vector<double> timeSet;
    for (auto iter = mileageWithTime.begin(); iter != mileageWithTime.end(); iter++)
    {
        timeSet.push_back(iter->second);
    }
    // 1. ������ǼƵ�ʱ���ֵ��Χ 24/10/30
    std::vector<double> clinometertimeSet;
    for (auto iter = clinometerWithTime.begin(); iter != clinometerWithTime.end(); iter++)
    {
        clinometertimeSet.push_back(iter->first);
    }
    /*-----------------------------------------------------------------------------------------*/

    std::vector<std::thread> threads;
    int span = flag;

    for (int i = 0; i < pointcloudRes.size(); i += span)
    {
        {
            double t = pointcloudRes[i].front().time;
            if (t < timeSet.front())
            {
                pointcloudRes[i].front().y = mileageWithTime.begin()->first;
            }
            else if (t > timeSet.back())
            {
                pointcloudRes[i].front().y = (--mileageWithTime.end())->first;
            }
            else
            {
                std::pair<int, int> pos = binarySearch<double>(timeSet, t);
                std::map<double, double>::const_iterator it = mileageWithTime.begin();
                std::advance(it, pos.first);  // ��λ���� x �Եĵ�����
                double t1 = it->second;
                double s1 = it->first;
                it = mileageWithTime.begin();
                std::advance(it, pos.second);  // ��λ���� x �Եĵ�����
                double t2 = it->second;
                double s2 = it->first;
                double s = ((s2 - s1) / (t2 - t1)) * (t - t1) + s1;
                pointcloudRes[i].front().y = s;
            }
        }

        // 2. ͨ����ֵ������ƿ�����һ�������� 24/10/30
        double clinometermin;
        {
            double t = pointcloudRes.at(i).back().time;
            if (t < clinometertimeSet.front())
            {
                clinometermin = (clinometerWithTime.begin()->first - 15000) / 1000;
            }
            else if (t > clinometertimeSet.back())
            {
                clinometermin = ((--clinometerWithTime.end())->first - 15000) / 1000;
            }
            else
            {
                std::pair<int, int> clinometerpos = binarySearch<double>(clinometertimeSet, t);
                std::map<double, double>::const_iterator it = clinometerWithTime.begin();
                std::advance(it, clinometerpos.first);  // ��λ����ֵ�������ʼ��
                double s1 = it->second;
                double t1 = it->first;
                it = clinometerWithTime.begin();
                std::advance(it, clinometerpos.second);  // ��λ����ֵ����Ľ�����
                double s2 = it->second;
                double t2 = it->first;
                double s = ((s2 - s1) / (t2 - t1)) * (t - t1) + s1;
                clinometermin = (s - 15000) / 1000;
            }
        }

        int num;
        // second
        {
            if ((i + span - 1) >= pointcloudRes.size())
                num = pointcloudRes.size() - 1;
            else
                num = i + span - 1;

            double t = pointcloudRes[num].back().time;
            if (t < timeSet.front())
            {
                pointcloudRes[num].back().y = mileageWithTime.begin()->first;
            }
            else if (t > timeSet.back())
            {
                pointcloudRes[num].back().y = (--mileageWithTime.end())->first;
            }
            else
            {
                std::pair<int, int> pos = binarySearch<double>(timeSet, t);
                std::map<double, double>::const_iterator it = mileageWithTime.begin();
                std::advance(it, pos.first);  // ��λ���� x �Եĵ�����
                double t1 = it->second;
                double s1 = it->first;
                it = mileageWithTime.begin();
                std::advance(it, pos.second);  // ��λ���� x �Եĵ�����
                double t2 = it->second;
                double s2 = it->first;
                double s = ((s2 - s1) / (t2 - t1)) * (t - t1) + s1;
                pointcloudRes[num].back().y = s;
            }
        }

        // 3. ͨ����ֵ������ƿ�ĵ�һ�������� 24/10/30
        double clinometermax;
        {
            double t = pointcloudRes.at(num).front().time;
            if (t < clinometertimeSet.front())
            {
                clinometermax = (clinometerWithTime.begin()->first - 15000) / 1000;
            }
            else if (t > clinometertimeSet.back())
            {
                clinometermax = ((--clinometerWithTime.end())->first - 15000) / 1000;
            }
            else
            {
                std::pair<int, int> clinometerpos = binarySearch<double>(clinometertimeSet, t);
                std::map<double, double>::const_iterator it = clinometerWithTime.begin();
                std::advance(it, clinometerpos.first);  // ��λ����ֵ�������ʼ��
                double s1 = it->second;
                double t1 = it->first;
                it = clinometerWithTime.begin();
                std::advance(it, clinometerpos.second);  // ��λ����ֵ����Ľ�����
                double s2 = it->second;
                double t2 = it->first;
                double s = ((s2 - s1) / (t2 - t1)) * (t - t1) + s1;
                clinometermax = (s - 15000) / 1000;
            }
        }


        // average
        double tempSpeed = (pointcloudRes[num].back().y - pointcloudRes[i].front().y) / (pointcloudRes[num].back().time - pointcloudRes[i].front().time);
        // 4. ������Ǳ仯���ʣ����ٶȣ�24/10/30
        double tempclinometerSpeed = (clinometermin - clinometermax) / (pointcloudRes[i].back().time - pointcloudRes.at(i).front().time);
        double kangle = 1.0; if (Resverse) { kangle = -1.0; }

        //auto func = [&](int _idx, int _span, double _speed, time_t beginT, double beginS)
        auto func = [&](int _idx, int _span, double _speed, time_t beginT, double beginS, double clinometerSpeed, double clinometermin)//24/10/30
            {
                for (int j = _idx; j <= _span; j++)
                {
                    for (int k = 0; k < pointcloudRes[j].size(); k++)
                    {
                        time_t deltaT = pointcloudRes[j][k].time - beginT; // ���һ�����ʱ���
                        double d_mileage = deltaT * _speed + beginS;
                        pointcloudRes[j][k].y = d_mileage;

                        // Compute current angle using clinometer speed and minimum angle
                        double angle = clinometermin + (pointcloudRes[j].back().time - pointcloudRes[j][k].time) * clinometerSpeed;

                        double c = cos(-angle * M_PI / 180.0 * kangle);
                        double s = sin(-angle * M_PI / 180.0 * kangle);

                        // Apply rotation to x and z based on computed angle
                        double x1 = pointcloudRes[j][k].x * c + pointcloudRes[j][k].z * s;
                        double z1 = -pointcloudRes[j][k].x * s + pointcloudRes[j][k].z * c;
                        pointcloudRes[j][k].x = x1;
                        pointcloudRes[j][k].z = z1;
                    }
                }
            };
        //std::thread t(func, i, num, tempSpeed, pointcloudRes[i].front().time, pointcloudRes[i].front().y);
        std::thread t(func, i, num, tempSpeed, pointcloudRes[i].front().time, pointcloudRes[i].front().y, tempclinometerSpeed, clinometermin);//24/10/30
        threads.push_back(std::move(t));
    }

    for (auto& t : threads)
    {
        t.join();
    }

    _EndMileage = pointcloudRes.back().back().y;
    _StartMileage = pointcloudRes[0][0].y;

    return true;
}
