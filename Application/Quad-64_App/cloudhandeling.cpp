#include "cloudhandeling.h"

#include <QtNetwork>
#include <QJsonDocument>
#include <QJsonObject>

#include "commonutility.h"
#include "dbroutines.h"
#include "connectivity.h"

const char* HTTPS_AGAIN_CLOUD_SERVER_URL = "https://asb.againdigitech.net/quad64/";
//const char* HTTPS_AGAIN_CLOUD_SERVER_URL = "http://3.137.177.215:1880/quad64";
const char* HTTPS_AGAIN_CLOUD_SERVER_KEY = "0WTYrJc298dY";

st_HTTPErr stHTTPErr[] = {
    { SYSTEM_TIME_NOT_UPTO_DATE,        "System time not upto date" },
    { MANF_CRED_NOT_MATCHING,           "Manufacturer credentials not matching" },
    { SYSTEM_DOES_NOT_EXIST_ON_CLOUD,   "System does not exist on cloud" },
    { PRODUCT_ALREADY_EXIST,            "Product already exist" },
    { PRODUCT_DOES_NOT_EXIST,           "Product does not exist" },
    { NODE_DOES_NOT_EXIST,              "Node does not exist" },
                          };

cloudHandeling::cloudHandeling( )
{
    isRequestOngoing = false;
    networkAccManager = createNetworkAccessManager();
    cloudServerURL = QString( HTTPS_AGAIN_CLOUD_SERVER_URL );

    httpRequestTimer = new QTimer(this);
    connect( httpRequestTimer, &QTimer::timeout, this, &cloudHandeling::httpRequestTimerElapsed );
    httpRequestTimer->setSingleShot( true );

    setCloudUpdateMesgState( IDLE_UPDATE_STATE );
}

cloudHandeling::~cloudHandeling( )
{
    delete networkAccManager;
}

void cloudHandeling::setCloudProcessStatus( bool _status )
{
    isCloudSyncProcessOngoing = _status;
}

bool cloudHandeling::isCloudProcessOngoing( void )
{
    return isCloudSyncProcessOngoing;
}

void cloudHandeling::httpRequestTimerElapsed( )
{
    logMessage( "HTTP Request Timer Elapsed : prodCreateMainPage", 1 );

    if( getCloudUpdateMesgState() == IDLE_UPDATE_STATE )
    {
        nonBlockingPopupMessage( ERROR_MESG,
                                 "No Response From Cloud, Please check Internet "
                                 "Conneciton or contact Manufacture" );
    }
    mesgStatus.timerExpired = true;
}

QNetworkAccessManager *cloudHandeling::createNetworkAccessManager( )
{
    QNetworkAccessManager *netWorkAccManager = new QNetworkAccessManager( this );

    /* We'll handle the finished reply in replyFinished */
    connect( netWorkAccManager, SIGNAL( finished( QNetworkReply* ) ),
             this, SLOT( replyFinished( QNetworkReply* ) ) );
    return netWorkAccManager;
}

void cloudHandeling::doHTTPPostRequest( QString _url, QString _payload )
{
    cloudServerURL = QString( _url );
    jsonByteArrActualMesg = _payload.toUtf8();

    logMessage( QString( "HTTP Payload Size : %1 \n %2" )
                .arg( _payload.length() ).arg( jsonByteArrActualMesg ), 1 );

    isRequestOngoing = true;
    doRequest( );
}

void cloudHandeling::doRequest( )
{
    /* Let's just create network request for this predifned URL... */
    QNetworkRequest request( this->cloudServerURL );

    request.setHeader( QNetworkRequest::ContentTypeHeader, "application/json" );

    /* ...and ask the manager to do the request. */
    logMessage( QString( "REQ : On Epoch : %1" ).arg( QDateTime::currentMSecsSinceEpoch() ), 1 );
    this->networkAccManager->post( request, jsonByteArrActualMesg );
}

void cloudHandeling::replyFinished( QNetworkReply* reply )
{
    /*
     * Reply is finished!
     * We'll ask for the reply about the Redirection attribute
     */
    QVariant possibleRedirectUrl = reply->attribute( QNetworkRequest::RedirectionTargetAttribute );

    /* We'll deduct if the redirection is valid in the redirectUrl function */
    redirectedCloudURL = this->redirectUrl( possibleRedirectUrl.toUrl(), redirectedCloudURL );

    /* If the URL is not empty, we're being redirected. */
    if( !redirectedCloudURL.isEmpty() )
    {
        /* We'll do another request to the redirection url. */
        this->networkAccManager->get( QNetworkRequest( redirectedCloudURL ) );
    }
    else
    {
        /*
         * We weren't redirected anymore
         * so we arrived to the final destination...
         */
        redirectedCloudURL.clear();
        isRequestOngoing = false;
        if( reply->error() == QNetworkReply::NoError )
        {
            //Success
            QByteArray buffer = reply->readAll( );
            logMessage( QString( "Replay Error : %1" ).arg( buffer ), 0 );
            logMessage( QString( "RESP : On Epoch : %1" ).arg( QDateTime::currentMSecsSinceEpoch() ), 0 );
            emit httpReplyFromCloud( buffer, true );
            httpReplyHandler( buffer, true );
        }
        else
        {
            //Handle error
            QString error = reply->errorString();
            logMessage( QString( "reply->errorString() " ).arg( error ), 0 );
            nonBlockingPopupMessage( ERROR_MESG,
                                     QString( "Error : %1" ).arg( CLOUD_COMMUNICATION_ERROR ) );
        }
    }
    /* Clean up. */
    reply->deleteLater();
}

void cloudHandeling::httpReplyHandler( QString _mesgFromCloud, bool _status )
{
    bool filedNotPresent = false;
    logMessage( QString( "\n\nMesg from cloud : %1 %2" )
                .arg( _status ).arg( _mesgFromCloud ), 0 );

    if( _status != true )
        return;

    QJsonDocument jsonDoc = QJsonDocument::fromJson( _mesgFromCloud.toUtf8() );
    QJsonObject jsonObj = jsonDoc.object();

    quint8 httpAction = MAX_INVALID_HTTP_CMD;
    if( jsonObj.contains( HTTP_ACTION_STR ) )
        httpAction = jsonObj[ HTTP_ACTION_STR ].toVariant().toUInt();
    else
    {
        filedNotPresent = true;
    }

    if( !jsonObj.isEmpty() )
    {
        switch( httpAction )
        {
        case PRODUCT_UPDATE_HTTP_MSG:
        {
            httpRequestTimer->stop();

            if( !jsonObj.contains( HTTP_ACTION_STATUS_STR ) ||
                jsonObj[ HTTP_ACTION_STATUS_STR ].toVariant().toUInt() != 0)
            {
                if( jsonObj.contains( HTTP_ERROR_LOGNOTE_STR ) )
                    nonBlockingPopupMessage( ERROR_MESG,
                                             jsonObj[ HTTP_ERROR_LOGNOTE_STR ].toVariant().toString() );
                return;
            }

            if( jsonObj.contains( DB_PROD_NAME ) )
            {
                if( mesgStatus.messageSent == true )
                    mesgStatus.mesgResponseReceived = true;

                DB_setProductInfoByColumn( DB_PROD_CLOUD_UPDATE_REQ_STR,
                                           jsonObj[ DB_PROD_NAME ].toVariant().toString(),
                                           QString::number( 0 ) );
                logMessage( QString( "Cloud Update status Updated for product %1" )
                            .arg( jsonObj[ DB_PROD_NAME ].toVariant().toString() ), 1 );
            }
        }
        break;

        case SYSTEM_GRNAT_HTTP_MSG:
        {
            if( jsonObj.contains( HTTP_APPLICAITON_UNIQUE_ID_STR ) )
            {
                quint8 syncStatus = jsonObj[ HTTP_SYNC_REQUIRED_STATUS_STR ].toVariant().toUInt();
                if( syncStatus == SYNC_ON_TEST_COUNT )
                {
                    if( jsonObj.contains( HTTP_SYNC_TEST_COUNT_STR ) )
                    {
                        httpRequestTimer->stop();
                    }
                }
                else if( syncStatus == SYNC_ON_PERIODIC_INTERVAL )
                {
                    if( jsonObj.contains( HTTP_SYNC_ON_INTERVAL_STR ) )
                    {
                        httpRequestTimer->stop();
                    }
                }
                else if( syncStatus == SYSTEM_ACCESS_DECLINED )
                {
                    httpRequestTimer->stop();
                }

                if( mesgStatus.messageSent == true )
                    mesgStatus.mesgResponseReceived = true;
            }
            logMessage( "Timer stopped on receiving response from cloud", 0 );
        }
        break;

        case SYSTEM_DEFAULT_CONFIGURATION_HTTP_MSG:
        {
            logMessage( "System deafule configuration received by cloud", 0 );
            httpRequestTimer->stop();
        }
        break;

        case SYSTEM_CONFIGURATION_UPDATE_HTTP_MSG:
        {
            logMessage( "System configuration update received by cloud", 0 );
            httpRequestTimer->stop();
            if( mesgStatus.messageSent == true )
                mesgStatus.mesgResponseReceived = true;
        }
        break;
        }
    }
}

QUrl cloudHandeling::redirectUrl( const QUrl& possibleRedirectUrl, const QUrl& oldRedirectUrl ) const
{
    QUrl redirectUrl;
    /*
     * Check if the URL is empty and
     * that we aren't being fooled into a infinite redirect loop.
     * We could also keep track of how many redirects we have been to
     * and set a limit to it, but we'll leave that to you.
     */
    if(!possibleRedirectUrl.isEmpty() &&
        possibleRedirectUrl != oldRedirectUrl)
    {
        redirectUrl = possibleRedirectUrl;
    }
    return redirectUrl;
}

quint8 cloudHandeling::getCloudUpdateMesgState( )
{
    return mesgStatus.updateState;
}

void cloudHandeling::setCloudUpdateMesgState( en_UpdateState _state )
{
    mesgStatus.updateState = _state;
    mesgStatus.messageSent = false;
    mesgStatus.mesgResponseReceived = false;
    mesgStatus.timerExpired = false;
    mesgStatus.retryCounter = 0;
    mesgStatus.stateEnteredOn = QDateTime::currentMSecsSinceEpoch();
    logMessage( QString( "Cloud update state changed to %1" ).arg( mesgStatus.updateState ), 1 );
}

quint64 cloudHandeling::getStateDuration( void )
{
    quint64 currTime = QDateTime::currentMSecsSinceEpoch();
    return ( currTime - mesgStatus.stateEnteredOn );
}

void cloudHandeling::cloudUpdateRoutine()
{
    static quint64 nextSyncWillHappenOn = QDateTime::currentMSecsSinceEpoch() + MAX_LOCAL_SYNC_INTERVAL;

    switch( getCloudUpdateMesgState( ) )
    {
    case IDLE_UPDATE_STATE:
    {
        if( nextSyncWillHappenOn != 0 &&
            nextSyncWillHappenOn >= QDateTime::currentMSecsSinceEpoch() )
            return;

        /* //Enable when Needed and process part is implemented
        if( isProcessRunning() )
            return;
        */
        saveIniConfigurationValues( LAST_TIME_READIN_ON_KEY, QString::number( QDateTime::currentMSecsSinceEpoch() ) );
        nextSyncWillHappenOn = QDateTime::currentMSecsSinceEpoch() + MAX_LOCAL_SYNC_INTERVAL;
        if( isSystemUpdateToCLoudRequired( ) )
        {
            logMessage( "***********************************", 1 );
            logMessage( "Cloud Set Sync Interval is Expired", 1 );
            logMessage( "***********************************", 1 );
            if( !httpRequestTimer->isActive() )
            {
                if( checkIfInternetIsAvailable( ) == false )
                {
                    nonBlockingPopupMessage( ERROR_MESG, "System Synchronization Required,"
                                                         "\n Please Provide Internet connetion" );
                }
                else
                {
                    logMessage( "Internet is Avaliable-1", 1 );
                    setCloudUpdateMesgState( SYSTEM_ACCESS_GRANT_FROM_CLOUD );
                    setCloudProcessStatus( true );
                }
                return;
            }
        }

        if( !httpRequestTimer->isActive() )
        {
            if( checkIfInternetIsAvailable() == false )
            {
                logMessage( "Avoiding Sync Due to no internet connection", 1 );
            }
            else
            {
                logMessage( "Internet is Avaliable-Periodic Sync on Local Sync Interval", 1 );
                setCloudUpdateMesgState( SYSTEM_ACCESS_GRANT_FROM_CLOUD );
                setCloudProcessStatus( true );
            }
            return;
        }
    }
    break;

    case SYSTEM_ACCESS_GRANT_FROM_CLOUD:
    {
        if( !httpRequestTimer->isActive() &&
            mesgStatus.messageSent == false )
        {
            logMessage( "Sync Required Hence Sending Request to Cloud to grant Acess to System", 1 );
            mesgStatus.messageSent = true;
            mesgStatus.mesgResponseReceived = false;
            sendSystemAccessGrantRequestToCloud();
        }
        else
        {
            if( mesgStatus.messageSent == true &&
                mesgStatus.mesgResponseReceived == true )
            {
                logMessage( "Moving To Update the Product Data****************************", 1 );
                setCloudUpdateMesgState( PRODUCT_UPDATE_WITH_CLOUD );
                return;
            }

            if( !httpRequestTimer->isActive() )
            {
                //As Mesg Response not received within defined duration, considering it as Failed
                mesgStatus.messageSent = false;
                mesgStatus.mesgResponseReceived = true;
                mesgStatus.retryCounter++;
                logMessage( QString( "Retrying send Grant Access message to CLoud " )
                            .arg( mesgStatus.retryCounter ), 1 );
            }

            if( mesgStatus.retryCounter >= 3 )
            {
                logMessage( "All Retyr Failed in SYSTEM_ACCESS_GRANT_FROM_CLOUD So "
                            "moving to Idle Situation", 1 );
                updateSystemAllowedStatus();
                if( isSystemAllowedToOperate() == false )
                {
                    nonBlockingPopupMessage( ERROR_MESG,
                                            "Please check internet connection as max "
                                            "retry for cloud update has reached" );
                }
                setCloudUpdateMesgState( IDLE_UPDATE_STATE );
                setCloudProcessStatus( false );
                return;
            }
        }
    }
    break;

    case PRODUCT_UPDATE_WITH_CLOUD:
    {
        if( mesgStatus.timerExpired == true )
        {
            logMessage( "Timer Expired : Retrying for the Same Product", 1 );

            mesgStatus.retryCounter++;
            mesgStatus.timerExpired = false;
            mesgStatus.messageSent = false;
            mesgStatus.mesgResponseReceived = false;
        }

        if( mesgStatus.retryCounter >= 3 )
        {
            setCloudUpdateMesgState( IDLE_UPDATE_STATE );
            logMessage( "Please restast the Applicaiton as max retry for cloud update has reached", 1 );

            updateSystemAllowedStatus();
            if( isSystemAllowedToOperate() == false )
            {
                nonBlockingPopupMessage( ERROR_MESG,
                                         "Please check internet connection as max retry "
                                         "for cloud update has reached" );
            }
            setCloudUpdateMesgState( IDLE_UPDATE_STATE );
            setCloudProcessStatus( false );
            return;
        }

        if( !httpRequestTimer->isActive() &&
            mesgStatus.messageSent == false )
        {
            getProductInfoNeedToSendToCloud( );

            //Move to System Configuration Update incase no product update left with cloud
            if( !httpRequestTimer->isActive() )
            {
                logMessage( "Moving Formward to Sync System Configuration with cloud", 1 );
                setCloudUpdateMesgState( SYSTEM_CONFIG_UPDATE_WITH_CLOUD );
            }
        }
    }
    break;

    case SYSTEM_CONFIG_UPDATE_WITH_CLOUD:
    {
        if( mesgStatus.timerExpired == true )
        {
            logMessage( "Timer Expired : Retrying for the Same Product", 1 );
            mesgStatus.retryCounter++;
            mesgStatus.timerExpired = false;
            mesgStatus.messageSent = false;
            mesgStatus.mesgResponseReceived = false;
        }

        if( mesgStatus.retryCounter >= 3 )
        {
            setCloudUpdateMesgState( IDLE_UPDATE_STATE );
            logMessage( "Max retry for cloud update has reached", 1 );

            updateSystemAllowedStatus();
            if( isSystemAllowedToOperate() == false )
            {
                nonBlockingPopupMessage( ERROR_MESG,
                                         "Please check internet connection as max retry "
                                         "for cloud update has reached" );
            }
            setCloudUpdateMesgState( IDLE_UPDATE_STATE );
            setCloudProcessStatus( false );
            return;
        }

        if( !httpRequestTimer->isActive() &&
            mesgStatus.messageSent == false )
        {
            logMessage( "Sending Product Update Message to cloud", 1 );
            sendSystemConfigurationToCloud( false );
            mesgStatus.messageSent = true;
            mesgStatus.mesgResponseReceived = false;
            return;
        }

        if( mesgStatus.messageSent == true &&
            mesgStatus.mesgResponseReceived == true )
        {
            logMessage( "Reply received for Product Configuraiton Update", 1 );
            setCloudUpdateMesgState( IDLE_UPDATE_STATE );
            setCloudProcessStatus( false );
        }
    }
    break;
    }
}

void cloudHandeling::getProductInfoNeedToSendToCloud( )
{
    quint8 prodCount = 0, cloudUpdateStatus = 0;
    QString ioConfig, lConfig, qrCOnfig, learnData;
    st_ProductDBInfo prodInfo;
    memset( &prodInfo, '\0', sizeof( st_ProductDBInfo ) );
    DB_GetproductInfoForCloudUpdate( &prodInfo, &prodCount, &cloudUpdateStatus,
                                     &ioConfig, &lConfig, &qrCOnfig, &learnData );
    if( prodCount )
    {
        QJsonObject jsonRootObj;

        logMessage( "***************************************************", 1 );
        logMessage( QString( "Sending Product Sync Message for %1 : %2" )
                    .arg( prodInfo.productName )
                    .arg( cloudUpdateStatus ), 1 );
        logMessage( "***************************************************", 1 );

        jsonRootObj.insert( HTTP_ACTION_STR, QJsonValue::fromVariant( PRODUCT_UPDATE_HTTP_MSG ) );
        jsonRootObj.insert( HTTP_API_KEY_STR, HTTPS_AGAIN_CLOUD_SERVER_KEY );
        jsonRootObj.insert( HTTP_CLIENT_ID_STR, getClientID() );
        jsonRootObj.insert( HTTP_SYSTEM_LOCAL_TIME_STR, QDateTime::currentMSecsSinceEpoch() );
        jsonRootObj.insert( HTTP_APPLICAITON_UNIQUE_ID_STR, getApplicationUniqueID() );

        //Node Specific Configurations
        jsonRootObj.insert( DB_PROD_NAME, prodInfo.productName );

        if( CHECK_BIT_32BITs( cloudUpdateStatus, PRODUCT_TEST_COUNT_UPDATE_BIT ) )
        {
            jsonRootObj.insert( DB_SUCCESSFUL_TEST_COUNT_PARAM, prodInfo.totalTestedProduct );
        }

        if( CHECK_BIT_32BITs( cloudUpdateStatus, PROUCT_LEARN_DATA_UPDATE_BIT ) )
        {
            jsonRootObj.insert( DB_PROD_LEARN_DATA, learnData );
        }

        if( CHECK_BIT_32BITs( cloudUpdateStatus, PRODUCT_CONFIG_UPDATE_BIT ) )
        {
            jsonRootObj.insert( DB_PROD_TOTAL_NODES, prodInfo.totalNodes );
            jsonRootObj.insert( DB_PROD_TOTAL_IO, prodInfo.totalInputOutputs );
            jsonRootObj.insert( DB_PROD_IO_CONFIG, ioConfig );
            jsonRootObj.insert( DB_PROD_LEAK_CONFIG, lConfig );
            jsonRootObj.insert( DB_PROD_QR_CONFIG, qrCOnfig );
        }

        QJsonDocument jsonDoc( jsonRootObj );
        QByteArray mesgByteArray = jsonDoc.toJson();

        doHTTPPostRequest( HTTPS_AGAIN_CLOUD_SERVER_URL, QString( mesgByteArray ) );
        httpRequestTimer->start( CLOUD_RESPONSE_WAIT_PERIOD );
    }
    else
    {
        httpRequestTimer->stop();
        logMessage( "All Products Are in sync with cloud", 1 );
    }
}

void cloudHandeling::sendSystemAccessGrantRequestToCloud( void )
{
    QJsonObject jsonRootObj;
    jsonRootObj.insert( HTTP_ACTION_STR, QJsonValue::fromVariant( SYSTEM_GRNAT_HTTP_MSG ) );
    jsonRootObj.insert( HTTP_API_KEY_STR, HTTPS_AGAIN_CLOUD_SERVER_KEY );
    jsonRootObj.insert( HTTP_CLIENT_ID_STR, getClientID() );
    jsonRootObj.insert( HTTP_SYSTEM_LOCAL_TIME_STR, QDateTime::currentMSecsSinceEpoch() );
    jsonRootObj.insert( HTTP_APPLICAITON_UNIQUE_ID_STR, getApplicationUniqueID() );

    jsonRootObj.insert( HTTP_SYNC_REQUIRED_STATUS_STR, getSystemSyncStatus() );
    jsonRootObj.insert( HTTP_SYNC_ON_INTERVAL_STR, QJsonValue::fromVariant( getSyncInterval() ) );
    jsonRootObj.insert( HTTP_SYNC_TEST_COUNT_STR, QJsonValue::fromVariant( getSyncTestCount() ) );

    if( getSystemSyncStatus() == SYNC_ON_TEST_COUNT )
    {
        jsonRootObj.insert( HTTP_SUCCESSFUL_TEST_COUNT_STR, QJsonValue::fromVariant( getSuccessfulTestCount() ) );
        jsonRootObj.insert( HTTP_TOTAL_TEST_COUNT_STR, QJsonValue::fromVariant( getTotalTestCount() ) );
    }
    QJsonDocument jsonDoc( jsonRootObj );
    QByteArray mesgByteArray = jsonDoc.toJson();

    doHTTPPostRequest( HTTPS_AGAIN_CLOUD_SERVER_URL, QString( mesgByteArray ) );
    httpRequestTimer->start( CLOUD_RESPONSE_WAIT_PERIOD );
    logMessage( "SYSTEM_GRNAT_HTTP_MESSAGE Sent to Cloud", 1 );
}

void cloudHandeling::sendSystemConfigurationToCloud( bool isdefault )
{
    QJsonObject jsonRootObj;

    if( isdefault )
        jsonRootObj.insert( HTTP_ACTION_STR, QJsonValue::fromVariant( SYSTEM_DEFAULT_CONFIGURATION_HTTP_MSG ) );
    else
        jsonRootObj.insert( HTTP_ACTION_STR, QJsonValue::fromVariant( SYSTEM_CONFIGURATION_UPDATE_HTTP_MSG ) );

    jsonRootObj.insert( HTTP_API_KEY_STR, HTTPS_AGAIN_CLOUD_SERVER_KEY );
    jsonRootObj.insert( HTTP_CLIENT_ID_STR, getClientID() );
    jsonRootObj.insert( HTTP_SYSTEM_LOCAL_TIME_STR, QDateTime::currentMSecsSinceEpoch() );
    jsonRootObj.insert( HTTP_APPLICAITON_UNIQUE_ID_STR, getApplicationUniqueID() );

    jsonRootObj.insert( DB_WIFI_SSID, DB_getIntSystemConfigParam( DB_WIFI_SSID ) );
    jsonRootObj.insert( DB_WIFI_PASS, DB_getIntSystemConfigParam( DB_WIFI_PASS ) );
    jsonRootObj.insert( DB_IP1_PARAM, DB_getIntSystemConfigParam( DB_IP1_PARAM ).toInt() );
    jsonRootObj.insert( DB_IP2_PARAM, DB_getIntSystemConfigParam( DB_IP2_PARAM ).toInt() );
    jsonRootObj.insert( DB_IP3_PARAM, DB_getIntSystemConfigParam( DB_IP3_PARAM ).toInt() );
    jsonRootObj.insert( DB_IP4_PARAM, DB_getIntSystemConfigParam( DB_IP4_PARAM ).toInt() );

    jsonRootObj.insert( DB_OUTPUT_DELAY_PARAM, DB_getIntSystemConfigParam( DB_OUTPUT_DELAY_PARAM ).toInt() );
    jsonRootObj.insert( DB_DELAY_INTERVAL_PARAM, DB_getIntSystemConfigParam( DB_DELAY_INTERVAL_PARAM ).toInt() );
    jsonRootObj.insert( DB_LEAK_TEST_VAL_OFFSET_PARAM,
                       DB_getIntSystemConfigParam( DB_LEAK_TEST_VAL_OFFSET_PARAM ).toInt() );
    jsonRootObj.insert( DB_MQTT_BROAKER_IP_ADD_PARAM,
                       DB_getIntSystemConfigParam( DB_MQTT_BROAKER_IP_ADD_PARAM ) );

    jsonRootObj.insert( DB_LOCK_PERIOD_1_PARAM, DB_getIntSystemConfigParam( DB_LOCK_PERIOD_1_PARAM ).toInt() );
    jsonRootObj.insert( DB_LOCK_PERIOD_2_PARAM, DB_getIntSystemConfigParam( DB_LOCK_PERIOD_2_PARAM ).toInt() );
    jsonRootObj.insert( DB_LOCK_1_2_BYPASS_PARAM, DB_getIntSystemConfigParam( DB_LOCK_1_2_BYPASS_PARAM ).toInt() );
    jsonRootObj.insert( DB_LOCK_3_BYPASS_PARAM, DB_getIntSystemConfigParam( DB_LOCK_3_BYPASS_PARAM ).toInt() );
    jsonRootObj.insert( DB_MACHINE_SERIAL_NUM_PARAM, DB_getIntSystemConfigParam( DB_MACHINE_SERIAL_NUM_PARAM ) );
    jsonRootObj.insert( DB_LEAK_CALIBRATION_PARAM, DB_getIntSystemConfigParam( DB_LEAK_CALIBRATION_PARAM ) );
    jsonRootObj.insert( DB_RELAY_1_PERIOD_PARAM, DB_getIntSystemConfigParam( DB_RELAY_1_PERIOD_PARAM ).toInt() );
    jsonRootObj.insert( DB_DEBUG_FILE_NUM_PARAM, DB_getIntSystemConfigParam( DB_DEBUG_FILE_NUM_PARAM ).toInt() );

    jsonRootObj.insert( HTTP_CLIENT_ID_STR, DB_getIntSystemConfigParam( HTTP_CLIENT_ID_STR ) );
    jsonRootObj.insert( HTTP_APPLICAITON_UNIQUE_ID_STR,
                       DB_getIntSystemConfigParam( HTTP_APPLICAITON_UNIQUE_ID_STR ) );
    jsonRootObj.insert( HTTP_SYNC_REQUIRED_STATUS_STR,
                       QJsonValue::fromVariant( DB_getIntSystemConfigParam( HTTP_SYNC_REQUIRED_STATUS_STR ).toUInt() ) );
    jsonRootObj.insert( HTTP_SYNC_ON_INTERVAL_STR,
                       QJsonValue::fromVariant( DB_getIntSystemConfigParam( HTTP_SYNC_ON_INTERVAL_STR ).toInt() ) );
    jsonRootObj.insert( HTTP_SYNC_TEST_COUNT_STR,
                       QJsonValue::fromVariant( DB_getIntSystemConfigParam( HTTP_SYNC_TEST_COUNT_STR ).toInt() ) );
    jsonRootObj.insert( DB_SUCCESSFUL_TEST_COUNT_PARAM,
                       QJsonValue::fromVariant( DB_getIntSystemConfigParam( DB_SUCCESSFUL_TEST_COUNT_PARAM ).toInt() ) );
    jsonRootObj.insert( DB_TOTAL_TEST_COUNT_PARAM,
                       QJsonValue::fromVariant( DB_getIntSystemConfigParam( DB_TOTAL_TEST_COUNT_PARAM ).toInt() ) );

    QJsonDocument jsonDoc( jsonRootObj );
    QByteArray mesgByteArray = jsonDoc.toJson();

    doHTTPPostRequest( HTTPS_AGAIN_CLOUD_SERVER_URL, QString( mesgByteArray ) );
    httpRequestTimer->start( CLOUD_RESPONSE_WAIT_PERIOD );
    logMessage( "SYSTEM_DEFAULT_CONFIGURATION_HTTP_MESSAGE Sent to Cloud", 1 );
}
