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
#define FRIEND_PROJECT_NAME		"ProjectName"
#define FRIEND_PROJECT_CONTENT	"ProjectContent"

#define FRIEND_MISSION_CONTENT	"MissionContent"
#define FRIEND_BETWEEN_NAME		"BetweenName"
#define FRIEND_DEVICE_TYPE		"DeviceModel"
#define FRIEND_SEGMENT_WIDE		"SegmentWidth"
#define FRIEND_JOB_NAME			"JobName"
#define FRIEND_LINE_NAME			"LineName"
#define FRIEND_LINE_TYPE			"LineType"
#define FRIEND_AUTHOR				"Author"
#define FRIEND_DIRECTION			"TrolleyDirection"
#define FRIEND_CAMERA_POSITION	"CameraPostion"
#define FRIEND_IMAGE_ACCURACY		"Accuracy"

static inline const QString kProjectNameSuffix = ".nfproj";//项目文件名后缀
static inline constexpr const char* kProjectInfoFileName = "project_info.json";////项目文件信息json数据
static inline constexpr const char* kTaskInfoFileName = "task_info.json";//任务文件信息json数据
static inline constexpr const char* kTaskDirCarName = "Task";//任务数据存储文件夹
static inline constexpr const char* kTaskDirCameraName = "Pics";//相机数据存储文件夹
static inline constexpr const char* kTaskDirPointCloudName = "PointCloud";//扫描仪数据存储文件夹

inline static const QString cKeyName{ "name" };
inline static const QString cKeyPath{ "path" };
inline static const QString cKeyData{ "data" };
//上下文信息,包含对应list的数据,列如项目中包含任务信息的json数据
inline static const QString cKeyContent{ "content" };

struct TaskInfo
{
    QString name; //名称 对应key cKeyName
    QString path; //绝对路径 对应key cKeyPath
    QJsonObject data;
};
//记录当前的项目和执行的任务信息
extern SHAREDLIB_EXPORT TaskInfo* gTask;//当前正在执行的任务信息
inline static Atomic<TaskStateType> gTaskState{ TaskState::TaskState_Waiting };//记录当前设备状态值 QAtomicInteger 类型

#define gTaskManager TaskManager::instance()
class SHAREDLIB_EXPORT TaskManager : public QObject
{
    Q_OBJECT
public:
    static TaskManager& instance() {//使用引用,返回其静态变量,不进行拷贝数据
        static TaskManager instance;
        return instance;
    }
    QJsonObject data;//工作目录下的json执行的任务信息数据,方便传输和交换

    Result AddProject(const QJsonObject& project) {
        QJsonObject projects = data[cKeyContent].toObject();
        QString name = project.value(cKeyName).toString();
        if (name.isEmpty() || projects.contains(name)) {
            return Result::Failure(tr("AddProject is Failed,The project name is empty or the project to be added already exists."));
        }
        //写入json配置到文件中
        if (!WriteJsonFile(project.value(cKeyPath).toString() + "/" + kProjectInfoFileName, project.value(cKeyData).toObject())) {
            return Result::Failure(tr("AddProject is Failed,The project file write failed."));
        }
        projects[name] = project;
        data[cKeyContent] = projects;
        return true;
    }

    Result DeleteProject(const QJsonObject& project) {
        QJsonObject projects = data[cKeyContent].toObject();
        QString name = project.value(cKeyName).toString();
        if (!projects.contains(name)) {
            return Result::Failure(tr("DeleteProject is Failed, The project removed does not exist."));
        }
        //删除项目文件夹
        QDir dir(project.value(cKeyPath).toString());
        if (dir.exists()) {
            if (!dir.removeRecursively()) {//永久删除,如果无法删除文件或目录，则会继续运行并尝试删除尽可能多的文件和子目录，然后返回 false
                return Result::Failure(tr("DeleteProject is Failed, The project folder removed failed."));
            }
        }
        projects.remove(name);
        data[cKeyContent] = projects;
        return true;
    }

    Result AddTask(const QJsonObject& task) {
        //获取任务目录的上一级目录名称就是项目名称
        QDir dir(task.value(cKeyPath).toString());
        if (!dir.cdUp()) {
            return Result::Failure(tr("AddTask is Failed, No parent directory for %1.").arg(dir.absolutePath()));
        }
        QString projectName = dir.dirName();
        QJsonObject projects = data[cKeyContent].toObject();
        if(projectName.isEmpty() || !projects.contains(projectName)){
            return Result::Failure(tr("AddTask is Failed, The project does not exist: %1.").arg(projectName));
        }
        QJsonObject project = projects.value(projectName).toObject();
        QString name = task.value(cKeyName).toString();
        QJsonObject tasks = project.value(cKeyContent).toObject();
        if (name.isEmpty() || tasks.contains(name)) {
            return Result::Failure(tr("AddTask is Failed, The task name is empty or the task to be added already exists: %1.").arg(name));
        }

        //写入json配置到文件中
        if (!WriteJsonFile(task.value(cKeyPath).toString() + "/" + kTaskInfoFileName, task.value(cKeyData).toObject())) {
            return Result::Failure(tr("AddTask is Failed, The task file write failed."));
        }
        tasks[name] = task; //更新任务
        project[cKeyContent] = tasks;//保存任务
        projects[projectName] = project;//更新项目
        data[cKeyContent] = projects;//保存项目
        return true;
    }
    Result DeleteTask(const QJsonObject& task) {
        //获取任务目录的上一级目录名称就是项目名称
        QDir dir(task.value(cKeyPath).toString());
        if (!dir.cdUp()) {
            return Result::Failure(tr("DeleteTask is Failed, No parent directory for %1.").arg(dir.absolutePath()));
        }
        QString projectName = dir.dirName();
        QJsonObject projects = data[cKeyContent].toObject();
        if(projectName.isEmpty() || !projects.contains(projectName)){
            return Result::Failure(tr("DeleteTask is Failed, The project does not exist: %1.").arg(projectName));
        }
        QJsonObject project = projects.value(projectName).toObject();
        QJsonObject tasks = project.value(cKeyContent).toObject();
        QString name = task.value(cKeyName).toString();
        if (!tasks.contains(name)) {
            return Result::Failure(tr("DeleteTask is Failed, The task removed does not exist."));
        }
        //删除任务文件夹
        QDir dirTask(task.value(cKeyPath).toString());
        if (dirTask.exists()) {
            if (!dir.removeRecursively()) {
                return Result::Failure(tr("DeleteTask is Failed, The task folder removed failed."));
            }
        }
        tasks.remove(name);
        project[cKeyContent] = tasks;
        projects[projectName] = project;
        data[cKeyContent] = projects;
        return true;
    }
protected:
    // 保护构造函数,只能继承使用
    explicit TaskManager(QObject* parent = nullptr) : QObject(parent) {
        qDebug() << ("TaskManager - Current thread:") << QThread::currentThread();
    }
    ~TaskManager();
signals:
    void started();
    void finished();
};

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
        if (gTask == nullptr) { //确保任务对象存在
            return Result::Failure(tr("Task is not initialized."));
        }
        QString filepath = gTask->path + "/" + filename;
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
    explicit SavaRawData(const QString& filename) :filename(filename),file(nullptr), QObject(&gTaskManager) {
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
        if (gTask == nullptr) { //确保任务对象存在
            return Result::Failure(tr("Task is not initialized."));
        }
        QString filepath = gTask->path + "/" + filename;
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
