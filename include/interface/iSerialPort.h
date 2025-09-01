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


inline static constexpr SerialDeviceType kSupportedSerialDevices = static_cast<SerialDeviceType>(kSupportSerialCar | kSupportSerialClover);
/*定义的串口方法*/
class SerialPortTemplate : public QObject
{
    Q_OBJECT
public:
    explicit SerialPortTemplate(QObject* parent = nullptr,QString portName = NULL)
        : QObject(parent),serialPort(new QSerialPort(this))    {
        //open serial port
        if(!portName.isEmpty()){
            if(GetAvailablePorts().contains(portName)){
                open(portName);
            }else{
                LOG_ERROR(tr("未检测到要打开的串口号:%1").arg(portName));
                qWarning() << GetAvailablePorts();
            }
        }
        connect(serialPort,&QSerialPort::readyRead,this,&SerialPortTemplate::handleReadyRead);
        connect(serialPort,&QSerialPort::errorOccurred,this,&SerialPortTemplate::handleError);
    }

    virtual ~SerialPortTemplate(){
        close();
    };

    virtual bool isOpen() {
        return serialPort->isOpen();
    }

    virtual QString GetPortName() {
        return serialPort->portName();
    }
    // 打开串口
    virtual bool open(const QString& port)
    {
        qDebug() << "准备打开串口:" << port;
        serialPort->setPortName(port);
        serialPort->setBaudRate(921600);
        serialPort->setDataBits(QSerialPort::Data8);
        serialPort->setStopBits(QSerialPort::OneStop);
        serialPort->setParity(QSerialPort::NoParity);

        if (serialPort->open(QIODevice::ReadWrite)) {
            WriteData(0x00);
            return true;
        }
        LOG_ERROR(tr("打开串口%1失败,请检查是否被其他应用程序占用").arg(port));
        return false;
    }

    // 关闭串口
    virtual  void close()
    {
        if (serialPort->isOpen()) {
            serialPort->close();
        }
    }

    // 发送数据
    bool WriteData(const QByteArray &data)
    {
        static QReadWriteLock write;
        QWriteLocker locker(&write);
        return serialPort->write(data) == data.size();
    }
    
    // 发送数据
    bool WriteData(FunctionCodeType code, const QByteArray& data=QByteArray()) {
        QByteArray frame;
        frame.append(HEADER_BYTES);frame.append(LOCAL_ADDRESS);
        frame.append(code);frame.append(data);
        // 转换 QByteArray 到 std::vector<uint8_t>
        //std::vector<uint8_t> vecData(
        //    reinterpret_cast<const uint8_t*>(frame.constData()),
        //    reinterpret_cast<const uint8_t*>(frame.constData()) + frame.size()
        //);
        QByteArray crc = CRC16(frame);
        frame.append(crc);frame.append(TAIL_BYTES);
        return WriteData(frame);
    }

    QByteArray GetData(){
        return serialPort->readAll();
    }

    // 获取可用串口列表
    QStringList GetAvailablePorts()
    {
        QStringList ports;
        for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
            ports << info.portName();
        }
        return ports;
    }

protected:
    QSerialPort *serialPort;
    quint8 u8result{ 0 };//缓存的执行结果,用于执行指令
    QString lastError;//0 正常, 非0为错误
    
    QByteArray buffer_;
    QReadWriteLock bufferLock_;

    virtual void handleReadyRead(){
        QByteArray data = GetData();
        {
            QWriteLocker locker(&bufferLock_);
            buffer_.append(data);
            //qDebug()<<"SerialPortTemplate handleReadyRead:"<<buffer_.toHex().toUpper();
        }
        ProcessBuffer();
    };
    virtual void handleError(QSerialPort::SerialPortError error){
        if (error == QSerialPort::NoError) {
            return;
        }
        LOG_ERROR(tr("Serial port error:%1").arg(serialPort->errorString()));
        lastError = serialPort->errorString();
    };
    // 处理接收数据的虚函数,子类实现具体的解析逻辑
    virtual bool ProcessBuffer() {
        // 获取当前接收缓冲区数据
         QByteArray receivedData = buffer_; // 假设m_buffer是存储接收数据的成员变量
        
        qsizetype frameStart = receivedData.indexOf(HEADER_BYTES);
        if (frameStart == -1) {
            LOG_ERROR(tr("Buffer Data Header %1 not found:2%").arg(HEADER_BYTES.toHex().toUpper(),receivedData.toHex().toUpper()));
            return false;
        }
        // 查找帧尾（从帧头之后开始查找，提高效率
        qsizetype frameEnd = receivedData.indexOf(TAIL_BYTES, frameStart + HEADER_BYTES.size());
        if (frameEnd == -1) {
            LOG_ERROR(tr("Buffer Data Tail %1 not found:2%").arg(TAIL_BYTES.toHex().toUpper(), receivedData.toHex().toUpper()));
            return false;
        }
        // 清除已处理的数据
        {
            QWriteLocker locker(&bufferLock_);
            buffer_.remove(0, frameEnd + TAIL_BYTE_LEN);
        }

        // 进行CRC校验 
        qsizetype crcPos = frameEnd - CRC_BYTE_LEN;//帧尾前2字节
        QByteArray crc = receivedData.mid(crcPos, CRC_BYTE_LEN);// 截取从frameStart开始到frameEnd结束的数据（包含帧尾）
        qDebug() << "crc is" << CRC16(receivedData.mid(frameStart, crcPos - frameStart));
        if (crc != CRC16(receivedData.mid(frameStart,crcPos-frameStart))) {
            LOG_ERROR(tr("Buffer Data CRC %1 not match:%2").arg(crc.toHex().toUpper(), receivedData.toHex().toUpper()));
            return false;
        }
        
        FunctionCodeType code = receivedData.at(frameStart + FUNCTION_CODE_POS);
        qsizetype dataPos = frameStart + DATA_POS;
        QByteArray data = receivedData.mid(dataPos, crcPos - dataPos);
        return HandleProtocol(code, data);

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
            LOG_ERROR(tr("Failed to invoke method: %1 data:%2").arg(QString::number(code, 16)).arg(data.toHex().toUpper()));
            return false;
        };
        return true;
    }

};

}//end namespace

#endif
