#include <QApplication>
#include <QDebug>
#include <QTcpSocket>
#include <QTcpServer>
#include <QTranslator>
#include <QFile>


#include "dialog.h"



QtMessageHandler g_oldMessageHandler = Q_NULLPTR;
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);


Dialog* g_mainDlg = Q_NULLPTR;
int main(int argc, char *argv[])
{
    g_oldMessageHandler = qInstallMessageHandler(myMessageOutput);
    //Stream::init();
    QApplication a(argc, argv);


    // windows下通过qmake VERSION变量或者rc设置版本号和应用名称后，这里可以直接拿到
    // mac下拿到的是CFBundleVersion的值
    qDebug() << a.applicationVersion();
    qDebug() << a.applicationName();





#ifdef Q_OS_WIN32
    qputenv("QTSCRCPY_ADB_PATH", "../../../../third_party/adb/win/adb.exe");
    qputenv("QTSCRCPY_SERVER_PATH", "../../../../third_party/scrcpy-server.jar");
    qputenv("QTSCRCPY_KEYMAP_PATH", "../../../../keymap");
#endif

#ifdef Q_OS_LINUX
    qputenv("QTSCRCPY_ADB_PATH", "../../../third_party/adb/linux/adb");
    qputenv("QTSCRCPY_SERVER_PATH", "../../../third_party/scrcpy-server.jar");
#endif

    g_mainDlg = new Dialog;
    g_mainDlg->show();

    return a.exec();
}


void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (g_oldMessageHandler) {
        g_oldMessageHandler(type, context, msg);
    }

    if (QtDebugMsg < type) {
        if (g_mainDlg && !msg.contains("app_proces")) {
            g_mainDlg->outLog(msg);
        }
    }
    if (QtFatalMsg == type) {
        //abort();
    }
}
