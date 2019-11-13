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
}

Dialog::~Dialog()
{
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
}

void Dialog::on_stopBtn_clicked()
{

}
