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
    CPing(QString ipAddress, unsigned int threadsForAsync = 0, QObject *parent = 0);
    CPing(QVector<QString> ipAddresses, unsigned int threadsForAsync = 0, QObject *parent = 0);
    ~CPing();

    ICPingOS *pingSync = nullptr;
    QVector<ICPingOS*> pingsAsyncForPingAll;
    QVector<ICPingOS*> pingsAsyncForPingOne;

    QVector<ICPingOS::CPingResponse> pingAllIp();
    ICPingOS::CPingResponse pingOneIp(int index = 0);
    void pingOneIpAsync(int index = 0);
    void pingAllIpAsync();

    void startPingOneIpByTimer(int interval, int index = 0);
    void startPingOneIpByTimerAsync(int interval, int index = 0);
    void startPingAllIpByTimer(int interval);
    void startPingAllIpByTimerAsync(int interval);

private:
    void init(unsigned int threadsForAsync);
    void threadPoolInit(unsigned int threads);

    QThreadPool *threadPool = nullptr;
    QVector<QString> ipAddresses;

    int indexIpAdrForTimerPingOneIp = 0;
    int indexIpAdrForTimerPingOneIpAsync = 0;

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
    void responsePingAllIp(QVector<ICPingOS::CPingResponse>);
};

#endif // CPING_H
