#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "../Inc/bencode.h"
#include "../Inc/bencode_utils.h"
#include "../Inc/codes.h"

#include <openssl/evp.h>

int p(char *f, struct bm* b) {

	uint8_t r;
	int32_t fc;
	
	FILE *fp;
	id t = NULL;

	bm_init(b);

	fp = fopen(f, "r");	
	if (f == NULL) {
		perror("Error encountered while attempting to access file: ");
		return -1;
	}

	/* Capturing character from file */
	if (gc(&fc, fp) != 0) return -1;
	t = idt(fc);
	if (t != &d) {
		printf("Error encountered while attempting to "
			"read file: First character was not the beginning "
			"of a dictionary\n");
		return -1;
	}

	/* Initiating state machine at dictionary */
	r = t(b, fp);

	if (r != 0) {
		printf("Error encountered while attempting to parse file: " 
			"Error code was %d and occured approximately at "
			"...\n", r);
	}

	fclose(fp);
	return r;
}

uint8_t d(struct bm *b, FILE *fp)
{	
	uint8_t r;
	int32_t fc;
	uint32_t i = 0;	
	id t;

	while (i < b->bs) {
		r = gc(&fc, fp);
		if (r != 0) return r;
		t = idt(fc);
		if (t != NULL) {
			r = t(b, fp);
			if (r == END_OF_TYPE) return PARSE_SUCCESS;
			b->hp = NULL;
			i = 0;
		} else {
			if (fc == ':') {
				b->b[i] = '\0';
				r = rv(b, fp);
				if (r != 0) return r;
				if (b->hp == NULL) {
					r = pdk(b, fp);
					if (r != 0) return r;
				} else {
					r = pdv(b, fp);
					if (r != 0) return r;
				}
				i = 0;
			} else {
				b->b[i] = fc;
				i++;
			}
		}
	}
	return BUFFER_EXCEEDED;
}

uint8_t l(struct bm *b, FILE *fp)
{	
	uint8_t r;
	int32_t fc;
	uint32_t i = 0;
	id t;

	while (i < b->bs) {
		r = gc(&fc, fp);
		if (r != 0) return r;
		t = idt(fc);
		if (t != NULL) {
			r = t(b, fp);				
			if (r == END_OF_TYPE) return PARSE_SUCCESS;
			i = 0;
		} else {
			if (fc == ':') {
				b->b[i] = '\0';
				r = rv(b, fp);
				if (r != 0) return r;
				r = plv(b, fp);
				if (r != 0) return r;
				i = 0;
			} else {
				b->b[i] = fc;
				i++;
			}
		}
	}
	return BUFFER_EXCEEDED;
}

uint8_t i(struct bm *b, FILE *fp) {

	uint8_t r;
	int32_t fc;
	uint32_t i = 0;
	id t;

	while (i < b->bs) {
		r = gc(&fc, fp);
		if (r != 0) return r;
		t = idt(fc);
		if (t != NULL) {
			r = t(b, fp);
			if (r == END_OF_TYPE) {
				b->b[i] = '\0';
				if (b->hp != (void *)IGNORE_FLAG) {
					r = vi(b->b, b->hp);	
					if (r != 0) return r;
				}
				return PARSE_SUCCESS;
			}
		} else {
			b->b[i] = fc;
			i++;
		}
	}
	return BUFFER_EXCEEDED;
}

uint8_t e(struct bm *b, FILE *fp) {
	return END_OF_TYPE;
}

uint8_t pdk(struct bm *b, FILE *fp)
{
	if (strcmp(b->b, "announce") == 0) {
		
		b->a = (char *)malloc(BS * sizeof(char));
		b->hp = (void *)b->a;
	
	} else if (strcmp(b->b, "announce-list") == 0) {

		b->al = (char **)malloc(b->als * sizeof(char *));
		b->hp = (void *)b->al;
		b->ip = &b->ali;
		b->sp = &b->als;
	
	} else if (strcmp(b->b, "comment") == 0) {
		
		b->c = (char *)malloc(BS * sizeof(char));
		b->hp = (void *)b->c;
	
	} else if (strcmp(b->b, "created by") == 0) {
			
		b->cb = (char *)malloc(BS * sizeof(char));
		b->hp = (void *)b->cb;
	
	} else if (strcmp(b->b, "creation date") == 0) {
			
		b->cd = (uint32_t *)malloc(sizeof(uint32_t));
		b->hp = (void *)b->cd;

	} else if (strcmp(b->b, "encoding") == 0) {
		
		b->e = (char *)malloc(BS * sizeof(char));
		b->hp = (void *)b->e;
		
	} else if (strcmp(b->b, "info") == 0) {

		b->i = (struct bi *)malloc(sizeof(struct bi));
		b->i->f = NULL;
		b->i->p = NULL;

		b->is = (int64_t *)malloc(sizeof(int64_t));
		b->ie = (int64_t *)malloc(sizeof(int64_t));

		*b->is = ftell(fp);
		b->hp = NULL;

	} else if (strcmp(b->b, "files") == 0) {
			
		b->i->f = (struct bf **)malloc(b->ifs * sizeof(struct bf *));

	} else if (strcmp(b->b, "length") == 0) {
			
		if (b->i->f != NULL) {
			/* Multi-file contents */
			b->i->f[b->ifi] = (struct bf *)malloc(sizeof(struct bf));
			b->i->f[b->ifi]->l = (uint32_t *)malloc(sizeof(uint32_t));
			b->hp = (void *)b->i->f[b->ifi]->l;
		} else {
			/* Single file contents */
			b->i->l = (uint32_t *)malloc(sizeof(uint32_t));
			b->hp = (void *)b->i->l;
		}						

	} else if (strcmp(b->b, "path") == 0) {
		
		b->i->f[b->ifi]->fpi = 0;
		b->i->f[b->ifi]->p = (char **)malloc(b->fps * sizeof(char *));
		b->hp = (void *)b->i->f[b->ifi]->p;
		b->ip = &b->i->f[b->ifi]->fpi;
		b->ifi++;

	} else if (strcmp(b->b, "name") == 0) {
			
		b->i->n = (char *)malloc(BS * sizeof(char));
		b->hp = (void *)b->i->n;
		
	} else if (strcmp(b->b, "piece length") == 0) {
		
		b->i->pl = (uint32_t *)malloc(sizeof(uint32_t));
		b->hp = (void *)b->i->pl;
		
	} else if (strcmp(b->b, "pieces") == 0) {

		b->i->p = (char *)malloc(*b->i->pl * sizeof(char));
		b->hp = (void *)b->i->p;
		
	} else if (strcmp(b->b, "url-list") == 0) {
			
		b->ul = (char **)malloc(b->uls * sizeof(char *));
		b->hp = (void *)b->ul;
		b->ip = &b->uli;
	
	} else {
		b->hp = (void *)IGNORE_FLAG;	
	}

	return PARSE_SUCCESS;
}

uint8_t cih(struct bm *b, FILE *fp)
{
	size_t lr;
	uint32_t is;
	char* ib;

	*b->ie = ftell(fp);
	if (*b->ie == -1) {
		if (errno != 0) {
			perror("Error encountered while attempting to "
				"capture stream location in file: ");
		}
	}
	is = *b->ie - *b->is + 1;
	ib = (char *)malloc(is * sizeof(char));
	fseek(fp, *b->is, SEEK_SET);	
	lr = fread(ib, 1, is, fp);

	sha1(b, ib, &lr);
	
	fseek(fp, *b->ie, SEEK_SET);

	free(ib);
	return PARSE_SUCCESS;
}

uint8_t pdv(struct bm *b, FILE *fp)
{
	uint8_t r;

	if (b->hp != (void *)IGNORE_FLAG) {
		strcpy((char *)b->hp, b->b);
		if (b->i != NULL) {
			if (b->hp == (void *)b->i->p) {
				r = cih(b, fp);
				if (r != 0) return r;
			}	
		}						
	}
	b->hp = NULL;
	return PARSE_SUCCESS;
}

uint8_t plv(struct bm *b, FILE *fp)
{
	if (b->hp != (void *)IGNORE_FLAG) {
		((char **)b->hp)[*b->ip] = (char *)malloc(BS * sizeof(char));
		strcpy(((char **)b->hp)[*b->ip], b->b);
		(*b->ip)++;
	}
	return PARSE_SUCCESS;
}

uint8_t sha1(struct bm *bencode, char *info_dict, size_t *len) {

	EVP_MD_CTX *ctx;
	ctx = EVP_MD_CTX_new();

	const EVP_MD *md;
	md = EVP_get_digestbyname("sha1");
	
	if(!md) {
       printf("Unknown message digest %s\n", "sha1");
	}

	EVP_DigestInit_ex(ctx, md, NULL);

//	need to create defines for v1 and v2 lengths, 40 and 64 characters depending on sha1 sha256

	unsigned char md_value[40];
	unsigned int md_len;
	EVP_DigestUpdate(ctx, info_dict, *len);

	EVP_DigestFinal_ex(ctx, md_value, &md_len);
	
	bencode->ih = (char *)malloc(41 * sizeof(char));
	bencode->ihhr = (char *)malloc(41 * sizeof(char));
	
	memcpy(bencode->ih, md_value, 40);
	strcpy((char *)bencode->ih, (char *)md_value);

	char *ptr = bencode->ihhr;

	for (int i = 0; i < 20; i++) {
		ptr += sprintf(ptr, "%02x", md_value[i]);
		printf("%02x", md_value[i]);
	}
	printf("\n");

	EVP_MD_CTX_free(ctx);

	bencode->ihhr[40] = '\0';

	return 0;
}

id idt(int c) {
	switch (c) {
		case 'd':
			return d;
			break;
		case 'l':
			return l;
			break;
		case 'i':
			return i;
			break;
		case 'e':
			return e;
			break;
		default:
			return NOT_A_TYPE;
			break;
	}
}
