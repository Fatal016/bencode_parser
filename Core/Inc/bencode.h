#ifndef H_BENCODE
#define H_BENCODE 	

#include <stdint.h>

#define BS 128 	/* Buffer Size */
#define ALS 256 /* Announce List Size */
#define IFS 128 /* Info File Size */
#define FPS 10 	/* File Path Size */
#define ULS 1 	/* Url List Size */

/* Bencode File (bencode -> info -> files) */
struct bf {
	uint32_t* l;	/* length */
	char** p;	/* path */
	uint16_t fpi;	/* file path index */
};

/* Bencode Info (bencode -> info) */
struct bi {
	struct bf** f; 	/* files */
	char* n;	/* name */
	uint32_t* l;	/* length (single file) */
	uint32_t* pl;	/* piece length */
	char* p;	/* pieces */
};

/* Core struct for storing information parsed from .torrent file */
/* Bencode Module */
struct bm {
	
	/**************************/
	/*** Bencode Components ***/
	/**************************/

	char* a;	/* announce */
	char** al;	/* announce_list */
	char* c;	/* comment */
	char* cb;	/* created by */
	uint32_t* cd;	/* creation_date */
	char* e;	/* encoding */
	struct bi *i;	/* info */
	char** ul;	/* url list */
	char* ih;	/* info hash */
	char* ihhr;	/* info hash human readable */

	/***********************/
	/*** List Parameters ***/
	/***********************/	

	/* List features */
	uint16_t ali;	/* announce_list_index */
	uint16_t ifi; 	/* info_file_index */
	uint16_t fpi; 	/* file_path_index */
	uint16_t uli; 	/* url_list_index */

	uint16_t als; 	/* announce_list_size */
	uint16_t ifs;	/* info_file_size */
	uint16_t fps; 	/* file_path_size */
	uint16_t uls;	/* url_list_size */

	/* Tracking for features of lists */
	void* hp; 	/* head pointer */
	uint16_t* ip; 	/* index pointer */
	uint16_t* sp; 	/* size pointer */

	/************************/
	/*** Other Parameters ***/
	/************************/

	/* Info Hash Parameters */
	int64_t* is; 	/* info start */
	int64_t* ie; 	/* info end */

	/* Buffer Parameters */
	uint32_t bs; 	/* buffer size */
	char* b;	/* buffer */
};

/* Function pointer template for return-type of idt */
typedef uint8_t (*id)(struct bm*, FILE*);

/* For identifying type of component being read */
id idt(int32_t*);

/* Bencode component parsing functions definitions */
uint8_t d(struct bm*, FILE*); 	/* dictionary */
uint8_t l(struct bm*, FILE*);	/* list */
uint8_t i(struct bm*, FILE*);	/* integer */
uint8_t e(struct bm*, FILE*);	/* end */

/* Root function for parsing .torrent file */
int p(char*, struct bm*); /* parse */

/* Helper Functions */

/* Parse Key */
uint8_t pdk(struct bm*, FILE*); /* parse dictionary key */

/* Parse Value Functions */
uint8_t pdv(struct bm*, FILE*); /* parse dictionary value */
uint8_t plv(struct bm*, FILE*); /* parse list value */

/* Logic Functions */
uint8_t cih(struct bm*, FILE*); /* calculate info hash */
uint8_t cs(struct bm*, char*, size_t*); /* calculate sha */

#endif
