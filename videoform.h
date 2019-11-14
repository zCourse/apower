#ifndef VIDEOFORM_H
#define VIDEOFORM_H

#include <QWidget>
#include <QTimer>


#include "adbprocess.h"
#include "server.h"
#include "frames.h"
#include "decoder.h"
#include "controller.h"
#include "qyuvopenglwidget.h"
#include "inputconvertnormal.h"
#include "inputconvertgame.h"
namespace Ui {
class VideoForm;
}

class VideoForm : public QWidget
{
    Q_OBJECT

public:
    explicit VideoForm(const QString& serial,QWidget *parent = 0);
    ~VideoForm();


protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

    virtual void wheelEvent(QWheelEvent *event);

    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

private:
    void updateShowSize(const QSize &newSize);

private:
    Ui::VideoForm *ui;
    QString m_serial = "";

    AdbProcess m_adb;

    server m_server;

    Frames m_frames;

    Decoder m_decoder;

    Controller m_controller;

    //InputConvertNormal m_inputConvert;
    InputConvertGame m_inputConvert;

    QSize m_frameSize;
};

#endif // VIDEOFORM_H
