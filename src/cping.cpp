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
    ipAddresses.append(ipAddress);
    init();
}

CPing::CPing(QVector<QString> ipAddresses, QObject *parent) : QObject(parent) {
    for (QString oneIp : ipAddresses)
        this->ipAddresses.append(oneIp);
    init();
}

CPing::~CPing() {
//    thread.quit();
//    thread.wait();
//TODO: add normal quit threadPool

    delete ping;
}

void CPing::init() {

    for(int i = 0; i < ipAddresses.count(); i++)
#ifdef _WIN32
        pings.append(new CPingWindows);
#elif __linux__
        pings.append(new CPingLinux);
#endif

    qRegisterMetaType<QVector<QString>>("QVector<QString>");
    qRegisterMetaType<ICPingOS::CPingResponse>("ICPingOS::CPingResponse");
    qRegisterMetaType<QVector<ICPingOS::CPingResponse>>("QVector<ICPingOS::CPingResponse>");

    for (ICPingOS *p : pings) {
        p->setAutoDelete(false);
        connect(p, &ICPingOS::responsePingAllIpAsync, this, &CPing::responsePingAllIpAsync);
        connect(p, &ICPingOS::responsePingOneIpAsync, this, &CPing::responsePingOneIpAsync);
    }

#ifdef _WIN32
    ping = new CPingWindows();
#elif __linux__
    ping = new CPingLinux();
#endif

    connect(&timerPingOneIp, SIGNAL(timeout()), this, SLOT(pingOneIpByTimer()));
    connect(&timerPingAllIp, SIGNAL(timeout()), this, SLOT(pingAllIpByTimer()));
    connect(&timerPingAllIpAsync, SIGNAL(timeout()), this, SLOT(pingAllIpByTimerAsync()));
    connect(&timerPingOneIpAsync, SIGNAL(timeout()), this, SLOT(pingOneIpByTimerAsync()));
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

void CPing::pingAllIpAsync(unsigned int threads) {
    if (threads > 0) {
        threadPool.setMaxThreadCount(threads);

        int ipCountForDefThread = ipAddresses.count() / threads;

        qDebug() << threadPool.activeThreadCount();

        for(int i = 0; i < threads; i++) {
            QVector<QString> ip;
            int additionalIpCount = ipAddresses.count() % threads;
            if (i == 0)
                ip << ipAddresses.mid(i, ipCountForDefThread + additionalIpCount);
            else
                ip << ipAddresses.mid(i + additionalIpCount, ipCountForDefThread);

            pings.at(i)->setIpForAsyncPing(ip);
            threadPool.start(pings.at(i));
        }
    }
}

void CPing::pingOneIpAsync(int index) {
    if (ipAddresses.length() > index) {
        ping->setIpForAsyncPing({ipAddresses.at(index)});
        threadPool.start(ping);
    }
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
    pingOneIpAsync(indexIpAdrForTimerPingOneIpAsync);

}

void CPing::pingAllIpByTimer() {
    emit responsePingAllIp(
                pingAllIp());
}

void CPing::pingAllIpByTimerAsync() {
    pingAllIpAsync(3);
}
