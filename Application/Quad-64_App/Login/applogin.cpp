#include "applogin.h"
#include "ui_applogin.h"
#include "../commonutility.h"
#include "../cloudhandeling.h"

#include <QLabel>

appLogin::appLogin(QWidget *parent) : QWidget(parent),
    ui(new Ui::appLogin)
{
    ui->setupUi(this);
    selectedScreen = APP_REGISTRATION_SCREEN;

    CurrentWidget = new QWidget;
    appRegistrationObj = new appRegistration( );
    appRestorationObj = new appRestoration( );
    loginObj = new login( );

    // Initialize hoverMessageLabel
    hoverMessageLabel = new QLabel( this );
    hoverMessageLabel->setFixedSize( 200, 30 ); // Adjust size as needed
    hoverMessageLabel->setAlignment( Qt::AlignCenter );
    hoverMessageLabel->hide( ); // Initially hide the label

    connect( appRegistrationObj, &appRegistration::sendRegistrationRequest,
            this, &appLogin::sendApplicationRegistartionRequestToCloud );

    updateUIParametersOnInit( );
    getHostHardwareDetails( );
}

appLogin::~appLogin( )
{
    delete appRegistrationObj;
    delete appRestorationObj;
    delete loginObj;
    delete ui;
}

void appLogin::updateUIParametersOnInit( )
{
    ui->appVersionLbl_->clear();
    ui->registrationTBtn->installEventFilter( this );
    ui->registrationTBtn_1->installEventFilter( this );

    ui->restorationTBtn->installEventFilter( this );
    ui->restorationTBtn_1->installEventFilter( this );
}

void appLogin::getHostHardwareDetails( )
{
    appRegistrationObj->updateHostHardwareDetails( getMacAddress(), QString( QSysInfo::machineUniqueId() ) );
    appRestorationObj->updateHostHardwareDetails( getMacAddress(), QString( QSysInfo::machineUniqueId() ) );
}

QString appLogin::getMacAddress()
{
    foreach(QNetworkInterface netInterface, QNetworkInterface::allInterfaces())
    {
        // Return only the first non-loopback MAC Address
        if( !( netInterface.flags() & QNetworkInterface::IsLoopBack ) )
            return netInterface.hardwareAddress( );
    }
    return QString();
}

bool appLogin::eventFilter( QObject *obj, QEvent *evnt )
{
    if( evnt->type() == QEvent::Enter )
    {
        qDebug() << "Enter : "<< obj->objectName();
        QLabel *label = qobject_cast<QLabel*>( obj );
        if( label ) {
            // Set Border color of the Label
            if( obj == ui->registrationTBtn || obj == ui->registrationTBtn_1 )
            {
                qDebug() << "Enter **: "<< obj->objectName();
                // Show hover message and position it near registrationTBtn
                hoverMessageLabel->setText("Hovering over registrationTBtn");
                hoverMessageLabel->move(ui->registrationTBtn->mapToGlobal(QPoint(0, ui->registrationTBtn->height())));
                hoverMessageLabel->show();
            }
        }
    }
    else if (evnt->type() == QEvent::Leave)
    {
        qDebug() << "Leave : "<< obj->objectName();
        QLabel *label = qobject_cast<QLabel*>( obj );
        if( label ) {
            // Set Border color of the Label
            if( obj == ui->registrationTBtn || obj == ui->registrationTBtn_1 )
            {
                qDebug() << "leave **: "<< obj->objectName();
                // Hide hover message when leaving registrationTBtn area
                hoverMessageLabel->hide();
            }
        }
    }

    if( evnt->type() == QMouseEvent::MouseButtonPress )
    {
        //Set Border color of the Lable
        QLabel *label = qobject_cast<QLabel*>( obj );
        if( label ) {
            // Set Border color of the Label
            if( obj == ui->registrationTBtn || obj == ui->registrationTBtn_1 )
                setBorderColor( ui->registrationTBtn, true, true );
            else if( obj == ui->restorationTBtn || obj == ui->restorationTBtn_1 )
                setBorderColor( ui->restorationTBtn, true, false );
        }
    }
    else if( evnt->type() == QMouseEvent::MouseButtonRelease )
    {
        //Set Border color of the Lable back to original
        QLabel *label = qobject_cast<QLabel*>( obj );
        if( label ) {
            // Set Border color of the Label
            if( obj == ui->registrationTBtn || obj == ui->registrationTBtn_1 )
                setBorderColor( ui->registrationTBtn, false, true );
            else if( obj == ui->restorationTBtn || obj == ui->restorationTBtn_1 )
                setBorderColor( ui->restorationTBtn, false, false );

            if( obj == ui->registrationTBtn || obj == ui->registrationTBtn_1 )
                on_registrationTBtn_clicked();
            else if( obj == ui->restorationTBtn || obj == ui->restorationTBtn_1 )
                on_restorationTBtn_clicked();
        }

    }
    return false;
}

void appLogin::setBorderColor( QLabel *_obj, bool _setOrReset, bool isLeftBorder )
{
    if( _setOrReset && isLeftBorder ) //Change the
        _obj->setStyleSheet( "border-top-right-radius: 5px; border-bottom-right-radius: 5px;"
                            "border-right: 5px solid gray; border-top: 5px solid gray; "
                            "border-bottom: 5px solid gray; padding: 5px; background-color: #97CADB;" );
    else if( _setOrReset && !isLeftBorder )
        _obj->setStyleSheet( "border-top-left-radius: 5px; border-bottom-left-radius: 5px;"
                            "border-left: 5px solid gray; border-top: 5px solid gray; "
                            "border-bottom: 5px solid gray; padding: 5px; background-color: #97CADB;" );
    else if( !_setOrReset && isLeftBorder )
        _obj->setStyleSheet( "border-top-right-radius: 5px; border-bottom-right-radius: 5px;"
                             "border-right: 5px solid white; border-top: 5px solid white; "
                             "border-bottom: 5px solid white; padding: 5px; background-color: #97CADB;" );
    else if( !_setOrReset && !isLeftBorder )
        _obj->setStyleSheet( "border-top-left-radius: 5px; border-bottom-left-radius: 5px;"
                            "border-left: 5px solid white; border-top: 5px solid white; "
                            "border-bottom: 5px solid white; padding: 5px; background-color: #97CADB;" );
}

void appLogin::setLoginScreen( en_loginScreen _screen )
{
    ui->errLbl->clear( );
    ui->appVersionLbl->setText( QString( "App Version : %1" ).arg( APPLICATION_VERSION ) );
    if( _screen <= APP_LOGIN_SCREEN )
        selectedScreen = _screen;
}

void appLogin::updateUIForSelectedScreen( )
{
    if( getLoginScreen() == APP_REGISTRATION_SCREEN )
        on_registrationTBtn_clicked();
    else if( getLoginScreen() == APP_RESTORATION_SCREEN )
        on_restorationTBtn_clicked();
    else if( getLoginScreen() == APP_LOGIN_SCREEN )
        on_loginScreen_selected();
}

en_loginScreen appLogin::getLoginScreen( )
{
    return selectedScreen;
}

void appLogin::beforeOptionClose( )
{
    switch( getLoginScreen( ) )
    {
        case APP_REGISTRATION_SCREEN:
        {

        }
        break;

        case APP_RESTORATION_SCREEN:
        {

        }
        break;

        case APP_LOGIN_SCREEN:
        {

        }
        break;
    }
}

void appLogin::updateWidgetToSelectedScreen( QWidget *_widget )
{
    ui->loginFrame->layout()->removeWidget( _widget );
    _widget->setVisible( false );

    switch( getLoginScreen() )
    {
        case APP_REGISTRATION_SCREEN:
        {
            ui->registrationFrame->setVisible( false );
            ui->restorationFrame->setVisible( true );
            ui->loginFrame->layout()->addWidget( appRegistrationObj );
            appRegistrationObj->setVisible( true );
            logMessage( "APP_REGISTRATION_SCREEN Set", 0 );
        }
        break;

        case APP_RESTORATION_SCREEN:
        {
            ui->restorationFrame->setVisible( false );
            ui->registrationFrame->setVisible( true );

            ui->loginFrame->layout()->addWidget( appRestorationObj );
            appRestorationObj->setVisible( true );
            logMessage( "APP_RESTORATION_SCREEN Set", 0 );
        }
        break;

        case APP_LOGIN_SCREEN:
        {
            ui->restorationFrame->setVisible( false );
            ui->registrationFrame->setVisible( false );

            ui->loginFrame->layout()->addWidget( loginObj );
            loginObj->setVisible( true );
            logMessage( "APP_LOGIN_SCREEN Set", 1 );
        }
        break;
    }
}

void appLogin::on_registrationTBtn_clicked( )
{
    beforeOptionClose( );
    setLoginScreen( APP_REGISTRATION_SCREEN );
    updateWidgetToSelectedScreen( CurrentWidget );
    CurrentWidget = appRegistrationObj;
    logMessage( "Registration screen set", 0 );
}

void appLogin::on_restorationTBtn_clicked()
{
    beforeOptionClose( );
    setLoginScreen( APP_RESTORATION_SCREEN );
    updateWidgetToSelectedScreen( CurrentWidget );
    CurrentWidget = appRestorationObj;
    logMessage( "Restoration screen set", 0 );
}

void appLogin::on_loginScreen_selected()
{
    beforeOptionClose( );
    setLoginScreen( APP_LOGIN_SCREEN );
    updateWidgetToSelectedScreen( CurrentWidget );
    CurrentWidget = loginObj;
    logMessage( "Login screen set", 0 );
}

void appLogin::sendApplicationRegistartionRequestToCloud( )
{
    QJsonObject jsonRootObj;
    jsonRootObj.insert( HTTP_API_KEY_STR, HTTPS_AGAIN_CLOUD_SERVER_KEY );
    jsonRootObj.insert( HTTP_ACTION_STR, QJsonValue::fromVariant( APP_REGISTRATION_HTTP_MSG ) );

    QJsonObject setupByObject;
    setupByObject.insert( HTTP_USER_NAME_STR, appRegistrationObj->getManUserID() );
    setupByObject.insert( HTTP_USER_PASSWORD_STR, appRegistrationObj->getManPassword() );
    jsonRootObj.insert( HTTP_SETUP_DONE_BY_STR, setupByObject );

    jsonRootObj.insert( HTTP_SYSTEM_LOCAL_TIME_STR, QDateTime::currentMSecsSinceEpoch() );
    jsonRootObj.insert( HTTP_SYSTEM_SR_NUMBER_STR, QString( QSysInfo::machineUniqueId() ) );
    jsonRootObj.insert( HTTP_HARDWARE_MAC_STR, getMacAddress() );
    jsonRootObj.insert( HTTP_CLIENT_ID_STR, appRegistrationObj->getClientID() );
    jsonRootObj.insert( HTTP_CLIENT_LOCATION_STR, appRegistrationObj->getClientName() );
    jsonRootObj.insert( HTTP_CLIENT_NAME_STR, appRegistrationObj->getClientLocation() );

    QJsonDocument jsonDoc( jsonRootObj );
    QByteArray mesgByteArray = jsonDoc.toJson();

    logMessage( "Sending Registration Request", 1 );
    emit httpPOSTRequest( HTTPS_AGAIN_CLOUD_SERVER_URL, QString( mesgByteArray ) );
}

void appLogin::httpReplyHandlerInAppLogin( QString _mesgFromCloud, bool _status )
{
    if( _status != true )
    {
        logMessage( QString( "Failed Response In AppLogin: " ).arg( _mesgFromCloud ), 1 );
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

    logMessage( QString( "HTTP action In AppLogin: " ).arg( httpAction ), 1 );

    if( !jsonObj.isEmpty() && !keyNotPresent )
    {
        switch( httpAction )
        {
        case SYSTEM_RESTORATION_REQUEST_HTTP_MSG:
        case APP_REGISTRATION_HTTP_MSG:
        {
            //Check if status is not success then send the received error to the applogin screen
            if( jsonObj[ HTTP_ACTION_STATUS_STR ].toVariant().toUInt() != 0 &&
                appRegistrationObj->isRegistrationRequestSentToCloud() )
                appRegistrationObj->updateRegistrationResponseOnUI(
                    jsonObj[ HTTP_ERROR_LOGNOTE_STR ].toVariant().toString() );

            //Validate the received response from cloud
            if( checkIfAppRegistrationResponseValid( _mesgFromCloud ) != true )
            {
                keyNotPresent = true;
            }
            else
            {
                if( httpAction != SYSTEM_RESTORATION_REQUEST_HTTP_MSG )
                    logMessage( "Fresh setup has been created", 1 );
                else
                    logMessage( "Setup restoration has been started", 1 );

                st_appSyncInfo * sysInfo = getSysInfoObject();

                QString appUniqueID = jsonObj[ HTTP_APPLICAITON_UNIQUE_ID_STR ].toVariant().toString();
                QString clientID = jsonObj[ HTTP_CLIENT_ID_STR ].toVariant().toString();
                quint8 syncStatus = jsonObj[ HTTP_SYNC_REQUIRED_STATUS_STR ].toVariant().toUInt();
                quint64 syncTestCount = jsonObj[ HTTP_SYNC_TEST_COUNT_STR ].toVariant().toULongLong();
                quint64 syncInterval = jsonObj[ HTTP_SYNC_ON_INTERVAL_STR ].toVariant().toULongLong();

                sysInfo->syncCount = syncTestCount;
                sysInfo->syncStatus = ( en_syncStatus )syncStatus;
                sysInfo->syncInterval = syncInterval;
                memcpy( sysInfo->appUniqueID, qPrintable( appUniqueID ), appUniqueID.length() );
                memcpy( sysInfo->clientID, qPrintable( clientID ), clientID.length() );

                setSuccessfulTestCount( 0 );
                setTotalTestCount( 0 );

                if( sysInfo->syncStatus != SYSTEM_ACCESS_DECLINED )
                {
                    setSystemAllowStatus( true );
                    logMessage( QString( "Set system allow status %1" ).arg( true ), 1 );
                }

                //Send Successful registration event back to HomePage to Proceed Further
                //by creating object of each scren
                emit registrationCompletedSuccessfully();
                logMessage( "Fresh setup is done on receiving message from cloud", 1 );
            }
        }
        break;
        }
    }
    if( keyNotPresent )
        nonBlockingPopupMessage( ERROR_MESG, "Wrong message Receive" );
        //@TODO: Replace with Error Code
}

bool appLogin::checkIfAppRegistrationResponseValid( QString _mesgFromCloud )
{
    QJsonDocument   jsonDoc = QJsonDocument::fromJson( _mesgFromCloud.toUtf8() );
    QJsonObject     jsonObj = jsonDoc.object();

    if( jsonObj.contains( HTTP_APPLICAITON_UNIQUE_ID_STR ) &&
        jsonObj.contains( HTTP_FIRST_ADMIN_USERID_STR ) &&
        jsonObj.contains( HTTP_FIRST_ADMIN_PASS_STR ) &&
        jsonObj.contains( HTTP_SYNC_REQUIRED_STATUS_STR ) &&
        jsonObj.contains( HTTP_SYNC_TEST_COUNT_STR ) &&
        jsonObj.contains( HTTP_SYNC_ON_INTERVAL_STR ) )
    {
        return true;
    }
    return false;
}
