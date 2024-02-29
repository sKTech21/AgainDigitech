#ifndef COMMONUTILITY_H
#define COMMONUTILITY_H

#include <QObject>
#include <QMessageBox>
#include <QTimer>

#include "cloudhandeling.h"
extern const char* APPLICATION_VERSION;

//#define WINDOWS_SYSTEM
#define LINUX_SYSTEM

#define SET_BIT_32BITs( a, pos )            ( ( a ) |=  ( 1U << ( pos ) ) )
#define CLEAR_BIT_32BITs(a, pos)            ( ( a ) &= ( ~( 1U << pos ) ) )
#define CHECK_BIT_32BITs(a, pos)            ( ( a ) & ( 1UL << pos ) )

#define BIT0                                ( 0 )
#define BIT1                                ( 1 )
#define BIT2                                ( 2 )
#define BIT3                                ( 3 )
#define BIT4                                ( 4 )
#define BIT5                                ( 5 )
#define BIT6                                ( 6 )
#define BIT7                                ( 7 )
#define BIT8                                ( 8 )
#define BIT9                                ( 9 )
#define BIT10                               ( 10 )
#define BIT11                               ( 11 )
#define BIT12                               ( 12 )
#define BIT13                               ( 13 )
#define BIT14                               ( 14 )
#define BIT15                               ( 15 )

#define DATE_FIELD_LENGTH                   ( 32 )
#define PRODUCT_NAME_LENGTH                 ( 64 )
#define PRODUCT_CREATER_NAME_LENGTH         ( 32 )
#define NODE_NAME_LENGTH                    ( 16 )
#define NODE_LOCATION_LENGTH                ( 32 )
#define MAC_STRING_LENGTH                   ( 12 )
#define DB_MAC_STRING_LENGTH                ( ( MAC_STRING_LENGTH * 2 ) + 1 )

#define MAX_ALLOWED_NODES                   ( 80 )
#define MAX_INPUT_OUTPUT_IN_NODE            ( 64 )
#define MAX_OUTPUT_IN_NODE                  ( 64 )
#define MAX_INPUT_IN_NODE                   ( 64 )
#define MAX_NODE_INPUT_VALUES               ( MAX_ALLOWED_NODES * 2 )

#define MAX_COORDINATE_STR_LENGTH           ( 4 )
#define MAX_FONT_SIZE_STR_LENGTH            ( 2 )
#define MAX_TEXT_FIELD_KEY_LENGTH           ( 32 )
#define MAX_TEXT_FIELD_VAL_LENGTH           ( 64 )
#define MAX_EXTRA_TEXT_FIELDS               ( 7 )

#define NODE_CONN_CHECK_PERIOD              ( 5 )        // 5 Secs
#define NODE_CONN_TIMEOUT_PERIOD            ( 15*1000 )  //15 Secs
#define MAX_ALLOWED_WAIT_INTERVAL           ( 1*1000 )   // 1 Secs
#define MAX_LOCAL_SYNC_INTERVAL             ( 30 * 60 * 1000 )

#define APP_LOGIN_UNAME_LENGTH              ( 16 )
#define APP_LOGIN_PASS_LENGTH               ( 16 )
#define MACHINE_SER_NUM_LENGTH              ( 32 )

#define INI_PARAM_MAX_STR_LENGTH            ( 64 )

#define MAX_PRODUCT_LOAD_COUNT              ( 2000 )

#define PRODUCT_TEST_COUNT_UPDATE_BIT       ( BIT0 )
#define PROUCT_LEARN_DATA_UPDATE_BIT        ( BIT1 )
#define PRODUCT_CONFIG_UPDATE_BIT           ( BIT2 )

#define MESSAGE_ON_SCREEN_TIMEOUT           ( 3000 )    //5 Seconds

#define DB_FILE_NAME                        "SWAT-1.db"
#define INI_FILE_CONFIG_TAG                 "TAG1"

#ifdef LINUX_SYSTEM
    #define INI_FILE                        "/lib/firmware/essPass.ini"
#else
    #define INI_FILE                        "essPass.ini"
#endif

extern const char* HTTP_ACTION_STR;
extern const char* HTTP_SETUP_DONE_BY_STR;
extern const char* HTTP_USER_NAME_STR;
extern const char* HTTP_USER_PASSWORD_STR;
extern const char* HTTP_SYSTEM_LOCAL_TIME_STR;
extern const char* HTTP_SYSTEM_SR_NUMBER_STR;
extern const char* HTTP_HARDWARE_MAC_STR;
extern const char* HTTP_CLIENT_ID_STR;
extern const char* HTTP_CLIENT_NAME_STR;
extern const char* HTTP_CLIENT_LOCATION_STR;
extern const char* HTTP_API_KEY_STR;
extern const char* HTTP_FIRST_ADMIN_USERID_STR;
extern const char* HTTP_FIRST_ADMIN_PASS_STR;
extern const char* HTTP_ACTION_STATUS_STR;
extern const char* HTTP_SYNC_REQUIRED_STATUS_STR;
extern const char* HTTP_SYNC_TEST_COUNT_STR;
extern const char* HTTP_SYNC_ON_INTERVAL_STR;
extern const char* HTTP_SUCCESSFUL_TEST_COUNT_STR;
extern const char* HTTP_TOTAL_TEST_COUNT_STR;
extern const char* HTTP_ERROR_STRING_STR;
extern const char* HTTP_APPLICAITON_UNIQUE_ID_STR;
extern const char* HTTP_ERROR_LOGNOTE_STR;
extern const char* HTTP_NODE_ADDITION_DELETION_STR;
extern const char* HTTP_NODE_MAC_STR;
extern const char* HTTP_NODE_NUMBER_STR;
extern const char* HTTP_PRODUCT_ADDITION_OR_DELETION_STR;
extern const char* HTTP_NODE_COUNT_STR;
extern const char* HTTP_NODE_INFORMATION_STR;
extern const char* HTTP_PRODUCT_COUNT_STR;

extern const char* DB_NODE_INFO_TABLE;
extern const char* DB_NODE_UNIQUE_NUMBER;
extern const char* DB_STR_NODE_NUMEBR;
extern const char* DB_STR_NODE_TYPE;
extern const char* DB_STR_NODE_MAC;
extern const char* DB_STR_NODE_LOCATION;
extern const char* DB_STR_PARENT_MAC;
extern const char* DB_STR_PARENT_NUMBER;
extern const char* DB_NODE_NAME;
extern const char* DB_SYS_CONFIG_TABLE;
extern const char* DB_SYS_CONFIG_PARAM;
extern const char* DB_SYS_CONFIG_VALUE;
extern const char* DB_WIFI_SSID;
extern const char* DB_WIFI_PASS;
extern const char* DB_IP1_PARAM;
extern const char* DB_IP2_PARAM;
extern const char* DB_IP3_PARAM;
extern const char* DB_IP4_PARAM;
extern const char* DB_SHOW_SIMILAR_IP_OP_PARAM;
extern const char* DB_DISP_ONLY_CHANGE_PARAM;
extern const char* DB_OUTPUT_DELAY_PARAM;
extern const char* DB_LEAK_TEST_VAL_OFFSET_PARAM;
extern const char* DB_MQTT_BROAKER_IP_ADD_PARAM;
extern const char* DB_LOCK_PERIOD_1_PARAM;
extern const char* DB_LOCK_PERIOD_2_PARAM;
extern const char* DB_LOCK_1_2_BYPASS_PARAM;
extern const char* DB_LOCK_3_BYPASS_PARAM;
extern const char* DB_DELAY_INTERVAL_PARAM;
extern const char* DB_MACHINE_SERIAL_NUM_PARAM;
extern const char* DB_RELAY_1_PERIOD_PARAM;
extern const char* DB_DEBUG_FILE_NUM_PARAM;
extern const char* DB_SUCCESSFUL_TEST_COUNT_PARAM;
extern const char* DB_TOTAL_TEST_COUNT_PARAM;
extern const char* DB_APP_OPERATED_ON_DATE_PARAM;
extern const char* DB_LEAK_CALIBRATION_PARAM;
extern const char* DB_POS_PRESS_MAX_ADC_PARAM;
extern const char* DB_POS_PRESS_MIN_ADC_PARAM;
extern const char* DB_NEG_PRESS_MAX_ADC_PARAM;
extern const char* DB_NEG_PRESS_MIN_ADC_PARAM;
extern const char* DB_POS_PRESS_MAX_VAL_PARAM;
extern const char* DB_POS_PRESS_MIN_VAL_PARAM;
extern const char* DB_NEG_PRESS_MAX_VAL_PARAM;
extern const char* DB_NEG_PRESS_MIN_VAL_PARAM;
extern const char* DB_PRODUCT_INFO_TABLE;
extern const char* DB_PROD_UNIQUE_ID;
extern const char* DB_PROD_CREATED_ON;
extern const char* DB_PROD_CREATOR;
extern const char* DB_PROD_NAME;
extern const char* DB_PROD_TOTAL_NODES;
extern const char* DB_DAY_WISE_PRODUCT_COUNT;
extern const char* DB_PROD_TOTAL_IO;
extern const char* DB_PROD_IO_CONFIG;
extern const char* DB_PROD_LEAK_CONFIG;
extern const char* DB_PROD_QR_CONFIG;
extern const char* DB_PROD_LEARN_DATA;
extern const char* DB_PROD_LEARNED_ON;
extern const char* DB_PROD_UID_START_FROM_STR;
extern const char* DB_PROD_ENABLE_DIABLE_STR;
extern const char* DB_PROD_CLOUD_UPDATE_REQ_STR;
extern const char* DB_TESTED_PROD_INFO;
extern const char* DB_TESTED_PROD_UNIQ_ID;
extern const char* DB_PROD_TESTED_ON;
extern const char* DB_TESTED_PROD_DATA;
extern const char* DB_LEAK_RESULT_DATA;
extern const char* DB_PROD_TEST_RESULT;
extern const char* DB_PROD_TESTED_BY;
extern const char* DB_USER_IFO_TABLE;
extern const char* DB_USER_NAME_STR;
extern const char* DB_USER_PASSWROD;
extern const char* JSON_PROD_IN_OUT_STR;
extern const char* JSON_LEAK_TEST_DURATION;
extern const char* JSON_LEAK_TEST_VALUE;

typedef enum
{
    DASHBOARD_SCREEN =   0,
    NODE_INFO_SCREEN,
    NODE_CONFIG_SCREEN,
    PRODUCT_CREATE_EDIT_SCREEN,
    LEARN_MODE_SCREEN,
    TEST_MODE_SCREEN,
    NAVIGATION_FLOW_SCREEN,
    SETTINGS_SCREEN,
    LOGIN_SCREEN,
    MAX_SCREEN
}en_screen;

typedef enum
{
    IDLE_MODE = 0,
    LEARN_MODE,
    TEST_MODE,
    NAVIGATION_MODE,
    NODE_CONFIG_MODE,
    MAX_SYSTEM_MODE
}en_processModes;

typedef enum
{
    IDLE_MODE_STATE = 0,

    LEARN_MODE_IDLE,
    LEARN_MODE_PRODUCT_SELECTED,
    LEARN_MODE_PROD_CONFIG_SENT,
    LEARN_MDOE_PRODUCT_LOADED,
    LEARN_MODE_PROCESS_START,
    LEARN_MODE_PROCESS_COMPLETE,
    LEARN_MODE_STOPPED,

    TEST_MODE_IDLE,
    TEST_MODE_PRODUCT_SELECTED,
    TEST_MODE_PROD_CONFIG_SENT,
    TEST_MODE_PRODUCT_LOADED,
    TEST_MODE_PROCESS_START,
    TEST_MODE_PROCESS_COMPLETE,
    TEST_MODE_TEST_PASS,             //If Pass in both Continuity and leak test
    TEST_MODE_TEST_FAILED,           //If Failed in both Continuity and leak test
    TEST_MODE_CONTINUITY_TEST_FAILED,//If Failed in only continuity test
    TEST_MODE_LEAK_TEST_FAILED,      //If Failed in leak test
    TEST_MODE_STOPPED,               //If process stopped by pressing on stop button

    PROCESS_RETRY_FAILED,
    PROCESS_STOPPED_DUE_TO_NO_ACK,
    MAX_PROCESS_MODE_STATE
}en_processModeState;

typedef enum
{
    APPLICATION_UNIQUE_ID_KEY,
    CLIENT_ID_KEY,
    SYNC_REQUIRED_STATUS_KEY,
    SYNC_TEST_COUNT_KEY,
    SYNC_INTEVAL_KEY,
    LAST_SYNC_ON_IN_EPOCH_KEY,
    LAST_TIME_READIN_ON_KEY,
    ADMIN_USER_ID_KEY,
    ADMIN_USER_PASS_KEY,
    SUCCESSFUL_TEST_COUNT_KEY,
    TOTAL_TEST_COUNT_KEY,
    MAX_INI_FILE_KEY
}en_iniFileParam;

typedef enum
{
    INFORMATION_MESG,
    WARNING_MESG,
    CRITICAL_MESG,
    ERROR_MESG,
    MAX_MESG_TYPE,
}en_PopupMesgType;

typedef enum
{
    INI_FILE_NOT_PRESENT = 101,
    INI_KEYS_NOT_PRESENT = 102,
    APPLICATION_AUTHENITCATION_FAILED = 103,
    APPLICATION_DEFAULT_UNIQUE_ID = 104,
    DB_ALREADY_CREATED = 105,
    DATABASE_FILE_NOT_PRESENT = 106,
    INI_CONFIG_NOT_MATCHING_WITHDB = 107,

    DATABASE_QUERY_EXECUTION_ERROR = 201,

    HTTP_MESSAGE_NOT_VALID = 301,
    CLOUD_COMMUNICATION_ERROR = 302,

    KEY_MISMATCH_FOUND_WITH_DB = 401,
    SYSTEM_TIME_MANUALLY_MODIFIED = 402,
    DATA_FIELD_MISSING_IN_OBJECT = 403,
}en_AppErrorCode;

typedef enum
{
    restorationInit = 0,
    restoreNodeInfo,
    restoreProductInfo,
    restoreSystemConfig,
    restoreDisable,
    restoreIdle,
}en_restoreState;

typedef struct
{
    quint8  mesgType;
    char    title[ 16 ];
}st_popupWin;

typedef enum
{
    QR_NOT_REQUIRED,
    QR_PRINT_SIZE_NOT_CONFIGURED,
    QR_TEXT_FIELD_NOT_PRESENT,
    QR_CODE_NOT_CONFIGURED,
    QR_ALL_OK
}enQrConfigErrorState;

typedef enum
{
    NO_QR_NO_BARCODE = 0,
    QR_SELECTED,
    BARCODE_SELECTED,
}enQROrBarcode;

typedef enum
{
    FIELD_QR,
    FIELD_BARCODE,
    FIELD_DATE,
    FIELD_TIME,
    FIELD_SER_NUM,
    FIELD_OPERATOR_NAME,
    FIELD_1,
    FIELD_2,
    FIELD_3,
    FIELD_4,
    FIELD_5,
    FIELD_6,
    FIELD_7,
    MAX_FIELDS
}enFields;

typedef struct
{
    quint8		isConfigured;
    quint8		qrSequence;
    quint8		TextSequence;
    quint8		fontSize;
    quint16 	xCoord;
    quint16 	yCoord;
    char 		key[ MAX_TEXT_FIELD_KEY_LENGTH + 1 ];
    char 		value[ MAX_TEXT_FIELD_VAL_LENGTH + 1 ];
}stFiledInfo;

typedef struct
{
    quint16 	stickerWidth;
    quint16		stickerHeight;
    quint16     qrXCoord;
    quint16     qrYCoord;
    quint8		QrFields;
    quint8		textFields;
    quint8      qrOrBarcode;
    quint16     qrOrBarcodeSize;
    quint8      dayWiseCntInsteadUID;
    stFiledInfo fieldInfo[ MAX_FIELDS ];
}stInfo;

typedef struct
{
    quint16 pumpOnTime;
    int setPressure;
    int pressureOffset;
}stTypeAConfig;

typedef struct
{
    quint16 pumpOnTime;
    quint16 stabilizeTime;
    quint16 testDelay;
    int     setPressure;
    quint16 stabilizeTimeDropValue;
    quint16 testDelayDropValue;
}stTypeBConfig;

typedef union
{
    stTypeAConfig typeA;
    stTypeBConfig typeB;
}unType;

typedef struct
{
    unType leakConfig;
    quint8 type;
    quint8 nodeLeakEnDis[ MAX_ALLOWED_NODES ];
    quint8 totalLeakEnabledNodes;
}stLeakTestConfig;

typedef struct
{
    quint8      isProductLearned;
    quint8      prodFetchedFromDB;
    quint32     prodUniqueId;
    quint16     totalNodes;
    quint16     totalInputOutputs;
    quint16     totalTestedProduct;
    quint16     totalDayWiseCount;
    char        productName[ PRODUCT_NAME_LENGTH + 1 ];
    char        createrName[ PRODUCT_CREATER_NAME_LENGTH + 1 ];
    char        createdOn[ DATE_FIELD_LENGTH + 1 ];
    char        learnedOn[ DATE_FIELD_LENGTH + 1 ];
    quint64     productRelearnedOn;
    quint64     productSerialNoStart;
    quint32     totalConnections;

    char        nodeName[ MAX_ALLOWED_NODES ][ NODE_NAME_LENGTH + 1 ];
    quint8      nodeInOuts[ MAX_ALLOWED_NODES ];
    quint32     digitalInputValues[ MAX_ALLOWED_NODES ][ MAX_OUTPUT_IN_NODE ][ MAX_NODE_INPUT_VALUES ];

    stLeakTestConfig leakTest;
    stInfo      qrInfo;
}st_ProductDBInfo;

typedef struct
{
    char            appUniqueID[ 64 ];
    char            clientID[ 64 ];
    en_syncStatus   syncStatus;
    quint32         syncCount;
    quint32         syncInterval; //In Hours
    bool            isRestorationInProgress;
}st_appSyncInfo;

typedef struct
{
    quint16 lockPeriod_1;
    quint16 lockPeriod_2;
    quint8 bypassLock1_2State;
    quint8 bypassLock3State;
}st_lockPeriod;

typedef struct
{
    en_screen           currScreen;
    en_processModes     currMode;
    en_processModeState currModeState;
    quint16             delayBetwnOp;
    quint16             delayInterval;
    bool                runAutoTest;
    char                currUserName[ APP_LOGIN_UNAME_LENGTH + 1 ];
    char                machineSerialNumber[ MACHINE_SER_NUM_LENGTH + 1 ];
    quint8              relay1Period;
    quint32             debugFileNumber;
    st_appSyncInfo      appSyncINfo;
    quint32             successfulTestCount;
    quint32             totalTestCount;
    quint64             lastSyncHappenedOn;
    bool                SystemAllowedStatus;
    quint8              appLastStartedOnDate;
    st_lockPeriod       lockConfig;
}st_systemState;

/*
typedef struct
{
    bool isRestorationInProgress;
    bool isAnySyncRequestSent;
    bool isResponseReceived;
    restoreState_en restoreState;
    quint8 isRequestTimeoutOccured;
    quint8 retryCounter;
    quint8 nodeCount;
    quint16 productCount;
    quint16 currentProductInfo;
    quint16 responseWaitCounter;
}restorationStatus_st;*/

class commonUtility : public QObject
{
    Q_OBJECT
public:
    static commonUtility& instance() {
        static commonUtility instance;
        return instance;
    }
    commonUtility();

    //Message Box Popup related Functions
    QMessageBox::StandardButton displayUserInputMessageBox( en_PopupMesgType _mesgType, QString _strMesg );
    void displayPopupMessage( en_PopupMesgType _mesgType, QString _strMesg );
    void nonBlockingPopupMessage( en_PopupMesgType _mesgType, QString _strMesg );

    void saveIniConfigurationValues( en_iniFileParam iniParam, QString _value );

    void setSystemAllowStatus( bool _status );
    bool isSystemAllowedToOperate( void );
    st_appSyncInfo *getSysInfoObject( void );
    en_syncStatus getSystemSyncStatus( void );
    bool isSystemUpdateToCLoudRequired( void );
    void updateSystemAllowedStatus( void );
    quint32 getSyncTestCount( void );
    quint32 getSyncInterval( void );
    QString getApplicationUniqueID( void );
    QString getClientID( void );
    void setSuccessfulTestCount( quint32 _testCount );
    void increamentSuccessfulTestCount( void );
    quint32 getSuccessfulTestCount( void );
    void setTotalTestCount( quint32 _testCount );
    void increamentTotalTestCount( void );
    quint32 getTotalTestCount( void );

private:
    commonUtility(const commonUtility &) = delete;
    commonUtility& operator=(const commonUtility &) = delete;

    QTimer *popUpDispTimer;
    QString popupMessages;
    en_PopupMesgType mesgType;

private slots:
    void popUpDispTimerElapsed( );
};

namespace debugUtility
{
    // Function for logging messages
    inline void logMessage( const QString& message, bool debugEnabled )
    {
        if( debugEnabled )
        {
            qDebug() << message;
        }
    }

    inline QMessageBox::StandardButton displayUserInputMessageBox( en_PopupMesgType _mesgType, QString _strMesg ) {
        return commonUtility::instance().displayUserInputMessageBox( _mesgType, _strMesg );
    }
    inline void displayPopupMessage( en_PopupMesgType _mesgType, QString _strMesg ) {
        return commonUtility::instance().displayPopupMessage( _mesgType, _strMesg );
    }
    inline void nonBlockingPopupMessage( en_PopupMesgType _mesgType, QString _strMesg ) {
        return commonUtility::instance().nonBlockingPopupMessage( _mesgType, _strMesg );
    }
    inline void saveIniConfigurationValues( en_iniFileParam iniParam, QString _value ) {
        return commonUtility::instance().saveIniConfigurationValues( iniParam, _value );
    }

    inline void setSystemAllowStatus( bool _status ) {
        return commonUtility::instance().setSystemAllowStatus( _status );
    }
    inline bool isSystemAllowedToOperate( ) {
        return commonUtility::instance().isSystemAllowedToOperate( );
    }

    inline st_appSyncInfo *getSysInfoObject( ) {
        return commonUtility::instance().getSysInfoObject( );
    }
    inline en_syncStatus getSystemSyncStatus( ) {
        return commonUtility::instance().getSystemSyncStatus( );
    }
    inline bool isSystemUpdateToCLoudRequired( ) {
        return commonUtility::instance().isSystemUpdateToCLoudRequired( );
    }
    inline void updateSystemAllowedStatus( ) {
        return commonUtility::instance().updateSystemAllowedStatus( );
    }
    inline quint32 getSyncTestCount( ) {
        return commonUtility::instance().getSyncTestCount( );
    }
    inline quint32 getSyncInterval( ) {
        return commonUtility::instance().getSyncInterval( );
    }
    inline QString getApplicationUniqueID( ) {
        return commonUtility::instance().getApplicationUniqueID( );
    }
    inline QString getClientID( ) {
        return commonUtility::instance().getClientID( );
    }
    inline void setSuccessfulTestCount( quint32 _testCount ) {
        return commonUtility::instance().setSuccessfulTestCount( _testCount );
    }
    inline void increamentSuccessfulTestCount( ) {
        return commonUtility::instance().increamentSuccessfulTestCount( );
    }
    inline quint32 getSuccessfulTestCount( ) {
        return commonUtility::instance().getSuccessfulTestCount( );
    }
    inline void setTotalTestCount( quint32 _testCount ) {
        return commonUtility::instance().setTotalTestCount( _testCount );
    }
    inline void increamentTotalTestCount( ) {
        return commonUtility::instance().increamentTotalTestCount( );
    }
    inline quint32 getTotalTestCount( ) {
        return commonUtility::instance().getTotalTestCount( );
    }
}

using debugUtility::logMessage;
using debugUtility::displayUserInputMessageBox;
using debugUtility::displayPopupMessage;
using debugUtility::nonBlockingPopupMessage;
using debugUtility::saveIniConfigurationValues;

using debugUtility::setSystemAllowStatus;
using debugUtility::isSystemAllowedToOperate;

using debugUtility::getSysInfoObject;
using debugUtility::getSystemSyncStatus;
using debugUtility::isSystemUpdateToCLoudRequired;
using debugUtility::updateSystemAllowedStatus;
using debugUtility::getSyncTestCount;
using debugUtility::getSyncInterval;
using debugUtility::getApplicationUniqueID;
using debugUtility::getClientID;
using debugUtility::setSuccessfulTestCount;
using debugUtility::increamentSuccessfulTestCount;
using debugUtility::getSuccessfulTestCount;
using debugUtility::setTotalTestCount;
using debugUtility::increamentTotalTestCount;
using debugUtility::getTotalTestCount;

#endif // COMMONUTILITY_H
