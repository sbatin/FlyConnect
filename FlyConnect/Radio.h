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
	double Transponder;
	double ParkingBrake;
	double VerticalSpeed;
	double RadioAltitude;
	double GForce;
	double SimOnGround;
};

struct RadioInterface {
	template <typename T>
	static T bcd2dec(T value) {
		int k = 1;
		T result = 0;

		while (value) {
			result += k * (value & 0xF);
			k *= 10;
			value = value >> 4;
		}

		return result;
	}

	template <typename T>
	static T dec2bcd(T dec) {
		T result = 0;
		int shift = 0;

		while (dec) {
			result +=  (dec % 10) << shift;
			dec = dec / 10;
			shift += 4;
		}
		return result;
	}

	static inline double convertBCD16(double frequency) {
		return 100 + (double)bcd2dec((unsigned short)frequency) / 100;
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
		SimConnect_AddToDataDefinition(hSimConnect, RADIO_DEF, "TRANSPONDER CODE:1", "BCO16");
		SimConnect_AddToDataDefinition(hSimConnect, RADIO_DEF, "BRAKE PARKING INDICATOR", "Bool");
		SimConnect_AddToDataDefinition(hSimConnect, RADIO_DEF, "VERTICAL SPEED", "Feet per second");
		SimConnect_AddToDataDefinition(hSimConnect, RADIO_DEF, "PLANE ALT ABOVE GROUND", "feet");
		SimConnect_AddToDataDefinition(hSimConnect, RADIO_DEF, "G FORCE", "GForce");
		SimConnect_AddToDataDefinition(hSimConnect, RADIO_DEF, "SIM ON GROUND", "Bool");

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

		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_ATC1_RADIO_WHOLE_DEC, "XPNDR_100_DEC");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_ATC1_RADIO_WHOLE_INC, "XPNDR_100_INC");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_ATC1_RADIO_FRACT_DEC, "XPNDR_DEC_CARRY");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_ATC1_RADIO_FRACT_INC, "XPNDR_INC_CARRY");

		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_COM1_RADIO_SET, "COM1_RADIO_HZ_SET");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_COM2_RADIO_SET, "COM2_RADIO_HZ_SET");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_NAV1_RADIO_SET, "NAV1_RADIO_SET");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_NAV2_RADIO_SET, "NAV2_RADIO_SET");
		SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_ADF1_RADIO_SET, "ADF_COMPLETE_SET");
	}

	static void toggle(HANDLE hSimConnect, EVENT_ID evt) {
		SimConnect_TransmitClientEvent(hSimConnect, 0, evt, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
	}

	static void set(HANDLE hSimConnect, EVENT_ID evt, DWORD value) {
		if (evt == EVENT_COM1_RADIO_SET || evt == EVENT_COM2_RADIO_SET) {
			value = value * 10000;
		} else if (evt == EVENT_NAV1_RADIO_SET || evt == EVENT_NAV2_RADIO_SET) {
			value = RadioInterface::dec2bcd(value);
		} else {
			value = RadioInterface::dec2bcd(value * 1000);
		}
		SimConnect_TransmitClientEvent(hSimConnect, 0, evt, value, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
	}

	static void rotate(HANDLE hSimConnect, char value, EVENT_ID inc, EVENT_ID dec) {
		if (value > 0) {
			SimConnect_TransmitClientEvent(hSimConnect, 0, inc, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
		} else if (value < 0) {
			SimConnect_TransmitClientEvent(hSimConnect, 0, dec, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
		}
	}

	static void setData(FSX_Radio_Data* dest, const FSX_Radio_Data* source) {
		dest->NAV1_Active  = source->NAV1_Active;
		dest->NAV1_StandBy = source->NAV1_StandBy;
		dest->NAV2_Active  = source->NAV2_Active;
		dest->NAV2_StandBy = source->NAV2_StandBy;
		dest->COM1_Active  = convertBCD16(source->COM1_Active);
		dest->COM1_StandBy = convertBCD16(source->COM1_StandBy);
		dest->COM2_Active  = convertBCD16(source->COM2_Active);
		dest->COM2_StandBy = convertBCD16(source->COM2_StandBy);
		dest->ADF1_Active  = (double)bcd2dec((unsigned int)source->ADF1_Active) / 10000;
		dest->Transponder  = bcd2dec((unsigned short)source->Transponder);
		dest->ParkingBrake = source->ParkingBrake;
		dest->VerticalSpeed = source->VerticalSpeed;
		dest->RadioAltitude = source->RadioAltitude;
		dest->GForce        = source->GForce;
		dest->SimOnGround   = source->SimOnGround;
	}
};