#include <QCoreApplication>

#include <devicefinder.h>
#include <devicehandler.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    DeviceHandler deviceHandler;
    DeviceFinder deviceFinder(&deviceHandler);

    deviceFinder.startDeviceDiscovery();

    return a.exec();
}
