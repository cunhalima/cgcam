#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <GL/glew.h>
#include "../defs.h"

static GLuint compile_shader(int type, const char *path) {
	GLuint ShaderID = glCreateShader(type == 0 ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string ShaderCode;
	std::ifstream ShaderStream(path, std::ios::in);
	if (ShaderStream.is_open()) {
		int lineno = 0;
		std::string Line = "";
		while (getline(ShaderStream, Line)) {
			lineno++;
			const char *ss = Line.c_str();
			if (rs.coreGL) {
				if (strncmp(ss, "invarying", 9) == 0) {
					Line.replace(0, 9, "in");
				} else if (strncmp(ss, "outvarying", 10) == 0) {
					Line.replace(0, 10, "out");
				} else if (strncmp(ss, "attribute", 9) == 0) {
					Line.replace(0, 9, "in");
				}
				if (lineno == 1) {
					Line = "#version 330 core";
				}
				if (type != 0) {
					if (lineno == 2) {
						ShaderCode += "\nout vec4 FragColor;";
					}
					size_t pos = Line.find("gl_FragColor");
					if (pos != std::string::npos) {
						Line.replace(pos, 3, "");
					}
				}
			} else {
				if (strncmp(ss, "invarying", 9) == 0) {
					Line.replace(0, 9, "varying");
				} else if (strncmp(ss, "outvarying", 10) == 0) {
					Line.replace(0, 10, "varying");
				}
			}
			ShaderCode += "\n" + Line;
        }
		//con_print(ShaderCode.c_str());
		ShaderStream.close();
	} else {
        glDeleteShader(ShaderID);
        CON_ERRS("unable to open %s", path);
		return 0;
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	char const * SourcePointer = ShaderCode.c_str();
	glShaderSource(ShaderID, 1, &SourcePointer , NULL);
	glCompileShader(ShaderID);

	// Check Vertex Shader
	glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(ShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (!Result && InfoLogLength > 0) {
		std::vector<char> ShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(ShaderID, InfoLogLength, NULL, &ShaderErrorMessage[0]);
        std::string errstr = "error compiling ";
        errstr += path;
        errstr += ": ";
        errstr += &ShaderErrorMessage[0];
        const char *sstr = errstr.c_str();
        CON_ERR(sstr);
        glDeleteShader(ShaderID);
        return 0;
	}
    return ShaderID;
}

glid_t RE_LoadShaders(const char* vert, const char* frag) {
    assert(vert != NULL);
    assert(frag != NULL);

    vert = ASS_GetShaderPath(vert);
    GLuint VertexShaderID = compile_shader(0, vert);
    if (VertexShaderID == 0) {
        return 0;
    }
    frag = ASS_GetShaderPath(frag);
    GLuint FragmentShaderID = compile_shader(1, frag);
    if (FragmentShaderID == 0) {
        glDeleteShader(VertexShaderID);
        return 0;
    }
	// Link the program
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (!Result && InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        CON_ERRS("compile error: %s", &ProgramErrorMessage[0]);
        glDeleteProgram(ProgramID);
        ProgramID = 0;
	}

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);
    return ProgramID;
}
