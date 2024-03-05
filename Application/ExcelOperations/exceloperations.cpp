#include "exceloperations.h"
#include <QDebug>
st_excelReadInfo excelInfo[ MAX_CELL_INDEX ] = {
    { MAINPAGE_PROD_NAME,                   "prodName",             "B1" },
    { MAINPAGE_PROD_CREATOR,                "prodCreator",          "B2" },
    { MAINPAGE_TOTAL_NODES,                 "totalNodes",           "B3" },
    { MAINPAGE_TOTAL_IOS,                   "totalIOs",             "D3" },
    { MAINPAGE_TOTAL_CONNECTORS,            "totalConnectors",      "F2" },
    { MAINPAGE_TOTAL_SWITCHES,              "totalSwitches",        "F3" },

    { MAINPAGE_T1_NODE_NUM,                 "nodeNum_T1",           "B7" },
    { MAINPAGE_T1_CONNECTORS,               "nodeConnector_T1", 	"C7" },
    { MAINPAGE_T1_LEAK_TEST_STATUS,         "nodeLeakTestStatus",	"D7" },
    { MAINPAGE_T1_NODE_IOS,                 "nodeTotalIOs", 		"E7" },
    { MAINPAGE_T1_NODE_SWITCHES,            "nodeTotalSwitches", 	"F7" },

    { MAINPAGE_T2_NODE_NUM,                 "nodeNum_T2",           "H7" },
    { MAINPAGE_T2_CONNECTOR_NUM,            "nodeConnector_T2", 	"I7" },
    { MAINPAGE_T2_NO_OF_PINS,               "connIOs",              "J7" },
    { MAINPAGE_T2_NO_OF_SW_FOR_CONNECTOR,   "connSwitches", 		"K7" },
    { MAINPAGE_T2_CONNECTOR_PART_NUM,       "connPartNumber_T2",	"L7" },
    { MAINPAGE_T2_LEAK_TEST,                "connLeakTestType", 	"M7" },

    { CONNECTORS_NODE_NUM,                  "nodeNum_T3",           "B3" },
    { CONNECTORS_CONNECTOR_NUM,             "nodeConnector_T3",     "C3" },
    { CONNECTORS_CONNECTORS_PART_NUM,       "connPartNumber_T3",	"D3" },
    { CONNECTORS_PIN,                       "pinNum ",              "E3" },
    { CONNECTORS_PIN_TYPE,                  "pinType",              "F3" },
    { CONNECTORS_CAVITY_NUM,                "cavityNumber", 		"G3" },
    { CONNECTORS_CAVITY_NAME,               "cavityName",           "H3" },
    { CONNECTORS_CAVITY_DESCRIPTION,        "cavityDescription",	"I3" },
    { CONNECTORS_CIRCUIT_NUMBER,            "circuitNumber", 		"J3" },
    { CONNECTORS_COLOR,                     "color",                "K3" },
    { CONNECTORS_COLOR_1,                   "color1",               "L3" },
    { CONNECTORS_COLOR_2,                   "color2",               "M3" },
    { CONNECTORS_COLOR_3,                   "color3",               "N3" },
    };


excelOperations::excelOperations() {

    st_prodBasicInfo prodBasicInfo;
    st_prodPrimaryInfo prodPrimaryInfo[ MAX_SUPPORTED_NODE ];

    memset( &prodBasicInfo, '\0', sizeof( prodBasicInfo ) );
    memset( prodPrimaryInfo, '\0', sizeof( prodPrimaryInfo ));

    QAxObject *excel = new QAxObject("Excel.Application", this);

    QAxObject *workbooks = excel->querySubObject("Workbooks");
    QAxObject *workbook = workbooks->querySubObject("Open(const QString&)", "C:\\Users\\admin\\Downloads\\KKK.xlsx");
    sheet = workbook->querySubObject("Worksheets(int)", 1);

    if (!workbook) {
        return;
    }
    QAxObject *sheets = workbook->querySubObject("Worksheets");
    QAxObject *mainPageSheet = sheets->querySubObject("Item(const QVariant&)", QVariant("MainPage"));
    //QAxObject *ConnectorsSheet = sheets->querySubObject("Item(const QVariant&)", QVariant("Connectors"));



    readBasicInfo(mainPageSheet);
    qDebug()<< "FINISH.....";

    delete sheets;
    //delete ConnectorsSheet;
    delete mainPageSheet;
    delete workbook;
}
QString excelOperations::getCellLocation(en_CellIndex index)
{
    if( index < MAX_CELL_INDEX )
    {
        return excelInfo[ index ]._cellValue;
    }
    return "true";
}

void excelOperations::readBasicInfo( QAxObject* mainPageSheet )
{
    QString productNameQString = getProductName(mainPageSheet);
    QString productNameTruncated = productNameQString.left(PRODUCT_NAME_LENGTH - 1); // Ensure length doesn't exceed array size
    qstrcpy(prodBasicInfo.productName, productNameTruncated.toUtf8().constData());
    qDebug()<<"prodBasicInfo.productName=" << prodBasicInfo.productName;

    QString productCreatorQString = getProductCreator(mainPageSheet);
    QString productCreatorTruncated = productCreatorQString.left(PRODUCT_CREATOR_NAME_LENGTH - 1); // Ensure length doesn't exceed array size
    qstrcpy(prodBasicInfo.productCreator, productCreatorTruncated.toUtf8().constData());
    qDebug()<<"prodBasicInfo.productName=" << prodBasicInfo.productCreator;

    prodBasicInfo.noOfNodes = getNumberOfNodes(mainPageSheet);
    qDebug()<<"prodBasicInfo.noOfNodes=" << prodBasicInfo.noOfNodes;

    prodBasicInfo.noOfConnectors = getNumberOfNodes(mainPageSheet);
    qDebug()<<"prodBasicInfo.noOfConnectors=" << prodBasicInfo.noOfConnectors;

    prodBasicInfo.noOfTotalIos = getNumberOfTotalIOs(mainPageSheet);
    qDebug()<<"prodBasicInfo.noOfTotalIos=" << prodBasicInfo.noOfTotalIos;

    prodBasicInfo.noOfSwitches = getNumberOfSwitches(mainPageSheet);
    qDebug()<<"prodBasicInfo.noOfSwitches=" << prodBasicInfo.noOfSwitches;

}

QString excelOperations::getProductName(QAxObject* mainPageSheet)
{
    QAxObject* cell = mainPageSheet->querySubObject("Range(const QString&)", getCellLocation(MAINPAGE_PROD_NAME));
    if (!cell) {
        qDebug() << "Error: Cell not found at " << getCellLocation(MAINPAGE_PROD_NAME);
       // return; // Or handle the error as appropriate
    }

    QString prodName = mainPageSheet->querySubObject("Range(const QString&)",  getCellLocation( MAINPAGE_PROD_NAME ))->property("Value").toString();
    //qDebug()<< "prodName == " << prodName;
    return prodName;

}

QString excelOperations::getProductCreator(QAxObject* mainPageSheet)
{
    QAxObject* cell = mainPageSheet->querySubObject("Range(const QString&)", getCellLocation(MAINPAGE_PROD_CREATOR));
    if (!cell) {
        qDebug() << "Error: Cell not found at " << getCellLocation(MAINPAGE_PROD_CREATOR);
        // return; // Or handle the error as appropriate
    }

    QString prodCreatorName = mainPageSheet->querySubObject("Range(const QString&)",  getCellLocation( MAINPAGE_PROD_CREATOR ))->property("Value").toString();
    //qDebug()<< "prodCreatorName == " << prodCreatorName;
    return prodCreatorName;
}

uint8_t excelOperations::getNumberOfNodes(QAxObject* mainPageSheet)
{
    QAxObject* cell = mainPageSheet->querySubObject("Range(const QString&)", getCellLocation(MAINPAGE_TOTAL_NODES));
    if (!cell) {
        qDebug() << "Error: Cell not found at " << getCellLocation(MAINPAGE_TOTAL_NODES);
        // return; // Or handle the error as appropriate
    }

    uint8_t totalNodes = mainPageSheet->querySubObject("Range(const QString&)",  getCellLocation( MAINPAGE_TOTAL_NODES ))->property("Value").toUInt();
    //qDebug()<< "totalNodes == " << totalNodes;
    return totalNodes;
}

uint16_t excelOperations::getNumberOfConnectors(QAxObject* mainPageSheet)
{
    QAxObject* cell = mainPageSheet->querySubObject("Range(const QString&)", getCellLocation(MAINPAGE_TOTAL_CONNECTORS));
    if (!cell) {
        qDebug() << "Error: Cell not found at " << getCellLocation(MAINPAGE_TOTAL_CONNECTORS);
        // return; // Or handle the error as appropriate
    }

    uint8_t totalConnectors = mainPageSheet->querySubObject("Range(const QString&)",  getCellLocation( MAINPAGE_TOTAL_CONNECTORS ))->property("Value").toUInt();
    //qDebug()<< "totalConnectors == " << totalConnectors;
    return totalConnectors;
}

uint16_t excelOperations::getNumberOfTotalIOs(QAxObject* mainPageSheet)
{
    QAxObject* cell = mainPageSheet->querySubObject("Range(const QString&)", getCellLocation(MAINPAGE_TOTAL_IOS));
    if (!cell) {
        qDebug() << "Error: Cell not found at " << getCellLocation(MAINPAGE_TOTAL_IOS);
        // return; // Or handle the error as appropriate
    }

    uint8_t totalIoS = mainPageSheet->querySubObject("Range(const QString&)",  getCellLocation( MAINPAGE_TOTAL_IOS ))->property("Value").toUInt();
    //qDebug()<< "totalIoS == " << totalIoS;
    return totalIoS;
}

uint16_t excelOperations::getNumberOfSwitches(QAxObject* mainPageSheet)
{
    QAxObject* cell = mainPageSheet->querySubObject("Range(const QString&)", getCellLocation(MAINPAGE_TOTAL_SWITCHES));
    if (!cell) {
        qDebug() << "Error: Cell not found at " << getCellLocation(MAINPAGE_TOTAL_SWITCHES);
        // return; // Or handle the error as appropriate
    }

    uint8_t totalSwitches = mainPageSheet->querySubObject("Range(const QString&)",  getCellLocation( MAINPAGE_TOTAL_SWITCHES ))->property("Value").toUInt();
    //qDebug()<< "totalSwitches == " << totalSwitches;
    return totalSwitches;
}

