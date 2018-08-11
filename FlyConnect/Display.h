#define DISP_OPTS_MINUS 0xC
#define DISP_OPTS_0_DP  0xD
#define DISP_OPTS_1_DP  0xE
#define DISP_OFF_MASK   0xFFFFFFFF
#define DISP_NO_L_VALUE (DISP_OFF_MASK >> 12)

typedef unsigned __int32 uint32_t;

uint32_t dec2bcd(int value, bool leading_zeros = false, uint32_t zero_value = 0) {
	if (value == 0) {
		return zero_value;
	}

	unsigned int dec = abs(value);

	uint32_t result = 0;
    unsigned char shift = 0;

    while (dec) {
        result |= (dec % 10) << shift;
        shift+= 4;
		dec = dec / 10;
    }

	if (value < 0) {
		result |= DISP_OPTS_MINUS << shift;
		shift+= 4;
	}

	if (!leading_zeros) {
		result |= DISP_OFF_MASK << shift;
	}

    return result;
}

uint32_t float2bcd(float value, float precision = 100) {
	float integral;
	float fractional = floorf(precision * modff(value, &integral) + 0.5f);

	uint32_t result = dec2bcd((int)fractional, true);
	unsigned char shift = 4 * (unsigned char)log10(precision);

	result |= (integral > 0 ? DISP_OPTS_1_DP : DISP_OPTS_0_DP) << shift;
	shift+= 4;

    return result | (DISP_OFF_MASK << shift);
}

uint32_t displayHi(short value) {
	return (dec2bcd(value, true) << 20) | (DISP_OFF_MASK >> 12);
}

uint32_t displayLo(int value, uint32_t zero_value = 0) {
	return dec2bcd(value, false, zero_value) & 0xFFFFF | 0xFFF00000;
}

uint32_t displayLo(float value) {
	return float2bcd(value) & 0xFFFFF | 0xFFF00000;
}

void display_test1(int x) {
	printf("value = %d, bcd = %x\n", x, dec2bcd(x));
}

void display_test2(float x) {
	printf("value = %f, bcd = %x\n", x, float2bcd(x));
}

void display_test() {
	display_test1(0);
	display_test1(-10);
	display_test1(-100);
	display_test1(-1000);
	display_test1(10);
	display_test1(100);
	display_test1(1000);
	display_test1(5);
	display_test1(50);
	display_test1(500);
	display_test1(-2400);
	display_test2(0.00f);
	display_test2(0.05f);
	display_test2(0.78f);
	display_test2(1.15f);
	display_test2(2.41f);
}