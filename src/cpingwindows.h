#ifndef CPINGWINDOWS_H
#define CPINGWINDOWS_H

#include "icpingos.h"
#include <stdio.h>

#ifdef _WIN32
#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <windows.h>
#endif

class CPingWindows :  public ICPingOS
{
public:
    CPingWindows(QObject *parent = 0);

public:
    CPingResult pingOneIp(QString ip);
    QVector<QPair<QString, CPingResult>> pingAllIp(QVector<QString> ip);

public slots:
    void pingAllIpAsync(QVector<QString> ip);
    void pingOneIpAsync(QString ip);
};

#endif // CPINGWINDOWS_H
