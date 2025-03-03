#include <stdio.h>
#include <stdlib.h>

#include "../Inc/bencode.h"

int main(int argc, char** argv) {

	int result;
	char *filepath = argv[1];

	struct bencode_module *bencode = (struct bencode_module*)malloc(sizeof(struct bencode_module));

	result = parse_single(filepath, bencode);
	if (result != 0) {
		printf("Error parsing file\n");
		return -1;
	}

	printf("Success!\n");
	result = free_bencode_module(bencode);	
	return 0;
}
