#ifndef SERVER_H
#define SERVER_H
#include <QObject>
#include <QSize>

#include "adbprocess.h"
#include "devicesocket.h"
#include "tcpserver.h"

class server : public QObject
{
    Q_OBJECT

    enum SERVER_START_STEP {
        SSS_NULL,
        SSS_PUSH,
        SSS_ENABLE_REVERSE,
        SSS_EXECUTE_SERVER,
        SSS_RUNNING,
    };

public:
    server(QObject *parent=Q_NULLPTR);

    bool start(const QString& serial, quint16 localPort, quint16 maxSize, quint32 bitRate);
    void stop();
    DeviceSocket* getDeviceSocket();
signals:
    void serverStartResult(bool success);
    void connectToResult(bool success, const QString& deviceName, const QSize& size);
    void onServerStop();

private slots:
    void onWorkProcessResult(AdbProcess::ADB_EXEC_RESULT processResult);

private:
    bool startServerByStep();
    bool pushServer();
    bool removeServer();
    bool enableTunnelReverse();
    bool disableTunnelReverse();
    bool execute();
    QString getServerPath();
    bool readInfo(QString& deviceName, QSize& size);

private:
    QString m_serial = "";
    quint16 m_localPort = 0;
    quint16 m_maxSize = 0;
    quint32 m_bitRate = 0;

    SERVER_START_STEP m_serverStartStep = SSS_NULL;

    AdbProcess m_workProcess;
    AdbProcess m_serverProcess;

    QString m_serverPath = "";
    bool m_serverCopiedToDevice = false;
    bool m_enableReverse = false;

    TcpServer m_serverSocket;
    DeviceSocket* m_deviceSocket = Q_NULLPTR;
};

#endif // SERVER_H
