#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QObject>
#include <QBluetoothDeviceInfo>

class DeviceInfo: public QObject
{
public:
    DeviceInfo(const QBluetoothDeviceInfo &device);

    QString getName() const;
    QString getAddress() const;
    QBluetoothDeviceInfo getDevice() const;

private:
    QBluetoothDeviceInfo m_device;
};

#endif // DEVICEINFO_H
