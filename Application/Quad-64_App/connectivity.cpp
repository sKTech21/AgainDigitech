#include "connectivity.h"
#include <QObject>
#include <QThread>
#include <QJsonObject>
#include <QJsonDocument>

connectivity::connectivity()
{
    tcpServer.listen( QHostAddress::Any, TCP_SERVER_PORT );
    connect( &tcpServer, SIGNAL( newConnection() ), this, SLOT( tcpClientConnectRequest() ) );

    udpSocket = new QUdpSocket( );
    udpSocket->bind( 9999, QUdpSocket::ShareAddress );
    connect( udpSocket, SIGNAL( readyRead() ), this, SLOT( onSocketDataReadyRead() ) );
}

bool connectivity::checkIfInternetIsAvailable( )
{
    QTcpSocket* sock = new QTcpSocket( );
    sock->connectToHost( "www.google.com", 80 );
    bool connected = sock->waitForConnected( 3000 );

    if( !connected )
    {
        sock->abort( );
        return false;
    }
    sock->close( );
    return true;
}

void connectivity::tcpClientConnectRequest( )
{
    qDebug() << "New TCP Client Connection";
    QTcpSocket *clientSocket = tcpServer.nextPendingConnection();
    connect( clientSocket, SIGNAL( readyRead() ), this, SLOT( onSocketDataReadyRead() ) );
    connect( clientSocket, SIGNAL( stateChanged( QAbstractSocket::SocketState ) ), this,
                           SLOT( onTCPSocketStateChanged( QAbstractSocket::SocketState ) ) );

    tcpSocketList.push_back( clientSocket );
    QString clientIpAddress = QHostAddress( clientSocket->peerAddress().toIPv4Address() ).toString();

    for( QTcpSocket* socket : tcpSocketList )
    {
        socket->write( QByteArray::fromStdString(
            QHostAddress( clientSocket->peerAddress().toIPv4Address() ).toString().toStdString() + " connected to server !\n" ) );
        QThread::msleep( 500 );
    }

    qDebug() << "Connected TCP Client IP List";
    for( QTcpSocket *handler : tcpSocketList )
    {
        QString hostIP = QHostAddress( handler->peerAddress().toIPv4Address() ).toString();
        qDebug() << "hostIP : " << hostIP ;
    }
}

void connectivity::onTCPSocketStateChanged( QAbstractSocket::SocketState socketState )
{
    if( socketState == QAbstractSocket::UnconnectedState )
    {
        QTcpSocket* sender = static_cast<QTcpSocket*>( QObject::sender() );
        tcpSocketList.removeOne( sender );

        QString clientIpAddress = QHostAddress( sender->peerAddress().toIPv4Address() ).toString();
        qDebug() << "Connection Disconnected : " << clientIpAddress;
    }
}

void connectivity::onSocketDataReadyRead()
{
    QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());
    QByteArray datas = sender->readAll();
    //qDebug() << "Data Received on Socket 4242+++++++++++++++++++++++++++++++++++++++++++";

    for( QTcpSocket* socket : tcpSocketList )
    {
        if ( socket == sender )
        {
            QJsonDocument jsonResp = QJsonDocument::fromJson( datas );
            QJsonObject jsonObj = jsonResp.object();

            QString strMesg = datas;
            //qDebug() << strMesg;

            /*
            if( jsonObj.contains( MQTT_MESSAGE_CODE_KEY ) )
            {
                //qDebug() << "Mesg Code : " << jsonObj[ MQTT_MESSAGE_CODE_KEY ].toInt();
                switch( jsonObj[ MQTT_MESSAGE_CODE_KEY ].toInt() )
                {

                }
            }*/
        }
    }
}

void connectivity::sendTCPPointToPoint( QString _message, quint8 _nodeNumber )
{
    /*
    //qDebug() << "Sending : " << _message << " TO Node Number : " << _nodeNumber;
    for( QTcpSocket *handler : tcpSocketList )
    {
        QString hostIP = QHostAddress( handler->peerAddress().toIPv4Address() ).toString();
        if( hostIP == objNodeInfo->getNodeNetworkIP( _nodeNumber ) )
        {
            qint64 bytesWritten = handler->write( QByteArray::fromStdString( _message.toStdString() ) );
            if( bytesWritten == -1 )
            {
                qCritical() << "Data not sent over TCP, retrying";
                for( quint8 ite = 0; ite < 5; ite++ )
                {
                    bytesWritten = handler->write( QByteArray::fromStdString( _message.toStdString() ) );
                    if( bytesWritten != -1 )
                    {
                        qDebug() << "Data Sent Successfully";
                        break;
                    }
                    QThread::msleep( 5 );
                    qCritical() << "Retrying TCP Tx : " << ite;
                }
            }
            qDebug() << "TCP TX->" <<  hostIP << bytesWritten;
            return;
        }
    }*/
}
