// FlyConnect.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "PMDG_NGX_SDK.h"
#include "Panel.h"
#include "SimConnect.h"
#include "Events.h"
#include "Radio.h"
#include "Joystick.h"
#include "PMDGWrapper.h"

HANDLE hSimConnect = NULL;

void CALLBACK MyDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void *pContext) {
	auto ngx = (NgxInterface*)pContext;

	switch (pData->dwID) {
	case SIMCONNECT_RECV_ID_EXCEPTION: {
		SIMCONNECT_RECV_EXCEPTION * except = (SIMCONNECT_RECV_EXCEPTION*)pData;
		printf("***** EXCEPTION=%d  SendID=%d  Index=%d  cbData=%d\n", except->dwException, except->dwSendID, except->dwIndex, cbData);
		break;
	}

	case SIMCONNECT_RECV_ID_OPEN:
		printf("MyDispatchProc.Received: SIMCONNECT_RECV_ID_OPEN\n");
		break;

	case SIMCONNECT_RECV_ID_QUIT:
		printf("MyDispatchProc.Received: SIMCONNECT_RECV_ID_QUIT\n");
		ngx->connected = 0;
		break;

	case SIMCONNECT_RECV_ID_EVENT:
		HandleSimEvent(ngx, (SIMCONNECT_RECV_EVENT*)pData);
		break;

	case SIMCONNECT_RECV_ID_CLIENT_DATA: {
		ngx->setData((SIMCONNECT_RECV_CLIENT_DATA*)pData);
		break;
	}

	case SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE: {
		SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE *pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE*)pData;

		if (pObjData->dwRequestID == RADIO_REQUEST) {
			DWORD ObjectID = pObjData->dwObjectID;
			radio.setRadioData((FSX_RadioData*)&pObjData->dwData);
		}
		break;
	}

	default:
		printf("MyDispatchProc.Received:%d\n", pData->dwID);
		break;
	}

	fflush(stdout);
}

DWORD WINAPI PollForData(LPVOID lpParam) {
	auto ngx = (NgxInterface*)lpParam;

	while (ngx->connected) {
		SimConnect_CallDispatch(hSimConnect, MyDispatchProc, lpParam);
		Sleep(10);
	}

	return 0;
}

void run() {
	HRESULT hr = SimConnect_Open(&hSimConnect, "PMDGWrapper", NULL, 0, 0, 0);

	if (FAILED(hr)) return;

	printf("Connected to Flight Simulator\n");
	Joystick_MapEvents(hSimConnect);
	radio.setup(hSimConnect);

	auto ngx = new NgxInterface(hSimConnect);
	auto pollForDataThread = CreateThread(NULL, 0, PollForData, ngx, 0, NULL);

	ngx->connect();

	while (ngx->connected) {
		sendNGX_PanelState(&ngx->data);
		radio.sendControlData(hSimConnect);
		//SimConnect_RequestDataOnSimObjectType(hSimConnect, RADIO_REQUEST, RADIO_DEF, 0, SIMCONNECT_SIMOBJECT_TYPE_USER);

		void* ptr = getControlData();

		if (ptr != NULL) {
			NGX_Control* ctrl = (NGX_Control*)ptr;

			ngx->adjust(EVENT_SPEED_SELECTOR, ctrl->airspeed);
			ngx->adjust(EVENT_HEADING_SELECTOR, ctrl->heading);
			ngx->adjust(EVENT_ALTITUDE_SELECTOR, ctrl->altitude);
			ngx->adjust(EVENT_COURSE_SELECTOR_L, ctrl->courseL);
			ngx->adjust(EVENT_COURSE_SELECTOR_R, ctrl->courseR);
			ngx->adjust(EVENT_BARO_SELECTOR_L, ctrl->efisBaro);
			ngx->adjust(EVENT_MINS_SELECTOR_L, ctrl->efisMins);

			// MIP
			ngx->pressButton(EVT_MPM_MFD_ENG_BUTTON, ctrl->mipButtons & BTN_MFD_ENG);
			ngx->pressButton(EVT_MPM_MFD_SYS_BUTTON, ctrl->mipButtons & BTN_MFD_SYS);
			ngx->pressButton(EVT_DSP_CPT_AP_RESET_SWITCH, ctrl->mipButtons & BTN_AP_RST);
			ngx->pressButton(EVT_DSP_CPT_AT_RESET_SWITCH, ctrl->mipButtons & BTN_AT_RST);
			ngx->pressButton(EVT_DSP_CPT_FMC_RESET_SWITCH, ctrl->mipButtons & BTN_FMC_RST);
			ngx->send(EVT_MPM_AUTOBRAKE_SELECTOR, ctrl->autoBreak, ngx->data.MAIN_AutobrakeSelector);
			ngx->send(EVT_EFIS_CPT_VOR_ADF_SELECTOR_L, ctrl->vorAdfSel1, ngx->data.EFIS_VORADFSel1[0]);
			ngx->send(EVT_EFIS_CPT_VOR_ADF_SELECTOR_R, ctrl->vorAdfSel2, ngx->data.EFIS_VORADFSel2[0]);
			ngx->send(EVT_DSP_CPT_MASTER_LIGHTS_SWITCH, ctrl->mainLights, ngx->data.MAIN_LightsSelector);
			ngx->send(EVT_DSP_CPT_DISENGAGE_TEST_SWITCH, ctrl->disengageLights, ngx->data.MAIN_DisengageTestSelector[0]);
			ngx->send(EVT_MPM_FUEL_FLOW_SWITCH, ctrl->fuelFlowSw, ngx->data.MAIN_FuelFlowSelector);
			ngx->send(EVT_DSP_CPT_MAIN_DU_SELECTOR, ctrl->mainPanelDuSel, ngx->data.MAIN_MainPanelDUSel[0]);
			ngx->send(EVT_DSP_CPT_LOWER_DU_SELECTOR, ctrl->loweDuSel, ngx->data.MAIN_LowerDUSel[0]);
			ngx->pressButton(EVT_MASTER_CAUTION_LIGHT_LEFT, ctrl->mipButtons & BTN_MSTR_CAUTION);
			ngx->pressButton(EVT_FIRE_WARN_LIGHT_LEFT, ctrl->mipButtons & BTN_FIRW_WARNING);
			ngx->pressButton(EVT_SYSTEM_ANNUNCIATOR_PANEL_LEFT, ctrl->mipButtons & BTN_WARN_RECALL);

			// MCP
			if (((ctrl->mcpButtons & BTN_FD_LFT) != 0) != ngx->data.MCP_FDSw[0]) ngx->send(EVT_MCP_FD_SWITCH_L, ctrl->mcpButtons & BTN_FD_LFT ? 0 : 1);
			if (((ctrl->mcpButtons & BTN_FD_RGH) != 0) != ngx->data.MCP_FDSw[1]) ngx->send(EVT_MCP_FD_SWITCH_R, ctrl->mcpButtons & BTN_FD_RGH ? 0 : 1);
			if (((ctrl->mcpButtons & BTN_ATARM) != 0) != ngx->data.MCP_ATArmSw) ngx->send(EVT_MCP_AT_ARM_SWITCH, ctrl->mcpButtons & BTN_ATARM ? 0 : 1);
			ngx->pressButton(EVT_MCP_SPEED_SWITCH, ctrl->mcpButtons & BTN_SPEED);
			ngx->pressButton(EVT_MCP_VNAV_SWITCH, ctrl->mcpButtons & BTN_VNAV);
			ngx->pressButton(EVT_MCP_LVL_CHG_SWITCH, ctrl->mcpButtons & BTN_LVLCHG);
			ngx->pressButton(EVT_MCP_HDG_SEL_SWITCH, ctrl->mcpButtons & BTN_HDGSEL);
			ngx->pressButton(EVT_MCP_LNAV_SWITCH, ctrl->mcpButtons & BTN_LNAV);
			ngx->pressButton(EVT_MCP_VOR_LOC_SWITCH, ctrl->mcpButtons & BTN_VORLOC);
			ngx->pressButton(EVT_MCP_APP_SWITCH, ctrl->mcpButtons & BTN_APP);
			ngx->pressButton(EVT_MCP_ALT_HOLD_SWITCH, ctrl->mcpButtons & BTN_ALTHLD);
			ngx->pressButton(EVT_MCP_CMD_A_SWITCH, ctrl->mcpButtons & BTN_CMDA);
			ngx->pressButton(EVT_MCP_CMD_B_SWITCH, ctrl->mcpButtons & BTN_CMDB);
			ngx->pressButton(EVT_MCP_VS_SWITCH, ctrl->mcpButtons & BTN_VS);
			// EFIS
			ngx->send(EVT_EFIS_CPT_MODE, ctrl->efisMode, ngx->data.EFIS_ModeSel[0]);
			ngx->send(EVT_EFIS_CPT_RANGE, ctrl->efisRange, ngx->data.EFIS_RangeSel[0]);
			ngx->pressButton(EVT_EFIS_CPT_WXR, ctrl->efisButtons & EFIS_WXR);
			ngx->pressButton(EVT_EFIS_CPT_STA, ctrl->efisButtons & EFIS_STA);
			ngx->pressButton(EVT_EFIS_CPT_WPT, ctrl->efisButtons & EFIS_WPT);
			ngx->pressButton(EVT_EFIS_CPT_ARPT, ctrl->efisButtons & EFIS_ARPT);
			ngx->pressButton(EVT_EFIS_CPT_DATA, ctrl->efisButtons & EFIS_DATA);
			ngx->pressButton(EVT_EFIS_CPT_POS, ctrl->efisButtons & EFIS_POS);
			ngx->pressButton(EVT_EFIS_CPT_TERR, ctrl->efisButtons & EFIS_TERR);
			ngx->pressButton(EVT_EFIS_CPT_WXR, ctrl->efisButtons & EFIS_WXR);
			ngx->pressButton(EVT_EFIS_CPT_FPV, ctrl->efisButtons & EFIS_FPV);
			ngx->pressButton(EVT_EFIS_CPT_MTRS, ctrl->efisButtons & EFIS_MTRS);
			ngx->pressButton(EVT_EFIS_CPT_BARO_STD, ctrl->efisButtons & EFIS_STD);
			ngx->pressButton(EVT_EFIS_CPT_MINIMUMS_RST, ctrl->efisButtons & EFIS_RST);
		}

		Sleep(50);
	}
}

void lab() {
	display_test();
	unsigned char counter1 = 0;
	unsigned char counter2 = 0;
	unsigned short value = 0;

	while (1) {
		void * ptr = getControlData();

		if (ptr != NULL) {
			NGX_Control* ctrl = (NGX_Control*)ptr;
			printf(">>> Control received, CourseL = %d, Autobreak = %d, EFIS = %d\n", ctrl->courseL, ctrl->autoBreak, (ctrl->efisButtons & EFIS_DATA));
			printf("EFIS Range = %d, EFIS Mode = %d\n", ctrl->efisRange, ctrl->efisMode);
			value += ctrl->airspeed;

			int i;
			for (i = 0; i < 16; i++) {
				if (dataReceived.mcp.buttons & (1 << i)) break;
			}
			printf("mcp buttons %x, bit %d\n", dataReceived.mcp.buttons, i);

			unsigned long b = dataReceived.mip.mipButtons;
			for (i = 0; i < 32; i++) {
				if (b == (unsigned long)(~(1 << i))) break;
			}
			printf("mip buttons %x, bit %d\n", b, i);

			unsigned long e = dataReceived.mip.efisButtons;
			unsigned long pat = 0xfff7fff3;
			for (i = 0; i < 32; i++) {
				if (e == (pat & ~(1 << i))) break;
			}
			printf("efis buttons %x, bit %d\n", e, i);
		}

		time_t seconds = time(NULL);

		dataToSend.mcp.buttons = (1 << counter1);
		dataToSend.mip.flaps = getGaugeValue(value);
		int mask = seconds % 2 ? 0xFFFFAFFF : 0xFFFFFFFF;
		dataToSend.mcp.speedCrsL = displayHi(value) & mask & displayLo((float)0.78);
		dataToSend.mcp.vspeedCrsR = 0xDEFFFFFF & displayLo(-1000);
		dataToSend.mcp.altitudeHdg = displayHi(counter1) & displayLo(getGaugeValue(value));
		dataToSend.mip.backlight = value * 5;
		dataToSend.mcp.backlight = value * 5;
		mcp.sendData(&dataToSend.mcp);
		mip.sendData(&dataToSend.mip);
		counter1++;
		counter2++;
		if (counter1 == 16) counter1 = 0;
		if (counter2 == 8) counter2 = 0;
		Sleep(500);
	}
}

void connect(SerialPort* port, const wchar_t* path, const char* name) {
	printf("%s connecting...\n", name);
	if (port->connect(path)) {
		char *message = port->readMessage();
		printf("%s connected: %s\n", name, message);
	}
}

int _tmain(int argc, _TCHAR* argv[]) {
	connect(&mcp, L"COM5", "MCP");
	connect(&mip, L"COM6", "MIP");
	//lab();
	run();
	mip.close();
	mcp.close();
	Sleep(1000);
	return 0;
}