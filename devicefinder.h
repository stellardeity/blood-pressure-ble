#ifndef DEVICEFINDER_H
#define DEVICEFINDER_H

#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>

#include <devicehandler.h>

class DeviceFinder: public QObject
{
public:
    DeviceFinder(DeviceHandler *handler);
    ~DeviceFinder();
public slots:
    void startDeviceDiscovery();
private slots:
    void addDevice(const QBluetoothDeviceInfo&);
    void scanError(QBluetoothDeviceDiscoveryAgent::Error error);
    void scanFinished();
private:
    QBluetoothDeviceDiscoveryAgent *discoveryAgent;
    DeviceHandler *m_deviceHandler;
};

#endif // DEVICEFINDER_H
