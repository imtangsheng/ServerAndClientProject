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

bool RealtimeSolving::writeFaroImage(TaskFaroPart& task, const QString& imagePath)
{
    static QDateTime startTime = QDateTime::currentDateTime();
    //#1-1 执行点云解析 读取里程
    if (!get_mileage_from_file(task.task_dir.toStdString(), task.mileage)) return false;
    task.direction = task.mileage[0].mileage_revision < task.mileage.back().mileage_revision;
    qDebug() << "#1 读取mileage文件" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";

    //#1-2 执行点云解析 倾角计文件和扫描仪小车时间文件 "/Inclinometer.txt" "/scannerTime.txt"
    bool hasClinoData = false;
    if (get_clinometer_from_file(task.task_dir.toStdString(), task.clinometer)) hasClinoData = true;
    if (task.clinometer.empty()) hasClinoData = false;
    qDebug() << "#1 读取倾角计文件" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
    for (auto& mVecsIt : task.mileage) {
        task.mileageWithTime.insert(std::make_pair(std::abs(mVecsIt.mileage_revision), mVecsIt.time));
    }
    for (auto& clinometerVecsIt : task.clinometer) {
        task.clinometerWithTime.insert(std::make_pair(clinometerVecsIt.time, clinometerVecsIt.x));
    }
    qDebug() << "#1 扫描仪小车时间与倾角计数据匹配" << startTime.msecsTo(QDateTime::currentDateTime()) << " ms";
    //#1-读取点云数据
    if (!ReadFaroFileData(task.faro_file.toStdString(), task.points, task.direction)) return false; 

    qDebug() << "#1 读取点云数据 " << startTime.msecsTo(QDateTime::currentDateTime()) << " ms";

    //#2-点云解析展开
    if (!PointCloudExpand(hasClinoData, task)) return false;


    qDebug() << "#2 点云解析展开 " << startTime.msecsTo(QDateTime::currentDateTime()) << " ms";

    //#2 图像生成
    QString fls_base_name = baseName(task.faro_file.toStdString()).data();
    QString grayPath = QDir(imagePath).filePath(fls_base_name + "_gray.jpg");//该格式更清晰,更小1566KB tiff为无压缩图像格式:6704KB opencv:为5786KB
    QString depthPath = QDir(imagePath).filePath(fls_base_name + "_depth.jpg");//181KB tiff:6709KB opencv:371KB
    task.resolving = 1000 / 5;
    task.diameter = 5.4;
    if(!WritePointCloudImage(task,grayPath, depthPath)) return false;
    qDebug() << "#2 图像生成 深度图和灰度图" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
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
    qDebug() << "#图像生成 深度图和灰度图 结果" << writeFaroImage(task, imagePath);
    return;
}

