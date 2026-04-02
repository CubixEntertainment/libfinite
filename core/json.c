#include "json.h"
#include "log.h"
#include "utils/jsmn.h"
#include <stdlib.h>
#include <string.h>

// parses arrays
FiniteJSONValue *finite_json_parse_debug(const char *file, const char *func, int line, char *data) {
    jsmn_parser p;
    jsmntok_t tokens[128];

    jsmn_init(&p);
    int r = jsmn_parse(&p, data, strlen(data), tokens, 128);
    if (r <= 0) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to parse");
        return NULL;
    }

    bool isKey = true;
    FiniteJSONValue *values = calloc(1, sizeof(FiniteJSONValue));
    values->key = "";
    values->value = data;
    int _keys = 0;
    for (int i = 1; i < r; i++) {
        int len = tokens[i].end - tokens[i].start;
        char item_token[len + 1];
        strncpy(item_token, data + tokens[i].start, len);
        item_token[len] = '\0';
        finite_log_internal(LOG_LEVEL_DEBUG, file, line, func, "item: %s (isKey: %s) [%d]", item_token, isKey == true ? "true" : "false", i);
        // if iskey then create a new FiniteJSONValue and add it to the array. Otherwise get the latest item (_keys) and add the value to it. If its an object do a recursive search
        if (isKey) {
            FiniteJSONValue *item = calloc(1, sizeof(FiniteJSONValue));
            item->key = malloc(len);
            strncpy(item->key, item_token, len);
            item->key[len] = '\0';
            FiniteJSONValue **tmp = realloc(values->members, sizeof(FiniteJSONValue *) * (_keys + 1));
            if (!tmp) {
                finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to parse (no memory)");
            }
            values->members = tmp;
            values->members[_keys] = item;
            isKey = false;
        } else {
            if (tokens[i].type == JSMN_OBJECT) {
                FiniteJSONValue *children =  finite_json_parse(item_token);
                FiniteJSONValue *item = values->members[_keys];
                item->members[_keys] = children;
            } else {
                FiniteJSONValue *item = values->members[_keys];
                item->value = malloc(len);
                strncpy(item->value, item_token, len);
                item->type = tokens[i].type;
                _keys++;
                isKey = true;
            }
        }

    }

    values->_members = _keys;
    values->type = JSMN_OBJECT;

    return values;
}

char *finite_json_get_value_debug(const char *file, const char *func, int line, FiniteJSONValue *item) {
    if (item->type == JSMN_OBJECT) {
        finite_log_internal(LOG_LEVEL_WARN, file, line, func, "This function returns the char value of the json type. You're probably looking for finite_json_get_value_from_key().");
    }
    return item->value;
}


char *finite_json_get_value_from_key_debug(const char *file, const char *func, int line, FiniteJSONValue *item, char *key) {
    if (!key) {
       return NULL;
    }


    if (strcmp(item->key, key) == 0) {
        finite_log_internal(LOG_LEVEL_DEBUG, file, line, func, "Key (inherited): %s", item->key);
        return item->value;
    }

    for (int i = 0; i < item->_members; i++) {
        finite_log_internal(LOG_LEVEL_INFO, file, line, func, "Key %s value: %s", item->members[i]->key, item->members[i]->value);
        if (strcmp(item->members[i]->key, key) == 0) {
            finite_log_internal(LOG_LEVEL_DEBUG, file, line, func, "Key (found): %s", item->members[i]->key);
            return item->members[i]->value;
        }
    }

    return NULL;
}

void finite_json_cleanup_debug(const char *file, const char *func, int line, FiniteJSONValue *item) {
    for (int i = 0; i < item->_members; i++) {
        free(item->members[i]);
    }

    free(item);
}