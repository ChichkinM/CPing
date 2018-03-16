#ifndef CPINGLINUX_H
#define CPINGLINUX_H

#include "icpingos.h"

class CPingLinux : public ICPingOS
{
public:
    CPingLinux(QObject *parent = nullptr);

    CPingResponse pingOneIp(QString ip) {}
    QVector<CPingResponse> pingAllIp(QVector<QString> ip){}

public slots:
    void pingAllIpAsync(QVector<QString> ip){}
    void pingOneIpAsync(QString ip){}
};

#endif // CPINGLINUX_H
