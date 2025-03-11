#pragma once
#include "global.h"
#include "DVPCamera.h"

//#pragma comment(lib, "x64/DVPCamera64.lib") //该文件放到生成目录下 或启动cmake的链接
//#ifdef _WIN64 //加载64-bit lib库 
//#pragma comment(lib, "lib_x86_64/DVPCamera64.lib") 
//#else //加载32-bit lib库 
//#pragma comment(lib, "lib_x86_32/DVPCamera.lib") 
//#endif 

//是否有效句柄
inline bool IsValidHandle(dvpHandle handle)
{
	bool bValidHandle;
	dvpIsValid(handle, &bValidHandle);
	return bValidHandle;
}

// 使用函数映射表来处理不同类型的参数
using SetParamFunc = std::function<dvpStatus(dvpHandle, dvpStr, const QJsonValue&)>;
const std::map<QString, SetParamFunc> ParamSetters = {
	{"enum", [](dvpHandle handle, dvpStr paramName, const QJsonValue& value) {
		return dvpSetEnumValue(handle, paramName, value.toInt());
	}},
	{"float", [](dvpHandle handle, dvpStr paramName, const QJsonValue& value) {
		return dvpSetFloatValue(handle, paramName, value.toDouble());
	}},
	{"int32", [](dvpHandle handle, dvpStr paramName, const QJsonValue& value) {
		return dvpSetInt32Value(handle, paramName, value.toInt());
	}},
	{"bool", [](dvpHandle handle, dvpStr paramName, const QJsonValue& value) {
		return dvpSetBoolValue(handle, paramName, value.toBool());
	}},
	{"string", [](dvpHandle handle, dvpStr paramName, const QJsonValue& value) {
		QByteArray ba = value.toString().toLocal8Bit();
		return dvpSetStringValue(handle, paramName, ba.data());
	}}
};

// 相机信息结构体
struct struCameraInfo {
	dvpCameraInfo info{};
	dvpHandle    handle{};//相机句柄
	dvpStreamState state{};//视频流状态
	QString name;
	QString path;
	QString id;
	static QString format;
	//QMutex mutex;
	struCameraInfo() = default;
	void update(QJsonObject json) {
		if (json.contains("name")) name = json["name"].toString();
		if (json.contains("path")) path = json["path"].toString();
		if (json.contains("id")) id = json["id"].toString();
	}
	QJsonObject ToJson() const {
		QJsonObject json;
		if (!name.isEmpty()) json["name"] = name;
		if (!path.isEmpty()) json["path"] = path;
		if (!id.isEmpty()) json["id"] = id;
		return json;
	}
	/**
	 * @brief 相等比较运算符
	 * @param other 要比较的其他相机信息对象 用于支持QSet
	 * @return 如果两个相机信息对象相等，返回true；否则返回false
	 */
	bool operator==(const struCameraInfo& other) const {
		// 基于唯一ID比较是否相等
		return name == other.name;
	}

	INT AcquiringImage(dvpFrame* pFrame, void* pBuffer) const
	{
		QString image_path = QString(path + "/%1#%2.%3").arg(pFrame->uTriggerId).arg(pFrame->uTimestamp).arg(format);
		qDebug() << QThread::currentThread() << "AcquiringImage" << pFrame->uFrameID << pFrame->userValue << pFrame->uTriggerId << pFrame->uTimestamp<< "图片路径"<<image_path;
		dvpSavePicture(pFrame, pBuffer, image_path.toLocal8Bit().data(), 100);
		return 0;
	}
};

// Add this after your CameraInfo class definition 用于支持QSet
inline uint qHash(const struCameraInfo& info, uint seed = 0)
{
	return qHash(info.name, seed);
}

class CameraSDK : public QObject
{
	Q_OBJECT
public:
	explicit CameraSDK(QObject* parent = nullptr);
	~CameraSDK() = default;
	void test();
	const static int CurrentCameraCount = 8;
	//相机内部的变量的声明
	dvpStatus resultDvp;//统一的相机返回状态处理
	dvpUint32 devicesCount;//用于写入相机扫描个数的加载
	qint8 activeIndex{ 0 };//当前选中默认
	QStringList devicesNamesList;//保存的相机名称,用于打开相机

	QJsonObject cameraConfigJson; //保存的相机配置json ,用于保存时更新相机信息,不用重复加载
	QJsonObject cameraInfoJson; //保存的相机信息
	QJsonObject cameraParamsJson;//保存的统一的相机参数信息
	//QSet<struCameraInfo> set_cameras;
	struCameraInfo cameraInfoArray[CurrentCameraCount]; //std::vector<int> cameraInfoArray(8);//也可以

	Result LoadConfigFromFile(const QString& jsonFilePath);//加载json
	Result SaveConfigToFile(const QString& jsonFilePath = QString());//保存json

	void initialize();
	Result SetCamerasParams(const dvpHandle& handle);//设置相机参数
	Result SetCamerasParams(const dvpHandle& handle, const QJsonObject& param);//设置相机参数
	//相机内部调用
	Result scan(); // 预先浏览相机设备
	Result open();
	Result close();
	Result start();
	Result stop();
	Result Property();//仅在windows有效
	Result triggerFire(); //软触发一次
	bool isSoftTrigger{true}; //软件触发 ToDo 预留

	//与界面的交互
	Result prepare(const Session& session);
	// 自定义的槽函数
	Result slotDispRate();
};

inline QPointer<CameraSDK> gCameraSDK = nullptr;
