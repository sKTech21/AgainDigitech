/*
 * continuity.c
 *
 *  Created on: May 21, 2023
 *      Author: Palak Patel
 */

/*@To DO : Change the Task time monitoring based on the following logic
 * https://chat.openai.com/share/db7b442e-53f6-41d1-9086-596d94af8557
 * https://chat.openai.com/share/95fc0924-06e9-4a37-9771-535c74cd4cc4
 */

#include "../incl/commonDeclaration.h"
#include "../incl/continuity.h"
#include "../incl/wifiHandleing.h"
#include "esp_system.h"
#include "time.h"

#define MIN_OPERATE_OUTPUT_TIME_REQUIRED		( continuityProdConfiguration.idlePeriodAfterScanning - 2 )
#define MAX_OFFSET_FOR_SCANNING					( getClockValueAfterOffset( ) + 2 )

typedef struct
{
	uint64_t operateIOClkInstant;
	uint64_t scanIOClkInstant;
}scanningInstant;

static TaskHandle_t	continuityTestTaskHandler;
static continuityProdConfiguration_t continuityProdConfiguration;
static scanProcessStatus_t	scanProcessStatus;
static scanValueStatus_t scanValueStatus[ MAX_SCAN_VALUE_STATIC_BUFFER_FOR_NODES ];
static scanValReportingStatus_t scanValReportingSts;
static operateOutputStatus_t operateOutputStatus;
static volatile continuityIOOperatingState continuityProcessState = CONTINUITY_PROCESS_IDLE;
static scanningInstant scanInstant;

//Continuity Timer Variables
static TickType_t xFrequency; // Task execution frequency in ticks
static TickType_t xLastWakeTime;
static bool continuityOnTimer = false;

//Delete During Production, Variable below are only use for debugging purpose
static uint64_t opDiff = 0, scanDiff = 0;

static uint64_t processExpectedStartTime = 0;

static void continuityTestHandelingTask( void* arg );
static void setContinuityProcessState( continuityIOOperatingState _state );
static uint64_t getScanStartClockInstant( uint8_t _node );
static uint8_t getScannedValueArrayIndex( uint8_t _node );
static void processContinuityIOOperating( void );
//static void operateOutputForContinuityOperation( const uint8_t _outputNumber );
static void scanNodeAssociatedIOs( const uint8_t _nodeArrayIndex, const uint8_t _outpuNumber );
static uint8_t performScanningOnIO( uint8_t _IONumber );
static uint8_t prepareScannedValueMessage( char* _mesg, uint8_t _index );
static void startContinuityTimer( TickType_t _lastWakeTime, uint32_t _timerPerioTickCount );
static void stopContinuityTimer( );
static bool hasValideProductConfigurationReceived( continuityProdConfiguration_t *prodCOnfig );
static void resetReportingStatusForAllScanVal( void );
static uint8_t getNodeCountBasedOnConnection( uint8_t _zeroOrNonZero );
static uint8_t getNoOfScannedValueNotReportedToApp( void );
static bool hasAckReceivedForAllNodes( void );
static void resetSelfDetectionForSimilarPins( );

//Delete when not in use
void printProductConfiguration( void );

void continuityTestTaskInit( void )
{
	resetcontinuityTestParams( );
	xTaskCreatePinnedToCore( continuityTestHandelingTask, "continuity_test_handeling", 20000, NULL, 1, &continuityTestTaskHandler, 1 );
}

bool setProductCOnfigurationParameters( continuityProdConfiguration_t _productCOnfigurations )
{
	//@To DO: Uncomment the Below Product config Validation code once
	if( hasValideProductConfigurationReceived( &_productCOnfigurations ) )
	{
		resetcontinuityTestParams( );
		memcpy( &continuityProdConfiguration, &_productCOnfigurations, sizeof( continuityProdConfiguration_t ) );
		scanProcessStatus.scanningRequired = 1;
		return true;
	}
	else
	{
		//memcpy( &continuityProdConfiguration, &_productCOnfigurations, sizeof( continuityProdConfiguration_t ) );
		//scanProcessStatus.scanningRequired = 0;
		return false;
	}
}

static bool hasValideProductConfigurationReceived( continuityProdConfiguration_t *prodCOnfig )
{
	if( prodCOnfig->startScanningFromNode > prodCOnfig->totalProductNodes )
	{
		JSN_DBG_ERR( JSN_DBG_TAG, "PROCESS_START_ERR-3" );
		return false;
	}

	if( prodCOnfig->totalProductNodes > 1  )
	{
		uint8_t totalIOs = 0;
		uint16_t totalScanPeriodForOneOutput = 0, totalTimeForScanning = 0;
		uint64_t scanEndsAt = 0;

		for( uint8_t nodeNum = 0; nodeNum < prodCOnfig->totalProductNodes; nodeNum++ )
		{
			totalIOs = prodCOnfig->IOassociatedWithNode[ nodeNum ];
			totalScanPeriodForOneOutput = prodCOnfig->totalScanningPeriodForOneOp;
			totalTimeForScanning = totalIOs * totalScanPeriodForOneOutput;
			scanEndsAt = prodCOnfig->startScanningOn[ nodeNum ] + totalTimeForScanning + GAURD_TIME_BETWEEN_TWO_NODE;

			CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "tIO:%d, sP:%d, TTFs:%d, scaEndAT:%lld",
														totalIOs, totalScanPeriodForOneOutput,
														totalTimeForScanning, scanEndsAt );

			if( prodCOnfig->IOassociatedWithNode[ nodeNum ] > MAX_NODE_SUPPORTED )
			{
				CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "Invalid IOs for the Node : %d",
									( nodeNum + prodCOnfig->startScanningFromNode ) );
				return false;
			}

			//Scan Instant check is not neede when total nodes in the product = 1
			//Check for scan instant of first node only when total product node != 1
			/*
			if( nodeNum == 0 && prodCOnfig->totalProductNodes != 1 )
			{
				if( prodCOnfig->startScanningOn[ nodeNum ] >= prodCOnfig->startScanningOn[ nodeNum + 1 ] )
				{
					CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "Scanning Instant Error:%d, %lld, %lld, %lld, %lld",
							nodeNum, scanEndsAt, prodCOnfig->startScanningOn[ nodeNum + 1 ],
							prodCOnfig->startScanningOn[ nodeNum ], prodCOnfig->startScanningOn[ nodeNum + 1 ] );
					return false;
				}
			}*/

			if( ( nodeNum == 0 && prodCOnfig->totalProductNodes != 1 ) ||
				( nodeNum != 0 && ( (nodeNum + 1 ) != prodCOnfig->totalProductNodes ) ) )
			{
				if( prodCOnfig->startScanningOn[ nodeNum ] >= prodCOnfig->startScanningOn[ nodeNum + 1 ] ||
					scanEndsAt > prodCOnfig->startScanningOn[ nodeNum + 1 ] )
				{
					JSN_DBG_ERR( JSN_DBG_TAG, "Scan Instant Err:%d, %lld, %lld, %lld",
							   nodeNum, prodCOnfig->startScanningOn[ nodeNum ], scanEndsAt,
							   prodCOnfig->startScanningOn[ nodeNum + 1 ] );
					return false;
				}
			}
		}
	}
	return true;
}

//This function can be called when Application sends UDP Broadcast message to halt the running process
void stopContinuityProcess( void )
{
	resetcontinuityTestParams( );
}

void resetcontinuityTestParams( void )
{
	memset( &continuityProdConfiguration, '\0', sizeof( continuityProdConfiguration_t ) );
	memset( &scanProcessStatus, '\0', sizeof( scanProcessStatus_t ) );
	memset( &scanValueStatus[0], '\0', MAX_SCAN_VALUE_STATIC_BUFFER_FOR_NODES * sizeof( scanValueStatus_t ) );
	memset( &scanValReportingSts, '\0', sizeof( scanValReportingStatus_t ) );
	memset( &scanInstant, '\0', sizeof( scanningInstant ) );
	memset( &operateOutputStatus, '\0', sizeof( operateOutputStatus_t ) );
	continuityProcessState = CONTINUITY_PROCESS_IDLE;
}

static void startContinuityTimer( TickType_t _lastWakeTime, uint32_t _timerPerioTickCount )
{
//	if( _timerPerioTickCount != xFrequency )
//		CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "Timer:%ld,%lld,%lld,%ld", _timerPerioTickCount,( uint64_t )xTaskGetTickCount(),
//				esp_timer_get_time()/1000,
//				( _timerPerioTickCount>xFrequency ) ? ( _timerPerioTickCount - xFrequency):( xFrequency - _timerPerioTickCount ) );


	continuityOnTimer = true;
	xLastWakeTime = _lastWakeTime;
	xFrequency = (TickType_t)_timerPerioTickCount;
}

static void stopContinuityTimer( )
{
	continuityOnTimer = false;
	//CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "Tmr Stpd" );
}

static void continuityTestHandelingTask( void* arg )
{
	CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "CONTINUITY TASK CREATED" );
	while( 1 )
	{
		if( continuityOnTimer )
		{
			if( ( xTaskGetTickCount() - xLastWakeTime) >= xFrequency )
			{
				CONTINUITY_DBG_ERR( CONTINUITY_DBG_TAG, "T Exp" );
			}
			vTaskDelayUntil( &xLastWakeTime, xFrequency );
		}
		else
		{
			vTaskDelay( 1 / portTICK_PERIOD_MS );
		}
		processContinuityIOOperating( );
	}
	vTaskDelete( NULL );
}

static void processContinuityIOOperating( void )
{
	rtc_wdt_feed();
	switch( continuityProcessState )
	{
		case CONTINUITY_PROCESS_IDLE:
		{
			/*Will Remain in Idle State until process start signal is not received from Application*/
			if( getNodeNumber() <= continuityProdConfiguration.totalProductNodes &&
				scanProcessStatus.scanningRequired == 1 )
			{
				scanProcessStatus.scanningRequired = 0;
				CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "CONTINUITY STARTED for:%d on %lld",
										getNodeNumber(), CURRENT_CLOCK_INSTANT_WITH_OFFSET );

				printProductConfiguration( );

				//Check if Node number is 1 then it has to operate output first before scanning.
				//Lets Say if selected product have 20 nodes and at a time only 10 nodes are being scanned,
				//All nodes are expected to send Scanned value to the application.
				//Once all scanned value received from all nodes, Application will again send the Product configuration
				//But this time it will send another clock instant from where start of scanning is expected.
				//If such configuration is received and next scanning should start from 11th node only, If the node is 11 it has to
				//Operate the output also while scanning.
				if( getNodeNumber() == continuityProdConfiguration.startScanningFromNode )
				{
					setContinuityProcessState( CONTINITY_OPERATE_OUTPUT_INIT );
					CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "Moving CONTINITY_OPERATE_OUTPUT_INIT" );
				}
				else
				{
					scanProcessStatus.scanningForNode = continuityProdConfiguration.startScanningFromNode;
					setContinuityProcessState( CONTINUITY_IO_SCANNING_INIT );
					CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "Moving CONTINUITY_IO_SCANNING_INIT" );
				}

				//Set Data Pins as Output, For Scanning and operating Output, All Data PINS must be output
				setDataPinsDirection( GPIO_MODE_OUTPUT );
				delayMicroseconds( 500 );
				resetInputOutput( );
				delayMicroseconds( 500 );

				//Turn OFF all continuity related LEDs
				operateStatusLeds( CONTINUITY_INOUT_STATUS_LED_P1, STATUS_LED_OFF );
				operateStatusLeds( CONTI_TEST_RESULT_STATUS_LED_P3, STATUS_LED_OFF );

				//Enable Group 2 IC and Give some delay of at least 150 uS ( This is after Discussion and Testing Observation )
				#if !defined( CHANNEL_DELAY )
				enableGroup2IC( );
				#endif
				setApplReqToShareScannedValueStatus( false );

				memset( &scanInstant, '\0', sizeof( scanningInstant ) );

				//Move immediately after Setting current Continuity State
				//For Safer side, kept the delay as 3ms so that time can capture the event
				stopContinuityTimer( );
			}
		}
		break; //case CONTINUITY_PROCESS_IDLE:

		case CONTINITY_OPERATE_OUTPUT_INIT:
		{
			//As the node is doing Operating Output and scanning operation both we are calculating scanning instants here.
			//For the first time node has to operate the output before scan start clock instant
			scanInstant.operateIOClkInstant = getScanStartClockInstant( getNodeNumber() ) -
					                          continuityProdConfiguration.idlePeriodAfterScanning;
			scanInstant.scanIOClkInstant = getScanStartClockInstant( getNodeNumber() );
			scanProcessStatus.scanningForNode = getNodeNumber();
			scanProcessStatus.scanningForOutput = 0;
			memset( &operateOutputStatus, '\0', sizeof( operateOutputStatus_t ) );

			CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "IO I:%lld->SI:%lld", scanInstant.operateIOClkInstant,
																	      scanInstant.scanIOClkInstant );

			//As Any node has to operate the output within Scan Idle wait period, check if that period is available,
			//return to process Stopped, If sufficient time is not available
			if( scanInstant.operateIOClkInstant < CURRENT_CLOCK_INSTANT_WITH_OFFSET )
			{
				//@ToDo: Send Process Error message back to application to restart the process
				//Or create a new state when any continuity related error occurs
				scanValueStatus[ scanProcessStatus.scanningForNode ].scanningError = NOT_ENOUGH_TIME_TO_OPERATE_OUTPUT;
				CONTINUITY_DBG_ERR( CONTINUITY_DBG_TAG, "Not Enough Time to Operate OP:%lld,%lld",
										scanInstant.operateIOClkInstant, CURRENT_CLOCK_INSTANT_WITH_OFFSET );
				setContinuityProcessState( CONTINUITY_PROCESS_IDLE );
				registerTxMessage( PROCESS_ERROR_MESG, 1, P2P_MESG, TCP_MESSAGE );
				return;
			}
			else
			{
				uint64_t clockDiff = scanInstant.operateIOClkInstant - CURRENT_CLOCK_INSTANT_WITH_OFFSET;
				startContinuityTimer( xTaskGetTickCount(), clockDiff );
				setContinuityProcessState( CONTINUITY_OPERATE_OUTPUT );
				CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "Moving to CONTINUITY_OPERATE_OUTPUT" );
			}

			//Turn ON Continuity Status LED as Node is doing both Operating Output and scanning Inputs
			operateStatusLeds( CONTINUITY_INOUT_STATUS_LED_P1, STATUS_LED_ON );
		}
		break; //case CONTINITY_OPERATE_OUTPUT_INIT:

		case CONTINUITY_IO_SCANNING_INIT:
		{
			//As the node is doing Operating Output and scanning operation both we are calculating scanning instants here.
			scanInstant.operateIOClkInstant = 0;
			scanInstant.scanIOClkInstant = getScanStartClockInstant( scanProcessStatus.scanningForNode );
			CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "SCAN For Node:%d", scanProcessStatus.scanningForNode );
			CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "SCAN START I:%lld", scanInstant.scanIOClkInstant );

			//As Any node has to operate the output within Scan Idle wait period, check if that period is available, return to process Stopped
			//If sufficient time is not available
			if( scanInstant.scanIOClkInstant < CURRENT_CLOCK_INSTANT_WITH_OFFSET )
			{
				//@ToDo: Send Process Error message back to application to restart the process
				//Or create a new state when any continuity related error occurs
				scanValueStatus[ scanProcessStatus.scanningForNode ].scanningError = TIME_FOR_SCANNING_ALREADY_EXPIRED;
				CONTINUITY_DBG_ERR( CONTINUITY_DBG_TAG, "Not Enough Time to Perform continuity operation" );

				setContinuityProcessState( CONTINUITY_PROCESS_IDLE );
				registerTxMessage( PROCESS_ERROR_MESG, 1, P2P_MESG, TCP_MESSAGE );
				return;
			}
			else
			{
				uint64_t clockDiff = scanInstant.scanIOClkInstant - CURRENT_CLOCK_INSTANT_WITH_OFFSET;
				startContinuityTimer( xTaskGetTickCount(), clockDiff );
				setContinuityProcessState( CONTINUITY_IO_SCANNING );
			}

			//Turn OFF Continuity Status LED as Node is just Scanning Inputs only
			operateStatusLeds( CONTINUITY_INOUT_STATUS_LED_P1, STATUS_LED_OFF );
		}
		break; //case CONTINUITY_IO_SCANNING_INIT:

		case CONTINUITY_OPERATE_OUTPUT:
		{
#if defined( CHECK_FOR_EXACT_CLOCK_INSTANT )
			//Operate Output Once we reach at the clock instant
			if( ( CURRENT_CLOCK_INSTANT_WITH_OFFSET == scanInstant.operateIOClkInstant ) ||
			    ( scanInstant.operateIOClkInstant - CURRENT_CLOCK_INSTANT_WITH_OFFSET == 1 ) )
			{
#endif
				if( operateOutputStatus.currentOutput == 0 )
					registerTxMessage( START_OPERATING_OUTPUT_MESG, 1, P2P_MESG, TCP_MESSAGE );

				operateOutputStatus.currentOutput++;
				operateOutputStatus.outputOperatedOn = CURRENT_CLOCK_INSTANT_WITH_OFFSET;
				//CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "\n\n");
				CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "OP %d Ope: %lld, D:%lld",
						operateOutputStatus.currentOutput, CURRENT_CLOCK_INSTANT_WITH_OFFSET,
						( CURRENT_CLOCK_INSTANT_WITH_OFFSET - opDiff) );

				opDiff = CURRENT_CLOCK_INSTANT_WITH_OFFSET;

				operateOutputForContinuityOperation( operateOutputStatus.currentOutput );

				if( operateOutputStatus.currentOutput >= getNoOfIOsAssociatedWithNode( getNodeNumber() ) )
				{
					//Not Scheduled Instant to operate output, This zero instant can be used to decide we are performing
					//scan for last output, No need to move to this state as there are no more outputs
					scanInstant.operateIOClkInstant = 0;
					//CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "Last OP" );
					//if( getNodeNumber() != 1 )
					registerTxMessage( OUTPUT_OPERATING_DONE_MESG, 1, P2P_MESG, TCP_MESSAGE );
				}
				else
				{
					scanInstant.operateIOClkInstant = scanInstant.operateIOClkInstant +
							                          continuityProdConfiguration.totalScanningPeriodForOneOp;
				}

				uint32_t clockDiff = scanInstant.scanIOClkInstant - CURRENT_CLOCK_INSTANT_WITH_OFFSET;
				startContinuityTimer( xTaskGetTickCount(), clockDiff );
				setContinuityProcessState( CONTINUITY_IO_SCANNING );
#if defined( CHECK_FOR_EXACT_CLOCK_INSTANT )
			}
			else //if( scanInstant.operateIOClkInstant > CURRENT_CLOCK_INSTANT_WITH_OFFSET )
			{
				CONTINUITY_DBG_ERR( CONTINUITY_DBG_TAG, "OP Operating Instant missed by %lld,%lld",
										CURRENT_CLOCK_INSTANT_WITH_OFFSET, scanInstant.operateIOClkInstant );
			}
#endif
		}
		break; //case CONTINUITY_OPERATE_OUTPUT:

		case CONTINUITY_IO_SCANNING:
		{
#if defined( CHECK_FOR_EXACT_CLOCK_INSTANT )
			//Scan All Required IO once we reach at the clock instant
			if( ( CURRENT_CLOCK_INSTANT_WITH_OFFSET == scanInstant.scanIOClkInstant ) ||
				( scanInstant.scanIOClkInstant - CURRENT_CLOCK_INSTANT_WITH_OFFSET == 1 ) )
			{
#endif
				uint8_t arrayIndex = getScannedValueArrayIndex( scanProcessStatus.scanningForNode );
				uint64_t totalScanTime = 0;

				scanProcessStatus.scanningForOutput++;
//				CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "I:%d, N:%d,O:%d,IO:%d", arrayIndex,
//															scanProcessStatus.scanningForNode,
//															scanProcessStatus.scanningForOutput,
//															getNoOfIOsAssociatedWithNode( scanProcessStatus.scanningForNode ) );

				CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "OP%d %d Sc:%lld,D:%lld",
						scanProcessStatus.scanningForOutput, arrayIndex, CURRENT_CLOCK_INSTANT_WITH_OFFSET,
						( CURRENT_CLOCK_INSTANT_WITH_OFFSET - scanDiff) );

				scanDiff = CURRENT_CLOCK_INSTANT_WITH_OFFSET;

				//Store the Scan Start time for future debugging and monitoring at application
				if( scanProcessStatus.scanningForOutput == 1 )
				{
					memset( &scanValueStatus[ arrayIndex ], '\0', sizeof( scanValueStatus_t ) );
					scanValueStatus[ arrayIndex ].scanStartedOn = CURRENT_CLOCK_INSTANT_WITH_OFFSET;
					scanValueStatus[ arrayIndex ].scanningDone = 0;
					scanValueStatus[ arrayIndex ].scanPerformedFor = arrayIndex + continuityProdConfiguration.startScanningFromNode;
				}

				scanValueStatus[ arrayIndex ].totalOutputsScanned++;
				totalScanTime = CURRENT_CLOCK_INSTANT_WITH_OFFSET;
				scanNodeAssociatedIOs( arrayIndex, scanProcessStatus.scanningForOutput );
				totalScanTime = CURRENT_CLOCK_INSTANT_WITH_OFFSET - totalScanTime;

				//Save Total Scan Period for one Output
				if( totalScanTime > scanValueStatus[ arrayIndex ].totalScanPeriod )
					scanValueStatus[ arrayIndex ].totalScanPeriod = totalScanTime;

				//Calculate new next scan instant
				scanInstant.scanIOClkInstant = scanInstant.scanIOClkInstant + continuityProdConfiguration.totalScanningPeriodForOneOp;

				//If All Outputs are scanned and scanned values are collected, we can move to next node if available for scanning
				if( scanProcessStatus.scanningForNode == getNodeNumber() )
				{
					if( scanProcessStatus.scanningForOutput == getNoOfIOsAssociatedWithNode( scanProcessStatus.scanningForNode ) )
					{
						scanValueStatus[ arrayIndex ].scanCompletedOn = CURRENT_CLOCK_INSTANT_WITH_OFFSET;
						scanValueStatus[ arrayIndex ].scanningDone = 1;
						scanInstant.scanIOClkInstant = 0;
						setContinuityProcessState( CONTINUITY_MOVE_TO_NEXT_NODE );
						stopContinuityTimer( );
						CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "MOVE_TO_NEXT_NODE: 2" );
						return;
					}
					else
					{
						uint64_t clockDiff = scanInstant.operateIOClkInstant - CURRENT_CLOCK_INSTANT_WITH_OFFSET;
						startContinuityTimer( xTaskGetTickCount(), clockDiff );
						setContinuityProcessState( CONTINUITY_OPERATE_OUTPUT );
						return;
					}
				}
				else
				{
					if( scanProcessStatus.scanningForOutput == getNoOfIOsAssociatedWithNode( scanProcessStatus.scanningForNode ) )
					{
						scanValueStatus[ arrayIndex ].scanCompletedOn = CURRENT_CLOCK_INSTANT_WITH_OFFSET;
						scanValueStatus[ arrayIndex ].scanningDone = 1;
						scanInstant.scanIOClkInstant = 0;
						setContinuityProcessState( CONTINUITY_MOVE_TO_NEXT_NODE );
						stopContinuityTimer( );
						CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "MOVE_TO_NEXT_NODE: 1" );
						return;
					}
				}

				//Perform next scan operation
				uint64_t clockDiff = scanInstant.scanIOClkInstant - CURRENT_CLOCK_INSTANT_WITH_OFFSET;
				startContinuityTimer( xTaskGetTickCount(), clockDiff );
#if defined( CHECK_FOR_EXACT_CLOCK_INSTANT )
			}
			else //if( scanInstant.operateIOClkInstant > CURRENT_CLOCK_INSTANT_WITH_OFFSET )
			{
				CONTINUITY_DBG_ERR( CONTINUITY_DBG_TAG, "Scan Instant missed by %lld,%lld",
										CURRENT_CLOCK_INSTANT_WITH_OFFSET, scanInstant.operateIOClkInstant );
			}
#endif
		}
		break; //case CONTINUITY_IO_SCANNING:

		case CONTINUITY_MOVE_TO_NEXT_NODE:
		{
			uint8_t lastNodeToBeScanned = continuityProdConfiguration.startScanningFromNode +
										  continuityProdConfiguration.noOfNodesToBeScanned - 1;

			if( scanProcessStatus.scanningForNode == lastNodeToBeScanned )
			{
				//Reset Message Reporting Status except Retransmission count
				#if !defined( CHANNEL_DELAY )
				disableGroup2IC( );
				#endif
				resetSelfDetectionForSimilarPins( );
				resetReportingStatusForAllScanVal( );
				setContinuityProcessState( CONTINUITY_MESG_TO_APP_MONITORING );
				CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "MESG_TO_APP_MONITORING: 1" );
			}
			else
			{
				scanProcessStatus.scanningForNode++;
				scanProcessStatus.scanningForOutput = 0;
				resetInputOutput( );
				#if !defined( CHANNEL_DELAY )
				enableGroup2IC( );
				#endif
				if( scanProcessStatus.scanningForNode == getNodeNumber() )
				{
					setContinuityProcessState( CONTINITY_OPERATE_OUTPUT_INIT );
					CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "OPERATE_OUTPUT_INIT: 1" );
				}
				else
				{
					setContinuityProcessState( CONTINUITY_IO_SCANNING_INIT );
					CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "IO_SCANNING_INIT: 1" );
				}
			}
		}
		break; //case CONTINUITY_MOVE_TO_NEXT_NODE:

		case CONTINUITY_MESG_TO_APP_MONITORING:
		{
			if( ( ( hasApplReqToShareScannedValue() == true ) && ( getNoOfScannedValueNotReportedToApp() > 0 ) ) &&
				( ( !checkIfAnySimilarMesgRegistered( CONTINUITY_SCANNED_NON_ZERO_DETECTION_MESG ) ) &&
				  ( !checkIfAnySimilarMesgRegistered( CONTINUITY_SCANNED_ZERO_DETECTION_MESG ) ) ) )
			{
				CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "A:%d, B-%d, C-%d, D-%d",
						hasApplReqToShareScannedValue() ? 1 : 0, getNoOfScannedValueNotReportedToApp(),
						checkIfAnySimilarMesgRegistered( CONTINUITY_SCANNED_NON_ZERO_DETECTION_MESG ),
						checkIfAnySimilarMesgRegistered( CONTINUITY_SCANNED_ZERO_DETECTION_MESG ) );

				scanValReportingSts.currentlySending = 0;

				//Register Zero Detection Reporting only if Zero Detection Reporting Message Not sent and
				//Have zero detection node count
				if( scanValReportingSts.zeroDetectionNodeCount != 0 &&
					scanValReportingSts.zeroDetectionValueSent == SCANNED_VALUE_NOT_REPORTED_TO_APP )
				{
					CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "ZERO_DCTN_MESG Reg.." );
					startContinuityTimer( xTaskGetTickCount(), 20 );
					registerTxMessage( CONTINUITY_SCANNED_ZERO_DETECTION_MESG, 5, P2P_MESG, TCP_MESSAGE );
				}
				else if( scanValReportingSts.nonZeroDetectionNodeCount != 0 )
				{
					if( getNoOfScannedValueNotReportedToApp() > 0 )
					{
						CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "NON_ZERO_DCTN_MESG Reg..: %d, %d",
								scanValReportingSts.nonZeroDetectionNodeCount, getNoOfScannedValueNotReportedToApp( ) );

						//Wait for the previous message to be registered to application
						if( !checkIfAnySimilarMesgRegistered( CONTINUITY_SCANNED_NON_ZERO_DETECTION_MESG ) )
						{
							registerTxMessage( CONTINUITY_SCANNED_NON_ZERO_DETECTION_MESG, 20, P2P_MESG, TCP_MESSAGE );
							//CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "Scan Val Registered" );
							startContinuityTimer( xTaskGetTickCount(), 20 );
						}
					}
				}
			}

			if( hasAckReceivedForAllNodes( ) == true )
			{
				stopContinuityTimer( );
				setContinuityProcessState( CONTINUITY_PROCESS_IDLE );
				CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "All Ack Rcvd" );
			}
		}
		break; //case CONTINUITY_MESG_TO_APP_MONITORING:
	}
}

static uint8_t getNoOfScannedValueNotReportedToApp( void )
{
	uint8_t count = 0;
	for( uint8_t ite = 0; ite < continuityProdConfiguration.noOfNodesToBeScanned; ite++ )
	{
		if( scanValueStatus[ ite ].scannedValueSentToApp == SCANNED_VALUE_NOT_REPORTED_TO_APP )
			count++;
	}
	return count;
}

static uint8_t getNodeCountBasedOnConnection( uint8_t _zeroOrNonZero )
{
	uint8_t count = 0;
	for( uint8_t ite = 0; ite < continuityProdConfiguration.noOfNodesToBeScanned; ite++ )
	{
		if( scanValueStatus[ ite ].noOfConnectionDetectedForNode == 0 && _zeroOrNonZero == ZERO_DETECTION_NODE_COUNT )
			count++;
		else if( scanValueStatus[ ite ].noOfConnectionDetectedForNode != 0 && _zeroOrNonZero == NON_ZERO_DETECTION_NODE_COUNT )
			count++;
	}
	return count;
}

static void resetReportingStatusForAllScanVal( void )
{
	for( uint8_t ite = 0; ite < MAX_SCAN_VALUE_STATIC_BUFFER_FOR_NODES; ite++ )
	{
		scanValueStatus[ ite ].scannedValueSentToApp = 0;
		scanValueStatus[ ite ].scanedValueReceivedByApp = 0;
		scanValueStatus[ ite ].scannedValueSentOn = 0;
		if( scanValReportingSts.reSentRequired != 1 )
			scanValueStatus[ ite ].noOfTimesScanValSentToAPp = 0;
	}

	CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "Reset Reporting Sts" );
	//scanValReportingSts.applrequestedForScannedVal = 0;
	scanValReportingSts.currentlySending = 0;
	scanValReportingSts.reSentRequired = 0;
	scanValReportingSts.nonZeroDetectionValueSent = 0;
	scanValReportingSts.zeroDetectionValueSent = 0;
	scanValReportingSts.zeroDetectionNodeCount = getNodeCountBasedOnConnection( ZERO_DETECTION_NODE_COUNT );
	scanValReportingSts.nonZeroDetectionNodeCount = getNodeCountBasedOnConnection( NON_ZERO_DETECTION_NODE_COUNT );
}

bool hasApplReqToShareScannedValue( void )
{
	return ( scanValReportingSts.applrequestedForScannedVal == 1 ) ? true : false;
}

void setApplReqToShareScannedValueStatus( uint8_t _status )
{
	scanValReportingSts.applrequestedForScannedVal = _status;
}

void intToBinaryCharArray( uint32_t value, char *binaryArray, size_t arraySize )
{
    for (int i = 31; i >= 0; i--) {
        binaryArray[ 31 - i ] = '0' + ( ( value >> i ) & 1 );
    }

    // Null-terminate the string
    binaryArray[32] = '\0';
}

void operateOutputForContinuityOperation( const uint8_t _outputNumber )
{
    uint8_t group1ChannelBits = 0;     //BS1, BS2, BS3
    uint8_t groupICSelectionBits = 0;  //BS7, BS8, BS9

    gpio_set_level( GROUP_1_IC_ENABLE_PIN, GPIO_PIN_HIGH );
    gpio_set_level( BIPOLAR1_PIN, GPIO_PIN_HIGH );

    /* Select Channel for Group 1*/
    group1ChannelBits = ( _outputNumber - 1 ) % TOTAL_OP_UNDER_1_4051;
    groupICSelectionBits = ( _outputNumber - 1 ) / TOTAL_OP_UNDER_1_4051;

    BIT_WRITE( BS1ToBS12_P1ToP5_16BitData, 0, BIT_CHECK( group1ChannelBits, 0 ) );
    BIT_WRITE( BS1ToBS12_P1ToP5_16BitData, 1, BIT_CHECK( group1ChannelBits, 1 ) );
    BIT_WRITE( BS1ToBS12_P1ToP5_16BitData, 2, BIT_CHECK( group1ChannelBits, 2 ) );

    BIT_WRITE( BS1ToBS12_P1ToP5_16BitData, 6, BIT_CHECK( groupICSelectionBits, 0 ) );
    BIT_WRITE( BS1ToBS12_P1ToP5_16BitData, 7, BIT_CHECK( groupICSelectionBits, 1 ) );
    BIT_WRITE( BS1ToBS12_P1ToP5_16BitData, 8, BIT_CHECK( groupICSelectionBits, 2 ) );

//    char binaryArray[ 33 ];
//    CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "O:%d,0x%x,0x%x ",_outputNumber, group1ChannelBits, groupICSelectionBits );
//    intToBinaryCharArray( BS1ToBS12_P1ToP5_16BitData, binaryArray, sizeof( binaryArray ) );
//    CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "BS1-12:%s", binaryArray );

//    CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "[ %02d ] : L2 : ", _outputNumber );
//    uint8_t iterator= 0;
//    for( iterator = 16; iterator > 0; iterator-- )
//    {
//        if( iterator == 8 )
//            CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, ", L1: " );
//            Serial.print( ", L1: " );
//        CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "%d ", BIT_CHECK( BS1ToBS12_P1ToP5_16BitData, iterator - 1 ) );
//    }

    /* Set D1 to D8 for latch 1 */
    for( uint8_t ite_1 = 0; ite_1 < 8; ite_1++ )
    	gpio_set_level( DATA_PINS[ ite_1 ], BIT_CHECK( BS1ToBS12_P1ToP5_16BitData, ite_1 ) );

    /* Trigger Latch 1 */
    gpio_set_level( LATCH_ENABLE_PINS[ LATCH_ENABLE_PIN_1 ], GPIO_PIN_HIGH );
    delayMicroseconds( 15 );
    gpio_set_level( LATCH_ENABLE_PINS[ LATCH_ENABLE_PIN_1 ], GPIO_PIN_LOW );
    delayMicroseconds( 15 );

    for( uint8_t ite_2 = 8; ite_2 < 16; ite_2++ )
    	gpio_set_level( DATA_PINS[ ite_2 - 8 ], BIT_CHECK( BS1ToBS12_P1ToP5_16BitData, ite_2 ) );

    /* Trigger Latch 1 */
    gpio_set_level( LATCH_ENABLE_PINS[ LATCH_ENABLE_PIN_2 ], GPIO_PIN_HIGH );
    delayMicroseconds( 15 );
    gpio_set_level( LATCH_ENABLE_PINS[ LATCH_ENABLE_PIN_2 ], GPIO_PIN_LOW );
    delayMicroseconds( 15 );

    gpio_set_level( GROUP_1_IC_ENABLE_PIN, GPIO_PIN_LOW );
    gpio_set_level( BIPOLAR1_PIN, GPIO_PIN_LOW );
    delayMicroseconds( 15 );
}

bool isContinuityScanningInProgress( void )
{
	return 1;
}

continuityIOOperatingState getContinuityProcessState( void )
{
	return continuityProcessState;
}

static void setContinuityProcessState( continuityIOOperatingState _state )
{
	continuityProcessState = _state;
}

uint8_t getNoOfIOsAssociatedWithNode( uint8_t _node )
{
	if( _node <= continuityProdConfiguration.totalProductNodes )
		return continuityProdConfiguration.IOassociatedWithNode[ _node - 1 ];
	return 0;
}

static void scanNodeAssociatedIOs( const uint8_t _nodeArrayIndex, const uint8_t _outpuNumber )
{
    uint8_t totalIO = getNoOfIOsAssociatedWithNode( getNodeNumber() );
    uint8_t index = _outpuNumber - 1;

    scanValueStatus[ _nodeArrayIndex ].scannedValues[ index ].noOfConnectionOnPIN = 0;

    uint8_t bitValue = 0;
    for( uint8_t input = 1; input <= 64; input++ )
    {
        if( input <= totalIO )
        {
        	bitValue = performScanningOnIO( input );
            if( input <= 32 )
            	BIT_WRITE( scanValueStatus[ _nodeArrayIndex ].scannedValues[ index ].value_1, ( input - 1 ), bitValue );
            else if( input > 32 && input <= 64 )
            	BIT_WRITE( scanValueStatus[ _nodeArrayIndex ].scannedValues[ index ].value_2, ( input - 33 ), bitValue );

            if( bitValue )
            {
            	scanValueStatus[ _nodeArrayIndex ].scannedValues[ index ].noOfConnectionOnPIN++;
            	scanValueStatus[ _nodeArrayIndex ].noOfConnectionDetectedForNode++;
            	//CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "I:%d", input );
            }
        }
        else
        {
            if( input <= 32 )
            	BIT_WRITE( scanValueStatus[ _nodeArrayIndex ].scannedValues[ index ].value_1, ( input - 1 ), 0 );
            else if( input > 32 && input <= 64 )
            	BIT_WRITE( scanValueStatus[ _nodeArrayIndex ].scannedValues[ index ].value_2, ( input - 33 ), 0 );
        }
    }
    CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "P2:%d,%d,%d,%ld,%ld",
    									  _nodeArrayIndex, index,
    									  scanValueStatus[ _nodeArrayIndex ].noOfConnectionDetectedForNode,
										  scanValueStatus[ _nodeArrayIndex ].scannedValues[ index ].value_1,
										  scanValueStatus[ _nodeArrayIndex ].scannedValues[ index ].value_2 );
}

static uint8_t performScanningOnIO( uint8_t _IONumber )
{
    uint8_t group1ChannelBits = 0;     //BS1, BS2, BS3
    uint8_t groupICSelectionBits = 0;  //BS7, BS8, BS9

    #if defined( CHANNEL_DELAY )
    gpio_set_level( GROUP_2_IC_ENABLE_PIN, GPIO_PIN_HIGH );
    #endif

    /* Select Channel for Group 1*/
    group1ChannelBits = ( _IONumber - 1 ) % TOTAL_OP_UNDER_1_4051;
    groupICSelectionBits = ( _IONumber - 1 ) / TOTAL_OP_UNDER_1_4051;

    BIT_WRITE( BS1ToBS12_P1ToP5_16BitData, 3, BIT_CHECK( group1ChannelBits, 0 ) );
    BIT_WRITE( BS1ToBS12_P1ToP5_16BitData, 4, BIT_CHECK( group1ChannelBits, 1 ) );
    BIT_WRITE( BS1ToBS12_P1ToP5_16BitData, 5, BIT_CHECK( group1ChannelBits, 2 ) );

    BIT_WRITE( BS1ToBS12_P1ToP5_16BitData, 9, BIT_CHECK( groupICSelectionBits, 0 ) );
    BIT_WRITE( BS1ToBS12_P1ToP5_16BitData, 10, BIT_CHECK( groupICSelectionBits, 1 ) );
    BIT_WRITE( BS1ToBS12_P1ToP5_16BitData, 11, BIT_CHECK( groupICSelectionBits, 2 ) );

    //char binaryArray[ 33 ];//, binArr_1[ 33 ], binArr_2[ 33 ];
//    intToBinaryCharArray( group1ChannelBits, binArr_1, sizeof( binArr_1 ) );
//    intToBinaryCharArray( groupICSelectionBits, binArr_2, sizeof( binArr_2 ) );
//	CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "IO:%d, 0x%x, 0x%x, %s, %s, %d, %d, %d",_IONumber, group1ChannelBits, groupICSelectionBits,
//			binArr_1, binArr_2, BIT_CHECK( group1ChannelBits, 0 ), BIT_CHECK( group1ChannelBits, 1 ), BIT_CHECK( group1ChannelBits, 2 ) );
	//intToBinaryCharArray( BS1ToBS12_P1ToP5_16BitData, binaryArray, sizeof( binaryArray ) );
	//CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "B1-12: %d, %s", BS1ToBS12_P1ToP5_16BitData, binaryArray );
	//CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "B1-12:%d", BS1ToBS12_P1ToP5_16BitData );

    /* Set D1 to D8 for latch 1 */
    for( uint8_t ite_1 = 0; ite_1 < 8; ite_1++ )
    	gpio_set_level( DATA_PINS[ ite_1 ], BIT_CHECK( BS1ToBS12_P1ToP5_16BitData, ite_1 ) );

    /* Trigger Latch 1 */
    gpio_set_level( LATCH_ENABLE_PINS[ LATCH_ENABLE_PIN_1 ], GPIO_PIN_HIGH );
    delayMicroseconds( 15 );
    gpio_set_level( LATCH_ENABLE_PINS[ LATCH_ENABLE_PIN_1 ], GPIO_PIN_LOW );
    delayMicroseconds( 15 );

    for( uint8_t ite_2 = 8; ite_2 < 16; ite_2++ )
    	gpio_set_level( DATA_PINS[ ite_2 - 8 ], BIT_CHECK( BS1ToBS12_P1ToP5_16BitData, ite_2 ) );

    /* Trigger Latch 1 */
    gpio_set_level( LATCH_ENABLE_PINS[ LATCH_ENABLE_PIN_2 ], GPIO_PIN_HIGH );
    delayMicroseconds( 15 );
    gpio_set_level( LATCH_ENABLE_PINS[ LATCH_ENABLE_PIN_2 ], GPIO_PIN_LOW );
    delayMicroseconds( 15 );

  #if defined( CHANNEL_DELAY )
    gpio_set_level( GROUP_2_IC_ENABLE_PIN, GPIO_PIN_LOW );
    delayMicroseconds( 25 );
  #else
    delayMicroseconds( 50 );
  #endif

#if !defined( DIGITAL_AVERAGING )
    return ( uint8_t )gpio_get_level( BIPOLAR2_PIN );
#else
    uint8_t lowCount = 0;
    for( uint8_t iterator = 0; iterator < DIGITAL_AVERAGING_COUNT; iterator++ )
    {
    	if( gpio_get_level( BIPOLAR2_PIN ) == GPIO_PIN_LOW )
    		lowCount++;
    	delayMicroseconds( SCAN_AVERAGE_DELAY );
    }

    //On Detection We receive low but we save inverted logic while saving scan status
    if ( lowCount > ( DIGITAL_AVERAGING_COUNT / 2 ) )
    	return 1;
    else
    	return 0;
#endif
}

static uint64_t getScanStartClockInstant( uint8_t _nodeNumber )
{
	if( _nodeNumber >= continuityProdConfiguration.startScanningFromNode )
	{
		uint8_t nodeIndex = _nodeNumber - continuityProdConfiguration.startScanningFromNode;
		return continuityProdConfiguration.startScanningOn[ nodeIndex ];
	}
	return 0;
}

void prepareProcessErrorMesg( char* _mesg )
{
	cJSON *rootJSNObj = cJSON_CreateObject();

	cJSON_AddNumberToObject( rootJSNObj, JSN_MESSAGE_CODE_STR, SCAN_ERROR_STATUS_CMD );
	cJSON_AddNumberToObject( rootJSNObj, JSN_NODE_NUMBER_STR, NODE_NUMBER );

	for( uint8_t ite = 0; ite < continuityProdConfiguration.noOfNodesToBeScanned; ite++ )
	{
		if( scanValueStatus[ ite ].scanningError != NO_ERROR )
		{
			cJSON_AddNumberToObject( rootJSNObj, JSN_SACN_ERROR_STS_STR, scanValueStatus[ ite ].scanningError );
			cJSON_AddNumberToObject( rootJSNObj, JSN_SCAN_ERROR_HAPPENED_FOR_STR, continuityProdConfiguration.startScanningFromNode + ite );
		}
	}

	char* jsonStr = cJSON_Print( rootJSNObj ); // Allocate memory for JSON string
	if( jsonStr != NULL )
	{
		strcpy( _mesg, jsonStr ); 	// Copy JSON string to _mesg
		free( jsonStr ); 			// Free allocated memory
	}

	//WIFI_DBG_LOG( WIFI_DBG_TAG, "Scan Err:%d,%s",strlen( cJSON_Print( rootJSNObj ) ), _mesg );
	cJSON_Delete( rootJSNObj );
	clearRegisteredMesg( PROCESS_ERROR_MESG );
}

void prepareOperateOutputStartEventMesg( char* _mesg )
{
	cJSON *rootJSNObj = cJSON_CreateObject();

	cJSON_AddNumberToObject( rootJSNObj, JSN_MESSAGE_CODE_STR, OPERATE_OUTPUT_CMD );
	cJSON_AddNumberToObject( rootJSNObj, JSN_NODE_NUMBER_STR, NODE_NUMBER );
	cJSON_AddNumberToObject( rootJSNObj, JSN_IO_ASSOCIATED_WITH_NODE_STR, getNoOfIOsAssociatedWithNode( NODE_NUMBER ) );

	if( operateOutputStatus.currentOutput == 1 )
		cJSON_AddNumberToObject( rootJSNObj, JSN_OUTPUT_OPERATED_ON_STR, ( double )operateOutputStatus.outputOperatedOn );
	else
		cJSON_AddNumberToObject( rootJSNObj, JSN_OUTPUT_OPERATED_ON_STR, 0 );

	cJSON_AddNumberToObject( rootJSNObj, JSN_NATIVE_CLOCK_WITH_OFFSET_STR, CURRENT_CLOCK_INSTANT_WITH_OFFSET );

	char* jsonStr = cJSON_Print( rootJSNObj ); // Allocate memory for JSON string
	if( jsonStr != NULL )
	{
		strcpy( _mesg, jsonStr ); 	// Copy JSON string to _mesg
		free( jsonStr ); 			// Free allocated memory
	}

	//WIFI_DBG_LOG( WIFI_DBG_TAG, "Tx-Proc Start Time Msg:%d,%s",strlen( cJSON_Print( rootJSNObj ) ), _mesg );
	cJSON_Delete( rootJSNObj );
	clearRegisteredMesg( START_OPERATING_OUTPUT_MESG );
}

void prepareOperateOutputDoneEventMesg( char* _mesg )
{
	cJSON *rootJSNObj = cJSON_CreateObject();

	cJSON_AddNumberToObject( rootJSNObj, JSN_MESSAGE_CODE_STR, OPERATE_OUTPUT_DONE_CMD );
	cJSON_AddNumberToObject( rootJSNObj, JSN_NODE_NUMBER_STR, NODE_NUMBER );
	cJSON_AddNumberToObject( rootJSNObj, JSN_IO_ASSOCIATED_WITH_NODE_STR, getNoOfIOsAssociatedWithNode( NODE_NUMBER ) );

	if( operateOutputStatus.currentOutput == getNoOfIOsAssociatedWithNode( NODE_NUMBER ) )
		cJSON_AddNumberToObject( rootJSNObj, JSN_OUTPUT_OPERATED_ON_STR, ( double )operateOutputStatus.outputOperatedOn );
	else
		cJSON_AddNumberToObject( rootJSNObj, JSN_OUTPUT_OPERATED_ON_STR, 0 );

	cJSON_AddNumberToObject( rootJSNObj, JSN_NATIVE_CLOCK_WITH_OFFSET_STR, CURRENT_CLOCK_INSTANT_WITH_OFFSET );

	char* jsonStr = cJSON_Print( rootJSNObj ); // Allocate memory for JSON string
	if( jsonStr != NULL )
	{
		strcpy( _mesg, jsonStr ); 	// Copy JSON string to _mesg
		free( jsonStr ); 			// Free allocated memory
	}
	//WIFI_DBG_LOG( WIFI_DBG_TAG, "UDP Tx-Process Start Time Mesg:%d,%s",strlen( cJSON_Print( rootJSNObj ) ), _mesg );
	cJSON_Delete( rootJSNObj );
	clearRegisteredMesg( OUTPUT_OPERATING_DONE_MESG );
}

static uint8_t getScannedValueArrayIndex( uint8_t _node )
{
	if( _node >= continuityProdConfiguration.startScanningFromNode )
	{
		return _node - continuityProdConfiguration.startScanningFromNode;
	}
	return 0;
}

void prepareScannedValueMesgForNextPossibleNode( char* _mesg )
{
	bool isAnyMesgPending = false;

	//If any scanned value for any node not sent register the message again
	uint8_t index = 0;
	for( uint8_t ite = 0; ite < continuityProdConfiguration.noOfNodesToBeScanned; ite++ )
	{
		if( scanValueStatus[ ite ].scannedValueSentToApp == 0 &&
			scanValueStatus[ ite ].noOfConnectionDetectedForNode != 0 )
		{
			index = ite;
			isAnyMesgPending = true;
			break;
		}
	}

	if( isAnyMesgPending )
	{
		CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "Sending For Node:%d, ConnCnt:%d", index,
							  scanValueStatus[ index ].noOfConnectionDetectedForNode );

		if( prepareScannedValueMessage( _mesg, index ) == 1 )
		{
			//scanValReportingSts.currentlySending = 0;
			scanValReportingSts.scannedValueSentOn = CURRENT_CLOCK_INSTANT_WITH_OFFSET;
			scanValueStatus[ index ].scannedValueSentToApp = 1;
			scanValueStatus[ index ].noOfTimesScanValSentToAPp++;
			clearRegisteredMesg( CONTINUITY_SCANNED_NON_ZERO_DETECTION_MESG );
		}
	}
}

void prepareZeroDetectionMessage( char* _mesg )
{
	bool zeroDetectionFound = false;
	uint8_t noNoNodesHaveZeroDetection = 0;
	cJSON *rootJSNObj = cJSON_CreateObject();
	cJSON *scannedOpJsnArray = cJSON_CreateArray();
	cJSON *scannedForNodeJsnArray = cJSON_CreateArray();
	cJSON *scanStartedOnJsnArray = cJSON_CreateArray();
	cJSON *scanCompletedOnJsnArray = cJSON_CreateArray();

	for( uint8_t ite = 0; ite < continuityProdConfiguration.noOfNodesToBeScanned; ite++ )
	{
		if( scanValueStatus[ ite ].noOfConnectionDetectedForNode == 0 )
		{
			zeroDetectionFound = true;
			cJSON_AddItemToArray( scannedOpJsnArray, cJSON_CreateNumber( scanValueStatus[ ite ].totalOutputsScanned ) );
			cJSON_AddItemToArray( scannedForNodeJsnArray, cJSON_CreateNumber( scanValueStatus[ ite ].scanPerformedFor ) );

			cJSON_AddItemToArray( scanStartedOnJsnArray, cJSON_CreateNumber( ( double )scanValueStatus[ ite ].scanStartedOn ) );
			cJSON_AddItemToArray( scanCompletedOnJsnArray, cJSON_CreateNumber( ( double )scanValueStatus[ ite ].scanCompletedOn ) );
			scanValueStatus[ ite ].scannedValueSentToApp = 1;
			noNoNodesHaveZeroDetection++;
		}
	}

	if( zeroDetectionFound == true )
	{
		cJSON_AddNumberToObject( rootJSNObj, JSN_MESSAGE_CODE_STR, SHARE_SCANNED_INPUT_VALUES_CMD );
		cJSON_AddNumberToObject( rootJSNObj, JSN_NODE_NUMBER_STR,  getNodeNumber() );
		cJSON_AddNumberToObject( rootJSNObj, JSN_ZERO_DETECTION_NODES_STR, 1 );
		cJSON_AddNumberToObject( rootJSNObj, JSN_ZERO_DETAION_NODE_COUNT_STR, scanValReportingSts.zeroDetectionNodeCount );
		cJSON_AddNumberToObject( rootJSNObj, JSN_NONZERO_DETAION_NODE_COUNT_STR, scanValReportingSts.nonZeroDetectionNodeCount );

		cJSON_AddItemToObject( rootJSNObj, JSN_SCANNED_FOR_NO_OF_OUTPUTS_STR, scannedOpJsnArray );
		cJSON_AddItemToObject( rootJSNObj, JSN_SCANNED_FOR_NODE_STR, scannedForNodeJsnArray );
		cJSON_AddItemToObject( rootJSNObj, JSN_SCAN_STARTED_ON_STR, scanStartedOnJsnArray );
		cJSON_AddItemToObject( rootJSNObj, JSN_SCAN_COMPLETED_ON_STR, scanCompletedOnJsnArray );

		char* jsonStr = cJSON_Print( rootJSNObj ); // Allocate memory for JSON string
		if( jsonStr != NULL )
		{
			strcpy( _mesg, jsonStr ); 	// Copy JSON string to _mesg
			free( jsonStr ); 			// Free allocated memory
		}
	}
	scanValReportingSts.zeroDetectionValueSent = SCANNED_VALUE_REPORTED_TO_APP;
	clearRegisteredMesg( CONTINUITY_SCANNED_ZERO_DETECTION_MESG );

	//Check if any node have any single connection detected.
	//If any single connection detected, register the same message again after 5 ms difference.
	if( noNoNodesHaveZeroDetection != continuityProdConfiguration.noOfNodesToBeScanned )
	{
		registerTxMessage( CONTINUITY_SCANNED_NON_ZERO_DETECTION_MESG, 20, P2P_MESG, TCP_MESSAGE );
		CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "NON_ZERO_DETECTION_MESG Reg.." );
	}

	cJSON_Delete( rootJSNObj );
}

static void resetSelfDetectionForSimilarPins( )
{
	uint8_t allSimilarDetected = 1;
	uint32_t w0To31ScannedValues = 0, w32To63ScannedValue = 0;
	uint8_t IOAssoWithSelfNode = getNoOfIOsAssociatedWithNode( getNodeNumber() );
	uint8_t index = 0, input = 0;

	for( index = 0; index < MAX_SCAN_VALUE_STATIC_BUFFER_FOR_NODES; index++ )
		if( scanValueStatus[ index ].scanPerformedFor == getNodeNumber() )
			break;

	CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "SNNI:%d, TConn:%d", index, scanValueStatus[ index ].noOfConnectionDetectedForNode );
	for( input = 0; input < IOAssoWithSelfNode; input++ )
	{
		if( input < 32 )
		{
			w0To31ScannedValues = scanValueStatus[ index ].scannedValues[ input ].value_1;
			if( BIT_CHECK( w0To31ScannedValues, input ) != 1 )
			{
				allSimilarDetected = 0;
				CONTINUITY_DBG_ERR( CONTINUITY_DBG_TAG, "PIN:%d", ( input + 1 ) );
				break;
			}
		}
		else
		{
			w32To63ScannedValue = scanValueStatus[ index ].scannedValues[ input ].value_2;
			if( BIT_CHECK( w32To63ScannedValue, input ) != 1 )
			{
				allSimilarDetected = 0;
				CONTINUITY_DBG_ERR( CONTINUITY_DBG_TAG, "PIN:%d", ( input + 1 ) );
				break;
			}
		}
	}

	if( allSimilarDetected )
	{
		for( input = 0; input < IOAssoWithSelfNode; input++ )
		{
			if( input < 32 )
			{
				BIT_CLEAR( scanValueStatus[ index ].scannedValues[ input ].value_1, input );
				scanValueStatus[ index ].noOfConnectionDetectedForNode--;
			}
			else
			{
				BIT_CLEAR( scanValueStatus[ index ].scannedValues[ input ].value_2, input );
				scanValueStatus[ index ].noOfConnectionDetectedForNode--;
			}
		}
	}
	CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "**SNNI:%d, TConn:%d", index,
			            scanValueStatus[ index ].noOfConnectionDetectedForNode );
}

static uint8_t prepareScannedValueMessage( char _mesg[ ], uint8_t _index )
{
	uint8_t inputNum = 0, totalAddedArrayIndex = 0; //totalAddedArrayIndex Added only For Debug Purpose
	uint8_t selfNodeNumber = getNodeNumber();
	uint8_t IOAssoWithSelfNode = getNoOfIOsAssociatedWithNode( selfNodeNumber );
	uint8_t scannedForNode = _index + continuityProdConfiguration.startScanningFromNode;
	uint32_t w0To31ScannedValues = 0, w32To63ScannedValue = 0;

	uint32_t value = ( ( uint32_t )scannedForNode << 24 ) +
					   ( scanValueStatus[ _index ].totalOutputsScanned << 16 ) +
					   ( scanValueStatus[ _index ].noOfTimesScanValSentToAPp << 8 ) +
					   ( ( ( uint8_t )DIGITAL_INPUT_TYPE << 3 ) | ( 0 ) );
				//scannedForNode		( 8 Bits )[ 24 - 32 Bits ]
				//totalOutputsScanned 	( 8 Bits )[ 16 - 24 Bits ]
				//NoOfTImesSentToApp	( 8 Bits )[ 08 - 16 Bits ]
				//Scanned Value Type	( 8 Bits )[ 00 - 08 Bits ]

	cJSON *rootJSNObj = cJSON_CreateObject();
	cJSON *jsonArr = cJSON_CreateArray();

	if( rootJSNObj == NULL || jsonArr == NULL )
	{
		cJSON_Delete( rootJSNObj );
		cJSON_Delete( jsonArr );
		return 0;
	}

	cJSON_AddNumberToObject( rootJSNObj, JSN_MESSAGE_CODE_STR, SHARE_SCANNED_INPUT_VALUES_CMD );
	cJSON_AddNumberToObject( rootJSNObj, JSN_NODE_NUMBER_STR,  getNodeNumber() );
	cJSON_AddNumberToObject( rootJSNObj, JSN_SCANNED_FOR_NO_OF_OUTPUTS_STR, scanValueStatus[ _index ].totalOutputsScanned );
	cJSON_AddNumberToObject( rootJSNObj, JSN_SCANNED_VALUE_STATUS_STR, value );
	cJSON_AddNumberToObject( rootJSNObj, JSN_SCAN_STARTED_ON_STR, ( double )scanValueStatus[ _index ].scanStartedOn );
	cJSON_AddNumberToObject( rootJSNObj, JSN_SCAN_COMPLETED_ON_STR, ( double )scanValueStatus[ _index ].scanCompletedOn );

	cJSON_AddNumberToObject( rootJSNObj, JSN_ZERO_DETECTION_NODES_STR, 0 );
	cJSON_AddNumberToObject( rootJSNObj, JSN_ZERO_DETAION_NODE_COUNT_STR, scanValReportingSts.zeroDetectionNodeCount );
	cJSON_AddNumberToObject( rootJSNObj, JSN_NONZERO_DETAION_NODE_COUNT_STR, scanValReportingSts.nonZeroDetectionNodeCount );

	if( scanValueStatus[ _index ].noOfTimesScanValSentToAPp == 0 )
	{
		//Set Bit Reset for bit operation
		//CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "IOAssoWithSelfNode:%d", IOAssoWithSelfNode );
		scanValueStatus[ _index ].scannedValueBitSetter = 0;
		for( inputNum = 0; inputNum < IOAssoWithSelfNode; inputNum++ )
		{
			if( inputNum == 32 || inputNum == 64 )
				scanValueStatus[ _index ].scannedValueBitSetter = MAX_32_BIT_VALUE;
			else
			{
				if( inputNum < 32 )
					scanValueStatus[ _index ].scannedValueBitSetter |= ( 1 << inputNum );
				else
					scanValueStatus[ _index ].scannedValueBitSetter |= ( 1 << ( inputNum - 32 ) );
			}
		}
		//CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "BitSet:%ld", scanValueStatus[ _index ].scannedValueBitSetter );
	}

	cJSON_AddNumberToObject( rootJSNObj, JSN_SCANNED_VALUE_BITSETTER_STR, scanValueStatus[ _index ].scannedValueBitSetter );

	CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "TTOSC:%d,NOTSTP:%d", scanValueStatus[ _index ].totalOutputsScanned,
																  scanValueStatus[ _index ].noOfTimesScanValSentToAPp );
	if( scanValueStatus[ _index ].noOfTimesScanValSentToAPp == 0 )
	{
		for( inputNum = 0; inputNum < scanValueStatus[ _index ].totalOutputsScanned; inputNum++ )
		{
			w0To31ScannedValues = scanValueStatus[ _index ].scannedValues[ inputNum ].value_1;
			w32To63ScannedValue = scanValueStatus[ _index ].scannedValues[ inputNum ].value_2;
//				CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "IP:%d, V1:%ld, V2:%ld",
//				                      inputNum, w0To31ScannedValues, w32To63ScannedValue );

			// Total Read Operation = No of WIO of the node doing IO Operation
			if( inputNum < 32 ) //scannedIPStatus->readOperations <= 32
			{
				/* If Total Read Operation <= 32 && Total IO <= 32
				 *    Refer just w0To31ScannedStatus_1
				 * If Total Read Operation <= 32 && Total IO > 32
				 *    Refer w0To31ScannedStatus_1 And w0To31ScannedStatus_2
				*/
				if( IOAssoWithSelfNode <= 32 )
				{
					w0To31ScannedValues &= scanValueStatus[ _index ].scannedValueBitSetter;
					scanValueStatus[ _index ].scannedValues[ inputNum ].value_1 = w0To31ScannedValues;

					//0 = No Any Input Detected
					if( w0To31ScannedValues != 0 )
						scanValueStatus[ _index ].w0To31ScannedStatus_1 |= ( 1 << inputNum );

//						CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "w0-31V: %ld, w0-31S_1: %ld",
//											  w0To31ScannedValues, scanValueStatus[ _index ].w0To31ScannedStatus_1 );
				}
				else
				{
					w32To63ScannedValue &= scanValueStatus[ _index ].scannedValueBitSetter;
					scanValueStatus[ _index ].scannedValues[ inputNum ].value_2 = w32To63ScannedValue;

					//0 = No Any Input Detected
					if( w0To31ScannedValues != 0 )
						scanValueStatus[ _index ].w0To31ScannedStatus_1 |= ( 1 << inputNum );
					if( w32To63ScannedValue != 0 )
						scanValueStatus[ _index ].w32To63ScannedStatus_1 |= ( 1 << inputNum );

					CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "w0-31V: %ld, w0To31S_1: %ld",
										  w0To31ScannedValues, scanValueStatus[ _index ].w0To31ScannedStatus_1 );
					CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "w32-63V: %ld, w32-63S_1: %ld",
										  w32To63ScannedValue, scanValueStatus[ _index ].w32To63ScannedStatus_1 );
				}
			}
			else
			{
				/* If Total Read Operation > 32 && Total IO <= 32
				 *    Refer w0To31ScannedStatus_1 And w32To63ScannedStatus_1
				 * If Total Read Operation > 32 && Total IO > 32
				 *    Refer just w0To31ScannedStatus_1 And w32To63ScannedStatus_1
				 *    Also Refer w0To31ScannedStatus_2 And w32To63ScannedStatus_2
				*/
				if( IOAssoWithSelfNode <= 32 )
				{
					w0To31ScannedValues &= scanValueStatus[ _index ].scannedValueBitSetter;
					scanValueStatus[ _index ].scannedValues[ inputNum ].value_1 = w0To31ScannedValues;

					//0 = No Any Input Detected
					if( w0To31ScannedValues != 0 )
						scanValueStatus[ _index ].w32To63ScannedStatus_1 |= ( 1 << ( inputNum - 32 ) );

					CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "w0-31Val: %ld, w0-31S_1: %ld",
										  w0To31ScannedValues, scanValueStatus[ _index ].w0To31ScannedStatus_1 );
				}
				else
				{
					w32To63ScannedValue &= scanValueStatus[ _index ].scannedValueBitSetter;
					scanValueStatus[ _index ].scannedValues[ inputNum ].value_2 = w32To63ScannedValue;

					//0 = No Any Input Detected
					if( w0To31ScannedValues != 0 )
						scanValueStatus[ _index ].w0To31ScannedStatus_2 |= ( 1 << ( inputNum - 32 ) );
					if( w32To63ScannedValue != 0 )
						scanValueStatus[ _index ].w32To63ScannedStatus_2 |= ( 1 << ( inputNum - 32 ) );

					CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "w0-31V: %ld, w0To31S_2: %ld",
										  w0To31ScannedValues, scanValueStatus[ _index ].w0To31ScannedStatus_2 );
					CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "w32-63V: %ld, w32To63S_2: %ld",
										  w32To63ScannedValue, scanValueStatus[ _index ].w32To63ScannedStatus_2 );
				}
			}
		}
	}

	// First Add Status of the Scanned Values
	cJSON_AddItemToArray( jsonArr, cJSON_CreateNumber( scanValueStatus[ _index ].w0To31ScannedStatus_1 ) );
	cJSON_AddItemToArray( jsonArr, cJSON_CreateNumber( scanValueStatus[ _index ].w0To31ScannedStatus_2 ) );
	cJSON_AddItemToArray( jsonArr, cJSON_CreateNumber( scanValueStatus[ _index ].w32To63ScannedStatus_1 ) );
	cJSON_AddItemToArray( jsonArr, cJSON_CreateNumber( scanValueStatus[ _index ].w32To63ScannedStatus_2 ) );

	for( inputNum = 0; inputNum < scanValueStatus[ _index ].totalOutputsScanned; inputNum++ )
	{
		w0To31ScannedValues = scanValueStatus[ _index ].scannedValues[ inputNum ].value_1;
		w32To63ScannedValue = scanValueStatus[ _index ].scannedValues[ inputNum ].value_2;

		if( inputNum < 32 )
		{
			if( IOAssoWithSelfNode <= 32 )
			{
				//0 = No Any Input Detected
				if( ( scanValueStatus[ _index ].w0To31ScannedStatus_1 & ( 1 << inputNum ) ) &&
					  w0To31ScannedValues != 0 )
				{
					cJSON_AddItemToArray( jsonArr, cJSON_CreateNumber( w0To31ScannedValues) );
					totalAddedArrayIndex++;
					//CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "A1 : %d", inputNum );
				}
			}
			else
			{
				//0 = No Any Input Detected
				if( ( scanValueStatus[ _index ].w0To31ScannedStatus_1 & ( 1 << inputNum ) ) &&
					( w0To31ScannedValues != 0 ) )
				{
					cJSON_AddItemToArray( jsonArr, cJSON_CreateNumber( w0To31ScannedValues ) );
					totalAddedArrayIndex++;
					//CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "A2 : %d", inputNum );
				}

				//0 = No Any Input Detected
				if( ( scanValueStatus[ _index ].w32To63ScannedStatus_1 & ( 1 << inputNum ) ) &&
					( w32To63ScannedValue != 0 ) )
				{
					cJSON_AddItemToArray( jsonArr, cJSON_CreateNumber( w32To63ScannedValue ) );
					totalAddedArrayIndex++;
					//CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "A3 : %d", inputNum );
				}
			}
		}
		else
		{
			if( IOAssoWithSelfNode <= 32 )
			{
				//0 = No Any Input Detected
				if( ( scanValueStatus[ _index ].w32To63ScannedStatus_1 & ( 1 << ( inputNum - 32 ) ) ) &&
					( w0To31ScannedValues != 0 ) )
				{
					cJSON_AddItemToArray( jsonArr, cJSON_CreateNumber( w0To31ScannedValues ) );
					totalAddedArrayIndex++;
					//CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "A4 : %d", inputNum );
				}
			}
			else
			{
				//0 = No Any Input Detected
				if( ( scanValueStatus[ _index ].w0To31ScannedStatus_2 & ( 1 << ( inputNum -32 ) ) ) &&
					( w0To31ScannedValues != 0 ) )
				{
					cJSON_AddItemToArray( jsonArr, cJSON_CreateNumber( w0To31ScannedValues ) );
					totalAddedArrayIndex++;
					//CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "A5 : %d", inputNum );
				}

				//0 = No Any Input Detected
				if( ( scanValueStatus[ _index ].w32To63ScannedStatus_2 & ( 1 << ( inputNum - 32 ) ) ) &&
					( w32To63ScannedValue != 0 ) )
				{
					cJSON_AddItemToArray( jsonArr, cJSON_CreateNumber( w32To63ScannedValue ) );
					totalAddedArrayIndex++;
					//CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "A6 : %d", inputNum );
				}
			}
		}
	}
	cJSON_AddItemToObject( rootJSNObj, JSN_SCANNED_VALUE_STR, jsonArr );

	char* jsonStr = cJSON_Print( rootJSNObj ); // Allocate memory for JSON string
	if( jsonStr != NULL )
	{
		strcpy( _mesg, jsonStr ); 	// Copy JSON string to _mesg
		free( jsonStr ); 			// Free allocated memory
	}

	CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "ML:%d, AI:%d, Heap:%ld\n", strlen( _mesg ),
			            totalAddedArrayIndex, esp_get_free_heap_size() );
	//CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "Data:%s\n", _mesg );

	cJSON_Delete( rootJSNObj );
	return 1;
}

void processScanValueReceivedAckResponse( cJSON *_scanForNodeJsnArrObj, cJSON *_scanValRecAckJsnArrObj )
{
	cJSON *scannedFor = NULL, *ackStatus = NULL;
	bool validNode = false;

	for( uint8_t ite = 0; ite < cJSON_GetArraySize( _scanValRecAckJsnArrObj ); ite++ )
	{
		scannedFor = cJSON_GetArrayItem( _scanForNodeJsnArrObj, ite );
		ackStatus  = cJSON_GetArrayItem( _scanValRecAckJsnArrObj, ite );
		validNode = false;

		for( uint8_t nodeNum = 0; nodeNum < continuityProdConfiguration.noOfNodesToBeScanned; nodeNum++ )
		{
			if( scannedFor->valueint == scanValueStatus[ nodeNum ].scanPerformedFor )
			{
				validNode = true;
				if( ackStatus->valueint == 1 )
				{
					scanValueStatus[ ite ].scanedValueReceivedByApp = 1;
					CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "ACK Rcvd Fr %d",
							            scanValueStatus[ nodeNum ].scanPerformedFor );
				}
				else
				{
					CONTINUITY_DBG_ERR( CONTINUITY_DBG_TAG, "Neg ACK Rcvd For %d",
							            scanValueStatus[ nodeNum ].scanPerformedFor );
				}
			}
		}

		if( validNode == false )
		{
			CONTINUITY_DBG_ERR( CONTINUITY_DBG_TAG, "Ack Rcvd for invalid node" );
		}
	}
}

static bool hasAckReceivedForAllNodes( void )
{
	for( uint8_t nodeNum = 0; nodeNum < continuityProdConfiguration.noOfNodesToBeScanned; nodeNum++ )
	{
		if( scanValueStatus[ nodeNum ].scanedValueReceivedByApp == 0 )
		{
			return false;
		}
	}
	return true;
}

void printProductConfiguration( void )
{
	//Set Node IO for Product
	for( uint8_t node = 0; node < continuityProdConfiguration.totalProductNodes; node++ )
	{
		CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "IO[ %d ] : %d", node, continuityProdConfiguration.IOassociatedWithNode[ node ] );
	}

	CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "totalProductNodes : %d", continuityProdConfiguration.totalProductNodes );
	CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "reportingDelay : %d", continuityProdConfiguration.reportingDelay );
	CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "startScanningFromNode : %d", continuityProdConfiguration.startScanningFromNode );
	CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "noOfNodesToBeScanned : %d", continuityProdConfiguration.noOfNodesToBeScanned );

	CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "allowedScanningPeriod : %d", continuityProdConfiguration.allowedScanningPeriod );
	CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "idlePeriodAfterScanning : %d", continuityProdConfiguration.idlePeriodAfterScanning );
	CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "totalScanningPeriodForOneOp : %d", continuityProdConfiguration.totalScanningPeriodForOneOp );

	for( uint8_t node = 0; node < continuityProdConfiguration.totalProductNodes; node++ )
	{
		CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "startScanningOn[ %d ] : %lld", node, continuityProdConfiguration.startScanningOn[ node ] );
	}
}

void calculateExpectedProcessTime( uint8_t totalNodes, uint8_t *_uintArray )
{
	if( totalNodes == 1 )
	{
		processExpectedStartTime = CURRENT_CLOCK_INSTANT_WITH_OFFSET + 500; //500ms from Current Clock Offset
	}
	else
	{
		processExpectedStartTime = CURRENT_CLOCK_INSTANT_WITH_OFFSET + ( totalNodes * 20 ) + 800;
		//20 ms Delay of sending TCP data from Application to all nodes
		//500 ms Transport delay
	}
}

uint64_t getProcessExpectedStartTime( void )
{
	return processExpectedStartTime;
}

void scanDummyInputs( void )
{
	uint8_t bitValue = 1;
	CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "Scanning IOs" );

	for( uint8_t ite = 1; ite < 5; ite++ )
	{
		operateOutputForContinuityOperation( ite );
		for( uint8_t input = 1; input < 8; input++ )
		{
			bitValue = performScanningOnIO( input );
			//CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "PIN[%d]:%d", input, bitValue );
			if( bitValue )
				CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "PIN Detected OP:%d, IP:%d, %d", ite, input, bitValue );
		}
	}
}

//Todo
//1). Merge Message of Non Detection other connection with less points
//2). Check how can we reduce the time between two TCP Messages.
//3). discard scan connection for the same node to be included into the same non zero detection message
