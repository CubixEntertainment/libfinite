#ifndef __JSON_H__
#define __JSON_H__

// jsmn
#include "jsmn.h"

typedef struct FiniteJSONValue FiniteJSONValue;

struct FiniteJSONValue {
    char *key;
    char *value; // all values that are not arrays can be stored as strings
    FiniteJSONValue **members; // FiniteJSONValues that belong to this one
    int _members;
    jsmntype_t type;
};

#define finite_json_parse(data) finite_json_parse_debug(__FILE__, __func__, __LINE__, data)
FiniteJSONValue *finite_json_parse_debug(const char *file, const char *func, int line, char *data);

#define finite_json_get_value_from_key(item, key) finite_json_get_value_from_key_debug(__FILE__, __func__, __LINE__, item, key)
char *finite_json_get_value_from_key_debug(const char *file, const char *func, int line, FiniteJSONValue *item, char *key);

#define finite_json_get_value(item) finite_json_get_value_debug(__FILE__, __func__, __LINE__, item)
char *finite_json_get_value_debug(const char *file, const char *func, int line, FiniteJSONValue *item);

#define finite_json_cleanup(item) finite_json_cleanup_debug(__FILE__, __func__, __LINE__, item)
void finite_json_cleanup_debug(const char *file, const char *func, int line, FiniteJSONValue *item);

#define finite_json_get_index_from_key(item, key) finite_json_get_index_from_key_debug(__FILE__, __func__, __LINE__, item, key)
int finite_json_get_index_from_key_debug(const char *file, const char *func, int line, FiniteJSONValue *item, char *key);

#endif