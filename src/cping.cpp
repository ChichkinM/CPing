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

    delete pingSync;
}

void CPing::init() {
    threadPool.setMaxThreadCount(1);

    for(int i = 0; i < ipAddresses.count(); i++) {
#ifdef _WIN32
        pingsAsync.append(new CPingWindows);
        pingsSync.append(new CPingWindows);
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

    connect(&timerPingAllIpAsync, &QTimer::timeout, this, [this](){ pingAllIpAsync(threadsForTimerPingAllIpAsync); });
    connect(&timerPingOneIpAsync, &QTimer::timeout, this, [this](){ pingOneIpAsync(indexIpAdrForTimerPingOneIpAsync); });

    connect(&timerPingOneIp, &QTimer::timeout, this, [this](){
        emit responsePingAllIp( {
                                    pingOneIp(indexIpAdrForTimerPingOneIp)}); });
    connect(&timerPingAllIp, &QTimer::timeout, this, [this](){
        emit responsePingAllIp(
                    pingAllIp()); });
}

void CPing::workWithMaxThreadCount(int newTaskCount) {
//    qDebug() << "******* start work with threads *******";
    int freeThreads = threadPool.maxThreadCount() - threadPool.activeThreadCount();
    if (freeThreads < newTaskCount) {
        int additionalthreads = newTaskCount - freeThreads;
//        qDebug() << "add" << additionalthreads;
        threadPool.setMaxThreadCount(threadPool.maxThreadCount() + additionalthreads);
    }

//    qDebug() << "max threads" << threadPool.maxThreadCount();
//    qDebug() << "active threads" << threadPool.activeThreadCount();
//    qDebug() << "******* end work with threads *******";
}

void CPing::responsePingAllIpAsyncAggregator(QVector<ICPingOS::CPingResponse> result) {
    agregatorResult << result;
    if (agregatorResult.count() == ipAddresses.count()) {
        emit responsePingAllIpAsync(agregatorResult);
        agregatorResult.clear();
    }
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

void CPing::pingAllIpAsync(unsigned int threads) {
    if (threads > 0) {
        workWithMaxThreadCount(threads);

        int ipCountForDefThread = ipAddresses.count() / threads;

        for(int i = 0; i < threads; i++) {
            QVector<QString> ip;
            int additionalIpCount = ipAddresses.count() % threads;
            if (i == 0)
                ip << ipAddresses.mid(i, ipCountForDefThread + additionalIpCount);
            else
                ip << ipAddresses.mid(i + additionalIpCount, ipCountForDefThread);

            pingsAsyncForPingAll.at(i)->setIpForAsyncPing(ip);
            threadPool.start(pingsAsyncForPingAll.at(i));
        }
    }
}

void CPing::pingOneIpAsync(int index) {
    if (ipAddresses.length() > index) {
        workWithMaxThreadCount(1);
        threadPool.start(pingsAsyncForPingOne.at(index));
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

void CPing::startPingAllIpByTimerAsync(int interval, int threads) {
    if (!timerPingAllIpAsync.isActive()) {
        threadsForTimerPingAllIpAsync = threads;
        timerPingAllIpAsync.start(interval);
    }
}
