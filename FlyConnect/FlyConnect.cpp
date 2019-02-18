// FlyConnect.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Panel.h"
#include "Events.h"
#include "NgxInterface.h"

Panel panel = Panel();
RadioPanel radio = RadioPanel();

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
		panel.mcp.speedCrsL = DISP_OFF_MASK;
		panel.mcp.vspeedCrsR = DISP_OFF_MASK;
		panel.mcp.altitudeHdg = DISP_OFF_MASK;
	}

	panel.mcp.alt_hld = state->MCP_annunALT_HOLD;
	panel.mcp.app = state->MCP_annunAPP;
	panel.mcp.at_arm = state->MCP_ATArmSw;
	panel.mcp.cmd_a = state->MCP_annunCMD_A;
	panel.mcp.cmd_b = state->MCP_annunCMD_B;
	panel.mcp.fd_ca = state->MCP_annunFD[0];
	panel.mcp.fd_fo = state->MCP_annunFD[1];
	panel.mcp.hdg_sel = state->MCP_annunHDG_SEL;
	panel.mcp.lnav = state->MCP_annunLNAV;
	panel.mcp.lvl_chg = state->MCP_annunLVL_CHG;
	panel.mcp.n1 = state->MCP_annunN1;
	panel.mcp.speed = state->MCP_annunSPEED;
	panel.mcp.vnav = state->MCP_annunVNAV;
	panel.mcp.vor_loc = state->MCP_annunVOR_LOC;
	panel.mcp.vs = state->MCP_annunVS;

	panel.mcp.backlight = state->LTS_MainPanelKnob[0];
	panel.mip.backlight = state->LTS_MainPanelKnob[0];
	panel.mip.flaps = getGaugeValue(state->MAIN_TEFlapsNeedle[0]);
	panel.mip.annunNGearGrn = state->MAIN_annunGEAR_locked[1];
	panel.mip.annunNGearRed = state->MAIN_annunGEAR_transit[1];
	panel.mip.annunRGearGrn = state->MAIN_annunGEAR_locked[0];
	panel.mip.annunRGearRed = state->MAIN_annunGEAR_transit[0];
	panel.mip.annunLGearGrn = state->MAIN_annunGEAR_locked[2];
	panel.mip.annunLGearRed = state->MAIN_annunGEAR_transit[2];
	panel.mip.annunAntiskidInop = state->MAIN_annunANTI_SKID_INOP;
	panel.mip.annunAutobreakDisarm = state->MAIN_annunAUTO_BRAKE_DISARM;
	panel.mip.annunFlapsExt = state->MAIN_annunLE_FLAPS_EXT;
	panel.mip.annunFlapsTransit = state->MAIN_annunLE_FLAPS_TRANSIT;
	panel.mip.annunStabOutOfTrim = state->MAIN_annunSTAB_OUT_OF_TRIM;
	panel.mip.annunAPRstRed = state->MAIN_annunAP[0];
	panel.mip.annunAPRstYel = state->MAIN_annunAP_Amber[0];
	panel.mip.annunATRstRed = state->MAIN_annunAT[0];
	panel.mip.annunATRstYel = state->MAIN_annunAT_Amber[0];
	panel.mip.annunFMCRstYel = state->MAIN_annunFMC[0];
	panel.mip.annunSpeedbrakeArmed = state->MAIN_annunSPEEDBRAKE_ARMED;
	panel.mip.annunSpeedbrakNotArm = state->MAIN_annunSPEEDBRAKE_DO_NOT_ARM;
	panel.mip.annunBelowGS = state->MAIN_annunBELOW_GS[0];
	panel.mip.annunFireWarning = state->WARN_annunFIRE_WARN[0];
	panel.mip.annunMasterCaution = state->WARN_annunMASTER_CAUTION[0];
	panel.mip.annunWarnFltCont = state->WARN_annunFLT_CONT;
	panel.mip.annunWarnElec = state->WARN_annunELEC;
	panel.mip.annunWarnIRS = state->WARN_annunIRS;
	panel.mip.annunWarnAPU = state->WARN_annunAPU;
	panel.mip.annunWarnFuel = state->WARN_annunFUEL;
	panel.mip.annunWarnOvht = state->WARN_annunOVHT_DET;

	// test
	if (state->MAIN_LightsSelector == 0 && state->ELEC_BatSelector > 0) {
		panel.lightsTest();
	// bright
	} else if (state->MAIN_LightsSelector == 1) {
		panel.mcp.brightness = 8;
	// dim
	} else if (state->MAIN_LightsSelector == 2) {
		panel.mcp.brightness = 5;
	}

	panel.send();
}

void run() {
	auto ngx = new NgxInterface();
	ngx->connect();

	while (ngx->connected) {
		sendNGX_PanelState(&ngx->data);

		radio.data.com1.active = round(ngx->radio.COM1_Active * 100.0);
		radio.data.com1.standby = round(ngx->radio.COM1_StandBy * 100.0);
		radio.data.com2.active = round(ngx->radio.COM2_Active * 100.0);
		radio.data.com2.standby = round(ngx->radio.COM2_StandBy * 100.0);
		radio.data.nav1.active = round(ngx->radio.NAV1_Active * 100.0);
		radio.data.nav1.standby = round(ngx->radio.NAV1_StandBy * 100.0);
		radio.data.nav2.active = round(ngx->radio.NAV2_Active * 100.0);
		radio.data.nav2.standby = round(ngx->radio.NAV2_StandBy * 100.0);
		radio.data.adf1 = round(ngx->radio.ADF1_Active * 10.0);
		radio.data.atc1 = round(ngx->radio.Transponder);
		radio.update();
		ngx->requestRadioData();

		if (radio.read()) {
			auto freqSelected = radio.ctrl.freqSelected;

			if (radio.ctrl.freqSwap == 1) ngx->radioToggle(EVENT_COM1_RADIO_SWAP);
			if (radio.ctrl.freqSwap == 2) ngx->radioToggle(EVENT_NAV1_RADIO_SWAP);
			if (radio.ctrl.freqSwap == 4) ngx->radioToggle(EVENT_COM2_RADIO_SWAP);
			if (radio.ctrl.freqSwap == 5) ngx->radioToggle(EVENT_NAV2_RADIO_SWAP);

			if (radio.ctrl.encWhole) {
				if (freqSelected == 1) ngx->radioRotate(radio.ctrl.encWhole, EVENT_COM1_RADIO_WHOLE_INC, EVENT_COM1_RADIO_WHOLE_DEC);
				if (freqSelected == 2) ngx->radioRotate(radio.ctrl.encWhole, EVENT_NAV1_RADIO_WHOLE_INC, EVENT_NAV1_RADIO_WHOLE_DEC);
				if (freqSelected == 3) ngx->radioRotate(radio.ctrl.encWhole, EVENT_ADF1_RADIO_WHOLE_INC, EVENT_ADF1_RADIO_WHOLE_DEC);
				if (freqSelected == 4) ngx->radioRotate(radio.ctrl.encWhole, EVENT_COM2_RADIO_WHOLE_INC, EVENT_COM2_RADIO_WHOLE_DEC);
				if (freqSelected == 5) ngx->radioRotate(radio.ctrl.encWhole, EVENT_NAV2_RADIO_WHOLE_INC, EVENT_NAV2_RADIO_WHOLE_DEC);
				//if (freqSelected == 6) sendLinearIncDecEvent(hSimConnect, ctrl.enc2, EVENT_ATC1_RADIO_WHOLE_INC, EVENT_ATC1_RADIO_WHOLE_DEC);
				if (freqSelected == 6) for (int i = 0; i < 64; i++) ngx->radioRotate(radio.ctrl.encWhole, EVENT_ATC1_RADIO_FRACT_INC, EVENT_ATC1_RADIO_FRACT_DEC);
			}

			if (radio.ctrl.encFract) {
				if (freqSelected == 1) ngx->radioRotate(radio.ctrl.encFract, EVENT_COM1_RADIO_FRACT_INC, EVENT_COM1_RADIO_FRACT_DEC);
				if (freqSelected == 2) ngx->radioRotate(radio.ctrl.encFract, EVENT_NAV1_RADIO_FRACT_INC, EVENT_NAV1_RADIO_FRACT_DEC);
				if (freqSelected == 3) ngx->radioRotate(radio.ctrl.encFract, EVENT_ADF1_RADIO_FRACT_INC, EVENT_ADF1_RADIO_FRACT_DEC);
				if (freqSelected == 4) ngx->radioRotate(radio.ctrl.encFract, EVENT_COM2_RADIO_FRACT_INC, EVENT_COM2_RADIO_FRACT_DEC);
				if (freqSelected == 5) ngx->radioRotate(radio.ctrl.encFract, EVENT_NAV2_RADIO_FRACT_INC, EVENT_NAV2_RADIO_FRACT_DEC);
				if (freqSelected == 6) ngx->radioRotate(radio.ctrl.encFract, EVENT_ATC1_RADIO_FRACT_INC, EVENT_ATC1_RADIO_FRACT_DEC);
			}

			ngx->send(EVT_TCAS_MODE, radio.XPDR_ModeSel, ngx->data.XPDR_ModeSel);
		}

		if (panel.read()) {
			auto mcpInput = &panel.input.mcp;
			auto mipInput = &panel.input.mip;
			auto overhead = &panel.input.overhead;
			printf(">>> Control received, MCP_Enc = %d, MINS = %d\n", mcpInput->value, mipInput->efisMins);

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
				case MCP_VERT_SPEED:
					ngx->adjust(EVENT_VERTSPEED_SELECTOR, mcpInput->value);
					break;
			}

			if (mipInput->gearUP && ngx->data.MAIN_GearLever != 0) ngx->send(EVT_GEAR_LEVER, MOUSE_FLAG_RIGHTSINGLE);
			if (mipInput->gearDN && ngx->data.MAIN_GearLever != 2) ngx->send(EVT_GEAR_LEVER, MOUSE_FLAG_LEFTSINGLE);
			if (!mipInput->gearUP && !mipInput->gearDN && ngx->data.MAIN_GearLever != 1) ngx->send(EVT_GEAR_LEVER_OFF, MOUSE_FLAG_LEFTSINGLE);

			// MCP
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
			ngx->pressButton(EVT_MPM_MFD_ENG_BUTTON, mipInput->mfdENG);
			ngx->pressButton(EVT_MPM_MFD_SYS_BUTTON, mipInput->mfdSYS);
			ngx->pressButton(EVT_DSP_CPT_AP_RESET_SWITCH, mipInput->afdsRstAP);
			ngx->pressButton(EVT_DSP_CPT_AT_RESET_SWITCH, mipInput->afdsRstAT);
			ngx->pressButton(EVT_DSP_CPT_FMC_RESET_SWITCH, mipInput->afdsRstFMC);
			ngx->send(EVT_MPM_AUTOBRAKE_SELECTOR, mipInput->autoBreak, ngx->data.MAIN_AutobrakeSelector);
			ngx->send(EVT_EFIS_CPT_VOR_ADF_SELECTOR_L, panel.input.vorAdfSel1, ngx->data.EFIS_VORADFSel1[0]);
			ngx->send(EVT_EFIS_CPT_VOR_ADF_SELECTOR_R, panel.input.vorAdfSel2, ngx->data.EFIS_VORADFSel2[0]);
			ngx->send(EVT_DSP_CPT_MASTER_LIGHTS_SWITCH, panel.input.mainLights, ngx->data.MAIN_LightsSelector);
			ngx->send(EVT_DSP_CPT_DISENGAGE_TEST_SWITCH, panel.input.disengageLights, ngx->data.MAIN_DisengageTestSelector[0]);
			ngx->send(EVT_MPM_FUEL_FLOW_SWITCH, panel.input.fuelFlowSw, ngx->data.MAIN_FuelFlowSelector);
			ngx->send(EVT_DSP_CPT_MAIN_DU_SELECTOR, mipInput->mainPanelDU, ngx->data.MAIN_MainPanelDUSel[0]);
			ngx->send(EVT_DSP_CPT_LOWER_DU_SELECTOR, mipInput->lowerDU, ngx->data.MAIN_LowerDUSel[0]);
			ngx->pressButton(EVT_MASTER_CAUTION_LIGHT_LEFT, mipInput->masterCautn);
			ngx->pressButton(EVT_FIRE_WARN_LIGHT_LEFT, mipInput->fireWarning);
			ngx->pressButton(EVT_SYSTEM_ANNUNCIATOR_PANEL_LEFT, mipInput->annunRecall);

			// EFIS
			ngx->adjust(EVENT_BARO_SELECTOR_L, mipInput->efisBaro * (-1));
			ngx->adjust(EVENT_MINS_SELECTOR_L, mipInput->efisMins * (-1));
			ngx->send(EVT_EFIS_CPT_MODE, panel.input.efisMode, ngx->data.EFIS_ModeSel[0]);
			ngx->send(EVT_EFIS_CPT_RANGE, mipInput->efisRange, ngx->data.EFIS_RangeSel[0]);
			ngx->pressButton(EVT_EFIS_CPT_WXR, mipInput->efisWXR);
			ngx->pressButton(EVT_EFIS_CPT_STA, mipInput->efisSTA);
			ngx->pressButton(EVT_EFIS_CPT_WPT, mipInput->efisWPT);
			ngx->pressButton(EVT_EFIS_CPT_ARPT, mipInput->efisARPT);
			ngx->pressButton(EVT_EFIS_CPT_DATA, mipInput->efisDATA);
			ngx->pressButton(EVT_EFIS_CPT_POS, mipInput->efisPOS);
			ngx->pressButton(EVT_EFIS_CPT_TERR, mipInput->efisTERR);
			ngx->pressButton(EVT_EFIS_CPT_WXR, mipInput->efisWXR);
			ngx->pressButton(EVT_EFIS_CPT_FPV, mipInput->efisFPV);
			ngx->pressButton(EVT_EFIS_CPT_MTRS, mipInput->efisMTRS);
			ngx->pressButton(EVT_EFIS_CPT_BARO_STD, mipInput->efisSTD);
			ngx->pressButton(EVT_EFIS_CPT_MINIMUMS_RST, mipInput->efisRST);

			// Overhead
			if (overhead->ldg_retract_l != ngx->data.LTS_LandingLtRetractableSw[0]) {
				ngx->send(EVT_OH_LIGHTS_L_RETRACT, overhead->ldg_retract_l ? 2 : 0);
			}

			if (overhead->ldg_retract_r != ngx->data.LTS_LandingLtRetractableSw[1]) {
				ngx->send(EVT_OH_LIGHTS_R_RETRACT, overhead->ldg_retract_r ? 2 : 0);
			}

			ngx->send(EVT_OH_LIGHTS_L_FIXED, overhead->ldg_fixed_l, ngx->data.LTS_LandingLtFixedSw[0]);
			ngx->send(EVT_OH_LIGHTS_R_FIXED, overhead->ldg_fixed_r, ngx->data.LTS_LandingLtFixedSw[1]);
			ngx->send(EVT_OH_LIGHTS_L_TURNOFF, overhead->rw_turnoff_l, ngx->data.LTS_RunwayTurnoffSw[0]);
			ngx->send(EVT_OH_LIGHTS_R_TURNOFF, overhead->rw_turnoff_r, ngx->data.LTS_RunwayTurnoffSw[1]);
			ngx->send(EVT_OH_LIGHTS_TAXI, overhead->taxi_light, ngx->data.LTS_TaxiSw);
			ngx->send(EVT_OH_LIGHTS_LOGO, overhead->logo_light, ngx->data.LTS_LogoSw);
			ngx->send(EVT_OH_LIGHTS_ANT_COL, overhead->anti_collisn, ngx->data.LTS_AntiCollisionSw);
			ngx->send(EVT_OH_LIGHTS_WING, overhead->wing_light, ngx->data.LTS_WingSw);

			auto ovhApuSw = toSwitchState(overhead->apu_off, overhead->apu_start);
			if (ovhApuSw != ngx->data.APU_Selector) {
				ngx->send(EVT_OH_LIGHTS_APU_START, ovhApuSw);
			}

			auto ovhPosSw = toSwitchState(overhead->strobe_light, overhead->steady_light);
			if (ovhPosSw != ngx->data.LTS_PositionSw) {
				ngx->send(EVT_OH_LIGHTS_POS_STROBE, ovhPosSw);
			}

			ngx->send(EVT_OH_LIGHTS_L_ENGINE_START, overhead->eng_start_l, ngx->data.ENG_StartSelector[0]);
			ngx->send(EVT_OH_LIGHTS_R_ENGINE_START, overhead->eng_start_r, ngx->data.ENG_StartSelector[1]);
		}

		Sleep(50);
	}
}

void lab() {
	display_test();
	unsigned char counter1 = 0;
	unsigned char value = 0;

	while (1) {
		if (panel.read()) {
			auto ctrl = &panel.input;
			printf(">>> Control received, Autobreak = %d, EFIS Range = %d, EFIS Mode = %d, Main Panel DU = %d, Lower DU = %d\n", ctrl->mip.autoBreak, ctrl->mip.efisRange, ctrl->efisMode, ctrl->mip.mainPanelDU, ctrl->mip.lowerDU);
			value+= (char)ctrl->mcp.value;
		}

		time_t seconds = time(NULL);

		panel.mip.flaps = getGaugeValue(value);
		int mask = seconds % 2 ? 0xFFFFAFFF : 0xFFFFFFFF;
		panel.mcp.speedCrsL = displayHi(value) & mask & displayLo((float)0.78);
		panel.mcp.vspeedCrsR = 0xDEFFFFFF & displayLo(-1000);
		panel.mcp.altitudeHdg = displayHi(counter1) & displayLo(getGaugeValue(value));
		panel.mcp.backlight = 0xFF;
		panel.mip.backlight = 0xFF;
		panel.mip.annunAntiskidInop = 1;
		panel.mip.annunATRstRed = 1;
		panel.mip.annunBelowGS = 1;
		panel.mip.annunFlapsTransit = 1;
		panel.mip.annunLGearRed = 1;
		panel.send();
		counter1++;
		if (counter1 == 16) counter1 = 0;
		Sleep(200);
	}
}

void testRadio() {
	radio.data.com1.active = 11800;
	radio.data.com1.standby = 13697;
	radio.data.com2.active = 13697;
	radio.data.com2.standby = 11800;
	radio.data.nav1.active = 10800;
	radio.data.nav1.standby = 11795;
	radio.data.nav2.active = 11795;
	radio.data.nav2.standby = 10800;
	radio.data.adf1 = 2660;
	radio.data.atc1 = 1200;
	radio.update();

	while (1) {
		if (radio.read()) {
			printf(">>> Control received: freq = %d; swap = %d; buttons = %x\n", radio.ctrl.freqSelected, radio.ctrl.freqSwap, radio.ctrl.buttons);
		}
		Sleep(100);
	}
}

int main() {
	panel.connect(L"COM6", L"COM9", L"COM5");
	radio.connect(L"\\\\.\\COM14");
	//lab();
	//testRadio();
	run();
	panel.disconnect();
	return 0;
}

struct mip_data_test {
	unsigned short flaps;
	unsigned short reserved;
	unsigned short value;
	unsigned char backlight;
};

struct mip_ctrl_test {
	unsigned short value;
	unsigned short _mipLo;
	unsigned short _efisHi;
	unsigned short _efisLo;
	unsigned short _efisEnc;
	unsigned char mipEncoders[2];
	unsigned char mipSpdRefSw;
};

int main2() {
	radio_data_t mipData;
	radio_ctrl_t mipCtrl;
	SerialPort mipPort;
	mipPort.connect(L"COM7", 76800);
	mipData.nav1.active = 11730;
	mipData.nav1.standby = 11280;
	mipData.atc1 = 7856;

	unsigned char counter1 = 0;
	unsigned char value = 0;

	while (1) {
		//auto ch = getchar();
		//printf(">>> counter1 = %d\n", counter1);

		/*if (mipPort.readData(&mipCtrl)) {
			printf(">>> Control received %x; encoders = %d, %d; SPD REF = %x\n", mipCtrl.value, mipCtrl.mipEncoders[0], mipCtrl.mipEncoders[1], mipCtrl.mipSpdRefSw);
			for (int i = 0; i < 16; i++) {
				if (mipCtrl.value & (1 << i)) {
					printf(">>> bit %d\n", i);
				}
			}
		}*/

		//mipData.value = (1 << counter1);
		mipPort.sendData(&mipData);
		counter1++;
		if (counter1 == 16) counter1 = 0;
		Sleep(100);
	}

	return 0;
}