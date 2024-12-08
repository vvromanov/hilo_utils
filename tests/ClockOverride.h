#include "time_utils.h"
class ClockOverride {
public:
	ClockOverride() {
		clock_override = true;
		clock_override_us = getClockUs();
	}
	~ClockOverride() {
		clock_override = false;
	}
	void AddSeconds(int seconds) {
		clock_override_us += seconds * (int64_t)1000'000;
	}
};