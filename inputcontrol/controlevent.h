#ifndef CONTROLEVENT_H
#define CONTROLEVENT_H

#include <QRect>
#include <QBuffer>

#include "qscrcpyevent.h"
#include "input.h"
#include "keycodes.h"


class ControlEvent :public QScrcpyEvent
{

public:
    enum ControlEventType {
        CET_KEYCODE = 0,
        CET_TEXT,
        CET_MOUSE,
        CET_SCROLL,
        CET_COMMAND,
        CET_TOUCH,
    };

//    ControlEvent();

    ControlEvent(ControlEventType type);


   void setMouseEventData(AndroidMotioneventAction action,AndroidMotioneventButtons buttons,QRect position);

   QByteArray serializeData();

protected:
    void write32(QBuffer &buffer,quint32 value);
    void write16(QBuffer &buffer,quint32 value);
    void writePosition(QBuffer &buffer,QRect &value);

private:
    struct ControlEventData{
        ControlEventType type;
        union{
            struct
            {
                AndroidMotioneventAction action;
                AndroidMotioneventButtons buttons;
                QRect position;
            } mouseEvent;
        };

        ControlEventData(){}

        ~ControlEventData(){}
    };

    ControlEventData m_data;


};


#endif // CONTROLEVENT_H
