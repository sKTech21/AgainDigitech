#ifndef DBROUTINES_H
#define DBROUTINES_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QComboBox>

#include "commonutility.h"

typedef enum
{
    TABLE_NOT_PRESENT = 0,
    QUERY_EXECUTION_ERROR,
    QUERY_EXECUTED_SUCCESSFULLY,
}en_dbErrors;

enum
{
    NODE_INFORMATION_TABLE,
    PRODUCT_INFORMATION_TABLE,
    SYSTEM_CONFIGURATION_TABLE,
    TESTED_PRODUCT_INFORMATION_TABLE,
    USER_INFORMATION_TABLE,
    MAX_DATABASE_TABLE
}en_dbTable;

enum
{
    WIFI_SSID_SYS_CONFIG = 0,
    WIFI_PASS_SYS_CONFIG,
    WIFI_IP1_SYS_CONFIG,
    WIFI_IP2_SYS_CONFIG,
    WIFI_IP3_SYS_CONFIG,
    WIFI_IP4_SYS_CONFIG,
    DELAY_BETWEEN_OUPUT_SYS_CONFIG,
    BROADCASTING_DELAY_SYS_CONFIG,
    LEAK_OFFSET_SYS_CONFIG,
    SERVER_IP_SYS_CONFIG,
    LOCK_1_PERIOD_SYS_CONFIG,
    LOCK_2_PERIOD_SYS_CONFIG,
    BYPASS_LOCK_1And2_SYS_CONFIG,
    BYPASS_LOCK_3_SYS_CONFIG,
    MACHINE_SERIAL_NUMBER_SYS_CONFIG,
    LEAK_CALIBRATION_SYS_CONFIG,
    RELAY_1_PERIOD_SYS_CONFGI,
    DEBUG_FILE_NUMBER_SYS_CONFGI,
    APPLICAIOTN_CLIENT_ID_CONFIG,
    APPLICAITON_UNIQUE_ID_CONFIG,
    SYNC_REQUIRED_STATUS_CONFIG,
    SYNC_ON_INTERVAL_CONFIG,
    SYNC_ON_TEST_COUNT_CONFIG,
    NO_OF_SUCCESSFULL_TEST_COUNT_CONFIG,
    NO_OF_TOTAL_TEST_COUNT_CONFIG,
    APP_LAST_OPERATED_ON_DATE_CONFIG,
    MAX_SYSTEM_CONFIGURATION,
};

typedef struct
{
    quint8  keyNumber;
    QString key;
    QString value;
}st_sysConfigTableInfo;

typedef struct
{
    QString user;
    QString pass;
}st_defaultLoginCredentials;

typedef struct
{
    char        creator[ PRODUCT_CREATER_NAME_LENGTH + 1 ];
    quint64     createdOn;
    quint8      totalNodes;
    quint16     totalIO;
    quint32     totalTestedProducts;
    bool        isQrConfigured;
    quint64     prodSerialNoStart;
    quint32     dayWiseProductCount;
}st_prodBasicInfo;

class dbRoutines: public QObject
{
    Q_OBJECT
public:
    static dbRoutines& dbInstance() {
        static dbRoutines dbInstance; //Singleton instance
        return dbInstance;
    }

    dbRoutines( );
    static st_sysConfigTableInfo sysConfigInformation[ MAX_SYSTEM_CONFIGURATION ];

    bool DB_loadAllEnabledProducts(QComboBox *_cBox );
    void DB_getProductBasicInfo( QString _prodName, st_prodBasicInfo *_basicInfo );
    bool DB_getProductInfo( st_ProductDBInfo *_dbInfo );
    bool DB_GetproductInfoForCloudUpdate( st_ProductDBInfo *_dbInfo , quint8 *_prodCount, quint8 *_cloudStatus , QString *ioConfig, QString *lConfig, QString *qrCOnfig, QString *learnData );
    en_dbErrors getProductCountWhichNeedsCloudUpdate( quint16 *_productCount );
    bool DB_executeSqlQuery( QString *_strQuery );
    QString DB_getIntSystemConfigParam( QString _param );
    en_dbErrors DB_saveSystemConfigurationParam(QString _param, QString _value );
    void DB_checkIfDebugNumberPresent();
    en_dbErrors initDatabaseTable();
    en_dbErrors DB_setProductInfoByColumn( QString _columnName, QString _prodname, QString _value );
    en_dbErrors DB_getSpecificColumnValueFromProductInfo(QString _prodName, QString _columnStr, QString *_value);
    en_dbErrors resetDayWiseCountForAllProduct();
    bool DB_GetProductCount( QString _productCount, quint32 *_count );

private:
    QSqlDatabase dbConn;
    static st_defaultLoginCredentials defaultLoginCredentials[ 5 ];

    void addParaminSysConfigTable();
    en_dbErrors validateSystemConfigurationTable();
    en_dbErrors validateProductInfoTable();
    en_dbErrors validateUserInfoTable();
    en_dbErrors validateNodeInformationTable();
    en_dbErrors validateTestedProductInfoTable();

    en_dbErrors checkIfTablePresent(QString _tableName , quint8 *tableCount);
    en_dbErrors createSystemConfigurationTable( );
    en_dbErrors createProductInfoTable( );
    en_dbErrors addSystemConfigurationpParameters( );
    en_dbErrors createUserInfoTable( );
    en_dbErrors addDefaultUsersForLogin( );
    en_dbErrors createNodeInfoTable();
    en_dbErrors createTestedProductInfoTable();
    en_dbErrors checkIfDayWiseColumnPresent();
};

namespace databaseUtility
{
    // Define static inline functions to forward calls to the singleton instance
    inline bool DB_loadAllEnabledProducts( QComboBox *_cBox ) {
        return dbRoutines::dbInstance().DB_loadAllEnabledProducts( _cBox );
    }

    inline void DB_getProductBasicInfo( QString _prodName, st_prodBasicInfo *_basicInfo ) {
        dbRoutines::dbInstance().DB_getProductBasicInfo(_prodName, _basicInfo);
    }

    inline bool DB_getProductInfo( st_ProductDBInfo *_dbInfo ) {
        return dbRoutines::dbInstance().DB_getProductInfo( _dbInfo );
    }

    inline bool DB_GetproductInfoForCloudUpdate( st_ProductDBInfo *_dbInfo, quint8 *_prodCount, quint8 *_cloudStatus , QString *ioConfig, QString *lConfig, QString *qrCOnfig, QString *learnData ) {
        return dbRoutines::dbInstance().DB_GetproductInfoForCloudUpdate( _dbInfo, _prodCount, _cloudStatus , ioConfig, lConfig, qrCOnfig, learnData );
    }

    inline en_dbErrors getProductCountWhichNeedsCloudUpdate( quint16 *_productCount ) {
        return dbRoutines::dbInstance().getProductCountWhichNeedsCloudUpdate( _productCount );
    }

    inline bool DB_executeSqlQuery( QString *_strQuery ) {
        return dbRoutines::dbInstance().DB_executeSqlQuery( _strQuery );
    }

    inline QString DB_getIntSystemConfigParam( QString _param ) {
        return dbRoutines::dbInstance().DB_getIntSystemConfigParam( _param );
    }

    inline en_dbErrors DB_saveSystemConfigurationParam(QString _param, QString _value ) {
        return dbRoutines::dbInstance().DB_saveSystemConfigurationParam( _param, _value );
    }

    inline void DB_checkIfDebugNumberPresent() {
        return dbRoutines::dbInstance().DB_checkIfDebugNumberPresent( );
    }

    inline en_dbErrors initDatabaseTable() {
        return dbRoutines::dbInstance().initDatabaseTable( );
    }

    inline en_dbErrors DB_setProductInfoByColumn( QString _columnName, QString _prodname, QString _value ) {
        return dbRoutines::dbInstance().DB_setProductInfoByColumn( _columnName, _prodname, _value );
    }

    inline en_dbErrors DB_getSpecificColumnValueFromProductInfo(QString _prodName, QString _columnStr, QString *_value) {
        return dbRoutines::dbInstance().DB_getSpecificColumnValueFromProductInfo( _prodName, _columnStr, _value);
    }

    inline en_dbErrors resetDayWiseCountForAllProduct() {
        return dbRoutines::dbInstance().resetDayWiseCountForAllProduct( );
    }

    inline bool DB_GetProductCount( QString _productCount, quint32 *_count ) {
        return dbRoutines::dbInstance().DB_GetProductCount( _productCount, _count );
    }
}

using databaseUtility::DB_loadAllEnabledProducts;
using databaseUtility::DB_getProductBasicInfo;
using databaseUtility::DB_getProductInfo;
using databaseUtility::DB_GetproductInfoForCloudUpdate;
using databaseUtility::getProductCountWhichNeedsCloudUpdate;
using databaseUtility::DB_executeSqlQuery;
using databaseUtility::DB_getIntSystemConfigParam;
using databaseUtility::DB_saveSystemConfigurationParam;
using databaseUtility::DB_checkIfDebugNumberPresent;
using databaseUtility::initDatabaseTable;
using databaseUtility::DB_setProductInfoByColumn;
using databaseUtility::DB_getSpecificColumnValueFromProductInfo;
using databaseUtility::resetDayWiseCountForAllProduct;
using databaseUtility::DB_GetProductCount;

#endif // DBROUTINES_H
