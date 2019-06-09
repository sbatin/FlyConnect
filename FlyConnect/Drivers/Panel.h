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

static void connectPort(SerialPort* port, const wchar_t* path, const char* name, DWORD baudRate) {
	printf("%s connecting...\n", name);
	if (port->connect(path, baudRate)) {
		char *message = port->readMessage();
		printf("%s connected: %s\n", name, message);
	}
}

enum DisplayState { Blank = 0, Enabled = 1, Overspeed = 2, Underspeed = 3 };

struct PanelInput {
	struct mcp_ctrl_t mcp;
	struct mip_ctrl_t mip;
	struct overhead_ctrl_t overhead;
	unsigned char vorAdfSel1;
	unsigned char vorAdfSel2;
	unsigned char mainLights;
	unsigned char disengageLights;
	unsigned char fuelFlowSw;
};

class Panel {
private:
	SerialPort mcpPort;
	SerialPort ovhPort;
public:
	struct mcp_data_t mcp;
	struct mip_data_t mip;
	struct PanelInput input;

	Panel(void) {};
	~Panel(void) {};

	void connect(const wchar_t* mcpPortPath, const wchar_t* ovhPortPath) {
		if (mcpPort.connect(mcpPortPath, CBR_115200)) {
			printf("MCP connected:\n");
		}
		connectPort(&ovhPort, ovhPortPath, "OVH", CBR_4800);
	}

	void disconnect() {
		mcpPort.close();
		ovhPort.close();
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
		mcp.cws_a = 1;
		mcp.cws_b = 1;
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
		mcp.mip = mip;
		mcpPort.sendDataRaw(&mcp);
	}

	bool read() {
		bool result = false;

		input.mcp.value = 0;

		unsigned char prevMode = input.mcp.efisMode;
		unsigned char prevRange = input.mcp.efisRange;

		if (mcpPort.readDataRaw(&input.mcp)) {
			input.mip = input.mcp.mip;

			input.mcp.efisMode = input.mcp.efisMode ? decodeRotaryState(input.mcp.efisMode) - 1 : prevMode;
			input.mcp.efisRange = input.mcp.efisRange ? decodeRotaryState(input.mcp.efisRange) - 1 : prevRange;
			input.vorAdfSel1 = toSwitchState(input.mcp.efisVOR1, input.mcp.efisADF1);
			input.vorAdfSel2 = toSwitchState(input.mcp.efisVOR2, input.mcp.efisADF2);
			input.mip.autoBreak = decodeRotaryState(input.mip.autoBreak);
			input.mip.mainPanelDU = decodeRotaryState(input.mip.mainPanelDU);
			input.mip.lowerDU = decodeRotaryState(input.mip.lowerDU);
			input.fuelFlowSw = toSwitchState(input.mip.ffReset, input.mip.ffUsed);
			input.mainLights = toSwitchState(input.mip.lightsTest, input.mip.lightsDim);
			input.disengageLights = toSwitchState(input.mip.afdsTest1, input.mip.afdsTest2);

			result = true;
		}

		if (ovhPort.readData(&input.overhead)) {
			input.overhead.eng_start_l = decodeRotaryState(input.overhead.eng_start_l);
			input.overhead.eng_start_r = decodeRotaryState(input.overhead.eng_start_r);
			result = true;
		}

		return result;
	}
};

class RadioPanel {
private:
	struct radio_data_t prevData;
	SerialPort port;
	const unsigned short navMin = 10800;
	const unsigned short navMax = 11795;
	const unsigned short vhfMin = 11800;
	const unsigned short vhfMax = 13697;
public:
	struct radio_data_t data;
	struct radio_ctrl_t ctrl;
	unsigned char XPDR_ModeSel;

	void connect(const wchar_t* path) {
		data.com1.active = vhfMin;
		data.com1.standby = vhfMax;
		data.com2.active = vhfMax;
		data.com2.standby = vhfMin;
		data.nav1.active = navMin;
		data.nav1.standby = navMax;
		data.nav2.active = navMax;
		data.nav2.standby = navMin;
		data.adf1 = 1900;
		data.atc1 = 7777;

		printf("VHF connecting...\n");
		if (port.connect(path, 76800)) {
			char *message = port.readMessage();
			printf("VHF connected: %s\n", message);
		}
	}

	void update() {
		port.sendData(&data);
		/*if (memcmp(&data, &prevData, sizeof(radio_data_t))) {
			port.sendData(&data);
			memcpy(&prevData, &data, sizeof(radio_data_t));
		}*/
	}

	bool read() {
		if (port.readData(&ctrl)) {
			ctrl.encWhole = -ctrl.encWhole;

			switch (ctrl.buttons) {
				case 0x80:
					XPDR_ModeSel = 1;
					break;
				case 0x40:
					XPDR_ModeSel = 2;
					break;
				case 0x20:
					XPDR_ModeSel = 3;
					break;
				case 0x10:
					XPDR_ModeSel = 4;
					break;
				case 0x01:
					XPDR_ModeSel = 0;
					break;
				default:
					break;
			}

			return true;
		}

		return false;
	}

	RadioPanel() {
	};

	~RadioPanel(void) {
	};
};