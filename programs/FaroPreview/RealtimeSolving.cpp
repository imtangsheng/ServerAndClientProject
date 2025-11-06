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
    if (!get_mileage_from_file(task.mileage_file, task.mileage)) return false;
    task.direction = task.mileage[0].mileage_revision < task.mileage.back().mileage_revision;
    qDebug() << "#1 读取mileage文件" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";

    //#1-2 执行点云解析 倾角计文件和扫描仪小车时间文件 "/Inclinometer.txt" "/scannerTime.txt"
    bool hasClinoData = false;
    //if (get_clinometer_from_file(task.task_dir, task.clinometer)) hasClinoData = true;
    //if (task.clinometer.empty()) hasClinoData = false;
    //qDebug() << "#1 读取倾角计文件" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";

    for (auto& mVecsIt : task.mileage) {
        task.mileageWithTime.insert(std::make_pair(std::abs(mVecsIt.mileage_revision), mVecsIt.time));
    }
    //for (auto& clinometerVecsIt : task.clinometer) {
        //task.clinometerWithTime.insert(std::make_pair(clinometerVecsIt.time, clinometerVecsIt.x));
    //}
    //qDebug() << "#1 扫描仪小车时间与倾角计数据匹配" << startTime.msecsTo(QDateTime::currentDateTime()) << " ms";
    //#1-读取点云数据
    if (!ReadFaroFileData(task.faro_file, task.points, task.direction)) return false; 

    qDebug() << "#1 读取点云数据 " << startTime.msecsTo(QDateTime::currentDateTime()) << " ms";
    //#2-点云解析展开
    if (!PointCloudExpand(hasClinoData, task)) return false;
    qDebug() << "#2 点云解析展开 " << startTime.msecsTo(QDateTime::currentDateTime()) << " ms";
    //#2 图像生成
    QDir imgDir(imagePath);
    if (!imgDir.exists()) imgDir.mkdir(".");

    g_file_name = baseName(task.faro_file.toStdString()).data();
    QString grayPath = imgDir.filePath(g_file_name + "_gray.jpg");//该格式更清晰,更小1566KB tiff为无压缩图像格式:6704KB opencv:为5786KB
    QString depthPath = imgDir.filePath(g_file_name + "_depth.jpg");//181KB tiff:6709KB opencv:371KB
    writeFaroTime(task.faro_file_info);
    if(!WritePointCloudImage(task,grayPath, depthPath)) return false;
    qDebug() << "#2 图像生成 深度图和灰度图" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
    return true;
}

void RealtimeSolving::writeFaroTime(const QString& filePath) {
    /*保存扫描仪的文件时间,记录起始时间和结束时间*/
    QFile file(filePath);
    bool fileExists = file.exists();
    // 以追加模式打开文件，如果文件不存在会自动创建
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        // 如果文件不存在（新创建），先写入标题行
        if (!fileExists) {
            static QString header = "FileName\tTimeStart\tTimeEnd\n";
            out << header;  // 示例标题
        }
        // 写入数据
        QString data = QString("%1\t%2\t%3\n").arg(g_file_name).arg(g_time_start).arg(g_time_end);
        out << data << Qt::endl;
        // 关闭文件
        file.close();
        qDebug() << "数据写入成功";
    } else {
        qDebug() << "文件打开失败：" << file.errorString();
    }
}

