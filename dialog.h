#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QPointer>

#include "adbprocess.h"
#include "server.h"
#include "frames.h"
#include "decoder.h"
#include "qyuvopenglwidget.h"
#include "controller.h"

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private slots:
    void on_deivceBut_clicked();

    void on_startBtn_clicked();

    void on_stopBtn_clicked();

    void on_test_clicked();

private:
    Ui::Dialog *ui;

    AdbProcess m_adb;

    server m_server;

    Frames m_frames;

    Decoder m_decoder;

    Controller m_controller;

    QYUVOpenGLWidget * m_videoWidget;
};

#endif // DIALOG_H
