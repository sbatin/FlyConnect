// FlyConnect.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Panel.h"
#include "Joystick.h"
#include "Events.h"
#include "NgxInterface.h"

Panel panel = Panel();
RadioPanel radio = RadioPanel();
Overhead overhead = Overhead();
JoystickTQ throttle = JoystickTQ();

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
		panel.mcpData->speedCrsL = DISP_OFF_MASK;
		panel.mcpData->vspeedCrsR = DISP_OFF_MASK;
		panel.mcpData->altitudeHdg = DISP_OFF_MASK;
	}

	panel.mcpData->alt_hld = state->MCP_annunALT_HOLD;
	panel.mcpData->app = state->MCP_annunAPP;
	panel.mcpData->at_arm = state->MCP_ATArmSw;
	panel.mcpData->cmd_a = state->MCP_annunCMD_A;
	panel.mcpData->cmd_b = state->MCP_annunCMD_B;
	panel.mcpData->cws_a = state->MCP_annunCWS_A;
	panel.mcpData->cws_b = state->MCP_annunCWS_B;
	panel.mcpData->fd_ca = state->MCP_annunFD[0];
	panel.mcpData->fd_fo = state->MCP_annunFD[1];
	panel.mcpData->hdg_sel = state->MCP_annunHDG_SEL;
	panel.mcpData->lnav = state->MCP_annunLNAV;
	panel.mcpData->lvl_chg = state->MCP_annunLVL_CHG;
	panel.mcpData->n1 = state->MCP_annunN1;
	panel.mcpData->speed = state->MCP_annunSPEED;
	panel.mcpData->vnav = state->MCP_annunVNAV;
	panel.mcpData->vor_loc = state->MCP_annunVOR_LOC;
	panel.mcpData->vs = state->MCP_annunVS;

	panel.mipData->backlight = state->LTS_MainPanelKnob[0];
	panel.mipData->flaps = getGaugeValue(state->MAIN_TEFlapsNeedle[0]);
	panel.mipData->annunNGearGrn = state->MAIN_annunGEAR_locked[1];
	panel.mipData->annunNGearRed = state->MAIN_annunGEAR_transit[1];
	panel.mipData->annunRGearGrn = state->MAIN_annunGEAR_locked[0];
	panel.mipData->annunRGearRed = state->MAIN_annunGEAR_transit[0];
	panel.mipData->annunLGearGrn = state->MAIN_annunGEAR_locked[2];
	panel.mipData->annunLGearRed = state->MAIN_annunGEAR_transit[2];
	panel.mipData->annunAntiskidInop = state->MAIN_annunANTI_SKID_INOP;
	panel.mipData->annunAutobreakDisarm = state->MAIN_annunAUTO_BRAKE_DISARM;
	panel.mipData->annunFlapsExt = state->MAIN_annunLE_FLAPS_EXT;
	panel.mipData->annunFlapsTransit = state->MAIN_annunLE_FLAPS_TRANSIT;
	panel.mipData->annunStabOutOfTrim = state->MAIN_annunSTAB_OUT_OF_TRIM;
	panel.mipData->annunAPRstRed = state->MAIN_annunAP[0];
	panel.mipData->annunAPRstYel = state->MAIN_annunAP_Amber[0];
	panel.mipData->annunATRstRed = state->MAIN_annunAT[0];
	panel.mipData->annunATRstYel = state->MAIN_annunAT_Amber[0];
	panel.mipData->annunFMCRstYel = state->MAIN_annunFMC[0];
	panel.mipData->annunSpeedbrakeArmed = state->MAIN_annunSPEEDBRAKE_ARMED;
	panel.mipData->annunSpeedbrakNotArm = state->MAIN_annunSPEEDBRAKE_DO_NOT_ARM;
	panel.mipData->annunBelowGS = state->MAIN_annunBELOW_GS[0];
	panel.mipData->annunFireWarning = state->WARN_annunFIRE_WARN[0];
	panel.mipData->annunMasterCaution = state->WARN_annunMASTER_CAUTION[0];
	panel.mipData->annunWarnFltCont = state->WARN_annunFLT_CONT;
	panel.mipData->annunWarnElec = state->WARN_annunELEC;
	panel.mipData->annunWarnIRS = state->WARN_annunIRS;
	panel.mipData->annunWarnAPU = state->WARN_annunAPU;
	panel.mipData->annunWarnFuel = state->WARN_annunFUEL;
	panel.mipData->annunWarnOvht = state->WARN_annunOVHT_DET;

	// test
	if (state->MAIN_LightsSelector == 0 && state->ELEC_BatSelector > 0) {
		panel.lightsTest();
	// bright
	} else if (state->MAIN_LightsSelector == 1) {
		panel.mcpData->brightness = 8;
	// dim
	} else if (state->MAIN_LightsSelector == 2) {
		panel.mcpData->brightness = 5;
	}

	panel.send();
}

unsigned short round_s(double f) {
	return (unsigned short)round(f);
}

unsigned short ngx_ADF1_StandBy = 3300;

void ngx_ADF1_Change(char value) {
	ngx_ADF1_StandBy += value;
	if (ngx_ADF1_StandBy > RadioPanel::adfMax) {
		ngx_ADF1_StandBy = RadioPanel::adfMin;
	}
	if (ngx_ADF1_StandBy < RadioPanel::adfMin) {
		ngx_ADF1_StandBy = RadioPanel::adfMax;
	}
}

void run() {
	auto ngx = new NgxInterface();
	ngx->connect();

	while (ngx->connected) {
		sendNGX_PanelState(&ngx->data);

		radio.data.com1.active = round_s(ngx->radio.COM1_Active * 100.0);
		radio.data.com1.standby = round_s(ngx->radio.COM1_StandBy * 100.0);
		radio.data.com2.active = round_s(ngx->radio.COM2_Active * 100.0);
		radio.data.com2.standby = round_s(ngx->radio.COM2_StandBy * 100.0);
		radio.data.nav1.active = round_s(ngx->radio.NAV1_Active * 100.0);
		radio.data.nav1.standby = round_s(ngx->radio.NAV1_StandBy * 100.0);
		radio.data.nav2.active = round_s(ngx->radio.NAV2_Active * 100.0);
		radio.data.nav2.standby = round_s(ngx->radio.NAV2_StandBy * 100.0);
		radio.data.adf1.active = round_s(ngx->radio.ADF1_Active * 10.0);
		radio.data.adf1.standby = ngx_ADF1_StandBy;
		radio.data.atc1 = round_s(ngx->radio.Transponder);
		radio.data.brk1 = round_s(ngx->radio.ParkingBrake);
		radio.update();
		ngx->requestRadioData();

		if (radio.read()) {
			auto freqSelected = radio.ctrl.freqSelected;

			if (radio.ctrl.freqSwap == 1) ngx->radioToggle(EVENT_COM1_RADIO_SWAP);
			if (radio.ctrl.freqSwap == 2) ngx->radioToggle(EVENT_NAV1_RADIO_SWAP);
			if (radio.ctrl.freqSwap == 3) {
				ngx->radioSet(EVENT_ADF1_RADIO_SET, ngx_ADF1_StandBy);
				ngx_ADF1_StandBy = ngx->radio.ADF1_Active * 10.0;
			}
			if (radio.ctrl.freqSwap == 4) ngx->radioToggle(EVENT_COM2_RADIO_SWAP);
			if (radio.ctrl.freqSwap == 5) ngx->radioToggle(EVENT_NAV2_RADIO_SWAP);

			switch (radio.ctrl.encIndex) {
				case RADIO_WHOLE:
					if (freqSelected == 1) ngx->radioRotate(radio.ctrl.encValue, EVENT_COM1_RADIO_WHOLE_INC, EVENT_COM1_RADIO_WHOLE_DEC);
					if (freqSelected == 2) ngx->radioRotate(radio.ctrl.encValue, EVENT_NAV1_RADIO_WHOLE_INC, EVENT_NAV1_RADIO_WHOLE_DEC);
					if (freqSelected == 3) ngx_ADF1_Change(radio.ctrl.encValue * 100);
					if (freqSelected == 4) ngx->radioRotate(radio.ctrl.encValue, EVENT_COM2_RADIO_WHOLE_INC, EVENT_COM2_RADIO_WHOLE_DEC);
					if (freqSelected == 5) ngx->radioRotate(radio.ctrl.encValue, EVENT_NAV2_RADIO_WHOLE_INC, EVENT_NAV2_RADIO_WHOLE_DEC);
					break;
				case RADIO_FRACT:
					if (freqSelected == 1) ngx->radioRotate(radio.ctrl.encValue, EVENT_COM1_RADIO_FRACT_INC, EVENT_COM1_RADIO_FRACT_DEC);
					if (freqSelected == 2) ngx->radioRotate(radio.ctrl.encValue, EVENT_NAV1_RADIO_FRACT_INC, EVENT_NAV1_RADIO_FRACT_DEC);
					if (freqSelected == 3) ngx_ADF1_Change(radio.ctrl.encValue);
					if (freqSelected == 4) ngx->radioRotate(radio.ctrl.encValue, EVENT_COM2_RADIO_FRACT_INC, EVENT_COM2_RADIO_FRACT_DEC);
					if (freqSelected == 5) ngx->radioRotate(radio.ctrl.encValue, EVENT_NAV2_RADIO_FRACT_INC, EVENT_NAV2_RADIO_FRACT_DEC);
					break;
				case RADIO_ATC_L:
					for (int i = 0; i < 64; i++) ngx->radioRotate(radio.ctrl.encValue, EVENT_ATC1_RADIO_FRACT_INC, EVENT_ATC1_RADIO_FRACT_DEC);
					break;
				case RADIO_ATC_R:
					ngx->radioRotate(radio.ctrl.encValue, EVENT_ATC1_RADIO_FRACT_INC, EVENT_ATC1_RADIO_FRACT_DEC);
					break;
			}

			ngx->send(EVT_TCAS_MODE, radio.ctrl.XPDR_Mode, ngx->data.XPDR_ModeSel);
		}

		if (overhead.read()) {
			auto input = &overhead.ctrl;

			if (input->ldg_retract_l != ngx->data.LTS_LandingLtRetractableSw[0]) {
				ngx->send(EVT_OH_LIGHTS_L_RETRACT, input->ldg_retract_l ? 2 : 0);
			}

			if (input->ldg_retract_r != ngx->data.LTS_LandingLtRetractableSw[1]) {
				ngx->send(EVT_OH_LIGHTS_R_RETRACT, input->ldg_retract_r ? 2 : 0);
			}

			ngx->send(EVT_OH_LIGHTS_L_FIXED, input->ldg_fixed_l, ngx->data.LTS_LandingLtFixedSw[0]);
			ngx->send(EVT_OH_LIGHTS_R_FIXED, input->ldg_fixed_r, ngx->data.LTS_LandingLtFixedSw[1]);
			ngx->send(EVT_OH_LIGHTS_L_TURNOFF, input->rw_turnoff_l, ngx->data.LTS_RunwayTurnoffSw[0]);
			ngx->send(EVT_OH_LIGHTS_R_TURNOFF, input->rw_turnoff_r, ngx->data.LTS_RunwayTurnoffSw[1]);
			ngx->send(EVT_OH_LIGHTS_TAXI, input->taxi_light, ngx->data.LTS_TaxiSw);
			ngx->send(EVT_OH_LIGHTS_LOGO, input->logo_light, ngx->data.LTS_LogoSw);
			ngx->send(EVT_OH_LIGHTS_ANT_COL, input->anti_collisn, ngx->data.LTS_AntiCollisionSw);
			ngx->send(EVT_OH_LIGHTS_WING, input->wing_light, ngx->data.LTS_WingSw);

			auto ovhApuSw = toSwitchState(input->apu_off, input->apu_start);
			if (ovhApuSw != ngx->data.APU_Selector) {
				ngx->send(EVT_OH_LIGHTS_APU_START, ovhApuSw);
			}

			auto ovhPosSw = toSwitchState(input->strobe_light, input->steady_light);
			if (ovhPosSw != ngx->data.LTS_PositionSw) {
				ngx->send(EVT_OH_LIGHTS_POS_STROBE, ovhPosSw);
			}

			ngx->send(EVT_OH_LIGHTS_L_ENGINE_START, input->eng_start_l, ngx->data.ENG_StartSelector[0]);
			ngx->send(EVT_OH_LIGHTS_R_ENGINE_START, input->eng_start_r, ngx->data.ENG_StartSelector[1]);
		}

		if (throttle.read()) {
			auto input = &throttle.data;

			if (input->flaps_up) {
				ngx->send(EVT_CONTROL_STAND_FLAPS_LEVER_0, MOUSE_FLAG_LEFTSINGLE);
			} else if (input->flaps_1) {
				ngx->send(EVT_CONTROL_STAND_FLAPS_LEVER_1, MOUSE_FLAG_LEFTSINGLE);
			} else if (input->flaps_2) {
				ngx->send(EVT_CONTROL_STAND_FLAPS_LEVER_2, MOUSE_FLAG_LEFTSINGLE);
			} else if (input->flaps_5) {
				ngx->send(EVT_CONTROL_STAND_FLAPS_LEVER_5, MOUSE_FLAG_LEFTSINGLE);
			} else if (input->flaps_10) {
				ngx->send(EVT_CONTROL_STAND_FLAPS_LEVER_10, MOUSE_FLAG_LEFTSINGLE);
			} else if (input->flaps_15) {
				ngx->send(EVT_CONTROL_STAND_FLAPS_LEVER_15, MOUSE_FLAG_LEFTSINGLE);
			} else if (input->flaps_25) {
				ngx->send(EVT_CONTROL_STAND_FLAPS_LEVER_25, MOUSE_FLAG_LEFTSINGLE);
			} else if (input->flaps_30) {
				ngx->send(EVT_CONTROL_STAND_FLAPS_LEVER_30, MOUSE_FLAG_LEFTSINGLE);
			}

			if (input->sw_12) {
				ngx->send(EVT_CONTROL_STAND_ENG1_START_LEVER, input->sw_12 - 1);
			}

			if (input->sw_34) {
				ngx->send(EVT_CONTROL_STAND_ENG2_START_LEVER, input->sw_34 - 1);
			}

			ngx->send(EVT_CONTROL_STAND_PARK_BRAKE_LEVER, input->brake_sw);
			ngx->send(EVT_OH_DOME_SWITCH, input->toggle_1 ? 0 : 1);
		}

		if (panel.read()) {
			auto mcpInput = panel.mcpCtrl;
			auto mipInput = panel.mipCtrl;
			
			printf(">>> Control received, MCP_Enc_Val = %d, MCP_Enc_Num = %d\n", mcpInput->value, mcpInput->encoder);

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
				case EFIS_BARO:
					ngx->adjust(EVENT_BARO_SELECTOR_L, mcpInput->value);
					break;
				case EFIS_MINS:
					ngx->adjust(EVENT_MINS_SELECTOR_L, mcpInput->value);
					break;
			}

			if (panel.input.gearLever != ngx->data.MAIN_GearLever) {
				if (panel.input.gearLever == 0) ngx->send(EVT_GEAR_LEVER, MOUSE_FLAG_RIGHTSINGLE);
				if (panel.input.gearLever == 1) ngx->send(EVT_GEAR_LEVER_OFF, MOUSE_FLAG_LEFTSINGLE);
				if (panel.input.gearLever == 2) ngx->send(EVT_GEAR_LEVER, MOUSE_FLAG_LEFTSINGLE);
			}

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
			ngx->pressButton(EVT_MCP_CWS_A_SWITCH, mcpInput->cws_a);
			ngx->pressButton(EVT_MCP_CWS_B_SWITCH, mcpInput->cws_b);
			ngx->pressButton(EVT_MCP_VS_SWITCH, mcpInput->vs);
			ngx->pressButton(EVT_MCP_ALT_INTV_SWITCH, mcpInput->alt_intv);
			ngx->pressButton(EVT_MCP_CO_SWITCH, mcpInput->speed_co);
			ngx->pressButton(EVT_MCP_SPD_INTV_SWITCH, mcpInput->spd_intv);

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
			ngx->send(EVT_EFIS_CPT_MODE, mcpInput->efisMode, ngx->data.EFIS_ModeSel[0]);
			ngx->send(EVT_EFIS_CPT_RANGE, mcpInput->efisRange, ngx->data.EFIS_RangeSel[0]);
			ngx->pressButton(EVT_EFIS_CPT_WXR, mcpInput->efisWXR);
			ngx->pressButton(EVT_EFIS_CPT_STA, mcpInput->efisSTA);
			ngx->pressButton(EVT_EFIS_CPT_WPT, mcpInput->efisWPT);
			ngx->pressButton(EVT_EFIS_CPT_ARPT, mcpInput->efisARPT);
			ngx->pressButton(EVT_EFIS_CPT_DATA, mcpInput->efisDATA);
			ngx->pressButton(EVT_EFIS_CPT_POS, mcpInput->efisPOS);
			ngx->pressButton(EVT_EFIS_CPT_TERR, mcpInput->efisTERR);
			ngx->pressButton(EVT_EFIS_CPT_WXR, mcpInput->efisWXR);
			ngx->pressButton(EVT_EFIS_CPT_FPV, mcpInput->efisFPV);
			ngx->pressButton(EVT_EFIS_CPT_MTRS, mcpInput->efisMTRS);
			ngx->pressButton(EVT_EFIS_CPT_BARO_STD, mcpInput->efisSTD);
			ngx->pressButton(EVT_EFIS_CPT_MINIMUMS_RST, mcpInput->efisRST);
		}

		Sleep(20);
	}
}

void lab() {
	display_test();
	unsigned char counter1 = 0;
	unsigned char value = 0;

	while (1) {
		if (throttle.read()) {
			printf("SW1=%d, SW2=%d, SW3=%d, BRAKE=%d, FLAPS_UP=%d\n", throttle.data.sw_12, throttle.data.sw_34, throttle.data.sw_56, throttle.data.brake_sw, throttle.data.flaps_up);
		}

		if (panel.read()) {
			printf(">>> Control received, Autobreak = %d, EFIS Range = %d, EFIS Mode = %d, Main Panel DU = %d, Lower DU = %d\n", panel.mipCtrl->autoBreak, panel.mcpCtrl->efisRange, panel.mcpCtrl->efisMode, panel.mipCtrl->mainPanelDU, panel.mipCtrl->lowerDU);
			value+= (char)panel.mcpCtrl->value;
		}

		if (radio.read()) {
			printf(">>> Control received: freq = %d; swap = %d; XPDR_Mode = %d\n", radio.ctrl.freqSelected, radio.ctrl.freqSwap, radio.ctrl.XPDR_Mode);
		}

		time_t seconds = time(NULL);

		panel.mipData->flaps = getGaugeValue(value);
		int mask = seconds % 2 ? 0xFFFFAFFF : 0xFFFFFFFF;
		panel.mcpData->speedCrsL = displayHi(value) & mask & displayLo((float)0.78);
		panel.mcpData->vspeedCrsR = 0xDEFFFFFF & displayLo(-1000);
		panel.mcpData->altitudeHdg = displayHi(counter1) & displayLo(getGaugeValue(value));
		panel.mipData->backlight = 0xFF;
		panel.mcpData->brightness = 10;
		panel.mipData->annunAntiskidInop = 1;
		panel.mipData->annunATRstRed = 1;
		panel.mipData->annunBelowGS = 1;
		panel.mipData->annunFlapsTransit = 1;
		panel.mipData->annunLGearRed = 1;
		panel.send();
		radio.data.adf1.active = 10000 + counter1;
		radio.data.brk1 = 1;
		radio.data.fail = 1;
		radio.data.atc1 = 7777;
		radio.update();
		counter1++;
		if (counter1 == 16) counter1 = 0;
		Sleep(50);
	}
}

int main() {
	throttle.connect();
	panel.connect();
	overhead.connect(L"COM6");
	radio.connect(L"\\\\.\\COM15");
	//lab();
	run();
	panel.disconnect();
	overhead.disconnect();
	return 0;
}