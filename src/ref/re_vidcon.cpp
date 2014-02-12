#include <cassert>
#include <cstring>
#include <cstdio>
#include <GL/glew.h>
//#include <GL/glfw.h>
#include "../defs.h"

static const int MAX_LINES = 256;
static const int SHOW_LINES = 16;
static const int MAX_EDLINE_LENGTH = 255;
static const int DEFAULT_LINE_LENGTH = 20; //80

static bool online = false;
static char *buf;
static char *lines[MAX_LINES];
static int line_maxlen;
static int cur_line;
static int cur_col;
static char edline[MAX_EDLINE_LENGTH + 1];
static int edline_len;

static void RE_AddString(const char *text, int start, int end, bool newline) {
	if (end <= start) {
		return;
	}
	int len = end - start;
	int screen_len = cur_col + len;
	if (screen_len > line_maxlen) {
		int dif = screen_len - line_maxlen;
		if (dif < len) {
			len -= dif;
		} else {
			len = 0;
		}
	}
	if (len > 0) {
		int row = cur_line % MAX_LINES;
		memcpy(&lines[row][cur_col], &text[start], len);
		lines[row][cur_col + len] = '\0';
	}
	if (newline) {
		cur_line++;
		cur_col = 0;
	} else {
		cur_col += len;
	}
}

static void RE_Print(const char *text) {
	if (!online) {
		return;
	}
	int start = 0;
	int i;
	for (i = 0; text[i] != '\0'; i++) {
		if (text[i] == '\n') {
			RE_AddString(text, start, i, true);
			start = i + 1;
		}
	}
	RE_AddString(text, start, i, false);
}

void RE_VidconReset() {
	if (!online) {
		return;
	}
	//line_maxlen = DEFAULT_LINE_LENGTH;
	line_maxlen = rs.xres / 8;
	memset(lines, 0, sizeof(lines));
	if (buf != NULL) {
		delete[] buf;
	}
	buf = new char[MAX_LINES * (line_maxlen + 1)];
	for (int i = 0; i < MAX_LINES; i++) {
		lines[i] = &buf[i * (line_maxlen + 1)];
	}
	cur_line = 0;
	cur_col = 0;
}

static void ed_addch(int ch)
{
	if (ch == 7) {
		if (edline_len > 0) {
			edline_len--;
			edline[edline_len] = '\0';
		}
		return;
	}
    if (edline_len >= MAX_EDLINE_LENGTH) {
        return;
    }
	edline[edline_len] = ch;
	edline_len++;
	edline[edline_len] = '\0';
}

static void ed_clear() {
	edline_len = 0;
	edline[edline_len] = '\0';
}

void RE_VidconKeyPress(int ch) {
	if (!online) {
		return;
	}
    if (ch == 295) { // backspace
        ed_addch(7);
    } else if (ch == 293) { // tab
        int count = 4 - (edline_len % 4);
        for (int i = 0; i < count; i++) {
            ed_addch(' ');
        }
    } else if (ch == 294) { //enter
        con_type(edline);
		ed_clear();
    }
}

void RE_VidconCharPress(int ch) {
	if (!online) {
		return;
	}
    if (ch >= ' ' && ch < 127) {
        ed_addch(ch);
    }
}

// Initialize the video console
void RE_VidconInit() {
	if (online) {
		return;
	}
	online = true;
	buf = NULL;
	edline_len = 0;
	edline[0] = '\0';
	RE_VidconReset();
	con_redirectOutput(RE_Print);
	CON_OK();
}

void RE_VidconCleanup() {
	if (!online) {
		return;
	}
	online = false;
	con_redirectOutput(NULL);
	delete[] buf;
	buf = NULL;
}

void RE_VidconDraw() {
	if (!online) {
		return;
	}
    if (!g_ConsoleOn) {
        return;
    }
	//RE_TextPrint(10, 10, "Oi");
	RE_2DSetColor(0.0f, 0.0f, 0.0f);
	RE_2DRect(0, rs.yres - 14 * (SHOW_LINES + 1), rs.xres, rs.yres);
	RE_2DRender();
	RE_2DSetColor(1.0f, 1.0f, 1.0f);
	ptrdiff_t start = cur_line - SHOW_LINES;
	int realRow;
	int dispRow = 0;
	for (ptrdiff_t i = start; i < ptrdiff_t(cur_line); i++, dispRow++) {
		if (i < 0) {
			continue;
		}
		//char nn[30];
		//sprintf_s(nn, "%d", i);
		realRow = i % MAX_LINES;
		//sprintf_s(nn, sizeof(nn), "%d %d", i, dispRow);
		//RE_TextPrint(0, dispRow, nn);
		RE_TextPrint(0, dispRow, lines[realRow]);
	}
	//dispRow = cur_line % MAX_LINES;
	RE_TextPrint(0, SHOW_LINES, "$");
	int st = 0;
	if (edline_len + 2 > line_maxlen) {
		st = int(edline_len + 2 - line_maxlen);
	}
	RE_TextPrint(1, SHOW_LINES, &edline[st]);
	if (g_Time & 256) {
		RE_TextPrint(int(edline_len) + 1 - st, SHOW_LINES, "_");
	}

}

void RE_VidconResize() {
	if (!online) {
		return;
	}
	RE_VidconReset();
}