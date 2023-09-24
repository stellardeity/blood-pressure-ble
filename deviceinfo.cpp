#include <QBluetoothAddress>

#include <deviceinfo.h>

DeviceInfo::DeviceInfo(const QBluetoothDeviceInfo &info): m_device(info)
{
}

QBluetoothDeviceInfo DeviceInfo::getDevice() const
{
    return m_device;
}

QString DeviceInfo::getName() const
{
    return m_device.name();
}

QString DeviceInfo::getAddress() const
{
    return m_device.address().toString();
}
