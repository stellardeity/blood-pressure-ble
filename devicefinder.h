#ifndef DEVICEFINDER_H
#define DEVICEFINDER_H

#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>

#include <devicehandler.h>
#include <bluetoothbaseclass.h>

class DeviceFinder: public BluetoothBaseClass
{
    Q_OBJECT
public:
    DeviceFinder(DeviceHandler *handler, QObject *parent = nullptr);

    void startDeviceDiscovery();

private:
    void addDevice(const QBluetoothDeviceInfo&);
    void scanError(QBluetoothDeviceDiscoveryAgent::Error error);
    void scanFinished();

    QBluetoothDeviceDiscoveryAgent *m_deviceDiscoveryAgent;
    DeviceHandler *m_deviceHandler;
    DeviceInfo *m_currentDevice;
};

#endif // DEVICEFINDER_H
