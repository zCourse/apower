#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QPointer>


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

private slots:
    void on_deivceBut_clicked();

    void on_startBtn_clicked();

    void on_stopBtn_clicked();

    void on_test_clicked();

private:
    Ui::Dialog *ui;

    QPointer<VideoForm> m_videoForm;




};

#endif // DIALOG_H
