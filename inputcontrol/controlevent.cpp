#include "controlevent.h"

ControlEvent::ControlEvent(ControlEventType type)
        :QScrcpyEvent(Control)
{
    m_data.type = type;
}

void ControlEvent::setMouseEventData(AndroidMotioneventAction action, AndroidMotioneventButtons buttons, QRect position)
{
    m_data.mouseEvent.action = action;
    m_data.mouseEvent.buttons = buttons;
    m_data.mouseEvent.position = position;
}


QByteArray ControlEvent::serializeData()
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QBuffer::WriteOnly);
    //够着控制指令
    //type  mouse控制指令
    buffer.putChar(m_data.type);
    switch (m_data.type) {
    case CET_MOUSE:
        buffer.putChar(m_data.mouseEvent.action);
        write32(buffer,m_data.mouseEvent.buttons);
        writePosition(buffer,m_data.mouseEvent.position);
        break;
    default:
        break;
    }

    //构造控制指令
    buffer.close();

    return byteArray;
}

void ControlEvent::write32(QBuffer &buffer, quint32 value)
{
    buffer.putChar(value >> 24); //放入00
    buffer.putChar(value >> 16); //放入11
    buffer.putChar(value >> 8);  //放入22
    buffer.putChar(value);      //放入33
}

void ControlEvent::write16(QBuffer &buffer, quint32 value)
{
      buffer.putChar(value >> 8); //放入00
      buffer.putChar(value);      //放入33
}

void ControlEvent::writePosition(QBuffer &buffer, QRect &value)
{
    write16(buffer,value.left());
    write16(buffer,value.top());
    write16(buffer,value.width());
    write16(buffer,value.height());
}
