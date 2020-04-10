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
#include "hidapi.h"

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

enum DisplayState { Blank = 0, Enabled = 1, Overspeed = 2, Underspeed = 3 };

struct PanelInput {
	unsigned char vorAdfSel1;
	unsigned char vorAdfSel2;
	unsigned char mainLights;
	unsigned char disengageLights;
	unsigned char fuelFlowSw;
	unsigned char gearLever;
};

#pragma pack(push, 1)
struct usb_data_t {
	unsigned char report_id;
	mcp_data_t mcp;
	mip_data_t mip;
};

struct usb_ctrl_t {
	unsigned char report_id;
	mcp_ctrl_t mcp;
	mip_ctrl_t mip;
};
#pragma pack(pop)

#define MAX_STR 255

class Panel {
private:
	hid_device *hidPanel;
	struct usb_data_t usbDataOut;
	struct usb_ctrl_t usbDataIn;
public:
	struct mcp_data_t *mcpData;
	struct mip_data_t *mipData;
	struct mcp_ctrl_t *mcpCtrl;
	struct mip_ctrl_t *mipCtrl;
	struct PanelInput input;

	Panel(void) {};
	~Panel(void) {};

	void connect() {
		printf("MCP connecting...\n");
		wchar_t wstr[MAX_STR];
		hid_init();
		hidPanel = hid_open(0x483, 0x5750, NULL);
		if (hidPanel != NULL) {
			hid_get_manufacturer_string(hidPanel, wstr, MAX_STR);
			wprintf(L"Manufacturer String: %s\n", wstr);
			hid_get_product_string(hidPanel, wstr, MAX_STR);
			wprintf(L"Product String: %s\n", wstr);
			hid_get_serial_number_string(hidPanel, wstr, MAX_STR);
			wprintf(L"Serial Number String: %s\n", wstr);
			mcpData = &usbDataOut.mcp;
			mipData = &usbDataOut.mip;
			mcpCtrl = &usbDataIn.mcp;
			mipCtrl = &usbDataIn.mip;
		}
	}

	void disconnect() {
		hid_close(hidPanel);
	}

	void test(unsigned char on) {
		auto seconds = time(NULL);

		mcpData->speedCrsL = seconds % 3 == 0 && on ? 0x888F8888 : DISP_OFF_MASK;
		mcpData->vspeedCrsR = seconds % 3 == 0 && on ? 0x888C8880 : DISP_OFF_MASK;
		mcpData->altitudeHdg = seconds % 3 == 0 && on ? 0x88888880 : DISP_OFF_MASK;
		mcpData->alt_hld = on;
		mcpData->app = on;
		mcpData->at_arm = on;
		mcpData->cmd_a = on;
		mcpData->cmd_b = on;
		mcpData->cws_a = on;
		mcpData->cws_b = on;
		mcpData->fd_ca = on;
		mcpData->fd_fo = on;
		mcpData->hdg_sel = on;
		mcpData->lnav = on;
		mcpData->lvl_chg = on;
		mcpData->n1 = on;
		mcpData->speed = on;
		mcpData->vnav = on;
		mcpData->vor_loc = on;
		mcpData->vs = on;
		mipData->annunNGearGrn = on;
		mipData->annunNGearRed = on;
		mipData->annunRGearGrn = on;
		mipData->annunRGearRed = on;
		mipData->annunLGearGrn = on;
		mipData->annunLGearRed = on;
		mipData->annunAntiskidInop = on;
		mipData->annunAutobreakDisarm = on;
		mipData->annunFlapsExt = on;
		mipData->annunFlapsTransit = on;
		mipData->annunStabOutOfTrim = on;
		mipData->annunBelowGS = on;
		mipData->annunSpeedbrakeArmed = on;
		mipData->annunSpeedbrakNotArm = on;
	}

	void setMCPDisplays(unsigned short courseL, float IASKtsMach, DisplayState IASState, unsigned short heading, unsigned short altitude, short vertSpeed, bool vsEnabled, unsigned short courseR) {
		mcpData->speedCrsL = displayHi(courseL);
		mcpData->vspeedCrsR = displayHi(courseR);
		mcpData->altitudeHdg = displayHi(heading) & displayLo(altitude);

		if (IASState != Blank) {
			if (IASState == Overspeed) {
				mcpData->speedCrsL &= 0xFFFFBFFF;
			}

			if (IASState == Underspeed) {
				mcpData->speedCrsL &= 0xFFFFAFFF;
			}

			if (IASKtsMach < 10) {
				mcpData->speedCrsL &= displayLo(IASKtsMach);
			} else {
				mcpData->speedCrsL &= displayLo((int)IASKtsMach);
			}
		}

		if (vsEnabled) {
			mcpData->vspeedCrsR &= displayLo(vertSpeed, 0xFFFF0000);
		}
	}

	void send() {
		usbDataOut.report_id = 2;
		hid_write(hidPanel, (unsigned char*)&usbDataOut, sizeof(usb_data_t));
	}

	bool read() {
		bool result = false;

		mcpCtrl->value = 0;
		unsigned char prevMode = mcpCtrl->efisMode;
		unsigned char prevRange = mcpCtrl->efisRange;

		if (hid_read_timeout(hidPanel, (unsigned char*)&usbDataIn, sizeof(usb_ctrl_t), 1)) {
			mcpCtrl->efisMode = mcpCtrl->efisMode ? decodeRotaryState(mcpCtrl->efisMode) - 1 : prevMode;
			mcpCtrl->efisRange = mcpCtrl->efisRange ? decodeRotaryState(mcpCtrl->efisRange) - 1 : prevRange;
			input.vorAdfSel1 = toSwitchState(mcpCtrl->efisVOR1, mcpCtrl->efisADF1);
			input.vorAdfSel2 = toSwitchState(mcpCtrl->efisVOR2, mcpCtrl->efisADF2);
			mipCtrl->autoBreak = decodeRotaryState(mipCtrl->autoBreak);
			mipCtrl->mainPanelDU = decodeRotaryState(mipCtrl->mainPanelDU);
			mipCtrl->lowerDU = decodeRotaryState(mipCtrl->lowerDU);
			input.fuelFlowSw = toSwitchState(mipCtrl->ffReset, mipCtrl->ffUsed);
			input.mainLights = toSwitchState(mipCtrl->lightsTest, mipCtrl->lightsDim);
			input.disengageLights = toSwitchState(mipCtrl->afdsTest1, mipCtrl->afdsTest2);
			input.gearLever = toSwitchState(mipCtrl->gearUP, mipCtrl->gearDN);
			result = true;
		}

		return result;
	}
};

class Overhead {
private:
	SerialPort port;
public:
	struct overhead_ctrl_t ctrl;

	Overhead(void) {};
	~Overhead(void) {};

	void connect(const wchar_t* path) {
		printf("Overhead connecting...\n");
		if (port.connect(path, CBR_4800)) {
			char *message = port.readMessage();
			printf("Overhead connected: %s\n", message);
		}
	}

	void disconnect() {
		port.close();
	}

	bool read() {
		if (port.readData(&ctrl)) {
			ctrl.eng_start_l = decodeRotaryState(ctrl.eng_start_l);
			ctrl.eng_start_r = decodeRotaryState(ctrl.eng_start_r);
			return true;
		}

		return false;
	}
};

class RadioPanel {
private:
	struct radio_data_t prevData;
	SerialPort port;
public:
	static const unsigned short navMin = 10800;
	static const unsigned short navMax = 11795;
	static const unsigned short vhfMin = 11800;
	static const unsigned short vhfMax = 13697;
	static const unsigned short adfMin = 1900;
	static const unsigned short adfMax = 17500;

	struct radio_data_t data;
	struct radio_ctrl_t ctrl;

	RadioPanel() {};
	~RadioPanel() {};

	void connect(const wchar_t* path) {
		data.com1.active = vhfMin;
		data.com1.standby = vhfMax;
		data.com2.active = vhfMax;
		data.com2.standby = vhfMin;
		data.nav1.active = navMin;
		data.nav1.standby = navMax;
		data.nav2.active = navMax;
		data.nav2.standby = navMin;
		data.adf1.active = adfMin;
		data.adf1.standby = adfMax;
		data.atc1 = 7777;

		printf("VHF connecting...\n");
		if (port.connect(path, 38400)) {
			char *message = port.readMessage();
			printf("VHF connected: %s\n", message);
		}
	}

	void test(unsigned char on) {
		auto seconds = time(NULL);

		unsigned short vhf_test_1 = seconds % 3 == 0 && on ? 18888 : 0;
		unsigned short vhf_test_2 = seconds % 3 == 1 && on ? 18888 : 0;
		unsigned short vhf_test_3 = seconds % 3 == 2 && on ? 18888 : 0;

		data.com1.active = vhf_test_1;
		data.com1.standby = vhf_test_1;
		data.com2.active = vhf_test_1;
		data.com2.standby = vhf_test_1;
		data.nav1.active = vhf_test_2;
		data.nav1.standby = vhf_test_2;
		data.nav2.active = vhf_test_2;
		data.nav2.standby = vhf_test_2;
		data.adf1.active = vhf_test_3;
		data.adf1.standby = vhf_test_3;
		data.atc1 = 8888;
	}

	void update() {
		if (memcmp(&prevData, &data, sizeof(radio_data_t))) {
			//printf("COM1 %d, COM2 %d, NAV1 %d, NAV2 %d, ADF1 %d\n", data.com1.active, data.com2.active, data.nav1.active, data.nav2.active, data.adf1.active);
			port.sendData(&data);
			prevData = data;
		}
	}

	bool read() {
		ctrl.encValue = 0;
		if (port.readData(&ctrl)) {
			if (ctrl.XPDR_Mode) {
				ctrl.XPDR_Mode = decodeRotaryState(ctrl.XPDR_Mode) - 1;
			}

			return true;
		}

		return false;
	}
};