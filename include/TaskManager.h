/**
 * @file TaskManager.h
 * @brief 工作任务处理流程信息和控制类方法集合
 *
 * 显示任务数据文件结构示例图
 *
 * @author Tang
 * @date May 13, 2025
 *
 * @dir data/ 数据总目录
 *     @dir project_name.nfproj/ 项目文件夹
 *         @file project_info.json 项目信息json数据
 *         @dir task_name/
 *             @file task_info.json 任务信息json数据
 *             @dir PoiontCloud/ 扫描仪数据存储文件夹
 *                  @file Scan001.fls 法如生成的数据
 *             @dir Task/ 任务目录文件夹
 *                  @file FaroFileInfo.txt 法如文件信息,保存时间信息
 *                      @data time	path
 *                  @file Inclinometer.txt 倾角计数据
 *                      @data X	Y	Time
 *                  @file mileage.txt 里程数据,默认双里程
 *                      @data ID	LeftMileage	LeftTime	LeftTimeRaw	RightMileage	RightTime	RightTimeRaw
 *                  @file scannerTime.txt 获取到的扫描仪和小车时间数据,对应里程中LeftTime扫描仪时间和LeftTimeRaw小车时间,即计算上面的时间是补差值法
 *                      @data ID	TrolleyTime	ScannerTime
 *                  @file singleMileage.txt 单里程数据,兼容后续数据,后续不使用
 *                      @data ID	Mileage	Time	TimeRaw
 *              @dir Pics/ 图像数据存储文件夹
 *                  @dir Cam%ID/ 相机ID 图像数据照片文件夹
 *                  @file centralPosition.txt 相机触发曝光次数机位信息数据
 *                      @data ID	CamID	CentralPosition	time	ActualPosition
 *
 */

#ifndef _WORK_H_
#define _WORK_H_
#include "file_read_and_save.h"

 //声明友元类的json数据名称,用于处理端交互固定的名称
 // project_info.json 第一级 key
#define JSON_PROJECT_VERSION	"Version" //版本名称
#define JSON_PROJECT_NAME		"ProjectName"
#define JSON_PROJECT_CONTENT	"ProjectContent"
#define JSON_LINE_NAME		"LineName" //采集线路
#define JSON_CREATOR		"Creator" //创建人
#define JSON_NOTE		"Note" //备注
// ProjectContent 第二级 key
#define JSON_TASK_NAME			"JobName" //默认为系统当前时间
#define JSON_TASK_CONTENT	"MissionContent" //数据参数主体 json 数据
//云图 参数主体 json 数据
#define JSON_DEVICE_TYPE		"DeviceModel" //设备类型,自动获取
#define JSON_SPEED	"Speed" //速度,m/h
#define JSON_DIRECTION    "TrolleyDirection" //方向,Forward/Backward 0/1

inline QString GetCarDirection(bool direction){
   return direction ? "Forward" : "Backward"; //0后退 1 前进
}

#define JSON_DIAMETER	"Diameter" //隧道直径,椭圆拟合时需要
#define JSON_SCAN_SN  "ScanSN" //扫描仪序列号 可通过 api 接口/swebapi/scanner-infos获取
#define JSON_SCAN_HEIGHT "ScanHeight" //扫描仪高度,单位mm
#define JSON_ACCURACY		"Accuracy" //点云精度,根据速度和参数公式计数
//TMP平台组显示参数
#define JSON_TMP_LINE_NAME			"LineName" //线路,TMP平台组参数
#define JSON_TMP_LINE_TYPE			"LineType" //线别,上/下行,左/右线,TMP平台组参数
inline QString GetLineType(int index) {
    QString line_type;
    switch (index) {
    case 0:line_type = "Left"; break;
    case 1:line_type = "Right"; break;
    case 2:line_type = "Up"; break;
    case 3:line_type = "Down"; break;
    default:line_type = "Left"; break;
    }
    return line_type;
}


#define JSON_TMP_BETWEEN_NAME		"BetweenName" //区间,TMP平台组参数

//其他显示信息,非相关
#ifdef __MS201__
#define JSON_OVERLAP_RATE "OverlapRate" //重叠率 MS201
#define JSON_CAMERA_POSITION	"CameraPostion" //拍照的时候机位 MS201
#define JSON_EXPOSE_TIME "ExposeTime" //相机的曝光时间 MS201
#endif // DeviceIsMS201__
//参考信息
#define JSON_SEGMENT_WIDE		"SegmentWidth" //管片宽度,用于自动分环
#define JSON_START_RING  "StartRing" //起始环号,预处理对齐
#define JSON_START_MILEAGE "StartMileage" //起始里程,预处理对齐

#define JSON_CREATE_TIME		"CreateTime" //创建时间
#define JSON_TEMPLATE		"ParameterTemplate" //模板信息,记录选择的模板信息

#define JSON_CAR_RATED_MILEAGE "CarRatedMileage" //小车额定里程标定值,大于此则自动结束任务
//其他信息参考
#define Json_TunnelType "TunnelType" //隧道类型
#define Json_CameraTemplate "CameraTemplate"
//扫描仪设备参数
#define Json_MeasurementRate "MeasurementRate" //测量速率
#define Json_SplitAfterLines "SplitAfterLines" //分块线数
#define Json_Resolution "Resolution" //分辨率
#define Json_Quality "Quality" //测量质量,扫描仪用户界面上参数
#define Json_NumCols "NumCols" //列数 扫描线数,到达此数会停止扫描

static inline const QString cProjectNameSuffix = ".nfproj";//项目文件名后缀
static inline constexpr const char* kProjectInfoFileName = "project_info.json";////项目文件信息 json数据
static inline constexpr const char* kTaskInfoFileName = "task_info.json";//任务文件信息 json数据
static inline constexpr const char* kTaskDirCarName = "Task";//任务数据存储文件夹
static inline constexpr const char* kTaskDirCameraName = "Pics";//相机数据存储文件夹
static inline constexpr const char* kTaskDirPointCloudName = "PointCloud";//扫描仪数据存储文件夹

inline static const QString cKeyName{ "name" };
inline static const QString cKeyPath{ "path" };
inline static const QString cKeyData{ "data" };
//上下文信息,包含对应 list的数据,列如项目中包含任务信息的json数据
inline static const QString cKeyContent{ "content" };

constexpr const char* kTimeFormat = "yyyy-MM-dd HH:mm:ss";//任务的时间格式
inline QString GetCurrentDateTime(){
    return QDateTime::currentDateTime().toString(kTimeFormat);
}

struct FileInfoDetails
{
    QString name; //名称 对应 key cKeyName
    QString path; //绝对路径 对应 key cKeyPath
    QJsonObject data;

    QJsonObject ToJsonObject() const {
        QJsonObject obj;
        obj[cKeyName] = name;
        obj[cKeyPath] = path;
        obj[cKeyData] = data;
        return obj;
    }
    bool FromJsonObject(const QJsonObject& obj) {
        name = obj[cKeyName].toString();
        path = obj[cKeyPath].toString();
        data = obj[cKeyData].toObject();
        return true;
    }

    FileInfoDetails() = default;
    FileInfoDetails(QJsonObject obj) {
        name = obj[cKeyName].toString();
        path = obj[cKeyPath].toString();
        data = obj[cKeyData].toObject();
    }
    QDateTime getTime() const {//用于时间排序等功能
        return QDateTime::fromString(data.value(JSON_CREATE_TIME).toString(), kTimeFormat);
    }

};

#define gTaskManager TaskManager::instance()

//记录当前的项目和执行的任务信息
extern SHAREDLIB_EXPORT  QSharedPointer<FileInfoDetails> gProjectFileInfo;//当前正在执行的项目信息(客户端使用)
extern SHAREDLIB_EXPORT FileInfoDetails* gTaskFileInfo;//当前正在执行的任务信息
inline static Atomic<TaskStateType> gTaskState{ TaskState::TaskState_Waiting };//记录当前设备状态值 QAtomicInteger 类型


class SHAREDLIB_EXPORT TaskManager : public QObject
{
    Q_OBJECT
public:
    static TaskManager& instance() {//使用引用,返回其静态变量,不进行拷贝数据
        static TaskManager instance;
        return instance;
    }
    QJsonObject data;//工作目录下的json执行的任务信息数据,方便传输和交换

protected:
    // 保护构造函数,只能继承使用
    explicit TaskManager(QObject* parent = nullptr) : QObject(parent) {
        // qDebug() << ("TaskManager - Current thread:") << QThread::currentThread();
    }
    ~TaskManager();
signals:
    void started();
    void finished();
};

inline static QString GetProjectName(const QString& name) {
    return name + cProjectNameSuffix;
}

inline static QString GetProjectPath(const QString& name) {
    return gTaskManager.data.value(cKeyPath).toString("../data") + "/" + name;
}
inline static bool GetProjectName(const QString& path, QString& name) {
    name = QFileInfo(path).baseName();
    int suffixIndex = name.lastIndexOf(cProjectNameSuffix);
    if (suffixIndex != -1) {
        name = name.left(suffixIndex);
        return true;
    }
    return false;
}


class SHAREDLIB_EXPORT SavaDataFile : public QObject
{
    Q_OBJECT
public:
    SavaDataFile(const QString& filename, const QString& firstline) :filename(filename), firstline(firstline), file(nullptr), QObject(&gTaskManager) {
        connect(&gTaskManager, &TaskManager::started, this, &SavaDataFile::initialize);
        connect(&gTaskManager, &TaskManager::finished, this, &SavaDataFile::close);
    }
    ~SavaDataFile() {
        close();
        qDebug() << "Destroying SavaDataFile for:" << filename;
    }
    bool isInitialized{ false };
    Result initialize() {
        if (isInitialized) return true;
        if (gTaskFileInfo == nullptr) { //确保任务对象存在
            return Result::Failure(tr("Task is not initialized."));
        }
        QString filepath = gTaskFileInfo->path + "/" + filename;
        // 确保目录存在
        QFileInfo fileInfo(filepath);
        QDir dir = fileInfo.dir();
        if (!dir.exists() && !dir.mkpath(".")) {//确保在写入文件之前,相应的目录结构已经存在
            return Result::Failure(tr("Failed to create directory: %1").arg(dir.absolutePath()));
        }
        // 打开文件
        file = new QFile(filepath);//缓冲区大小由 Qt 的底层实现和操作系统决定（通常为 4KB 或 16KB，具体取决于平台）
        if (!file->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {//使用 QIODevice::Unbuffered 标志打开文件，禁用 Qt 的内部缓冲，直接写入磁盘
            delete file; file = nullptr;
            return Result::Failure(tr("Failed to open file: %1").arg(filepath));
        }
        stream = new QTextStream(file);
        WriteLine(firstline);//写入第一行数据
        isInitialized = true;
        return true;

    }

    void WriteLine(const QString& line) {//Write a line of data
        *stream << line;
        //stream->flush(); //立即刷新缓冲区,写入数据到文件
    }
    void WriteLineAndFlush(const QString& line) {//Write a line of data
        *stream << line;
        stream->flush(); //立即刷新缓冲区,写入数据到文件
    }
    void close() {
        if (stream) {
            stream->device()->close();//立即将缓冲区数据写入文件并关闭文件句柄
            delete stream; stream = nullptr;
        }
        if (file) {
            file->close();
            delete file; file = nullptr;
        }
        isInitialized = false;
    }
private:
    QString filename, firstline;
    QFile* file;
    QTextStream* stream{ nullptr };
};

class SHAREDLIB_EXPORT SavaRawData : public QObject
{
    Q_OBJECT
public:
    explicit SavaRawData(const QString& filename) :filename(filename), file(nullptr), QObject(&gTaskManager) {
        connect(&gTaskManager, &TaskManager::started, this, &SavaRawData::initialize);
        connect(&gTaskManager, &TaskManager::finished, this, &SavaRawData::close);
    }
    ~SavaRawData() {
        close();
        qDebug() << "Destroying SavaDataFile for:" << filename;
    }
    bool isInitialized{ false };
    Result initialize() {
        if (isInitialized) return true;
        if (gTaskFileInfo == nullptr) { //确保任务对象存在
            return Result::Failure(tr("Task is not initialized."));
        }
        QString filepath = gTaskFileInfo->path + "/" + filename;
        // 确保目录存在
        QFileInfo fileInfo(filepath);
        QDir dir = fileInfo.dir();
        if (!dir.exists() && !dir.mkpath(".")) {//确保在写入文件之前,相应的目录结构已经存在
            return Result::Failure(tr("Failed to create directory: %1").arg(dir.absolutePath()));
        }
        // 打开文件
        file = new QFile(filepath);//缓冲区大小由 Qt 的底层实现和操作系统决定（通常为 4KB 或 16KB，具体取决于平台）
        if (!file->open(QIODevice::WriteOnly | QIODevice::Append)) {//使用 QIODevice::Unbuffered 标志打开文件，禁用 Qt 的内部缓冲，直接写入磁盘
            delete file; file = nullptr;
            return Result::Failure(tr("Failed to open file: %1").arg(filepath));
        }
        isInitialized = true;
        return true;

    }

    void Write(const char* data, int size) {//Write a line of data
        file->write(data, size);
    }
    void WriteAndFlush(const char* data, int size) {//Write a line of data
        file->write(data, size);
        file->flush();
    }
    void close() {
        if (file) {
            file->close();
            delete file; file = nullptr;
        }
        isInitialized = false;
    }
private:
    QString filename;
    QFile* file;
};

#endif // _WORK_H_
