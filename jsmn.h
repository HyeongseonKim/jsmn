#ifndef __JSMN_H_
#define __JSMN_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * JSON type identifier. Basic types are:
 * 	o Object
 * 	o Array
 * 	o String
 * 	o Other primitive: number, boolean (true/false) or null
 */
typedef enum {
	JSMN_UNDEFINED = 0,
	JSMN_OBJECT = 1,
	JSMN_ARRAY = 2,
	JSMN_STRING = 3,
	JSMN_PRIMITIVE = 4
} jsmntype_t;

enum jsmnerr {
	/* Not enough tokens were provided */
	JSMN_ERROR_NOMEM = -1,
	/* Invalid character inside JSON string */
	JSMN_ERROR_INVAL = -2,
	/* The string is not a full JSON packet, more bytes expected */
	JSMN_ERROR_PART = -3
};

/**
 * JSON token description.
 * type		type (object, array, string etc.)
 * start	start position in JSON data string
 * end		end position in JSON data string
 */
typedef struct {
	jsmntype_t type;
	int start;
	int end;
	int size;
#ifdef JSMN_PARENT_LINKS
	int parent;
#endif
} jsmntok_t;

/**
 * JSON parser. Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string
 */
typedef struct {
	unsigned int pos; /* offset in the JSON string */
	unsigned int toknext; /* next token to allocate */
	int toksuper; /* superior token node, e.g parent object or array */
} jsmn_parser;

/**
 * Create JSON parser over an array of tokens
 */
 void jsmn_init(jsmn_parser *parser) {
 	parser->pos = 0;
 	parser->toknext = 0;
 	parser->toksuper = -1;
 }

/**
 * Run JSON parser. It parses a JSON data string into and array of tokens, each describing
 * a single JSON object.
 */
 nt jsmn_parse(jsmn_parser *parser, const char *js, size_t len,
 		jsmntok_t *tokens, unsigned int num_tokens) {
 	int r;
 	int i;
 	jsmntok_t *token;
 	int count = parser->toknext;
 	char c;
 	jsmntype_t type;

 	for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
 	// read the jsmn string until string end and increase paser's pos value
 		c = js[parser->pos];
 		switch (c) {
 			case '{': case '[':
 				count++;
 				if (tokens == NULL) {
 					break;
 				}
 				token = jsmn_alloc_token(parser, tokens, num_tokens);
 				// jsmntok_t token memory allocation
 				// jsmn_alloc_token()
 				if (token == NULL)
 					return JSMN_ERROR_NOMEM;
 				if (parser->toksuper != -1) {
 					tokens[parser->toksuper].size++;
 #ifdef JSMN_PARENT_LINKS
 					token->parent = parser->toksuper;
 #endif
 				}
 				token->type = (c == '{' ? JSMN_OBJECT : JSMN_ARRAY);
 				token->start = parser->pos;
 				parser->toksuper = parser->toknext - 1;
 				break;
 			case '}': case ']':
 				if (tokens == NULL)
 					break;
 				type = (c == '}' ? JSMN_OBJECT : JSMN_ARRAY);
 #ifdef JSMN_PARENT_LINKS
 				if (parser->toknext < 1) {
 					return JSMN_ERROR_INVAL;
 				}
 				token = &tokens[parser->toknext - 1];
 				for (;;) {
 					if (token->start != -1 && token->end == -1) {
 						if (token->type != type) {
 							return JSMN_ERROR_INVAL;
 						}
 						token->end = parser->pos + 1;
 						parser->toksuper = token->parent;
 						break;
 					}
 					if (token->parent == -1) {
 						if(token->type != type || parser->toksuper == -1) {
 							return JSMN_ERROR_INVAL;
 						}
 						break;
 					}
 					token = &tokens[token->parent];
 				}
 #else
 				for (i = parser->toknext - 1; i >= 0; i--) {
 					token = &tokens[i];
 					if (token->start != -1 && token->end == -1) {
 						if (token->type != type) {
 							return JSMN_ERROR_INVAL;
 						}
 						parser->toksuper = -1;
 						token->end = parser->pos + 1;
 						break;
 					}
 				}
 				/* Error if unmatched closing bracket */
 				if (i == -1) return JSMN_ERROR_INVAL;
 				for (; i >= 0; i--) {
 					token = &tokens[i];
 					if (token->start != -1 && token->end == -1) {
 						parser->toksuper = i;
 						break;
 					}
 				}
 #endif
 				break;
 			case '\"':
 				r = jsmn_parse_string(parser, js, len, tokens, num_tokens);
 				if (r < 0) return r;
 				count++;
 				if (parser->toksuper != -1 && tokens != NULL)
 					tokens[parser->toksuper].size++;
 				break;
 			case '\t' : case '\r' : case '\n' : case ' ':
 				break;
 			case ':':
 				parser->toksuper = parser->toknext - 1;
 				break;
 			case ',':
 				if (tokens != NULL && parser->toksuper != -1 &&
 						tokens[parser->toksuper].type != JSMN_ARRAY &&
 						tokens[parser->toksuper].type != JSMN_OBJECT) {
 #ifdef JSMN_PARENT_LINKS
 					parser->toksuper = tokens[parser->toksuper].parent;
 #else
 					for (i = parser->toknext - 1; i >= 0; i--) {
 						if (tokens[i].type == JSMN_ARRAY || tokens[i].type == JSMN_OBJECT) {
 							if (tokens[i].start != -1 && tokens[i].end == -1) {
 								parser->toksuper = i;
 								break;
 							}
 						}
 					}
 #endif
 				}
 				break;
 #ifdef JSMN_STRICT
 			/* In strict mode primitives are: numbers and booleans */
 			case '-': case '0': case '1' : case '2': case '3' : case '4':
 			case '5': case '6': case '7' : case '8': case '9':
 			case 't': case 'f': case 'n' :
 				/* And they must not be keys of the object */
 				if (tokens != NULL && parser->toksuper != -1) {
 					jsmntok_t *t = &tokens[parser->toksuper];
 					if (t->type == JSMN_OBJECT ||
 							(t->type == JSMN_STRING && t->size != 0)) {
 						return JSMN_ERROR_INVAL;
 					}
 				}
 #else
 			/* In non-strict mode every unquoted value is a primitive */
 			default:
 #endif
 				r = jsmn_parse_primitive(parser, js, len, tokens, num_tokens);
 				if (r < 0) return r;
 				count++;
 				if (parser->toksuper != -1 && tokens != NULL)
 					tokens[parser->toksuper].size++;
 				break;

 #ifdef JSMN_STRICT
 			/* Unexpected char in strict mode */
 			default:
 				return JSMN_ERROR_INVAL;
 #endif
 		}
 	}

 	if (tokens != NULL) {
 		for (i = parser->toknext - 1; i >= 0; i--) {
 			/* Unmatched opened object or array */
 			if (tokens[i].start != -1 && tokens[i].end == -1) {
 				return JSMN_ERROR_PART;
 			}
 		}
 	}

 	return count;
 }

#ifdef __cplusplus
}
#endif

#endif /* __JSMN_H_ */
