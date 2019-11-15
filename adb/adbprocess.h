#ifndef ADBPROCESS_H
#define ADBPROCESS_H
#include <QProcess>

class AdbProcess : public QProcess
{
    Q_OBJECT
public:
    enum ADB_EXEC_RESULT {
        AER_SUCCESS_START,          // 启动成功
        AER_ERROR_START,            // 启动失败
        AER_SUCCESS_EXEC,           // 执行成功
        AER_ERROR_EXEC,             // 执行失败
        AER_ERROR_MISSING_BINARY,   // 找不到文件
    };

    explicit AdbProcess(QObject *parent = nullptr);
    virtual ~AdbProcess();

    void execute(const QString& serial, const QStringList& args);
    void push(const QString& serial, const QString& local, const QString& remote);
    void removePath(const QString& serial, const QString& path);
    void reverse(const QString& serial, const QString& deviceSocketName, quint16 localPort);
    void reverseRemove(const QString& serial, const QString& deviceSocketName);
    QStringList getDevicesSerialFromStdOut();
    QString getDeviceIPFromStdOut();
    QString getStdOut();
    QString getErrorOut();
    bool isRuning();
    static QString getAdbPath();

signals:
    void adbProcessResult(ADB_EXEC_RESULT processResult);

private:
    void initSignals();

    static QString s_adbPath;
    QString m_standardOutput = "";
    QString m_errorOutput = "";
};

#endif // ADBPROCESS_H
