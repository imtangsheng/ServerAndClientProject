/*
* @file LeiShenLidarN10Plus.h
* @brief 雷神激光雷达N10Plus的声明文件 
* @date 2025-05
* @author Tang
* 
*/
#pragma once

//#include "WorkHandler.h"
// 定义雷达数据帧结构
constexpr quint8 LIDAR_DATA_FRAME_SIZE = 108;
static inline const QByteArray LIDAR_DATA_FRAME_HEADER = QByteArrayLiteral("\xA5\x5A");
constexpr auto LIDAR_DATA_POINT_CLOUD_SIZE = 32;

constexpr qint32 LidarBaudRate = 460800; //波特率 为460800;
#pragma pack(push, 1)  // 设置1字节对齐
// 雷达数据点结构
typedef struct {
    uint16_t distance;  // 距离，单位mm
    uint8_t intensity;  // 信号强度
} RadarPoint;
// 一帧雷达数据结构
typedef struct {
    uint16_t speed;           // 雷达转速,例如:0x1046,4166us,码盘转一圈 4166us*24=100ms,即转速为10Hz
    uint16_t startAngle;      // 起始角度,例如:0x4208,16904,表示169.04度
    RadarPoint points[LIDAR_DATA_POINT_CLOUD_SIZE];    // 点云数据
    uint16_t reservedSpace;           // 预留位
    uint16_t stopAngle;       // 结束角度,例如:0x4208,16904,表示169.04度
} RadarFrame;
#pragma pack(pop)  // 恢复默认对齐

// 计算校验和 CRC = byte0+byte1+…+byte106
inline static uint8_t CalculateCRC(QByteArray data, uint8_t length) {
    uint8_t crc = 0;
    for (uint8_t i = 0; i < length; ++i) {
        crc += data[i];
    }
    return crc;
}

/*雷达数据处理函数
* @param data 雷达数据
*/
static void RecvRadarFrameData(const RadarFrame& data) {
    static SavaRawData radar_frame_file(QString("%1/radar.bin").arg(kTaskDirCameraName));
    static quint8 data_len = sizeof(RadarFrame);
    //雷达数据
    if (gTaskState == TaskState_Running && radar_frame_file.initialize()) {
        //雷达数据
        radar_frame_file.WriteAndFlush(reinterpret_cast<const char*>(&data), data_len);
    }
}

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QReadWriteLock>
class LeiShenLidarN10Plus :public QObject
{
    Q_OBJECT
public:
    explicit LeiShenLidarN10Plus(QObject* parent = nullptr, QString portName = NULL)
        : QObject(parent), serialPort(new QSerialPort(this)) {
        //open serial port
        if (!portName.isEmpty()) {
            if (GetAvailablePorts().contains(portName)) {
                open(portName);
            } else {
                LOG_ERROR(tr("Unable to scan the specified serial port:%1").arg(portName));
            }
        }
        connect(serialPort, &QSerialPort::readyRead, this, &LeiShenLidarN10Plus::handleReadyRead);
        connect(serialPort, &QSerialPort::errorOccurred, this, &LeiShenLidarN10Plus::handleError);
    }

    ~LeiShenLidarN10Plus() {
        close();
    }
    // 打开串口
    virtual bool open(const QString& port) {
        if (serialPort->isOpen()) {
            LOG_WARNING(tr("Serial port %1 is already open").arg(port));
            return false;
        }
        serialPort->setPortName(port);
        serialPort->setBaudRate(LidarBaudRate);
        serialPort->setDataBits(QSerialPort::Data8);
        serialPort->setStopBits(QSerialPort::OneStop);
        serialPort->setParity(QSerialPort::NoParity);

        if (serialPort->open(QIODevice::ReadWrite)) {
            return true;
        }
        LOG_ERROR(tr("Failed to open serial port %1").arg(port));
        return false;
    }

    // 关闭串口
    virtual  void close() {
        if (serialPort->isOpen()) {
            serialPort->close();
        }
    }

    // 获取可用串口列表
    QStringList GetAvailablePorts() {
        QStringList ports;
        for (const QSerialPortInfo& info : QSerialPortInfo::availablePorts()) {
            ports << info.portName();
        }
        return ports;
    }

protected:
    QSerialPort* serialPort;
    QByteArray buffer_;
    QReadWriteLock bufferLock_;

    void handleReadyRead() {
        QByteArray data = serialPort->readAll();
        {
            QWriteLocker locker(&bufferLock_);
            buffer_.append(data);
            //qDebug()<<"SerialPortTemplate handleReadyRead:"<<buffer_.toHex().toUpper();
        }
        qsizetype frameStart = buffer_.indexOf(LIDAR_DATA_FRAME_HEADER);
        if (frameStart == -1) {
            LOG_ERROR(tr("Buffer Data Header %1 not found:%2").arg(LIDAR_DATA_FRAME_HEADER.toHex().toUpper()).arg(buffer_.toHex().toUpper()));
            return;
        }
        quint8 frameEnd = frameStart + LIDAR_DATA_FRAME_SIZE;
        if (buffer_.size() < frameEnd) {
            LOG_ERROR(tr("Buffer Data not enough:%1").arg(buffer_.toHex().toUpper()));
            return;
        }
        // 获取当前接收缓冲区数据
        QByteArray lidarFrameBuffer = buffer_.mid(frameStart, LIDAR_DATA_FRAME_SIZE); // 假设m_buffer是存储接收数据的成员变量
        // 清除已处理的数据
        {
            QWriteLocker locker(&bufferLock_);
            buffer_.remove(0, frameEnd);
        }
        // 校验CRC
        quint8 calculatedCRC = CalculateCRC(lidarFrameBuffer, LIDAR_DATA_FRAME_SIZE - 1);
        if (calculatedCRC != lidarFrameBuffer[LIDAR_DATA_FRAME_SIZE - 1]) {
            LOG_ERROR(tr("Radar frame CRC error: 0x%1, 0x%2").arg(calculatedCRC, 0, 16).arg(lidarFrameBuffer.toHex().toUpper()));
            return;
        }

        QByteArray frameData = lidarFrameBuffer.mid(3, LIDAR_DATA_FRAME_SIZE - 4);
        QDataStream stream(frameData);
        stream.setByteOrder(QDataStream::BigEndian);  // 设置字节序
        RadarFrame frame{};
        stream >> frame.speed >> frame.startAngle;
        for (int i = 0; i < LIDAR_DATA_POINT_CLOUD_SIZE; ++i) {
            stream >> frame.points[i].distance >> frame.points[i].intensity;
        }
        stream >> frame.reservedSpace >> frame.stopAngle;
        RecvRadarFrameData(frame);
    }

    void handleError(QSerialPort::SerialPortError error) {
        if (error == QSerialPort::NoError) {
            return;
        }
        LOG_ERROR(tr("Serial port error:%1").arg(serialPort->errorString()));
    }

};