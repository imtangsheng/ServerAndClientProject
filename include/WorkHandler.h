/**
 * @file WorkHandler.h
 * @brief ����������������Ϣ�Ϳ����෽������
 *
 * ��ʾ���������ļ��ṹʾ��ͼ
 *
 * @author Tang
 * @date May 13, 2025
 *
 * @dir data/ ������Ŀ¼
 *     @dir project_name.nfproj/ ��Ŀ�ļ���
 *         @file project_info.json ��Ŀ��Ϣjson����
 *         @dir task_name/
 *             @file task_info.json ������Ϣjson����
 *             @dir PoiontCloud/ ɨ�������ݴ洢�ļ���
 *                  @file Scan001.fls �������ɵ�����
 *             @dir Task/ ����Ŀ¼�ļ���
 *                  @file FaroFileInfo.txt �����ļ���Ϣ,����ʱ����Ϣ
 *                      @data time	path
 *                  @file Inclinometer.txt ��Ǽ�����
 *                      @data X	Y	Time
 *                  @file mileage.txt �������,Ĭ��˫���
 *                      @data ID	LeftMileage	LeftTime	LeftTimeRaw	RightMileage	RightTime	RightTimeRaw
 *                  @file scannerTime.txt ��ȡ����ɨ���Ǻ�С��ʱ������,��Ӧ�����LeftTimeɨ����ʱ���LeftTimeRawС��ʱ��,�����������ʱ���ǲ���ֵ��
 *                      @data ID	TrolleyTime	ScannerTime
 *                  @file singleMileage.txt ���������,���ݺ�������,������ʹ��
 *                      @data ID	Mileage	Time	TimeRaw
 *              @dir Pics/ ͼ�����ݴ洢�ļ���
 *                  @dir Cam%ID/ ���ID ͼ��������Ƭ�ļ���
 *                  @file centralPosition.txt ��������ع������λ��Ϣ����
 *                      @data ID	CamID	CentralPosition	time	ActualPosition
 * 
 */

#ifndef _WORK_H_
#define _WORK_H_

//������Ԫ���json��������,���ڴ���˽����̶�������
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

static inline constexpr std::string_view kProjectNameSuffix = ".nfproj";//��Ŀ�ļ�����׺
static inline constexpr const char* kProjectInfoFileName = "project_info.json";////��Ŀ�ļ���Ϣjson����
static inline constexpr const char* kTaskInfoFileName = "task_info.json";//�����ļ���Ϣjson����
static inline constexpr const char* kTaskDirCarName = "Task";//�������ݴ洢�ļ���
static inline constexpr const char* kTaskDirCameraName = "Pics";//������ݴ洢�ļ���
static inline constexpr const char* kTaskDirPointCloudName = "PointCloud";//ɨ�������ݴ洢�ļ���

#include <QSet>

struct TaskInfo //The current task tunnel
{
    QString path;
    QJsonObject obj;//������Ϣ��¼json����
    TaskInfo(const QString& absilutePath = QString()) :path(absilutePath) {}
    // ������Ϣ��¼json����
    // ��ӱȽ������
    bool operator<(const TaskInfo& other) const {
        return path < other.path;
    }
    // �����������
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
    QJsonObject obj;//��Ŀ��Ϣ��¼json
    QSet <TaskInfo> tasks;//ʹ��QSet�洢������Ϣ,�����ظ������ͬ������
    ProjectInfo(const QString& absilutePath = QString()) :path(absilutePath) {}

    Result AddTask(const TaskInfo& task) {
        if (tasks.contains(task)) {
            return Result::Failure(QObject::tr("#AddTask:Task already exists:%1").arg(task.path));
        }
        tasks.insert(task);
        //д��json���õ��ļ���
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
        //ɾ�������ļ���
        QDir dir(task.path);
        if (dir.exists()) {
            if (!dir.removeRecursively()) {//����ɾ��,����޷�ɾ���ļ���Ŀ¼�����������в�����ɾ�������ܶ���ļ�����Ŀ¼��Ȼ�󷵻� false
                return Result::Failure(QObject::tr("#DeleteTask:Task folder removed failed:%1").arg(task.path));
            }
        }
        return Result::Success();
    }
    // ��ӱȽ������
    bool operator<(const ProjectInfo& other) const {
        return path < other.path;
    }
    // �����������
    bool operator==(const ProjectInfo& other) const {
        return path == other.path;
    }
};

inline static size_t qHash(const ProjectInfo& info, size_t seed = 0) noexcept {
    return qHash(info.path, seed);
}

//��¼��ǰ����Ŀ��ִ�е�������Ϣ
extern SHAREDLIB_EXPORT ProjectInfo* gProject;//��ǰ����ִ�е���Ŀ��Ϣ
extern SHAREDLIB_EXPORT TaskInfo* gTask;//��ǰ����ִ�е�������Ϣ
inline static Atomic<quint8> gTaskState{ TaskState::TaskState_Waiting };//��¼��ǰ�豸״ֵ̬ QAtomicInteger ����

#define gWorkHandler WorkHandler::instance()
class SHAREDLIB_EXPORT WorkHandler : public QObject
{
    Q_OBJECT
public:
    static WorkHandler& instance() {//ʹ������,�����侲̬����,�����п�������
        static WorkHandler instance;
        return instance;
    }

    QSet<ProjectInfo> projects;

    Result initialize(const QString& path) {
        QDir dir(path);
        if (!dir.exists() && !dir.mkpath(".")) {// QDIR.mkpath(".") ʱ,���᳢���ڵ�ǰĿ¼�´���һ���µ�Ŀ¼�����Ŀ¼�Ѿ�����,���������κβ���
            return Result::Failure(tr("#WorkHandler:Directory creation failed:%1").arg(path));
        }
        //���ֻ���ص�ǰĿ¼�µ�������Ŀ¼(������ . �� ..) ��Ч��
        QStringList projectNameList = dir.entryList(QStringList() << QString("*%1").arg(kProjectNameSuffix), QDir::Dirs | QDir::NoDotAndDotDot);
        // Read project data
        projects.clear();
        for (auto& projectDir : projectNameList) {
            //�����Ŀ
            QString absiluteProjectPath = dir.absoluteFilePath(projectDir);//��ȡ��Ŀ����·��
            QString projectFilePath = absiluteProjectPath + "/" + kProjectInfoFileName;
            ProjectInfo project(absiluteProjectPath);//·������Ψһֵ
            if (!GetJsonValue(projectFilePath, project.obj)) {
                continue;//��ȡ���߽���jsonʧ��
            }
            projects.insert(project);
            //��Ŀ������
            QDir dirTask(absiluteProjectPath);
            QStringList taskNameList = dirTask.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            for (auto& taskDir : taskNameList) {
                //�������
                QString absiluteTaskPath = dirTask.absoluteFilePath(taskDir);//��ȡ�������·��
                QString taskFilePath = absiluteTaskPath + "/" + kTaskInfoFileName;
                TaskInfo task(absiluteTaskPath);
                if (!GetJsonValue(taskFilePath, task.obj)) {
                    continue;
                }
                //�������
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
        //д��json���õ��ļ���
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
        //ɾ����Ŀ�ļ���
        QDir dir(project.path);
        if (dir.exists()) {
            if (!dir.removeRecursively()) {//����ɾ��,����޷�ɾ���ļ���Ŀ¼�����������в�����ɾ�������ܶ���ļ�����Ŀ¼��Ȼ�󷵻� false
                return Result::Failure(tr("DeleteProject is Failed, The project folder removed failed."));
            }
        }
        return true;
    }

protected:
    // �������캯��,ֻ�ܼ̳�ʹ��
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
        if (gTask == nullptr) { //ȷ������������
            return Result::Failure(tr("Task is not initialized."));
        }
        QString filepath = gTask->path + "/" + filename;
        // ȷ��Ŀ¼����
        QFileInfo fileInfo(filepath);
        QDir dir = fileInfo.dir();
        if (!dir.exists() && !dir.mkpath(".")) {//ȷ����д���ļ�֮ǰ,��Ӧ��Ŀ¼�ṹ�Ѿ�����
            return Result::Failure(tr("Failed to create directory: %1").arg(dir.absolutePath()));
        }
        // ���ļ�
        file = new QFile(filepath);//��������С�� Qt �ĵײ�ʵ�ֺͲ���ϵͳ������ͨ��Ϊ 4KB �� 16KB������ȡ����ƽ̨��
        if (!file->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {//ʹ�� QIODevice::Unbuffered ��־���ļ������� Qt ���ڲ����壬ֱ��д�����
            delete file; file = nullptr;
            return Result::Failure(tr("Failed to open file: %1").arg(filepath));
        }
        stream = new QTextStream(file);
        WriteLine(firstline);//д���һ������
        isInitialized = true;
        return true;

    }

    void WriteLine(const QString& line) {//Write a line of data
        *stream << line;
        //stream->flush(); //����ˢ�»�����,д�����ݵ��ļ�
    }
    void WriteLineAndFlush(const QString& line) {//Write a line of data
        *stream << line;
        stream->flush(); //����ˢ�»�����,д�����ݵ��ļ�
    }
    void close() {
        if (stream) {
            stream->device()->close();//����������������д���ļ����ر��ļ����
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
        if (gTask == nullptr) { //ȷ������������
            return Result::Failure(tr("Task is not initialized."));
        }
        QString filepath = gTask->path + "/" + filename;
        // ȷ��Ŀ¼����
        QFileInfo fileInfo(filepath);
        QDir dir = fileInfo.dir();
        if (!dir.exists() && !dir.mkpath(".")) {//ȷ����д���ļ�֮ǰ,��Ӧ��Ŀ¼�ṹ�Ѿ�����
            return Result::Failure(tr("Failed to create directory: %1").arg(dir.absolutePath()));
        }
        // ���ļ�
        file = new QFile(filepath);//��������С�� Qt �ĵײ�ʵ�ֺͲ���ϵͳ������ͨ��Ϊ 4KB �� 16KB������ȡ����ƽ̨��
        if (!file->open(QIODevice::WriteOnly | QIODevice::Append)) {//ʹ�� QIODevice::Unbuffered ��־���ļ������� Qt ���ڲ����壬ֱ��д�����
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
