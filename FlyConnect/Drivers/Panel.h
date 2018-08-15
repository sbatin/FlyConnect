//  Copyright (c) 2018 Sergey Batin. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#pragma once

#include "Types.h"
#include "Display.h"
#include "SerialPort.h"

// FLAPS40 = 810 (4/3 circle)
float flapsValues[] = { 0, 1, 2, 5, 10, 15, 25, 30, 40 };
float gaugeValues[] = { 0, 105, 227, 338, 455, 540, 627, 718, 810 };

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

template <typename T>
static unsigned char toButtonState(T buttonsState, T mask0, T mask2) {
	if ((buttonsState & mask0) == 0) return 0;
	if ((buttonsState & mask2) == 0) return 2;
	return 1;
}

template <typename T>
static unsigned char decodeRotaryState(T buttonState, T mask, unsigned char maxValue) {
	unsigned char result = 1;

	while (result <= maxValue) {
		if (buttonState & mask) return result;
		result++;
		mask = mask << 1;
	}

	return 0;
}

static void connectPort(SerialPort* port, const wchar_t* path, const char* name) {
	printf("%s connecting...\n", name);
	if (port->connect(path)) {
		char *message = port->readMessage();
		printf("%s connected: %s\n", name, message);
	}
}

enum DisplayState { Blank = 0, Enabled = 1, Overspeed = 2, Underspeed = 3 };

struct PanelInput {
	struct mcp_ctrl_t mcp;
	struct mip_ctrl_t mip;
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

class Panel {
private:
	SerialPort mip;
	SerialPort mcp;
public:
	struct mcp_data_t mcpData;
	struct mip_data_t mipData;
	struct PanelInput input;

	Panel(void) {};
	~Panel(void) {};

	void connect(const wchar_t* mcpPort, const wchar_t* mipPort) {
		connectPort(&mcp, mcpPort, "MCP");
		connectPort(&mip, mipPort, "MIP");
	}

	void disconnect() {
		mip.close();
		mcp.close();
	}

	void lightsTest() {
		auto seconds = time(NULL);

		mcpData.speedCrsL = seconds % 3 == 0 ? 0x888F8888 : DISP_OFF_MASK;
		mcpData.vspeedCrsR = seconds % 3 == 0 ? 0x888C8880 : DISP_OFF_MASK;
		mcpData.altitudeHdg = seconds % 3 == 0 ? 0x88888880 : DISP_OFF_MASK;
		mcpData.alt_hld = 1;
		mcpData.app = 1;
		mcpData.at_arm = 1;
		mcpData.cmd_a = 1;
		mcpData.cmd_b = 1;
		mcpData.fd_ca = 1;
		mcpData.fd_fo = 1;
		mcpData.hdg_sel = 1;
		mcpData.lnav = 1;
		mcpData.lvl_chg = 1;
		mcpData.n1 = 1;
		mcpData.speed = 1;
		mcpData.vnav = 1;
		mcpData.vor_loc = 1;
		mcpData.vs = 1;
		mipData.annunNGearGrn = 1;
		mipData.annunNGearRed = 1;
		mipData.annunRGearGrn = 1;
		mipData.annunRGearRed = 1;
		mipData.annunLGearGrn = 1;
		mipData.annunLGearRed = 1;
		mipData.annunAntiskidInop = 1;
		mipData.annunAutobreakDisarm = 1;
		mipData.annunFlapsExt = 1;
		mipData.annunFlapsTransit = 1;
		mipData.annunStabOutOfTrim = 1;
		mipData.annunBelowGS = 1;
		mipData.annunSpeedbrakeArmed = 1;
		mipData.annunSpeedbrakNotArm = 1;
	}

	void setMCPDisplays(unsigned short courseL, float IASKtsMach, DisplayState IASState, unsigned short heading, unsigned short altitude, short vertSpeed, bool vsEnabled, unsigned short courseR) {
		mcpData.speedCrsL = displayHi(courseL);
		mcpData.vspeedCrsR = displayHi(courseR);
		mcpData.altitudeHdg = displayHi(heading) & displayLo(altitude);

		if (IASState != Blank) {
			if (IASState == Overspeed) {
				mcpData.speedCrsL &= 0xFFFFBFFF;
			}

			if (IASState == Underspeed) {
				mcpData.speedCrsL &= 0xFFFFAFFF;
			}

			if (IASKtsMach < 10) {
				mcpData.speedCrsL &= displayLo(IASKtsMach);
			} else {
				mcpData.speedCrsL &= displayLo((int)IASKtsMach);
			}
		}

		if (vsEnabled) {
			mcpData.vspeedCrsR &= displayLo(vertSpeed, 0xFFFF0000);
		}
	}

	void send() {
		mcp.sendData(&mcpData);
		mip.sendData(&mipData);
	}

	bool read() {
		bool result = false;

		input.mip.baro = 0;
		input.mip.mins = 0;
		input.mip.mipButtons = 0;
		input.mip.efisButtons = 0;

		if (mcp.readData(&input.mcp)) {
			result = true;
		}

		if (mip.readData(&input.mip)) {
			if (input.mip.mipButtons & BTN_AB_RTO)
				input.autoBreak = 0;
			else if (input.mip.mipButtons & BTN_AB_1)
				input.autoBreak = 2;
			else if (input.mip.mipButtons & BTN_AB_2)
				input.autoBreak = 3;
			else if (input.mip.mipButtons & BTN_AB_3)
				input.autoBreak = 4;
			else if (input.mip.mipButtons & BTN_AB_MAX)
				input.autoBreak = 5;
			else
				input.autoBreak = 1;

			if (input.mip.mipButtons & BTN_MAIN_DU_OUTBD)
				input.mainPanelDuSel = 0;
			else if (input.mip.mipButtons & BTN_MAIN_DU_ENG)
				input.mainPanelDuSel = 2;
			else if (input.mip.mipButtons & BTN_MAIN_DU_PFD)
				input.mainPanelDuSel = 3;
			else if (input.mip.mipButtons & BTN_MAIN_DU_MFD)
				input.mainPanelDuSel = 4;
			else
				input.mainPanelDuSel = 1;

			if (input.mip.mipButtons & BTN_LOW_DU_ENG)
				input.loweDuSel = 0;
			else if (input.mip.mipButtons & BTN_LOW_DU_ND)
				input.loweDuSel = 2;
			else
				input.loweDuSel = 1;

			if (input.mip.efisButtons & EFIS_VOR)
				input.efisMode = 1;
			else if (input.mip.efisButtons & EFIS_MAP)
				input.efisMode = 2;
			else if (input.mip.efisButtons & EFIS_PLN)
				input.efisMode = 3;
			else
				input.efisMode = 0;

			input.efisRange = decodeRotaryState<unsigned long>(input.mip.efisButtons, EFIS_10, 7);
			input.fuelFlowSw = toButtonState<unsigned long>(input.mip.mipButtons, BTN_FF_RST, BTN_FF_USED);
			input.vorAdfSel1 = toButtonState<unsigned long>(input.mip.efisButtons, EFIS_VOR1, EFIS_ADF1);
			input.vorAdfSel2 = toButtonState<unsigned long>(input.mip.efisButtons, EFIS_VOR2, EFIS_ADF2);
			input.mainLights = toButtonState<unsigned long>(input.mip.mipButtons, BTN_LS_TEST, BTN_LS_DIM);
			input.disengageLights = toButtonState<unsigned long>(input.mip.mipButtons, BTN_TEST_1, BTN_TEST_2);

			result = true;
		}

		return result;
	}
};