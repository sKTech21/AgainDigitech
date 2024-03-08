/*
 * continuity.h
 *
 *  Created on: May 21, 2023
 *      Author: pbhum
 */

#ifndef MAIN_INCL_CONTINUITY_H_
#define MAIN_INCL_CONTINUITY_H_

//#define CHECK_FOR_EXACT_CLOCK_INSTANT

#define MAX_SUPPORTED_IOs_IN_HW					( 64 )
#define MAX_NODE_SUPPORTED						( 80 )
#define MAX_SCAN_VALUE_STATIC_BUFFER_FOR_NODES	( 40 )

#define MIN_SCAN_IDLE_WAIT_PERIOD				( 5 )
#define MAX_SCAN_IDLE_WAIT_PERIOD				( 15 )

#define MIN_SCAN_TIME_FOR_ONE_OUTPUT			( 5 )
#define MAX_SCAN_TIME_FOR_ONE_OUTPUT			( 15 )

#define TOTAL_OP_UNDER_1_4051             		( 8 )

//Parameters for Scanning Average Mechanism
#define DIGITAL_AVERAGING_COUNT					( 5 )
#define SCAN_AVERAGE_DELAY						( 10 ) //In Milli Seconds

#define GAURD_TIME_BETWEEN_TWO_NODE				( 50 ) //In Milli Seconds

#define ZERO_DETECTION_NODE_COUNT				( 0 )
#define NON_ZERO_DETECTION_NODE_COUNT			( 1 )

#define SCANNED_VALUE_NOT_REPORTED_TO_APP		( 0 )
#define SCANNED_VALUE_REPORTED_TO_APP			( 1 )
#define SCANNED_VALUE_REPORTING_NOT_REQUIRED	( 2 )

#define SCANNED_VALUE_SE

//typedef enum
//{
//	CONTINUITY_IDLE,		//Default State would be Idle in case no continuity operation is going on
//	CONTINUITY_SCAN_INIT,   //Variable initialization, Update GPIO, Update LEDs status if required
//	OPERATE_OUTPUT,			//Operate Output of the node for scanning
//	WAIT_FOR_SCANNING_PERIOD_START,	//Start Scanning all required Inputs
//	START_SCANNING,			//Scan For operated Output
//};

typedef enum
{
	CONTINUITY_PROCESS_IDLE,
	CONTINITY_OPERATE_OUTPUT_INIT,
	CONTINUITY_IO_SCANNING_INIT,
	CONTINUITY_OPERATE_OUTPUT,
	CONTINUITY_IO_SCANNING,
	CONTINUITY_MOVE_TO_NEXT_NODE,
	CONTINUITY_MESG_TO_APP_MONITORING,
}continuityIOOperatingState;

/*
 *	This structure will get updated on receiving Product configuration from application
 *	before starting the process.
 *	Some of the members like idlePeriodAfterScanning, allowedScanningPeriod, startScanningOn,
 *	totalScanningPeriodForOneOp, scanningRequired, scaningForOwnNode will only get updated if
 *	Application will send configuration message ten times with 10 ms Difference and wait for
 *	Acknowledgment from nodes for next 1 Second. If Ack not received within 1 Seconds from
 *	configured nodes, Application will send Configuration message again for 4 more tries.
 *	Max Time for Product configuration and its ack wait period would be around 5 to 5.5 Seconds
 *	Once Configuration received, and Process started at one go, All nodes will store Scanned value
 *	for 10 nodes and send value to application once all 10 nodes are done with doing Input and output
 *	operations.
 *	If Product is configured for more than 10 nodes, Application will again repeat the same steps as done
 *	for first 10 nodes like send once again new configuration and send it to the nodes.
 */
typedef struct
{
	uint8_t 	IOassociatedWithNode[ MAX_NODE_SUPPORTED ];
	uint8_t 	totalProductNodes;
	uint8_t 	reportingDelay;

	uint8_t		startScanningFromNode;		 //This is the node number from which Node has to start scan for
	uint8_t		noOfNodesToBeScanned;		 //This is the total number of nodes to be scanned continuously in one time
	uint64_t 	startScanningOn[ MAX_SCAN_VALUE_STATIC_BUFFER_FOR_NODES ];
	                                         //In ms since power on, Used Offset bound Ms Period
	uint16_t	allowedScanningPeriod;	     //Total duration allowed for scanning of max 64 inputs for one output
	uint16_t	idlePeriodAfterScanning;     //Idle period between two scanning for two different outputs scanning
	uint16_t	totalScanningPeriodForOneOp; //At this period node will again start scanning the input for
	                                         //different output
	uint64_t 	productUniqueID;			 //Expected Product unique ID for which Scanning is happening
}continuityProdConfiguration_t;

typedef struct
{
	uint8_t 	currentOutput;				//Output Just Operated
	uint64_t 	outputOperatedOn;
}operateOutputStatus_t;

typedef struct
{
	uint8_t 	scanningRequired; 			//1 = Scanning Required else 0
	uint8_t 	scanningForNode;			//Scanning happening for the node
	uint8_t		scanningForOutput;			//Scanning happening for the output
}scanProcessStatus_t;

typedef struct
{
	uint32_t 	value_1;   					//Value of 1 - 32 Bit Digital Input
    uint32_t 	value_2;   					//Value of 32 - 64 Bit Digital Input
    uint16_t 	noOfConnectionOnPIN;	    //Defined the total connection for the output
}scannedValues_t;

typedef struct
{
	//THis is the container will actually holds the Scanned value, rest other within this structure are just
	//status parameters only about performed scanning
	scannedValues_t	  	scannedValues[ MAX_SUPPORTED_IOs_IN_HW ];
	uint16_t 			noOfConnectionDetectedForNode;		//This holds the no Of Connection detected for entire node

	uint64_t			scanStartedOn;						//In Ms, Use Offset bound Ms Period
	uint64_t 			scanCompletedOn;					//In Ms, Use Offset bound Ms Period
	uint16_t			totalScanPeriod;					//Time taken by Node for scanning all outputs
	uint8_t 			totalOutputsScanned;    			//1 to 64 only possible
	uint8_t				scanningDone;						//1 = Scanning Done else 0
	uint8_t				scanPerformedFor;

	errorList_e			scanningError;
//	char 				scanningErrorString;
	uint32_t			scannedValueBitSetter;				//This is to make sure that, scanned value sent to application only for the
															//associated IO, rest all bits will be set to zero( means no input detection )
	uint8_t				scannedValueSentToApp;				//1 = Scanned Value Sent, else  0
	uint8_t				noOfTimesScanValSentToAPp;
	uint8_t				scanedValueReceivedByApp;			//1= Acknowledgment Received from Application else 0
	uint64_t			scannedValueSentOn;					//In Ms Bound to offset, scanned value sent to application

	uint32_t			w0To31ScannedStatus_1;
	uint32_t			w0To31ScannedStatus_2;
	uint32_t			w32To63ScannedStatus_1;
	uint32_t			w32To63ScannedStatus_2;
}scanValueStatus_t;

typedef struct
{
	uint8_t 			applrequestedForScannedVal;			//true = RequestReceived, else false
	uint8_t 			reSentRequired;
	uint8_t				currentlySending;
	uint8_t 			zeroDetectionValueSent;
	uint8_t 			nonZeroDetectionValueSent;
	uint8_t				zeroDetectionNodeCount;
	uint8_t				nonZeroDetectionNodeCount;
	uint64_t			scannedValueSentOn;					//In Ms Bound to offset, scanned value sent to application
}scanValReportingStatus_t;

void continuityTestTaskInit( void );
void stopContinuityProcess( void );
void resetcontinuityTestParams( void );
bool isContinuityScanningInProgress( void );
continuityIOOperatingState getContinuityProcessState( void );
bool setProductCOnfigurationParameters( continuityProdConfiguration_t _productCOnfigurations );
void prepareProcessErrorMesg( char* _mesg );
void prepareOperateOutputStartEventMesg( char* _mesg );
void prepareOperateOutputDoneEventMesg( char* _mesg );
void prepareScannedValueMesgForNextPossibleNode( char* _mesg );
void prepareZeroDetectionMessage( char* _mesg );
void calculateExpectedProcessTime( uint8_t totalNodes, uint8_t *_uintArray );
uint64_t getProcessExpectedStartTime( void );
uint8_t getNoOfIOsAssociatedWithNode( uint8_t _node );

bool hasApplReqToShareScannedValue( void );
void setApplReqToShareScannedValueStatus( uint8_t _status );
void processScanValueReceivedAckResponse( cJSON *_scanForNodeJsnArrObj, cJSON *_scanValRecAckJsnArrObj );
void scanDummyInputs( void );
void operateOutputForContinuityOperation( const uint8_t _outputNumber );

#endif /* MAIN_INCL_CONTINUITY_H_ */
