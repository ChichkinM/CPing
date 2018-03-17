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

ICPingOS::CPingResponse CPingLinux::pingOneIp(QString ipAdr) {
    CPingResponse response;
    response.ip = ipAdr;
    response.result = ERROR_RESPONSE;

    if (sock < 0) {
        response.result = sockError;
        return response;
    }

    char recvbuf[1500]; //TODO: why bufsize == 1500?
    char sendbuf[1500]; //TODO: why bufsize == 1500?

    //    QMap<QString, QHostAddress> ethAddress;     // Имя хостов
    QMap<QString, Ticmp_result> icmp_result;    // Результаты
    /*QMap<QString,*/ send_data/*>*/  sasend;         // посылка

    //    ethAddress.insert("test",QHostAddress("127.0.0.1"));
    Ticmp_result res;
    (void) memset(&res, 0, sizeof (Ticmp_result));
    icmp_result.insert("127.0.0.1",res);

    send_data sendData;
    sendData.sAdr =new struct sockaddr_in;
    memset((char *)sendData.sAdr,0x00, sizeof (struct sockaddr_in));
    sendData.sAdr->sin_family = AF_INET;
    sendData.sAdr->sin_addr.s_addr = inet_addr(ipAdr.toStdString().c_str());
    sendData.nSent = 0;
    sasend = sendData;


    int		len;
    struct icmp	*icmp = (struct icmp *) sendbuf;
    socklen_t 	sa_len=sizeof (struct sockaddr_in);

    int pid = 1;
    icmp = (struct icmp *) sendbuf;
    icmp->icmp_type = ICMP_ECHO;
    icmp->icmp_code = 0;
    icmp->icmp_id = pid;
    //    icmp->icmp_seq = nsent++;

    //    if(!isInitOk)   return response;
    //    if(!sock)       return response;

    //for(i=0;i<255;i++)icmp_result[i].SW=0x00;
    //icmp_result.SW=0x00;
//    foreach (const QString &str, icmp_result.keys())
//    {
//        icmp_result[str].SW=0;
//    }

    if(gettimeofday((struct timeval *) icmp->icmp_data, NULL)==-1)
    {
        //        icmp_error("gettimeofday:",errno);
        return response;
    }

    len = 64;		/* checksum ICMP header and data */

    //    foreach (const QString &str, sasend.keys())
    //    {
    icmp->icmp_seq = sasend.nSent++;
    icmp->icmp_cksum = 0;
    icmp->icmp_cksum = in_cksum((ushort *) icmp, len);

    int sendtoLen = sendto(sock, sendbuf, len, 0, (sockaddr*)(sasend.sAdr) , sa_len);

    qDebug() << sendtoLen;

    if(sendtoLen!=len)
    {
        //icmp_error("ping sendto:",errno);

        if(errno == EHOSTDOWN)
            qDebug() << ipAdr << "HOST_IS_DOWN";
        //                 emit resultPing(QHostAddress(str), HOST_IS_DOWN);
        else if(errno == EHOSTUNREACH)
            qDebug() << ipAdr << "HOST_IS_UNREACHABLE";
        //                emit resultPing(QHostAddress(str), HOST_IS_UNREACHABLE);

        qDebug() << "sendto error" << errno;
    }



    struct sockaddr  sarecv;

    for (;;) {
        len=recvfrom(sock, recvbuf, sizeof(recvbuf), 0,&sarecv, &sa_len);
        if (len < 0) {
            if (errno == EINTR)
                continue;
            //icmp_error("recvfrom: ",errno);
            usleep(500000); //TODO: config delay
            continue;
        }


        int			hlen1, icmplen;
        double			rtt;
        struct ip		*ip;
        struct icmp		*icmp;
        struct timeval	*tvsend;

        //struct sockaddr_in	*sin = (struct sockaddr_in *) sarecv;

        ip = (struct ip *) recvbuf;		/* start of IP header */
        hlen1 = ip->ip_hl << 2;		/* length of IP header */

        icmp = (struct icmp *) (recvbuf + hlen1);	/* start of ICMP header */
        if ( (icmplen = len - hlen1) < 8)
            continue ;

        if (icmp->icmp_type == ICMP_ECHOREPLY)
        {
            if (icmp->icmp_id != pid)
            {
                continue ;		/* not a response to our ECHO_REQUEST */
            }

            if (icmplen < 16)
            {
                //                icmp_error("icmplen: < 16", icmplen);
                continue ;
            }
            tvsend = (struct timeval *) icmp->icmp_data;

            //                if(ptval && tvsend)
            //                {
            //                    timersub(ptval, tvsend,ptval);
            //                    rtt = ptval->tv_sec * 1000.0 + ptval->tv_usec / 1000.0;
            //                }

            QHostAddress recv_adr(&sarecv);

            if (recv_adr.toString() == ipAdr)
//            if(icmp_result.contains(recv_adr.toString()))
            {
//                Ticmp_result res;
                //                mtx.lock();
//                res.SW=icmp_result[recv_adr.toString()].SW = 0x80;
//                res.rtt=icmp_result[recv_adr.toString()].rtt = rtt;
//                res.seq=icmp_result[recv_adr.toString()].seq = icmp->icmp_seq;
                //                mtx.unlock();
                //                emit resultPing(recv_adr, SUCCESS);
                qDebug() << "ok";
                response.result = SUCCESS;


            }else
            {
                qDebug()<<"Recive ICMP answer from "<<recv_adr<<" "<<ip;
            }

            break;
        }
    }

    return response;
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
