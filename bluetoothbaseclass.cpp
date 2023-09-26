#include <QDebug>

#include <bluetoothbaseclass.h>

BluetoothBaseClass::BluetoothBaseClass(QObject *parent) : QObject(parent)
{
}

QString BluetoothBaseClass::error() const
{
    return m_error;
}

QString BluetoothBaseClass::info() const
{
    return m_info;
}

void BluetoothBaseClass::setError(const QString &error)
{
    if (m_error != error) {
        m_error = error;
        emit errorChanged();
    }
    if (m_error != "") {
        qDebug() << m_error;
    }
}

void BluetoothBaseClass::setInfo(const QString &info)
{
    if (m_info != info) {
        m_info = info;
        emit infoChanged();
    }
    if (m_info != "") {
        qDebug() << m_info;
    }
}

void BluetoothBaseClass::clearMessages()
{
    setInfo("");
    setError("");
}
