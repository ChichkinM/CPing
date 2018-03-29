#ifndef CPINGWINDOWS_H
#define CPINGWINDOWS_H

#include "icpingos.h"
#include <stdio.h>

#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <windows.h>

class CPingWindows :  public ICPingOS
{
public:
    CPingWindows(QObject *parent = nullptr);

    QVector<CPingResponse> pingAllIp(QVector<QString> ip);

private:
    ICPingOS::CPingResponse pingOneIp(QString ip);
};

#endif // CPINGWINDOWS_H
