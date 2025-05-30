/**
 * @file TaskManager.h
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
#include "file_read_and_save.h"
//������Ԫ���json��������,���ڴ���˽����̶�������
// project_info.json ��һ��key
#define FRIEND_PROJECT_VERSION	"Version"
#define FRIEND_PROJECT_NAME		"ProjectName"
#define FRIEND_PROJECT_CONTENT	"ProjectContent"
// ProjectContent �ڶ���key

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

//������ʾ��Ϣ,�����
#define FRIEND_CREATE_TIME		"CreateTime"
#define FRIEND_NOTE				"Note"

static inline const QString cProjectNameSuffix = ".nfproj";//��Ŀ�ļ�����׺
static inline constexpr const char* kProjectInfoFileName = "project_info.json";////��Ŀ�ļ���Ϣjson����
static inline constexpr const char* kTaskInfoFileName = "task_info.json";//�����ļ���Ϣjson����
static inline constexpr const char* kTaskDirCarName = "Task";//�������ݴ洢�ļ���
static inline constexpr const char* kTaskDirCameraName = "Pics";//������ݴ洢�ļ���
static inline constexpr const char* kTaskDirPointCloudName = "PointCloud";//ɨ�������ݴ洢�ļ���

inline static const QString cKeyName{ "name" };
inline static const QString cKeyPath{ "path" };
inline static const QString cKeyData{ "data" };
//��������Ϣ,������Ӧlist������,������Ŀ�а���������Ϣ��json����
inline static const QString cKeyContent{ "content" };

struct FileInfoDetails
{
    QString name; //���� ��Ӧkey cKeyName
    QString path; //����·�� ��Ӧkey cKeyPath
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
};

#define gTaskManager TaskManager::instance()

//��¼��ǰ����Ŀ��ִ�е�������Ϣ
extern SHAREDLIB_EXPORT FileInfoDetails* gProjectFileInfo;//��ǰ����ִ�е���Ŀ��Ϣ(�ͻ���ʹ��)
extern SHAREDLIB_EXPORT FileInfoDetails* gTaskFileInfo;//��ǰ����ִ�е�������Ϣ
inline static Atomic<TaskStateType> gTaskState{ TaskState::TaskState_Waiting };//��¼��ǰ�豸״ֵ̬ QAtomicInteger ����


class SHAREDLIB_EXPORT TaskManager : public QObject
{
    Q_OBJECT
public:
    static TaskManager& instance() {//ʹ������,�����侲̬����,�����п�������
        static TaskManager instance;
        return instance;
    }
    QJsonObject data;//����Ŀ¼�µ�jsonִ�е�������Ϣ����,���㴫��ͽ���

protected:
    // �������캯��,ֻ�ܼ̳�ʹ��
    explicit TaskManager(QObject* parent = nullptr) : QObject(parent) {
        qDebug() << ("TaskManager - Current thread:") << QThread::currentThread();
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
    return gTaskManager.data[cKeyPath].toString() + "/" + name;
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
        if (gTaskFileInfo == nullptr) { //ȷ������������
            return Result::Failure(tr("Task is not initialized."));
        }
        QString filepath = gTaskFileInfo->path + "/" + filename;
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
        if (gTaskFileInfo == nullptr) { //ȷ������������
            return Result::Failure(tr("Task is not initialized."));
        }
        QString filepath = gTaskFileInfo->path + "/" + filename;
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
