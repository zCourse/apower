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



}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_deivceBut_clicked()
{

    //m_adb.execute("", QStringList() << "devices");

}

void Dialog::on_startBtn_clicked()
{
    if(!m_videoForm){
        m_videoForm = new VideoForm("");
    }
    m_videoForm->show();

//    QTimer::singleShot(0, this, [this](){
//        m_server.start("", 27183, 720, 8000000);
//    });
//    m_videoWidget->show();
}

void Dialog::on_stopBtn_clicked()
{
    if(m_videoForm){
        m_videoForm->close();
    }
}

void Dialog::on_test_clicked()
{

}
