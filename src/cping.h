#ifndef CPING_H
#define CPING_H

#include <QObject>
#include <QThreadPool>
#include <QVector>
#include <QTimer>
#include "icpingos.h"

class CPing : public QObject
{
    Q_OBJECT
public:
    CPing(QObject *parent = 0);
    CPing(QString ipAddress, QObject *parent = 0);
    CPing(QVector<QString> ipAddresses, QObject *parent = 0);
    ~CPing();

    ICPingOS *pingSync = nullptr;
    QVector<ICPingOS*> pingsAsyncForPingAll;
    QVector<ICPingOS*> pingsAsyncForPingOne;

    QVector<ICPingOS::CPingResponse> pingAllIp();
    ICPingOS::CPingResponse pingOneIp(int index = 0);
    void pingOneIpAsync(int index = 0);
    void pingAllIpAsync(unsigned int threads = 1);

    void startPingOneIpByTimer(int interval, int index = 0);
    void startPingOneIpByTimerAsync(int interval, int index = 0);
    void startPingAllIpByTimer(int interval);
    void startPingAllIpByTimerAsync(int interval, int threads = 1);

private:
    void init();
    void workWithMaxThreadCount(int newTaskCount);

    QThreadPool threadPool;
    QVector<QString> ipAddresses;

    int indexIpAdrForTimerPingOneIp = 0;
    int indexIpAdrForTimerPingOneIpAsync = 0;
    int threadsForTimerPingAllIpAsync = 1;

    QTimer timerPingOneIp;
    QTimer timerPingOneIpAsync;
    QTimer timerPingAllIp;
    QTimer timerPingAllIpAsync;

    QVector<ICPingOS::CPingResponse> agregatorResult;

private slots:
    void responsePingAllIpAsyncAggregator(QVector<ICPingOS::CPingResponse> result);

signals:
    void pingAllIpAsyncStart(QVector<QString> ip);
    void pingOneIpAsyncStart(QString ip);


    void responsePingAllIpAsync(QVector<ICPingOS::CPingResponse>);
    void responsePingOneIpAsync(ICPingOS::CPingResponse);

    void responsePingAllIp(QVector<ICPingOS::CPingResponse>);
    void responsePingOneIp(ICPingOS::CPingResponse);
};

#endif // CPING_H
