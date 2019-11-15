#ifndef DEVICESOCKET_H
#define DEVICESOCKET_H

#include <QTcpSocket>
#include <QWaitCondition>
#include <QMutex>

class DeviceSocket : public QTcpSocket
{
    Q_OBJECT
public:
    DeviceSocket(QObject *parent = Q_NULLPTR);
    ~DeviceSocket();

    qint32 subThreadRecvData(quint8* buf, qint32 bufSize);

protected:
    bool event(QEvent *event);

protected slots:
    void onReadyRead();
    void quitNotify();

private:
    // 锁
    QMutex m_mutex;
    QWaitCondition m_recvDataCond;

    // 标志
    bool m_recvData = false;
    bool m_quit = false;


    // 数据缓存
    quint8* m_buffer = Q_NULLPTR;
    qint32 m_bufferSize = 0;
    qint32 m_dataSize = 0;
};

#endif // DEVICESOCKET_H
