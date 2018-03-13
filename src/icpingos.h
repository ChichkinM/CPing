#ifndef ICPINGOS_H
#define ICPINGOS_H

#include <QObject>
#include <QPair>
#include <QVector>

class ICPingOS : public QObject {
    Q_OBJECT
public:
    ICPingOS(QObject *parent = nullptr) : QObject(parent) { }

    enum CPingResult {
        SUCCESS =                   0,
        BUF_TO_SMAL =               11001,
        DEST_NET_UNREACHABLE =      11002,
        DEST_HOST_UNREACHABLE =     11003,
        DEST_PROT_UNREACHABLE =     11004,
        DEST_PORT_UNREACHABLE =     11005,
        NO_RESOURCES =              11006,
        BAD_OPTION =                11007,
        HW_ERROR =                  11008,
        PACKET_TOO_BIG =            11009,
        REQ_TIMED_OUT =             11010,
        BAD_REQ =                   11011,
        BAD_ROUTE =                 11012,
        TTL_EXPIRED_TRANSIT =       11013,
        TTL_EXPIRED_REASSEM =       11014,
        PARAM_PROBLEM =             11015,
        SOURCE_QUENCH =             11016,
        OPTION_TOO_BIG =            11017,
        BAD_DESTINATION =           11018,
        GENERAL_FAILURE =           11050,

        USING_ERROR =               12000,
        OS_NOT_DEFINED =            13000
    };

    struct CPingResponse {
        QString ip;
        CPingResult result = USING_ERROR;
        int tripTime = -1; //ms
    };

    virtual CPingResponse pingOneIp(QString ip) = 0;
    virtual QVector<ICPingOS::CPingResponse> pingAllIp(QVector<QString> ip) = 0;

public slots:
    virtual void pingAllIpAsync(QVector<QString> ip) = 0;
    virtual void pingOneIpAsync(QString ip) = 0;

signals:
    void responsePingAllIpAsync(QVector<ICPingOS::CPingResponse>);
    void responsePingOneIpAsync(ICPingOS::CPingResponse);
};

#endif // ICPINGOS_H
