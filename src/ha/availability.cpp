#include "availability.h"
#include "utils.h"

void HAAvailability::serialize(const JsonObject& json) const {
	json["topic"] = topic;
	jsonFromOptional(json, "pl_avail", HA_DEFAULT_ONLINE, payload_available);
	jsonFromOptional(json, "pl_not_avail", HA_DEFAULT_OFFLINE, payload_not_available);
}

void serializeAvailabilityMode(const JsonObject& json, const HAAvailabilityMode mode) {
	switch (mode) {
		case all: json["avty_mode"] = toSTR(all);
			break;
		case any: json["avty_mode"] = toSTR(any);
			break;
		case latest: //HA DEFAULT - NOOP
			break;
	}
}
