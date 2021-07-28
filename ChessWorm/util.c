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