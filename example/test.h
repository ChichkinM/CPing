#ifndef TEST_H
#define TEST_H

#include <QDebug>
#include "cping.h"

class Test : public QObject {
    Q_OBJECT
public:
    Test() : QObject() {
        cping = new CPing({ "8.8.8.8", "8.8.8.9" });

        connect(cping, SIGNAL(responsePingAllIpAsync(QVector<QPair<QString,ICPingOS::CPingResult>>)), this,
                SLOT(onResponsePingAllIp(QVector<QPair<QString,ICPingOS::CPingResult>>)));
        connect(cping, SIGNAL(responsePingAllIp(QVector<QPair<QString,ICPingOS::CPingResult>>)), this,
                SLOT(onResponsePingAllIp(QVector<QPair<QString,ICPingOS::CPingResult>>)));
        connect(cping, SIGNAL(responsePingOneIp(ICPingOS::CPingResult)), this,
                SLOT(onResponsePingOneIp(ICPingOS::CPingResult)));
        connect(cping, SIGNAL(responsePingOneIpAsync(ICPingOS::CPingResult)), this,
                SLOT(onResponsePingOneIp(ICPingOS::CPingResult)));

//        cping->pingAllIpAsync();
//        cping->pingOneIp();
//        cping->pingAllIpAsync();
//        cping->pingOneIpAsync();
//        cping->startPingAllIpByTimer(500);
//        cping->startPingOneIpByTimer(500, 1);
//        cping->startPingAllIpByTimerAsync(500);
//        cping->startPingOneIpByTimerAsync(500, 1);
        qDebug() << "start test";
    }

private:
    CPing *cping = nullptr;

private slots:
    void onResponsePingAllIp(QVector<QPair<QString, ICPingOS::CPingResult>> result) {
        for (QPair<QString, ICPingOS::CPingResult> r : result)
            qDebug() << r.first << r.second;
    }

    void onResponsePingOneIp(ICPingOS::CPingResult result) {
            qDebug() << result;
    }
};

#endif // TEST_H
