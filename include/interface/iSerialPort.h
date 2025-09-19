#ifndef _ISERIALPORT_H
#define _ISERIALPORT_H
#include <QByteArray>
#include <QString>
#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QReadWriteLock>
#include <QTimer>
#include <QAtomicInteger>
#include "serialport_protocol.h"

namespace serial {

/*定义的串口方法*/
class SerialPortTemplate : public QObject
{
    Q_OBJECT
    
public:
    explicit SerialPortTemplate(QObject* parent = nullptr, QString portName = NULL)
        : QObject(parent), serial(new QSerialPort(this)) {
        //open serial port
        if (!portName.isEmpty()) {
            if (GetAvailablePorts().contains(portName)) {
                open(portName);
            } else {
                LOG_ERROR(tr("未检测到要打开的串口号:%1").arg(portName));
                qWarning() << GetAvailablePorts();
            }
        }
        connect(serial, &QSerialPort::readyRead, this, &SerialPortTemplate::handleReadyRead);
        connect(serial, &QSerialPort::errorOccurred, this, &SerialPortTemplate::handleError);

        uCurrentSerialDevice = gShare.RegisterSettings->value("type", 0).toInt();
    }

    virtual ~SerialPortTemplate() {
        close();
        delete serial;
    };

    virtual bool isOpen() {
        return serial->isOpen();
    }

    virtual QString GetPortName() {
        return serial->portName();
    }
    // 打开串口
    bool open(const QString& port) {
        qDebug() << "准备打开串口:" << port;
        serial->setPortName(port);
        serial->setBaudRate(921600);
        serial->setDataBits(QSerialPort::Data8);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setParity(QSerialPort::NoParity);

        if (serial->open(QIODevice::ReadWrite)) {
            WriteData(CODE_TEST);
            //QThread::msleep(25); // 25ms 间隔不会有粘包的现象
            //WriteData(AUTOMATION_TIME_SYNC);
            return true;
        }
        
        return false;
    }

    // 关闭串口
    void close() {
        if (serial->isOpen()) {
            serial->close();
        }
    }

    // 发送数据
    bool WriteData(const QByteArray& data) {
        static QReadWriteLock write;
        QWriteLocker locker(&write);
        //serial->flush(); // 强制立即发送,无用
        return serial->write(data) == data.size();
    }

    // 发送数据
    bool WriteData(FunctionCodeType code, const QByteArray& data = QByteArray()) {
        QByteArray frame;
        frame.append(HEADER_BYTES); 
        frame.append(LOCAL_ADDRESS);
        //frame.append(static_cast<char>(data.size()));//目前校验不加长度
        frame.append(code);
        frame.append(data);
        // 转换 QByteArray 到 std::vector<uint8_t>
        //std::vector<uint8_t> vecData(
        //    reinterpret_cast<const uint8_t*>(frame.constData()),
        //    reinterpret_cast<const uint8_t*>(frame.constData()) + frame.size()
        //);
        QByteArray crc = CRC16(frame);
        frame.append(crc); 
        //frame.append(TAIL_BYTES);
        return WriteData(frame);
    }

    QByteArray GetData() {
        return serial->readAll();
    }

    // 获取可用串口列表
    QStringList GetAvailablePorts() {
        QStringList ports;
        foreach(const QSerialPortInfo & info, QSerialPortInfo::availablePorts()) {
            ports << info.portName();
        }
        return ports;
    }

protected:
    quint8 uCurrentSerialDevice;//保存当前串口类的设备型号数字 1 中控 2 三叶草 3 三叶草21机位
    QSerialPort* serial;
    qint8 i8result{ 0 };//缓存的执行结果,用于执行指令 -1 为失败 0 为成功 其他为错误码
    QString lastError;//0 正常, 非0为错误

    QByteArray buffer_;
    QReadWriteLock bufferLock_;

    void handleReadyRead() {
        QByteArray data = GetData();
        {
            QWriteLocker locker(&bufferLock_);
            //根据时间,大于5秒清空一次数据
            if (!buffer_.isEmpty()) {
                static qint64 latency = QDateTime::currentMSecsSinceEpoch();
                qint64 current = QDateTime::currentMSecsSinceEpoch();
                if (current - latency > 5000) {//5秒 清除
                    qDebug() << "串口缓冲区数据清除:" << buffer_.toHex().toUpper();
                    buffer_.clear();
                }
                latency = current;
            }
            buffer_.append(data);
        }
        ProcessBuffer();
    };
    virtual void handleError(QSerialPort::SerialPortError error) {
        if (error == QSerialPort::NoError) {
            return;
        }
        LOG_ERROR(tr("串口错误:%1").arg(serial->errorString()));
        lastError = serial->errorString();
    };
    // 处理接收数据的虚函数,子类实现具体的解析逻辑
    virtual void ProcessBuffer() {
        // 获取当前接收缓冲区数据
        while (!buffer_.isEmpty()) {
            QByteArray receivedData = buffer_; // 假设m_buffer是存储接收数据的成员变量
            qsizetype frameStart = receivedData.indexOf(HEADER_BYTES);
            if (frameStart == -1) {
                qDebug() << QString("数据标志头:%1 没有找到:%2").arg(HEADER_BYTES.toHex().toUpper(), receivedData.toHex().toUpper()) << "数据大小" << receivedData.size();
                break;
            }
            //起始位+帧头长度 2+ 数据码长度 1+功能码长度1 + 数据长度
            qsizetype dataLen = receivedData.at(frameStart + DATA_LEN_POS);
            qsizetype frameCrcPos = frameStart + HEADER_BYTE_LEN +1+1+ dataLen;
            
            qsizetype frameLen = frameCrcPos + CRC_BYTE_LEN;//CRC位置 + CRC长度
            if (receivedData.size() < frameLen) {
                qDebug() << (tr("数据长度不足, 数据:%1").arg(receivedData.toHex().toUpper()));
                break;
            }
            // 清除已处理的数据
            {
                QWriteLocker locker(&bufferLock_);
                buffer_.remove(0, frameLen);
            }
            FunctionCodeType code = receivedData.at(frameStart + FUNCTION_CODE_POS);
            // 进行CRC校验 
            QByteArray crc = receivedData.mid(frameCrcPos, CRC_BYTE_LEN);// 截取从 frameStart开始到 frameEnd结束的数据（包含帧尾）
            qDebug() << "CRC校验值是:" << receivedData.mid(frameStart, frameCrcPos).toHex().toUpper() << CRC16(receivedData.mid(frameStart, frameCrcPos)).toHex();
            if (crc != CRC16(receivedData.mid(frameStart, frameCrcPos))) {
                LOG_ERROR(tr("CRC校验值: %1 没有匹配:%2").arg(crc.toHex().toUpper(), receivedData.toHex().toUpper()));
                break;
            }
            QByteArray data = receivedData.mid(frameStart + DATA_POS, dataLen);
            HandleProtocol(code, data);
        }

    };
    /*
    方案	调用耗时(ns)	适用场景
    switch - case	1 - 10	高频核心协议
    invokeMethod	100 - 1000	低频扩展协议 / 跨线程调用
    注册表(std::function)	    10 - 50	平衡性能与灵活性
    */
    virtual bool HandleProtocol(FunctionCodeType code, const QByteArray& data) {
        //switch (code) {
        //case 0x00: /* 高频操作 */ break;
        //case 0x01: /* 关键路径 */ break;
        //QByteArray method = FunctionCodeToString(code).toUtf8();
        //qDebug() << "code is" << QString::number(code, 16) << method.constData();
        QString method = FunctionCodeToString(code);
        if (!QMetaObject::invokeMethod(this, method.toUtf8(), Q_ARG(QByteArray, data))) {
            LOG_ERROR(tr("调用方法错误: %1 数据:%2").arg(QString::number(code, 16)).arg(data.toHex().toUpper()));
            return false;
        };
        return true;
    }

};

}//end namespace

#endif
