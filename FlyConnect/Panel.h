#include <time.h>
#include "SerialPort.h"
#include "Helpers.h"
#include "Display.h"

SerialPort mip = SerialPort();
SerialPort mcp = SerialPort();

struct mip_data_t {
	unsigned char annunFireWarning     : 1;
	unsigned char /* reserved bits */  : 7;
	unsigned char annunWarnFltCont     : 1;
	unsigned char annunWarnElec        : 1;
	unsigned char annunWarnIRS         : 1;
	unsigned char annunWarnAPU         : 1;
	unsigned char annunWarnFuel        : 1;
	unsigned char annunWarnOvht        : 1;
	unsigned char annunMasterCaution   : 1;
	unsigned char /* reserved bits */  : 1;
	unsigned short flaps;
	unsigned char annunNGearRed        : 1;
	unsigned char annunNGearGrn        : 1;
	unsigned char annunRGearRed        : 1;
	unsigned char annunRGearGrn        : 1;
	unsigned char annunLGearRed        : 1;
	unsigned char annunLGearGrn        : 1;
	unsigned char /* reserved bits */  : 2;
	unsigned char annunFlapsTransit    : 1;
	unsigned char annunFlapsExt        : 1;
	unsigned char annunAutobreakDisarm : 1;
	unsigned char annunAntiskidInop    : 1;
	unsigned char annunStabOutOfTrim   : 1;
	unsigned char /* reserved bits */  : 3;
	unsigned char annunBelowGS         : 1;
	unsigned char annunAPRstYel        : 1;
	unsigned char annunAPRstRed        : 1;
	unsigned char annunATRstYel        : 1;
	unsigned char annunATRstRed        : 1;
	unsigned char annunFMCRstYel       : 1;
	unsigned char annunSpeedbrakNotArm : 1;
	unsigned char annunSpeedbrakeArmed : 1;
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

void lightsTest() {
	auto seconds = time(NULL);

	dataToSend.mcp.speedCrsL = seconds % 3 == 0 ? 0x888F8888 : DISP_OFF_MASK;
	dataToSend.mcp.vspeedCrsR = seconds % 3 == 0 ? 0x888C8880 : DISP_OFF_MASK;
	dataToSend.mcp.altitudeHdg = seconds % 3 == 0 ? 0x88888880 : DISP_OFF_MASK;
	dataToSend.mcp.buttons = 0xFFFF;
	dataToSend.mip.annunNGearGrn = 1;
	dataToSend.mip.annunNGearRed = 1;
	dataToSend.mip.annunRGearGrn = 1;
	dataToSend.mip.annunRGearRed = 1;
	dataToSend.mip.annunLGearGrn = 1;
	dataToSend.mip.annunLGearRed = 1;
	dataToSend.mip.annunAntiskidInop = 1;
	dataToSend.mip.annunAutobreakDisarm = 1;
	dataToSend.mip.annunFlapsExt = 1;
	dataToSend.mip.annunFlapsTransit = 1;
	dataToSend.mip.annunStabOutOfTrim = 1;
	dataToSend.mip.annunBelowGS = 1;
	dataToSend.mip.annunSpeedbrakeArmed = 1;
	dataToSend.mip.annunSpeedbrakNotArm = 1;
}

enum DisplayState { Blank = 0, Enabled = 1, Overspeed = 2, Underspeed = 3 };

void setMCPDisplays(unsigned short courseL, float IASKtsMach, DisplayState IASState, unsigned short heading, unsigned short altitude, short vertSpeed, bool vsEnabled, unsigned short courseR) {
	dataToSend.mcp.speedCrsL = displayHi(courseL);
	dataToSend.mcp.vspeedCrsR = displayHi(courseR);
	dataToSend.mcp.altitudeHdg = displayHi(heading) & displayLo(altitude);

	if (IASState != Blank) {
		if (IASState == Overspeed) {
			dataToSend.mcp.speedCrsL &= 0xFFFFBFFF;
		}

		if (IASState == Underspeed) {
			dataToSend.mcp.speedCrsL &= 0xFFFFAFFF;
		}

		if (IASKtsMach < 10) {
			dataToSend.mcp.speedCrsL &= displayLo(IASKtsMach);
		} else {
			dataToSend.mcp.speedCrsL &= displayLo((int)IASKtsMach);
		}
	}

	if (vsEnabled) {
		dataToSend.mcp.vspeedCrsR &= displayLo(vertSpeed, 0xFFFF0000);
	}
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

		setMCPDisplays(state->MCP_Course[0], state->MCP_IASMach, IASState, state->MCP_Heading, state->MCP_Altitude, state->MCP_VertSpeed, !state->MCP_VertSpeedBlank, state->MCP_Course[1]);
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
	
	dataToSend.mcp.backlight = state->LTS_MainPanelKnob[0];
	dataToSend.mip.backlight = state->LTS_MainPanelKnob[0];
	dataToSend.mip.flaps = getGaugeValue(state->MAIN_TEFlapsNeedle[0]);
	dataToSend.mip.annunNGearGrn = state->MAIN_annunGEAR_locked[1];
	dataToSend.mip.annunNGearRed = state->MAIN_annunGEAR_transit[1];
	dataToSend.mip.annunRGearGrn = state->MAIN_annunGEAR_locked[0];
	dataToSend.mip.annunRGearRed = state->MAIN_annunGEAR_transit[0];
	dataToSend.mip.annunLGearGrn = state->MAIN_annunGEAR_locked[2];
	dataToSend.mip.annunLGearRed = state->MAIN_annunGEAR_transit[2];
	dataToSend.mip.annunAntiskidInop = state->MAIN_annunANTI_SKID_INOP;
	dataToSend.mip.annunAutobreakDisarm = state->MAIN_annunAUTO_BRAKE_DISARM;
	dataToSend.mip.annunFlapsExt = state->MAIN_annunLE_FLAPS_EXT;
	dataToSend.mip.annunFlapsTransit = state->MAIN_annunLE_FLAPS_TRANSIT;
	dataToSend.mip.annunStabOutOfTrim = state->MAIN_annunSTAB_OUT_OF_TRIM;
	dataToSend.mip.annunAPRstRed = state->MAIN_annunAP[0];
	dataToSend.mip.annunAPRstYel = state->MAIN_annunAP_Amber[0];
	dataToSend.mip.annunATRstRed = state->MAIN_annunAT[0];
	dataToSend.mip.annunATRstYel = state->MAIN_annunAT_Amber[0];
	dataToSend.mip.annunFMCRstYel = state->MAIN_annunFMC[0];
	dataToSend.mip.annunSpeedbrakeArmed = state->MAIN_annunSPEEDBRAKE_ARMED;
	dataToSend.mip.annunSpeedbrakNotArm = state->MAIN_annunSPEEDBRAKE_DO_NOT_ARM;
	dataToSend.mip.annunBelowGS = state->MAIN_annunBELOW_GS[0];
	dataToSend.mip.annunFireWarning = state->WARN_annunFIRE_WARN[0];
	dataToSend.mip.annunMasterCaution = state->WARN_annunMASTER_CAUTION[0];
	dataToSend.mip.annunWarnFltCont = state->WARN_annunFLT_CONT;
	dataToSend.mip.annunWarnElec = state->WARN_annunELEC;
	dataToSend.mip.annunWarnIRS = state->WARN_annunIRS;
	dataToSend.mip.annunWarnAPU = state->WARN_annunAPU;
	dataToSend.mip.annunWarnFuel = state->WARN_annunFUEL;
	dataToSend.mip.annunWarnOvht = state->WARN_annunOVHT_DET;

	// test
	if (state->MAIN_LightsSelector == 0) {
		lightsTest();
	// bright
	} else if (state->MAIN_LightsSelector == 1) {
		dataToSend.mcp.brightness = 8;
	// dim
	} else if (state->MAIN_LightsSelector == 2) {
		dataToSend.mcp.brightness = 5;
	}

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

		ctrl.mcpButtons = dataReceived.mcp.buttons;

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