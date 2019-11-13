#include <QFileInfo>
#include <QCoreApplication>

#include "server.h"

#define DEVICE_SERVER_PATH "/data/local/tmp/scrcpy-server.jar"
#define SOCKET_NAME "scrcpy"
#define DEVICE_NAME_FIELD_LENGTH 64
server::server(QObject *parent)
    : QObject(parent)
{
    connect(&m_workProcess, &AdbProcess::adbProcessResult, this, &server::onWorkProcessResult);
    connect(&m_serverProcess, &AdbProcess::adbProcessResult, this, &server::onWorkProcessResult);
    connect(&m_serverSocket, &QTcpServer::newConnection, this, [this](){
        m_deviceSocket = dynamic_cast<DeviceSocket*>(m_serverSocket.nextPendingConnection());

        QString deviceName;
        QSize size;
        // devices name, size
        if (m_deviceSocket && m_deviceSocket->isValid() && readInfo(deviceName, size)) {
            disableTunnelReverse();
            removeServer();
            emit connectToResult(true, deviceName, size);
        } else {
            stop();
            emit connectToResult(false, deviceName, size);
        }
    });
}

bool server::start(const QString &serial, quint16 localPort, quint16 maxSize, quint32 bitRate)
{
    m_serial = serial;
    m_localPort = localPort;
    m_maxSize = maxSize;
    m_bitRate = bitRate;

    // start push server
    m_serverStartStep = SSS_PUSH;
    return startServerByStep();
}

void server::stop()
{
    if (m_deviceSocket) {
        m_deviceSocket->close();
        //m_deviceSocket->deleteLater();
    }

    m_serverProcess.kill();
    disableTunnelReverse();
    removeServer();
    m_serverSocket.close();
}

DeviceSocket *server::getDeviceSocket()
{
    return m_deviceSocket;
}

void server::onWorkProcessResult(AdbProcess::ADB_EXEC_RESULT processResult)
{
    if (sender() == &m_workProcess) {
        if (SSS_NULL != m_serverStartStep) {
            switch (m_serverStartStep) {
            case SSS_PUSH:
                if (AdbProcess::AER_SUCCESS_EXEC == processResult) {
                    m_serverCopiedToDevice = true;
                    m_serverStartStep = SSS_ENABLE_REVERSE;
                    startServerByStep();
                } else if (AdbProcess::AER_SUCCESS_START != processResult) {
                    qCritical("adb push failed");
                    m_serverStartStep = SSS_NULL;
                    emit serverStartResult(false);
                }
                break;
            case SSS_ENABLE_REVERSE:
                if (AdbProcess::AER_SUCCESS_EXEC == processResult) {
                    m_enableReverse = true;
                    m_serverStartStep = SSS_EXECUTE_SERVER;
                    startServerByStep();
                } else if (AdbProcess::AER_SUCCESS_START != processResult) {
                    qCritical("adb reverse failed");
                    m_serverStartStep = SSS_NULL;
                    // removeServer
                    removeServer();
                    emit serverStartResult(false);
                }
                break;
            default:
                break;
            }
        }
    }

    if (sender() == &m_serverProcess) {
        if (SSS_EXECUTE_SERVER == m_serverStartStep) {
            if (AdbProcess::AER_SUCCESS_START == processResult) {
                m_serverStartStep = SSS_RUNNING;
                emit serverStartResult(true);
            } else if (AdbProcess::AER_ERROR_START == processResult) {
                // disable reverse
                disableTunnelReverse();
                qCritical("adb shell start server failed");
                m_serverStartStep = SSS_NULL;
                // removeServer
                removeServer();
                emit serverStartResult(false);
            }
        } else if (SSS_RUNNING == m_serverStartStep) {
            m_serverStartStep = SSS_NULL;
            emit onServerStop();
        }
    }
}

bool server::startServerByStep()
{
    bool stepSuccess = false;
    // push, enable reverse, execute server
    if (SSS_NULL != m_serverStartStep) {
        switch (m_serverStartStep) {
        case SSS_PUSH:
            stepSuccess = pushServer();
            break;
        case SSS_ENABLE_REVERSE:
            stepSuccess = enableTunnelReverse();
            break;
        case SSS_EXECUTE_SERVER:
            m_serverSocket.setMaxPendingConnections(1);
            if (!m_serverSocket.listen(QHostAddress::LocalHost, m_localPort)) {
                qCritical(QString("Could not listen on port %1").arg(m_localPort).toStdString().c_str());
                m_serverStartStep = SSS_NULL;
                disableTunnelReverse();
                removeServer();
                emit serverStartResult(false);
                return false;
            }
            stepSuccess = execute();
            break;
        default:
            break;
        }
    }

    if (!stepSuccess) {
        emit serverStartResult(false);
    }

    return stepSuccess;
}

bool server::pushServer()
{
    m_workProcess.push(m_serial, getServerPath(), DEVICE_SERVER_PATH);
    return true;
}

bool server::removeServer()
{
    if (!m_serverCopiedToDevice) {
        return true;
    }
    m_serverCopiedToDevice = false;

    AdbProcess* adb = new AdbProcess();
    if (!adb) {
        return false;
    }
    connect(adb, &AdbProcess::adbProcessResult, this, [this](AdbProcess::ADB_EXEC_RESULT processResult){
        if (AdbProcess::AER_SUCCESS_START != processResult) {
            sender()->deleteLater();
        }
    });
    adb->removePath(m_serial, DEVICE_SERVER_PATH);
    return true;
}

bool server::enableTunnelReverse()
{
    m_workProcess.reverse(m_serial, SOCKET_NAME, m_localPort);
    return true;
}

bool server::disableTunnelReverse()
{
    if (!m_enableReverse) {
        return true;
    }
    m_enableReverse = false;

    AdbProcess* adb = new AdbProcess();
    if (!adb) {
        return false;
    }
    connect(adb, &AdbProcess::adbProcessResult, this, [this](AdbProcess::ADB_EXEC_RESULT processResult){
        if (AdbProcess::AER_SUCCESS_START != processResult) {
            sender()->deleteLater();
        }
    });
    adb->reverseRemove(m_serial, SOCKET_NAME);
    return true;
}

bool server::execute()
{
    // adb shell CLASSPATH=/data/local/tmp/scrcpy-server.jar app_process / com.genymobile.scrcpy.Server 1080 2000000 false ""
    QStringList args;
    args << "shell";
    args << QString("CLASSPATH=%1").arg(DEVICE_SERVER_PATH);
    args << "app_process";
    args << "/";
    args << "com.genymobile.scrcpy.Server";
    args << QString::number(m_maxSize);
    args << QString::number(m_bitRate);
    args << "false";
    args << "";

    m_serverProcess.execute(m_serial, args);
    return true;
}

QString server::getServerPath()
{
    if (m_serverPath.isEmpty()) {
        m_serverPath = QString::fromLocal8Bit(qgetenv("QTSCRCPY_SERVER_PATH"));
        QFileInfo fileInfo(m_serverPath);
        if (m_serverPath.isEmpty() || !fileInfo.isFile()) {
            m_serverPath = QCoreApplication::applicationDirPath() + "/scrcpy-server.jar";
        }
    }
    return m_serverPath;
}

bool server::readInfo(QString &deviceName, QSize &size)
{
    // abk001-----------------------0x0438 0x02d0
    //               64b            2b w   2b h
    unsigned char buf[DEVICE_NAME_FIELD_LENGTH + 4];
    if (m_deviceSocket->bytesAvailable() <= (DEVICE_NAME_FIELD_LENGTH + 4)) {
        m_deviceSocket->waitForReadyRead(300);
    }

    qint64 len = m_deviceSocket->read((char*)buf, sizeof(buf));
    if (len < DEVICE_NAME_FIELD_LENGTH + 4) {
        qInfo("Could not retrieve device information");
        return false;
    }
    buf[DEVICE_NAME_FIELD_LENGTH - 1] = '\0';
    deviceName = (char*)buf;
    size.setWidth((buf[DEVICE_NAME_FIELD_LENGTH] << 8) | buf[DEVICE_NAME_FIELD_LENGTH + 1]);
    size.setHeight((buf[DEVICE_NAME_FIELD_LENGTH + 2] << 8) | buf[DEVICE_NAME_FIELD_LENGTH + 3]);
    return true;
}
