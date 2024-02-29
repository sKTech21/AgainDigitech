/*
 * leakTestHandeling.h
 *
 *  Created on: May 21, 2023
 *      Author: pbhum
 */

#ifndef MAIN_INCL_LEAKTESTHANDELING_H_
#define MAIN_INCL_LEAKTESTHANDELING_H_

#include "../incl/commonDeclaration.h"

#define PRESSURE_READING_INTERVAL       ( 200 ) //In mS, Read Analog Value( For Pressure ) on every mentioned interval
#define LEAK_RESULT_SHARE_WAIT_PERIOD	( 5 )

typedef enum
{
    TEST_NOT_RUNNING = 0,
    TEST_RUNNING,
    TEST_PERFORMED,
    TEST_RESULT_NOT_SENT,
    TEST_RESULT_SENT,         //After Sending Values to Application, State will be switched to "TEST_PERFORMED"
}enLeakTestProcStatus;

typedef enum
{
    POSITIVE_CALIBRATION = 0,
    NEGATIVE_CALIBRATION,
    MAX_CALIBRATION
}enCalibTypes;

typedef enum
{
    NO_LEAK_TESTING = 0,
    LEAK_TEST_TYPE_A,
    LEAK_TEST_TYPE_B,
    MAX_LEAK_TEST_TYPE
}enLeakTestType;

typedef enum
{
    LEAK_TEST_PUMP_OFF = 0,
    LEAK_TEST_PUMP_ON,
    MAX_PUMP_STATE,
}enLeakPumpState;

typedef enum
{
    LEAK_TEST_PASS = 0,
    PUMP_ON_TIMEOUT,
    FAILED_TO_ACHIEVE_SET_PRESSURE,
    FAILED_TO_SUSTAIN_IN_STABILIZATION,
    FAILED_TO_SUSTAIN_IN_TEST_DELAY,
    LEAK_TEST_NOT_PERFORMED_OR_RUNNING
}enLeakTestResult;

/* Do not change this enum sequence, Must be matched with Application */
typedef enum
{
    TYPE_A_PUMP_ON_TIME = 0,
    TYPE_A_SET_PRESSURE,
    TYPE_A_PRESSURE_OFFSET,
    TYPE_B_PUMP_ON_TIME,
    TYPE_B_STABILIZE_TIME,
    TYPE_B_TEST_DELAY,
    TYPE_B_SET_PRESSURE,
    TYPE_B_STABILIZE_DROP_VALUE,
    TYPE_B_TEST_DROP_VALUE,
    MAX_LEAK_CONFIG
}enLeakCongigType;

typedef enum
{
    TYPE_A_IDLE_STATE = 0,
    TYPE_A_PUMP_ON_STATE,
    TYPE_A_PRESS_MEASUREMENT_STATE,
    TYPE_A_SET_PRESS_ACHIEVED_STATE,
    TYPE_A_PUMP_ON_TIMEOUT_STATE,
    TYPE_A_RESULT_TO_APP_WAIT_CHECK,
    TYPE_A_MAX_FM_STATE
}enTypeA_SMState;

typedef enum
{
    TYPE_B_IDLE_STATE = 0,
    TYPE_B_PUMP_ON_STATE,
    TYPE_B_PRESS_MEASUREMENT_STATE,
    TYPE_B_SET_PRESS_ACHIEVED_STATE,
    TYPE_B_PRESS_STABILIZATION_MONITORING_STATE,
    TYPE_B_TEST_DELAY_MONITORING_STATE,
    TYPE_B_PUMP_ON_TIMEOUT_STATE,
	TYPE_B_RESULT_TO_APP_WAIT_CHECK,
}enTypeB_SMState;

typedef struct
{
    uint16_t  	minAdcCnt;
    uint16_t  	maxAdcCnt;
    int       	minPressValue;
    int      	maxPressValue;
}sensorCalibValues_t;

typedef struct
{
    enTypeA_SMState   	typeASMState;
    uint16_t    		pumpOnTimeT1;
    int         		setPressureValue;
    int         		pressureOffset;
    int         		pressAfterPumpOnTimeOutT1;   //This will store current pressure ADC count after T1 Expired
}stTypeAConfig;

typedef struct
{
    enTypeB_SMState   	typeBSMState;

    //Configuration Received from Application
    uint16_t  			pumpOnTime_T1;                 // Value in milliSecond
    uint16_t  			stabilizeTime_T2;              // Value in milliSecond
    uint16_t  			testDelay_T3;                  // Value in milliSecond
    int       			setPressure_P;
    uint8_t   			stabilizeTimeDropValue_D1;
    uint8_t   			testDelayDropValue_D2;

    //Pressure Values after state change
    int       			pressAfterPumpOnTimeOutT1;     //Save Current Achieved Pressure Value once Pump On Timeout occurs and failed to achieve set pressure
    int       			pressAfterStabilization_P1;    //Save Current Pressure value once Stabilization period is over
    int       			pressAfterTestDelay_P2;        //Save Current Pressure value once Test Delay period is over
}stTypeBConfig;

typedef union
{
    stTypeAConfig   	typeA;
    stTypeBConfig   	typeB;
}testTypeData_t;

/*
 * Main Leak Test Structure
*/
typedef struct
{
    enLeakPumpState         pumpState;
    enCalibTypes            calibType;
    sensorCalibValues_t     sensorCalibration;
    testTypeData_t          testTypeData;
    enLeakTestProcStatus    testProcessStatus;
    enLeakTestResult        leakTestResult;      //Leak Test Result and its failed state if any
    uint8_t                 leakTestType;
    uint16_t                currAdcCount;
    uint16_t                readIntervalInMs;
    int                     currPressure;
}leakData_t;

typedef struct
{
	uint8_t					leakTestResultSentTOApp;	//1 = Sent TO APP, else 0
	uint8_t 				noOfTimesLeakTestResultSent;
	uint8_t					leakTestResultAckFromApp;	//1 = Ack Received, else 0
	uint64_t				leakTestResultSentOn;
}leakTestMesgStatus_T;

void leakTestTaskInit( void );
void resetLeakTestParams( void );
void resumeOrSuspendLeakTestTask( bool _state );

//Get Set Function for LeakTest Type
enLeakTestType getCurrentLeakTestType( void );
void setCurrentLeakTestType( uint8_t _value );

//Get Set Functions for Pump Status
enLeakPumpState getPumpState( void );
void setPumpState( enLeakPumpState _state );

leakData_t *getLeaskTestDataObject( void );
void *getLeaskTestDataObjectByLeakTestType( enLeakTestType _type );
sensorCalibValues_t *getLeakTestCalibrationDataObject( void );

#endif /* MAIN_INCL_LEAKTESTHANDELING_H_ */
