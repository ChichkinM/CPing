#ifndef CPINGLINUX_H
#define CPINGLINUX_H

#include <QtNetwork/QHostAddress>
#include "icpingos.h"


class CPingLinux : public ICPingOS
{
public:
    CPingLinux(QObject *parent = nullptr);
    ~CPingLinux();

    QVector<CPingResponse> pingAllIp(QVector<QString> ip);

private:
    ICPingOS::CPingResponse pingOneIp(QString ipAddr);

    CPingResult sockError = SUCCESS;
    int sock;
    unsigned short in_cksum(unsigned short *addr, int len);
};

#endif // CPINGLINUX_H
