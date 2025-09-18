/*
* @file LineF400Imu.h
* @brief LINS-F400 �������ݹ��Բ�����Ԫ ʹ��B6Э��
* @date 2025-07
* @author Tang
*
*/
#pragma once

//#include "TaskManager.h"
// �����״�����֡�ṹ
constexpr quint8 IMU_DATA_FRAME_SIZE = 41; //41+8+8+1�ֽ����ݳ�
static inline const QByteArray IMU_DATA_FRAME_HEADER = QByteArrayLiteral("\x7F\x82");
// B6Э�鳣��
static constexpr double B6_ACCEL_SCALE = 134217728.0;
static constexpr double B6_RATE_SCALE = 134217728.0;
static constexpr double B6_ANGLE_SCALE = 536870912.0;

constexpr qint32 ImuBaudRate = 115200; //������
#pragma pack(push, 1)  // ����1�ֽڶ���
// һ֡���ݽṹ
typedef struct {
    int32_t gyroX, gyroY, gyroZ;   // ����������
    int32_t accX, accY, accZ;      // ���ٶ�����
    int64_t time; //�ڲ�ʱ������
} ImuFrame;
#pragma pack(pop)  // �ָ�Ĭ�϶���

// ����У��� CRC = byte0+byte1+�� Ȼ��ȡ��
inline static uint8_t CalculateCRC(QByteArray data, uint8_t length) {
    uint8_t crc = 0;
    for (uint8_t i = 0; i < length; ++i) {
        crc += data[i];
    }
    return ~crc;
}

/*���ݴ�����
* @param data ����
*/
static void RecvImuData(const ImuFrame& data) {
    static QString header = "time\tXRotation\tYRotation\tZRotation\tXAcceleration\tYAcceleration\tZAcceleration\n";
    static SavaDataFile imu_file(QString("%1/Imu.txt").arg(kTaskDirCarName),header);
    //�ߵ�����
    if (gTaskState == TaskState_Running && imu_file.create_file()) {
        //�������ܴ�,��������ճ��
        QString str = QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\n").arg(data.time).arg(data.gyroX).arg(data.gyroY).arg(data.gyroZ).arg(data.accX).arg(data.accY).arg(data.accZ);
        imu_file.WriteLine(str);
    }
}

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QReadWriteLock>
class LinsF400Imu :public QObject
{
    Q_OBJECT
public:
    explicit LinsF400Imu(QObject* parent = nullptr, QString portName = NULL)
        : QObject(parent), serial(new QSerialPort(this)) {
        //open serial port
        if (!portName.isEmpty()) {
            if (GetAvailablePorts().contains(portName)) {
                open(portName);
            } else {
                LOG_ERROR(tr("Unable to scan the specified serial port:%1").arg(portName));
            }
        }
        connect(serial, &QSerialPort::readyRead, this, &LinsF400Imu::handleReadyRead);
        connect(serial, &QSerialPort::errorOccurred, this, &LinsF400Imu::handleError);
    }

    ~LinsF400Imu() {
        close();
    }
    // �򿪴���
    bool open(const QString& port) {
        if (serial->isOpen()) {
            LOG_WARNING(tr("Serial port %1 is already open").arg(port));
            return false;
        }
        serial->setPortName(port);
        serial->setBaudRate(ImuBaudRate);
        serial->setDataBits(QSerialPort::Data8);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setParity(QSerialPort::NoParity);

        if (serial->open(QIODevice::ReadWrite)) {
            return true;
        }
        LOG_ERROR(tr("Failed to open serial port %1").arg(port));
        return false;
    }

    // �رմ���
    void close() {
        if (serial->isOpen()) {
            serial->close();
        }
    }

    // ��ȡ���ô����б�
    inline QStringList GetAvailablePorts() {
        QStringList ports;
        foreach (const QSerialPortInfo& info , QSerialPortInfo::availablePorts()) {
            ports << info.portName();
        }
        return ports;
    }

protected:
    QSerialPort* serial;
    QByteArray buffer_;
    QReadWriteLock bufferLock_;

    void handleReadyRead() {
        QByteArray data = serial->readAll();
        {
            QWriteLocker locker(&bufferLock_);
            buffer_.append(data);
            //qDebug()<<"SerialPortTemplate handleReadyRead:"<<buffer_.toHex().toUpper();
        }
        qsizetype frameStart = buffer_.indexOf(IMU_DATA_FRAME_HEADER);
        if (frameStart == -1) {
            LOG_ERROR(tr("Buffer Data Header %1 not found:%2").arg(IMU_DATA_FRAME_HEADER.toHex().toUpper()).arg(buffer_.toHex().toUpper()));
            return;
        }
        quint8 frameEnd = frameStart + IMU_DATA_FRAME_SIZE;
        if (buffer_.size() < frameEnd) {
            LOG_ERROR(tr("Buffer Data not enough:%1").arg(buffer_.toHex().toUpper()));
            return;
        }
        // ��ȡ��ǰ���ջ���������
        QByteArray frameBuffer = buffer_.mid(frameStart, IMU_DATA_FRAME_SIZE); // ����m_buffer�Ǵ洢�������ݵĳ�Ա����
        // ����Ѵ��������
        {
            QWriteLocker locker(&bufferLock_);
            buffer_.remove(0, frameEnd);
        }
        // У��CRC
        quint8 checksum = CalculateCRC(frameBuffer, IMU_DATA_FRAME_SIZE - 1);
        if (checksum != frameBuffer[IMU_DATA_FRAME_SIZE - 1]) {
            LOG_ERROR(tr("Radar frame CRC error: 0x%1, 0x%2").arg(checksum, 0, 16).arg(frameBuffer.toHex().toUpper()));
            return;
        }

        // QByteArray frameData = frameBuffer.mid(3, IMU_DATA_FRAME_SIZE - 4);
        QDataStream stream(frameBuffer);
        stream.setByteOrder(QDataStream::BigEndian);  // �����ֽ���
        ImuFrame frame{};
        stream >> frame.accX >> frame.accY >> frame.accZ >> frame.gyroX >> frame.gyroY >> frame.gyroZ;

        // RecvImuData(frame);
    }

    void handleError(QSerialPort::SerialPortError error) {
        if (error == QSerialPort::NoError) {
            return;
        }
        LOG_ERROR(tr("Serial port error:%1").arg(serial->errorString()));
    }

};
