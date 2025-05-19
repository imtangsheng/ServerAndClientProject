/*
* @file LeiShenLidarN10Plus.h
* @brief ���񼤹��״�N10Plus�������ļ� 
* @date 2025-05
* @author Tang
* 
*/
#pragma once

//#include "WorkHandler.h"
// �����״�����֡�ṹ
constexpr quint8 LIDAR_DATA_FRAME_SIZE = 108;
static inline const QByteArray LIDAR_DATA_FRAME_HEADER = QByteArrayLiteral("\xA5\x5A");
constexpr auto LIDAR_DATA_POINT_CLOUD_SIZE = 32;

constexpr qint32 LidarBaudRate = 460800; //������ Ϊ460800;
#pragma pack(push, 1)  // ����1�ֽڶ���
// �״����ݵ�ṹ
typedef struct {
    uint16_t distance;  // ���룬��λmm
    uint8_t intensity;  // �ź�ǿ��
} RadarPoint;
// һ֡�״����ݽṹ
typedef struct {
    uint16_t speed;           // �״�ת��,����:0x1046,4166us,����תһȦ 4166us*24=100ms,��ת��Ϊ10Hz
    uint16_t startAngle;      // ��ʼ�Ƕ�,����:0x4208,16904,��ʾ169.04��
    RadarPoint points[LIDAR_DATA_POINT_CLOUD_SIZE];    // ��������
    uint16_t reservedSpace;           // Ԥ��λ
    uint16_t stopAngle;       // �����Ƕ�,����:0x4208,16904,��ʾ169.04��
} RadarFrame;
#pragma pack(pop)  // �ָ�Ĭ�϶���

// ����У��� CRC = byte0+byte1+��+byte106
inline static uint8_t CalculateCRC(QByteArray data, uint8_t length) {
    uint8_t crc = 0;
    for (uint8_t i = 0; i < length; ++i) {
        crc += data[i];
    }
    return crc;
}

/*�״����ݴ�����
* @param data �״�����
*/
static void RecvRadarFrameData(const RadarFrame& data) {
    static SavaRawData radar_frame_file(QString("%1/radar.bin").arg(kTaskDirCameraName));
    static quint8 data_len = sizeof(RadarFrame);
    //�״�����
    if (gTaskState == TaskState_Running && radar_frame_file.initialize()) {
        //�״�����
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
    // �򿪴���
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

    // �رմ���
    virtual  void close() {
        if (serialPort->isOpen()) {
            serialPort->close();
        }
    }

    // ��ȡ���ô����б�
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
        // ��ȡ��ǰ���ջ���������
        QByteArray lidarFrameBuffer = buffer_.mid(frameStart, LIDAR_DATA_FRAME_SIZE); // ����m_buffer�Ǵ洢�������ݵĳ�Ա����
        // ����Ѵ��������
        {
            QWriteLocker locker(&bufferLock_);
            buffer_.remove(0, frameEnd);
        }
        // У��CRC
        quint8 calculatedCRC = CalculateCRC(lidarFrameBuffer, LIDAR_DATA_FRAME_SIZE - 1);
        if (calculatedCRC != lidarFrameBuffer[LIDAR_DATA_FRAME_SIZE - 1]) {
            LOG_ERROR(tr("Radar frame CRC error: 0x%1, 0x%2").arg(calculatedCRC, 0, 16).arg(lidarFrameBuffer.toHex().toUpper()));
            return;
        }

        QByteArray frameData = lidarFrameBuffer.mid(3, LIDAR_DATA_FRAME_SIZE - 4);
        QDataStream stream(frameData);
        stream.setByteOrder(QDataStream::BigEndian);  // �����ֽ���
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