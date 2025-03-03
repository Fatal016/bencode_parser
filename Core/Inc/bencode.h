#ifndef H_BENCODE
#define H_BENCODE 	

#define NOT_A_TYPE NULL
#define BUFFER_EXCEEDED -1
#define END_OF_TYPE 1
#define DATA_LENGTH_EXCEEDED -2
#define CONVERSION_FAILED -3
#define CONVERSION_SUCCESS 0

#define PARSE_SUCCESS 0
#define PARSE_FAILURE -1

#define IGNORE_FLAG -1

/* Struct for bencode -> info -> files */
struct info_file {
	unsigned long int* length;
	char** path;

	int file_path_index;
};

/* Struct for bencode -> info */
struct bencode_info {
	struct info_file** files;
	char* name;
	long unsigned int* length;
	long unsigned int* piece_length;
	char* pieces;
};

/* Core struct for storing information parsed from .torrent file */
struct bencode_module {
	
	/**************************/
	/*** Bencode Components ***/
	/**************************/

	char* announce;
	char** announce_list;
	char* comment;
	char* created_by;
	long long int* creation_date;
	char* encoding;
	struct bencode_info *info;	
	char** url_list;
	char* info_hash;
	char* info_hash_human_readable;

	/***********************/
	/*** List Parameters ***/
	/***********************/	

	/* Indexes for list components */
	int announce_list_index;
	int info_file_index;
	int file_path_index;
	int url_list_index;

	/* Sizes for list components */
	int announce_list_size;
	int info_file_size;
	int file_path_size;
	int url_list_size;

	/* Tracking for features of lists */
	void* head_pointer;
	int* index_pointer;
	int* size_pointer;

	
	/************************/
	/*** Other Parameters ***/
	/************************/

	/* Info Hash Parameters */
	size_t info_start;
	size_t info_end;

	/* Input buffer for filestream */
	char* buffer;
	long long int buffer_size;
};

/* Serves as template for return-type of 'identify' */
typedef int (*id)(struct bencode_module*, FILE*);

/* For identifying type of component being read */
id identify(char c);

/* Bencode component parsing functions definitions */
int dictionary(struct bencode_module*, FILE*);
int list(struct bencode_module*, FILE*);
int integer(struct bencode_module*, FILE*);
int end(struct bencode_module* __attribute__((unused)), FILE* __attribute__((unused)));

/* Root function for parsing .torrent file */
int parse_single(char*, struct bencode_module*);

int free_bencode_module(struct bencode_module*);

/* Helper Functions */
void parse_key(struct bencode_module*, FILE*);
int sha1(struct bencode_module*, char*, size_t*);

int verify_int(char*, long long int*);

/* Tools */
void printBencode(struct bencode_module *bencode);

#endif
