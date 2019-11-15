#include <QDebug>
#include <QDir>
#include <QBuffer>
#include <QTimer>
#include "input.h"
#include "keycodes.h"
#include "dialog.h"
#include "ui_dialog.h"
#include "controlevent.h"
#include "videoform.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

        connect(&m_adb, &AdbProcess::adbProcessResult, this, [this](AdbProcess::ADB_EXEC_RESULT processResult){
            QString log = "";
            bool newLine = true;
            switch (processResult) {
            case AdbProcess::AER_SUCCESS_START:
                log = "adb run";
                newLine = false;
                break;
            case AdbProcess::AER_ERROR_EXEC:
                log = m_adb.getErrorOut();
                break;
            case AdbProcess::AER_ERROR_MISSING_BINARY:
                log = "adb not find";
                break;
            case AdbProcess::AER_SUCCESS_EXEC:
                QStringList args = m_adb.arguments();
                if (args.contains("devices")) {
                    QStringList devices = m_adb.getDevicesSerialFromStdOut();
                    ui->serialBox->clear();
                    for (auto& item : devices) {
                        ui->serialBox->addItem(item);
                    }
                }else if (args.contains("show") && args.contains("wlan0")) {
                    QString ip = m_adb.getDeviceIPFromStdOut();
                    if (!ip.isEmpty()) {
                        ui->deviceIpEdt->setText(ip);
                    }
                } else if (args.contains("ifconfig") && args.contains("wlan0")) {
                    QString ip = m_adb.getDeviceIPFromStdOut();
                    if (!ip.isEmpty()) {
                        ui->deviceIpEdt->setText(ip);
                    }
                }
                break;
            }

            if (!log.isEmpty()) {
                outLog(log, newLine);
            }
        });

}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::outLog(const QString &log, bool newLine)
{
    QString tmpLog = log;
    bool tmpNewLine = newLine;
    QTimer::singleShot(0, this, [this, tmpLog, tmpNewLine](){
        ui->outEdt->append(tmpLog);
        if (tmpNewLine) {
            ui->outEdt->append("<br/>");
        }
    });
}

void Dialog::on_startServerBtn_clicked()
{
    if (!m_videoForm) {
        m_videoForm = new VideoForm(ui->serialBox->currentText().trimmed());
    }
    m_videoForm->show();

//    QTimer::singleShot(0, this, [this](){
//        m_server.start("", 27183, 720, 8000000);
//    });
}

void Dialog::on_stopServerBtn_clicked()
{
    if (m_videoForm) {
        m_videoForm->close();
    }
}


void Dialog::on_updateDevice_clicked()
{
    if (checkAdbRun()) {
        return;
    }
    outLog("update devices...", false);
    m_adb.execute("", QStringList() << "devices");
}

void Dialog::on_startAdbdBtn_clicked()
{
    outLog("start devices adbd...", false);
    // adb tcpip 5555
    QStringList adbArgs;
    adbArgs << "tcpip";
    adbArgs << "5555";
    m_adb.execute(ui->serialBox->currentText().trimmed(), adbArgs);
}

void Dialog::on_getIPBtn_clicked()
{
    if (checkAdbRun()) {
        return;
    }

    outLog("get ip...", false);
    // adb -s P7C0218510000537 shell ifconfig wlan0
    // or
    // adb -s P7C0218510000537 shell ip -f inet addr show wlan0
    QStringList adbArgs;
#if 0
    adbArgs << "shell";
    adbArgs << "ip";
    adbArgs << "-f";
    adbArgs << "inet";
    adbArgs << "addr";
    adbArgs << "show";
    adbArgs << "wlan0";
#else
    adbArgs << "shell";
    adbArgs << "ifconfig";
    adbArgs << "wlan0";
#endif
    m_adb.execute(ui->serialBox->currentText().trimmed(), adbArgs);
}

void Dialog::on_wirelessConnectBtn_clicked()
{
    outLog("wireless connect...", false);
    QString addr = ui->deviceIpEdt->text().trimmed();
    if (!ui->devicePortEdt->text().isEmpty()) {
        addr += ":";
        addr += ui->devicePortEdt->text().trimmed();
    }
    // connect ip:port
    QStringList adbArgs;
    adbArgs << "connect";
    adbArgs << addr;
    m_adb.execute(ui->serialBox->currentText().trimmed(), adbArgs);
}


bool Dialog::checkAdbRun()
{
    if (m_adb.isRuning()) {
        outLog("wait for the end of the current command to run");
    }
    return m_adb.isRuning();
}

void Dialog::on_wirelessDisConnectBtn_clicked()
{
    if (checkAdbRun()) {
        return;
    }
    QString addr = ui->deviceIpEdt->text().trimmed();
    outLog("wireless disconnect...", false);
    QStringList adbArgs;
    adbArgs << "disconnect";
    adbArgs << addr;
    m_adb.execute("", adbArgs);

}
