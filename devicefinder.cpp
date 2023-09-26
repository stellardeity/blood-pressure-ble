#include <deviceinfo.h>
#include <devicefinder.h>

DeviceFinder::DeviceFinder(DeviceHandler *handler, QObject *parent):
    BluetoothBaseClass(parent),
    m_deviceHandler(handler)
{
    m_deviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent();
    m_deviceDiscoveryAgent->setLowEnergyDiscoveryTimeout(5000);
    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &DeviceFinder::addDevice);
    connect(m_deviceDiscoveryAgent, QOverload<QBluetoothDeviceDiscoveryAgent::Error>::of(&QBluetoothDeviceDiscoveryAgent::error),
            this, &DeviceFinder::scanError);
    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &DeviceFinder::scanFinished);
}

void DeviceFinder::startDeviceDiscovery()
{
    clearMessages();
    m_deviceHandler->setDevice(nullptr);

    m_deviceDiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    setInfo(tr("Scanning for devices..."));
}

void DeviceFinder::addDevice(const QBluetoothDeviceInfo &device)
{
    if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        m_currentDevice = new DeviceInfo(device);
        m_deviceHandler->setDevice(m_currentDevice);
    }
}

void DeviceFinder::scanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    if (error == QBluetoothDeviceDiscoveryAgent::PoweredOffError)
        setError(tr("The Bluetooth adaptor is powered off."));
    else if (error == QBluetoothDeviceDiscoveryAgent::InputOutputError)
        setError(tr("Writing or reading from the device resulted in an error."));
    else
        setError(tr("An unknown error has occurred."));
}

void DeviceFinder::scanFinished()
{
    setInfo(tr("Scanning done."));
}

