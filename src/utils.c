#include "utils.h"

bool str_eq(const char* s1, const char* s2, int MAX_CMP) {
    if (s1 == s2) return true;
    for (u32 i = 0; i < MAX_CMP && *s1; i++) {
        if (s1[i] != s2[i])
            return false;
        if (s1[i] == '\0' && s2[i] == '\0') {
            return true;
        }
    }
    return true;
}

bool str_startswith(const char* target, const char* query, int MAX_CMP) {
    if (target == query) return true;
    if (target == NULL) return false;
    if (query == NULL) return true;
    for (u32 i = 0; i < MAX_CMP; i++) {
        if (query[i] == '\0')
            return true;
        if (target[i] != query[i])
            return false;
    }
    return true;
}