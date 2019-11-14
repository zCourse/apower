#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <QObject>
#include <QPointer>
#include <QByteArray>

class DeviceSocket;
class ControlEvent;

class Controller : public QObject
{
    Q_OBJECT
public:
    Controller(QObject * parent = Q_NULLPTR);

    void setDeviceSocket(DeviceSocket * deviceSocket);

    void postControlEvent(ControlEvent * controlEvent);

    void test(QRect rc);

protected:
    virtual bool event(QEvent * event);

private:
    bool sendControl(const QByteArray & buffer);

    QPointer<DeviceSocket> m_deviceSocket;

};

#endif // CONTROLLER_H
