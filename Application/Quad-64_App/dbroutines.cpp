#include "dbroutines.h"
#include "commonutility.h"

st_sysConfigTableInfo dbRoutines::sysConfigInformation[ MAX_SYSTEM_CONFIGURATION ] =
    {
#if defined( APP_DEVELOPMENT_IN_PROGRESS )
        { WIFI_SSID_SYS_CONFIG,               DB_WIFI_SSID,                   "Palak" },
#else
        { WIFI_SSID_SYS_CONFIG,               DB_WIFI_SSID,                   "Again-AX73" },
#endif
        { WIFI_PASS_SYS_CONFIG,               DB_WIFI_PASS,                   "11223344" },
        { WIFI_IP1_SYS_CONFIG,                DB_IP1_PARAM,                   "192" },
        { WIFI_IP2_SYS_CONFIG,                DB_IP2_PARAM,                   "168" },
        { WIFI_IP3_SYS_CONFIG,                DB_IP3_PARAM,                   "1" },
        { WIFI_IP4_SYS_CONFIG,                DB_IP4_PARAM,                   "2" },
        { DELAY_BETWEEN_OUPUT_SYS_CONFIG,     DB_OUTPUT_DELAY_PARAM,          "40"},
        { BROADCASTING_DELAY_SYS_CONFIG,      DB_DELAY_INTERVAL_PARAM,        "10" },
        { LEAK_OFFSET_SYS_CONFIG,             DB_LEAK_TEST_VAL_OFFSET_PARAM,  "25" },
        { SERVER_IP_SYS_CONFIG,               DB_MQTT_BROAKER_IP_ADD_PARAM,   "127.0.0.1" },
        { LOCK_1_PERIOD_SYS_CONFIG,           DB_LOCK_PERIOD_1_PARAM,         "500" }, //In Ms
        { LOCK_2_PERIOD_SYS_CONFIG,           DB_LOCK_PERIOD_2_PARAM,         "700" }, //In Ms
        { BYPASS_LOCK_1And2_SYS_CONFIG,       DB_LOCK_1_2_BYPASS_PARAM,       "0" },
        { BYPASS_LOCK_3_SYS_CONFIG,           DB_LOCK_3_BYPASS_PARAM,         "0" },
        { MACHINE_SERIAL_NUMBER_SYS_CONFIG,   DB_MACHINE_SERIAL_NUM_PARAM,    "" },
        { LEAK_CALIBRATION_SYS_CONFIG,        DB_LEAK_CALIBRATION_PARAM,           "{" \
         "\"negMaxPress\": -1," \
         "\"negMinPress\": -580, " \
         "\"negPressMaxAdc\": 1400, " \
         "\"negPressMinAdc\": 655, " \
         "\"posMaxPress\": 950, " \
         "\"posMinPress\": 1, " \
         "\"posPressMaxAdc\": 2000, " \
         "\"posPressMinAdc\": 10 " \
         "}" },
        { RELAY_1_PERIOD_SYS_CONFGI,          DB_RELAY_1_PERIOD_PARAM,        "3" }, // In Seconds
        { DEBUG_FILE_NUMBER_SYS_CONFGI,       DB_DEBUG_FILE_NUM_PARAM,        "1" },
        { APPLICAIOTN_CLIENT_ID_CONFIG,       HTTP_CLIENT_ID_STR,             "" },
        { APPLICAITON_UNIQUE_ID_CONFIG,       HTTP_APPLICAITON_UNIQUE_ID_STR, "" },
        { SYNC_REQUIRED_STATUS_CONFIG,        HTTP_SYNC_REQUIRED_STATUS_STR,  "" },
        { SYNC_ON_TEST_COUNT_CONFIG,          HTTP_SYNC_TEST_COUNT_STR,       "" },
        { SYNC_ON_INTERVAL_CONFIG,            HTTP_SYNC_ON_INTERVAL_STR,      "" },
        { NO_OF_SUCCESSFULL_TEST_COUNT_CONFIG,DB_SUCCESSFUL_TEST_COUNT_PARAM, "0" },
        { NO_OF_TOTAL_TEST_COUNT_CONFIG,      DB_TOTAL_TEST_COUNT_PARAM,      "0" },
        { APP_LAST_OPERATED_ON_DATE_CONFIG,   DB_APP_OPERATED_ON_DATE_PARAM,  "0" } };

st_defaultLoginCredentials dbRoutines::defaultLoginCredentials[ 5 ] =
    {
        { "Again", "13579" },
        { "Palak", "1" },
        { "root", "root" },
        { "admin", "admin" },
        { "sandip", "1" }
};

dbRoutines::dbRoutines(  ) //QSqlDatabase db
{
    //dbConn = db;
}

en_dbErrors dbRoutines::initDatabaseTable( )
{
    if( validateSystemConfigurationTable() != QUERY_EXECUTED_SUCCESSFULLY ||
        validateProductInfoTable() != QUERY_EXECUTED_SUCCESSFULLY ||
        validateUserInfoTable() != QUERY_EXECUTED_SUCCESSFULLY ||
        validateNodeInformationTable() != QUERY_EXECUTED_SUCCESSFULLY ||
        validateTestedProductInfoTable() != QUERY_EXECUTED_SUCCESSFULLY )
    {
        return QUERY_EXECUTION_ERROR;
    }
    return QUERY_EXECUTED_SUCCESSFULLY;
}

en_dbErrors dbRoutines::checkIfTablePresent( QString _tableName, quint8 *tableCount )
{
    QString strQuery = QString( "SELECT count(*) FROM sqlite_master WHERE type='table' AND name='%1';" )
                           .arg( _tableName );
    QSqlQuery sqlQuery( strQuery, dbConn );
    if( !sqlQuery.exec() )
    {
        qDebug() << sqlQuery.lastError().text();
        return QUERY_EXECUTION_ERROR;
    }

    //qDebug() << "Qry : " << strQuery;
    while( sqlQuery.next() )
    {
        //qDebug() << sqlQuery.value( 0 ).toInt();
        *tableCount = sqlQuery.value( 0 ).toInt();
    }
    return QUERY_EXECUTED_SUCCESSFULLY;
}

en_dbErrors dbRoutines::validateSystemConfigurationTable( )
{
    quint8 tableCount = 0;

    if( checkIfTablePresent( DB_SYS_CONFIG_TABLE, &tableCount ) == QUERY_EXECUTED_SUCCESSFULLY )
    {
        if( tableCount == 0 )
        {
            nonBlockingPopupMessage( ERROR_MESG,
                                     "System configuration database table not present!\n"
                                     "Creating Table with default values" );
            if( createSystemConfigurationTable() != QUERY_EXECUTED_SUCCESSFULLY ||
                addSystemConfigurationpParameters( ) != QUERY_EXECUTED_SUCCESSFULLY )
            {
                return QUERY_EXECUTION_ERROR;
            }
        }
        else
        {
            if( addSystemConfigurationpParameters() != QUERY_EXECUTED_SUCCESSFULLY )
                return QUERY_EXECUTION_ERROR;
        }
    }
    return QUERY_EXECUTED_SUCCESSFULLY;
}

en_dbErrors dbRoutines::createSystemConfigurationTable( )
{
    QString strQuery = QString( "CREATE TABLE IF NOT EXISTS %1 ( %2 CHAR, %3 CHAR );" )
                           .arg( DB_SYS_CONFIG_TABLE ).arg( DB_SYS_CONFIG_PARAM ).arg( DB_SYS_CONFIG_VALUE );
    QSqlQuery sqlQuery( strQuery, dbConn );
    //qDebug() << "Qry : " << strQuery;

    if( !sqlQuery.exec() )
    {
        qDebug() << sqlQuery.lastError().text();
        return QUERY_EXECUTION_ERROR;
    }
    else
        return QUERY_EXECUTED_SUCCESSFULLY;
}

en_dbErrors dbRoutines::addSystemConfigurationpParameters( )
{
    quint16 rowCount = 0;
    for( rowCount = 0; rowCount < MAX_SYSTEM_CONFIGURATION; rowCount++ )
    {
        QString strQuery = QString( "INSERT INTO %1 (%2, %3) "
                                   "SELECT * FROM (SELECT '%4', '%5') AS tmp "
                                   "WHERE NOT EXISTS ( SELECT * FROM %6 WHERE %7 = '%8' ) LIMIT 1;")
                               .arg( DB_SYS_CONFIG_TABLE ).arg( DB_SYS_CONFIG_PARAM ).arg( DB_SYS_CONFIG_VALUE )
                               .arg( sysConfigInformation[ rowCount ].key ).arg( sysConfigInformation[ rowCount ].value )
                               .arg( DB_SYS_CONFIG_TABLE ).arg( DB_SYS_CONFIG_PARAM )
                               .arg( sysConfigInformation[ rowCount ].key );

        QSqlQuery sqlQuery( strQuery, dbConn );
        //qDebug() << "Qry : " << strQuery;

        if( !sqlQuery.exec() )
        {
            qDebug() << sqlQuery.lastError().text();
            return QUERY_EXECUTION_ERROR;
        }
    }
    return QUERY_EXECUTED_SUCCESSFULLY;
}

QString dbRoutines::DB_getIntSystemConfigParam( QString _param )
{
    QString retValue = 0;
    QString strQuery = QString( "SELECT * FROM %1 WHERE %2 = '%3';" )
                           .arg( DB_SYS_CONFIG_TABLE ).arg( DB_SYS_CONFIG_PARAM ).arg( _param );

    QSqlQuery sqlQuery( strQuery, dbConn );
    sqlQuery.exec();

    while( sqlQuery.next() )
    {
        retValue = sqlQuery.value( DB_SYS_CONFIG_VALUE ).toString();
    }
    return retValue;
}

en_dbErrors dbRoutines::DB_saveSystemConfigurationParam( QString _param, QString _value )
{
    QString retValue = 0;
    QString strQuery = QString( "UPDATE %1 SET %2 = '%3' WHERE %4 = '%5'" ).arg( DB_SYS_CONFIG_TABLE )
                           .arg( DB_SYS_CONFIG_VALUE ).arg( _value ).arg( DB_SYS_CONFIG_PARAM ).arg( _param );

    if( dbRoutines::DB_executeSqlQuery( &strQuery ) )
    {
        return QUERY_EXECUTED_SUCCESSFULLY;
    }
    else
    {
        return QUERY_EXECUTION_ERROR;
    }
}

en_dbErrors dbRoutines::validateProductInfoTable()
{
    quint8 tableCount = 0;

    if( checkIfTablePresent( DB_PRODUCT_INFO_TABLE, &tableCount ) == QUERY_EXECUTED_SUCCESSFULLY )
    {
        if( tableCount == 0 )
        {
            nonBlockingPopupMessage( ERROR_MESG,
                                     "Product Info database table not present!\n"
                                     "Creating Table" );
            if( createProductInfoTable() != QUERY_EXECUTED_SUCCESSFULLY )
            {
                return QUERY_EXECUTION_ERROR;
            }
        }
        else
        {
            //Check For Colums in Existing Table
            checkIfDayWiseColumnPresent();
        }
    }
    return QUERY_EXECUTED_SUCCESSFULLY;
}

en_dbErrors dbRoutines::checkIfDayWiseColumnPresent()
{
    QString strQuery = QString( "ALTER TABLE %1 ADD COLUMN %2 integer default %3;" )
                           .arg( DB_PRODUCT_INFO_TABLE ).arg( DB_DAY_WISE_PRODUCT_COUNT ).arg( 0 );

    QSqlQuery sqlQuery( strQuery, dbConn );
    //qDebug() << "Qry : " << strQuery;

    if( !sqlQuery.exec() )
    {
        qDebug() << sqlQuery.lastError().text();
        return QUERY_EXECUTION_ERROR;
    }
    else
        return QUERY_EXECUTED_SUCCESSFULLY;
}

en_dbErrors dbRoutines::createProductInfoTable( )
{
    QString strQuery = QString( "CREATE TABLE IF NOT EXISTS %1 "
                               "( %2 INTEGER, %3 INTEGER, %4 TEXT, %5 TEXT, %6 INTEGER, %7 INTEGER, %8 INTEGER,"
                               "%9 INTEGER, %10 TEXT, %11 TEXT, %12 TEXT, %13 TEXT, %14 TEXT,"
                               "%15 INTEGER, %16 INTEGER, %17 INTEGER, PRIMARY KEY( %18 ) );" )
                           .arg( DB_PRODUCT_INFO_TABLE )
                           .arg( DB_PROD_UNIQUE_ID ).arg( DB_PROD_CREATED_ON ).arg( DB_PROD_CREATOR )
                           .arg( DB_PROD_NAME ).arg( DB_PROD_TOTAL_NODES )
                           .arg( DB_SUCCESSFUL_TEST_COUNT_PARAM ).arg( DB_TOTAL_TEST_COUNT_PARAM )
                           .arg( DB_PROD_TOTAL_IO ).arg( DB_PROD_IO_CONFIG ).arg( DB_PROD_LEAK_CONFIG )
                           .arg( DB_PROD_QR_CONFIG ).arg( DB_PROD_LEARN_DATA ).arg( DB_PROD_LEARNED_ON )
                           .arg( DB_PROD_UID_START_FROM_STR ).arg( DB_PROD_CLOUD_UPDATE_REQ_STR )
                           .arg( DB_DAY_WISE_PRODUCT_COUNT ).arg( DB_PROD_UNIQUE_ID );
    QSqlQuery sqlQuery( strQuery, dbConn );
    //qDebug() << "Qry : " << strQuery;

    if( !sqlQuery.exec() )
    {
        qDebug() << sqlQuery.lastError().text();
        return QUERY_EXECUTION_ERROR;
    }
    else
        return QUERY_EXECUTED_SUCCESSFULLY;
}

en_dbErrors dbRoutines::validateUserInfoTable()
{
    quint8 tableCount = 0;

    if( checkIfTablePresent( DB_USER_IFO_TABLE, &tableCount ) == QUERY_EXECUTED_SUCCESSFULLY )
    {
        if( tableCount == 0 )
        {
            nonBlockingPopupMessage( ERROR_MESG,
                                     "User Info database table not present!\n"
                                     "Creating Table with default Login credentials" );
            if( createUserInfoTable() != QUERY_EXECUTED_SUCCESSFULLY ||
                addDefaultUsersForLogin() != QUERY_EXECUTED_SUCCESSFULLY )
            {
                return QUERY_EXECUTION_ERROR;
            }
        }
    }
    return QUERY_EXECUTED_SUCCESSFULLY;
}

en_dbErrors dbRoutines::addDefaultUsersForLogin( )
{
    for( quint8 ite = 0; ite < 5; ite++ )
    {
        QString strQuery = QString( "INSERT INTO %1 (%2, %3) "
                                   "SELECT * FROM (SELECT '%4', '%5') AS tmp "
                                   "WHERE NOT EXISTS ( SELECT * FROM %6 WHERE %7 = '%8' ) LIMIT 1;")
                               .arg( DB_USER_IFO_TABLE ).arg( DB_USER_NAME_STR ).arg( DB_USER_PASSWROD )
                               .arg( defaultLoginCredentials[ ite ].user )
                               .arg( defaultLoginCredentials[ ite ].pass )
                               .arg( DB_USER_IFO_TABLE ).arg( DB_USER_NAME_STR )
                               .arg( defaultLoginCredentials[ ite ].user );

        QSqlQuery sqlQuery( strQuery, dbConn );
        //qDebug() << "Qry : " << strQuery;

        if( !sqlQuery.exec() )
        {
            qDebug() << sqlQuery.lastError().text();
            return QUERY_EXECUTION_ERROR;
        }
    }
    return QUERY_EXECUTED_SUCCESSFULLY;
}

en_dbErrors dbRoutines::createUserInfoTable( )
{
    QString strQuery = QString( "CREATE TABLE IF NOT EXISTS %1 ( %2	TEXT, %3 TEXT ) " )
                           .arg( DB_USER_IFO_TABLE ).arg( DB_USER_NAME_STR ).arg( DB_USER_PASSWROD );

    QSqlQuery sqlQuery( strQuery, dbConn );
    qDebug() << "Qry : " << strQuery;

    if( !sqlQuery.exec() )
    {
        qDebug() << sqlQuery.lastError().text();
        return QUERY_EXECUTION_ERROR;
    }
    else
        return QUERY_EXECUTED_SUCCESSFULLY;
}

en_dbErrors dbRoutines::validateNodeInformationTable()
{
    quint8 tableCount = 0;

    if( checkIfTablePresent( DB_NODE_INFO_TABLE, &tableCount ) == QUERY_EXECUTED_SUCCESSFULLY )
    {
        if( tableCount == 0 )
        {
            nonBlockingPopupMessage( ERROR_MESG,
                                     "Node Info database table not present!\n"
                                     "Creating Table" );
            if( createNodeInfoTable() != QUERY_EXECUTED_SUCCESSFULLY )
            {
                return QUERY_EXECUTION_ERROR;
            }
        }
    }
    return QUERY_EXECUTED_SUCCESSFULLY;
}

en_dbErrors dbRoutines::createNodeInfoTable()
{
    QString strQuery = QString( "CREATE TABLE IF NOT EXISTS %1 ( %2 INTEGER, %3 INTEGER, %4 INTEGER, %5 TEXT, "
                               "%6 TEXT, %7 TEXT, %8 INTEGER, %9 TEXT, PRIMARY KEY(%10) );" )
                           .arg( DB_NODE_INFO_TABLE )
                           .arg( DB_NODE_UNIQUE_NUMBER ).arg( DB_STR_NODE_NUMEBR ).arg( DB_STR_NODE_TYPE )
                           .arg( DB_STR_NODE_MAC ).arg( DB_STR_NODE_LOCATION ).arg( DB_STR_PARENT_MAC )
                           .arg( DB_STR_PARENT_NUMBER ).arg( DB_NODE_NAME ).arg( DB_NODE_UNIQUE_NUMBER );

    QSqlQuery sqlQuery( strQuery, dbConn );
    //qDebug() << "Qry : " << strQuery;

    if( !sqlQuery.exec() )
    {
        qDebug() << sqlQuery.lastError().text();
        return QUERY_EXECUTION_ERROR;
    }
    else
        return QUERY_EXECUTED_SUCCESSFULLY;
}

en_dbErrors dbRoutines::resetDayWiseCountForAllProduct()
{
    QString strQuery;
    strQuery = QString( "UPDATE %1 SET %2 = %3" )
                   .arg( DB_PRODUCT_INFO_TABLE ).arg( DB_DAY_WISE_PRODUCT_COUNT )
                   .arg( 0 );

    //qDebug() << "Query : " << strQuery;
    QSqlQuery sqlQuery( strQuery, dbConn );

    if( !sqlQuery.exec() )
    {
        qDebug() << sqlQuery.lastError().text();
        return QUERY_EXECUTION_ERROR;
    }
    else
    {
        return QUERY_EXECUTED_SUCCESSFULLY;
    }
}

en_dbErrors dbRoutines::validateTestedProductInfoTable()
{
    quint8 tableCount = 0;

    if( checkIfTablePresent( DB_TESTED_PROD_INFO, &tableCount ) == QUERY_EXECUTED_SUCCESSFULLY )
    {
        if( tableCount == 0 )
        {
            nonBlockingPopupMessage( ERROR_MESG,
                                     "Product Test History database table not present!\n"
                                     "Creating Table" );
            if( createTestedProductInfoTable() != QUERY_EXECUTED_SUCCESSFULLY )
            {
                return QUERY_EXECUTION_ERROR;
            }
        }
    }
    return QUERY_EXECUTED_SUCCESSFULLY;
}

en_dbErrors dbRoutines::createTestedProductInfoTable()
{
    QString strQuery = QString( "CREATE TABLE IF NOT EXISTS %1 ( %2 INTEGER, %3 INTEGER, %4 TEXT, %5 TEXT, %6 INTEGER, "
                               "%7 TEXT, %8 TEXT, %9 TEXT, PRIMARY KEY( %10 ) );" )
                           .arg( DB_TESTED_PROD_INFO )
                           .arg( DB_TESTED_PROD_UNIQ_ID ).arg( DB_PROD_UNIQUE_ID ).arg( DB_PROD_NAME )
                           .arg( DB_PROD_TESTED_ON ).arg( DB_PROD_TEST_RESULT ).arg( DB_TESTED_PROD_DATA )
                           .arg( DB_PROD_TESTED_BY ).arg( DB_LEAK_RESULT_DATA ).arg( DB_TESTED_PROD_UNIQ_ID );

    QSqlQuery sqlQuery( strQuery, dbConn );
    //qDebug() << "Qry : " << strQuery;

    if( !sqlQuery.exec() )
    {
        qDebug() << sqlQuery.lastError().text();
        return QUERY_EXECUTION_ERROR;
    }
    else
        return QUERY_EXECUTED_SUCCESSFULLY;
}

void dbRoutines::DB_checkIfDebugNumberPresent( )
{
    QString strQuery = QString( "SELECT COUNT( * ) FROM sysConfig where param='debugFileNum'" );
    QSqlQuery sqlQuery( strQuery, dbConn );
    sqlQuery.exec();

    while( sqlQuery.next() )
    {
        qDebug() << "Debug File Number : " << sqlQuery.value(0).toUInt( );
        if( sqlQuery.value(0).toUInt( ) <= 0 )
        {
            qDebug() << "Entry Not Found";
            addParaminSysConfigTable( );
        }
    }
}

void dbRoutines::addParaminSysConfigTable( )
{
    QString strQuery = QString( "INSERT INTO %1 ( %2, %3 )"
                               "VALUES ( '%4', %5 )" )
                           .arg( DB_SYS_CONFIG_TABLE ).arg( DB_SYS_CONFIG_PARAM ).arg( DB_SYS_CONFIG_VALUE )
                           .arg( DB_DEBUG_FILE_NUM_PARAM ).arg( 1 );

    //qDebug() << "Qry : " << strQuery;
    if( !dbRoutines::DB_executeSqlQuery( &strQuery ) )
        nonBlockingPopupMessage( ERROR_MESG,
                                 "Database error in saving test process result" );
}

/*************************************************************************************************/
/**
  * @desc   :  This function will be called to load all available and enabled product in database.
  * @param  :  _cBox : To fill all available product in shared combobox
  * @return :  TRUE if Database operation success else failed.
**/
bool dbRoutines::DB_loadAllEnabledProducts( QComboBox *_cBox )
{
    QString strQuery = QString( "SELECT %1 FROM %2 ORDER BY %3 DESC LIMIT %4;" )
                           .arg( DB_PROD_NAME ).arg( DB_PRODUCT_INFO_TABLE )
                           .arg( DB_PROD_UNIQUE_ID ).arg( MAX_PRODUCT_LOAD_COUNT );

    QSqlQuery sqlQuery( strQuery, dbConn );
    sqlQuery.exec();

    _cBox->clear();
    _cBox->addItem( "-----" );

    while( sqlQuery.next() )
    {
        _cBox->addItem( sqlQuery.value( DB_PROD_NAME ).toString() );
    }
}

/*************************************************************************************************/
/**
  * @desc   :  This function will be called to get the basic information of the selected product
  * @param  :  _prodName : name of the product
  * @return : st_DBproductBasicInfo : product basic info
**/
void dbRoutines::DB_getProductBasicInfo( QString _prodName, st_prodBasicInfo *_basicInfo )
{
    QString strQuery = QString( "SELECT * FROM %1 WHERE %2 = '%3';" )
                           .arg( DB_PRODUCT_INFO_TABLE ).arg( DB_PROD_NAME ).arg( _prodName );

    QSqlQuery sqlQuery( strQuery, dbConn );
    sqlQuery.exec();

    while( sqlQuery.next() )
    {
        memcpy( &_basicInfo->creator,
               sqlQuery.value( DB_PROD_CREATOR ).toString().toStdString().c_str(),
               PRODUCT_CREATER_NAME_LENGTH );

        _basicInfo->createdOn = sqlQuery.value( DB_PROD_CREATED_ON ).toULongLong();
        _basicInfo->totalNodes = sqlQuery.value( DB_PROD_TOTAL_NODES ).toUInt();
        _basicInfo->totalIO = sqlQuery.value( DB_PROD_TOTAL_IO ).toUInt();
        _basicInfo->totalTestedProducts = sqlQuery.value( DB_SUCCESSFUL_TEST_COUNT_PARAM ).toUInt();
        _basicInfo->prodSerialNoStart =  sqlQuery.value( DB_PROD_UID_START_FROM_STR ).toULongLong();
        _basicInfo->dayWiseProductCount = sqlQuery.value( DB_DAY_WISE_PRODUCT_COUNT ).toULongLong();

        QString qrString = sqlQuery.value( DB_PROD_QR_CONFIG ).toString();
        QJsonDocument jsonDoc = QJsonDocument::fromJson( qrString.toUtf8() );
        QJsonObject tempJsnObj = jsonDoc.object();

        //_basicInfo->isQrConfigured = ( tempJsnObj.contains( JSN_TOTAL_FIELDS_STR ) ? true : false );
        //Uncomment
    }
}

/**
  * @desc   :  This will called be used to load the product information from the database and save all
  *            product information in the ram variables
  * @param  :  NILL
  * @return :  NILL
**/
bool dbRoutines::DB_getProductInfo( st_ProductDBInfo *_dbInfo )
{
    QString ioConfig, leakConfig, qrConfig;
    quint8 place1 = 0, place2 = 0;

    QString strQuery = QString( "SELECT * FROM %1 WHERE %2 = '%3';" )
                           .arg( DB_PRODUCT_INFO_TABLE ).arg( DB_PROD_NAME )
                           .arg( _dbInfo->productName );

    QSqlQuery sqlQuery( strQuery, dbConn );
    sqlQuery.exec();

    while( sqlQuery.next() )
    {
        /* Get Product Unique ID */
        _dbInfo->prodUniqueId = sqlQuery.value( DB_PROD_UNIQUE_ID ).toInt();

        /* Get Product Creator Name */
        memcpy( &_dbInfo->createrName, sqlQuery.value( DB_PROD_CREATOR ).toString().toStdString().c_str(),
               PRODUCT_CREATER_NAME_LENGTH );

        /* Get Product Creation Date */
        quint64 creeatdOnEpoch = sqlQuery.value( DB_PROD_CREATED_ON ).toULongLong();
        memcpy( &_dbInfo->createdOn,
               QDateTime::fromSecsSinceEpoch( creeatdOnEpoch ).toString( "dd/MM/yyyy hh:mm:ss" ).toStdString().c_str(),
               DATE_FIELD_LENGTH );
        qDebug() << "CreatedON : " << _dbInfo->createdOn;

        /* Get total Configured Nodes For the Product */
        _dbInfo->totalNodes = sqlQuery.value( DB_PROD_TOTAL_NODES ).toInt();

        /* Get total Configured Inputs Outputs For the Product */
        _dbInfo->totalInputOutputs = sqlQuery.value( DB_PROD_TOTAL_IO ).toInt();

        /* Get total Day wise Succcessful count for the Product */
        _dbInfo->totalDayWiseCount = sqlQuery.value( DB_DAY_WISE_PRODUCT_COUNT ).toInt();

        /* Get Product Wise Serial number Start */
        _dbInfo->productSerialNoStart = sqlQuery.value( DB_PROD_UID_START_FROM_STR ).toULongLong();

        /* Return If Values are not correct */
        if( _dbInfo->totalNodes <= 0 ||
            _dbInfo->totalInputOutputs <= 0 )
            return false;

        /* Total tested product */
        _dbInfo->totalTestedProduct = sqlQuery.value( DB_SUCCESSFUL_TEST_COUNT_PARAM ).toInt();

        /* Get Input Output for each Node Data*/
        ioConfig = sqlQuery.value( DB_PROD_IO_CONFIG ).toString();

        /* Get Leak Test Configuration */
        leakConfig = sqlQuery.value( DB_PROD_LEAK_CONFIG ).toString();

        /* Get QR Configuration */
        qrConfig = sqlQuery.value( DB_PROD_QR_CONFIG ).toString();

        // Respective of any mode we are reading all product information
        //getProductInfoFromJsn( &ioConfig, &leakConfig, &qrConfig, _dbInfo, TEST_MODE );//Uncomment

        /* Get Product Learn Data */
        if( sqlQuery.value( DB_PROD_LEARN_DATA ).toString() == "NIL" )
        {
            _dbInfo->isProductLearned = 0;
            qDebug() << "Node Number ::: " << _dbInfo->totalNodes;
            return true;
        }
        else
        {
            QString strlearnData = sqlQuery.value( DB_PROD_LEARN_DATA ).toString();

            QJsonDocument jsonDoc = QJsonDocument::fromJson( strlearnData.toUtf8() );
            QJsonObject rootJson = jsonDoc.object();

            _dbInfo->isProductLearned = 1;
            memcpy( &_dbInfo->learnedOn, sqlQuery.value( DB_PROD_LEARNED_ON ).toString().toStdString().c_str(),
                   DATE_FIELD_LENGTH );
            qDebug() << "LearnedON : " << _dbInfo->learnedOn;

            for( quint8 nodeNum = 0; nodeNum < _dbInfo->totalNodes; nodeNum++ )
            {
                QString nodeNumStr = QString( "n_%1" ).arg( nodeNum + 1 );
                if( rootJson.contains( nodeNumStr ) )
                {
                    QJsonObject jsonNodeNumObj = rootJson[ nodeNumStr ].toObject();
                    for( quint8 nodeOutNum = 0; nodeOutNum < _dbInfo->nodeInOuts[ nodeNum ]; nodeOutNum++ )
                    {
                        QString nodeOutNumber = QString( "o_%1" ).arg( nodeOutNum + 1 );
                        if( jsonNodeNumObj.contains( nodeOutNumber ) )
                        {
                            QJsonObject jsonLastObj = jsonNodeNumObj[ nodeOutNumber ].toObject();
                            for( quint8 ite = 0; ite < _dbInfo->totalNodes; ite++ )
                            {
                                place1 = ( ite * 2 );
                                place2 = ( ite * 2 ) + 1;

                                QString nodeOutValues = QString( "%1" ).arg( ite + 1 );
                                QJsonArray jsnArray = jsonLastObj.value( nodeOutValues ).toArray();

                                //                                qDebug() << "[ " << nodeNum << " ][ " << nodeOutNum << " ][ " << place1 << " ] : "
                                //                                         << jsnArray.at( 0 ).toVariant().toUInt();
                                //                                qDebug() << "[ " << nodeNum << " ][ " << nodeOutNum << " ][ " << place2 << " ] : "
                                //                                         << jsnArray.at( 1 ).toVariant().toUInt();

                                _dbInfo->digitalInputValues[ nodeNum ][ nodeOutNum ][ place1 ] =
                                    jsnArray.at( 0 ).toVariant().toUInt();
                                _dbInfo->digitalInputValues[ nodeNum ][ nodeOutNum ][ place2 ] =
                                    jsnArray.at( 1 ).toVariant().toUInt();
                            }
                        }
                    }
                }
            }
            //_dbInfo->totalConnections = rootJson.value( MQTT_TOTAL_CONNECTION_STR ).toInt();
            //Uncomment
        }
    }
    _dbInfo->prodFetchedFromDB = 1;

    qDebug() << "Node Number ::: " << _dbInfo->totalNodes;

    return true;
}

en_dbErrors dbRoutines::DB_setProductInfoByColumn( QString _columnName, QString _prodname, QString _value )
{
    QString strQuery;
    strQuery = QString( "UPDATE %1 SET %2 = '%3' WHERE %4 = '%5';" )
                   .arg( DB_PRODUCT_INFO_TABLE ).arg( DB_PROD_CLOUD_UPDATE_REQ_STR )
                   .arg( 0 ).arg( DB_PROD_NAME ).arg( _prodname );

    //qDebug() << "Query : " << strQuery;
    QSqlQuery sqlQuery( strQuery, dbConn );

    if( !sqlQuery.exec() )
    {
        qDebug() << sqlQuery.lastError().text();
        return QUERY_EXECUTION_ERROR;
    }
    else
    {
        return QUERY_EXECUTED_SUCCESSFULLY;
    }
}

en_dbErrors dbRoutines::DB_getSpecificColumnValueFromProductInfo( QString _prodName, QString _columnStr, QString *_value )
{
    QString strQuery = QString( "SELECT %1 FROM %2 WHERE %2 = '%3';" )
                           .arg( _columnStr ).arg( DB_PRODUCT_INFO_TABLE ).arg( DB_PROD_NAME )
                           .arg( _prodName );

    QSqlQuery sqlQuery( strQuery, dbConn );

    if( !sqlQuery.exec() )
    {
        qDebug() << sqlQuery.lastError().text();
        return QUERY_EXECUTION_ERROR;
    }
    else
    {
        QString columnValue = sqlQuery.value( DB_PROD_CLOUD_UPDATE_REQ_STR ).toString();
        memcpy( _value, columnValue.toUtf8(), columnValue.length() );
        return QUERY_EXECUTED_SUCCESSFULLY;
    }
}

bool dbRoutines::DB_executeSqlQuery( QString *_strQuery )
{
    QSqlQuery query( dbConn );
    query.prepare( *_strQuery );

    if( !query.exec() )
    {
        qDebug() << query.lastError().text();
        nonBlockingPopupMessage( ERROR_MESG, "Database Error!" );
        return false;
    }
    else
    {
        //displayPopupMessage( INFO_MESG, "Saved Successfully!", "Info" );
        return true;
    }
}

bool dbRoutines::DB_GetproductInfoForCloudUpdate( st_ProductDBInfo *_dbInfo, quint8 *_prodCount, quint8 *_cloudStatus,
                                                 QString *ioConfig, QString *lConfig, QString *qrCOnfig, QString *learnData )
{
    QString strQuery = QString( "SELECT * FROM %1 WHERE %2 != %3 ORDER BY %4 DESC LIMIT %5;" )
                           .arg( DB_PRODUCT_INFO_TABLE )
                           .arg( DB_PROD_CLOUD_UPDATE_REQ_STR ).arg( 0 )
                           .arg( DB_PROD_UNIQUE_ID ).arg( 1 );

    QSqlQuery sqlQuery( strQuery, dbConn );
    sqlQuery.exec();

    qDebug() << "Query : " << strQuery;
    while( sqlQuery.next() )
    {
        (*_prodCount)++;
        *_cloudStatus = sqlQuery.value( DB_PROD_CLOUD_UPDATE_REQ_STR ).toInt();
        /* Get Product Unique ID */
        _dbInfo->prodUniqueId = sqlQuery.value( DB_PROD_UNIQUE_ID ).toInt();

        /* Get Product Creator Name */
        memcpy( &_dbInfo->productName, sqlQuery.value( DB_PROD_NAME ).toString().toStdString().c_str(),
               PRODUCT_NAME_LENGTH );

        /* Get Product Creator Name */
        memcpy( &_dbInfo->createrName, sqlQuery.value( DB_PROD_CREATOR ).toString().toStdString().c_str(),
               PRODUCT_CREATER_NAME_LENGTH );

        /* Get Product Creation Date */
        memcpy( &_dbInfo->createdOn, sqlQuery.value( DB_PROD_CREATED_ON ).toString().toStdString().c_str(),
               DATE_FIELD_LENGTH );

        /* Get total Configured Nodes For the Product */
        _dbInfo->totalNodes = sqlQuery.value( DB_PROD_TOTAL_NODES ).toInt();

        /* Get total Configured Inputs Outputs For the Product */
        _dbInfo->totalInputOutputs = sqlQuery.value( DB_PROD_TOTAL_IO ).toInt();

        /* Get Product Wise Serial number Start */
        _dbInfo->productSerialNoStart = sqlQuery.value( DB_PROD_UID_START_FROM_STR ).toULongLong();

        /* Return If Values are not correct */
        if( _dbInfo->totalNodes <= 0 ||
            _dbInfo->totalInputOutputs <= 0 )
            return false;

        /* Total tested product */
        _dbInfo->totalTestedProduct = sqlQuery.value( DB_SUCCESSFUL_TEST_COUNT_PARAM ).toInt();

        /* Get Input Output for each Node Data*/
        *ioConfig = sqlQuery.value( DB_PROD_IO_CONFIG ).toString();

        /* Get Leak Test Configuration */
        *lConfig = sqlQuery.value( DB_PROD_LEAK_CONFIG ).toString();

        /* Get QR Configuration */
        *qrCOnfig = sqlQuery.value( DB_PROD_QR_CONFIG ).toString();

        *learnData = sqlQuery.value( DB_PROD_LEARN_DATA ).toString();
    }
    _dbInfo->prodFetchedFromDB = 1;
    return true;
}

bool dbRoutines::DB_GetProductCount( QString _productCount, quint32 *_count )
{
    QString strQuery = QString( "SELECT COUNT( * ) FROM %1 where %2='%3'" )
                           .arg( DB_PRODUCT_INFO_TABLE ).arg( DB_PROD_NAME ).arg( _productCount );
    QSqlQuery sqlQuery( strQuery, dbConn );
    sqlQuery.exec();

    while( sqlQuery.next() )
    {
        qDebug() << "Debug File Number : " << sqlQuery.value(0).toUInt( );
        *_count = sqlQuery.value(0).toUInt( );
        return true;
    }
    return false;
}


