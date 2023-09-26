#ifndef DEVICEHANDLER_H
#define DEVICEHANDLER_H

#include <QLowEnergyController>
#include <QObject>

#include <deviceinfo.h>
#include <bluetoothbaseclass.h>

class DeviceHandler : public BluetoothBaseClass
{
public:
    DeviceHandler(QObject *parent = nullptr);

    void setDevice(DeviceInfo *device);

    // not used
    void disconnectService();

private:
    void serviceDiscovered(const QBluetoothUuid &);
    void serviceScanDone();

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


    int m_sys = 0;
    int m_dia = 0;
    int m_pul = 0;
};

#endif // DEVICEHANDLER_H
