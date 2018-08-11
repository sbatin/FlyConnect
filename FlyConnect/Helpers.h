
unsigned char reverseByte(char a) {
	return
		((a & 0x1)  << 7) | ((a & 0x2)  << 5) |
		((a & 0x4)  << 3) | ((a & 0x8)  << 1) |
		((a & 0x10) >> 1) | ((a & 0x20) >> 3) |
		((a & 0x40) >> 5) | ((a & 0x80)  >> 7);
}

unsigned short setBit(unsigned short mask, bool value) {
	return value ? mask : 0;
}

template <typename T>
unsigned char toButtonState(T buttonsState, T mask0, T mask2) {
	if ((buttonsState & mask0) == 0) return 0;
	if ((buttonsState & mask2) == 0) return 2;
	return 1;
}

template <typename T>
unsigned char decodeRotaryState(T buttonState, T mask, unsigned char maxValue) {
	unsigned char result = 1;

	while (result <= maxValue) {
		if (buttonState & mask) return result;
		result++;
		mask = mask << 1;
	}

	return 0;
}