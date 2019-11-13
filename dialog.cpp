#include <QDebug>
#include <QDir>
#include <QBuffer>
#include <QTimer>

#include "dialog.h"
#include "ui_dialog.h"
Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    connect(&m_server,&server::serverStartResult,this,[this](bool success){
        qDebug() << "server start" << success;
    });

    connect(&m_server,&server::connectToResult,this,[this](bool success, const QString& deviceName, const QSize& size){
        qDebug() << "connectToResult start" << success << deviceName <<size;

        if(success){
            m_decoder.setDeviceSocket(m_server.getDeviceSocket());
            m_decoder.startDecode();
        }
    });

    m_frames.init();

    m_decoder.setFrames(&m_frames);

    connect(&m_decoder,&Decoder::onNewFrame,this,[this]{
           qDebug() << "Decoder::onNewFrame";
           m_frames.lock();
           const AVFrame * frame = m_frames.consumeRenderedFrame();
           //渲染frame
           m_videoWidget->setFrameSize(QSize(frame->width,frame->height));
           m_videoWidget->updateTextures(frame->data[0],frame->data[1],frame->data[2],frame->linesize[0],frame->linesize[1],frame->linesize[2]);

           m_frames.unLock();
    });

    m_videoWidget = new QYUVOpenGLWidget(Q_NULLPTR);
    m_videoWidget->resize(320,480);

}

Dialog::~Dialog()
{
    m_frames.deInit();
    delete ui;
}

void Dialog::on_deivceBut_clicked()
{

    m_adb.execute("", QStringList() << "devices");

}

void Dialog::on_startBtn_clicked()
{
    QTimer::singleShot(0, this, [this](){
        m_server.start("", 27183, 720, 8000000);
    });
    m_videoWidget->show();
}

void Dialog::on_stopBtn_clicked()
{
    m_server.stop();
    m_videoWidget->close()

}
