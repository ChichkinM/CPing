#ifndef CPINGLINUX_H
#define CPINGLINUX_H

#include <QtNetwork/QHostAddress>
#include "icpingos.h"

#define	BUFSIZE		1500
#define ERR_STRLEN -1
#define ERR_MEMORY -2
#define ERR_COPYSTR -3

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

enum PingStatus
{
    SUCCESS,
    HOST_IS_DOWN,
    HOST_IS_UNREACHABLE
};


class CPingLinux : public ICPingOS
{
public:
    CPingLinux(QObject *parent = nullptr);

    CPingResponse pingOneIp(QString ip);
    QVector<CPingResponse> pingAllIp(QVector<QString> ip){}

public slots:
    void pingAllIpAsync(QVector<QString> ip){}
    void pingOneIpAsync(QString ip){}

private:
    unsigned short in_cksum(unsigned short *addr, int len);



    qint8       isInitOk;
    QMap<QString, QHostAddress> ethAddress;     // Имя хостов
    QMap<QString, Ticmp_result> icmp_result;    // Результаты
    QMap<QString, send_data>  sasend;         // посылка

    char         recvbuf[BUFSIZE];        /*Буффер для приема сообщения*/
    char         sendbuf[BUFSIZE];        /*Буффер для отправки сообщения*/
    int         sock;                        /*Сокет*/
    bool isSTOP;
    int                 pid;
    bool send(void);
    void icmp_error(const char *str, int err);
//    unsigned short in_cksum(unsigned short *addr, int len);

};

#endif // CPINGLINUX_H
