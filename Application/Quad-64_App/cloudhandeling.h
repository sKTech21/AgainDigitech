#ifndef CLOUDHANDELING_H
#define CLOUDHANDELING_H

#include <QObject>
#include <QtNetwork>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#define CLOUD_RESPONSE_WAIT_PERIOD          ( 6000 ) //In ms

extern const char* HTTPS_AGAIN_CLOUD_SERVER_URL;
extern const char* HTTPS_AGAIN_CLOUD_SERVER_KEY;

typedef enum
{
    APP_REGISTRATION_HTTP_MSG = 0,
    NODE_ADD_OR_DELETE_HTTP_MSG = 1,
    PRODUCT_ADDITION_OR_DELETION_HTTP_MSG = 2,
    PRODUCT_UPDATE_HTTP_MSG = 3,
    SYSTEM_DEFAULT_CONFIGURATION_HTTP_MSG = 4,
    SYSTEM_CONFIGURATION_UPDATE_HTTP_MSG = 5,
    SYSTEM_GRNAT_HTTP_MSG = 6,

    SYSTEM_RESTORATION_REQUEST_HTTP_MSG = 7,
    NODE_INFO_IN_RESTORATION_HTTP_MSG = 8,
    PRODUCT_INFO_IN_RESTORATION_HTTP_MSG = 9,
    SYSTEM_CONFIGURAITON_IN_HTTP_MSG = 10,
    DISABLE_SYSTEM_RESTORATION_HTTP_MSG = 11,

    MAX_INVALID_HTTP_CMD
}en_httpActionCmd;

typedef enum
{
    SYNC_NOT_REQUIRED,
    SYNC_ON_TEST_COUNT,
    SYNC_ON_PERIODIC_INTERVAL,
    SYSTEM_ACCESS_DECLINED,
}en_syncStatus;

typedef enum
{
    NO_ERROR,
    SYSTEM_TIME_NOT_UPTO_DATE,
    MANF_CRED_NOT_MATCHING,
    SYSTEM_DOES_NOT_EXIST_ON_CLOUD,
    PRODUCT_ALREADY_EXIST,
    PRODUCT_DOES_NOT_EXIST,
    NODE_DOES_NOT_EXIST
}httpActionOrError_en;

typedef enum
{
    IDLE_UPDATE_STATE,
    SYSTEM_ACCESS_GRANT_FROM_CLOUD,
    PRODUCT_UPDATE_WITH_CLOUD,
    SYSTEM_CONFIG_UPDATE_WITH_CLOUD,
    MAX_UPDATE_WITH_CLOUD
}en_UpdateState;

typedef struct
{
    quint16         erroNo;
    QString         errMessage;
}st_HTTPErr;

typedef struct
{
    en_UpdateState  updateState;
    quint64         stateEnteredOn;
    bool            messageSent;
    bool            mesgResponseReceived;
    quint8          retryCounter;
    bool            timerExpired;
}cloudMesgStatus;

class cloudHandeling : public QObject
{
    Q_OBJECT

public:
    cloudHandeling();
    ~cloudHandeling();

    cloudMesgStatus mesgStatus;
    void httpReplyHandler(QString _mesgFromCloud, bool _status);
    void cloudUpdateRoutine();
    quint8 getCloudUpdateMesgState();
    void sendSystemAccessGrantRequestToCloud();
    void sendSystemConfigurationToCloud(bool isdefault);
    void setCloudProcessStatus( bool _status );
    bool isCloudProcessOngoing( void );

public slots:
    void doHTTPPostRequest(QString _url, QString _payload);

signals:
    void httpReplyFromCloud( QString _mesgFromCloud, bool _status );

private slots:
    void doRequest();
    void replyFinished(QNetworkReply* reply);
    void httpRequestTimerElapsed();

private:
    QPointer< QNetworkAccessManager > networkAccManager;
    QUrl        cloudServerURL, redirectedCloudURL;
    QByteArray  jsonByteArrActualMesg;
    bool        isRequestOngoing;
    bool        isCloudSyncProcessOngoing;
    QTimer      *httpRequestTimer;

    QNetworkAccessManager *createNetworkAccessManager();
    QUrl redirectUrl( const QUrl& possibleRedirectUrl, const QUrl& oldRedirectUrl ) const;
    void getProductInfoNeedToSendToCloud();
    void setCloudUpdateMesgState(en_UpdateState _state);
    quint64 getStateDuration();
};

#endif // CLOUDHANDELING_H
