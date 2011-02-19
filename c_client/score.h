#ifndef SCORE_H_
#define SCORE_H_
#include <string>

struct score_t {
	int id;
	char nick[32];
	float score;

	bool operator<(const score_t &s) const {
		return score>s.score;
	};
};

#endif
