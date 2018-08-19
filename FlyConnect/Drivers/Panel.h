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
	if (buttonsState & mask0) return 0;
	if (buttonsState & mask2) return 2;
	return 1;
}

static unsigned char toSwitchState(bool v0, bool v2) {
	if (v0) return 0;
	if (v2) return 2;
	return 1;
}

static unsigned char decodeRotaryState(unsigned char value) {
	for (int i = 0; i < 8; i++) {
		if (value & (1 << i)) {
			return i + 1;
		}
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
	unsigned char mainPanelDU;
	unsigned char lowerDU;
};

class Panel {
private:
	SerialPort mipPort;
	SerialPort mcpPort;
public:
	struct mcp_data_t mcp;
	struct mip_data_t mip;
	struct PanelInput input;

	Panel(void) {};
	~Panel(void) {};

	void connect(const wchar_t* mcpPortPath, const wchar_t* mipPortPath) {
		connectPort(&mcpPort, mcpPortPath, "MCP");
		connectPort(&mipPort, mipPortPath, "MIP");
	}

	void disconnect() {
		mipPort.close();
		mcpPort.close();
	}

	void lightsTest() {
		auto seconds = time(NULL);

		mcp.speedCrsL = seconds % 3 == 0 ? 0x888F8888 : DISP_OFF_MASK;
		mcp.vspeedCrsR = seconds % 3 == 0 ? 0x888C8880 : DISP_OFF_MASK;
		mcp.altitudeHdg = seconds % 3 == 0 ? 0x88888880 : DISP_OFF_MASK;
		mcp.alt_hld = 1;
		mcp.app = 1;
		mcp.at_arm = 1;
		mcp.cmd_a = 1;
		mcp.cmd_b = 1;
		mcp.fd_ca = 1;
		mcp.fd_fo = 1;
		mcp.hdg_sel = 1;
		mcp.lnav = 1;
		mcp.lvl_chg = 1;
		mcp.n1 = 1;
		mcp.speed = 1;
		mcp.vnav = 1;
		mcp.vor_loc = 1;
		mcp.vs = 1;
		mip.annunNGearGrn = 1;
		mip.annunNGearRed = 1;
		mip.annunRGearGrn = 1;
		mip.annunRGearRed = 1;
		mip.annunLGearGrn = 1;
		mip.annunLGearRed = 1;
		mip.annunAntiskidInop = 1;
		mip.annunAutobreakDisarm = 1;
		mip.annunFlapsExt = 1;
		mip.annunFlapsTransit = 1;
		mip.annunStabOutOfTrim = 1;
		mip.annunBelowGS = 1;
		mip.annunSpeedbrakeArmed = 1;
		mip.annunSpeedbrakNotArm = 1;
	}

	void setMCPDisplays(unsigned short courseL, float IASKtsMach, DisplayState IASState, unsigned short heading, unsigned short altitude, short vertSpeed, bool vsEnabled, unsigned short courseR) {
		mcp.speedCrsL = displayHi(courseL);
		mcp.vspeedCrsR = displayHi(courseR);
		mcp.altitudeHdg = displayHi(heading) & displayLo(altitude);

		if (IASState != Blank) {
			if (IASState == Overspeed) {
				mcp.speedCrsL &= 0xFFFFBFFF;
			}

			if (IASState == Underspeed) {
				mcp.speedCrsL &= 0xFFFFAFFF;
			}

			if (IASKtsMach < 10) {
				mcp.speedCrsL &= displayLo(IASKtsMach);
			} else {
				mcp.speedCrsL &= displayLo((int)IASKtsMach);
			}
		}

		if (vsEnabled) {
			mcp.vspeedCrsR &= displayLo(vertSpeed, 0xFFFF0000);
		}
	}

	void send() {
		mcpPort.sendData(&mcp);
		mipPort.sendData(&mip);
	}

	bool read() {
		bool result = false;

		input.mip.efisBaro = 0;
		input.mip.efisMins = 0;
		input.mip.mipButtons = 0;
		input.mip.efisButtons = 0;
		input.mcp.value = 0;

		if (mcpPort.readData(&input.mcp)) {
			result = true;
		}

		if (mipPort.readData(&input.mip)) {
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
				input.mainPanelDU = 0;
			else if (input.mip.mipButtons & BTN_MAIN_DU_ENG)
				input.mainPanelDU = 2;
			else if (input.mip.mipButtons & BTN_MAIN_DU_PFD)
				input.mainPanelDU = 3;
			else if (input.mip.mipButtons & BTN_MAIN_DU_MFD)
				input.mainPanelDU = 4;
			else
				input.mainPanelDU = 1;

			if (input.mip.mipButtons & BTN_LOW_DU_ENG)
				input.lowerDU = 0;
			else if (input.mip.mipButtons & BTN_LOW_DU_ND)
				input.lowerDU = 2;
			else
				input.lowerDU = 1;

			if (input.mip.efisButtons & EFIS_VOR)
				input.efisMode = 1;
			else if (input.mip.efisButtons & EFIS_MAP)
				input.efisMode = 2;
			else if (input.mip.efisButtons & EFIS_PLN)
				input.efisMode = 3;
			else
				input.efisMode = 0;

			input.mip.efisRange = decodeRotaryState(input.mip.efisRange);
			input.vorAdfSel1 = toSwitchState(input.mip.efisVOR1, input.mip.efisADF1);
			input.vorAdfSel2 = toSwitchState(input.mip.efisVOR2, input.mip.efisADF2);
			input.fuelFlowSw = toButtonState<unsigned long>(input.mip.mipButtons, BTN_FF_RST, BTN_FF_USED);
			input.mainLights = toButtonState<unsigned long>(input.mip.mipButtons, BTN_LS_TEST, BTN_LS_DIM);
			input.disengageLights = toButtonState<unsigned long>(input.mip.mipButtons, BTN_TEST_1, BTN_TEST_2);

			result = true;
		}

		return result;
	}
};