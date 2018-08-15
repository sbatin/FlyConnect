// FlyConnect.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Panel.h"
#include "Events.h"
#include "SimConnect.h"
#include "Radio.h"
#include "Joystick.h"
#include "NgxInterface.h"

Panel panel = Panel();
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

void sendNGX_PanelState(PMDG_NGX_Data* state) {
	auto seconds = time(NULL);

	if (state->ELEC_BatSelector) {
		DisplayState IASState = Blank;

		if (!state->MCP_IASBlank) {
			if (state->MCP_IASOverspeedFlash && seconds % 2 == 0) {
				IASState = Overspeed;
			} else if (state->MCP_IASUnderspeedFlash && seconds % 2 == 0) {
				IASState = Underspeed;
			} else {
				IASState = Enabled;
			}
		}

		panel.setMCPDisplays(state->MCP_Course[0], state->MCP_IASMach, IASState, state->MCP_Heading, state->MCP_Altitude, state->MCP_VertSpeed, !state->MCP_VertSpeedBlank, state->MCP_Course[1]);
	} else {
		panel.mcpData.speedCrsL = DISP_OFF_MASK;
		panel.mcpData.vspeedCrsR = DISP_OFF_MASK;
		panel.mcpData.altitudeHdg = DISP_OFF_MASK;
	}

	panel.mcpData.alt_hld = state->MCP_annunALT_HOLD;
	panel.mcpData.app = state->MCP_annunAPP;
	panel.mcpData.at_arm = state->MCP_ATArmSw;
	panel.mcpData.cmd_a = state->MCP_annunCMD_A;
	panel.mcpData.cmd_b = state->MCP_annunCMD_B;
	panel.mcpData.fd_ca = state->MCP_annunFD[0];
	panel.mcpData.fd_fo = state->MCP_annunFD[1];
	panel.mcpData.hdg_sel = state->MCP_annunHDG_SEL;
	panel.mcpData.lnav = state->MCP_annunLNAV;
	panel.mcpData.lvl_chg = state->MCP_annunLVL_CHG;
	panel.mcpData.n1 = state->MCP_annunN1;
	panel.mcpData.speed = state->MCP_annunSPEED;
	panel.mcpData.vnav = state->MCP_annunVNAV;
	panel.mcpData.vor_loc = state->MCP_annunVOR_LOC;
	panel.mcpData.vs = state->MCP_annunVS;

	panel.mcpData.backlight = state->LTS_MainPanelKnob[0];
	panel.mipData.backlight = state->LTS_MainPanelKnob[0];
	panel.mipData.flaps = getGaugeValue(state->MAIN_TEFlapsNeedle[0]);
	panel.mipData.annunNGearGrn = state->MAIN_annunGEAR_locked[1];
	panel.mipData.annunNGearRed = state->MAIN_annunGEAR_transit[1];
	panel.mipData.annunRGearGrn = state->MAIN_annunGEAR_locked[0];
	panel.mipData.annunRGearRed = state->MAIN_annunGEAR_transit[0];
	panel.mipData.annunLGearGrn = state->MAIN_annunGEAR_locked[2];
	panel.mipData.annunLGearRed = state->MAIN_annunGEAR_transit[2];
	panel.mipData.annunAntiskidInop = state->MAIN_annunANTI_SKID_INOP;
	panel.mipData.annunAutobreakDisarm = state->MAIN_annunAUTO_BRAKE_DISARM;
	panel.mipData.annunFlapsExt = state->MAIN_annunLE_FLAPS_EXT;
	panel.mipData.annunFlapsTransit = state->MAIN_annunLE_FLAPS_TRANSIT;
	panel.mipData.annunStabOutOfTrim = state->MAIN_annunSTAB_OUT_OF_TRIM;
	panel.mipData.annunAPRstRed = state->MAIN_annunAP[0];
	panel.mipData.annunAPRstYel = state->MAIN_annunAP_Amber[0];
	panel.mipData.annunATRstRed = state->MAIN_annunAT[0];
	panel.mipData.annunATRstYel = state->MAIN_annunAT_Amber[0];
	panel.mipData.annunFMCRstYel = state->MAIN_annunFMC[0];
	panel.mipData.annunSpeedbrakeArmed = state->MAIN_annunSPEEDBRAKE_ARMED;
	panel.mipData.annunSpeedbrakNotArm = state->MAIN_annunSPEEDBRAKE_DO_NOT_ARM;
	panel.mipData.annunBelowGS = state->MAIN_annunBELOW_GS[0];
	panel.mipData.annunFireWarning = state->WARN_annunFIRE_WARN[0];
	panel.mipData.annunMasterCaution = state->WARN_annunMASTER_CAUTION[0];
	panel.mipData.annunWarnFltCont = state->WARN_annunFLT_CONT;
	panel.mipData.annunWarnElec = state->WARN_annunELEC;
	panel.mipData.annunWarnIRS = state->WARN_annunIRS;
	panel.mipData.annunWarnAPU = state->WARN_annunAPU;
	panel.mipData.annunWarnFuel = state->WARN_annunFUEL;
	panel.mipData.annunWarnOvht = state->WARN_annunOVHT_DET;

	// test
	if (state->MAIN_LightsSelector == 0) {
		panel.lightsTest();
		// bright
	} else if (state->MAIN_LightsSelector == 1) {
		panel.mcpData.brightness = 8;
		// dim
	} else if (state->MAIN_LightsSelector == 2) {
		panel.mcpData.brightness = 5;
	}

	panel.send();
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

		if (panel.read()) {
			auto mcpInput = &panel.input.mcp;
			auto mipInput = &panel.input.mip;

			switch (mcpInput->encoder) {
				case MCP_ALTITUDE:
					ngx->adjust(EVENT_ALTITUDE_SELECTOR, mcpInput->value);
					break;
				case MCP_HEADING:
					ngx->adjust(EVENT_HEADING_SELECTOR, mcpInput->value);
					break;
				case MCP_IAS_MACH:
					ngx->adjust(EVENT_SPEED_SELECTOR, mcpInput->value);
					break;
				case MCP_COURSE_CA:
					ngx->adjust(EVENT_COURSE_SELECTOR_L, mcpInput->value);
					break;
				case MCP_COURSE_FO:
					ngx->adjust(EVENT_COURSE_SELECTOR_R, mcpInput->value);
					break;
			}

			if (mcpInput->fd_ca != ngx->data.MCP_FDSw[0]) ngx->send(EVT_MCP_FD_SWITCH_L, mcpInput->fd_ca ? 0 : 1);
			if (mcpInput->fd_fo != ngx->data.MCP_FDSw[1]) ngx->send(EVT_MCP_FD_SWITCH_R, mcpInput->fd_fo ? 0 : 1);
			if (mcpInput->at_arm != ngx->data.MCP_ATArmSw) ngx->send(EVT_MCP_AT_ARM_SWITCH, mcpInput->at_arm ? 0 : 1);
			ngx->pressButton(EVT_MCP_SPEED_SWITCH, mcpInput->speed);
			ngx->pressButton(EVT_MCP_VNAV_SWITCH, mcpInput->vnav);
			ngx->pressButton(EVT_MCP_LVL_CHG_SWITCH, mcpInput->lvl_chg);
			ngx->pressButton(EVT_MCP_HDG_SEL_SWITCH, mcpInput->hdg_sel);
			ngx->pressButton(EVT_MCP_LNAV_SWITCH, mcpInput->lnav);
			ngx->pressButton(EVT_MCP_VOR_LOC_SWITCH, mcpInput->vor_loc);
			ngx->pressButton(EVT_MCP_APP_SWITCH, mcpInput->app);
			ngx->pressButton(EVT_MCP_ALT_HOLD_SWITCH, mcpInput->alt_hld);
			ngx->pressButton(EVT_MCP_CMD_A_SWITCH, mcpInput->cmd_a);
			ngx->pressButton(EVT_MCP_CMD_B_SWITCH, mcpInput->cmd_b);
			ngx->pressButton(EVT_MCP_VS_SWITCH, mcpInput->vs);

			// MIP
			ngx->pressButton(EVT_MPM_MFD_ENG_BUTTON, mipInput->mipButtons & BTN_MFD_ENG);
			ngx->pressButton(EVT_MPM_MFD_SYS_BUTTON, mipInput->mipButtons & BTN_MFD_SYS);
			ngx->pressButton(EVT_DSP_CPT_AP_RESET_SWITCH, mipInput->mipButtons & BTN_AP_RST);
			ngx->pressButton(EVT_DSP_CPT_AT_RESET_SWITCH, mipInput->mipButtons & BTN_AT_RST);
			ngx->pressButton(EVT_DSP_CPT_FMC_RESET_SWITCH, mipInput->mipButtons & BTN_FMC_RST);
			ngx->send(EVT_MPM_AUTOBRAKE_SELECTOR, panel.input.autoBreak, ngx->data.MAIN_AutobrakeSelector);
			ngx->send(EVT_EFIS_CPT_VOR_ADF_SELECTOR_L, panel.input.vorAdfSel1, ngx->data.EFIS_VORADFSel1[0]);
			ngx->send(EVT_EFIS_CPT_VOR_ADF_SELECTOR_R, panel.input.vorAdfSel2, ngx->data.EFIS_VORADFSel2[0]);
			ngx->send(EVT_DSP_CPT_MASTER_LIGHTS_SWITCH, panel.input.mainLights, ngx->data.MAIN_LightsSelector);
			ngx->send(EVT_DSP_CPT_DISENGAGE_TEST_SWITCH, panel.input.disengageLights, ngx->data.MAIN_DisengageTestSelector[0]);
			ngx->send(EVT_MPM_FUEL_FLOW_SWITCH, panel.input.fuelFlowSw, ngx->data.MAIN_FuelFlowSelector);
			ngx->send(EVT_DSP_CPT_MAIN_DU_SELECTOR, panel.input.mainPanelDuSel, ngx->data.MAIN_MainPanelDUSel[0]);
			ngx->send(EVT_DSP_CPT_LOWER_DU_SELECTOR, panel.input.loweDuSel, ngx->data.MAIN_LowerDUSel[0]);
			ngx->pressButton(EVT_MASTER_CAUTION_LIGHT_LEFT, mipInput->mipButtons & BTN_MSTR_CAUTION);
			ngx->pressButton(EVT_FIRE_WARN_LIGHT_LEFT, mipInput->mipButtons & BTN_FIRW_WARNING);
			ngx->pressButton(EVT_SYSTEM_ANNUNCIATOR_PANEL_LEFT, mipInput->mipButtons & BTN_WARN_RECALL);

			// EFIS
			ngx->adjust(EVENT_BARO_SELECTOR_L, mipInput->baro);
			ngx->adjust(EVENT_MINS_SELECTOR_L, mipInput->mins);
			ngx->send(EVT_EFIS_CPT_MODE, panel.input.efisMode, ngx->data.EFIS_ModeSel[0]);
			ngx->send(EVT_EFIS_CPT_RANGE, panel.input.efisRange, ngx->data.EFIS_RangeSel[0]);
			ngx->pressButton(EVT_EFIS_CPT_WXR, mipInput->efisButtons & EFIS_WXR);
			ngx->pressButton(EVT_EFIS_CPT_STA, mipInput->efisButtons & EFIS_STA);
			ngx->pressButton(EVT_EFIS_CPT_WPT, mipInput->efisButtons & EFIS_WPT);
			ngx->pressButton(EVT_EFIS_CPT_ARPT, mipInput->efisButtons & EFIS_ARPT);
			ngx->pressButton(EVT_EFIS_CPT_DATA, mipInput->efisButtons & EFIS_DATA);
			ngx->pressButton(EVT_EFIS_CPT_POS, mipInput->efisButtons & EFIS_POS);
			ngx->pressButton(EVT_EFIS_CPT_TERR, mipInput->efisButtons & EFIS_TERR);
			ngx->pressButton(EVT_EFIS_CPT_WXR, mipInput->efisButtons & EFIS_WXR);
			ngx->pressButton(EVT_EFIS_CPT_FPV, mipInput->efisButtons & EFIS_FPV);
			ngx->pressButton(EVT_EFIS_CPT_MTRS, mipInput->efisButtons & EFIS_MTRS);
			ngx->pressButton(EVT_EFIS_CPT_BARO_STD, mipInput->efisButtons & EFIS_STD);
			ngx->pressButton(EVT_EFIS_CPT_MINIMUMS_RST, mipInput->efisButtons & EFIS_RST);
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
		if (panel.read()) {
			auto ctrl = &panel.input;
			printf(">>> Control received, Autobreak = %d, EFIS Range = %d, EFIS Mode = %d\n", ctrl->autoBreak, ctrl->efisRange, ctrl->efisMode);

			int i;

			for (i = 0; i < 32; i++) {
				if (ctrl->mip.mipButtons & (1 << i)) break;
			}
			printf("mip buttons %x, bit %d\n", ctrl->mip.mipButtons, i);

			for (i = 0; i < 32; i++) {
				if (ctrl->mip.efisButtons & (1 << i)) break;
			}
			printf("efis buttons %x, bit %d\n", ctrl->mip.efisButtons, i);
		}

		time_t seconds = time(NULL);

		panel.mipData.flaps = getGaugeValue(value);
		int mask = seconds % 2 ? 0xFFFFAFFF : 0xFFFFFFFF;
		panel.mcpData.speedCrsL = displayHi(value) & mask & displayLo((float)0.78);
		panel.mcpData.vspeedCrsR = 0xDEFFFFFF & displayLo(-1000);
		panel.mcpData.altitudeHdg = displayHi(counter1) & displayLo(getGaugeValue(value));
		panel.mcpData.backlight = value * 5;
		panel.mcpData.backlight = value * 5;
		panel.send();
		counter1++;
		counter2++;
		if (counter1 == 16) counter1 = 0;
		if (counter2 == 8) counter2 = 0;
		Sleep(100);
	}
}

int main() {
	panel.connect(L"COM5", L"COM6");
	//lab();
	run();
	panel.disconnect();
	Sleep(1000);
	return 0;
}