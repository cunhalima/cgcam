#include <string>
#include "../defs.h"

static const char *dataDir = "data/";
static std::string pathName;

const char *ASS_GetModelPath(const char *modelname, const char *filename) {
    pathName = dataDir;
    pathName += "models/";
    pathName += modelname;
    pathName += "/";
    pathName += filename;
    return pathName.c_str();
}

const char *ASS_GetScriptPath(const char *filename) {
    pathName = dataDir;
    pathName += "scripts/";
    pathName += filename;
    return pathName.c_str();
}

const char *ASS_GetShaderPath(const char *filename) {
    pathName = dataDir;
#if 1
    //if (rs.coreGL) {
    //    pathName += "33/";
    //} else {
        pathName += "21/";
    //}
#else
    if (rs.coreGL) {
        pathName += "33/";
    } else {
        pathName += "21/";
    }
#endif
    pathName += filename;
    return pathName.c_str();
}

const char *ASS_GetPath(const char *filename) {
    pathName = dataDir;
    pathName += filename;
    return pathName.c_str();
}
