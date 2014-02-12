#if defined( __GNUC__ )

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>
#include <cstdio>
#include <cstdarg>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>
#include "../defs.h"

// TERMIOS stuff ======================

static std::string kbd_buf;
static struct termios orig_termios;
static struct termios new_termios;
static bool input_on;

static void set_raw(bool raw)
{
    assert(input_on);
    if (raw) {
        tcsetattr(0, TCSANOW, &new_termios);
    } else {
        tcsetattr(0, TCSANOW, &orig_termios);
    }
}

static int tty_erase;

static void init_term()
{
    assert(input_on);
    tcgetattr(0, &orig_termios);
    tty_erase = orig_termios.c_cc[VERASE];
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));
    cfmakeraw(&new_termios);
}

static int kbhit()
{
    assert(input_on);
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

static int getch()
{
    assert(input_on);
    int r;
    unsigned char c;
    if ((r = read(0, &c, sizeof(c))) < 0) {
        return r;
    } else {
        return c;
    }
}

//======================================

bool Syscon_Init()
{
    input_on = true;
    init_term();
    set_raw(false);
    con_redirectOutput(Syscon_Print);
    return true;
}

void Syscon_Cleanup() {

    con_redirectOutput(NULL);
    if (input_on) {
        set_raw(false);
    }
}

void Syscon_Print(const char *msg) {
    if (input_on) {
        set_raw(false);
    }
    printf("%s", msg);
    fflush(stdout);
    if (input_on) {
        set_raw(true);
    }
}

void Syscon_ProcessEvents() {
    if (!input_on) {
        return;
    }
    while (kbhit()) {
        int r;
        char cc[2];
        cc[0] = getch();
        if (cc[0] == tty_erase) {
            int len = kbd_buf.size();
            if (len > 0) {
                r = write(1, "\b \b", 3);
                kbd_buf.resize(len - 1);
            }
            continue;
        }
        r = write(1, &cc, 1);
        if (cc[0] == '\r') {
            cc[0] = '\n';
            r = write(1, &cc, 1);
        }
        cc[1] = '\0';
        kbd_buf += cc;
        if (cc[0] == '\n') {

            con_type(kbd_buf.c_str());

            kbd_buf.clear();
        }
    }
}

void SYS_ShowError(const char *text) {
    if (input_on) {
        set_raw(false);
    }
    printf("Error: %s", text);
    fflush(stdout);
    if (input_on) {
        set_raw(true);
    }
}

void SYS_ReadOptions(int argc, char **argv) {
    readoptions(argc, argv);

    // FIXME: should read from cmfline
    opt.client = true;
    opt.server = true;
    opt.xres = 640;
    opt.yres = 400;
    opt.fullscreen = false;

}

#endif
