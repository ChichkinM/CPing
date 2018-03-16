#include "cping.h"

#include <QDebug>
#include <QMetaType>

#ifdef _WIN32
#include "cpingwindows.h"
#elif __linux__
#include "cpinglinux.h"
#endif


CPing::CPing(QObject *parent) : QObject(parent) {
    init();
}

CPing::CPing(QString ipAddress, QObject *parent) : QObject(parent) {
    init();
    ipAddresses.append(ipAddress);
}

CPing::CPing(QVector<QString> ipAddresses, QObject *parent) : QObject(parent) {
    init();
    for (QString oneIp : ipAddresses)
        this->ipAddresses.append(oneIp);
}

CPing::~CPing() {
    thread.quit();
    thread.wait();

    delete ping;
}

void CPing::init() {
#ifdef _WIN32
    ping = new CPingWindows();
    pingAsync = new CPingWindows();
#elif __linux__
    ping = new CPingLinux();
    pingAsync = new CPingLinux();
#endif

    connect(&timerPingOneIp, SIGNAL(timeout()), this, SLOT(pingOneIpByTimer()));
    connect(&timerPingAllIp, SIGNAL(timeout()), this, SLOT(pingAllIpByTimer()));
    connect(&timerPingAllIpAsync, SIGNAL(timeout()), this, SLOT(pingAllIpByTimerAsync()));
    connect(&timerPingOneIpAsync, SIGNAL(timeout()), this, SLOT(pingOneIpByTimerAsync()));

    if (pingAsync != nullptr) {
        pingAsync->moveToThread(&thread);

        qRegisterMetaType<QVector<QString>>("QVector<QString>");
        qRegisterMetaType<ICPingOS::CPingResponse>("ICPingOS::CPingResponse");
        qRegisterMetaType<QVector<ICPingOS::CPingResponse>>("QVector<ICPingOS::CPingResponse>");

        connect(&thread, &QThread::finished, pingAsync, &QObject::deleteLater);
        connect(this, &CPing::pingAllIpAsyncStart, pingAsync, &ICPingOS::pingAllIpAsync);
        connect(this, &CPing::pingOneIpAsyncStart, pingAsync, &ICPingOS::pingOneIpAsync);
        connect(pingAsync, &ICPingOS::responsePingAllIpAsync, this, &CPing::responsePingAllIpAsync);
        connect(pingAsync, &ICPingOS::responsePingOneIpAsync, this, &CPing::responsePingOneIpAsync);
    }
}


QVector<ICPingOS::CPingResponse> CPing::pingAllIp() {
    QVector<ICPingOS::CPingResponse> result;

    if (ping != nullptr)
        result = ping->pingAllIp(ipAddresses);

    return result;
}

ICPingOS::CPingResponse CPing::pingOneIp(int index) {
    ICPingOS::CPingResponse result;

    if (ipAddresses.length() > index) {
        if (ping != nullptr)
            result = ping->pingOneIp(ipAddresses.at(index));
    }

    return result;
}

void CPing::pingAllIpAsync() {
    if (!thread.isRunning())
        thread.start();
    emit pingAllIpAsyncStart(ipAddresses);
}

void CPing::pingOneIpAsync(int index) {
    if (!thread.isRunning())
        thread.start();

    if (ipAddresses.length() > index)
        emit pingOneIpAsyncStart(ipAddresses.at(index));
}

void CPing::startPingOneIpByTimer(int interval, int index) {
    if (!timerPingOneIp.isActive() && ipAddresses.length() > index) {
        indexIpAdrForTimerPingOneIp = index;
        timerPingOneIp.start(interval);
    }
}

void CPing::startPingOneIpByTimerAsync(int interval, int index) {
    if (!timerPingOneIpAsync.isActive() && ipAddresses.length() > index) {
        timerPingOneIpAsync.start(interval);
        indexIpAdrForTimerPingOneIpAsync = index;
    }
}

void CPing::startPingAllIpByTimer(int interval) {
    if (!timerPingAllIp.isActive())
        timerPingAllIp.start(interval);
}

void CPing::startPingAllIpByTimerAsync(int interval) {
    if (!timerPingAllIpAsync.isActive())
        timerPingAllIpAsync.start(interval);
}

void CPing::pingOneIpByTimer() {
    emit responsePingOneIp(
                pingOneIp(indexIpAdrForTimerPingOneIp));
}

void CPing::pingOneIpByTimerAsync() {
    if (!thread.isRunning())
        thread.start();
    emit pingOneIpAsyncStart(ipAddresses.at(indexIpAdrForTimerPingOneIpAsync));
}

void CPing::pingAllIpByTimer() {
    emit responsePingAllIp(
                pingAllIp());
}

void CPing::pingAllIpByTimerAsync() {
    if (!thread.isRunning())
        thread.start();
    emit pingAllIpAsyncStart(ipAddresses);
}
