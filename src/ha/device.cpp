#include "device.h"
#include "utils.h"
#include "homeassistant.h"

void HADevice::serialize(const JsonObject& json) const {
	JsonObject obj = json.createNestedObject("dev");
	obj["name"] = name;
	jsonFromOptional(obj, "mf", manufacturer);
	jsonFromOptional(obj, "mdl", model);
	jsonFromOptional(obj, "sw", sw_version, SW_VERSION);
	jsonFromOptional(obj, "via_device", via_device);
	if (!identifiers.empty()) {
		if (std::distance(identifiers.begin(), identifiers.end()) == 1) {
			obj["ids"] = identifiers.front();
		} else {
			JsonArray arr = obj.createNestedArray("ids");
			for (const auto& id : identifiers)
				arr.add(id);
		}
	}
	if (!connections.empty()) {
		JsonArray arr = obj.createNestedArray("cns");
		for (const auto& con : connections) {
			JsonArray conarr = arr.createNestedArray();
			conarr.add(con.type);
			conarr.add(con.identifier);
		}
	}
}
