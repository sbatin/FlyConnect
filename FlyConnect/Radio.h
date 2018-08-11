#include "SerialPort.h"

enum DATA_DEFINE_ID {
    RADIO_DEF,
};

struct cdu_ctrl_t {
	unsigned char enc1;
	unsigned char enc2;
	unsigned char freqSelected;
	unsigned char atcMode;
};

struct frequency_t {
	unsigned short active;
	unsigned short standby;
};

struct cdu_data_t {
	frequency_t com1;
	frequency_t com2;
	frequency_t nav1;
	frequency_t nav2;
	frequency_t adf1;
	unsigned short atcCode;
	unsigned char atcMode;
	unsigned char freqSelected;
};

struct FSX_RadioData {
	double nav1_active;
	double nav1_standby;
	double nav2_active;
	double nav2_standby;
	double com1_active;
	double com1_standby;
	double com2_active;
	double com2_standby;
	double adf1_active;
	double adf1_standby;
	double transponder;
};

unsigned short convertFrequency(double frequency) {
	return (unsigned short)floor(100 * frequency + 0.5);
}

template <typename T>
T converBCD(T value) {
	int k = 1;
	T result = 0;

	while (value) {
		result+= k * (value & 0xF);
		k*= 10;
		value = value >> 4;
	}

	return result;
}

static inline unsigned short convertBCD16(double frequency) {
	return 10000 + converBCD((unsigned short)frequency);
}

class RadioPanel {
private:
	struct cdu_ctrl_t ctrl;
	struct cdu_data_t currData;
	struct cdu_data_t prevData;
	SerialPort* port;
	unsigned char freqSelected;

	void update() {
		if (memcmp(&currData, &prevData, sizeof(cdu_data_t))) {
			port->sendData(&currData);
			memcpy(&prevData, &currData, sizeof(cdu_data_t));
		}
	}
public:
	void connect(LPCWSTR path) {
		printf("Radio panel connecting...\n");
		if (port->connect(path)) {
			char *message = port->readMessage();
			printf("Radio panel connected: %s\n", message);
		}
	}

	void setXpndrMode(unsigned char mode) {
		currData.atcMode = mode;
		update();
	}

	void setRadioData(FSX_RadioData* rawData) {
		currData.nav1.active  = convertFrequency(rawData->nav1_active);
		currData.nav1.standby = convertFrequency(rawData->nav1_standby);
		currData.nav2.active  = convertFrequency(rawData->nav2_active);
		currData.nav2.standby = convertFrequency(rawData->nav2_standby);
		currData.com1.active  = convertBCD16(rawData->com1_active);
		currData.com1.standby = convertBCD16(rawData->com1_standby);
		currData.com2.active  = convertBCD16(rawData->com2_active);
		currData.com2.standby = convertBCD16(rawData->com2_standby);
		currData.adf1.active  = converBCD((unsigned int)rawData->adf1_active) / 1000;
		currData.adf1.standby = (unsigned int)(rawData->adf1_standby) / 100;
		currData.atcCode      = convertBCD16(rawData->transponder);
		currData.freqSelected = freqSelected;
		update();
	}

	void setup(HANDLE hSimConnect) {
		SimConnect_AddToDataDefinition(hSimConnect, RADIO_DEF, "NAV ACTIVE FREQUENCY:1", "MHz");
		SimConnect_AddToDataDefinition(hSimConnect, RADIO_DEF, "NAV STANDBY FREQUENCY:1", "MHz");
		SimConnect_AddToDataDefinition(hSimConnect, RADIO_DEF, "NAV ACTIVE FREQUENCY:2", "MHz");
		SimConnect_AddToDataDefinition(hSimConnect, RADIO_DEF, "NAV STANDBY FREQUENCY:2", "MHz");
		SimConnect_AddToDataDefinition(hSimConnect, RADIO_DEF, "COM ACTIVE FREQUENCY:1", "Frequency BCD16");
		SimConnect_AddToDataDefinition(hSimConnect, RADIO_DEF, "COM STANDBY FREQUENCY:1", "Frequency BCD16");
		SimConnect_AddToDataDefinition(hSimConnect, RADIO_DEF, "COM ACTIVE FREQUENCY:2", "Frequency BCD16");
		SimConnect_AddToDataDefinition(hSimConnect, RADIO_DEF, "COM STANDBY FREQUENCY:2", "Frequency BCD16");
		SimConnect_AddToDataDefinition(hSimConnect, RADIO_DEF, "ADF ACTIVE FREQUENCY:1", "Frequency ADF BCD32");
		SimConnect_AddToDataDefinition(hSimConnect, RADIO_DEF, "ADF STANDBY FREQUENCY:1", "Hz");
		SimConnect_AddToDataDefinition(hSimConnect, RADIO_DEF, "TRANSPONDER CODE:1", "BCO16");

		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_NAV1_RADIO_WHOLE_DEC, "NAV1_RADIO_WHOLE_DEC");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_NAV1_RADIO_WHOLE_INC, "NAV1_RADIO_WHOLE_INC");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_NAV1_RADIO_FRACT_DEC, "NAV1_RADIO_FRACT_DEC_CARRY");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_NAV1_RADIO_FRACT_INC, "NAV1_RADIO_FRACT_INC_CARRY");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_NAV1_RADIO_SWAP, "NAV1_RADIO_SWAP");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_NAV2_RADIO_WHOLE_DEC, "NAV2_RADIO_WHOLE_DEC");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_NAV2_RADIO_WHOLE_INC, "NAV2_RADIO_WHOLE_INC");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_NAV2_RADIO_FRACT_DEC, "NAV2_RADIO_FRACT_DEC_CARRY");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_NAV2_RADIO_FRACT_INC, "NAV2_RADIO_FRACT_INC_CARRY");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_NAV2_RADIO_SWAP, "NAV2_RADIO_SWAP");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_COM1_RADIO_WHOLE_DEC, "COM_RADIO_WHOLE_DEC");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_COM1_RADIO_WHOLE_INC, "COM_RADIO_WHOLE_INC");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_COM1_RADIO_FRACT_DEC, "COM_RADIO_FRACT_DEC_CARRY");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_COM1_RADIO_FRACT_INC, "COM_RADIO_FRACT_INC_CARRY");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_COM1_RADIO_SWAP, "COM_STBY_RADIO_SWAP");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_COM2_RADIO_WHOLE_DEC, "COM2_RADIO_WHOLE_DEC");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_COM2_RADIO_WHOLE_INC, "COM2_RADIO_WHOLE_INC");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_COM2_RADIO_FRACT_DEC, "COM2_RADIO_FRACT_DEC_CARRY");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_COM2_RADIO_FRACT_INC, "COM2_RADIO_FRACT_INC_CARRY");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_COM2_RADIO_SWAP, "COM2_RADIO_SWAP");

		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_ADF1_RADIO_WHOLE_DEC, "ADF1_WHOLE_DEC");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_ADF1_RADIO_WHOLE_INC, "ADF1_WHOLE_INC");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_ADF1_RADIO_FRACT_DEC, "ADF_FRACT_DEC_CARRY");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_ADF1_RADIO_FRACT_INC, "ADF_FRACT_INC_CARRY");

		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_ATC1_RADIO_WHOLE_DEC, "XPNDR_100_DEC");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_ATC1_RADIO_WHOLE_INC, "XPNDR_100_INC");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_ATC1_RADIO_FRACT_DEC, "XPNDR_DEC_CARRY");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_ATC1_RADIO_FRACT_INC, "XPNDR_INC_CARRY");
	}

	void sendControlData(HANDLE hSimConnect) {
		if (port->readData(&ctrl)) {
			if (ctrl.freqSelected) {
				if (ctrl.freqSelected == freqSelected) {
					if (ctrl.freqSelected == 1) sendToggleEvent(hSimConnect, EVENT_COM1_RADIO_SWAP);
					if (ctrl.freqSelected == 2) sendToggleEvent(hSimConnect, EVENT_NAV1_RADIO_SWAP);
					if (ctrl.freqSelected == 4) sendToggleEvent(hSimConnect, EVENT_COM2_RADIO_SWAP);
					if (ctrl.freqSelected == 5) sendToggleEvent(hSimConnect, EVENT_NAV2_RADIO_SWAP);
					freqSelected = 0;
				} else {
					freqSelected = ctrl.freqSelected;
				}
			}

			if (ctrl.enc2) {
				if (freqSelected == 1) sendLinearIncDecEvent(hSimConnect, ctrl.enc2, EVENT_COM1_RADIO_WHOLE_INC, EVENT_COM1_RADIO_WHOLE_DEC);
				if (freqSelected == 2) sendLinearIncDecEvent(hSimConnect, ctrl.enc2, EVENT_NAV1_RADIO_WHOLE_INC, EVENT_NAV1_RADIO_WHOLE_DEC);
				if (freqSelected == 3) for (int i = 0; i < 10; i++) sendLinearIncDecEvent(hSimConnect, ctrl.enc2, EVENT_ADF1_RADIO_WHOLE_INC, EVENT_ADF1_RADIO_WHOLE_DEC);
				if (freqSelected == 4) sendLinearIncDecEvent(hSimConnect, ctrl.enc2, EVENT_COM2_RADIO_WHOLE_INC, EVENT_COM2_RADIO_WHOLE_DEC);
				if (freqSelected == 5) sendLinearIncDecEvent(hSimConnect, ctrl.enc2, EVENT_NAV2_RADIO_WHOLE_INC, EVENT_NAV2_RADIO_WHOLE_DEC);
				//if (freqSelected == 6) sendLinearIncDecEvent(hSimConnect, ctrl.enc2, EVENT_ATC1_RADIO_WHOLE_INC, EVENT_ATC1_RADIO_WHOLE_DEC);
				if (freqSelected == 6) for (int i = 0; i < 64; i++) sendLinearIncDecEvent(hSimConnect, ctrl.enc2, EVENT_ATC1_RADIO_FRACT_INC, EVENT_ATC1_RADIO_FRACT_DEC);
			}

			if (ctrl.enc1) {
				if (freqSelected == 1) sendLinearIncDecEvent(hSimConnect, ctrl.enc1, EVENT_COM1_RADIO_FRACT_INC, EVENT_COM1_RADIO_FRACT_DEC);
				if (freqSelected == 2) sendLinearIncDecEvent(hSimConnect, ctrl.enc1, EVENT_NAV1_RADIO_FRACT_INC, EVENT_NAV1_RADIO_FRACT_DEC);
				if (freqSelected == 3) sendLinearIncDecEvent(hSimConnect, ctrl.enc1, EVENT_ADF1_RADIO_FRACT_INC, EVENT_ADF1_RADIO_FRACT_DEC);
				if (freqSelected == 4) sendLinearIncDecEvent(hSimConnect, ctrl.enc1, EVENT_COM2_RADIO_FRACT_INC, EVENT_COM2_RADIO_FRACT_DEC);
				if (freqSelected == 5) sendLinearIncDecEvent(hSimConnect, ctrl.enc1, EVENT_NAV2_RADIO_FRACT_INC, EVENT_NAV2_RADIO_FRACT_DEC);
				if (freqSelected == 6) sendLinearIncDecEvent(hSimConnect, ctrl.enc1, EVENT_ATC1_RADIO_FRACT_INC, EVENT_ATC1_RADIO_FRACT_DEC);
			}
		}
	}

	RadioPanel() {
		port = new SerialPort();
	};

	~RadioPanel(void) {
		port->close();
		delete port;
	};
};

RadioPanel radio = RadioPanel();