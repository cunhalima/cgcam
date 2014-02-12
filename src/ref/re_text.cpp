#include <cstring>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "../defs.h"

#define NUM_FONTS   1

struct basic2d_s {
    std::vector<glm::vec2> XY;
    std::vector<glm::vec3> RGB;
    GLuint buf_xy;
    GLuint buf_rgb;
	GLuint id_xy;
	GLuint id_rgb;
	GLuint shader;
};

static basic2d_s basic2d;

static struct font_s {
    std::vector<glm::vec2> XY;
    std::vector<glm::vec2> UV;
    std::vector<glm::vec3> RGB;
    GLuint fbuf_xy;
    GLuint fbuf_uv;
    GLuint fbuf_rgb;
    int w, h;
} fonts[NUM_FONTS];

static GLuint fid_xy;
static GLuint fid_uv;
static GLuint fid_rgb;
static GLuint fid_tex;
static GLuint f_shader;
static int this_font;
static glm::vec3 fcolor = glm::vec3(1.0f, 1.0f, 1.0f);
static GLuint Text2DTextureID2;             // Texture containing the font

void RE_2DInit() {
	Text2DTextureID2 = RE_LoadTexture("font.tga", TEX_NN | TEX_REPEAT);
    this_font = 0;
    for (int i = 0; i < NUM_FONTS; i++) {
        glGenBuffers(1, &fonts[i].fbuf_xy);
        glGenBuffers(1, &fonts[i].fbuf_uv);
        glGenBuffers(1, &fonts[i].fbuf_rgb);
    }
    f_shader = RE_LoadShaders("font.vert", "font.frag");
    fid_xy  = glGetAttribLocation(f_shader, "xy_in");
    fid_uv  = glGetAttribLocation(f_shader, "uv_in");
    fid_rgb = glGetAttribLocation(f_shader, "rgb_in");
    fid_tex = glGetUniformLocation(f_shader, "tex");
	
    fonts[0].w = 8;
    fonts[0].h = 14;

	glGenBuffers(1, &basic2d.buf_xy);
	glGenBuffers(1, &basic2d.buf_rgb);
    basic2d.shader = RE_LoadShaders("basic2d.vert", "basic2d.frag");
    basic2d.id_xy  = glGetAttribLocation(basic2d.shader, "xy_in");
    basic2d.id_rgb = glGetAttribLocation(basic2d.shader, "rgb_in");
}

void RE_TextClear()
{
    for (int i = 0; i < NUM_FONTS; i++) {
        fonts[i].XY.clear();
        fonts[i].UV.clear();
        fonts[i].RGB.clear();
    }
}

void RE_2DClear() {
	basic2d.XY.clear();
    basic2d.RGB.clear();
}

void RE_2DSetColor(float r, float g, float b)
{
    fcolor = glm::vec3(r, g, b);
}

void RE_TextRender() {
	int hasSomething = 0;
    for (int i = 0; i < NUM_FONTS; i++) {
		hasSomething += int(fonts[i].XY.size());
	}
	if (hasSomething == 0) {
		return;
	}
    // Bind shader
    glUseProgram(f_shader);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (int i = 0; i < NUM_FONTS; i++) {
        if (fonts[i].XY.size() == 0) {
            continue;
        }
        glBindBuffer(GL_ARRAY_BUFFER, fonts[i].fbuf_xy);
        glBufferData(GL_ARRAY_BUFFER, fonts[i].XY.size() * sizeof(glm::vec2), &fonts[i].XY[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, fonts[i].fbuf_uv);
        glBufferData(GL_ARRAY_BUFFER, fonts[i].UV.size() * sizeof(glm::vec2), &fonts[i].UV[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, fonts[i].fbuf_rgb);
        glBufferData(GL_ARRAY_BUFFER, fonts[i].RGB.size() * sizeof(glm::vec3), &fonts[i].RGB[0], GL_STATIC_DRAW);

        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Text2DTextureID2);

        // Set our "myTextureSampler" sampler to user Texture Unit 0
        glUniform1i(fid_tex, 0);

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(fid_xy);
        glBindBuffer(GL_ARRAY_BUFFER, fonts[i].fbuf_xy);
        glVertexAttribPointer(fid_xy, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // 2nd attribute buffer : UVs
        glEnableVertexAttribArray(fid_uv);
        glBindBuffer(GL_ARRAY_BUFFER, fonts[i].fbuf_uv);
        glVertexAttribPointer(fid_uv, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // 3rd attribute buffer : RGBs
        glEnableVertexAttribArray(fid_rgb);
        glBindBuffer(GL_ARRAY_BUFFER, fonts[i].fbuf_rgb);
        glVertexAttribPointer(fid_rgb, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // Draw call
        glDrawArrays(GL_TRIANGLES, 0, GLsizei(fonts[i].XY.size()));

        glDisableVertexAttribArray(fid_rgb);
        glDisableVertexAttribArray(fid_uv);
        glDisableVertexAttribArray(fid_xy);
    }
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
	RE_TextClear();
}

void RE_TextPrint(int x, int y, const char *text) {
    int i = this_font;
    float fontw = float(fonts[i].w);
    float fonth = float(fonts[i].h);

    x *= int(fontw);
    y *= int(fonth);

    if (x < 0) {
        x += rs.xres;
    }
    if (y < 0) {
        y += rs.yres;
    }
	float xres_f = float(rs.xres);
	float yres_f = float(rs.yres);

    float xchars = xres_f / fontw;
    float ychars = yres_f / fonth;

    float scw = fontw/xres_f*2.0f;
    float sch = fonth/yres_f*2.0f;
    int length = int(strlen(text));

    float xchars2 = xchars / 2.0f;
    float ychars2 = ychars / 2.0f;

    float scx;
    scx = float(x);
    scx -= (xres_f/2.0f);
    scx /= (xres_f/2.0f);

    float scy;
    scy = float(y);
    scy -= (yres_f/2.0f);
    scy /= (yres_f/2.0f);
    scy = -scy;

    float uv_p = 1.0f / 16.0f / fontw;
    float uv_p2 = uv_p / 2.0f;
    float uv_w = fontw * uv_p;
    float uv_h = fonth * uv_p;

    for (int j = 0; text[j] != '\0'; j++) {
        int character = text[j];
        if (character == ' ' || character >= 127) {
            continue;
        }
        character -= ' ';

        glm::vec2 vertex_up_left    = glm::vec2( scx+j*scw     , scy );
        glm::vec2 vertex_up_right   = glm::vec2( scx+j*scw+scw,  scy );
        glm::vec2 vertex_down_right = glm::vec2( scx+j*scw+scw,  scy-sch      );
        glm::vec2 vertex_down_left  = glm::vec2( scx+j*scw     , scy-sch      );

        fonts[i].XY.push_back(vertex_up_left   );
        fonts[i].XY.push_back(vertex_down_left );
        fonts[i].XY.push_back(vertex_up_right  );
        fonts[i].XY.push_back(vertex_down_right);
        fonts[i].XY.push_back(vertex_up_right);
        fonts[i].XY.push_back(vertex_down_left);

        float uv_x = (character % 16) * fontw * uv_p;
        float uv_y = (character / 16) * fonth * uv_p;

        glm::vec2 uv_up_left    = glm::vec2( uv_p2+uv_x           , 1.0f - uv_y+uv_p2 );
        glm::vec2 uv_up_right   = glm::vec2( uv_p2+uv_x+uv_w, 1.0f - uv_y+uv_p2 );
        glm::vec2 uv_down_right = glm::vec2( uv_p2+uv_x+uv_w, 1.0f - (uv_y + uv_h)+uv_p2 );
        glm::vec2 uv_down_left  = glm::vec2( uv_p2+uv_x           , 1.0f - (uv_y + uv_h)+uv_p2 );

        fonts[i].UV.push_back(uv_up_left   );
        fonts[i].UV.push_back(uv_down_left );
        fonts[i].UV.push_back(uv_up_right  );

        fonts[i].UV.push_back(uv_down_right);
        fonts[i].UV.push_back(uv_up_right);
        fonts[i].UV.push_back(uv_down_left);

        for (int k = 0; k < 6; k++) {
            fonts[i].RGB.push_back(fcolor);
        }
    }
 
}

void RE_2DCleanup()
{
    glDeleteBuffers(1, &basic2d.buf_xy);
    glDeleteBuffers(1, &basic2d.buf_rgb);
    glDeleteProgram(basic2d.shader);

    for (int i = 0; i < NUM_FONTS; i++) {
        glDeleteBuffers(1, &fonts[i].fbuf_xy);
        glDeleteBuffers(1, &fonts[i].fbuf_uv);
        glDeleteBuffers(1, &fonts[i].fbuf_rgb);
    }
    glDeleteProgram(f_shader);
	glDeleteTextures(1, &Text2DTextureID2);
}

void RE_2DRender()
{
    if (basic2d.XY.size() == 0) {
		return;
	}
    // Bind shader
    glUseProgram(basic2d.shader);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindBuffer(GL_ARRAY_BUFFER, basic2d.buf_xy);
    glBufferData(GL_ARRAY_BUFFER, basic2d.XY.size() * sizeof(glm::vec2), &basic2d.XY[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, basic2d.buf_rgb);
    glBufferData(GL_ARRAY_BUFFER, basic2d.RGB.size() * sizeof(glm::vec3), &basic2d.RGB[0], GL_STATIC_DRAW);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(basic2d.id_xy);
    glBindBuffer(GL_ARRAY_BUFFER, basic2d.buf_xy);
    glVertexAttribPointer(basic2d.id_xy, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // 3rd attribute buffer : RGBs
    glEnableVertexAttribArray(basic2d.id_rgb);
    glBindBuffer(GL_ARRAY_BUFFER, basic2d.buf_rgb);
    glVertexAttribPointer(basic2d.id_rgb, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // Draw call
    glDrawArrays(GL_TRIANGLES, 0, GLsizei(basic2d.XY.size()));
    glDisableVertexAttribArray(basic2d.id_rgb);
    glDisableVertexAttribArray(basic2d.id_xy);

    //glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
	RE_2DClear();
}

void RE_2DRect(int x1, int y1, int x2, int y2) {
	float x[2];
	float y[2];

	float fxres = float(rs.xres) / 2.0f;
	float fyres = float(rs.yres) / 2.0f;
	x[0] = (x1 - fxres) / fxres;
	x[1] = (x2 - fxres) / fxres;
	y[0] = (y1 - fyres) / fyres;
	y[1] = (y2 - fyres) / fyres;

	basic2d.XY.push_back(glm::vec2(x[0], y[0]));
	basic2d.XY.push_back(glm::vec2(x[1], y[0]));
	basic2d.XY.push_back(glm::vec2(x[1], y[1]));
	basic2d.XY.push_back(glm::vec2(x[0], y[1]));
	basic2d.XY.push_back(glm::vec2(x[0], y[0]));
	basic2d.XY.push_back(glm::vec2(x[1], y[1]));
    for (int k = 0; k < 6; k++) {
        basic2d.RGB.push_back(fcolor);
    }	
}
