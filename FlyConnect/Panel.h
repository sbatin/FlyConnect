#include <time.h>
#include "SerialPort.h"
#include "Helpers.h"
#include "Display.h"

SerialPort mip = SerialPort();
SerialPort mcp = SerialPort();

struct mip_data_t {
	unsigned short warnLeds;
	unsigned short flaps;
	unsigned char leds[3];
	unsigned char backlight;
};

struct mcp_data_t {
	unsigned long altitudeHdg;
	unsigned long vspeedCrsR;
	unsigned long speedCrsL;
	unsigned short buttons;
	unsigned char brightness;
	unsigned char backlight;
};

#pragma pack(push, 1)
struct mip_ctrl_t {
	unsigned long mipButtons;
	unsigned long efisButtons;
	unsigned char baro;
	unsigned char mins;
};
#pragma pack(pop)

struct mcp_ctrl_t {
	unsigned char encoder;
	unsigned char value;
	unsigned short buttons;
};

struct panel_data_t {
	struct mcp_data_t mcp;
	struct mip_data_t mip;
};

struct panel_ctrl_t {
	struct mcp_ctrl_t mcp;
	struct mip_ctrl_t mip;
};

struct panel_ctrl_t dataReceived;
struct panel_data_t dataToSend;

#define BTN_LNAV   (1 << 0)
#define BTN_VORLOC (1 << 1)
#define BTN_ALTHLD (1 << 2)
#define BTN_VS     (1 << 3)
#define BTN_CMDA   (1 << 4)
#define BTN_CMDB   (1 << 5)
#define BTN_FD_RGH (1 << 6)
// pin 7 is not connected
#define BTN_ATARM  (1 << 8)
#define BTN_FD_LFT (1 << 9)
#define BTN_N1     (1 << 10)
#define BTN_SPEED  (1 << 11)
#define BTN_VNAV   (1 << 12)
#define BTN_LVLCHG (1 << 13)
#define BTN_HDGSEL (1 << 14)
#define BTN_APP    (1 << 15)

// LEDS group 0
#define LED_NGEAR_RED  (1 << 0)
#define LED_NGEAR_GRN  (1 << 1)
#define LED_RGEAR_RED  (1 << 2)
#define LED_RGEAR_GRN  (1 << 3)
#define LED_LGEAR_RED  (1 << 4)
#define LED_LGEAR_GRN  (1 << 5)

// LEDS group 1
#define LED_FLAPS_TRANS      (1 << 0)
#define LED_FLAPS_EXT        (1 << 1)
#define LED_AUTOBREAK_DISARM (1 << 2)
#define LED_ANTISKID_INOP    (1 << 3)
#define LED_STAB_OUTOF_TRIM  (1 << 4)

// LEDS group 2
#define LED_BELOW_GS (1 << 0)
#define LED_AP_YEL   (1 << 1)
#define LED_AP_RED   (1 << 2)
#define LED_AT_YEL   (1 << 3)
#define LED_AT_RED   (1 << 4)
#define LED_FMC_YEL  (1 << 5)
#define LED_SPEEDBRAKE_NOTARM (1 << 6)
#define LED_SPEEDBRAKE_ARMED  (1 << 7)

#define LED_FIRE_WARNING (1 << 0)
#define LED_WARN_FLTCONT (1 << 8)
#define LED_WARN_ELEC    (1 << 9)
#define LED_WARN_IRS     (1 << 10)
#define LED_WARN_APU     (1 << 11)
#define LED_WARN_FUEL    (1 << 12)
#define LED_WARN_OVHT    (1 << 13)
#define LED_MSTR_CAUTION (1 << 14)

/*
*** MIP buttons mapping ***
*/

#define BTN_LOW_DU_ENG    (1 << 2)
#define BTN_LOW_DU_ND     (1 << 3)
#define BTN_MAIN_DU_MFD   (1 << 4)
#define BTN_MAIN_DU_PFD   (1 << 5)
#define BTN_MAIN_DU_ENG   (1 << 6)
#define BTN_MAIN_DU_OUTBD (1 << 7)
#define BTN_WARN_RECALL   (1 << 13)
#define BTN_MSTR_CAUTION  (1 << 14)
#define BTN_FIRW_WARNING  (1 << 15)
#define BTN_LS_DIM        (1 << 16)
#define BTN_LS_TEST       (1 << 17)
#define BTN_TEST_2        (1 << 18)
#define BTN_TEST_1        (1 << 19)
#define BTN_FMC_RST       (1 << 20)
#define BTN_AT_RST        (1 << 21)
#define BTN_AP_RST        (1 << 22)
#define BTN_FF_RST        (1 << 23)
#define BTN_AB_RTO        (1 << 24)
#define BTN_AB_1          (1 << 25)
#define BTN_AB_2          (1 << 26)
#define BTN_AB_3          (1 << 27)
#define BTN_AB_MAX        (1 << 28)
#define BTN_MFD_SYS       (1 << 29)
#define BTN_MFD_ENG       (1 << 30)
#define BTN_FF_USED       (1 << 31)

/*
*** EFIS buttons mapping ***
*/

#define EFIS_MAP  (1 << 0)
#define EFIS_VOR  (1 << 1)
#define EFIS_STD  (1 << 5)
#define EFIS_RST  (1 << 6)
#define EFIS_PLN  (1 << 7)
#define EFIS_10   (1 << 8)
#define EFIS_20   (1 << 9)
#define EFIS_40   (1 << 10)
#define EFIS_80   (1 << 11)
#define EFIS_160  (1 << 12)
#define EFIS_320  (1 << 13)
#define EFIS_640  (1 << 14)
#define EFIS_POS  (1 << 16)
#define EFIS_DATA (1 << 17)
#define EFIS_ARPT (1 << 18)
#define EFIS_VOR2 (1 << 20)
#define EFIS_ADF2 (1 << 21)
#define EFIS_MTRS (1 << 23)
#define EFIS_FPV  (1 << 24)
#define EFIS_ADF1 (1 << 26)
#define EFIS_VOR1 (1 << 27)
#define EFIS_TERR (1 << 28)
#define EFIS_WPT  (1 << 29)
#define EFIS_STA  (1 << 30)
#define EFIS_WXR  (1 << 31)

// FLAPS40 = 810 (4/3 circle)
float flapsValues[] = {0, 1, 2, 5, 10, 15, 25, 30, 40};
float gaugeValues[] = {0, 105, 227, 338, 455, 540, 627, 718, 810};

unsigned short getGaugeValue(float flapsValue) {
	for (int i = 0; i < sizeof(flapsValues); i++) {
		if (flapsValue >= flapsValues[i] && flapsValue <= flapsValues[i + 1]) {
			float k = (flapsValue - flapsValues[i]) / (flapsValues[i + 1] - flapsValues[i]);
			float d = gaugeValues[i + 1] - gaugeValues[i];
			return static_cast<unsigned short>(d*k + gaugeValues[i]);
		}
	}

	return 0;
}

void sendTestData() {
	time_t seconds = time(NULL);

	dataToSend.mcp.speedCrsL = seconds % 3 == 0 ? 0x888F8888 : DISP_OFF_MASK;
	dataToSend.mcp.vspeedCrsR = seconds % 3 == 0 ? 0x888C8880 : DISP_OFF_MASK;
	dataToSend.mcp.altitudeHdg = seconds % 3 == 0 ? 0x88888880 : DISP_OFF_MASK;

	dataToSend.mcp.buttons = 0xFFFF;
	dataToSend.mcp.brightness = 2;
	dataToSend.mcp.backlight = 0;
	dataToSend.mip.leds[0] = 0xFF;
	dataToSend.mip.leds[1] = 0xFF;
	dataToSend.mip.leds[2] = LED_SPEEDBRAKE_ARMED | LED_SPEEDBRAKE_NOTARM | LED_BELOW_GS;

	mcp.sendData(&dataToSend.mcp);
	mip.sendData(&dataToSend.mip);
}

void sendNGX_PanelState(PMDG_NGX_Data* state) {
	// test mode
	if (state->MAIN_LightsSelector == 0) {
		sendTestData();
		return;
	}

	if (state->ELEC_BatSelector) {
		dataToSend.mcp.speedCrsL = displayHi(state->MCP_Course[0]);
		dataToSend.mcp.vspeedCrsR = displayHi(state->MCP_Course[1]);
		dataToSend.mcp.altitudeHdg = displayHi(state->MCP_Heading) & displayLo(state->MCP_Altitude);

		time_t seconds = time(NULL);

		if (!state->MCP_IASBlank) {
			if (state->MCP_IASOverspeedFlash && seconds % 2 == 0) {
				dataToSend.mcp.speedCrsL &= 0xFFFFBFFF;
			}

			if (state->MCP_IASUnderspeedFlash && seconds % 2 == 0) {
				dataToSend.mcp.speedCrsL &= 0xFFFFAFFF;
			}

			if (state->MCP_IASMach < 10) {
				dataToSend.mcp.speedCrsL &= displayLo(state->MCP_IASMach);
			} else {
				dataToSend.mcp.speedCrsL &= displayLo((int)state->MCP_IASMach);
			}
		}

		if (!state->MCP_VertSpeedBlank) {
			dataToSend.mcp.vspeedCrsR &= displayLo(state->MCP_VertSpeed, 0xFFFF0000);
		}
	} else {
		dataToSend.mcp.speedCrsL = DISP_OFF_MASK;
		dataToSend.mcp.vspeedCrsR = DISP_OFF_MASK;
		dataToSend.mcp.altitudeHdg = DISP_OFF_MASK;
	}

	dataToSend.mcp.buttons =
		setBit(BTN_HDGSEL, state->MCP_annunHDG_SEL) |
		setBit(BTN_ALTHLD, state->MCP_annunALT_HOLD) |
		setBit(BTN_ATARM, state->MCP_ATArmSw) |
		setBit(BTN_APP, state->MCP_annunAPP) |
		setBit(BTN_VORLOC, state->MCP_annunVOR_LOC) |
		setBit(BTN_SPEED, state->MCP_annunSPEED) |
		setBit(BTN_FD_LFT, state->MCP_annunFD[0]) |
		setBit(BTN_FD_RGH, state->MCP_annunFD[1]) |
		setBit(BTN_CMDA, state->MCP_annunCMD_A) |
		setBit(BTN_LNAV, state->MCP_annunLNAV) |
		setBit(BTN_VNAV, state->MCP_annunVNAV) |
		setBit(BTN_LVLCHG, state->MCP_annunLVL_CHG) |
		setBit(BTN_N1, state->MCP_annunN1) |
		setBit(BTN_VS, state->MCP_annunVS) |
		setBit(BTN_CMDB, state->MCP_annunCMD_B);

	// bright
	if (state->MAIN_LightsSelector == 1) {
		dataToSend.mcp.brightness = 8;
	// dim
	} else if (state->MAIN_LightsSelector == 2) {
		dataToSend.mcp.brightness = 5;
	}
	
	dataToSend.mcp.backlight = state->LTS_MainPanelKnob[0];
	dataToSend.mip.backlight = state->LTS_MainPanelKnob[0];

	dataToSend.mip.flaps = getGaugeValue(state->MAIN_TEFlapsNeedle[0]);

	dataToSend.mip.leds[0] =
		setBit(LED_RGEAR_GRN, state->MAIN_annunGEAR_locked[0]) |
		setBit(LED_RGEAR_RED, state->MAIN_annunGEAR_transit[0]) |
		setBit(LED_NGEAR_GRN, state->MAIN_annunGEAR_locked[1]) |
		setBit(LED_NGEAR_RED, state->MAIN_annunGEAR_transit[1]) |
		setBit(LED_LGEAR_GRN, state->MAIN_annunGEAR_locked[2]) |
		setBit(LED_LGEAR_RED, state->MAIN_annunGEAR_transit[2]);

	dataToSend.mip.leds[1] =
		setBit(LED_ANTISKID_INOP, state->MAIN_annunANTI_SKID_INOP) |
		setBit(LED_AUTOBREAK_DISARM, state->MAIN_annunAUTO_BRAKE_DISARM) |
		setBit(LED_FLAPS_EXT, state->MAIN_annunLE_FLAPS_EXT) |
		setBit(LED_FLAPS_TRANS, state->MAIN_annunLE_FLAPS_TRANSIT) |
		setBit(LED_STAB_OUTOF_TRIM, state->MAIN_annunSTAB_OUT_OF_TRIM);

	dataToSend.mip.leds[2] =
		setBit(LED_AP_RED, state->MAIN_annunAP[0]) |
		setBit(LED_AT_RED, state->MAIN_annunAT[0]) |
		setBit(LED_FMC_YEL, state->MAIN_annunFMC[0]) |
		setBit(LED_AP_YEL, state->MAIN_annunAP_Amber[0]) |
		setBit(LED_AT_YEL, state->MAIN_annunAT_Amber[0]) |
		setBit(LED_SPEEDBRAKE_NOTARM, state->MAIN_annunSPEEDBRAKE_DO_NOT_ARM) |
		setBit(LED_SPEEDBRAKE_ARMED, state->MAIN_annunSPEEDBRAKE_ARMED) |
		setBit(LED_BELOW_GS, state->MAIN_annunBELOW_GS[0]);

	dataToSend.mip.warnLeds =
		setBit(LED_MSTR_CAUTION, state->WARN_annunMASTER_CAUTION[0]) |
		setBit(LED_FIRE_WARNING, state->WARN_annunFIRE_WARN[0]) |
		setBit(LED_WARN_FLTCONT, state->WARN_annunFLT_CONT) |
		setBit(LED_WARN_ELEC, state->WARN_annunELEC) |
		setBit(LED_WARN_IRS, state->WARN_annunIRS) |
		setBit(LED_WARN_APU, state->WARN_annunAPU) |
		setBit(LED_WARN_FUEL, state->WARN_annunFUEL) |
		setBit(LED_WARN_OVHT, state->WARN_annunOVHT_DET);

	mip.sendData(&dataToSend.mip);
	mcp.sendData(&dataToSend.mcp);
}

struct NGX_Control {
	char courseL;
	char courseR;
	char airspeed;
	char heading;
	char altitude;
	char efisBaro;
	char efisMins;
	unsigned short mcpButtons;
	unsigned long mipButtons;
	unsigned long efisButtons;
	unsigned char autoBreak;
	unsigned char vorAdfSel1;
	unsigned char vorAdfSel2;
	unsigned char mainLights;
	unsigned char disengageLights;
	unsigned char fuelFlowSw;
	unsigned char efisMode;
	unsigned char efisRange;
	unsigned char mainPanelDuSel;
	unsigned char loweDuSel;
};

struct NGX_Control ctrl;

NGX_Control* getControlData() {
	NGX_Control* result = NULL;

	ctrl.courseL  = 0;
	ctrl.courseR  = 0;
	ctrl.airspeed = 0;
	ctrl.heading  = 0;
	ctrl.altitude = 0;
	ctrl.efisBaro = 0;
	ctrl.efisMins = 0;
	ctrl.mcpButtons&= BTN_FD_LFT | BTN_FD_RGH | BTN_ATARM;
	ctrl.mipButtons = 0;
	ctrl.efisButtons = 0;

	if (mcp.readData(&dataReceived.mcp)) {
		switch (dataReceived.mcp.encoder) {
			case 0:
				ctrl.altitude = (char)dataReceived.mcp.value;
				break;
			case 1:
				ctrl.heading = (char)dataReceived.mcp.value;
				break;
			case 2:
				ctrl.airspeed = (char)dataReceived.mcp.value;
				break;
			case 3:
				ctrl.courseL = (char)dataReceived.mcp.value;
				break;
			case 4:
				// VERT SPEED
				break;
			case 5:
				ctrl.courseR = (char)dataReceived.mcp.value;
				break;
		}

		unsigned char hi = dataReceived.mcp.buttons >> 8;
		unsigned char lo = dataReceived.mcp.buttons & 0xFF;
		ctrl.mcpButtons = ~(reverseByte(hi) << 8 | reverseByte(lo));

		result = &ctrl;
	}

	if (mip.readData(&dataReceived.mip)) {
		ctrl.efisBaro = (char)dataReceived.mip.baro;
		ctrl.efisMins = (char)dataReceived.mip.mins;

		ctrl.efisButtons = ~dataReceived.mip.efisButtons;
		ctrl.mipButtons = ~dataReceived.mip.mipButtons;

		if (ctrl.mipButtons & BTN_AB_RTO)
			ctrl.autoBreak = 0;
		else if (ctrl.mipButtons & BTN_AB_1)
			ctrl.autoBreak = 2;
		else if (ctrl.mipButtons & BTN_AB_2)
			ctrl.autoBreak = 3;
		else if (ctrl.mipButtons & BTN_AB_3)
			ctrl.autoBreak = 4;
		else if (ctrl.mipButtons & BTN_AB_MAX)
			ctrl.autoBreak = 5;
		else
			ctrl.autoBreak = 1;

		if (ctrl.mipButtons & BTN_MAIN_DU_OUTBD)
			ctrl.mainPanelDuSel = 0;
		else if (ctrl.mipButtons & BTN_MAIN_DU_ENG)
			ctrl.mainPanelDuSel = 2;
		else if (ctrl.mipButtons & BTN_MAIN_DU_PFD)
			ctrl.mainPanelDuSel = 3;
		else if (ctrl.mipButtons & BTN_MAIN_DU_MFD)
			ctrl.mainPanelDuSel = 4;
		else
			ctrl.mainPanelDuSel = 1;

		if (ctrl.mipButtons & BTN_LOW_DU_ENG)
			ctrl.loweDuSel = 0;
		else if (ctrl.mipButtons & BTN_LOW_DU_ND)
			ctrl.loweDuSel = 2;
		else
			ctrl.loweDuSel = 1;

		if (ctrl.efisButtons & EFIS_VOR)
			ctrl.efisMode = 1;
		else if (ctrl.efisButtons & EFIS_MAP)
			ctrl.efisMode = 2;
		else if (ctrl.efisButtons & EFIS_PLN)
			ctrl.efisMode = 3;
		else
			ctrl.efisMode = 0;

		ctrl.efisRange = decodeRotaryState<unsigned long>(ctrl.efisButtons, EFIS_10, 7);
		ctrl.fuelFlowSw = toButtonState<unsigned long>(dataReceived.mip.mipButtons, BTN_FF_RST, BTN_FF_USED);
		ctrl.vorAdfSel1 = toButtonState<unsigned long>(dataReceived.mip.efisButtons, EFIS_VOR1, EFIS_ADF1);
		ctrl.vorAdfSel2 = toButtonState<unsigned long>(dataReceived.mip.efisButtons, EFIS_VOR2, EFIS_ADF2);
		ctrl.mainLights = toButtonState<unsigned long>(dataReceived.mip.mipButtons, BTN_LS_TEST, BTN_LS_DIM);
		ctrl.disengageLights = toButtonState<unsigned long>(dataReceived.mip.mipButtons, BTN_TEST_1, BTN_TEST_2);

		result = &ctrl;
	}

	return result;
}