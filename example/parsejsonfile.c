#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../jsmn.c"
// SW 수업 너무 어렵다..
/*
 * A small example of jsmn parsing when JSON structure is known and number of
 * tokens is predictable.
 */


static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
			strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}
	return -1;
}

char* readjsonfile(const char* filename) {

	FILE* fp = fopen(filename, "r");
	int memsize = 60;
	char* Str = (char*)malloc(sizeof(char)*memsize);
	char* Tmp = (char*)malloc(sizeof(char)*memsize);


	if(fp == NULL) {
					 perror("fopen");
					 exit(1);
			 }


	while (fscanf(fp,"%s",Tmp) != EOF) {

		if ( memsize < strlen(Str) + strlen(Tmp))
		{
				memsize *= 2;
				Str = (char*)realloc(Str,  strlen(Str) + memsize);
		}
		strcat(Str, " ");
		strcat(Str, Tmp);

	}

	return Str;
}

char* substring(char part[], int start, int end) {

	int count = 0;
	int length = end - start + 1;
	char* temp = (char*)malloc(sizeof(char) * 100);

	while ( count < length - 1)
	{
		temp[count] = part[start + count];
		count++;
	}

	return temp;

}

void printall(const char *json, jsmntok_t *t, int tokcount) {

	int i;



	for ( i = 1 ; i < tokcount ; i++ ) {

		printf("[ %d] %s (size = %d, %d~%d, %d)\n", i, substring(json, t[i].start, t[i].end), t[i].size, t[i].start, t[i].end, t[i].type );

	}

	return;

}

int findkeys(const char* json, jsmntok_t *t, int tokcount, int* key ) {

	int i = 0;
	int count = 0;

	for ( i = 1 ; i < tokcount ; i++ ) {

		if ( t[i].type != 0) {
				count++;
				key[count] = i;
		}

	}

	return count;
}

void printkeys(const char* json, jsmntok_t *t, int tokcount) {

	int i = 0;
	int count = 0;

	for ( i = 1 ; i < tokcount ; i++ ) {

		if ( t[i].type != 0) {
				count++;
				printf("[%d] %s(%d)\n", i, substring(json, t[i].start, t[i].end), count);
		}

	}

	return;
}
int main() {
	int i;
	int r;
	int tokcount = 0;
	jsmn_parser p;
	jsmntok_t t[128]; /* We expect no more than 128 tokens */
	jsmn_init(&p);

	int keyarray[128], keyamount;

	char* JSON_STRING = readjsonfile("data.json");
	printf("-----------Lab 1 Read Json File--------------\n");

	r = jsmn_parse(&p, JSON_STRING, strlen(JSON_STRING), t, sizeof(t)/sizeof(t[0]));
	if (r < 0) {
		printf("Failed to parse JSON: %d\n", r);
		return 1;
	}

	/* Assume the top-level element is an object */
	if (r < 1 || t[0].type != JSMN_OBJECT) {
		printf("Object expected\n");
		return 1;
	}

	/* Loop over all keys of the root object */
	for (i = 1; i < r; i++) {
		if (jsoneq(JSON_STRING, &t[i], "name") == 0) {
			/* We may use strndup() to fetch string value */
			printf("- User: %.*s\n", t[i+1].end-t[i+1].start,
					JSON_STRING + t[i+1].start);
			i++;
		} else if (jsoneq(JSON_STRING, &t[i], "keywords") == 0) {
			/* We may additionally check if the value is either "true" or "false" */
			printf("- keywords: %.*s\n", t[i+1].end-t[i+1].start,
					JSON_STRING + t[i+1].start);
			i++;
		} else if (jsoneq(JSON_STRING, &t[i], "description") == 0) {
			/* We may want to do strtol() here to get numeric value */
			printf("- description: %.*s\n", t[i+1].end-t[i+1].start,
					JSON_STRING + t[i+1].start);
			i++;
		} else if (jsoneq(JSON_STRING, &t[i], "type") == 0) {
			/* We may want to do strtol() here to get numeric value */
			printf("- type: %.*s\n", t[i+1].end-t[i+1].start,
					JSON_STRING + t[i+1].start);
			i++;
		} else if (jsoneq(JSON_STRING, &t[i], "url") == 0) {
			/* We may want to do strtol() here to get numeric value */
			printf("- url: %.*s\n", t[i+1].end-t[i+1].start,
					JSON_STRING + t[i+1].start);
			i++;
		} else if (jsoneq(JSON_STRING, &t[i], "frameworks") == 0) {
			/* We may want to do strtol() here to get numeric value */
			printf("- frameworks: %.*s\n", t[i+1].end-t[i+1].start,
					JSON_STRING + t[i+1].start);
			i++;
		} else if (jsoneq(JSON_STRING, &t[i], "platforms") == 0) {
			/* We may want to do strtol() here to get numeric value */
			printf("- platforms: %.*s\n", t[i+1].end-t[i+1].start,
					JSON_STRING + t[i+1].start);
			i++;
		} else if (jsoneq(JSON_STRING, &t[i], "exclude") == 0) {
			/* We may want to do strtol() here to get numeric value */
			printf("- exclude: %.*s\n", t[i+1].end-t[i+1].start,
					JSON_STRING + t[i+1].start);
			i++;
		}
		 else if (jsoneq(JSON_STRING, &t[i], "examples") == 0) {
			int j;
			printf("- examples:\n");
			if (t[i+1].type != JSMN_ARRAY) {
				continue; /* We expect groups to be an array of strings */
			}
			for (j = 0; j < t[i+1].size; j++) {
				jsmntok_t *g = &t[i+j+2];
				printf("  * %.*s\n", g->end - g->start, JSON_STRING + g->start);
			}
			i += t[i+1].size + 1;
		} else if (jsoneq(JSON_STRING, &t[i], "repository") == 0) {
			int j;
			printf("- repository:\n");
			if (t[i+1].type != JSMN_OBJECT) {
				continue; /* We expect groups to be an array of strings */
			}
			for (j = 0; j < t[i+1].size; j++) {
				jsmntok_t *g = &t[i+j+2];
				printf("  * %.*s\n", g->end - g->start, JSON_STRING + g->start);
			}
			i += t[i+1].size + 1;
		} else {
			printf("Unexpected key: %.*s\n", t[i].end-t[i].start,
					JSON_STRING + t[i].start);
		}

		tokcount++;

	}
	printf("---------------Lab 2 Print All--------------------\n");
	printall( JSON_STRING, t, tokcount);
	printf("--------------Lab 3 Print All keys----------------\n");
	printkeys( JSON_STRING, t, tokcount);
	printf("-----------Lab 4 Print All keyamount--------------\n");
	keyamount = findkeys(JSON_STRING, t, tokcount, keyarray);
	printf("keyamount: %d\n", keyamount);


	return EXIT_SUCCESS;
}
