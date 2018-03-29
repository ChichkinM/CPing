#ifndef TEST_H
#define TEST_H

#include <QDebug>
#include "cping.h"

class Test : public QObject {
    Q_OBJECT
public:
    Test() : QObject() {
        cping = new CPing({ "8.8.8.8", "8.8.8.9", "10.7.16.208" });

        connect(cping, SIGNAL(responsePingAllIpAsync(QVector<ICPingOS::CPingResponse>)), this,
                SLOT(onResponsePingAllIp(QVector<ICPingOS::CPingResponse>)));
        connect(cping, SIGNAL(responsePingAllIp(QVector<ICPingOS::CPingResponse>)), this,
                SLOT(onResponsePingAllIp(QVector<ICPingOS::CPingResponse>)));
        connect(cping, SIGNAL(responsePingOneIp(ICPingOS::CPingResponse)), this,
                SLOT(onResponsePingOneIp(ICPingOS::CPingResponse)));
        connect(cping, SIGNAL(responsePingOneIpAsync(ICPingOS::CPingResponse)), this,
                SLOT(onResponsePingOneIp(ICPingOS::CPingResponse)));



//        ICPingOS::CPingResponse r = cping->pingOneIp(2);
//        qDebug() << r.ip << r.result << r.tripTime;

//        for (ICPingOS::CPingResponse r : cping->pingAllIp())
//            qDebug() << r.ip << r.result << r.tripTime;

//        cping->pingAllIpAsync(2);

//        cping->pingOneIpAsync(0);
//        cping->pingOneIpAsync(1);
//        cping->pingOneIpAsync(1);

//        cping->startPingAllIpByTimer(500);

//        cping->startPingOneIpByTimer(500, 1);
//        cping->startPingAllIpByTimerAsync(500);
        cping->startPingOneIpByTimerAsync(500, 1);

        qDebug() << "test";
    }

    ~Test() { delete cping; }

private:
    CPing *cping = nullptr;

private slots:
    void onResponsePingAllIp(QVector<ICPingOS::CPingResponse> result) {
        for (ICPingOS::CPingResponse r : result)
            qDebug() << r.ip << r.result << r.tripTime;
    }

    void onResponsePingOneIp(ICPingOS::CPingResponse result) {
            qDebug() << result.ip << result.result << result.tripTime;
    }
};

#endif // TEST_H
