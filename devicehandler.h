#ifndef DEVICEHANDLER_H
#define DEVICEHANDLER_H

#include <QLowEnergyController>
#include <QObject>

#include <deviceinfo.h>

class DeviceHandler : public QObject
{
public:
    DeviceHandler();

    void setDevice(DeviceInfo *device);

private:
    //QLowEnergyController
    void serviceDiscovered(const QBluetoothUuid &);
    void serviceScanDone();

    //QLowEnergyService
    void serviceStateChanged(QLowEnergyService::ServiceState s);
    void updateBloodPressureValue(const QLowEnergyCharacteristic &c,
                              const QByteArray &value);
    void confirmedDescriptorWrite(const QLowEnergyDescriptor &d,
                              const QByteArray &value);

    bool m_foundBloodPressureService;

    QLowEnergyDescriptor m_notificationDesc;
    DeviceInfo *m_currentDevice = nullptr;
    QLowEnergyController *m_control = nullptr;
    QLowEnergyService *m_service = nullptr;
};

#endif // DEVICEHANDLER_H
