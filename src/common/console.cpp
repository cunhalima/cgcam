#define _CRT_SECURE_NO_WARNINGS
#include <vector>
#include <map>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include "../defs.h"


#if 0
/*
#include <cstdlib>
#include <cstring>
#include <string>
#include <cstdio>
*/
//#include "luacon.h"
//extern "C" {
//#include <lualib.h>
//};

//======================================
lua_State *g_L;

static int l_exit(lua_State* L) {
    g_quitOrder = true;
    return 0;
}

static int l_print(lua_State* L) {
    int nargs = lua_gettop(L);

    for (int i=1; i <= nargs; i++) {
        if (lua_isstring(L, i)) {
            con_print(lua_tostring(L, i));
        } else if (lua_isboolean(L, i)) {
            if (lua_toboolean(L, i)) {
                con_print("true");
            } else {
                con_print("false");
            }
        } else if (lua_isnil(L, i)) {
            con_print("nil");
        }
    }
    return 0;
}

static const struct luaL_Reg printlib [] = {
    {"print", l_print},
    {"exit", l_exit},
    {NULL, NULL},
};

void con_regfuncs(const luaL_Reg *reg)
{
    //lua_pushglobaltable(L);
    lua_getglobal(g_L, "gg");
    luaL_setfuncs(g_L, reg, 0); 
    lua_pop(g_L, 1);
}
    input_on = inpon;
    if (input_on) {
        init_term();
        set_raw(true);
    }

    g_L = luaL_newstate();
    lua_State *L = g_L;
    luaL_openlibs(L);
    lua_newtable(L);
    lua_setglobal(L, "gg");
    lua_pushglobaltable(L);
    luaL_setfuncs(L, printlib, 0); 
    lua_pop(L, 1);
    con_regfuncs(printlib);
    luaL_dostring(L, "package.path = package.path .. \";data/scripts/?.lua\"");

	    lua_close(g_L);
    if (input_on) {
        set_raw(false);
    }
void con_print(const char *msg)
{
    if (input_on) {
        set_raw(false);
    }
    printf("%s", msg);
    fflush(stdout);
    if (input_on) {
        set_raw(true);
    }
}

void con_run(const char *text)
{
    char *st;

    //printf("<<%s>>\n", text);
    if (strncmp(text, "quit\n", 5) == 0) {
        g_quitOrder = true;
        return;
    }
    std::string cmd;
    if (text[0] == '?') {
        cmd = "print(";
        cmd += &text[1];
        cmd += ",\"\\n\");";
        text = cmd.c_str();
        //con_printf("<<%s>>", text);
    }
    //luaL_loadstring(L, text);
    //int result = lua_pcall(L, 0, 0, 0);
    //int result = lua_pcall(L, 0, LUA_MULTRET, 0);
    //if (result) {
    //    con_print("error\n");
    //}
    if (luaL_dostring(g_L, text)) {
        con_printf("script error\n");
    }

}

void con_runFile(const char *filename)
{
    const char *path = ass_getScriptPath(filename);
    //if (luaL_dofile(g_L, path)) {
    //    con_printf("error running script %s\n", path);
    //}
    int error = luaL_loadfile(g_L, path);
    if (error) {
        if (!lua_isstring(g_L, lua_gettop(g_L))) {
            CON_ERR("strange error");
            return;
        }
        const char *str = lua_tostring(g_L, lua_gettop(g_L));
        CON_ERR(str);
        lua_pop(g_L, 1);
        return;
    } else {
        // if not an error, then the top of the stack will be the function to call to run the file
        //lua_pcall(g_L, 0, LUA_MULTRET, 0); // once again, returns non-0 on error, you should probably add a little check
        error = lua_pcall(g_L, 0, LUA_MULTRET, 0);
        if (error) {
            if (!lua_isstring(g_L, lua_gettop(g_L))) {
                CON_ERR("strange error");
                return;
            }
            const char *str = lua_tostring(g_L, lua_gettop(g_L));
            CON_ERR(str);
            lua_pop(g_L, 1);
            return;
        }
    }
}

float cvar_float(const char *name)
{
    lua_getglobal(g_L, name);
    float v = lua_tonumber(g_L, -1);
    lua_pop(g_L, 1);
    return v;
}

int cvar_int(const char *name)
{
    lua_getglobal(g_L, name);
    int v = lua_tointeger(g_L, -1);
    lua_pop(g_L, 1);
    return v;
}

bool cvar_bool(const char *name)
{
    lua_getglobal(g_L, name);
    bool v = lua_toboolean(g_L, -1);
    lua_pop(g_L, 1);
    return v;
}

const char *cvar_str(const char *name)
{
    static std::string s;
    lua_getglobal(g_L, name);
    s = lua_tostring(g_L, -1);
    lua_pop(g_L, 1);
    return s.c_str();;
}

void cvar_set(const char *name, const char *value)
{
    lua_pushstring(g_L, value);
    lua_setglobal(g_L, name);
}

void cvar_set(const char *name, float value)
{
    lua_pushnumber(g_L, value);
    lua_setglobal(g_L, name);
}

void cvar_set(const char *name, int value)
{
    lua_pushinteger(g_L, value);
    lua_setglobal(g_L, name);
}

void con_checkInput()
{
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
            con_run(kbd_buf.c_str());
            kbd_buf.clear();
        }
    }
}

void con_stackDump() {
    lua_State *L = g_L;
    int i=lua_gettop(L);
    con_printf("----- Stack Dump -----\n" );
    while(  i   ) {
        int t = lua_type(L, i);
        switch (t) {
            case LUA_TSTRING:
                con_printf("%d:`%s'\n", i, lua_tostring(L, i));
                break;
            case LUA_TBOOLEAN:
                con_printf("%d: %s\n",i,lua_toboolean(L, i) ? "true" : "false");
                break;
            case LUA_TNUMBER:
                con_printf("%d: %g\n",  i, lua_tonumber(L, i));
                break;
            default: con_printf("%d: %s\n", i, lua_typename(L, t)); break;
        }
        i--;
    }
    con_printf("- Stack Dump Finished -\n" );
}


#endif

static bool con_ok = false;
static FILE* logfile = NULL;
static conprintfunc_t cpfunc = NULL;

void con_init() {
	if (!opt.nolog) {
		char ss[32];
		sprintf(ss, "game%d.log", opt.winpos);
		logfile = fopen(ss, "wt");
	}
	con_ok = true;
    CON_OK();
}

void con_cleanup() {
	con_ok = false;
	if (logfile != NULL) {
		fclose(logfile);
		logfile = NULL;
	}
}

void con_logprint(const char *msg) {
	assert(msg != NULL);
	if (!con_ok) {
		return;
	}
	if (logfile) {
		fprintf(logfile, "%s", msg);
	}
}

void con_print(const char *msg) {
	assert(msg != NULL);
	if (!con_ok) {
		return;
	}
	con_logprint(msg);
	if (cpfunc != NULL) {
		cpfunc(msg);
	}
    //Syscon_Print(msg);
}

void con_printf(const char *fmt, ...)
{
    char text[1024];
    va_list ap;
    if (fmt == NULL)
        return;      
    va_start(ap, fmt);

#if defined(_MSC_VER)
	vsnprintf_s(text, sizeof(text), fmt, ap);
#else
    vsnprintf(text, sizeof(text), fmt, ap);
#endif
    va_end(ap);    
    con_print(text);
}

void con_redirectOutput(conprintfunc_t func) {
	cpfunc = func;
	//con_printf("redir to %p\n", func);
}

#if 0
/*
Quake Console Commands
by Niklata
The latest version of the Quake Console Commands List is available at Niklata's Technical Resources.

Here's the most comprehensive list of Quake console commands that I know of. I've browsed through QUAKE.EXE Shareware 1.00 and have dug out as many commands as I can find :). I've also included the documented commands so that you can have a single index of every Quake command you could possibly want.

Table of Contents

1.1: Client Movement Commands 
1.2: Mouse-Specific Commands 
1.3: v_ Commands 
1.4: Standard Quake Variables 
1.5: Standard Console Commands 
1.6: CD-Specific Commands 
1.7: Video Mode Commands 
1.8: Technical Graphics Commands 
1.9: Ambient Sound Commands 
1.10: Sound Control Commands 
1.11: Screen-Related Commands 
1.12: Console Commands 
1.13: Host Commands 
1.14: Dedicated Server Commands 
1.15: Network Commands 
1.16: Menu-Related Commands 
1.17: COM Port Commands 
1.18: MIP-Texture-Related Commands 
1.19: Movement Control Commands 
1.20: Environment Commands 
1.1 Client Movement Commands

cl_rollspeed  - adjusts amount of screen rolling when moving forwards and turning
cl_rollangle  - adjusts the angle that the screen rolls towards when moving
cl_bob <1|0> - if zero, then no weapon bobbing
cl_bobcycle  - adjusts time between each bob
cl_bobup  - adjusts amount that the weapon bobs
cl_forwardspeed  - adjusts maximum forwards speed
cl_backspeed  - adjusts maximum backwards speed
cl_upspeed  - adjusts maximum move up/down speed
cl_sidespeed  - adjusts maximum strafe speed
cl_movespeedkey  - adjusts maximum run speed
cl_pitchspeed  - ??
cl_anglespeedkey  - ??
cl_nolerp <1|0> - ??

_cl_name  - changes player name
_cl_color  - changes player color
Back to TOC
1.2 Mouse-Specific Commands

m_pitch  - adjusts look up/down sensitivity of mouse
m_yaw  - adjusts turn side/side sensitivity of mouse
m_forward  - adjusts forward/back sensitivity of mouse
m_side  - adjusts strafe left/right sensitivity of mouse
m_filter <1|0> - ??
Back to TOC
1.3v_ Commands

v_kickroll  - ??
v_kickpitch  - ??
v_iyaw_cycle  - ??
v_iroll_cycle  - ??
v_ipitch_cycle  - ??
v_iyaw_level  - ??
v_ipitch_level  - ??
v_idlescale  - ??
Back to TOC
1.4 Standard Quake Variables

crosshair <1|0> - toggles display of crosshair
showturtle <1|0> - turtle never displayed if zero
showram <1|0> - low ram never displayed if zero
showpause <1|0> - pause box not displayed if zero
bgmvolume  - sets background music volume
gamma  - sets gamma correction level (screen brightness)
sensitivity  - sets mouse sensitivity
volume  - sets sound fx music volume
lookspring <1|0> - centers view after jump
lookstrafe <1|0> - centers view after strafe
noexit <1|0> - if one then no one can exit the level
skill  - sets skill level (0 == easy, 1 == normal, 2 == hard, 3 == nightmare)
deathmatch <1|0> - if one, deathmatch mode; if two, old deathmatch mode
coop <1|0> - if one, cooperative mode
pausable <1|0> - if zero, game cannot be paused
fraglimit  - when fraglimit is reached in deathmatch, game ends with end screen
hostname  - name of the server
maxplayers  - maximum number of players allowed on server
registered <1|0> - if registered == 1 then "trigger_registered" triggers function.
name  - name of the player (client)
listen <1|0> - if one, listens for clients wanting to join the game
joystick <1|0> - zero means no joystick support
joybuttons  - number of buttons on joystick
Back to TOC
1.5 Standard Console Commands

alias  - assigns a list of commands to a console command (a macro)
bf - flashes the screen
bind  - binds a command to a key
unbind  - unbinds the command from a key
unbindall - unbinds all custom bindings
save  - saves game
load  - loads game
screenshot - takes a screenshot
sizeup - sizes the screen up by one
sizedown - sizes the screen down by one
status - displays status info
quit - quits the game
god - toggles invincibility
notarget - toggles "monsters can't see you" mode
fly - toggles fly mode
map  - starts a new server in a map
restart - restarts the server
changelevel  - changes maps without terminating the server
connect  - connects to a server
reconnect  - reconnects to a server
spawn - will not work from the console
slist - shows all currently running servers accessible
begin - will not work from the console
prespawn - will not work from the console
kick  - kicks someone out of the game
ping - returns lag times from all players
give   - gives you a weapon or item
impulse  - switches/toggles weapons; 9 all weapons; -1 quad damage
startdemos  - starts a demo loop
noclip - toggles no clipping mode
name  - changes the client name
version - displays Quake version information
say  - says something to all players
say_team  - says something to all players on your team
tell   - says a message to just one player
color   - changes color of the player
kill  - kills a player
sbinfo - displays sound card information
soundinfo - displays portable sound information
pause - pauses the game
demos - starts game's demo loop
stopdemo - stops a demo's playback
playdemo  - plays back a demo
timedemo  - plays back a demo and times its duration
timerefresh - times the FPS of a scene
viewmodel - ??
viewframe - ??
viewnext - ??
viewprev - ??
help - brings up the Quake help screen
togglemenu - brings the menu up
toggleconsole - brings the console up
clear - clears the console backscroll
messagemode - puts up the "say:" prompt
messagemode2 - puts up the "say:" prompt
exec  - runs a cfg file
echo  - echos a message to the screen
cmd  - executes a QuakeC script?
stuffcmds - ??
wait - pauses between commands
Back to TOC
1.6 CD-Specific Commands

cd on - re-enables the CD subsystem
cd off - disables the CD subsystem
cd reset - causes CD subsystem to reinitalize; useful if you changed CDs
cd play  - plays the specified track
cd loop  - keeps playing the specified track
cd stop - stops playing the current track
cd resume - resumes playback from a stop
cd eject - ejects the CD
cd remap   ... - remaps CD tracks so 1st track is track one.. etc..
cd info - reports information on the CD
Back to TOC
1.7 Video Mode Commands

vid_mode  - changes the video mode
vid_describemodes - tells all video modes Quake can use
vid_describecurrentmode - describes video mode Quake is currently in
vid_describemode  - tells the mode Quake is currently running at
vid_testmode  - tests a video mode
vid_nummodes - tells total number of modes available to Quake
vid_nopageflip <1|0> - if zero, then page-flipped modes are selected whenever possible
vid_wait  - 0: no wait 1: wait/vertical sync active 2: wait/display enable active

_vid_wait_override <1|0> - if one, forces vertical sync wait in all modes
_vid_default_mode  - specifies default startup video mode
Back to TOC
1.8 Technical Graphics Commands

r_drawflat <1|0> - if one then textures are not drawn
r_fullbright <1|0> - if one then everything is drawn at maximum light level
r_ambient  - adjusts ambient light level
r_speeds <1|0> - shows speed of screen redraws/shows number of polygons
r_timegraph <1|0> - toggles display of graph showing time to draw screen
r_graphheight  - adjusts height of graph
r_clearcolor  - changes transparency color
r_waterwarp <1|0> - if zero then there is no wave effect while under water
r_drawentities <1|0> - if zero then entities are not drawn
r_polymodelstats <1|0> - displays number of polygon models shown
r_dspeeds <1|0> - displays speeds of some sort
r_reportsurfout  - ??
r_aliastransadj  - ??
r_aliastransbase  - ??
r_maxsurfs  - sets maximum number of brush planes to be displayed at a time
r_numsurfs <1|0> - shows number of brush planes displayed at a time
r_reportedgeout  - ??
r_maxedges  - sets maximum number of edges to be displayed at a time
r_numedges <1|0> -  shows number of edges displayed at a time
Back to TOC

1.9 Ambient Sound Commands

ambient_level  - adjusts volume of ambient sounds
ambient_fade  - adjusts level of fading for ambient sounds
Back to TOC

1.10 Sound Control Commands

snd_noextraupdate <1|0> - messes up sounds; may speed up slower computers
snd_show <1|0> - shows all playing sounds
Back to TOC

1.11 Screen-Related Commands

scr_conspeed  - speed at which console pulls down
scr_centertime - ??
scr_printspeed - ??
scr_ofsx  - adjusts viewpoint x offset
scr_ofsy  - adjusts viewpoint y offset
scr_ofsz  - adjusts viewpoint z offset
Back to TOC

1.12 Console Commands

con_notifytime - ??
Back to TOC

1.13 Host Commands

host_framerate  - wierd command.. messes up if one
host_speeds <1|0> - shows total time spent on frame, server, graphics, and sound.
Back to TOC

1.14 Dedicated Server Commands

sys_ticrate  - adjusts rate of updates sent out per second (dedicated server only)
Back to TOC

1.15 Network Commands

net_messagetimeout  - time with no response before Quake assumes connection is dead
net_stats - displays network information
Back to TOC

1.16 Menu-Related Commands

menu_main - pulls up main menu
menu_singleplayer - pulls up single player menu
menu_load - pulls up load game menu
menu_save - pulls up save game menu
menu_multiplayer - pulls up multiplayer menu
menu_setup - pulls up multiplayer setup menu
menu_options - pulls up general options menu
menu_keys - pulls up customize keys menu
menu_video - pulls up change video mode menu
Back to TOC

1.17 COM Port Commands

com1 / com2 - displays com port status
com#  - enables or disables com port
com#  - select modem if using a modem; select direct if null modem
com# reset - resets settings back to normal
com# port <#> - sets port to #
com# irq <#> - sets irq to #
com# baud <#> - sets baud to #
com# <8250|16550> - selects UART type
com# clear  - selects clear modem command string
com# startup  - selects startup modem command string
com# shutdown  - selects shutdown modem command string
com# <-cts|+cts> - enables/disables CTS
com# <-dsr|+dsr> - enables/disables DSR
com# <-cd|+cd> - enables/disables carrier detection
Back to TOC

1.18 MIP-Texture-Related Commands

d_subdiv16 <1|0> - if zero, perspective correction is done more precisely
d_mipcap <1|0> - if one, forces use of non-detailed mip textures
d_mipscale <1|0> - if zero, forces use of max detail mip textures
Back to TOC

1.19 Movement Control Commands

+moveup - moves up
+klook - keyboard look toggle
+mlook - mouse look toggle
+lookdown - looks down
+lookup - looks up
+strafe - strafe toggle
+moveleft - strafes to the left
+moveright - strafes to the right
+speed - run toggle
+right - turns right
+left - turns left
+back - moves back
+forward - moves forward
+jump - jumps
+attack - attacks
+use - uses object
+impulse 10 - cycles through weapons
Back to TOC

1.20 Environment Commands

sv_aim  - adjusts amount of auto-aiming
sv_friction  - friction
sv_stopspeed  - stopping speed
sv_gravity  - adjusts amount of gravity
sv_maxvelocity  - maximum speed allowed
sv_nostep <1|0> - enables/disables walking up steps
sv_idealpitchscale  - ??
sv_maxspeed  - ??
sv_accelerate  - adjusts rate of acceleration
Back to TOC

Quake is ©1996 id Software.

Disclaimer: This document is provided as is and may not be perfect. I do not guarentee the validity of any of the information in it. I will not be held liable or responsible for any damages caused from use or misuse of the information contained within this document.
*/

#endif
