#include "dsc.h"

DSC::DSC(const DSCConfig& config, Client& netClient, const MQTTConfig& mqttConfig, const HAConfig& haConfig,
	const HADevice& haDevice)
	:ha(netClient, mqttConfig, haConfig, haDevice),
	 dsc(config.wiring.clockPin, config.wiring.readPin, config.wiring.writePin), config(config),
	 trouble_status({
		 .base{
			 .name="Security Trouble"
		 },
		 .deviceClass = HABinarySensor_DeviceClass::problem
	 }),
	 dsc_status({
		 .base{
			 .name="DSC Keybus Connection"
		 },
		 .deviceClass = HABinarySensor_DeviceClass::connectivity
	 }) {

	for (int i = 0; i < dscPartitions; ++i) {
		const auto& partition_config = config.partitions[i];
		if (partition_config.name == nullptr) continue;

		auto* sensName = new String(partition_config.name);
		sensName->concat(" Status");
		auto* partition = new DSCPartition {
			.panel = new HAAlarmPanel({
					.base{
						.name = partition_config.name
					},
					.code = config.accessCode
				},
				[this, i](HAAlarm_Command cmd) {
				  onAlarmCommand(i, cmd);
				}),
			.status_sensor = new HASensor<const char*>({
				.base{
					.name = sensName->c_str()
				}
			}, ""),
			.config = partition_config
		};
		for (int j = 0; j < dscZones; ++j) {
			const auto zone_config = partition_config.zones[j];
			if (zone_config.name == nullptr) continue;
			auto* zone = new HABinarySensor(
				{
					.base {
						.name = zone_config.name,
					},
					.deviceClass = zone_config.type
				});
			partition->zones[j] = zone;
		}
		partitions[i] = partition;
	};
}

DSC::~DSC() {
	for (const auto& partition: partitions) {
		delete partition->panel;
		delete partition->status_sensor;
		for (const auto& item : partition->zones)
			delete item;
		delete partition;
	}
}

void DSC::setup(const String& mac, const String& serial) {
	std::forward_list<HAComponent*> components;

	components.push_front(&trouble_status);
	components.push_front(&dsc_status);

	for (const auto& partition : partitions) {
		if (partition == nullptr) continue;
		components.push_front(partition->panel);
		components.push_front(partition->status_sensor);
		for (const auto& zone : partition->zones) {
			if (zone == nullptr) continue;
			components.push_front(zone);
		}
	}

	ha.setup(components, mac, serial);

	dsc.begin();
}

// based on taligentx/dscKeybusInterface esp8266/HomeAssistant-MQTT example
void DSC::loop() {
	ha.loop();
	dsc.loop();

	if (!dsc.statusChanged) return; // Checks if the security system status has changed
	dsc.statusChanged = false;  // Reset the status tracking flag

	// If the Keybus data buffer is exceeded, the sketch is too busy to process all Keybus commands.  Call
	// loop() more often, or increase dscBufferSize in the library: src/dscKeybusInterface.h
	if (dsc.bufferOverflow) {
		Serial.println(F("Keybus buffer overflow"));
		dsc.bufferOverflow = false;
	}
	// Check if the interface is connected to the Keybus
	if (dsc.keybusChanged) {
		dsc.keybusChanged = false;  // Reset the Keybus data status flag
		dsc_status.setState(dsc.keybusConnected);
		if (dsc.keybusConnected)
			Serial.println(F("DSC Keybus connected"));
		else
			Serial.println(F("DSC Keybus disconnected"));
	}

	// Send the access code when needed by the panel for arming
	if (dsc.accessCodePrompt) {
		dsc.accessCodePrompt = false;
		dsc.write(config.accessCode);
	}

	if (dsc.troubleChanged) {
		dsc.troubleChanged = false;  // Reset the trouble status flag
		trouble_status.setState(dsc.trouble);
	}

	// Publishes status per partition
	for (byte partition = 0; partition < dscPartitions; partition++) {

		// Skip processing if the partition is disabled or in installer programming
		if (dsc.disabled[partition]) continue;

		// Publishes the partition status message
		setPartitionStatus(partition);

		// Publishes armed/disarmed status
		if (dsc.armedChanged[partition]) {

			if (dsc.armed[partition]) {
				if (dsc.armedAway[partition] && dsc.noEntryDelay[partition])
					partitions[partition]->panel->setState(armed_night);
				else if (dsc.armedAway[partition])
					partitions[partition]->panel->setState(armed_away);
				else if (dsc.armedStay[partition] && dsc.noEntryDelay[partition])
					partitions[partition]->panel->setState(armed_night);
				else if (dsc.armedStay[partition])
					partitions[partition]->panel->setState(armed_home);

			} else
				partitions[partition]->panel->setState(disarmed);

		}

		// Publishes exit delay status
		if (dsc.exitDelayChanged[partition]) {
			dsc.exitDelayChanged[partition] = false;  // Reset the exit delay status flag

			if (dsc.exitDelay[partition])
				partitions[partition]->panel->setState(pending);
			else if (!dsc.exitDelay[partition] && !dsc.armed[partition])
				partitions[partition]->panel->setState(disarmed);

		}

		// Publishes alarm status
		if (dsc.alarmChanged[partition]) {
			dsc.alarmChanged[partition] = false;  // Reset the partition alarm status flag

			if (dsc.alarm[partition])
				partitions[partition]->panel->setState(triggered); // Alarm tripped
			else if (!dsc.armedChanged[partition])
				partitions[partition]->panel->setState(disarmed);
		}
		if (dsc.armedChanged[partition]) dsc.armedChanged[partition] = false;  // Resets the partition armed status flag
	}

	if (dsc.openZonesStatusChanged) {
		dsc.openZonesStatusChanged = false;                           // Resets the open zones status flag
		for (byte zoneGroup = 0; zoneGroup < dscZones; zoneGroup++) {
			for (byte zoneBit = 0; zoneBit < 8; zoneBit++) {
				if (bitRead(dsc.openZonesChanged[zoneGroup], zoneBit)) {  // Checks an individual open zone status flag
					bitWrite(dsc.openZonesChanged[zoneGroup], zoneBit,
						0);  // Resets the individual open zone status flag

					partitions[zoneGroup]->zones[zoneBit]->setState(bitRead(dsc.openZones[zoneGroup], zoneBit));

				}
			}
		}
	}
}

void DSC::onAlarmCommand(byte partition, HAAlarm_Command command) {
	// Resets status if attempting to change the armed mode while armed or not ready
	if (command != disarm && !dsc.ready[partition]) {
		dsc.armedChanged[partition] = true;
		dsc.statusChanged = true;
		return;
	}

	// Arm stay
	if (command == arm_home && !dsc.armed[partition] && !dsc.exitDelay[partition]) {
		dsc.writePartition = partition;         // Sets writes to the partition number
		dsc.write('s');                             // Virtual keypad arm stay
	}

		// Arm away
	else if (command== arm_away && !dsc.armed[partition] && !dsc.exitDelay[partition]) {
		dsc.writePartition = partition;         // Sets writes to the partition number
		dsc.write('w');                             // Virtual keypad arm away
	}

		// Arm night
	else if (command == arm_night && !dsc.armed[partition] && !dsc.exitDelay[partition]) {
		dsc.writePartition = partition;         // Sets writes to the partition number
		dsc.write('n');                             // Virtual keypad arm away
	}

		// Disarm
	else if (command == disarm
		&& (dsc.armed[partition] || dsc.exitDelay[partition] || dsc.alarm[partition])) {
		dsc.writePartition = partition;         // Sets writes to the partition number
		dsc.write(config.accessCode);
	}
}

// Publishes the partition status message
void DSC::setPartitionStatus(byte partition) {
	// Publishes the current partition message
	switch (dsc.status[partition]) {
		case 0x01: partitions[partition]->status_sensor->setState("Partition ready");
			break;
		case 0x02: partitions[partition]->status_sensor->setState("Stay zones open");
			break;
		case 0x03: partitions[partition]->status_sensor->setState("Zones open");
			break;
		case 0x04: partitions[partition]->status_sensor->setState("Armed: Stay");
			break;
		case 0x05: partitions[partition]->status_sensor->setState("Armed: Away");
			break;
		case 0x06: partitions[partition]->status_sensor->setState("Armed: No entry delay");
			break;
		case 0x07: partitions[partition]->status_sensor->setState("Failed to arm");
			break;
		case 0x08: partitions[partition]->status_sensor->setState("Exit delay in progress");
			break;
		case 0x09: partitions[partition]->status_sensor->setState("Arming with no entry delay");
			break;
		case 0x0B: partitions[partition]->status_sensor->setState("Quick exit in progress");
			break;
		case 0x0C: partitions[partition]->status_sensor->setState("Entry delay in progress");
			break;
		case 0x0D: partitions[partition]->status_sensor->setState("Entry delay after alarm");
			break;
		case 0x0E: partitions[partition]->status_sensor->setState("Function not available");
			break;
		case 0x10: partitions[partition]->status_sensor->setState("Keypad lockout");
			break;
		case 0x11: partitions[partition]->status_sensor->setState("Partition in alarm");
			break;
		case 0x12: partitions[partition]->status_sensor->setState("Battery check in progress");
			break;
		case 0x14: partitions[partition]->status_sensor->setState("Auto-arm in progress");
			break;
		case 0x15: partitions[partition]->status_sensor->setState("Arming with bypassed zones");
			break;
		case 0x16: partitions[partition]->status_sensor->setState("Armed: No entry delay");
			break;
		case 0x17: partitions[partition]->status_sensor->setState("Power saving: Keypad blanked");
			break;
		case 0x19: partitions[partition]->status_sensor->setState("Disarmed: Alarm memory");
			break;
		case 0x22: partitions[partition]->status_sensor->setState("Disarmed: Recent closing");
			break;
		case 0x2F: partitions[partition]->status_sensor->setState("Keypad LCD test");
			break;
		case 0x33: partitions[partition]->status_sensor->setState("Command output in progress");
			break;
		case 0x3D: partitions[partition]->status_sensor->setState("Disarmed: Alarm memory");
			break;
		case 0x3E: partitions[partition]->status_sensor->setState("Partition disarmed");
			break;
		case 0x40: partitions[partition]->status_sensor->setState("Keypad blanked");
			break;
		case 0x8A: partitions[partition]->status_sensor->setState("Activate stay/away zones");
			break;
		case 0x8B: partitions[partition]->status_sensor->setState("Quick exit");
			break;
		case 0x8E: partitions[partition]->status_sensor->setState("Function not available");
			break;
		case 0x8F: partitions[partition]->status_sensor->setState("Invalid access code");
			break;
		case 0x9E: partitions[partition]->status_sensor->setState("Enter * function key");
			break;
		case 0x9F: partitions[partition]->status_sensor->setState("Enter access code");
			break;
		case 0xA0: partitions[partition]->status_sensor->setState("*1: Zone bypass");
			break;
		case 0xA1: partitions[partition]->status_sensor->setState("*2: Trouble menu");
			break;
		case 0xA2: partitions[partition]->status_sensor->setState("*3: Alarm memory");
			break;
		case 0xA3: partitions[partition]->status_sensor->setState("*4: Door chime enabled");
			break;
		case 0xA4: partitions[partition]->status_sensor->setState("*4: Door chime disabled");
			break;
		case 0xA5: partitions[partition]->status_sensor->setState("Enter master code");
			break;
		case 0xA6: partitions[partition]->status_sensor->setState("*5: Access codes");
			break;
		case 0xA7: partitions[partition]->status_sensor->setState("*5: Enter new 4-digit code");
			break;
		case 0xA9: partitions[partition]->status_sensor->setState("*6: User functions");
			break;
		case 0xAA: partitions[partition]->status_sensor->setState("*6: Time and date");
			break;
		case 0xAB: partitions[partition]->status_sensor->setState("*6: Auto-arm time");
			break;
		case 0xAC: partitions[partition]->status_sensor->setState("*6: Auto-arm enabled");
			break;
		case 0xAD: partitions[partition]->status_sensor->setState("*6: Auto-arm disabled");
			break;
		case 0xAF: partitions[partition]->status_sensor->setState("*6: System test");
			break;
		case 0xB0: partitions[partition]->status_sensor->setState("*6: Enable DLS");
			break;
		case 0xB2:
		case 0xB3: partitions[partition]->status_sensor->setState("*7: Command output");
			break;
		case 0xB7: partitions[partition]->status_sensor->setState("Enter installer code");
			break;
		case 0xB8: partitions[partition]->status_sensor->setState("Enter * function key while armed");
			break;
		case 0xB9: partitions[partition]->status_sensor->setState("*2: Zone tamper menu");
			break;
		case 0xBA: partitions[partition]->status_sensor->setState("*2: Zones with low batteries");
			break;
		case 0xBC: partitions[partition]->status_sensor->setState("*5: Enter new 6-digit code");
			break;
		case 0xBF: partitions[partition]->status_sensor->setState("*6: Auto-arm select day");
			break;
		case 0xC6: partitions[partition]->status_sensor->setState("*2: Zone fault menu");
			break;
		case 0xC8: partitions[partition]->status_sensor->setState("*2: Service required menu");
			break;
		case 0xCD: partitions[partition]->status_sensor->setState("Downloading in progress");
			break;
		case 0xCE: partitions[partition]->status_sensor->setState("Active camera monitor selection");
			break;
		case 0xD0: partitions[partition]->status_sensor->setState("*2: Keypads with low batteries");
			break;
		case 0xD1: partitions[partition]->status_sensor->setState("*2: Keyfobs with low batteries");
			break;
		case 0xD4: partitions[partition]->status_sensor->setState("*2: Sensors with RF delinquency");
			break;
		case 0xE4: partitions[partition]->status_sensor->setState("*8: Installer programming, 3 digits");
			break;
		case 0xE5: partitions[partition]->status_sensor->setState("Keypad slot assignment");
			break;
		case 0xE6: partitions[partition]->status_sensor->setState("Input: 2 digits");
			break;
		case 0xE7: partitions[partition]->status_sensor->setState("Input: 3 digits");
			break;
		case 0xE8: partitions[partition]->status_sensor->setState("Input: 4 digits");
			break;
		case 0xE9: partitions[partition]->status_sensor->setState("Input: 5 digits");
			break;
		case 0xEA: partitions[partition]->status_sensor->setState("Input HEX: 2 digits");
			break;
		case 0xEB: partitions[partition]->status_sensor->setState("Input HEX: 4 digits");
			break;
		case 0xEC: partitions[partition]->status_sensor->setState("Input HEX: 6 digits");
			break;
		case 0xED: partitions[partition]->status_sensor->setState("Input HEX: 32 digits");
			break;
		case 0xEE: partitions[partition]->status_sensor->setState("Input: 1 option per zone");
			break;
		case 0xEF: partitions[partition]->status_sensor->setState("Module supervision field");
			break;
		case 0xF0: partitions[partition]->status_sensor->setState("Function key 1");
			break;
		case 0xF1: partitions[partition]->status_sensor->setState("Function key 2");
			break;
		case 0xF2: partitions[partition]->status_sensor->setState("Function key 3");
			break;
		case 0xF3: partitions[partition]->status_sensor->setState("Function key 4");
			break;
		case 0xF4: partitions[partition]->status_sensor->setState("Function key 5");
			break;
		case 0xF5: partitions[partition]->status_sensor->setState("Wireless module placement test");
			break;
		case 0xF6: partitions[partition]->status_sensor->setState("Activate device for test");
			break;
		case 0xF7: partitions[partition]->status_sensor->setState("*8: Installer programming, 2 digits");
			break;
		case 0xF8: partitions[partition]->status_sensor->setState("Keypad programming");
			break;
		case 0xFA: partitions[partition]->status_sensor->setState("Input: 6 digits");
			break;
		default: return;
	}
}
