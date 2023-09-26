#include <QtEndian>
#include <QDateTime>

#include <devicehandler.h>
#include <deviceinfo.h>

DeviceHandler::DeviceHandler(QObject *parent) :
    BluetoothBaseClass(parent),
    m_foundBloodPressureService(false)
{
}

void DeviceHandler::setDevice(DeviceInfo *device)
{
    m_currentDevice = device;

    if (m_control) {
        m_control->disconnectFromDevice();
        delete m_control;
        m_control = nullptr;
    }

    if (m_currentDevice) {
        m_control = QLowEnergyController::createCentral(m_currentDevice->getDevice(), this);
        connect(m_control, &QLowEnergyController::serviceDiscovered,
                this, &DeviceHandler::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished,
                this, &DeviceHandler::serviceScanDone);

        connect(m_control, static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, [this](QLowEnergyController::Error error) {
            Q_UNUSED(error);
            setError("Cannot connect to remote device.");
        });
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            setInfo("Controller connected. Search services...");
            m_control->discoverServices();
        });
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            setError("LowEnergy controller disconnected");
        });

        m_control->connectToDevice();
    }
}

void DeviceHandler::serviceDiscovered(const QBluetoothUuid &gatt)
{
    if (gatt ==  QBluetoothUuid(QBluetoothUuid::BloodPressure)) {
         setInfo("Blood pressure service discovered. Waiting for service scan to be done...");
        m_foundBloodPressureService = true;
    }
}

void DeviceHandler::serviceScanDone()
{
    if (m_service) {
        delete m_service;
        m_service = nullptr;
    }

    if (m_foundBloodPressureService)
        m_service = m_control->createServiceObject(QBluetoothUuid(QBluetoothUuid::BloodPressure), this);

    if (m_service) {
        connect(m_service, &QLowEnergyService::stateChanged, this, &DeviceHandler::serviceStateChanged);
        connect(m_service, &QLowEnergyService::characteristicChanged, this, &DeviceHandler::updateBloodPressureValue);
        connect(m_service, &QLowEnergyService::descriptorWritten, this, &DeviceHandler::confirmedDescriptorWrite);
        m_service->discoverDetails();
    } else {
        setError("Blood pressure service not found.");
    }
}

void DeviceHandler::serviceStateChanged(QLowEnergyService::ServiceState s)
{
    switch (s) {
    case QLowEnergyService::DiscoveringServices:
        setInfo(tr("Discovering services..."));
        break;
    case QLowEnergyService::ServiceDiscovered:
    {
        setInfo(tr("Service discovered."));

        const QLowEnergyCharacteristic timeChar = m_service->characteristic(QBluetoothUuid(QBluetoothUuid::DateTime));
        const QLowEnergyCharacteristic bpChar = m_service->characteristic(QBluetoothUuid(QBluetoothUuid::BloodPressureMeasurement));

        if (!bpChar.isValid()) {
            setError("Blood pressure measurement data not found.");
            break;
        }

        QDateTime currentDateTime = QDateTime::currentDateTime();
        QByteArray dateTimeBytes;

        dateTimeBytes.append(currentDateTime.toString(Qt::ISODate).toUtf8());
        m_service->writeCharacteristic(timeChar, dateTimeBytes);

        m_notificationDesc = bpChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
        if (m_notificationDesc.isValid())
            m_service->writeDescriptor(m_notificationDesc, "10");

        break;
    }
    default:
        break;
    }
}

void DeviceHandler::updateBloodPressureValue(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    if (c.uuid() != QBluetoothUuid(QBluetoothUuid::BloodPressureMeasurement))
        return;

    QString currentDateTime = QDateTime::currentDateTime().toTimeSpec(Qt::LocalTime).toString(Qt::ISODate);

    auto data = reinterpret_cast<const quint8 *>(value.constData());
    quint8 flags = *data;

    if (flags & 0x1) {
        m_sys = static_cast<int>(qFromLittleEndian<quint16>(data[1]));
        m_dia = static_cast<int>(qFromLittleEndian<quint16>(data[3]));
        m_pul = static_cast<int>(qFromLittleEndian<quint16>(data[7]));
    } else {
        m_sys = static_cast<int>(data[1]);
        m_dia = static_cast<int>(data[3]);
        m_pul = static_cast<int>(data[7]);
    }

    setInfo("SYS: " + QString::number(m_sys));
    setInfo("DIA: " + QString::number(m_dia));
    setInfo("PUL: " + QString::number(m_pul));
    setInfo("Date and time: " + currentDateTime);
}

void DeviceHandler::disconnectService()
{
    m_foundBloodPressureService = false;

    if (m_notificationDesc.isValid() && m_service
            && m_notificationDesc.value() == QByteArray::fromHex("10")) {
        m_service->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0000"));
    } else {
        if (m_control)
            m_control->disconnectFromDevice();

        delete m_service;
        m_service = nullptr;
    }
}

void DeviceHandler::confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value)
{
    if (d.isValid() && d == m_notificationDesc && value == QByteArray::fromHex("0000")) {
        m_control->disconnectFromDevice();
        delete m_service;
        m_service = nullptr;
    }
}
