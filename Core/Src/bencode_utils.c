#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#include "../Inc/bencode.h"
#include "../Inc/bencode_utils.h"
#include "../Inc/codes.h"

uint8_t rv(struct bm *b, FILE *fp)
{
	uint8_t r;
	uint32_t l;
	size_t rs;

	r = vi(b->b, &l);

	if (r != CONVERSION_SUCCESS) {
		printf("Parse error: Length of data element could not be "
			"determined.\nPlease verify the integrity of your "
			".torrent file.\n");
		return PARSE_FAILURE;
	}

	if (l > b->bs) {
		while (l > b->bs) {
			b->bs = b->bs * 2;
		}
		b->b = realloc(b->b, b->bs);
	}

	rs = fread(b->b, 1, l, fp);
	b->b[l] = '\0';

	if (rs != l) {
		printf("Parse error: Could not capture full data segment. "
			"Please verify the integrity of you .torrent file\n");
		return DATA_LENGTH_EXCEEDED;
	}
	
	return 0;
}

uint8_t gc(int32_t *fc, FILE *fp)
{
	*fc = fgetc(fp);
	if (*fc == EOF) {
		if (errno) {
			perror("Error encountered while "
				"attempting to read file: ");
			return -1;
		} else {
			printf("Error encountered while "
				"attempting to read file: " 
				"Unexpected End Of File (EOF) detected");
			return -1;
		}
	}
	return 0;
}


uint8_t bm_init(struct bm *b)
{
	b->a 	= NULL;
	b->al	= NULL;
	b->c	= NULL;
	b->cb 	= NULL;
	b->cd	= NULL;
	b->e 	= NULL;
	b->i 	= NULL;
	b->ul	= NULL;
	b->ih	= NULL;
	b->ihhr	= NULL;
	
	b->ali 	= 0;
	b->ifi 	= 0;
	b->fpi 	= 0;
	b->uli 	= 0;

	b->als	= ALS;
	b->ifs	= IFS;
	b->fps	= FPS;
	b->uls	= ULS;

	b->hp 	= NULL;
	b->ip	= NULL;
	b->sp 	= NULL;

	b->is	= NULL;
	b->ie	= NULL;

	b->bs 	= BS;
	b->b	= (char *)malloc(b->bs * sizeof(char));
	if (b->b == NULL) {
		perror("Error encountered while attempting "
			"to allocate memory: ");
		return -1;
	}

	return 0;
}


int vi(char *in, uint32_t *out) {
		
	uint32_t val = 0;
	errno = 0;

	/* Performing strtoull on string and then checking errno and output buffers to check if valid integer */
	val = strtoul(in, NULL, 10);
//	if ((errno == ERANGE && (val == ULONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0)) {
//		perror("strtol");
//		return CONVERSION_FAILED;
//	}

	/* Setting value of second parameter to parsed value of integer */
	*(uint32_t *)out = val;
	
	return CONVERSION_SUCCESS;
}

uint8_t fbm(struct bm *b) {
	
	int i, j = 0;
	if (b->a != NULL) {
		free(b->a);
	}
	if (b->al != NULL) {
		for (i = 0; i < b->ali; i++) {
			free(b->al[i]);
		}
		free(b->al);
	}
	if (b->c != NULL) {
		free(b->c);
	}
	if (b->cb != NULL) {
		free(b->cb);
	}
	if (b->cd != NULL) {
		free(b->cd);
	}
	if (b->e != NULL) {
		free(b->e);
	}
	if (b->i != NULL) {
		for (i = 0; i < b->ifi; i++) {
			for (j = 0; j < b->i->f[i]->fpi; j++) {
				free(b->i->f[i]->p[j]);
			}
			free(b->i->f[i]->l);
			free(b->i->f[i]);
		}
		free(b->i->n);
		free(b->i->l);
		free(b->i->pl);
		free(b->i->p);
		free(b->i);
	}
	if (b->ul != NULL) {
		for (i = 0; i < b->uli; i++) {
			free(b->ul[i]);
		}
		free(b->ul);
	}
	
	if (b->ih != NULL) {
		free(b->ih);
	}
	if (b->ihhr != NULL) {
		free(b->ihhr);
	}
	if (b->is != NULL) {
		free(b->is);
	}
	if (b->ie != NULL) {
		free(b->ie);
	}
	if (b->b != NULL) {
		free(b->b);
	}
	free(b);

	return 0;
}
