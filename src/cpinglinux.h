#ifndef CPINGLINUX_H
#define CPINGLINUX_H

#include <QtNetwork/QHostAddress>
#include "icpingos.h"


class CPingLinux : public ICPingOS
{
public:
    CPingLinux(QObject *parent = nullptr);
    ~CPingLinux();

    CPingResponse pingOneIp(QString ipAddr);
    QVector<CPingResponse> pingAllIp(QVector<QString> ip);

public slots:
    void pingAllIpAsync(QVector<QString> ip);
    void pingOneIpAsync(QString ip);

private:
    CPingResult sockError = SUCCESS;
    int sock;
    unsigned short in_cksum(unsigned short *addr, int len);
};

#endif // CPINGLINUX_H
