enum DATA_DEFINE_ID {
    RADIO_DEF,
};

struct FSX_Radio_Data {
	double NAV1_Active;
	double NAV1_StandBy;
	double NAV2_Active;
	double NAV2_StandBy;
	double COM1_Active;
	double COM1_StandBy;
	double COM2_Active;
	double COM2_StandBy;
	double ADF1_Active;
	double ADF1_StandBy;
	double Transponder;
};

/*static inline double convertFrequency(double frequency) {
	return floor(100 * frequency + 0.5);
}*/

struct RadioInterface {
	template <typename T>
	static T converBCD(T value) {
		int k = 1;
		T result = 0;

		while (value) {
			result += k * (value & 0xF);
			k *= 10;
			value = value >> 4;
		}

		return result;
	}

	static inline double convertBCD16(double frequency) {
		return 100 + (double)converBCD((unsigned short)frequency) / 100;
	}

	static void connect(HANDLE hSimConnect) {
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

	static void toggle(HANDLE hSimConnect, EVENT_ID evt) {
		SimConnect_TransmitClientEvent(hSimConnect, 0, evt, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
	}

	static void rotate(HANDLE hSimConnect, char value, EVENT_ID inc, EVENT_ID dec) {
		if (value > 0)
			SimConnect_TransmitClientEvent(hSimConnect, 0, inc, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);

		if (value < 0)
			SimConnect_TransmitClientEvent(hSimConnect, 0, dec, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
	}

	static void setData(FSX_Radio_Data* destionation, const FSX_Radio_Data* source) {
		destionation->NAV1_Active  = source->NAV1_Active;
		destionation->NAV1_StandBy = source->NAV1_StandBy;
		destionation->NAV2_Active  = source->NAV2_Active;
		destionation->NAV2_StandBy = source->NAV2_StandBy;
		destionation->COM1_Active  = convertBCD16(source->COM1_Active);
		destionation->COM1_StandBy = convertBCD16(source->COM1_StandBy);
		destionation->COM2_Active  = convertBCD16(source->COM2_Active);
		destionation->COM2_StandBy = convertBCD16(source->COM2_StandBy);
		destionation->ADF1_Active  = (double)converBCD((unsigned int)source->ADF1_Active) / 10000;
		//destionation->ADF1_StandBy = source->ADF1_StandBy / 100;
		destionation->Transponder  = converBCD((unsigned short)source->Transponder);
	}
};

/*class RadioPanel {
private:
	struct cdu_ctrl_t ctrl;
	struct cdu_data_t currData;
	struct cdu_data_t prevData;
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
		currData.freqSelected = freqSelected;
		update();
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
	};

	~RadioPanel(void) {
	};
};*/