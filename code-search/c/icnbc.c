#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "TRACE.h"

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

#define ALPHABET_LEN 7

void usage(char *argv0) {
	fprintf(stderr, "%s: <n> <min_ld> <min_iso>\n", argv0);
}

int lee_distance(uint8_t* x, uint8_t *y, uint8_t n) {
	int i, ld = 0;
	for(i=0; i<n; i++) {
		ld += MIN((x[i]-y[i]), ALPHABET_LEN-(x[i]-y[i]));
	}
	return ld;
}

typedef struct codeword_list_t {
	uint16_t index;
	uint8_t *codewords;
} codeword_list;

codeword_list *new_codeword_list(uint8_t n, int size) {
	codeword_list *cw_list;
	cw_list = malloc(sizeof(cw_list));
	if (cw_list==NULL)
		exit(1);
	cw_list->index = 0;
	cw_list->codewords = malloc(size * n);
	if (cw_list->codewords==NULL)
		exit(1);
	return cw_list;
}

static inline void copy_codeword(uint8_t *src, uint8_t *dst, int n) {
	int i;
	for(i=0; i<n; i++)
		dst[i] = src[i];
}

void populate_candidates(codeword_list* code, codeword_list *candidates,
						 uint8_t n, codeword_list* next_candidates,
						 uint8_t min_dist)
{
	uint8_t *nc;
	int i, j, k;
	for (i = 0; i < candidates->index; i++) {
		nc = candidates->codewords + n*i;
		if (lee_distance(nc, code->codewords + code->index*n, n) >= min_dist) {
			j = n * next_candidates->index++;
			
			for(k=0; k<n; k++)
				next_candidates->codewords[j+k] = nc[k];
			//copy_codeword(nc, next_candidates->codewords + j, n);
		}
	}
}

void print_set(codeword_list *set, uint8_t n) {
	int i,j,k;
	for(i=0; i<set->index; i++) {
		printf("[");
		j = i*n;
		for(k=j; k<j+n; k++)
			printf("%x,", set->codewords[k]);
		printf("]\n");
	}
}

codeword_list *create_search_space(uint8_t n) {
	int size, i, j, k;
	codeword_list *space;
	
	size = pow(ALPHABET_LEN, n);
	space = new_codeword_list(n, size);
	space->index = size;
	
	for(i=0; i<n; i++)
		space->codewords[i] = 0;

	for(i=1; i<size; i++) {
		j = i*n;
		for(k=j; k<j+n; k++)
			space->codewords[k] = space->codewords[k-n];

		for(k--; k>=j; k--) {
			space->codewords[k] = (space->codewords[k] + 1) % ALPHABET_LEN;
			if(space->codewords[k])
				break;
		}
	}
	return space;
}

uint16_t find_comp(codeword_list *a_code, codeword_list *b_code,
				   codeword_list* candidates, uint8_t min_ld,
				   uint16_t min_b_len, uint16_t min_len) {
	return 1;
}

int find_iso(codeword_list *code, codeword_list *candidates,
			 codeword_list *b_candidates, uint8_t n, uint8_t min_ld,
			 uint8_t min_iso, uint16_t a_len, uint16_t min_b_len,
			 uint16_t min_len) {
	codeword_list *next_candidates, *next_b_candidates;
	uint16_t max_b_len, total_len, best_len, longest;
	longest = 0;
	
	next_candidates = new_codeword_list(n, candidates->index);
	next_b_candidates = new_codeword_list(n, b_candidates->index);
	
	while (candidates->index) {
		copy_codeword(candidates->codewords + --candidates->index,
					  code->codewords + code->index++, n);
		
		populate_candidates(code, candidates, n, next_candidates, min_ld);
		printf("next_candidates:\n");
		print_set(next_candidates, n);
		populate_candidates(code, b_candidates, n, next_b_candidates, min_iso);
		printf("next_candidates:\n");
		print_set(next_b_candidates, n);

		if (((next_candidates->index + code->index) >= a_len) &&
			(next_b_candidates->index >= min_b_len)) {
			if (code->index == a_len) {
				codeword_list *b_code = new_codeword_list(n, next_b_candidates->index);
				best_len =  find_comp(code, b_code,  next_b_candidates,
									  min_ld, min_b_len, min_len);
				if (best_len >= min_len) {
					longest = best_len;
					min_len = best_len;
				}
			}
			if ((next_candidates->index) && (code->index < a_len)) {
				best_len = find_iso(code, next_candidates, next_b_candidates, n,
									min_ld, min_iso, a_len, min_b_len, min_len);
				if (best_len >= min_len) {
					longest = best_len;
					min_len = best_len;
				}
			}
		}
		code->index--;
	}
	return longest;
}

int find_iso_from_start(uint8_t n, uint8_t min_ld, uint8_t min_iso,
						uint16_t a_len, uint16_t min_b_len) {
	codeword_list *code, *candidates, *next_candidates, *next_b_candidates;
	int i;
	
	candidates = create_search_space(n);
	code = new_codeword_list(n, candidates->index);
	for(i=0; i<n; i++)
		code->codewords[i] = 0;
	code->index = 1;

	next_candidates = new_codeword_list(n, candidates->index);
	next_b_candidates = new_codeword_list(n, candidates->index);
	populate_candidates(code, candidates, n, next_candidates, min_ld);
	populate_candidates(code, candidates, n, next_b_candidates, min_iso);
	
	print_set(next_candidates, n);
	printf("\n");
	
	return find_iso(code, next_candidates, next_b_candidates, n, min_ld,
					min_iso, a_len, min_b_len, a_len + min_b_len);
}

void find_best_iso(uint8_t n, uint8_t min_ld, uint8_t min_iso)
{
	uint16_t a_len, min_b_len, longest, longest_b;
	min_b_len = 2;

	for (a_len = 1<<(n-1); a_len >= min_b_len; a_len--) {
		printf("trying a: %d, min b: %d, total: %d\n", a_len,
				min_b_len, a_len + min_b_len);
		longest = find_iso_from_start(n, min_ld, min_iso, a_len, min_b_len);
		if (longest >= (a_len + min_b_len)) {
			min_b_len = longest - a_len + 1;
		}

	}
}

int main(int argc, char** argv)
{
	if (argc < 4) {
		usage(argv[0]);
		exit(1);
	}

	uint8_t n, min_ld, min_iso;
	if ((n 		 = atoi(argv[1])) == 0 ||
		(min_ld  = atoi(argv[2])) == 0 ||
		(min_iso = atoi(argv[3])) == 0) {
		usage(argv[0]);
		exit(1);
	}
	
	find_best_iso(n, min_ld, min_iso);

	return 0;
}
