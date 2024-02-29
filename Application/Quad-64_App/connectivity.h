#ifndef CONNECTIVITY_H
#define CONNECTIVITY_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>

#define TCP_SERVER_PORT                 ( 4242 )
#define UDP_SERVER_PORT                 ( 9999 )

class connectivity : public QObject
{
    Q_OBJECT

public:
    connectivity();

private:
    QUdpSocket *udpSocket;
    bool udpSocketConnectionStatus;
    QString hostUDPID, broadcastUDPID;

    QTcpServer tcpServer;
    QList<QTcpSocket*> tcpSocketList;

    quint8  internetConnectivityCheck, internetConnectivityStatus;
    quint8  cloudConnectivityCheck, cloudConnectivityStatus;

private slots:
    void tcpClientConnectRequest( void );
    void onTCPSocketStateChanged( QAbstractSocket::SocketState socketState );

    void onSocketDataReadyRead( void );
    void sendTCPPointToPoint( QString _message, quint8 _nodeNumber );

public:
    static connectivity& instance() {
        static connectivity instance;
        return instance;
    }

    bool checkIfInternetIsAvailable( );
};

namespace connectivityUtility
{
    inline bool checkIfInternetIsAvailable( ) {
        return connectivity::instance().checkIfInternetIsAvailable( );
    }
}
using connectivityUtility::checkIfInternetIsAvailable;

#endif // CONNECTIVITY_H
