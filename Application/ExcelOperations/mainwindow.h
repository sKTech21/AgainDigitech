#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAxObject>
#include <QAxWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileDialog>
#include "exceloperations.h"

// Define struct to hold variable names for different sheets and tables
struct ExcelVariables {
    struct MainPage {
        QString prodName = "B1";
        QString prodCreator = "B2";
        QString totalNodes = "B3";
        QString totalIOs = "D3";
        QString totalConnectors = "F2";
        QString totalSwitches = "F3";

        struct Table1 {
            QString nodeNum = "B7";
            QString nodeConnector = "C7";
            QString nodeLeakTestStatus = "D7";
            QString nodeTotalIOs = "E7";
            QString nodeTotalSwitches = "F7";
        } table1;

        struct Table2 {
            QString nodeNum = "H7";
            QString nodeConnector = "I7";
            QString connIOs = "J7";
            QString connSwitches = "K7";
            QString connPartNumber = "L7";
            QString connLeakTestType = "M7";
        } table2;
    } mainPage;

    struct Connectors {
        struct Table1 {
            QString nodeNum = "B2";
            QString nodeConnector = "C2";
            QString connPartNumber = "D2";
            QString pinNum = "E2";
            QString pinType = "F2";
            QString cavityNumber = "G2";
            QString cavityName = "H2";
            QString cavityDescription = "I2";
            QString circuitNumber = "J2";
            QString color = "K2";
        } table1;
    } connectors;
};


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_openButton_clicked();

private:
    Ui::MainWindow *ui;
    QAxObject *sheet;
    excelOperations *ObjExcelOperations;
};
#endif // MAINWINDOW_H
