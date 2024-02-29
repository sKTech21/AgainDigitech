/*
 * unitTesting.c
 *
 *  Created on: Jun. 11, 2023
 *      Author: pbhum
 */

#ifndef MAIN_SRC_UNITTESTING_C_
#define MAIN_SRC_UNITTESTING_C_

#include "../incl/unitTesting.h"
#include "../incl/continuity.h"
#include "../incl/commonDeclaration.h"

void prepareTestMessage( void )
{
	uint64_t startScanningOn = 4000;
	continuityProdConfiguration_t prodCOnfig;

	//Set Node IO for Product
	for( uint8_t node = 0; node < 10; node++ )
	{
		prodCOnfig.IOassociatedWithNode[ node ] = node + 20;
	}
	prodCOnfig.totalProductNodes = 10;
	prodCOnfig.reportingDelay = 10;
	prodCOnfig.startScanningFromNode = 1;
	prodCOnfig.noOfNodesToBeScanned = 10;

	prodCOnfig.allowedScanningPeriod = 20;
	prodCOnfig.idlePeriodAfterScanning = 15;
	prodCOnfig.totalScanningPeriodForOneOp = prodCOnfig.allowedScanningPeriod + prodCOnfig.idlePeriodAfterScanning;
	prodCOnfig.productUniqueID = 12345678;

	for( uint8_t node = 0; node < 10; node++ )
	{
		if( node == 0 )
			prodCOnfig.startScanningOn[ node ] = startScanningOn + prodCOnfig.totalScanningPeriodForOneOp;
		else
			prodCOnfig.startScanningOn[ node ] = prodCOnfig.startScanningOn[ node - 1 ] + 1000;
	}
	setProductCOnfigurationParameters( prodCOnfig );
}

#endif /* MAIN_SRC_UNITTESTING_C_ */
