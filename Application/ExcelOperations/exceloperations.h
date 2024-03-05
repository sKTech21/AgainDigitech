#ifndef EXCELOPERATIONS_H
#define EXCELOPERATIONS_H

#include <QObject>
#include <QAxObject>
#include <QAxWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileDialog>

#define MAX_SUPPORTED_NODE					( 80 )
#define MAX_IO_WITHIN_CONNECTOR 			( 128 )
#define MAX_CONNECTORS_WITHIN_NDOE			( 8 )
#define CONN_PART_NUMBER_LENGTH				( 32 )
#define CAVITY_NUMBER_LENGTH				( 8 )
#define CAVITY_NAME_LENGTH					( 16 )
#define CAVITY_DESCRIPTION_LENGTH			( 32 )
#define CIRCUITY_NUMBER_LENGTH				( 8 )
#define WIRE_COLOR_LENGTH					( 8 )
#define PRODUCT_NAME_LENGTH					( 32 )
#define PRODUCT_CREATOR_NAME_LENGTH			( 32 )
#define CELL_INDEX_IDENTIFIER_LENGTH		( 4 )

typedef enum
{
    MAINPAGE_PROD_NAME = 0,
    MAINPAGE_PROD_CREATOR,
    MAINPAGE_TOTAL_NODES,
    MAINPAGE_TOTAL_IOS,
    MAINPAGE_TOTAL_CONNECTORS,
    MAINPAGE_TOTAL_SWITCHES,

    MAINPAGE_T1_NODE_NUM,
    MAINPAGE_T1_CONNECTORS,
    MAINPAGE_T1_LEAK_TEST_STATUS,
    MAINPAGE_T1_NODE_IOS,
    MAINPAGE_T1_NODE_SWITCHES,

    MAINPAGE_T2_NODE_NUM,
    MAINPAGE_T2_CONNECTOR_NUM,
    MAINPAGE_T2_NO_OF_PINS,
    MAINPAGE_T2_NO_OF_SW_FOR_CONNECTOR,
    MAINPAGE_T2_CONNECTOR_PART_NUM,
    MAINPAGE_T2_LEAK_TEST,

    CONNECTORS_NODE_NUM,
    CONNECTORS_CONNECTOR_NUM,
    CONNECTORS_CONNECTORS_PART_NUM,
    CONNECTORS_PIN,
    CONNECTORS_PIN_TYPE,
    CONNECTORS_CAVITY_NUM,
    CONNECTORS_CAVITY_NAME,
    CONNECTORS_CAVITY_DESCRIPTION,
    CONNECTORS_CIRCUIT_NUMBER,
    CONNECTORS_COLOR,
    CONNECTORS_COLOR_1,
    CONNECTORS_COLOR_2,
    CONNECTORS_COLOR_3,

    MAX_CELL_INDEX
}en_CellIndex;


typedef enum
{
    NO_LEAK_TEST,
    LEAK_TEST_APlus,
    LEAK_TEST_AMinus,
    LEAK_TEST_APlusAndAMinus,
    LEAK_TEST_BPlus,
    LEAK_TEST_BMinus,
    INVALID_LEAK_TEST
}en_LeakType;

typedef struct
{
    en_CellIndex 	_cellIndex;
    char            _cellKey[ 32 ];
    char 			_cellValue[ 8 ];
}st_excelReadInfo;

typedef struct
{
    char	productName[PRODUCT_NAME_LENGTH ];
    char	productCreator[ PRODUCT_CREATOR_NAME_LENGTH ];
    uint8_t     noOfNodes;			//Limited to max 80 nodes
    uint16_t	noOfConnectors;		//Each node can have max 8 connectors so max possible connectors are 80*8 = 640
    uint16_t	noOfTotalIos;		//Each node can have max 128 IOs, so max possible IOs are 80*128 = 10,240
    uint16_t	noOfSwitches;		//Each node can have max 4 Switches, so max possible Switches are 80*8*4 = 2560
}st_prodBasicInfo;

typedef struct
{
    uint8_t	nodeLeakStatus; 	//1 = Enabled, 0 = Disabled, Can accomodate the leak status for total 96 node which is higher than max limit which is 80
    uint8_t	nodeConnectors; 	//Total Connectors Within Node, Each Node can have max 8 connectors
    uint8_t	nodeIOs;		   	//Total Ios within Node, Each node can have max 128 IOs.
    uint8_t	nodeSwitches;	   	//Total switches within node	//Max 4 switches are allowed per connector
}st_prodPrimaryInfo;


class excelOperations : public QObject
{
    Q_OBJECT
public:
    excelOperations();

private:
    QAxObject *sheet;
    st_prodBasicInfo prodBasicInfo;
    st_prodPrimaryInfo prodPrimaryInfo[ MAX_SUPPORTED_NODE ];

    QString getProductName(QAxObject *mainPageSheet);
    void readBasicInfo(QAxObject* mainPageSheet);
    QString getCellLocation(en_CellIndex index);
    QString getProductCreator(QAxObject* mainPageSheet);
    uint8_t getNumberOfNodes(QAxObject* mainPageSheet);
    uint16_t getNumberOfConnectors(QAxObject* mainPageSheet);
    uint16_t getNumberOfTotalIOs(QAxObject* mainPageSheet);
    uint16_t getNumberOfSwitches(QAxObject* mainPageSheet);
};

#endif // EXCELOPERATIONS_H
