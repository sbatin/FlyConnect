#include <string>

enum GROUP_ID {
	GROUP_0,
};

void Joystick_MapButton(HANDLE hSimConnect, char button, SIMCONNECT_CLIENT_EVENT_ID eventId) {
	using namespace std;
	auto name = "joystick:2:button:" + to_string(static_cast<long long>(button));

	// Set up a private event and add it to the group
	SimConnect_MapClientEventToSimEvent(hSimConnect, eventId);
	SimConnect_AddClientEventToNotificationGroup(hSimConnect, GROUP_0, eventId);

	// Map input event to the private client event
	SimConnect_MapInputEventToClientEvent(hSimConnect, button, name.c_str(), eventId);
	SimConnect_SetInputGroupState(hSimConnect, button, SIMCONNECT_STATE_ON);
}

void Joystick_MapEvents(HANDLE hSimConnect) {
	SimConnect_SetNotificationGroupPriority(hSimConnect, GROUP_0, SIMCONNECT_GROUP_PRIORITY_HIGHEST);

	Joystick_MapButton(hSimConnect, 5, EVENT_INPUT_SW1);
	Joystick_MapButton(hSimConnect, 6, EVENT_INPUT_SW2);
	Joystick_MapButton(hSimConnect, 7, EVENT_INPUT_SW3);
	Joystick_MapButton(hSimConnect, 8, EVENT_INPUT_SW4);
	Joystick_MapButton(hSimConnect, 9, EVENT_INPUT_SW5);
	Joystick_MapButton(hSimConnect, 10, EVENT_INPUT_SW6);
	Joystick_MapButton(hSimConnect, 11, EVENT_INPUT_FLAPS_1);
	Joystick_MapButton(hSimConnect, 12, EVENT_INPUT_FLAPS_UP);
	Joystick_MapButton(hSimConnect, 13, EVENT_INPUT_FLAPS_10);
	Joystick_MapButton(hSimConnect, 14, EVENT_INPUT_FLAPS_15);
	Joystick_MapButton(hSimConnect, 15, EVENT_INPUT_FLAPS_2);
	Joystick_MapButton(hSimConnect, 16, EVENT_INPUT_FLAPS_5);
	Joystick_MapButton(hSimConnect, 17, EVENT_INPUT_FLAPS_30);
	Joystick_MapButton(hSimConnect, 18, EVENT_INPUT_FLAPS_25);
	Joystick_MapButton(hSimConnect, 32, EVENT_INPUT_MODE_SW);
}