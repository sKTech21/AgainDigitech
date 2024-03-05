#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    excelOperations *ObjExcelOperations;
    ObjExcelOperations = new excelOperations();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_openButton_clicked()
{

    QString fileName = QFileDialog::getOpenFileName(this,"Open Excel File", "","Excel Files (*.xlsx *.xls)");

    if (!fileName.isEmpty()) {
        // Use the fileName for opening the Excel file with ActiveX:
        //QAxObject *excel = new QAxObject("Excel.Application", this);
    }

    QAxObject *excel = new QAxObject("Excel.Application", this);
    //Below will open the Excelsheet in sheet editor(i.e msExcel or etc), we dont need it
    //excel->setProperty("Visible", true);

    QAxObject *workbooks = excel->querySubObject("Workbooks");
    QAxObject *workbook = workbooks->querySubObject("Open(const QString&)", fileName);
    sheet = workbook->querySubObject("Worksheets(int)", 1);

    if (workbook) {
        QAxObject *sheets = workbook->querySubObject("Worksheets");
        QAxObject *mainPageSheet = sheets->querySubObject("Item(const QVariant&)", QVariant("MainPage"));


        if (mainPageSheet) {
            ExcelVariables excelVarsForMainSheet;
            QString prodName = mainPageSheet->querySubObject("Range(const QString&)", excelVarsForMainSheet.mainPage.prodName)->property("Value").toString();
            QString prodCreator = mainPageSheet->querySubObject("Range(const QString&)", excelVarsForMainSheet.mainPage.prodCreator)->property("Value").toString();
            QString totalNodes = mainPageSheet->querySubObject("Range(const QString&)", excelVarsForMainSheet.mainPage.totalNodes)->property("Value").toString();
            QString totalIOs = mainPageSheet->querySubObject("Range(const QString&)", excelVarsForMainSheet.mainPage.totalIOs)->property("Value").toString();
            QString totalConnectors = mainPageSheet->querySubObject("Range(const QString&)", excelVarsForMainSheet.mainPage.totalConnectors)->property("Value").toString();
            QString totalSwitches = mainPageSheet->querySubObject("Range(const QString&)", excelVarsForMainSheet.mainPage.totalSwitches)->property("Value").toString();

            qDebug() << "Product Name:" << prodName;
            qDebug() << "Product Creator:" << prodCreator;
            qDebug() << "totalNodes ::" << totalNodes;
            qDebug() << "totalIOs ::" << totalIOs;
            qDebug() << "totalConnectors ::" << totalConnectors;
            qDebug() << "totalSwitches ::" << totalSwitches;

            //Table - 1
            QString nodeNum = mainPageSheet->querySubObject("Range(const QString&)", excelVarsForMainSheet.mainPage.table1.nodeNum)->property("Value").toString();
            QString nodeConnector = mainPageSheet->querySubObject("Range(const QString&)", excelVarsForMainSheet.mainPage.table1.nodeConnector)->property("Value").toString();
            QString nodeLeakTestStatus = mainPageSheet->querySubObject("Range(const QString&)", excelVarsForMainSheet.mainPage.table1.nodeLeakTestStatus)->property("Value").toString();
            QString nodeTotalIOs = mainPageSheet->querySubObject("Range(const QString&)", excelVarsForMainSheet.mainPage.table1.nodeTotalIOs)->property("Value").toString();
            QString nodeTotalSwitches = mainPageSheet->querySubObject("Range(const QString&)", excelVarsForMainSheet.mainPage.table1.nodeTotalSwitches)->property("Value").toString();

            qDebug() << "nodeNum ::" << nodeNum;
            qDebug() << "nodeConnector ::" << nodeConnector;
            qDebug() << "nodeLeakTestStatus ::" << nodeLeakTestStatus;
            qDebug() << "nodeTotalIOs ::" << nodeTotalIOs;
            qDebug() << "nodeTotalSwitches ::" << nodeTotalSwitches;

            //Table-2
            QString nodeNumTableTable2 = mainPageSheet->querySubObject("Range(const QString&)", excelVarsForMainSheet.mainPage.table2.nodeNum)->property("Value").toString();
            QString nodeConnectorTable2 = mainPageSheet->querySubObject("Range(const QString&)", excelVarsForMainSheet.mainPage.table2.nodeConnector)->property("Value").toString();
            QString connIOsTable2 = mainPageSheet->querySubObject("Range(const QString&)", excelVarsForMainSheet.mainPage.table2.connIOs)->property("Value").toString();
            QString connSwitchesTable2 = mainPageSheet->querySubObject("Range(const QString&)", excelVarsForMainSheet.mainPage.table2.connSwitches)->property("Value").toString();
            QString connPartNumberTable2 = mainPageSheet->querySubObject("Range(const QString&)", excelVarsForMainSheet.mainPage.table2.connPartNumber)->property("Value").toString();
            QString connLeakTestTypeTable2 = mainPageSheet->querySubObject("Range(const QString&)", excelVarsForMainSheet.mainPage.table2.connLeakTestType)->property("Value").toString();

            qDebug() << "nodeNumTableTable2 ::" << nodeNumTableTable2;
            qDebug() << "nodeConnectorTable2 ::" << nodeConnectorTable2;
            qDebug() << "connIOsTable2 ::" << connIOsTable2;
            qDebug() << "connSwitchesTable2 ::" << connSwitchesTable2;
            qDebug() << "connPartNumberTable2 ::" << connPartNumberTable2;
            qDebug() << "connLeakTestTypeTable2 ::" << connLeakTestTypeTable2;

            delete mainPageSheet;
        }

        QAxObject *ConnectorsSheet = sheets->querySubObject("Item(const QVariant&)", QVariant("Connectors"));
        if (ConnectorsSheet) {
            ExcelVariables excelVarsForConnectorSheet;
            QString nodeNum = ConnectorsSheet->querySubObject("Range(const QString&)", excelVarsForConnectorSheet.connectors.table1.nodeNum)->property("Value").toString();
            QString nodeConnector = ConnectorsSheet->querySubObject("Range(const QString&)", excelVarsForConnectorSheet.connectors.table1.nodeConnector)->property("Value").toString();
            QString connPartNumber = ConnectorsSheet->querySubObject("Range(const QString&)", excelVarsForConnectorSheet.connectors.table1.connPartNumber)->property("Value").toString();
            QString pinNum = ConnectorsSheet->querySubObject("Range(const QString&)", excelVarsForConnectorSheet.connectors.table1.pinNum)->property("Value").toString();
            QString pinType = ConnectorsSheet->querySubObject("Range(const QString&)", excelVarsForConnectorSheet.connectors.table1.pinType)->property("Value").toString();
            QString cavityNumber = ConnectorsSheet->querySubObject("Range(const QString&)", excelVarsForConnectorSheet.connectors.table1.cavityNumber)->property("Value").toString();
            QString cavityName = ConnectorsSheet->querySubObject("Range(const QString&)", excelVarsForConnectorSheet.connectors.table1.cavityName)->property("Value").toString();
            QString cavityDescription = ConnectorsSheet->querySubObject("Range(const QString&)", excelVarsForConnectorSheet.connectors.table1.cavityDescription)->property("Value").toString();
            QString circuitNumber = ConnectorsSheet->querySubObject("Range(const QString&)", excelVarsForConnectorSheet.connectors.table1.circuitNumber)->property("Value").toString();
            QString color = ConnectorsSheet->querySubObject("Range(const QString&)", excelVarsForConnectorSheet.connectors.table1.color)->property("Value").toString();

            //Table-1
            qDebug() << "nodeNum ::" << nodeNum;
            qDebug() << "nodeConnector:" << nodeConnector;
            qDebug() << "connPartNumber:" << connPartNumber;
            qDebug() << "pinNum:" << pinNum;
            qDebug() << "pinType:" << pinType;
            qDebug() << "cavityNumber:" << cavityNumber;
            qDebug() << "cavityName:" << cavityName;
            qDebug() << "cavityDescription:" << cavityDescription;
            qDebug() << "circuitNumber:" << circuitNumber;
            qDebug() << "color:" << color;
            // Print other fetched data

            delete ConnectorsSheet;
        }

        delete sheets;
        delete workbook;
    }
    delete workbooks;
}
