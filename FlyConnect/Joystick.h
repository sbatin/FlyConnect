#pragma pack(push, 1)
struct throttle_data_t {
	unsigned short reserved1;
	unsigned char reserved2;
	unsigned long buttons;
	unsigned char z;
	unsigned char rx;
	unsigned char rz;
	unsigned char ry;
	unsigned char reserved3;
};

struct tq_buttons_t {
	unsigned char /* NC */ : 1;
	unsigned char sw_12    : 2;
	unsigned char sw_34    : 2;
	unsigned char sw_56    : 2;
	unsigned char flaps_1  : 1;
	unsigned char flaps_up : 1;
	unsigned char flaps_10 : 1;
	unsigned char flaps_15 : 1;
	unsigned char flaps_2  : 1;
	unsigned char flaps_5  : 1;
	unsigned char flaps_30 : 1;
	unsigned char flaps_25 : 1;
	unsigned char /* NC */ : 1;
	unsigned char reserved;
	unsigned char /* NC */ : 2;
	unsigned char toggle_1 : 1;
	unsigned char brake_sw : 3;
	unsigned char /* NC */ : 2;
};
#pragma pack(pop)

static unsigned long tqButtonsPrev = 0;
static unsigned char tqDataReady = 0;
static throttle_data_t tqDataReceived = {0};

DWORD WINAPI __joystickLoop(LPVOID lpParameter) {
	hid_device* device = (hid_device*)lpParameter;

    while (1) {
        int n = hid_read_timeout(device, (unsigned char*)&tqDataReceived, MAX_STR, 1);
		if (n == sizeof(throttle_data_t)) {
			if (tqDataReceived.buttons != tqButtonsPrev) {
				//while (!tqDataReady) {
					tqButtonsPrev = tqDataReceived.buttons;
					tqDataReady = 1;
				//}
			}
		}
        Sleep(5);
    }
}

class JoystickTQ {
private:
	HANDLE hLoop;

	static unsigned char decodeRotaryState(unsigned char value) {
		for (int i = 0; i < 8; i++) {
			if (value & (1 << i)) {
				return i;
			}
		}

		return 0;
	}
public:
	tq_buttons_t data = {0};

	void connect() {
		auto device = hid_open(0x0738, 0xA215, NULL);

		if (device == NULL) {
			return;
		}

		wchar_t wstr[MAX_STR];
		hid_get_product_string(device, wstr, MAX_STR);
		wprintf(L"JoystickTQ connected: %s\n", wstr);

		hLoop = CreateThread(NULL, 0, __joystickLoop, (void*)device, 0, NULL);
	}

	unsigned long read() {
		if (!tqDataReady) {
			return 0;
		}

		memcpy(&data, &tqButtonsPrev, 4);
		data.brake_sw = decodeRotaryState(data.brake_sw);
		tqDataReady = 0;
		return 1;
	}
};