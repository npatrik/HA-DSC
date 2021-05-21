#ifndef HA_MQTT_UTILS_H
#define HA_MQTT_UTILS_H

#include <ArduinoJson.h>

#define toSTR(thing) # thing

void buildSTR(String& out, std::initializer_list<const char*> sources);

template<class T>
void jsonFromOptional(const JsonObject& json, const char* name, std::optional<T> opt, T fallbackValue) {
	json[name] = opt.value_or(fallbackValue);
}

template<class T>
void jsonFromOptional(const JsonObject& json, const char* name, T defaultValue, std::optional<T> opt,
	T fallbackValue) {
	if (opt.has_value() && opt.value() != defaultValue)
		jsonFromOptional(json, name, opt, fallbackValue);
}

template<class T>
void jsonFromOptional(const JsonObject& json, const char* name, std::optional<T> opt) {
	if (opt.has_value())
		json[name] = opt.value();
}

template<class T>
void jsonFromOptional(const JsonObject& json, const char* name, T defaultValue, std::optional<T> opt) {
	if (opt.has_value() && opt.value() != defaultValue)
		jsonFromOptional(json, name, opt);
}

#endif //HA_MQTT_UTILS_H
