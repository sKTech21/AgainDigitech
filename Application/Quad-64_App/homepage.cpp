#include "homepage.h"
#include "./ui_homepage.h"
#include "commonutility.h"

#include <QSqlDatabase>

HomePage::HomePage(QWidget *parent) : QMainWindow(parent)
    , ui(new Ui::HomePage)
{
    ui->setupUi(this);

    appInitDone = false;
    cloudHandelingObj = new cloudHandeling( );
    connect( cloudHandelingObj, &cloudHandeling::httpReplyFromCloud, this, &HomePage::httpReplyHandlerInHomePage );

    if( isApplicationRegistrationRequired() )
    {
        /* Just Create the Cloud handleing Object, Once We receive the positive ack from cloud, We will create
           other Object */
        objAppLogin = new appLogin();
        connect( objAppLogin, &appLogin::httpPOSTRequest, cloudHandelingObj, &cloudHandeling::doHTTPPostRequest );
        objAppLogin->setLoginScreen( APP_REGISTRATION_SCREEN );
        objAppLogin->updateUIForSelectedScreen( );
        objAppLogin->setVisible( true );
        objAppLogin->show( );
    }
    else
    {
        //loadINIFileConfiguration();
        //initObjectsOnPowerON( );
    }
}

HomePage::~HomePage()
{
    delete ui;
}

//Alter the INI Value for database creation
bool HomePage::isApplicationRegistrationRequired( )
{
    logMessage( "Database File not present", 1 );
    if( !QFile::exists( DB_FILE_NAME ) )
    {
        return true;
    }
    return false;
}

void HomePage::initObjectsOnPowerON( )
{

}

void HomePage::createNewDatabaseFile( )
{
    QDir databasePath;
    QString path = databasePath.currentPath() + '/' + DB_FILE_NAME ;
    logMessage( "Creating new database file", 1 );
    QSqlDatabase db = QSqlDatabase::addDatabase( "QSQLITE" );
    db.setDatabaseName( path );
    db.open( );
    db.close( );
}

void HomePage::registrationStatusHandler( )
{
    logMessage( "Registration done successfully, Creating Objects of classes", 1 );
    createNewDatabaseFile( );
    objAppLogin->close( );
    initObjectsOnPowerON( );
}

void HomePage::httpReplyHandlerInHomePage( QString _mesgFromCloud, bool _status )
{
    logMessage( QString( "\n\nMesg from cloud HP: %1 , %2" ).arg( _status ).arg( _mesgFromCloud ), 1 );

    if( _status != true )
    {
        logMessage( QString( "Failed Response : " ).arg( _mesgFromCloud ), 1 );
        return;
    }

    QJsonDocument   jsonDoc = QJsonDocument::fromJson( _mesgFromCloud.toUtf8() );
    QJsonObject     jsonObj = jsonDoc.object();
    quint8          httpAction = MAX_INVALID_HTTP_CMD;
    bool            keyNotPresent = false;

    if( jsonObj.contains( HTTP_ACTION_STR ) )
        httpAction = jsonObj[ HTTP_ACTION_STR ].toVariant().toUInt();
    else
        keyNotPresent = true;

    if( ( !jsonObj.contains( HTTP_ACTION_STATUS_STR ) ||
          jsonObj[ HTTP_ACTION_STATUS_STR ].toVariant().toUInt() != 0 ) &&
        SYSTEM_RESTORATION_REQUEST_HTTP_MSG != httpAction )
    {
        if( jsonObj.contains( HTTP_ERROR_LOGNOTE_STR ) )
            nonBlockingPopupMessage( ERROR_MESG,
                                     QString( "%1" ).arg( jsonObj[ HTTP_ERROR_LOGNOTE_STR ].toVariant().toString() ) );
        return;
    }

    logMessage( QString( "HTTP action : " ).arg( httpAction ), 1 );
    if( !jsonObj.isEmpty() && !keyNotPresent )
    {
        switch( httpAction )
        {
        /*
        case SYSTEM_GRNAT_HTTP_MSG:
        {
            if( jsonObj.contains( HTTP_APPLICAITON_UNIQUE_ID_STR ) )
            {
                quint8 syncStatus = jsonObj[ HTTP_SYNC_REQUIRED_STATUS_STR ].toVariant().toUInt();

                st_appSyncInfo * sysInfo = getSysInfoObject();
                sysInfo->syncStatus = ( en_syncStatus )syncStatus;
                saveIniConfigurationValues( SYNC_REQUIRED_STATUS_KEY, QString::number( sysInfo->syncStatus ) );
                DB_saveIntSystemConfigParam( HTTP_SYNC_REQUIRED_STATUS_STR, QString::number( sysInfo->syncStatus ) );

                if( sysInfo->syncStatus == SYNC_NOT_REQUIRED )
                {
                    setSystemAllowStatus( true );
                }
                else if( jsonObj.contains( HTTP_SYNC_TEST_COUNT_STR ) &&
                         sysInfo->syncStatus == SYNC_ON_TEST_COUNT )
                {
                    quint32 syncCount = jsonObj[ HTTP_SYNC_TEST_COUNT_STR ].toVariant().toUInt();
                    sysInfo->syncCount = syncCount;

                    saveIniConfigurationValues( SYNC_TEST_COUNT_KEY, QString::number( sysInfo->syncCount ) );
                    DB_saveIntSystemConfigParam( HTTP_SYNC_TEST_COUNT_STR, QString::number( sysInfo->syncCount ) );

                    quint32 totalTests = dbRoutines::DB_getIntSystemConfigParam( HTTP_TOTAL_TEST_COUNT_STR ).toULong();
                    if( totalTests >= sysInfo->syncCount )
                    {
                        setSystemAllowStatus( false );
                        openLoginPage( );
                        nonBlockingPopupMessage( ERROR_MESG,
                                                 "Test Counts are exceeded, Please Contact manufacturer" );
                    }
                    else
                    {
                        setSystemAllowStatus( true );
                    }
                }
                else if( jsonObj.contains( HTTP_SYNC_ON_INTERVAL_STR ) &&
                         sysInfo->syncStatus == SYNC_ON_PERIODIC_INTERVAL )
                {
                    quint32 syncInterval = jsonObj[ HTTP_SYNC_ON_INTERVAL_STR ].toVariant().toUInt();
                    sysInfo->syncInterval = syncInterval;
                    saveIniConfigurationValues( SYNC_INTEVAL_KEY, QString::number( sysInfo->syncInterval ) );
                    DB_saveIntSystemConfigParam( HTTP_SYNC_ON_INTERVAL_STR,
                                                QString::number( sysInfo->syncInterval ) );
                    setSystemAllowStatus( true );
                }
                else if( sysInfo->syncStatus == SYSTEM_ACCESS_DECLINED )
                {
                    setSystemAllowStatus( false );
                    openLoginPage();
                }

                quint64 currentTime = QDateTime::currentMSecsSinceEpoch();
                saveIniConfigurationValues( LAST_SYNC_ON_IN_EPOCH_KEY, QString::number( currentTime ) );
                saveIniConfigurationValues( LAST_TIME_READIN_ON_KEY, QString::number( currentTime ) );
            }
        }
        break;
        */
        }
    }

    if( keyNotPresent )
        nonBlockingPopupMessage( ERROR_MESG, "Wrong message Receive" );
}

/*
1). User Logo For Logout purpose
2). Settings
3). Hardware Debugging
  1). Self Hardware Assessment
  2). Continuous Live debugging at Board Side or Fixture side.
4). Libraries[ Connector and Wire color Settings ]
5). History
6). Node Configuration Or Node Details.
7). User Management.

Home Page
1). Left Side Vertical Pane
2). Single Frame
    1). Node Information Group.
        This will be a long horizontal frame with left and right arrow to know status about Nodes
        This will contain following information
        1). Overall Sync Status
            User can perform Sync operation from here as well.

      1). Hardware Power reset self Assessment Status
      2). Sync Clock Value
      3). Over All Sync Status
      4). Number of Online Nodes
      5). Error Received.

*/
