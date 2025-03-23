#include <stdio.h>
#include <stdlib.h>

#include "../Inc/bencode.h"
#include "../Inc/bencode_utils.h"

int main(int argc, char** argv) {

	uint8_t r;
	char *fp = argv[1];

	struct bm *b = (struct bm*)malloc(sizeof(struct bm));

	r = p(fp, b);

	if (r != 0) {
		printf("Error parsing file\n "
			"If you believe this an error ... "
			"please submit an issue to the github with your "
			"... and file attempted to parse");
		return -1;
	}

	r = fbm(b);
	printf("Success!\n");
	return 0;
}
