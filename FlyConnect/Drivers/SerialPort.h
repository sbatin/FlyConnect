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

#define SERAIL_RX_BUFFER_LENGTH 255
#define SERAIL_PACKET_SIGNATURE 0x7E
#define SERAIL_REPLACE_SIGNATURE 0x7D

class SerialPort {
private:
	HANDLE handle;
	bool connected;

	unsigned char uart_rx_buffer[SERAIL_RX_BUFFER_LENGTH];
	unsigned char uart_counter;
	unsigned char uart_replace;
	unsigned char uart_started;
	unsigned char uart_received;

	int send(char* bytesData, int bytesCount) {
		if (!connected) return 0;

		DWORD bytes_written = 0;
		return WriteFile(handle, bytesData, bytesCount, &bytes_written, NULL);
	}
public:
	SerialPort() {
		uart_counter = 0;
		uart_replace = 0;
		uart_started = 0;
		uart_received = 0;
		connected = false;
	};

	~SerialPort(void) {};

	int connect(const wchar_t* path);
	int close();
	template<typename T> void sendData(T* data);
	template<typename T> int readData(T* dest);

	char* readMessage() {
		if (!connected) return NULL;

		DWORD iSize;
		unsigned char sReceived;
		unsigned char counter = 0;

		while (1) {
			ReadFile(handle, &sReceived, 1, &iSize, 0);
			if (iSize > 0) {
				uart_rx_buffer[counter++] = sReceived;
				if (sReceived == '\0') {
					void *s = malloc(counter);
					memcpy(s, uart_rx_buffer, counter);
					return (char*)s;
				}
			}

			Sleep(1);
		}

		return NULL;
	}
};

int SerialPort::connect(const wchar_t* path) {
	DCB dcbSerialParams = {0};
	COMMTIMEOUTS timeouts = {0};

	handle = CreateFile(path, GENERIC_READ|GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);

	if (handle == INVALID_HANDLE_VALUE) {
		printf("Error opening serial port %d\n", GetLastError());
		return 0;
	}

	// Set device parameters (19200 baud, 1 start bit, 1 stop bit, no parity)
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	if (GetCommState(handle, &dcbSerialParams) == 0) {
		printf("Error getting device state\n");
		CloseHandle(handle);
		return 0;
	}

	dcbSerialParams.BaudRate = CBR_19200;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;
	if (SetCommState(handle, &dcbSerialParams) == 0) {
		printf("Error setting device parameters\n");
		CloseHandle(handle);
		return 0;
	}

	// Set COM port timeout settings
	timeouts.ReadIntervalTimeout = 0xFFFFFFFF;
	timeouts.ReadTotalTimeoutConstant = 0;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 0;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	/*timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;*/
	if (SetCommTimeouts(handle, &timeouts) == 0) {
		printf("Error setting timeouts\n");
		CloseHandle(handle);
		return 0;
	}

	connected = true;
	return 1;
}

int SerialPort::close() {
	if (!connected) return 0;

	if (CloseHandle(handle) == 0) {
		printf("Error closing serial port\n");
		return 0;
	}

	connected = false;
	return 1;
}

template<typename T>
void SerialPort::sendData(T* data) {
	char uart_tx_buffer[50];
	char uart_tx_count = 0;
	char* p = (char*)data;

	uart_tx_buffer[uart_tx_count++] = SERAIL_PACKET_SIGNATURE;

	for (int i = 0; i < sizeof(T); i++) {
		if (p[i] == SERAIL_PACKET_SIGNATURE) {
			uart_tx_buffer[uart_tx_count++] = SERAIL_REPLACE_SIGNATURE;
			uart_tx_buffer[uart_tx_count++] = 0x5E;
		} else if (p[i] == SERAIL_REPLACE_SIGNATURE) {
			uart_tx_buffer[uart_tx_count++] = SERAIL_REPLACE_SIGNATURE;
			uart_tx_buffer[uart_tx_count++] = 0x5D;
		} else {
			uart_tx_buffer[uart_tx_count++] = p[i];
		}
	}

	uart_tx_buffer[uart_tx_count++] = SERAIL_PACKET_SIGNATURE;
	send(uart_tx_buffer, uart_tx_count);
}

template<typename T>
int SerialPort::readData(T* dest) {
	if (!connected) return 0;

	DWORD iSize;
	unsigned char sBuff[SERAIL_RX_BUFFER_LENGTH];
	unsigned char sReceived;

	ReadFile(handle, sBuff, SERAIL_RX_BUFFER_LENGTH, &iSize, 0);

	if (iSize > 0) {
		printf("Size = %d, received = ", iSize);
		for (DWORD i = 0; i < iSize; i++) {
			printf("%02X ", sBuff[i]);
		}

		printf("\n");

		for (DWORD i = 0; i < iSize; i++) {
			sReceived = sBuff[i];

			// start byte detected, reset UART buffer
			if (sReceived == SERAIL_PACKET_SIGNATURE) {
				// whole packet has been received
				if (uart_started && i == iSize - 1) {
					uart_started = 0;
					memcpy(dest, (const void*)uart_rx_buffer, sizeof(T));
					return 1;
				} else {
					uart_counter = 0;
					uart_replace = 0;
					uart_started = 1;
				}
			// replace indicator detected
			} else if (sReceived == SERAIL_REPLACE_SIGNATURE) {
				uart_replace = 1;
			// append byte to buffer
			} else if (uart_counter < SERAIL_RX_BUFFER_LENGTH) {
				if (uart_replace) {
					uart_replace = 0;
					sReceived ^= 32;
				}
				uart_rx_buffer[uart_counter++] = sReceived;
			}
		}
	}
	
	return 0;
}