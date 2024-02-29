/*
 * nvs_nodeConfiguration.h
 *
 *  Created on: May 21, 2023
 *      Author: pbhum
 */

#ifndef MAIN_INCL_NVS_NODECONFIGURATION_H_
#define MAIN_INCL_NVS_NODECONFIGURATION_H_

#include "esp_system.h"

bool initNVSFlashMemory( );
bool NVS_setNodeConfiguration( uint8_t isConfigured, uint8_t _nodeNumber, uint16_t _relayOnPeriod,
		                       uint16_t connectorLock1Period, uint16_t connectorLock2Period );

#endif /* MAIN_INCL_NVS_NODECONFIGURATION_H_ */
