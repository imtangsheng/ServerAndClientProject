#include "FaroHandle.h"
#include "iFaro.h"
using namespace faro;


/**
 * @brief import directive of iQOpen.SDK
 * 声明法如扫描的解析变量,参考FARO自动化手册
*/
static IiQLicensedInterfaceIfPtr liPtr{ nullptr }; //许可
static IiQLibIfPtr libRef{ nullptr }; //reference 指向变量或对象的引用
static bool gIsFaroInitialized = false;//法如初始化标志
static void FaroCleanup() {
    if (gIsFaroInitialized) {
        if (libRef) { libRef = nullptr; }
        if (liPtr) { liPtr = nullptr; }
        CoUninitialize();
        gIsFaroInitialized = false;
    }
}

static Result FaroInitialized() {
    if (gIsFaroInitialized) { return true; }
    try {
        // 初始化COM组件
        if (FAILED(CoInitialize(nullptr))) { throw QObject::tr("[#Faro]初始化COM组件错误"); }
        // 创建并验证许可接口
        liPtr = IiQLicensedInterfaceIfPtr(__uuidof(iQLibIf));
        if (!liPtr) { throw QObject::tr("[#Faro]创建并验证许可接口错误"); }
        liPtr->License = LicenseCode;
        libRef = static_cast<IiQLibIfPtr>(liPtr);
        if (!libRef) { throw QObject::tr("[#Faro]验证许可接口错误"); }
    } catch (const QString& error) {
        LOG_ERROR(QObject::tr("#Faro:法如扫描初始化错误: %1").arg(error));
        return false;
    }
    return true;
}

FaroHandle::FaroHandle(QObject* parent)
    : QObject(parent) {
    qDebug() << "[#FaroHandle]构造函数:" << QThread::currentThread();
    thread = new QThread;// moveToThread，对象和它的父对象必须在同一个线程中
    moveToThread(thread);
    connect(thread, &QThread::started, this, &FaroHandle::awake);
    thread->start();
}

FaroHandle::~FaroHandle() {
    qDebug() << "[#FaroHandle]析构函数:" << QThread::currentThread();
    //内存泄漏 线程未停止 不可预测的行为 需要确定退出线程在删除
    //thread->wait();     // 等待线程真正结束Thread tried to wait on itself,不可在自己的线程中等待自己结束,容易死锁
    //安全删除线程对象 在被销毁（delete thread）时仍在运行
    
    //// 检查当前线程是否为 thread
    //if (QThread::currentThread() == thread) {
    //    qDebug() << "[#FaroHandle] 析构函数运行在 thread 线程中，使用 deleteLater";
        thread->quit();           // 请求线程退出
        thread->deleteLater();    // 延迟删除 thread
        thread = nullptr;
    //} else {
    //    // 在非 thread 线程中，可以安全停止和删除
    //    if (thread->isRunning()) {
    //        thread->quit();       // 请求线程退出
    //        thread->wait(1000);   // 等待最多 1 秒（避免无限等待）
    //        if (thread->isRunning()) {
    //            qDebug() << "[#FaroHandle] 线程未能在 1 秒内停止，可能存在阻塞任务";
    //            thread->terminate(); // 强制终止（谨慎使用）
    //            thread->wait(500);  // 再次等待
    //        }
    //    }
    //    delete thread;            // 安全删除
    //    thread = nullptr;
    //}
    FaroCleanup();
    //delete thread; // 安全删除线程对象
}

void FaroHandle::awake() {
    qDebug() << "[FaroHandle]awake初始化构造:" << QThread::currentThread();
    FaroInitialized();
}

Result FaroHandle::LoadFlsFile(QString filepath) {
    try {
        // 加载FLS文件 行 列  4268 5000 13722 ms
        static QDateTime startTime = QDateTime::currentDateTime();
        _bstr_t filepathBstr(filepath.toStdString().c_str());
        // 加载文件不支持多个实例多线程,有锁
        int ret = libRef->load(filepathBstr);//Return Values: 	0, 11, 12, 13, 25
        if (ret != 0) {
            throw QObject::tr("[#Faro]文件格式错误,错误码%1").arg(ret);//11 没有工作空间 No workspace 
        }
        qDebug() << "#Faro:加载FLS文件:" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
        return true;
    } catch (const QString error) {
        return Result::Failure(tr("#Faro:FLS解析错误: %1").arg(error));
    }
}


//极坐标点向上移动z米的计算后新的坐标
static ScanPointPolar PointSphericalOriginalShiftUp(ScanPointPolar original, double shiftUp) {
    ScanPointPolar newPoint;
    // 1. phi(方位角)保持不变，因为垂直移动不影响水平方向的角度
    newPoint.phi = original.phi;
    if (original.r < InvalidValue) {
        return newPoint;
    }
    // 2. 计算新的r和theta
    double x = original.r * cos(original.theta) * cos(original.phi);
    double y = original.r * cos(original.theta) * sin(original.phi);
    double z = original.r * sin(original.theta);
    // 垂直位移
    z -= shiftUp;
    // 3. 新的距离r
    newPoint.r = sqrt(x * x + y * y + z * z);
    // 4. 新的天顶角theta atan(y/x) 只能返回 -π/2 到 π/2 的角度 atan2(y, x) 可以返回完整的 -π 到 π 范围它考虑了 x 和 y 的符号，能正确判断角度在哪个象限
    //newPoint.theta = atan2(horizontalDist,verticalDist);
    newPoint.theta = asin(z / newPoint.r);
    return newPoint;
}

//计算中位数
static double CalculateMedian(QVector<double>& values) {
    if (values.isEmpty()) return 0.0;
    // 对数组进行排序
    std::sort(values.begin(), values.end());
    size_t n = values.size();
    // 计算中位数
    if (n % 2 == 0) {
        return (values[n / 2 - 1] + values[n / 2]) / 2.0;
    } else {
        return values[n / 2];
    }
}

//inline static double FaroElevationToCameraAngle(double theta, bool first = true) {
//	if (first) 
//		return 3.0 / 2 * M_PI + theta;
//	else 
//		return (theta -1.0 / 2 * M_PI)*-1;
//}

constexpr int col_focus = 50;//相机计算列,默认中间列
void FaroHandle::CreateCameraFocalByScanFile(const QJsonObject& in) {
    qDebug() << "进行相机角度焦距计算:" << QThread::currentThread();

    QString flsFileDir = in.value("dir").toString(); 
    if (flsFileDir.isEmpty()) {
        LOG_ERROR(tr("检测到要输入的目录参数为空"));
        return;
    }
    // 获取当前文件列表
    QDir dir(flsFileDir);// 只监控 .txt 和 .log 文件
    QStringList filters; filters << "*.fls";
    QFileInfoList currentFiles = dir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);//QDir::
    if (currentFiles.isEmpty()) { LOG_ERROR(tr("没有找到FLS文件")); return; }
    if (currentFiles.size() > 1) { LOG_INFO(tr("找到%1个FLS文件,默认使用第一个").arg(currentFiles.size())); }

    double cameraHight = in.value("CameraHight").toDouble(1.8);//相机中心到轨面的高度
    //static double gScannerHight = ;
    double scannerHight = in.value("ScannerHight").toDouble(0.5544);//扫描仪中心到轨面高度是554.4mm
    if (cameraHight < 2 * scannerHight || cameraHight > 5) {
        LOG_ERROR(tr("相机中心到轨面的高度%1不符合要求,请重新输入").arg(cameraHight));
        return;
    }
    double shiftUp = cameraHight - scannerHight;//向上移动原点
    size_t parteQual = in.value("partes").toInt(15);//等分数 分布的角度值 范围(0-2*M_PI)
    static double rangeValue = 0.2;//组与其他一个组之间的重叠率,取开始的一个计算范围

    double interval_angle = (2 * M_PI / parteQual);//分组的角度值
    QList<double> rangeAngle(parteQual);//等分的起始角度
    for (int i = 0; i < parteQual; i++) {
        rangeAngle[i] = (i * interval_angle + rangeValue * interval_angle);
    }

    //读取第一个文件
    Result ret;
    ret = LoadFlsFile(currentFiles.at(0).absoluteFilePath());
    if (!ret) { LOG_ERROR(ret.message); return; }
    int numRows = libRef->getScanNumRows(0);
    int numCols = libRef->getScanNumCols(0);
    qDebug() << "#Faro:FLS文件加载完成,列数 行数" << libRef->getScanNumCols(0) << libRef->getScanNumRows(0);
    // Access all points column per column in polar coordinates
    QVector<ScanPointPolar> firstHalf(numRows);
    QVector<ScanPointPolar> secondHalf(numRows);
    double* positions = new double[numRows * 3];
    int* reflections = new int[numRows];

    auto getPolarScanPoints = [&](int col, QVector<ScanPointPolar>& points) {
        int result = libRef->getPolarScanPoints(0, 0, col, numRows, positions, reflections);//0, 26, 92 
        if (result != ErrorNumbers::OK) {
            LOG_ERROR(tr("严重错误,获取法如文件的坐标值错误")); return;
        }
        for (int row = 0; row < numRows; row++) {
            ScanPointPolar polar;
            polar.r = positions[3 * row + 0];
            polar.phi = positions[3 * row + 1];
            polar.theta = positions[3 * row + 2];
            points[row] = PointSphericalOriginalShiftUp(polar, shiftUp);
        }
        };
    // 处理两半数据
    getPolarScanPoints(col_focus, firstHalf);
    getPolarScanPoints(numCols / 2 + col_focus, secondHalf);
    //const double rad_to_deg = 180.0 / M_PI;// 弧度转角度
    qDebug() << "#Faro:FLS文件数据提取完成";
    delete[] positions;
    delete[] reflections;

    QVector<QVector<double>> groups(parteQual);//预分配组
    auto processPoints = [&](const QVector<ScanPointPolar>& points, bool isFirstHalf = true) {
        for (const auto& point : points) {
            //取正上方向为拍照焦距设置
            double angle = isFirstHalf ? (3.0 / 2 * M_PI + point.theta) : ((point.theta - 1.0 / 2 * M_PI) * -1);
            size_t groupIndex = static_cast<int>(angle / interval_angle) % parteQual;//分组索引 有范围2pi[0-2pi)
            // 处理重叠部分 摄像头景深,取某个范围的值
            if (angle < rangeAngle.at(groupIndex)) {
                groups[groupIndex].append(point.r);
            }
        }
        };
    // 处理两半数据
    processPoints(firstHalf, true);
    processPoints(secondHalf, false);
    qDebug() << "#Faro:FLS文件数据分组完成";
    // 计算中位数
    QJsonArray focals;
    for (size_t i = 0; i < parteQual; i++) {
        double median = CalculateMedian(groups[i]);
        if (median < InvalidValue) median = cameraHight;
        //扫描死角出现为0,默认设置为相机高度,(小于扫描仪高度就可以,如果输入的值小于标定的最小高度就按标定的值计算)
        focals.insert(i, median - gCameraCenterToLens);
    }
    qDebug() << "#Faro:FLS文件数据分组完成计算中位数" << focals;
    QString outfilepath = flsFileDir + "/camera_focal.json";
    QJsonObject out;
    out["focals"] = focals;
    ret = WriteJsonFile(outfilepath, out);
    if (!ret) { LOG_ERROR(ret.message); return; }
    session.result = out;
    gShare.on_session(session.ResponseString(out, tr("测量相机焦距成功")) , session.socket);
}
