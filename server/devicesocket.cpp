#include <QCoreApplication>
#include <QThread>
#include <QMutexLocker>

#include "devicesocket.h"
#include "qscrcpyevent.h"

DeviceSocket::DeviceSocket(QObject *parent)
    : QTcpSocket(parent)
{
    connect(this, &DeviceSocket::readyRead, this, &DeviceSocket::onReadyRead);
    connect(this, &DeviceSocket::disconnected, this, &DeviceSocket::quitNotify);
    connect(this, &DeviceSocket::aboutToClose, this, &DeviceSocket::quitNotify);

}

DeviceSocket::~DeviceSocket()
{

}

qint32 DeviceSocket::subThreadRecvData(quint8 *buf, qint32 bufSize)
{
    // 保证在子线程中调用
    Q_ASSERT(QCoreApplication::instance()->thread() != QThread::currentThread());
    if (m_quit) {
        return 0;
    }

    QMutexLocker locker(&m_mutex);

    m_buffer = buf;
    m_bufferSize = bufSize;
    m_dataSize = 0;

    // 发送事件
    DeviceSocketEvent* getDataEvent = new DeviceSocketEvent();
    QCoreApplication::postEvent(this, getDataEvent);

    while (!m_recvData) {
        m_recvDataCond.wait(&m_mutex);
    }

    m_recvData = false;
    return m_dataSize;
}

bool DeviceSocket::event(QEvent *event)
{
    if (event->type() == QScrcpyEvent::DeviceSocket) {
        onReadyRead();
        return true;
    }
    return QTcpSocket::event(event);
}

void DeviceSocket::onReadyRead()
{
    QMutexLocker locker(&m_mutex);
    if (m_buffer && 0 < bytesAvailable()) {
        // 接收数据
        qint64 readSize = qMin(bytesAvailable(), (qint64)m_bufferSize);
        m_dataSize = read((char*)m_buffer, readSize);

        m_buffer = Q_NULLPTR;
        m_bufferSize = 0;
        m_recvData = true;
        m_recvDataCond.wakeOne();
    }
}

void DeviceSocket::quitNotify()
{
    m_quit = true;
    QMutexLocker locker(&m_mutex);
    if (m_buffer) {
        m_buffer = Q_NULLPTR;
        m_bufferSize = 0;
        m_recvData = true;
        m_dataSize = 0;
        m_recvDataCond.wakeOne();
    }
}
