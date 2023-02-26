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

#define MCP_ALTITUDE   0
#define MCP_IAS_MACH   1
#define MCP_COURSE_CA  2
#define EFIS_BARO      3
#define EFIS_MINS      4
#define MCP_VERT_SPEED 5
#define MCP_COURSE_FO  6
#define MCP_HEADING    7

#pragma pack(push, 1)
struct mip_data_t {
	unsigned short flaps;
	unsigned char annunAntiskidInop    : 1;
	unsigned char /* reserved bits */  : 5;
	unsigned char annunFlapsTransit    : 1;
	unsigned char annunAutobreakDisarm : 1;
	unsigned char annunFlapsExt        : 1;
	unsigned char annunBelowGS         : 1;
	unsigned char annunNGearRed        : 1;
	unsigned char annunNGearGrn        : 1;
	unsigned char annunRGearRed        : 1;
	unsigned char annunRGearGrn        : 1;
	unsigned char annunLGearRed        : 1;
	unsigned char annunLGearGrn        : 1;
	unsigned char annunWarnFltCont     : 1;
	unsigned char annunWarnIRS         : 1;
	unsigned char annunWarnFuel        : 1;
	unsigned char annunMasterCaution   : 1;
	unsigned char annunFireWarning     : 1;
	unsigned char annunWarnOvht        : 1;
	unsigned char annunWarnAPU         : 1;
	unsigned char annunWarnElec        : 1;
	unsigned char annunAPRstYel        : 1;
	unsigned char annunAPRstRed        : 1;
	unsigned char annunATRstYel        : 1;
	unsigned char annunATRstRed        : 1;
	unsigned char annunFMCRstYel       : 1;
	unsigned char annunSpeedbrakeArmed : 1;
	unsigned char annunSpeedbrakNotArm : 1;
	unsigned char annunStabOutOfTrim   : 1;
	unsigned char backlight;
	unsigned char backlight2;
};

struct mcp_data_t {
	unsigned long vspeedCrsR;
	unsigned long altitudeHdg;
	unsigned long speedCrsL;
	unsigned char alt_hld     : 1;
	unsigned char vs          : 1;
	unsigned char fd_fo       : 1;
	unsigned char cws_a       : 1;
	unsigned char cws_b       : 1;
	unsigned char cmd_b       : 1;
	unsigned char cmd_a       : 1;
	unsigned char lnav        : 1;
	unsigned char fd_ca       : 1;
	unsigned char n1          : 1;
	unsigned char speed       : 1;
	unsigned char lvl_chg     : 1;
	unsigned char hdg_sel     : 1;
	unsigned char app         : 1;
	unsigned char vor_loc     : 1;
	unsigned char vnav        : 1;
	unsigned char at_arm      : 1;
	unsigned char reserved    : 3;
	unsigned char brightness  : 4;
};

struct mip_ctrl_t {
	unsigned char mipSpdRefSel;
	unsigned char mipN1Sel    : 4;
	unsigned char fuelFlowSw  : 2;
	unsigned char mfdENG      : 1;
	unsigned char mfdSYS      : 1;
	unsigned char gearLever   : 2;
	unsigned char autoBreak   : 6;
	unsigned char masterCautn : 1;
	unsigned char annunRecall : 1;
	unsigned char mainPanelDU : 4;
	unsigned char lowerDU     : 2;
	unsigned char fireWarning : 1;
	unsigned char lightsSw    : 2;
	unsigned char afdsTestSw  : 2;
	unsigned char afdsRstFMC  : 1;
	unsigned char afdsRstAT   : 1;
	unsigned char afdsRstAP   : 1;
};

struct mcp_ctrl_t {
	unsigned char encoder;
	unsigned char value;
	// MCP buttons mapping
	unsigned char hdg_sel  : 1;
	unsigned char app      : 1;
	unsigned char vor_loc  : 1;
	unsigned char spd_intv : 1;
	unsigned char fd_ca    : 1;
	unsigned char n1       : 1;
	unsigned char speed    : 1;
	unsigned char lvl_chg  : 1;
	unsigned char cmd_b    : 1;
	unsigned char cmd_a    : 1;
	unsigned char cws_b    : 1;
	unsigned char cws_a    : 1;
	unsigned char fd_fo    : 1;
	unsigned char alt_hld  : 1;
	unsigned char vs       : 1;
	unsigned char alt_intv : 1;
	unsigned char at_arm   : 1;
	unsigned char vnav     : 1;
	unsigned char lnav     : 1;
	unsigned char speed_co : 1;
	unsigned char /* NC */ : 4;
	// EFIS buttons mapping
	unsigned char efisRST  : 1;
	unsigned char efisSTD  : 1;
	unsigned char /* NC */ : 2;
	unsigned char efisMode : 4;
	unsigned char efisRange;
	unsigned char efisPOS  : 1;
	unsigned char efisDATA : 1;
	unsigned char efisARPT : 1;
	unsigned char /* NC */ : 1;
	unsigned char efisVOR2 : 2;
	unsigned char efisBARO : 1;
	unsigned char efisMTRS : 1;
	unsigned char efisFPV  : 1;
	unsigned char efisMINS : 1;
	unsigned char efisVOR1 : 2;
	unsigned char efisTERR : 1;
	unsigned char efisWPT  : 1;
	unsigned char efisSTA  : 1;
	unsigned char efisWXR  : 1;
};
#pragma pack(pop)

#define RADIO_WHOLE 0
#define RADIO_FRACT 1
#define RADIO_ATC_L 2
#define RADIO_ATC_R 3

struct radio_ctrl_t {
	unsigned char encIndex;
	unsigned char encValue;
	unsigned char freqSelected;
	unsigned char freqSwap;
	unsigned char XPDR_Mode  : 5;
	unsigned char XPDR_Ident : 1;
	unsigned char XPDR_Sel   : 1;
	unsigned char ALT_Source : 1;
};

struct frequency_t {
	unsigned short active;
	unsigned short standby;
};

struct radio_data_t {
	frequency_t com1;
	frequency_t com2;
	frequency_t nav1;
	frequency_t nav2;
	frequency_t adf1;
	unsigned short atc1;
	unsigned char atcFail  : 1;
	unsigned char atcLed0  : 1;
	unsigned char atcLed1  : 1;
	unsigned char atcLed2  : 1;
	unsigned char atcLeds  : 4;
	unsigned char parking  : 1;
	unsigned char notInUse : 7;
};

struct overhead_ctrl_t {
	unsigned char logo_light    : 1;
	unsigned char steady_light  : 1;
	unsigned char strobe_light  : 1;
	unsigned char anti_collisn  : 1;
	unsigned char wing_light    : 1;
	unsigned char wheel_well    : 1;
	unsigned char apu_start     : 1;
	unsigned char /* NC */      : 1;
	unsigned char eng_start_r   : 3;
	unsigned char eng_igniter   : 2;
	unsigned char eng_start_l   : 3;
	unsigned char apu_off       : 1;
	unsigned char taxi_light    : 1;
	unsigned char rw_turnoff_r  : 1;
	unsigned char rw_turnoff_l  : 1;
	unsigned char ldg_fixed_r   : 1;
	unsigned char ldg_fixed_l   : 1;
	unsigned char ldg_retract_r : 1;
	unsigned char ldg_retract_l : 1;
};