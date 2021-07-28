#include "util.h"

int StringsEqual (char* a, char* b) {
	int index = 0;
	while (a[index] == b[index]) {
		if (a[index] == 0 || b[index] == 0)
			return 1;
		index++;
	}
	if (a[index] == '\n' || b[index] == '\n')
		return 1;
	return 0;
}

int StringStartsWith (char* string, char* prefix) {
	int index = 0;
	while (string[index] == prefix[index]) {
		if (prefix[index] == 0)
			return 1;
		if (string[index] == 0)
			return 0;
		index++;
	}
	if (prefix[index] == 0)
		return 1;
	return 0;
}