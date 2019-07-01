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
	unsigned char /* reserved bits */  : 3;
	unsigned char annunBelowGS         : 1;
	unsigned char /* reserved bits */  : 2;
	unsigned char annunAntiskidInop    : 1;
	unsigned char annunAutobreakDisarm : 1;
	unsigned char annunFlapsExt        : 1;
	unsigned char annunLGearGrn        : 1;
	unsigned char annunNGearGrn        : 1;
	unsigned char annunRGearGrn        : 1;
	unsigned char annunRGearRed        : 1;
	unsigned char annunNGearRed        : 1;
	unsigned char annunLGearRed        : 1;
	unsigned char annunFlapsTransit    : 1;
	unsigned char backlight;
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
	unsigned char ffUsed      : 1;
	unsigned char ffReset     : 1;
	unsigned char /* NC */    : 2;
	unsigned char mfdENG      : 1;
	unsigned char mfdSYS      : 1;
	unsigned char gearUP      : 1;
	unsigned char gearDN      : 1;
	unsigned char mipN1Sel    : 3;
	unsigned char autoBreak   : 5;
	unsigned char masterCautn : 1;
	unsigned char annunRecall : 1;
	unsigned char mainPanelDU : 4;
	unsigned char lowerDU     : 2;
	unsigned char fireWarning : 1;
	unsigned char lightsTest  : 1;
	unsigned char lightsDim   : 1;
	unsigned char afdsTest2   : 1;
	unsigned char afdsTest1   : 1;
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
	unsigned char efisVOR2 : 1;
	unsigned char efisADF2 : 1;
	unsigned char /* NC */ : 1;
	unsigned char efisMTRS : 1;
	unsigned char efisFPV  : 1;
	unsigned char /* NC */ : 1;
	unsigned char efisADF1 : 1;
	unsigned char efisVOR1 : 1;
	unsigned char efisTERR : 1;
	unsigned char efisWPT  : 1;
	unsigned char efisSTA  : 1;
	unsigned char efisWXR  : 1;
};
#pragma pack(pop)

struct radio_ctrl_t {
	unsigned char encWhole;
	unsigned char encFract;
	unsigned char freqSelected;
	unsigned char freqSwap;
	unsigned char buttons;
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
	unsigned short adf1;
	unsigned short brk1 : 1;
	unsigned short atc1 : 15;
};

struct overhead_ctrl_t {
	unsigned char eng_ign_r     : 1;
	unsigned char eng_ign_l     : 1;
	unsigned char /* NC */      : 1;
	unsigned char wing_light    : 1;
	unsigned char anti_collisn  : 1;
	unsigned char strobe_light  : 1;
	unsigned char steady_light  : 1;
	unsigned char logo_light    : 1;
	unsigned char eng_start_r   : 3;
	unsigned char /* NC */      : 1;
	unsigned char eng_start_l   : 3;
	unsigned char apu_start     : 1;
	unsigned char ldg_retract_l : 1;
	unsigned char ldg_retract_r : 1;
	unsigned char ldg_fixed_l   : 1;
	unsigned char ldg_fixed_r   : 1;
	unsigned char rw_turnoff_l  : 1;
	unsigned char rw_turnoff_r  : 1;
	unsigned char taxi_light    : 1;
	unsigned char apu_off       : 1;
};