/*
 * leakTestHandeling.c
 *
 *  Created on: May 21, 2023
 *      Author: pbhum
 */

#include "../incl/leakTestHandeling.h"
#include "../incl/wifiHandleing.h"
#include "../incl/continuity.h"
#include "../incl/commonDeclaration.h"
#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

/////////////////////////////////////////////////////////////////////////////////
/*
 * Global Static variables
 */
/////////////////////////////////////////////////////////////////////////////////
static leakData_t 				leakTestData;
static TaskHandle_t 			leakTestTaskHandler;
//static leakTestMesgStatus_T		leakTestMesgStatus;
/////////////////////////////////////////////////////////////////////////////////
/*
 * Static Functions
 */
/////////////////////////////////////////////////////////////////////////////////
static void leakTestHandelingTask( void* arg );

static void typeAStateMachine( void );
static void typeBStateMachine( void );

static void resetTypeAStateMachine( void );
static void resetTypeBStateMachine( void );

static void startOrStopTypeAStateMachine( bool _state );
static void startOrStopTypeBStateMachine( bool _state );

static void operatePressureOrVaccumPump( enLeakPumpState _state );

static void ( *leakTestSMFunPtr[ ] )( ) = { typeAStateMachine,
                                     	    typeBStateMachine };

//static void ( *resetLeakTestFunPtr[ ] )( ) = { resetTypeAStateMachine,
//                                               resetTypeBStateMachine };

static void ( *initializeLeakTestFuncPtr[ ] )( bool ) = { startOrStopTypeAStateMachine,
                                                          startOrStopTypeBStateMachine };

//Static Function Definition

void resumeOrSuspendLeakTestTask( bool _state )
{
	if( _state )
		initializeLeakTestFuncPtr[ leakTestData.leakTestType - LEAK_TEST_TYPE_A ]( true );
	else
		initializeLeakTestFuncPtr[ leakTestData.leakTestType - LEAK_TEST_TYPE_A ]( false );
}

static void resetTypeAStateMachine( void )
{
    stTypeAConfig *typeA = &leakTestData.testTypeData.typeA;

    typeA->typeASMState = TYPE_A_IDLE_STATE;
    typeA->pressAfterPumpOnTimeOutT1 = 0;
}

static void resetTypeBStateMachine( void )
{
    stTypeBConfig *typeB = &leakTestData.testTypeData.typeB;

    typeB->typeBSMState = TYPE_B_IDLE_STATE;
    typeB->pressAfterPumpOnTimeOutT1 = 0;
    typeB->pressAfterStabilization_P1 = 0;
    typeB->pressAfterTestDelay_P2 = 0;
}

static void startOrStopTypeAStateMachine( bool _state )
{
    stTypeAConfig *typeA = &leakTestData.testTypeData.typeA;

    resetTypeAStateMachine( );
    if( _state )
    {
    	LEAK_DBG_LOG( LEAK_A_DBG_TAG, "Type A Monitoring Start" );
        typeA->typeASMState = TYPE_A_PUMP_ON_STATE;
        leakTestData.testProcessStatus = TEST_RUNNING;
        vTaskResume( leakTestTaskHandler );
    }
    else
    {
    	LEAK_DBG_LOG( LEAK_A_DBG_TAG, "Type A Monitoring Stop" );
        typeA->typeASMState = TYPE_A_IDLE_STATE;
        vTaskSuspend( leakTestTaskHandler );
    }
}

void startOrStopTypeBStateMachine( bool _state )
{
    stTypeBConfig *typeB = &leakTestData.testTypeData.typeB;

    resetTypeBStateMachine( );
    if( _state && typeB->typeBSMState == TYPE_B_PUMP_ON_STATE )
    {
    	LEAK_DBG_LOG( LEAK_B_DBG_TAG, "Type B Monitoring Start" );
        typeB->typeBSMState = TYPE_B_PUMP_ON_STATE;
        leakTestData.testProcessStatus = TEST_RUNNING;
        vTaskResume( leakTestTaskHandler );
    }
    else
    {
    	LEAK_DBG_LOG( LEAK_B_DBG_TAG, "Type B Monitoring Stop" );
        typeB->typeBSMState = TYPE_B_IDLE_STATE;
        vTaskSuspend( leakTestTaskHandler );
    }
}

void stopLeakTestingIfRunning( void )
{
    if( leakTestData.leakTestType != NO_LEAK_TESTING &&
        leakTestData.testProcessStatus == TEST_RUNNING )
    {
    	LEAK_DBG_LOG( LEAK_A_DBG_TAG, "Leak Testing Stopped" );
        setPumpState( LEAK_TEST_PUMP_OFF );
        operateStatusLeds( LEAK_TEST_RESULT_STATUS_LED_P2, STATUS_LED_OFF );

        leakTestData.currAdcCount = 0;
        leakTestData.currPressure = 0;

        leakTestData.testProcessStatus = TEST_NOT_RUNNING;
        leakTestData.leakTestResult = LEAK_TEST_PASS;
        resumeOrSuspendLeakTestTask( false );
    }
}

void leakTestTaskInit( void )
{
	resetLeakTestParams();
	xTaskCreatePinnedToCore( leakTestHandelingTask, "leak_test_handeling", 4096, NULL, 5, &leakTestTaskHandler, 1 );
}

static void leakTestHandelingTask( void* arg )
{
	for(;;)
	{
		if( leakTestData.leakTestType != NO_LEAK_TESTING )
		    leakTestSMFunPtr[ leakTestData.leakTestType - LEAK_TEST_TYPE_A ]( );

		vTaskDelay( 10 / portTICK_PERIOD_MS );
		//LEAK_DBG_LOG( LEAK_A_DBG_TAG, "LEAK Test:%lld", CURRENT_CLOCK_INSTANT_WITH_OFFSET );
	}
}

static void typeAStateMachine( void )
{
    static leakData_t *leakData;
    static stTypeAConfig *typeA = &leakTestData.testTypeData.typeA;
    static sensorCalibValues_t *calib;
    static int pressureToBeAchied = 0;
    static uint64_t processStartedOn = 0, lastReadOn = 0;
    uint64_t currentTime = esp_timer_get_time()/1000;

    switch( typeA->typeASMState )
    {
        case TYPE_A_IDLE_STATE:
        break;

        case TYPE_A_PUMP_ON_STATE:
        {
        	leakData = &leakTestData;
            typeA = &leakData->testTypeData.typeA;
            calib = &leakData->sensorCalibration;
            processStartedOn = esp_timer_get_time()/1000;
            lastReadOn = processStartedOn;

            LEAK_DBG_LOG( LEAK_A_DBG_TAG, "Set Pressure : %d, Offset : %d, Pump On Time : %d\n",
            		                        typeA->setPressureValue, typeA->pressureOffset, typeA->pumpOnTimeT1 );
            LEAK_DBG_LOG( LEAK_A_DBG_TAG, "CA Type : %s, CADCH: %d, CADCL: %d, CPRESSL: %d, CPRESSH: %d\n",
            		                        ( leakData->calibType) ? "Neg":"Pos",
            		                        calib->minAdcCnt, calib->maxAdcCnt,
                                            calib->minPressValue, calib->maxPressValue );

            if( typeA->setPressureValue > 0 )
            {
            	LEAK_DBG_LOG( LEAK_A_DBG_TAG, "Set Pressure is +" );
                pressureToBeAchied = typeA->setPressureValue - typeA->pressureOffset;
            }
            else if( typeA->setPressureValue < 0 )
            {
            	LEAK_DBG_LOG( LEAK_A_DBG_TAG, "Set Pressure is -" );
                pressureToBeAchied = typeA->setPressureValue + typeA->pressureOffset;
            }

            operateStatusLeds( LEAK_TEST_RESULT_STATUS_LED_P2, STATUS_LED_OFF );
            setPumpState( LEAK_TEST_PUMP_ON );
            typeA->typeASMState = TYPE_A_PRESS_MEASUREMENT_STATE;
            LEAK_DBG_LOG( LEAK_A_DBG_TAG, "TYPE_A_PUMP_ON_STATE" );
        }
        break;

        case TYPE_A_PRESS_MEASUREMENT_STATE:
        {
            if( ( currentTime - lastReadOn ) >= leakData->readIntervalInMs )
            {
            	lastReadOn = esp_timer_get_time()/1000;
                //uint16_t currAdc = ( uint16_t )analogRead( ANALOG_PIN ) >> 1; //@TODO:Uncomment the required Line
            	uint16_t currAdc = 0;//@TODO:Delete while uncommenting above line

                if( leakData->currAdcCount > 0 )
                	leakData->currAdcCount = ( leakData->currAdcCount + currAdc ) / 2;
                else
                	leakData->currAdcCount = currAdc;

                if( leakData->currAdcCount > 2048 )
                	leakData->currAdcCount = 2048;

                leakData->currPressure = ( int )map( leakData->currAdcCount,
                		                             calib->minAdcCnt, calib->maxAdcCnt,
                                                     calib->minPressValue, calib->maxPressValue );
//                LEAK_DBG_LOG( LEAK_TYPEA_TAG, "ADC : %d, Press : %d\n", mainStruct->currAdcCount,
//                                                                          mainStruct->currPressure );

                /* Check if set pressure achieved */
                if( typeA->setPressureValue > 0 && leakData->currPressure >= pressureToBeAchied )
                {
                    /* Jump to Pressure Achieved State */
                	LEAK_DBG_LOG( LEAK_A_DBG_TAG, "Positive Pressure Achieved" );
                    typeA->typeASMState = TYPE_A_SET_PRESS_ACHIEVED_STATE;
                    return;
                }
                else if( typeA->setPressureValue < 0 && leakData->currPressure <= pressureToBeAchied )
                {
                    /* Jump to Pressure Achieved State */
                	LEAK_DBG_LOG( LEAK_A_DBG_TAG, "Negative Pressure Achieved" );
                    typeA->typeASMState = TYPE_A_SET_PRESS_ACHIEVED_STATE;
                    return;
                }
            }

            if( ( currentTime - processStartedOn ) >= typeA->pumpOnTimeT1 )
            {
                /* Jump to Pump on Timeout */
                typeA->pressAfterPumpOnTimeOutT1 = leakData->currPressure;
                typeA->typeASMState = TYPE_A_PUMP_ON_TIMEOUT_STATE;
            }
        }
        break;

        case TYPE_A_SET_PRESS_ACHIEVED_STATE:
        {
            setPumpState( LEAK_TEST_PUMP_OFF );

            leakData->leakTestResult = LEAK_TEST_PASS;
            typeA->typeASMState = TYPE_A_RESULT_TO_APP_WAIT_CHECK;
            LEAK_DBG_LOG( LEAK_A_DBG_TAG, "Pressure is achieved -> Pump Turned OFF" );
        }
        break;

        case TYPE_A_PUMP_ON_TIMEOUT_STATE:
        {
            setPumpState( LEAK_TEST_PUMP_OFF );
            operateStatusLeds( LEAK_TEST_RESULT_STATUS_LED_P2, STATUS_LED_ON );
            leakData->leakTestResult = PUMP_ON_TIMEOUT;
            typeA->typeASMState = TYPE_A_RESULT_TO_APP_WAIT_CHECK;
            LEAK_DBG_LOG( LEAK_A_DBG_TAG, "Pump On Timeout has occured -> Pump Turned OFF" );
        }
        break;

        case TYPE_A_RESULT_TO_APP_WAIT_CHECK:
        {
            if( isContinuityScanningInProgress( ) ||
            	getContinuityProcessState() == CONTINUITY_PROCESS_IDLE )
            {
                typeA->typeASMState = TYPE_A_IDLE_STATE;
                leakData->testProcessStatus = TEST_RESULT_NOT_SENT;
                registerTxMessage( LEAK_TEST_RESULT_MESG, NODE_NUMBER * LEAK_RESULT_SHARE_WAIT_PERIOD, P2P_MESG, TCP_MESSAGE );
                LEAK_DBG_LOG( LEAK_A_DBG_TAG, "Flag Raised to share leak test values with APP" );
            }
        }
        break;

        default:
        break;
    }
}

//This state machine is expected to be called at every 1 ms interval
static void typeBStateMachine( void )
{
    static leakData_t *leakData;
    static stTypeBConfig *typeB = &leakTestData.testTypeData.typeB;
    static sensorCalibValues_t *calib;
    //static int pressureToBeAchied = 0;
    static uint64_t processStartedOn = 0, lastReadOn = 0, milliSecond = 0;
    uint64_t currentTime = esp_timer_get_time()/1000;

    switch( typeB->typeBSMState )
    {
        case TYPE_B_IDLE_STATE:
        break;

        case TYPE_B_PUMP_ON_STATE:
        {
        	leakData = &leakTestData;
            typeB = &leakData->testTypeData.typeB;
            calib = &leakData->sensorCalibration;
            processStartedOn = esp_timer_get_time()/1000;
            lastReadOn = processStartedOn;

            LEAK_DBG_LOG( LEAK_B_DBG_TAG, "Type B -> P : %d, T1 : %d, T2 : %d, T3 : %d, D1 : %d, D2 : %d\n",
                            typeB->setPressure_P, typeB->pumpOnTime_T1, typeB->stabilizeTime_T2, typeB->testDelay_T3,
                            typeB->stabilizeTimeDropValue_D1, typeB->testDelayDropValue_D2 );

            setPumpState( LEAK_TEST_PUMP_ON );
            operateStatusLeds( LEAK_TEST_RESULT_STATUS_LED_P2, STATUS_LED_OFF );
            typeB->typeBSMState = TYPE_B_PRESS_MEASUREMENT_STATE;
            leakData->leakTestResult = LEAK_TEST_NOT_PERFORMED_OR_RUNNING;
        }
        break;

        case TYPE_B_PRESS_MEASUREMENT_STATE:
        {
            if( ( currentTime - lastReadOn ) >= leakData->readIntervalInMs )
            {
            	lastReadOn = esp_timer_get_time()/1000;
                //uint16_t currAdc = ( uint16_t )analogRead( ANALOG_PIN ) >> 1;//@TODO:Uncomment the required Line
            	uint16_t currAdc = 0;//@TODO:Delete while uncommenting above line

                if( leakData->currAdcCount > 0 )
                	leakData->currAdcCount = ( leakData->currAdcCount + currAdc ) / 2;
                else
                	leakData->currAdcCount = currAdc;

                if( leakData->currAdcCount > 2048 )
                	leakData->currAdcCount = 2048;

                leakData->currPressure = map( leakData->currAdcCount, calib->minAdcCnt, calib->maxAdcCnt,
                                                                      calib->minPressValue, calib->maxPressValue );
//                LEAK_DBG_LOG( LEAK_TYPEB_TAG, "Curr ADC:%d, PMS-AvgADC : %d, CurrPress:%d, AvgPress : %d, \n", currAdc, mainStruct->currAdcCount,
//                       map( currAdc, calib->minAdcCnt, calib->maxAdcCnt, calib->minPressValue, calib->maxPressValue ),
//                       mainStruct->currPressure );

                if( ( typeB->setPressure_P > 0 && leakData->currPressure >= typeB->setPressure_P ) ||
                    ( typeB->setPressure_P < 0 && leakData->currPressure <= typeB->setPressure_P ) )
                {
                    setPumpState( LEAK_TEST_PUMP_OFF );
                    typeB->typeBSMState = TYPE_B_SET_PRESS_ACHIEVED_STATE;
                    LEAK_DBG_LOG( LEAK_B_DBG_TAG, "Set Pressure Achieved" );
                    return;
                }
            }

            //Check if Pump on time expired
            if( ( currentTime - processStartedOn ) >= typeB->pumpOnTime_T1 )
            {
                /* Jump to Pump on Timeout */
                setPumpState( LEAK_TEST_PUMP_OFF );
                typeB->typeBSMState = TYPE_B_PUMP_ON_TIMEOUT_STATE;
            }
        }
        break;

        case TYPE_B_SET_PRESS_ACHIEVED_STATE:
        {
            /* Do Nothing : This state is created for future purpose */
            typeB->typeBSMState = TYPE_B_PRESS_STABILIZATION_MONITORING_STATE;
            milliSecond = esp_timer_get_time()/1000;
            LEAK_DBG_LOG( LEAK_B_DBG_TAG, "Moving to Pressure Stabilization Monitoring State" );
        }
        break;

        case TYPE_B_PRESS_STABILIZATION_MONITORING_STATE:
        {
            if( ( currentTime - milliSecond ) >= typeB->stabilizeTime_T2 )
            {
            	milliSecond = esp_timer_get_time()/1000;
                //leakData->currAdcCount = ( uint16_t )analogRead( ANALOG_PIN ) >> 1;//@TODO:Uncomment the required Line
                leakData->currAdcCount = 0;//@TODO:Delete while uncommenting above line
                leakData->currPressure = ( int )map( leakData->currAdcCount, calib->minAdcCnt, calib->maxAdcCnt,
                                                     calib->minPressValue, calib->maxPressValue );
                typeB->pressAfterStabilization_P1 = leakData->currPressure;
                LEAK_DBG_LOG( LEAK_B_DBG_TAG, "PSMS-ADC : %d, Press : %d\n", leakData->currAdcCount,
                															   leakData->currPressure );

                int currPress = typeB->pressAfterStabilization_P1;
                int dropValue = typeB->stabilizeTimeDropValue_D1;

                // i. Node should report Test OK to Application if P1 >= ( P - D1 ) incase of positive set pressure and P1 <= ( P - D1 ) incase of negative set pressure
                //ii. Node should report error to application if P1 < ( P - D1 )
                if( ( typeB->setPressure_P > 0 ) &&
                    ( currPress >= ( typeB->setPressure_P - dropValue ) ) )
                {
                    typeB->typeBSMState = TYPE_B_TEST_DELAY_MONITORING_STATE;
                    LEAK_DBG_LOG( LEAK_B_DBG_TAG, "+ Pressure Sustained in Stabilized Period" );
                }
                else if( ( typeB->setPressure_P < 0 ) &&
                         ( currPress <= ( typeB->setPressure_P + dropValue ) ) )
                {
                    typeB->typeBSMState = TYPE_B_TEST_DELAY_MONITORING_STATE;
                    LEAK_DBG_LOG( LEAK_B_DBG_TAG, "- Pressure Sustained in Stabilized Period" );
                }
                else
                {
                    operateStatusLeds( LEAK_TEST_RESULT_STATUS_LED_P2, STATUS_LED_ON );
					typeB->typeBSMState = TYPE_B_RESULT_TO_APP_WAIT_CHECK;
					leakData->leakTestResult = FAILED_TO_SUSTAIN_IN_STABILIZATION;

					typeB->typeBSMState = TYPE_B_RESULT_TO_APP_WAIT_CHECK;
					LEAK_DBG_LOG( LEAK_B_DBG_TAG, "Failed To Sustain in stabilization Period" );
                }
            }
        }
        break;

        case TYPE_B_TEST_DELAY_MONITORING_STATE:
        {
            if( ( currentTime - milliSecond ) >= typeB->testDelay_T3 )
            {
            	milliSecond = esp_timer_get_time()/1000;
                //leakData->currAdcCount = ( uint16_t )analogRead( ANALOG_PIN ) >> 1;//@TODO:Uncomment the required Line
                leakData->currAdcCount = 0;//@TODO:Delete while uncommenting above line
                leakData->currPressure = ( int )map( leakData->currAdcCount, calib->minAdcCnt, calib->maxAdcCnt,
                                                                             calib->minPressValue, calib->maxPressValue );
                LEAK_DBG_LOG( LEAK_B_DBG_TAG, "TDMS-ADC : %d, Press : %d\n", leakData->currAdcCount, leakData->currPressure );

                typeB->pressAfterTestDelay_P2 = leakData->currPressure;

                int currPress = typeB->pressAfterTestDelay_P2;
                int dropValue = typeB->testDelayDropValue_D2;

				typeB->typeBSMState = TYPE_B_RESULT_TO_APP_WAIT_CHECK;

                // i. Node should report Test OK to application of P1-P2 < D2
                //ii. Node should report Test Error to application of P1-P2 >= D2.
                //P1 = pressAfterStabilization
                //P2 = currPress
                if( typeB->setPressure_P > 0 )
                {
                    if( ( currPress >= typeB->pressAfterStabilization_P1 ) ||
                        ( ( typeB->pressAfterStabilization_P1 - currPress ) < dropValue ) )
                    {
                    	leakData->leakTestResult = LEAK_TEST_PASS;
                    	LEAK_DBG_LOG( LEAK_B_DBG_TAG, "B+ : Test Pass!!" );
                        return;
                    }
                }
                else if( typeB->setPressure_P < 0 )
                {
                    if( ( currPress <= typeB->pressAfterStabilization_P1 ) ||
                        ( ( typeB->pressAfterStabilization_P1 - currPress ) < dropValue ) )
                    {
                    	leakData->leakTestResult = LEAK_TEST_PASS;
                    	LEAK_DBG_LOG( LEAK_B_DBG_TAG, "B- : Test Pass!!" );
                        return;
                    }
                }
                leakData->leakTestResult = FAILED_TO_SUSTAIN_IN_TEST_DELAY;
                operateStatusLeds( LEAK_TEST_RESULT_STATUS_LED_P2, STATUS_LED_ON );
                LEAK_DBG_LOG( LEAK_B_DBG_TAG, "Failed to Sustain in Test Delay Period" );
            }
        }
        break;

        case TYPE_B_PUMP_ON_TIMEOUT_STATE:
        {
            operateStatusLeds( LEAK_TEST_RESULT_STATUS_LED_P2, STATUS_LED_ON );
            typeB->pressAfterPumpOnTimeOutT1 = leakData->currPressure;
            leakData->leakTestResult = PUMP_ON_TIMEOUT;
			typeB->typeBSMState = TYPE_B_RESULT_TO_APP_WAIT_CHECK;
			LEAK_DBG_LOG( LEAK_B_DBG_TAG, "Pump On Timeout has occurred" );
        }
        break;

		case TYPE_B_RESULT_TO_APP_WAIT_CHECK:
		{
			if( isContinuityScanningInProgress( ) ||
				getContinuityProcessState() == CONTINUITY_PROCESS_IDLE )
            {
				typeB->typeBSMState = TYPE_B_IDLE_STATE;
				leakData->testProcessStatus = TEST_RESULT_NOT_SENT;
                registerTxMessage( LEAK_TEST_RESULT_MESG, NODE_NUMBER * LEAK_RESULT_SHARE_WAIT_PERIOD, P2P_MESG, TCP_MESSAGE );
                LEAK_DBG_LOG( LEAK_B_DBG_TAG, "Flag Raised to share leak test values with APP" );
            }
		}
		break;

        default:
        break;
    }
}

void resetLeakTestParams( void )
{
    setPumpState( LEAK_TEST_PUMP_OFF );
    operateStatusLeds( LEAK_TEST_RESULT_STATUS_LED_P2, STATUS_LED_OFF );

    memset( &leakTestData, '\0', sizeof( leakData_t ) );
    leakTestData.readIntervalInMs = PRESSURE_READING_INTERVAL;

    /*
    leakTestData.calibType = POSITIVE_CALIBRATION;
    memset( &leakTestData.sensorCalibration, '\0', MAX_CALIBRATION * sizeof( sensorCalibValues_t ) );
    leakTestData.leakTestType = NO_LEAK_TESTING;
    memset( &leakTestData.testTypeData, '\0', sizeof( testTypeData_t ) );

    leakTestData.currAdcCount = 0;
    leakTestData.currPressure = 0;

    leakTestData.readIntervalInMs = PRESS_READING_INTERVAL;
    leakTestData.testProcessStatus = TEST_NOT_RUNNING;
    leakTestData.leakTestResult = LEAK_TEST_PASS;
    */
}

leakData_t *getLeaskTestDataObject( void )
{
	return &leakTestData;
}

void *getLeaskTestDataObjectByLeakTestType( enLeakTestType _type )
{
    if( _type == LEAK_TEST_TYPE_A )
        return &leakTestData.testTypeData.typeA;
    else if( _type == LEAK_TEST_TYPE_B )
        return &leakTestData.testTypeData.typeB;
    return NULL;
}

sensorCalibValues_t *getLeakTestCalibrationDataObject( void )
{
	return &leakTestData.sensorCalibration;
}

enLeakTestType getCurrentLeakTestType( void )
{
    return ( enLeakTestType )leakTestData.leakTestType;
}

void setCurrentLeakTestType( uint8_t _value )
{
    if( _value < MAX_LEAK_TEST_TYPE )
    	leakTestData.leakTestType = ( enLeakTestType )_value;
}

enLeakPumpState getPumpState( void )
{
    return leakTestData.pumpState;
}

void setPumpState( enLeakPumpState _state )
{
    if( _state >= MAX_PUMP_STATE )
      return;

    leakTestData.pumpState = _state;
    operatePressureOrVaccumPump( _state );
}

static void operatePressureOrVaccumPump( enLeakPumpState _state )
{
    /* Set GPIO PIN direction as output */
	/*
    if( getPINCurrentDirectionStatus( ) != IO_OUTPUT )
        setDataPinsDirection( OUTPUT );

    if( _state == LEAK_TEST_PUMP_OFF )
        bitWrite( P5ToP12_16BitData, PUMP_ON_OFF_P11, LOW );
    else if( _state == LEAK_TEST_PUMP_ON )
        bitWrite( P5ToP12_16BitData, PUMP_ON_OFF_P11, HIGH );
    else
        return;

    for( uint8_t ite = 0; ite < 8; ite++ )
        digitalWrite( DATA_PINS[ ite ], bitRead( P5ToP12_16BitData, ite ) );

    digitalWrite( LATCH_ENABLE_PINS[ 2 ], HIGH );
    delayMicroseconds( 15 );
    digitalWrite( LATCH_ENABLE_PINS[ 2 ], LOW );
    delayMicroseconds( 15 );*/ //TODO : Uncomment all above code
}


