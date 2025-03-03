#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "../Inc/bencode.h"

#define BUFFER_SIZE 128
#define ANNOUNCE_LIST_SIZE 256
#define INFO_FILE_SIZE 128
#define FILE_PATH_SIZE 10
#define URL_LIST_SIZE 1

int parse_single(char *filepath, struct bencode_module* bencode) {

	char file_char;
	int result;
	id type;
	
	FILE *file = fopen(filepath, "r");

	/* Struct initialization */
	bencode->buffer_size 				= BUFFER_SIZE;
	bencode->announce_list_size			= ANNOUNCE_LIST_SIZE;
	bencode->info_file_size				= INFO_FILE_SIZE;
	bencode->file_path_size				= FILE_PATH_SIZE;
	bencode->url_list_size				= URL_LIST_SIZE;

	bencode->announce 					= NULL;
	bencode->announce_list 				= NULL;
	bencode->comment 					= NULL;
	bencode->created_by 				= NULL;
	bencode->encoding 					= NULL;
	bencode->info 						= NULL;
	bencode->url_list 					= NULL;
	bencode->info_hash					= NULL;
	bencode->info_hash_human_readable 	= NULL;

	bencode->head_pointer 				= NULL;
	bencode->size_pointer 				= NULL;
	bencode->announce_list_index 		= 0;
	bencode->info_file_index 			= 0;
	bencode->file_path_index 			= 0;
	bencode->url_list_index 			= 0;

	/* Error checking for existence of file */
	if (file == NULL) {
		fprintf(stderr, "Error reading from file: File not found\n");
		return -1;
	}

	/* Checking first character of file for dictionary */	
	file_char = fgetc(file);
	type = identify(file_char);

	/* Error checking for presence of dictionary */	
	if (type != &dictionary) {
		printf("Parse error: First character was not the beginning of a dictionary\n");
		return -1;
	}

	/* Allocating buffer for reading in and evaluating file contents */
	bencode->buffer = (char *)malloc(bencode->buffer_size * sizeof(char));

	/* Running function pointed to by function pointer 'type' */
	result = type(bencode, file);
	fclose(file);
	
	if (result == 0) {
		return 0;
	} else {
		switch (result) {
			case -1:
				printf("some error\n");
				break;
			default:
				printf("unknown error\n");
				break;
		}		

		return -1;
	}
}

int dictionary(struct bencode_module *bencode, FILE *file) {
	
	unsigned int result, buffer_index = 0;
	char file_char;
	id type;

	unsigned int return_size;
	long long int length;

	while (buffer_index < bencode->buffer_size) {

		file_char = fgetc(file);

		type = identify(file_char);
		
		/* If the beginning of a bencode object is detected */
		if (type != NULL) {

			/* Running function associated with detected type */
			result = type(bencode, file);

			/* If received character for end of dictionary, return */
			if (result == END_OF_TYPE) {
				return PARSE_SUCCESS;
			}

			bencode->head_pointer = NULL;
			buffer_index = 0;
		} else {
			if (file_char == ':') {
				bencode->buffer[buffer_index] = '\0';

				result = verify_int(bencode->buffer, &length);

				if (result != CONVERSION_SUCCESS) {
					printf("Parse error: Length of data element could not be determined.\nPlease verify the integrity of your .torrent file.\n");
					return PARSE_FAILURE;
				}			

				/* Expanding buffer to accomodate item length */
				if (length > bencode->buffer_size) {	
					
					/* Doubling buffer size until it can fit data */
					while (length > bencode->buffer_size) {
						bencode->buffer_size = bencode->buffer_size * 2;	
					}
					bencode->buffer = realloc(bencode->buffer, bencode->buffer_size);
				}

				return_size = fread(bencode->buffer, 1, length, file);
				bencode->buffer[length] = '\0';
				
				/* Error handling for item read */
				if (return_size != length) {	
					printf("Parse error: Could not capture full data segment. Please verify the integrity of your .torrent file\n");
					return DATA_LENGTH_EXCEEDED;
				}

				/* If member of struct to place data hasn't been set, we need to set it */
				if (bencode->head_pointer == NULL) {
					parse_key(bencode, file);
				} else {
				
					/* If not looking for key, store buffer as value */
					if (bencode->head_pointer != (void *)IGNORE_FLAG) {
						strcpy((char *)bencode->head_pointer, bencode->buffer);
						
/*
						if (bencode->info != NULL) {
							if (bencode->head_pointer == (void *)bencode->info->pieces) {
								
								size_t len_read;
								long int info_size;
								char* info_buffer;
	
								bencode->info_end = ftell(file);							
	
								info_size = bencode->info_end - bencode->info_start + 1;
								info_buffer = (char *)malloc(info_size * sizeof(char));
								fseek(file, bencode->info_start, SEEK_SET);	
								len_read = fread(info_buffer, 1, info_size, file);
					
								//sha1(bencode, info_buffer, &len_read);
								
								fseek(file, bencode->info_end, SEEK_SET);

								free(info_buffer);
							}	
						}
*/								

						bencode->head_pointer = NULL;
					}
				}

				buffer_index = 0;	
			} else {
				bencode->buffer[buffer_index] = file_char;
				buffer_index++;
			}
		}
	}
	return BUFFER_EXCEEDED;
}

int list(struct bencode_module *bencode, FILE *file) {
	
	unsigned int result, buffer_index = 0;
	char file_char;
	id type;
	
	unsigned int return_size;
	long long int length;

	while (buffer_index < bencode->buffer_size)	{
		file_char = fgetc(file);

		type = identify(file_char);
		
		if (type != NULL) {
			
			result = type(bencode, file);
			
			if (result == END_OF_TYPE) {
				return PARSE_SUCCESS;
			}
			
			buffer_index = 0;
		} else {
			if (file_char == ':') {
				bencode->buffer[buffer_index] = '\0';
				
				result = verify_int(bencode->buffer, &length);
				
				if (result != CONVERSION_SUCCESS) {
					printf("Parse error: Length of data element could not be determined.\nPlease verify the integrity of your .torrent file.\n");
					return PARSE_FAILURE;
				}

				return_size = fread(bencode->buffer, 1, length, file);
				bencode->buffer[length] = '\0';

				if (return_size != length) {
					printf("Parse error: Could not capture full segment.\nPlease verify the integrity of your .torrent file.\n");
					return DATA_LENGTH_EXCEEDED;
				}

				if (bencode->head_pointer != (void *)IGNORE_FLAG) {

					/* Extending size of pointer array if necessary. */
					/* Not functional as realloc not preserving properly... */	
					/*
					if (*bencode->index_pointer == *bencode->size_pointer) {
						*bencode->size_pointer *= 2;
						bencode->head_pointer = realloc((char **)bencode->head_pointer, *bencode->size_pointer);
					}
					*/

					((char **)bencode->head_pointer)[*bencode->index_pointer] = (char *)malloc(BUFFER_SIZE * sizeof(char));
					strcpy(((char **)bencode->head_pointer)[*bencode->index_pointer], bencode->buffer);
				
					(*bencode->index_pointer)++;
				}

				buffer_index = 0;
			} else {
				bencode->buffer[buffer_index] = file_char;
				buffer_index++;
			}
		}
	}
	return BUFFER_EXCEEDED;
}


/* rewrite to remove type checking since integer cannot be nested */
int integer(struct bencode_module *bencode, FILE *file) {

	unsigned int result, buffer_index = 0;
	char file_char;
	id type;

	while (buffer_index < bencode->buffer_size) {
		file_char = fgetc(file);
		
		/* Identify if character signifies beginning of type */
		type = identify(file_char);

		if (type != NULL) {
	
			/* Running function to parse detected type */
			result = type(bencode, file);
			
			/* If captured character which signifies end of type, check and store value */
			if (result == END_OF_TYPE) {
				bencode->buffer[buffer_index] = '\0';
				if (bencode->head_pointer != (void *)IGNORE_FLAG) {
					result = verify_int(bencode->buffer, bencode->head_pointer);	

					/* Error checking for conversion */					
					if (result != CONVERSION_SUCCESS) {
						printf("Parse error: Length of data element could not be determined.\nPlease verify the integrity of your .torrent file.\n");
						return PARSE_FAILURE;
					}

				}
				return PARSE_SUCCESS;
			}
		} else {
			bencode->buffer[buffer_index] = file_char;
			buffer_index++;
		}
	}
	return BUFFER_EXCEEDED;
}

int verify_int(char *input, long long int *output) {
		
	long long int val = 0;
	errno = 0;

	/* Performing strtoull on string and then checking errno and output buffers to check if valid integer */
	val = strtoull(input, NULL, 10);
	if ((errno == ERANGE && (val == LLONG_MAX || val == LLONG_MIN)) || (errno != 0 && val == 0)) {
		perror("strtol");
		return CONVERSION_FAILED;
	}

	/* Setting value of second parameter to parsed value of integer */
	*(long long int *)output = val;

	return CONVERSION_SUCCESS;
}

int end(struct bencode_module *bencode __attribute__((unused)), FILE *file __attribute__((unused))) {
	return END_OF_TYPE;
}

void parse_key(struct bencode_module *bencode, FILE *file) {
	if (strcmp(bencode->buffer, "announce") == 0) {
		
		bencode->announce = (char *)malloc(BUFFER_SIZE * sizeof(char));
		bencode->head_pointer = (void *)bencode->announce;
	
	} else if (strcmp(bencode->buffer, "announce-list") == 0) {

		bencode->announce_list = (char **)malloc(bencode->announce_list_size * sizeof(char *));
		bencode->head_pointer = (void *)bencode->announce_list;
		bencode->index_pointer = &bencode->announce_list_index;
		bencode->size_pointer = &bencode->announce_list_size;
	
	} else if (strcmp(bencode->buffer, "comment") == 0) {
		
		bencode->comment = (char *)malloc(BUFFER_SIZE * sizeof(char));
		bencode->head_pointer = (void *)bencode->comment;
	
	} else if (strcmp(bencode->buffer, "created by") == 0) {
			
		bencode->created_by = (char *)malloc(BUFFER_SIZE * sizeof(char));
		bencode->head_pointer = (void *)bencode->created_by;
	
	} else if (strcmp(bencode->buffer, "creation date") == 0) {
			
		bencode->creation_date = (long long int *)malloc(sizeof(long long int));
		bencode->head_pointer = (void *)bencode->creation_date;

	} else if (strcmp(bencode->buffer, "encoding") == 0) {
		
		bencode->encoding = (char *)malloc(BUFFER_SIZE * sizeof(char));
		bencode->head_pointer = (void *)bencode->encoding;
		
	} else if (strcmp(bencode->buffer, "info") == 0) {

		bencode->info = (struct bencode_info *)malloc(sizeof(struct bencode_info));
		bencode->info->files = NULL;
		bencode->info->pieces = NULL;

		bencode->info_start = ftell(file);
		bencode->head_pointer = NULL;

	} else if (strcmp(bencode->buffer, "files") == 0) {
			
		bencode->info->files = (struct info_file **)malloc(bencode->info_file_size * sizeof(struct info_file *));

	} else if (strcmp(bencode->buffer, "length") == 0) {
			
		if (bencode->info->files != NULL) {
			/* Multi-file contents */
			bencode->info->files[bencode->info_file_index] = (struct info_file *)malloc(sizeof(struct info_file));
			bencode->info->files[bencode->info_file_index]->length = (unsigned long int *)malloc(sizeof(unsigned long int));
			bencode->head_pointer = (void *)bencode->info->files[bencode->info_file_index]->length;
		} else {
			/* Single file contents */
			bencode->info->length = (long unsigned int *)malloc(sizeof(long int));
			bencode->head_pointer= (void *)bencode->info->length;
		}						

	} else if (strcmp(bencode->buffer, "path") == 0) {
		
		bencode->info->files[bencode->info_file_index]->file_path_index = 0;
		bencode->info->files[bencode->info_file_index]->path = (char **)malloc(bencode->file_path_size * sizeof(char *));
		bencode->head_pointer = (void *)bencode->info->files[bencode->info_file_index]->path;
		bencode->index_pointer = &bencode->info->files[bencode->info_file_index]->file_path_index;
		bencode->info_file_index++;

	} else if (strcmp(bencode->buffer, "name") == 0) {
			
		bencode->info->name = (char *)malloc(BUFFER_SIZE * sizeof(char));
		bencode->head_pointer = (void *)bencode->info->name;
		
	} else if (strcmp(bencode->buffer, "piece length") == 0) {
		
		bencode->info->piece_length = (long unsigned int *)malloc(sizeof(long int));
		bencode->head_pointer = (void *)bencode->info->piece_length;
		
	} else if (strcmp(bencode->buffer, "pieces") == 0) {

		bencode->info->pieces = (char *)malloc(*bencode->info->piece_length * sizeof(char));
		bencode->head_pointer = (void *)bencode->info->pieces;
		
	} else if (strcmp(bencode->buffer, "url-list") == 0) {
			
		bencode->url_list = (char **)malloc(bencode->url_list_size * sizeof(char *));
		bencode->head_pointer = (void *)bencode->url_list;
		bencode->index_pointer = &bencode->url_list_index;
	
	} else {
		/* Setting pointer to effective NULL value to then have unexpected key-value pairs ignored */
		bencode->head_pointer = (void *)IGNORE_FLAG;	
	}
}
/*
int sha1(struct bencode_module *bencode, char *info_dict, size_t *len) {

	EVP_MD_CTX *ctx;
	ctx = EVP_MD_CTX_new();

	const EVP_MD *md;
	md = EVP_get_digestbyname("sha1");
	
	if(!md) {
       printf("Unknown message digest %s\n", "sha1");
	}

	EVP_DigestInit_ex(ctx, md, NULL);

	need to create defines for v1 and v2 lengths, 40 and 64 characters depending on sha1 sha256

	unsigned char md_value[40];
	unsigned int md_len;
	EVP_DigestUpdate(ctx, info_dict, *len);

	EVP_DigestFinal_ex(ctx, md_value, &md_len);
	
	bencode->info_hash = (char *)malloc(40 * sizeof(char));
	bencode->info_hash_human_readable = (char *)malloc(40 * sizeof(char));
	
	memcpy(bencode->info_hash, md_value, 40);
	//strcpy((char *)bencode->info_hash, (char *)md_value);

	char *ptr = bencode->info_hash_human_readable;

	for (int i = 0; i < 20; i++) {
		ptr += sprintf(ptr, "%02x", md_value[i]);
	}

	EVP_MD_CTX_free(ctx);

	bencode->info_hash_human_readable[40] = '\0';

	return 0;
}
*/


id identify(char c) {
	switch (c) {
		case 'd':
			return dictionary;
			break;
		case 'l':
			return list;
			break;
		case 'i':
			return integer;
			break;
		case 'e':
			return end;
			break;
		default:
			return NOT_A_TYPE;
			break;
	}
}

int free_bencode_module(struct bencode_module *bencode) {
	
	int i, j = 0;
	if (bencode->announce != NULL) {
		free(bencode->announce);
	}
	if (bencode->announce_list != NULL) {
		for (i = 0; i < bencode->announce_list_index; i++) {
			free(bencode->announce_list[i]);
		}
		free(bencode->announce_list);
	}
	if (bencode->comment != NULL) {
		free(bencode->comment);
	}
	if (bencode->created_by != NULL) {
		free(bencode->created_by);
	}
	if (bencode->creation_date != NULL) {
		free(bencode->creation_date);
	}
	if (bencode->encoding != NULL) {
		free(bencode->encoding);
	}
	if (bencode->info != NULL) {
		for (i = 0; i < bencode->info_file_index; i++) {
			for (j = 0; j < bencode->info->files[i]->file_path_index; j++) {
				free(bencode->info->files[i]->path[j]);
			}
			free(bencode->info->files[i]->length);
			free(bencode->info->files[i]);
		}
		free(bencode->info->name);
		free(bencode->info->length);
		free(bencode->info->piece_length);
		free(bencode->info->pieces);
		free(bencode->info);
	}
	if (bencode->url_list != NULL) {
		for (i = 0; i < bencode->url_list_size; i++) {
			free(bencode->url_list[i]);
		}
		free(bencode->url_list);
	}
	
	if (bencode->info_hash != NULL) {
		free(bencode->info_hash);
	}
	if (bencode->info_hash_human_readable != NULL) {
		free(bencode->info_hash_human_readable);
	}
	if (bencode->buffer != NULL) {
		free(bencode->buffer);
	}
	free(bencode);

	return 0;
}

void printBencode(struct bencode_module *bencode) {
	printf("Announce: %s\n\n", bencode->announce);
	for (int i = 0; i < bencode->announce_list_index; i++) {
		printf("Announce-List %d: %s\n", i, bencode->announce_list[i]);
	}
	if (bencode->comment != NULL) printf("\nComment: %s\n", bencode->comment);
	if (bencode->created_by != NULL) printf("Created By: %s\n", bencode->created_by);
	if (bencode->creation_date != NULL) printf("Creation Date: %lld\n", *bencode->creation_date);
	if (bencode->encoding != NULL) printf("Encoding: %s\n\n", bencode->encoding);
	for (int i = 0; i < bencode->info_file_index; i++) {
		printf("Info File: %d\n\tLength: %ld\n\tPath: ", i, *bencode->info->files[i]->length);
		for (int j = 0; j < bencode->info->files[i]->file_path_index; j++) {
			printf("/%s", bencode->info->files[i]->path[j]);
		}
		printf("\n\n");
	}
	if (bencode->info->name != NULL) printf("\nName: %s\n", bencode->info->name);
	if (bencode->info->piece_length != NULL) printf("Piece Length: %ld\n", *bencode->info->piece_length);
	if (bencode->info_hash_human_readable != NULL) printf("Info Hash: %s\n", bencode->info_hash_human_readable);

//if (bencode->info->pieces != NULL) printf("Pieces: %s\n\n", bencode->info->pieces);

	if (bencode->url_list != NULL) {
		for (int i = 0; i < bencode->url_list_index; i++) {
			printf("Url List %d: %s\n", i, bencode->url_list[i]);
		}
	}
}
