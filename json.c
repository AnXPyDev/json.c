#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef DEBUG
#define DEBUG 0
#endif

enum json_type {
	json_type_none, json_type_object, json_type_array, json_type_number, json_type_boolean, json_type_string, json_type_null, json_type_undefined
};

enum json_number_type {
	json_type_int, json_type_double
};

enum json_boolean_value {
	json_true, json_false
};

struct json_item {
	struct json_item *next;
	enum json_type type;
	char value;	
};

char *json_empty_string = "";

struct json_item *json_item_alloc(int value_size) {
	struct json_item *item = malloc(sizeof(struct json_item) + value_size - 1);
	item->next = NULL;
	item->type = json_type_none;
	return item;
}

struct json_object {
	struct json_item *keys;
	struct json_item *items;
};

struct json_item *json_object_create() {
	struct json_item *item = json_item_alloc(sizeof(struct json_object));
	struct json_object *object = (struct json_object*)&item->value;
	item->type = json_type_object;
	object->keys = NULL;
	object->items = NULL;
	return item;
}

struct json_array {
	struct json_item *items;
};

struct json_item *json_array_create() {
	struct json_item *item = json_item_alloc(sizeof(struct json_array));
	struct json_array *array = (struct json_array*)&item->value;
	item->type = json_type_array;
	array->items = NULL;
	return item;
}

union json_number_value {
	int i; double d;
};

struct json_number {
	enum json_number_type type;
	union json_number_value value;
};

struct json_item *json_number_create() {
	struct json_item *item = json_item_alloc(sizeof(struct json_number));
	struct json_number *number = (struct json_number*)&item->value;
	item->type = json_type_number;
	number->type = json_type_int;
	number->value.i = 0;
	return item;
}

struct json_boolean {
	enum json_boolean_value value;
};

struct json_item *json_boolean_create() {
	struct json_item *item = json_item_alloc(sizeof(struct json_boolean));
	struct json_boolean *boolean = (struct json_boolean*)&item->value;
	item->type = json_type_boolean;
	boolean->value = json_false;
	return item;
}


struct json_string {
	char *string;
	int should_free;
};

struct json_item *json_string_create() {
	struct json_item *item = json_item_alloc(sizeof(struct json_number));
	struct json_string *string = (struct json_string*)&item->value;
	item->type = json_type_string;
	string->string = json_empty_string;
	string->should_free = 0;
	return item;
}

struct json_item *json_get(struct json_item *item, int index) {
	int i = 0;
	while (i != index && item != NULL) {
		item = item->next;
		i++;
	}

	return item;
}

int json_cmp(struct json_item *ia, struct json_item *ib) {
	if (ia->type != ib->type) {
		return ia->type - ib->type;
	}

	switch (ia->type) {
		case json_type_string:
			return strcmp(((struct json_string*)&ia->value)->string, ((struct json_string*)&ib->value)->string);
	}

	return 1;
}

struct json_item *json_index(struct json_item *item, struct json_item *search) {
	while (item != NULL) {
		if (json_cmp(item, search) == 0) {
			break;
		}
		item = item->next;
	}
	return item;
}

const int json_indent_offset = 4;

void json_indent(FILE *file, int indent) {
	for (int i = 0; i < indent; i++) {
		fputc(' ', file);
	}
}

void json_item_print(FILE *file, struct json_item *item, int indent) {
	if (item == NULL) {
		if (DEBUG)
			fprintf(stderr, "DEBUG: attempt to print NULL\n");
		return;
	}
	switch (item->type) {
		case json_type_object:
			if (DEBUG)
				fprintf(stderr, "DEBUG: print object\n");
			struct json_object *object = (struct json_object*)&item->value;
			struct json_item *key = object->keys;
			struct json_item *value = object->items;
			int new_indent = indent + json_indent_offset;
			fprintf(file, "{\n");
			if (key != NULL && value != NULL) {
				json_indent(file, new_indent);
				json_item_print(file, key, new_indent);
				fprintf(file, ": ");
				json_item_print(file, value, new_indent);
				key = key->next;
				value = value->next;
			}
			while (key != NULL && value != NULL) {
				fprintf(file, ",\n");
				json_indent(file, new_indent);
				json_item_print(file, key, new_indent);
				fprintf(file, ": ");
				json_item_print(file, value, new_indent);
				key = key->next;
				value = value->next;
			}
			fprintf(file, "\n");
			json_indent(file, indent);
			fprintf(file, "}");

			break;
		case json_type_array:
			if (DEBUG)
				fprintf(stderr, "DEBUG: print array\n");
			struct json_array *array = (struct json_array*)&item->value;
			fprintf(file, "[");
			struct json_item *i = array->items;
			if (i != NULL) {
				json_item_print(file, i, indent + json_indent_offset);
				i = i->next;
			}
			while (i != NULL) {
				fprintf(file, ", ");
				json_item_print(file, i, indent + json_indent_offset);
				i = i->next;
			}
			fprintf(file, "]");
			break;
		case json_type_string:
			if (DEBUG)
				fprintf(stderr, "DEBUG: print string\n");
			struct json_string *string = (struct json_string*)&item->value;
			fprintf(file, "\"%s\"", string->string);
			break;
		case json_type_number:
			if (DEBUG)
				fprintf(stderr, "DEBUG: print number\n");
			struct json_number *number = (struct json_number*)&item->value;
			switch (number->type) {
				case json_type_int:
					fprintf(file, "%i", number->value.i);
					break;
				case json_type_double:
					fprintf(file, "%f", number->value.d);
					break;
			}
			break;
		case json_type_boolean:
			if (DEBUG)
				fprintf(stderr, "DEBUG: print boolean\n");
			struct json_boolean *boolean = (struct json_boolean*)&item->value;
			switch (boolean->value) {
				case json_true:
					fprintf(file, "true");
					break;
				case json_false:
					fprintf(file, "false");
					break;
			}
			break;
		case json_type_null:
			if (DEBUG)
				fprintf(stderr, "DEBUG: print null\n");
			fprintf(file, "null");
			break;
		case json_type_undefined:
			if (DEBUG)
				fprintf(stderr, "DEBUG: print undefined\n");
			fprintf(file, "undefined");
			break;
		case json_type_none:
			if (DEBUG)
				fprintf(stderr, "DEBUG: print none\n");
			fprintf(file, "NONE!!");
			break;
	}
}


void json_item_destroy(struct json_item *item) {
	if (item == NULL) {
		return;
	}
	switch (item->type) {
		case json_type_object:
			struct json_object *object = (struct json_object*)&item->value;
			json_item_destroy(object->keys);
			json_item_destroy(object->items);
			break;
		case json_type_array:
			struct json_array *array = (struct json_array*)&item->value;
			json_item_destroy(array->items);
			break;
		case json_type_string:
			struct json_string *string = (struct json_string*)&item->value;
			if (string->should_free) {
				free(string->string);
			}
			break;
	}
	if (item->next != NULL) {
		json_item_destroy(item->next);
	}
	free(item);
}

int is_whitespace(int c) {
	switch (c) {
		case 0: //null
		case 9: //tab
		case 10: //lf
		case 13: //cr
		case 32: //space
			return 1;
		default:
			return 0;
	}
}

typedef struct json_item *(*json_parse_fn)(FILE *file);

struct json_item *json_item_parse(FILE*);

#define json_max_num_len 32

struct json_item *json_number_parse(FILE *file) {
	if (DEBUG)
		fprintf(stderr, "DEBUG: json_number_parse\n");
	char buffer[json_max_num_len + 1];
	char *bp = buffer;
	enum json_number_type type = json_type_int;
	int c;
	for (int i = 0; i < json_max_num_len + 1; i++) {
		c = fgetc(file);
		if (feof(file)) {
			goto graceful_end;
		}

		if (is_whitespace(c)) {
			goto graceful_end;
		}
		switch (c) {
			case 46: // .
				type = json_type_double;
				break;
			case 44: // ,
			case 58: // :
			case 93: // ]
			case 125: // }
				goto graceful_end;
		}
		
		*bp = c;
		bp++;
	}

	bp--;
	*bp = 0;
	goto malformed_error;	

	graceful_end:;	
	*bp = 0;

	ungetc(c, file);
	
	union json_number_value value;
	int e;
	switch (type) {
		case json_type_int:
			e = sscanf(buffer, "%i", &value.i);
			break;
		case json_type_double:
			e = sscanf(buffer, "%lf", &value.d);
			break;
	}

	if (e != 1) {
		goto malformed_error;
	}
	struct json_item *result = json_number_create();
	struct json_number *number = (void*)&result->value;
	number->value = value;
	number->type = type;
	return result;

	malformed_error:;
	fprintf(stderr, "error: malformed number at \"%s\"\n", buffer);
	return NULL;

}

#undef json_max_num_len

#define json_max_token_len 9

struct json_item *json_token_parse(FILE *file) {
	if (DEBUG)
		fprintf(stderr, "DEBUG: json_token_parse\n");
	char buffer[json_max_token_len + 1];
	char *bp = buffer;

	int c;
	for (int i = 0; i < json_max_token_len + 1; i++) {
		c = fgetc(file);
		if (feof(file)) {
			goto graceful_end;
		}

		if (is_whitespace(c)) {
			goto graceful_end;
		}
		switch (c) {
			case 44: // ,
			case 58: // :
			case 93: // ]
			case 125: // }
				goto graceful_end;
		}
		
		*bp = c;
		bp++;
	}
	
	bp--;
	*bp = 0;
	fprintf(stderr, "error: token too long at \"%s\"\n", buffer);
	return NULL;

	graceful_end:
	*bp = 0;
	ungetc(c, file);

	enum json_type type = json_type_none;
	enum json_boolean_value boolval = json_false;
	if (strcmp(buffer, "null") == 0) {
		type = json_type_null;
	} else if (strcmp(buffer, "undefined") == 0) {
		type = json_type_undefined;
	} else if (strcmp(buffer, "true") == 0) {
		type = json_type_boolean;
		boolval = json_true;
	} else if (strcmp(buffer, "false") == 0) {
		type = json_type_boolean;
		boolval = json_false;
	}
	
	struct json_item *result = NULL;
	switch (type) {
		case json_type_null:
		case json_type_undefined:
			result = json_item_alloc(0);
			result->type = type;
			break;
		case json_type_boolean:
			result = json_boolean_create();
			struct json_boolean *boolean = (void*)&result->value;
			boolean->value = boolval;
			break;
	}
	return result;
}

#undef json_max_token_len

#define json_max_string_len 256

struct json_item *json_string_parse(FILE *file) {
	if (DEBUG)
		fprintf(stderr, "DEBUG: json_string_parse\n");
	int c = fgetc(file);
	if (c != 34) {
		if (DEBUG)
			fprintf(stderr, "DEBUG: no quotation mark at beginning of string\n");
		return NULL;
	}

	char *buffer = malloc(json_max_string_len + 1);
	char *bp = buffer;

	int escape = 0;
	for (int i = 0; i < json_max_string_len + 1; i++) {
		c = fgetc(file);
		if (feof(file)) {
			goto graceful_end;
		}

		switch (c) {
			case 34: // "
				if (!escape) {
					goto graceful_end;
				}
				break;
			case 92: // backslash 
				if (!escape) {
					escape = 1;
					goto next_loop;
				}
				break;
		}

		*bp = c;
		bp++;
		escape = 0;
		next_loop:;
	}
	
	*bp = 0;	
	fprintf(stderr, "error: string longer than %i at \"%s\"\n", json_max_string_len, buffer);
	return NULL;
	
	graceful_end:;
	*bp = 0;

	struct json_item *result = json_string_create();
	struct json_string *string = (void*)&result->value;

	string->string = buffer;
	string->should_free = 1;

	return result;
}

#undef json_max_string_len

struct json_item *json_array_parse(FILE *file) {
	if (DEBUG)
		fprintf(stderr, "DEBUG: json_array_parse\n");
	int c = fgetc(file);
	if (c != 91) { // [
		if (DEBUG)
			fprintf(stderr, "DEBUG: no bracket at beginning of array\n");
		return NULL;
	}

	struct json_item *head = NULL;
	struct json_item **tail = &head;

	while (1) {
		c = fgetc(file);
		if (feof(file)) {
			goto eof_error;
		}

		if (is_whitespace(c)) {
			continue;
		}

		switch (c) {
			case 93: // ]
				goto graceful_end;
			case 44: // ,
				break;
			default:
				ungetc(c, file);
				struct json_item *item = json_item_parse(file);
				if (item == NULL) {
					goto element_error;
				}
				*tail = item;
				tail = &item->next;
		}
	}

	graceful_end:;
	struct json_item *result = json_array_create();
	struct json_array *array = (void*)&result->value;
	array->items = head;
		
	return result;

	element_error:;
	fprintf(stderr, "error: unknown error while parsing element of array\n");
	json_item_destroy(head);
	return NULL;

	eof_error:;
	fprintf(stderr, "error: EOF while parsing array\n");
	json_item_destroy(head);
	return NULL;
}

struct json_item *json_object_parse(FILE *file) {
	if (DEBUG)
		fprintf(stderr, "DEBUG: json_object_parse\n");
	int c = fgetc(file);
	if (c != 123) { // {
		if (DEBUG)
			fprintf(stderr, "DEBUG: no bracket at beginning of object\n");
		return NULL;
	}

	struct json_item *keys_head = NULL;
	struct json_item **keys_tail = &keys_head;
	struct json_item *items_head = NULL;
	struct json_item **items_tail = &items_head;
	struct json_item ***tail = &keys_tail;

	while (1) {
		c = fgetc(file);
		if (feof(file)) {
			goto eof_error;
		}

		if (is_whitespace(c)) {
			continue;
		}

		switch (c) {
			case 125: // }
				goto graceful_end;
			case 44: // ,
				tail = &keys_tail;
				break;
			case 58: // :
				tail = &items_tail;
				break;
			default:
				ungetc(c, file);
				struct json_item *item = json_item_parse(file);
				if (item == NULL) {
					goto element_error;
				}
				**tail = item;
				*tail = &item->next;
		}
	}

	graceful_end:;
	struct json_item *result = json_object_create();
	struct json_object *object = (void*)&result->value;
	object->keys = keys_head;
	object->items = items_head;

		
	return result;

	element_error:;
	fprintf(stderr, "error: unknown error while parsing element of object\n");
	goto fail;

	eof_error:;
	fprintf(stderr, "error: EOF while parsing array\n");
	goto fail;
	
	fail:;
	json_item_destroy(keys_head);
	json_item_destroy(items_head);
	return NULL;
}

struct json_item *json_item_parse(FILE *file) {
	if (DEBUG)
		fprintf(stderr, "DEBUG: json_item_parse\n");
	struct json_item *result = NULL;
	int c;
	while (1) {
		c = fgetc(file);
		if (feof(file)) {
			break;
		}

		if (is_whitespace(c)) {
			continue;
		}

		json_parse_fn parser = NULL;

		if (c >= 48 && c <= 57) { // 0 to 9 or -
			parser = &json_number_parse;
			goto recurse_parse;
		}

		switch (c) {
			case 34: // "
				parser = &json_string_parse;
				break;
			case 45: // -
				parser = &json_number_parse;
				break;
			case 102: //f for false
			case 110: //n for null
			case 116: //t for true
			case 117: //u for undefined
				parser = &json_token_parse;
				break;
			case 91: // [
				parser = &json_array_parse;
				break;
			case 123: // {
				parser = &json_object_parse;
				break;
			default:
				goto cancel_loop;
		}
		
		recurse_parse:;
		ungetc(c, file);
		if (DEBUG)
			fprintf(stderr, "DEBUG: json_item_parse unget '%c'\n", c);
		result = parser(file);
		break;
		cancel_loop:;
		if (DEBUG)
			fprintf(stderr, "DEBUG: json_item_parse cancel_loop %i\n", c);
		break;
	}
	return result;
}

int main(int argc, char **argv) {
	if (argc < 2) {
		return 1;
	}

	if (DEBUG)
		fprintf(stderr, "DEBUG\n");

	FILE *f = fopen(argv[1], "r");

	struct json_item *parsed_tree = json_item_parse(f);
	json_item_print(stdout, parsed_tree, 0);
	json_item_destroy(parsed_tree);

	fclose(f);
}
