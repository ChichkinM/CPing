#ifndef CPINGLINUX_H
#define CPINGLINUX_H

#include <QtNetwork/QHostAddress>
#include "icpingos.h"


#pragma pack(push,1)
struct Ticmp_result
{
    quint8      SW; //слово состояния
    quint32 	seq;//
    double		rtt;//время прохождения команды
};
#pragma pack(pop)

#pragma pack(push,1)
struct send_data
{
    struct sockaddr_in  *sAdr;
    quint32             nSent;			/*Счетчик для посылок add 1 for each sendto() */
};
#pragma pack(pop)


class CPingLinux : public ICPingOS
{
public:
    CPingLinux(QObject *parent = nullptr);
    ~CPingLinux();

    CPingResponse pingOneIp(QString ipAdr);
    QVector<CPingResponse> pingAllIp(QVector<QString> ip){}

public slots:
    void pingAllIpAsync(QVector<QString> ip){}
    void pingOneIpAsync(QString ip){}

private:
    CPingResult sockError = SUCCESS;
    int sock;
    unsigned short in_cksum(unsigned short *addr, int len);
};

#endif // CPINGLINUX_H
