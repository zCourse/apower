#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QPointer>

#include "adbprocess.h"
#include "server.h"
namespace Ui {
class Dialog;
}

class VideoForm;

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

    void outLog(const QString& log, bool newLine = true);

private slots:
    void on_startServerBtn_clicked();

    void on_stopServerBtn_clicked();

    void on_updateDevice_clicked();

    void on_startAdbdBtn_clicked();

    void on_getIPBtn_clicked();

    void on_wirelessConnectBtn_clicked();

    void on_wirelessDisConnectBtn_clicked();

    void on_getPicBtn_clicked();

private:
     bool checkAdbRun();

private:
    Ui::Dialog *ui;

    QPointer<VideoForm> m_videoForm;

    AdbProcess m_adb;


    QString  s_absImgFilePath ="";


};

#endif // DIALOG_H
