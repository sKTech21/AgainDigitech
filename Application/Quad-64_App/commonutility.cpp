#include "commonutility.h"

#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>

#include "dbroutines.h"

const char* APPLICATION_VERSION = "00.00.01.00";

const char* HTTP_ACTION_STR = "action";
const char* HTTP_SETUP_DONE_BY_STR = "setupBy";
const char* HTTP_USER_NAME_STR = "userName";
const char* HTTP_USER_PASSWORD_STR = "password";
const char* HTTP_SYSTEM_LOCAL_TIME_STR ="systemTime";
const char* HTTP_SYSTEM_SR_NUMBER_STR = "serialNumber";
const char* HTTP_HARDWARE_MAC_STR = "hardwareMac";
const char* HTTP_CLIENT_ID_STR = "clientID";
const char* HTTP_CLIENT_LOCATION_STR = "clientLocaiton";
const char* HTTP_CLIENT_NAME_STR = "clientName";
const char* HTTP_API_KEY_STR = "apikey";
const char* HTTP_FIRST_ADMIN_USERID_STR = "adminUser";
const char* HTTP_FIRST_ADMIN_PASS_STR = "adminPassword";
const char* HTTP_ACTION_STATUS_STR = "status";
const char* HTTP_SYNC_REQUIRED_STATUS_STR = "syncRequired";
const char* HTTP_SYNC_TEST_COUNT_STR = "syncTestCount";
const char* HTTP_SYNC_ON_INTERVAL_STR = "syncAfterEveryHour";
const char* HTTP_SUCCESSFUL_TEST_COUNT_STR = "sTestCnt";
const char* HTTP_TOTAL_TEST_COUNT_STR = "tTestCnt";
const char* HTTP_ERROR_STRING_STR = "comment";
const char* HTTP_APPLICAITON_UNIQUE_ID_STR = "appUniqueID";
const char* HTTP_ERROR_LOGNOTE_STR = "lognote";
const char* HTTP_NODE_ADDITION_DELETION_STR = "nodeAOrD";
const char* HTTP_NODE_MAC_STR = "nodeMac";
const char* HTTP_NODE_NUMBER_STR = "nodeNumber";
const char* HTTP_PRODUCT_ADDITION_OR_DELETION_STR = "prodAOrD";
const char* HTTP_NODE_COUNT_STR = "nodeCount";
const char* HTTP_NODE_INFORMATION_STR = "nodeInfo";
const char* HTTP_PRODUCT_COUNT_STR = "prodCount";
const char* HTTP_LAST_SYNC_ON_STR  = "lastSyncOn";
const char* HTTP_TIME_LAST_NOTED_ON_STR = "timeLastNotedOn";

const char* DB_NODE_INFO_TABLE              = "nodeInfo";
const char* DB_NODE_UNIQUE_NUMBER           = "nodeUniqueNumber";
const char* DB_STR_NODE_NUMEBR              = "nodeNumber";
const char* DB_STR_NODE_TYPE                = "nodeType";
const char* DB_STR_NODE_MAC                 = "nodeMac";
const char* DB_STR_NODE_LOCATION            = "nodeLocation";
const char* DB_STR_PARENT_MAC               = "parentNodeMac";
const char* DB_STR_PARENT_NUMBER            = "parentNodeNumber";
const char* DB_NODE_NAME                    = "nodeName";

const char* DB_SYS_CONFIG_TABLE             = "sysConfig";
const char* DB_SYS_CONFIG_PARAM             = "param";
const char* DB_SYS_CONFIG_VALUE             = "value";
const char* DB_WIFI_SSID                    = "ssid";
const char* DB_WIFI_PASS                    = "passPhrase";
const char* DB_IP1_PARAM                    = "ip1";
const char* DB_IP2_PARAM                    = "ip2";
const char* DB_IP3_PARAM                    = "ip3";
const char* DB_IP4_PARAM                    = "ip4";
const char* DB_SHOW_SIMILAR_IP_OP_PARAM     = "dispSimilar";
const char* DB_DISP_ONLY_CHANGE_PARAM       = "onlyChange";
const char* DB_OUTPUT_DELAY_PARAM           = "opDelay";
const char* DB_LEAK_TEST_VAL_OFFSET_PARAM   = "leakOffset";
const char* DB_MQTT_BROAKER_IP_ADD_PARAM    = "broakerIP";
const char* DB_LOCK_PERIOD_1_PARAM          = "lockPeriod_1";
const char* DB_LOCK_PERIOD_2_PARAM          = "lockPeriod_2";
const char* DB_LOCK_1_2_BYPASS_PARAM        = "bypassLock1_2";
const char* DB_LOCK_3_BYPASS_PARAM          = "bypassLock3";
const char* DB_DELAY_INTERVAL_PARAM         = "delayInterval";
const char* DB_MACHINE_SERIAL_NUM_PARAM     = "machineSrNum";
const char* DB_RELAY_1_PERIOD_PARAM         = "relay1Period";
const char* DB_DEBUG_FILE_NUM_PARAM         = "debugFileNum";
const char* DB_SUCCESSFUL_TEST_COUNT_PARAM  = "sTestCnt";
const char* DB_TOTAL_TEST_COUNT_PARAM       = "tTestCnt";
const char* DB_APP_OPERATED_ON_DATE_PARAM   = "lOpDate";

const char* DB_LEAK_CALIBRATION_PARAM       = "lCalib";
const char* DB_POS_PRESS_MAX_ADC_PARAM      = "posPressMaxAdc";
const char* DB_POS_PRESS_MIN_ADC_PARAM      = "posPressMinAdc";
const char* DB_NEG_PRESS_MAX_ADC_PARAM      = "negPressMaxAdc";
const char* DB_NEG_PRESS_MIN_ADC_PARAM      = "negPressMinAdc";
const char* DB_POS_PRESS_MAX_VAL_PARAM      = "posMaxPress";
const char* DB_POS_PRESS_MIN_VAL_PARAM      = "posMinPress";
const char* DB_NEG_PRESS_MAX_VAL_PARAM      = "negMaxPress";
const char* DB_NEG_PRESS_MIN_VAL_PARAM      = "negMinPress";

const char* DB_PRODUCT_INFO_TABLE           = "prodInfo";
const char* DB_PROD_UNIQUE_ID               = "prodUID";
const char* DB_PROD_CREATED_ON              = "createdOn";
const char* DB_PROD_CREATOR                 = "creator";
const char* DB_PROD_NAME                    = "prodName";
const char* DB_PROD_TOTAL_NODES             = "tNodes";
const char* DB_DAY_WISE_PRODUCT_COUNT       = "dwCount";
const char* DB_PROD_TOTAL_IO                = "tIOs";
const char* DB_PROD_IO_CONFIG               = "ioConfig";
const char* DB_PROD_LEAK_CONFIG             = "lConfig";
const char* DB_PROD_QR_CONFIG               = "qrConfig";
const char* DB_PROD_LEARN_DATA              = "learnData";
const char* DB_PROD_LEARNED_ON              = "learnedOn";
const char* DB_PROD_UID_START_FROM_STR      = "UIDstartsOn";
const char* DB_PROD_ENABLE_DIABLE_STR       = "enDis";
const char* DB_PROD_CLOUD_UPDATE_REQ_STR    = "cldUpdate";

const char* DB_TESTED_PROD_INFO             = "testProdInfo";
const char* DB_TESTED_PROD_UNIQ_ID          = "number";
const char* DB_PROD_TESTED_ON               = "testedOn";
const char* DB_TESTED_PROD_DATA             = "contiData";
const char* DB_LEAK_RESULT_DATA             = "leakData";
const char* DB_PROD_TEST_RESULT             = "result";
const char* DB_PROD_TESTED_BY               = "testedBy";

const char* DB_USER_IFO_TABLE               = "userInfo";
const char* DB_USER_NAME_STR                = "uName";
const char* DB_USER_PASSWROD                = "phrase";

const char* JSON_PROD_IN_OUT_STR            = "inOut";
const char* JSON_LEAK_TEST_DURATION         = "leakDur";
const char* JSON_LEAK_TEST_VALUE            = "leakVal";

const char* iniFileKeyString[ MAX_INI_FILE_KEY ] = {
    HTTP_APPLICAITON_UNIQUE_ID_STR,
    HTTP_CLIENT_ID_STR,
    HTTP_SYNC_REQUIRED_STATUS_STR,
    HTTP_SYNC_TEST_COUNT_STR,
    HTTP_SYNC_ON_INTERVAL_STR,
    HTTP_LAST_SYNC_ON_STR,
    HTTP_TIME_LAST_NOTED_ON_STR,
    HTTP_FIRST_ADMIN_USERID_STR,
    HTTP_FIRST_ADMIN_PASS_STR,
    HTTP_SUCCESSFUL_TEST_COUNT_STR,
    HTTP_TOTAL_TEST_COUNT_STR
};

st_popupWin popupWin[ MAX_MESG_TYPE ] = {
    INFORMATION_MESG,       "Information",
    WARNING_MESG,           "Warning",
    CRITICAL_MESG,          "Critical",
    ERROR_MESG,             "Error"
};

st_systemState systemState;

commonUtility::commonUtility()
{
    popUpDispTimer = new QTimer();
    connect( popUpDispTimer, &QTimer::timeout, this, &commonUtility::popUpDispTimerElapsed );
    popUpDispTimer->setSingleShot( true );
}

void commonUtility::nonBlockingPopupMessage( en_PopupMesgType _mesgType, QString _strMesg )
{
    if( popUpDispTimer->isActive() )
    {
        popUpDispTimer->stop();
        popupMessages = _strMesg;
        mesgType = _mesgType;
        popUpDispTimer->start( 50 );
    }
}

void commonUtility::popUpDispTimerElapsed( )
{
    displayPopupMessage( mesgType, popupMessages );
}

QMessageBox::StandardButton commonUtility::displayUserInputMessageBox( en_PopupMesgType _mesgType, QString _strMesg )
{
    QMessageBox msgBox;
    msgBox.setWindowTitle( popupWin[ _mesgType ].title );

    msgBox.setText( _strMesg );
    msgBox.setStandardButtons( QMessageBox::Yes );
    msgBox.addButton( QMessageBox::No );
    msgBox.setDefaultButton( QMessageBox::No );
    msgBox.setStyleSheet( "color: rgb( 0, 0, 0 ); background-color: rgb( 85, 87, 83 ); "
                         "font-family: Serif; font-size: 11pt;" );
    msgBox.setMinimumSize( 500, 150 );

    if( msgBox.exec() == QMessageBox::Yes )
        return QMessageBox::Yes;
    else
        return QMessageBox::No;
}

void commonUtility::displayPopupMessage( en_PopupMesgType _mesgType, QString _strMesg )
{
    QMessageBox msgBox;
    msgBox.setWindowTitle( popupWin[ _mesgType ].title );
    msgBox.setStyleSheet( "color: rgb( 0, 0, 0 ); background-color: rgb( 85, 87, 83 ); "
                         "font-family: Serif; font-size: 11pt;" );

    msgBox.setStandardButtons( QMessageBox::Ok );
    msgBox.setText( _strMesg );
    msgBox.setMinimumSize( 500, 150 );

    if( msgBox.exec() == QMessageBox::Ok )
    {
    }
}

void commonUtility::saveIniConfigurationValues( en_iniFileParam iniParam, QString _value )
{
#ifdef LINUX_SYSTEM
    QSettings settings( INI_FILE, QSettings::IniFormat );
#else
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString filePath = QDir(appDataPath).filePath(INI_FILE);
#endif

    settings.beginGroup( INI_FILE_CONFIG_TAG );
    settings.setValue( QString( iniFileKeyString[ iniParam ] ), _value );
    settings.sync( );
    settings.endGroup( );
    logMessage( QString( "Saving : %1 -> %2" ).arg( iniFileKeyString[ iniParam ] ).arg( _value ), 1 );
}

st_appSyncInfo* commonUtility::getSysInfoObject( void )
{
    return &systemState.appSyncINfo;
}

en_syncStatus commonUtility::getSystemSyncStatus( void )
{
    return systemState.appSyncINfo.syncStatus;
}

quint32 commonUtility::getSyncTestCount( void )
{
    return systemState.appSyncINfo.syncCount;
}

quint32 commonUtility::getSyncInterval( void )
{
    return systemState.appSyncINfo.syncInterval;
}

QString commonUtility::getApplicationUniqueID( void )
{
    return systemState.appSyncINfo.appUniqueID;
}

QString commonUtility::getClientID( void )
{
    return systemState.appSyncINfo.clientID;
}

void commonUtility::setSuccessfulTestCount( quint32 _testCount )
{
    systemState.successfulTestCount = _testCount;
}

void commonUtility::increamentSuccessfulTestCount( void )
{
    systemState.successfulTestCount++;
    DB_saveSystemConfigurationParam( DB_SUCCESSFUL_TEST_COUNT_PARAM,
                                     QString::number( systemState.successfulTestCount ) );
    saveIniConfigurationValues( SUCCESSFUL_TEST_COUNT_KEY, QString::number( systemState.successfulTestCount ) );
}

quint32 commonUtility::getSuccessfulTestCount( void )
{
    return systemState.successfulTestCount;
}

void commonUtility::setTotalTestCount( quint32 _testCount )
{
    systemState.totalTestCount = _testCount;
}

void commonUtility::increamentTotalTestCount( void )
{
    systemState.totalTestCount++;
    DB_saveSystemConfigurationParam( DB_TOTAL_TEST_COUNT_PARAM,
                                     QString::number( systemState.totalTestCount ) );
    saveIniConfigurationValues( TOTAL_TEST_COUNT_KEY, QString::number( systemState.totalTestCount ) );
}

quint32 commonUtility::getTotalTestCount( void )
{
    return systemState.totalTestCount;
}

bool commonUtility::isSystemUpdateToCLoudRequired( void )
{
    if( getSystemSyncStatus() == SYNC_NOT_REQUIRED )
    {
        setSystemAllowStatus( true );
        return false;
    }
    else if( getSystemSyncStatus() == SYNC_ON_TEST_COUNT )
    {
        if( getTotalTestCount() >= getSyncTestCount() )
        {
            qDebug() << "True is returned from here" << getTotalTestCount() << getSyncTestCount();
            return true;
        }
    }
    else if( getSystemSyncStatus() == SYNC_ON_PERIODIC_INTERVAL )
    {
        quint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        quint64 lastSyncOn = systemState.lastSyncHappenedOn;
        quint64 syncIntervalInMs = getSyncInterval();

        syncIntervalInMs = syncIntervalInMs * 60 * 60 * 1000;
        quint64 timeDifference = currentTime - lastSyncOn;

        if( timeDifference >= syncIntervalInMs )
        {
            return true;
        }
    }
    return false;
}

void commonUtility::updateSystemAllowedStatus( void )
{
    if( getSystemSyncStatus() == SYNC_NOT_REQUIRED )
    {
        setSystemAllowStatus( true );
        qDebug() << "Update Sync Status " << getSystemSyncStatus();
    }
    else if( getSystemSyncStatus() == SYNC_ON_TEST_COUNT )
    {
        if( getTotalTestCount() >= getSyncTestCount() )
        {
            setSystemAllowStatus( false );
        }
        else
        {
            setSystemAllowStatus( true );
        }
        qDebug() << "Update Sync Status : SYNC_ON_TEST_COUNT : "
                 << getTotalTestCount() << getSyncTestCount() << isSystemAllowedToOperate();
    }
    else if( getSystemSyncStatus() == SYNC_ON_PERIODIC_INTERVAL )
    {
        quint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        quint64 lastSyncOn = systemState.lastSyncHappenedOn;
        quint64 syncIntervalInMs = getSyncInterval( );

        syncIntervalInMs = syncIntervalInMs * 60 * 60 * 1000;
        quint64 timeDifference = currentTime - lastSyncOn;

        if( timeDifference >= syncIntervalInMs )
        {
            setSystemAllowStatus( false );
        }
        else
        {
            setSystemAllowStatus( true );
        }
        qDebug() << "Update Sync Status : SYNC_ON_PERIODIC_INTERVAL : "
                 << timeDifference << syncIntervalInMs << isSystemAllowedToOperate();
    }
    else if( getSystemSyncStatus() == SYSTEM_ACCESS_DECLINED )
    {
        setSystemAllowStatus( false );
    }
}

void commonUtility::setSystemAllowStatus( bool _status )
{
    systemState.SystemAllowedStatus = _status;
}
bool commonUtility::isSystemAllowedToOperate( void )
{
    return systemState.SystemAllowedStatus;
}

en_processModes getCurrentRunningMode( )
{
    return systemState.currMode;
}

en_processModeState getCurrentModeState( )
{
    return systemState.currModeState;
}

bool isProcessRunning( )
{
    bool retVal = false;
    if( ( ( getCurrentRunningMode() == LEARN_MODE ) && ( getCurrentModeState() == LEARN_MODE_PROCESS_START ) ) ||
        ( ( getCurrentRunningMode() == TEST_MODE ) && ( getCurrentModeState() == TEST_MODE_PROCESS_START ) ) )
    {
        retVal = true;
    }

    return retVal;
}
