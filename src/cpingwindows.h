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

public:
    CPingResult pingOneIp(QString ip);
    QVector<QPair<QString, CPingResult>> pingAllIp(QVector<QString> ip);

public slots:
    void pingAllIpAsync(QVector<QString> ip);
    void pingOneIpAsync(QString ip);
};

#endif // CPINGWINDOWS_H
