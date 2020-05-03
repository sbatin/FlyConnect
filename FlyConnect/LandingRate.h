#pragma once

static const int a3xx_ratings_v[] = { 180, 240, 600, 830, 10000 };
static const char *a3xx_ratings_s[] = {
	"Smooth landing",
	"Firm landing",
	"Uncomfortable landing",
	"Hard landing, requires inspection",
	"Severe hard landing, damage likely"
};

class LandingRate {
public:
	static const char* getA3xxRating(unsigned short vertSpeed) {
		for (int i = 0; i < sizeof(a3xx_ratings_v); i++) {
			if (vertSpeed < a3xx_ratings_v[i]) {
				return a3xx_ratings_s[i];
			}
		}

		return "N/A";
	}
};