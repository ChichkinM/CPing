#include "cping.h"

#include <QDebug>
#include <QMetaType>

#ifdef _WIN32
#include "cpingwindows.h"
#elif __linux__
#include "cpinglinux.h"
#endif


CPing::CPing(QObject *parent) : QObject(parent) {
    init(0);
}

CPing::CPing(QString ipAddress, unsigned int threadsForAsync, QObject *parent) : QObject(parent) {
    ipAddresses.append(ipAddress);
    init(threadsForAsync);
}

CPing::CPing(QVector<QString> ipAddresses, unsigned int threadsForAsync, QObject *parent) : QObject(parent) {
    for (QString oneIp : ipAddresses)
        this->ipAddresses.append(oneIp);
    init(threadsForAsync);
}

CPing::~CPing() {
    delete threadPool;

    for(ICPingOS *p : pingsAsyncForPingAll) delete p;
    for(ICPingOS *p : pingsAsyncForPingOne) delete p;
    delete pingSync;
}

void CPing::threadPoolInit(unsigned int threads) {
    if (threadPool == nullptr && threads > 0) {
        threadPool = new QThreadPool;
        threadPool->setMaxThreadCount(threads);
    }
}

void CPing::init( unsigned int threadsForAsync) {
    threadPoolInit(threadsForAsync);

    for(int i = 0; i < ipAddresses.count(); i++) {
#ifdef _WIN32
        pingsAsyncForPingAll.append(new CPingWindows);
        pingsAsyncForPingOne.append(new CPingWindows);
        if (pingSync == nullptr)
            pingSync = new CPingWindows;
#elif __linux__
        pingsAsyncForPingAll.append(new CPingLinux);
        pingsAsyncForPingOne.append(new CPingLinux);
        if (pingSync == nullptr)
            pingSync = new CPingLinux;
#endif
    }

    qRegisterMetaType<QVector<QString>>("QVector<QString>");
    qRegisterMetaType<ICPingOS::CPingResponse>("ICPingOS::CPingResponse");
    qRegisterMetaType<QVector<ICPingOS::CPingResponse>>("QVector<ICPingOS::CPingResponse>");

    for (ICPingOS *p : pingsAsyncForPingAll) {
        p->setAutoDelete(false);
        connect(p, &ICPingOS::responsePingAllIpAsync, this, &CPing::responsePingAllIpAsyncAggregator);
    }

    int i = 0;
    for(ICPingOS *p : pingsAsyncForPingOne) {
        p->setAutoDelete(false);
        p->setIpForAsyncPing({ipAddresses.at(i)});
        connect(p, &ICPingOS::responsePingAllIpAsync, this, &CPing::responsePingAllIpAsync);
        i++;
    }

    connect(&timerPingAllIpAsync, &QTimer::timeout, this, [this](){ pingAllIpAsync(); });
    connect(&timerPingOneIpAsync, &QTimer::timeout, this, [this](){ pingOneIpAsync(indexIpAdrForTimerPingOneIpAsync); });

    connect(&timerPingOneIp, &QTimer::timeout, this, [this](){
        emit responsePingAllIp( {
                                    pingOneIp(indexIpAdrForTimerPingOneIp)}); });
    connect(&timerPingAllIp, &QTimer::timeout, this, [this](){
        emit responsePingAllIp(
                    pingAllIp()); });
}

void CPing::responsePingAllIpAsyncAggregator(QVector<ICPingOS::CPingResponse> result) {
    agregatorResult << result;
    if (agregatorResult.count() == ipAddresses.count())
        emit responsePingAllIpAsync(agregatorResult);
}



QVector<ICPingOS::CPingResponse> CPing::pingAllIp() {
    QVector<ICPingOS::CPingResponse> result;

    if (pingSync != nullptr)
        result = pingSync->pingAllIp(ipAddresses);

    return result;
}

ICPingOS::CPingResponse CPing::pingOneIp(int index) {
    ICPingOS::CPingResponse result;

    if (ipAddresses.length() > index) {
        if (pingSync != nullptr)
            result = pingSync->pingAllIp({ipAddresses.at(index)}).at(0);
    }

    return result;
}

void CPing::pingAllIpAsync() {
    threadPoolInit(ipAddresses.count());

    agregatorResult.clear();

    int threads = ipAddresses.count() <= threadPool->maxThreadCount() ?
                ipAddresses.count() : threadPool->maxThreadCount();
    int ipCountForDefThread = ipAddresses.count() / threads;

    for(unsigned int i = 0; i < threads; i++) {
        QVector<QString> ip;
        int additionalIpCount = ipAddresses.count() % threads;
        if (i == 0)
            ip << ipAddresses.mid(0, ipCountForDefThread + additionalIpCount);
        else
            ip << ipAddresses.mid(i * ipCountForDefThread + additionalIpCount, ipCountForDefThread);


        pingsAsyncForPingAll.at(i)->setIpForAsyncPing(ip);
        threadPool->start(pingsAsyncForPingAll.at(i));
    }
}

void CPing::pingOneIpAsync(int index) {
    threadPoolInit(1);
    if (ipAddresses.length() > index) {
        threadPool->start(pingsAsyncForPingOne.at(index));
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
        indexIpAdrForTimerPingOneIpAsync = index;
        timerPingOneIpAsync.start(interval);
    }
}

void CPing::startPingAllIpByTimer(int interval) {
    if (!timerPingAllIp.isActive())
        timerPingAllIp.start(interval);
}

void CPing::startPingAllIpByTimerAsync(int interval) {
    if (!timerPingAllIpAsync.isActive()) {
        timerPingAllIpAsync.start(interval);
    }
}
