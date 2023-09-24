#include <QDebug>

#include <deviceinfo.h>
#include <devicefinder.h>

DeviceFinder::DeviceFinder(DeviceHandler *handler) : m_deviceHandler(handler)
{
    discoveryAgent = new QBluetoothDeviceDiscoveryAgent();
    discoveryAgent->setLowEnergyDiscoveryTimeout(5000);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &DeviceFinder::addDevice);
    connect(discoveryAgent, QOverload<QBluetoothDeviceDiscoveryAgent::Error>::of(&QBluetoothDeviceDiscoveryAgent::error),
            this, &DeviceFinder::scanError);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &DeviceFinder::scanFinished);
}

DeviceFinder::~DeviceFinder()
{
}

void DeviceFinder::startDeviceDiscovery()
{
    discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

void DeviceFinder::addDevice(const QBluetoothDeviceInfo &device)
{
    if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        DeviceInfo *currentDevice = new DeviceInfo(device);
        m_deviceHandler->setDevice(currentDevice);
    }
}

void DeviceFinder::scanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    qDebug() << "Scan error";
}

void DeviceFinder::scanFinished()
{
    qDebug() << "Scan finished";
}
