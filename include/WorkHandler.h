/**
 * @file WorkHandler.h
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

static inline constexpr std::string_view kProjectNameSuffix = ".nfproj";//项目文件名后缀
static inline constexpr const char* kProjectInfoFileName = "project_info.json";////项目文件信息json数据
static inline constexpr const char* kTaskInfoFileName = "task_info.json";//任务文件信息json数据
static inline constexpr const char* kTaskDirCarName = "Task";//任务数据存储文件夹
static inline constexpr const char* kTaskDirCameraName = "Pics";//相机数据存储文件夹
static inline constexpr const char* kTaskDirPointCloudName = "PointCloud";//扫描仪数据存储文件夹

#include <QSet>

struct TaskInfo //The current task tunnel
{
    QString path;
    QJsonObject obj;//任务信息记录json数据
    TaskInfo(const QString& absilutePath = QString()) :path(absilutePath) {}
    // 任务信息记录json数据
    // 添加比较运算符
    bool operator<(const TaskInfo& other) const {
        return path < other.path;
    }
    // 添加相等运算符
    bool operator==(const TaskInfo& other) const {
        return path == other.path;
    }
};
inline static size_t qHash(const TaskInfo& info, size_t seed = 0) noexcept {
    return qHash(info.path, seed);
}

struct ProjectInfo
{
    QString path;
    QJsonObject obj;//项目信息记录json
    QSet <TaskInfo> tasks;//使用QSet存储任务信息,避免重复添加相同的任务
    ProjectInfo(const QString& absilutePath = QString()) :path(absilutePath) {}

    Result AddTask(const TaskInfo& task) {
        if (tasks.contains(task)) {
            return Result::Failure(QObject::tr("#AddTask:Task already exists:%1").arg(task.path));
        }
        tasks.insert(task);
        //写入json配置到文件中
        if (!WriteJsonFile(task.path + "/" + kTaskInfoFileName, task.obj)) {
            return Result::Failure(QObject::tr("#AddTask:Task file write failed:%1").arg(task.path));
        }
        return Result::Success();
    }
    Result DeleteTask(const TaskInfo& task) {
        if (!tasks.contains(task)) {
            return Result::Failure(QObject::tr("#DeleteTask:Task does not exist:%1").arg(task.path));
        }
        tasks.remove(task);
        //删除任务文件夹
        QDir dir(task.path);
        if (dir.exists()) {
            if (!dir.removeRecursively()) {//永久删除,如果无法删除文件或目录，则会继续运行并尝试删除尽可能多的文件和子目录，然后返回 false
                return Result::Failure(QObject::tr("#DeleteTask:Task folder removed failed:%1").arg(task.path));
            }
        }
        return Result::Success();
    }
    // 添加比较运算符
    bool operator<(const ProjectInfo& other) const {
        return path < other.path;
    }
    // 添加相等运算符
    bool operator==(const ProjectInfo& other) const {
        return path == other.path;
    }
};

inline static size_t qHash(const ProjectInfo& info, size_t seed = 0) noexcept {
    return qHash(info.path, seed);
}

//记录当前的项目和执行的任务信息
extern SHAREDLIB_EXPORT ProjectInfo* gProject;//当前正在执行的项目信息
extern SHAREDLIB_EXPORT TaskInfo* gTask;//当前正在执行的任务信息
inline static Atomic<quint8> gTaskState{ TaskState::TaskState_Waiting };//记录当前设备状态值 QAtomicInteger 类型

#define gWorkHandler WorkHandler::instance()
class SHAREDLIB_EXPORT WorkHandler : public QObject
{
    Q_OBJECT
public:
    static WorkHandler& instance() {//使用引用,返回其静态变量,不进行拷贝数据
        static WorkHandler instance;
        return instance;
    }

    QSet<ProjectInfo> projects;

    Result initialize(const QString& path) {
        QDir dir(path);
        if (!dir.exists() && !dir.mkpath(".")) {// QDIR.mkpath(".") 时,它会尝试在当前目录下创建一个新的目录。如果目录已经存在,它不会做任何操作
            return Result::Failure(tr("#WorkHandler:Directory creation failed:%1").arg(path));
        }
        //获得只返回当前目录下的所有子目录(不包括 . 和 ..) 的效果
        QStringList projectNameList = dir.entryList(QStringList() << QString("*%1").arg(kProjectNameSuffix), QDir::Dirs | QDir::NoDotAndDotDot);
        // Read project data
        projects.clear();
        for (auto& projectDir : projectNameList) {
            //添加项目
            QString absiluteProjectPath = dir.absoluteFilePath(projectDir);//获取项目绝对路径
            QString projectFilePath = absiluteProjectPath + "/" + kProjectInfoFileName;
            ProjectInfo project(absiluteProjectPath);//路径名称唯一值
            if (!GetJsonValue(projectFilePath, project.obj)) {
                continue;//读取或者解析json失败
            }
            projects.insert(project);
            //项目任务处理
            QDir dirTask(absiluteProjectPath);
            QStringList taskNameList = dirTask.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            for (auto& taskDir : taskNameList) {
                //添加任务
                QString absiluteTaskPath = dirTask.absoluteFilePath(taskDir);//获取任务绝对路径
                QString taskFilePath = absiluteTaskPath + "/" + kTaskInfoFileName;
                TaskInfo task(absiluteTaskPath);
                if (!GetJsonValue(taskFilePath, task.obj)) {
                    continue;
                }
                //添加任务
                project.AddTask(task);
            }
        }
        return true;
    };

    Result AddProject(const ProjectInfo& project) {
        if (projects.contains(project)) {
            return Result::Failure(tr("AddProject is Failed,The project added already exists."));
        }
        projects.insert(project);
        //写入json配置到文件中
        if (!WriteJsonFile(project.path + "/" + kProjectInfoFileName, project.obj)) {
            return Result::Failure(tr("AddProject is Failed,The project file write failed."));
        }
        return true;
    }

    Result DeleteProject(const ProjectInfo& project) {
        if (!projects.contains(project)) {
            return Result::Failure(tr("DeleteProject is Failed, The project removed does not exist."));
        }
        projects.remove(project);
        //删除项目文件夹
        QDir dir(project.path);
        if (dir.exists()) {
            if (!dir.removeRecursively()) {//永久删除,如果无法删除文件或目录，则会继续运行并尝试删除尽可能多的文件和子目录，然后返回 false
                return Result::Failure(tr("DeleteProject is Failed, The project folder removed failed."));
            }
        }
        return true;
    }

protected:
    // 保护构造函数,只能继承使用
    explicit WorkHandler(QObject* parent = nullptr) : QObject(parent) {
        qDebug() << ("WorkHandler - Current thread:") << QThread::currentThread();
    }
    ~WorkHandler();
signals:
    void started();
    void completed();
};

class SHAREDLIB_EXPORT SavaDataFile : public QObject
{
    Q_OBJECT
public:
    SavaDataFile(const QString& filename, const QString& firstline) :filename(filename), firstline(firstline), file(nullptr), QObject(&gWorkHandler) {
        connect(&gWorkHandler, &WorkHandler::started, this, &SavaDataFile::initialize);
        connect(&gWorkHandler, &WorkHandler::completed, this, &SavaDataFile::close);
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
    explicit SavaRawData(const QString& filename) :filename(filename),file(nullptr), QObject(&gWorkHandler) {
        connect(&gWorkHandler, &WorkHandler::started, this, &SavaRawData::initialize);
        connect(&gWorkHandler, &WorkHandler::completed, this, &SavaRawData::close);
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
