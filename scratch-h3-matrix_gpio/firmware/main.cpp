#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "hardware.h"
#include "networkh3emac.h"
#include "ledblink.h"

#include "display.h"

#include "spiflashinstall.h"
#include "spiflashstore.h"
#include "remoteconfig.h"
#include "remoteconfigparams.h"

#include "storenetwork.h"
#include "storeremoteconfig.h"

#include "networkhandleroled.h"

#include "firmwareversion.h"

static const char SOFTWARE_VERSION[] = "0.0";

#include "h3/shell.h"

#include "hub75bdisplay.h"

#include "e131bridge.h"

#define COLUMNS 64
#define ROWS 	32

extern "C" {

void notmain(void) {
	Hardware hw;
	NetworkH3emac nw;
	LedBlink lb;
	Display display(0,4);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	Shell shell;

	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	fw.Print();

	NetworkHandlerOled networkHandlerOled;

	nw.SetNetworkDisplay(&networkHandlerOled);
	nw.SetNetworkStore(StoreNetwork::Get());
	nw.Init(StoreNetwork::Get());
	nw.Print();

	networkHandlerOled.ShowIp();

	RemoteConfig remoteConfig(REMOTE_CONFIG_RDMNET_LLRP_ONLY, REMOTE_CONFIG_MODE_CONFIG, 0);

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if (remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;

	/*
	 *
	 */

	Hub75bDisplay matrix(COLUMNS,ROWS);
	matrix.Start();

	uint32_t n = 0;

	for (uint32_t nRow = 0; nRow < ROWS; nRow++) {
		for (uint32_t nColomn = 0; nColomn < COLUMNS; nColomn++) {
			switch (n) {
				case 0:
					matrix.SetPixel(nColomn, nRow, 0xFF, 0, 0);
					break;
				case 1:
					matrix.SetPixel(nColomn, nRow, 0, 0xFF, 0);
					break;
				case 2:
					matrix.SetPixel(nColomn, nRow, 0, 0, 0xFF);
					break;
				default:
					break;
			}
		}

		n++;

		if (n == 3) {
			n = 0;
		}
	}

	E131Bridge bridge;

	for (uint32_t i = 0; i < 13; i++) { // ceil(COLUMNS * ROWS / 170)
		bridge.SetUniverse(i, E131_OUTPUT_PORT, i + 1);
	}

	bridge.SetOutput(&matrix);
	bridge.Print();


	//matrix.Dump();

	display.ClearLine(0);
	display.ClearLine(1);
	display.ClearLine(3);

//	int nPrevSeconds = 60; // Force initial update

	for (;;) {
		nw.Run();
		remoteConfig.Run();
		//spiFlashStore.Flash();
		//lb.Run();
		//shell.Run();
		bridge.Run();

		/*
		 * Debugging
		 */
		matrix.Run();

//		time_t ltime = time(nullptr);
//		const struct tm *tm = localtime(&ltime);
//
//		if (tm->tm_sec != nPrevSeconds) {
//			nPrevSeconds = tm->tm_sec;
//			display.Printf(1, "%u", matrix.GetFps());
//		}
	}
}

}
