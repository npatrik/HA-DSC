#include "utils.h"

//append all sources to a preallocated string with only 1 heap reallocation
void buildSTR(String& out, const std::initializer_list<const char*> sources) {
	size_t len = 0;

	for (auto it = begin(sources); it != end(sources); ++it)
		len += strlen(*it);

	out.reserve(len + 1);
	for (auto it = begin(sources); it != end(sources); ++it)
		out += *it;
}
