#include "cpingwindows.h"
#include <QDebug>

CPingWindows::CPingWindows(QObject *parent) : ICPingOS(parent) {
    
}

ICPingOS::CPingResponse CPingWindows::pingOneIp(QString ip) {
    CPingResponse response;
    response.ip = ip;
    response.result = ERROR_RESPONSE;

    unsigned long ipaddr = inet_addr(ip.toLatin1().data());;
    char sendData[32] = "Data Buffer";
    LPVOID replyBuffer = NULL;
    DWORD replySize = 0;


    HANDLE icmpFile = IcmpCreateFile();
    if (icmpFile == INVALID_HANDLE_VALUE)
        return response; //Unable to open handle


    replySize = sizeof(ICMP_ECHO_REPLY) + sizeof(sendData);
    replyBuffer = (VOID*) malloc(replySize);
    if (replyBuffer == NULL)
        return response; //Unable to allocate memory


    if (IcmpSendEcho(icmpFile, ipaddr, sendData, sizeof(sendData),
                     NULL, replyBuffer, replySize, 500) != 0 ) {

        PICMP_ECHO_REPLY echoReply = (PICMP_ECHO_REPLY)replyBuffer;

        response.result = (CPingResult)echoReply->Status;
        response.tripTime = echoReply->RoundTripTime;
    }

    return response;
}


QVector<ICPingOS::CPingResponse> CPingWindows::pingAllIp(QVector<QString> ip) {
    QVector<CPingResponse> result;

    for (QString oneIp : ip)
        result.append(pingOneIp(oneIp));

    return result;
}

void CPingWindows::pingAllIpAsync(QVector<QString> ip) {
    emit responsePingAllIpAsync(pingAllIp(ip));
}

void CPingWindows::pingOneIpAsync(QString ip) {
    emit responsePingOneIpAsync(pingOneIp(ip));
}
