#include "cpingwindows.h"
#include <QDebug>

CPingWindows::CPingWindows(QObject *parent) : ICPingOS(parent) {
    
}

ICPingOS::CPingResult CPingWindows::pingOneIp(QString ip) {
#ifdef _WIN32
    unsigned long ipaddr = inet_addr(ip.toLatin1().data());;
    char sendData[32] = "Data Buffer";
    LPVOID replyBuffer = NULL;
    DWORD replySize = 0;


    HANDLE icmpFile = IcmpCreateFile();
    if (icmpFile == INVALID_HANDLE_VALUE)
        return USING_ERROR; //Unable to open handle


    replySize = sizeof(ICMP_ECHO_REPLY) + sizeof(sendData);
    replyBuffer = (VOID*) malloc(replySize);
    if (replyBuffer == NULL)
        return USING_ERROR; //Unable to allocate memory

    IcmpSendEcho(icmpFile, ipaddr, sendData, sizeof(sendData),
                            NULL, replyBuffer, replySize, 1000);

    PICMP_ECHO_REPLY echoReply = (PICMP_ECHO_REPLY)replyBuffer;

    return (CPingResult)echoReply->Status;
#else
    return ICPingOS::OS_NOT_DEFINED;
#endif
}


QVector<QPair<QString, ICPingOS::CPingResult>> CPingWindows::pingAllIp(QVector<QString> ip) {
    QVector<QPair<QString, ICPingOS::CPingResult>> result;

    for (QString oneIp : ip) {
        QPair<QString, ICPingOS::CPingResult> ipResult (oneIp, pingOneIp(oneIp));
        result.append(ipResult);
    }

    return result;
}

void CPingWindows::pingAllIpAsync(QVector<QString> ip) {
    emit responsePingAllIpAsync(pingAllIp(ip));
}

void CPingWindows::pingOneIpAsync(QString ip) {
    emit responsePingOneIpAsync(pingOneIp(ip));
}
