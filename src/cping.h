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

    ICPingOS *ping = nullptr;
//    ICPingOS *pingAsync = nullptr;
    QVector<ICPingOS*> pings;

    QVector<ICPingOS::CPingResponse> pingAllIp();
    ICPingOS::CPingResponse pingOneIp(int index = 0);
    void pingOneIpAsync(int index = 0);
    void pingAllIpAsync(unsigned int threads = 1);

    void startPingOneIpByTimer(int interval, int index = 0);
    void startPingOneIpByTimerAsync(int interval, int index = 0);
    void startPingAllIpByTimer(int interval);
    void startPingAllIpByTimerAsync(int interval);

private:
    void init();

    QThreadPool threadPool;
    QVector<QString> ipAddresses;

    int indexIpAdrForTimerPingOneIp = 0;
    int indexIpAdrForTimerPingOneIpAsync = 0;
    QTimer timerPingOneIp;
    QTimer timerPingOneIpAsync;
    QTimer timerPingAllIp;
    QTimer timerPingAllIpAsync;

signals:
    void pingAllIpAsyncStart(QVector<QString> ip);
    void pingOneIpAsyncStart(QString ip);


    void responsePingAllIpAsync(QVector<ICPingOS::CPingResponse>);
    void responsePingOneIpAsync(ICPingOS::CPingResponse);

    void responsePingAllIp(QVector<ICPingOS::CPingResponse>);
    void responsePingOneIp(ICPingOS::CPingResponse);
};

#endif // CPING_H
