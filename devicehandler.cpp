#include <QDebug>
#include <QtEndian>
#include <QDateTime>

#include <devicehandler.h>
#include <deviceinfo.h>

DeviceHandler::DeviceHandler()
{
}

void DeviceHandler::setDevice(DeviceInfo *device)
{
    m_currentDevice = device;

    // Disconnect and delete old connection
    if (m_control) {
        m_control->disconnectFromDevice();
        delete m_control;
        m_control = nullptr;
    }

    // Create new controller and connect it if device available
    if (m_currentDevice) {
        m_control = QLowEnergyController::createCentral(m_currentDevice->getDevice(), this);
        connect(m_control, &QLowEnergyController::serviceDiscovered,
                this, &DeviceHandler::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished,
                this, &DeviceHandler::serviceScanDone);

        connect(m_control, static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, [this](QLowEnergyController::Error error) {
            Q_UNUSED(error);
            qWarning() << "Cannot connect to remote device.";
        });
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            qInfo() << "Controller connected. Search services...";
            m_control->discoverServices();
        });
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            qWarning() << "LowEnergy controller disconnected";
        });

        m_control->connectToDevice();
    }
}

void DeviceHandler::serviceDiscovered(const QBluetoothUuid &gatt)
{
    if (gatt ==  QBluetoothUuid(QBluetoothUuid::BloodPressure)) {
        qDebug() << "Blood pressure service discovered. Waiting for service scan to be done...";
        m_foundBloodPressureService = true;
    }
}

void DeviceHandler::serviceScanDone()
{
    // Delete old service if available
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
        qWarning() << "Service not found.";
    }
}

void DeviceHandler::serviceStateChanged(QLowEnergyService::ServiceState s)
{
    switch (s) {
    case QLowEnergyService::DiscoveringServices:
        qDebug() << "Discovering services...";
        break;
    case QLowEnergyService::ServiceDiscovered:
    {
        qDebug() << "Service discovered.";

        const QLowEnergyCharacteristic timeChar = m_service->characteristic(QBluetoothUuid(QBluetoothUuid::DateTime));
        const QLowEnergyCharacteristic hrChar = m_service->characteristic(QBluetoothUuid(QBluetoothUuid::BloodPressureMeasurement));

        if (!hrChar.isValid()) {
            qWarning() << "HR Data not found.";
            break;
        }

        QDateTime currentDateTime = QDateTime::currentDateTime();
        QByteArray dateTimeBytes;

        dateTimeBytes.append(currentDateTime.toString(Qt::ISODate).toUtf8());
        m_service->writeCharacteristic(timeChar, dateTimeBytes);

        m_notificationDesc = hrChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
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

    int sysValue = 0;
    int diaValue = 0;
    int pulValue = 0;

    if (flags & 0x1) {
        sysValue = static_cast<int>(qFromLittleEndian<quint16>(data[1]));
        diaValue = static_cast<int>(qFromLittleEndian<quint16>(data[3]));
        pulValue = static_cast<int>(qFromLittleEndian<quint16>(data[7]));
    } else {
        sysValue = static_cast<int>(data[1]);
        diaValue = static_cast<int>(data[3]);
        pulValue = static_cast<int>(data[7]);
    }

    qDebug() << "SYS: " << sysValue;
    qDebug() << "DIA: " << diaValue;
    qDebug() << "PUL" << pulValue;
    qDebug() << "Date and time: " << currentDateTime;
}

void DeviceHandler::confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value)
{
    if (d.isValid() && d == m_notificationDesc && value == QByteArray::fromHex("0000")) {
        //disabled notifications -> assume disconnect intent
        m_control->disconnectFromDevice();
        delete m_service;
        m_service = nullptr;
    }
}
