#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <map>
#include <cassert>
#include "../defs.h"

static void cvar_set_internal(cvar_t *cvar, const char *str) {
	if (cvar->str != NULL) {
		delete[] cvar->str;
	}
	cvar->str = my_strdup(str);
}

void cvar_set(cvar_t *cvar, const char *str) {
	assert(cvar != NULL);
	assert(str != NULL);
	cvar_set_internal(cvar, str);
	cvar->integer = atoi(str);
	cvar->number = float(atof(str));
}

void cvar_set(cvar_t *cvar, float number) {
	assert(cvar != NULL);
	char str[128];
	//sprintf_s(str, sizeof(str), "%f", number);
    //FIXME: possible buffer overflow
	sprintf(str, "%f", number);
	cvar_set_internal(cvar, str);
	cvar->integer = int(number);
	cvar->number = number;
}

void cvar_set(cvar_t *cvar, int integer) {
	assert(cvar != NULL);
	char str[128];
	//sprintf_s(str, sizeof(str), "%d", integer);
    //FIXME: possible buffer overflow
	sprintf(str, "%d", integer);
	cvar_set_internal(cvar, str);
	cvar->integer = integer;
	cvar->number = float(integer);
}

struct cmp_str {
    bool operator()(char const * a, char const *b) {
        return std::strcmp(a, b) < 0;
    }
};

static std::map<const char *, cvar_t *, cmp_str> vnames;

cvar_t *cvar_get(const char *name) {
	assert(name != NULL);
    std::map<const char *, cvar_t *, cmp_str>::iterator it;
    it = vnames.find(name);
    if (it == vnames.end()) {
        return NULL;
    }
    return vnames[name];
}

cvar_t *cvar_register(const char *name, const char *v) {
	cvar_t *cvar = cvar_get(name);
	if (cvar == NULL) {
		cvar = new cvar_t();
		vnames[name] = cvar;
	}
	if (v == NULL) {
		v = "";
	}
	cvar_set(cvar, v);
	return cvar;
}
