#include "cpinglinux.h"

#include <QDebug>

#include <sys/socket.h>
#include <netinet/ip.h>

#include <iostream>

#include	<arpa/inet.h>	/* inet(3) functions */

#include	<netinet/in.h>	/* sockaddr_in{} and other Internet defns */

#include    <unistd.h>
#include    <fcntl.h>
#include	<sys/ioctl.h>
#include	<sys/socket.h>	/* basic socket definitions */
#include	<sys/stat.h>	/* for S_xxx file mode constants */
#include	<sys/time.h>	/* timeval{} for select() */
#include	<sys/types.h>	/* basic system data types */
#include	<sys/wait.h>
#include	<sys/uio.h>		/* for iovec{} and readv/writev */
#include	<sys/un.h>		/* for Unix domain sockets */
#include    <errno.h>
#include <netinet/ip.h>
#include        <netinet/in_systm.h>

#include        <netinet/ip.h>

#include        <netinet/ip_icmp.h>

#include <chrono>
#include <ctime>





CPingLinux::CPingLinux(QObject *parent) : ICPingOS(parent) {
    sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    if(sock >= 0) {
        int size = 60 * 1024;		/* OK if setsockopt fails */ //TODO: int value is strange
        if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &size, sizeof(int)) < 0)
            sockError = LINUX_SOCK_OPT_ERROR;
        else
            fcntl(sock,F_SETFL,O_NONBLOCK); //TODO: it't realy need?
    }
    else
        switch (errno)
        {
        case EINVAL:            sockError = LINUX_SOCK_ERR_EINVAL;              break;
        case ENOMEM:            sockError = LINUX_SOCK_ERR_ENOMEM;              break;
        case ENOBUFS:           sockError = LINUX_SOCK_ERR_ENOBUFS;             break;
        case EACCES:            sockError = LINUX_SOCK_ERR_EACCES;              break;
        case ENFILE:            sockError = LINUX_SOCK_ERR_ENFILE;              break;
        case EMFILE:            sockError = LINUX_SOCK_ERR_EMFILE;              break;
        case EPROTONOSUPPORT:	sockError = LINUX_SOCK_ERR_EPROTONOSUPPORT;     break;
        default:                sockError = LINUX_SOCK_ERR_UNNOWN;              break;
        }
}

CPingLinux::~CPingLinux() {
    close(sock);
}

ICPingOS::CPingResponse CPingLinux::pingOneIp(QString ipAddr) {
    CPingResponse response;
    response.ip = ipAddr;
    response.result = ERROR_RESPONSE;

    if (sock < 0) {
        response.result = sockError;
        return response;
    }





    char recvbuf[1024]; //TODO: why bufsize == 1500?
    char sendbuf[1024]; //TODO: why bufsize == 1500?



    struct sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = inet_addr(ipAddr.toStdString().c_str());



    socklen_t 	sa_len = sizeof (struct sockaddr_in);

    int seed = std::chrono::system_clock::now().time_since_epoch().count();
    qsrand(seed);
    int icmpPackageId = qrand() % 10000;


    struct icmp	*icmp = (struct icmp *) sendbuf;
    icmp->icmp_type = ICMP_ECHO;
    icmp->icmp_code = 0;
    icmp->icmp_id = icmpPackageId;
    //    icmp->icmp_seq = nsent++;


    int len = 64;		/* checksum ICMP header and data */ //TODO: why len is 64?

    icmp->icmp_seq = /*sasend.nSent++*/1;
    icmp->icmp_cksum = 0;
    icmp->icmp_cksum = in_cksum((ushort *) icmp, len);

    auto startIcmpPackageTrip = std::chrono::system_clock::now();
    int sendtoLen = sendto(sock, sendbuf, len, 0, (sockaddr*) &sockAddr , sa_len);

    if(sendtoLen != len)
    {
        if(errno == EHOSTDOWN)
            qDebug() << ipAddr << "HOST_IS_DOWN";
        //                 emit resultPing(QHostAddress(str), HOST_IS_DOWN);
        else if(errno == EHOSTUNREACH)
            qDebug() << ipAddr << "HOST_IS_UNREACHABLE";
        //                emit resultPing(QHostAddress(str), HOST_IS_UNREACHABLE);

//        qDebug() << "sendto error" << errno;
    }



    struct sockaddr  sarecv;

    int noResponseCounter = 0;

    for (;;) {
        len = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, &sarecv, &sa_len);
        if (len < 0) {
            noResponseCounter++;

            if (noResponseCounter == 2) {
                response.result = ERROR_RESPONSE;
                return response;
            }

            usleep(250000); //TODO: config delay
            continue;
        }


        int			hlen1, icmplen;
        struct ip		*ip;

        ip = (struct ip *) recvbuf;		/* start of IP header */
        hlen1 = ip->ip_hl << 2;		/* length of IP header */

        icmp = (struct icmp *) (recvbuf + hlen1);	/* start of ICMP header */
        if ( (icmplen = len - hlen1) < 8)
            continue ;

        if (icmp->icmp_type == ICMP_ECHOREPLY)
        {
            if (icmp->icmp_id != icmpPackageId || icmplen < 16)
                continue ;


            if (QHostAddress(&sarecv).toString() == ipAddr) {
                auto endIcmpPackageTrip = std::chrono::system_clock::now();
                std::chrono::duration<double> elapsedSeconds = endIcmpPackageTrip - startIcmpPackageTrip;

                response.result = SUCCESS;
                response.tripTime = elapsedSeconds.count() * 1000;

                break;
            }
        }
    }

    return response;
}

QVector<ICPingOS::CPingResponse> CPingLinux::pingAllIp(QVector<QString> ip) {
    QVector<CPingResponse> result;

    for (QString oneIp : ip)
        result.append(pingOneIp(oneIp));

    return result;
}

void CPingLinux::pingAllIpAsync(QVector<QString> ip) {
    emit responsePingAllIpAsync(pingAllIp(ip));
}

void CPingLinux::pingOneIpAsync(QString ip) {
    emit responsePingOneIpAsync(pingOneIp(ip));
}

unsigned short CPingLinux::in_cksum(unsigned short *addr, int len)
{
    int				nleft = len;
    int				sum = 0;
    unsigned short	*w = addr;
    unsigned short	answer = 0;

    /*
     * Our algorithm is simple, using a 32 bit accumulator (sum), we add
     * sequential 16 bit words to it, and at the end, fold back all the
     * carry bits from the top 16 bits into the lower 16 bits.
     */
    while (nleft > 1)  {
        sum += *w++;
        nleft -= 2;
    }

    /* 4mop up an odd byte, if necessary */
    if (nleft == 1) {
        *(unsigned char *)(&answer) = *(unsigned char *)w;
        sum += answer;
    }

    /* 4add back carry outs from top 16 bits to low 16 bits */
    sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
    sum += (sum >> 16);			/* add carry */
    answer = ~sum;				/* truncate to 16 bits */

    return answer;
}
