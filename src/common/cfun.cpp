#include <cstdlib>
#include <cstring>
//#include <string>
#include <vector>
#include <map>
#include <cassert>
#include "../defs.h"

struct cmp_str {
    bool operator()(char const * a, char const *b) {
        return std::strcmp(a, b) < 0;
    }
};

static std::map<const char *, cfun_t, cmp_str> fnames;
static std::vector<char *> argv;
static std::vector<char> runbuf;

static cfun_t cfun_get(const char *name) {
    std::map<const char *, cfun_t, cmp_str>::iterator it;

    it = fnames.find(name);
    if (it == fnames.end()) {
        return NULL;
    }
    return fnames[name];
}

bool cfun_register(cfun_t func, const char *name) {
    cfun_t fn = cfun_get(name);
    if (fn != NULL) {
        return false;
    }
    fnames[name] = func;
    return true;
}

int cfun_argc() {
    return int(argv.size());
}

int cfun_argi(int i) {
    const char *ss = cfun_args(i);
    if (ss == NULL) {
        return 0;
    }
    return atoi(ss);
}

const char *cfun_args(int i) {
    if (i >= (int)argv.size()) {
        return NULL;
    }
    return argv[i];
}

bool cfun_argdown() {
	if (argv.size() == 0) {
		return false;
	}
	return argv[0][0] == '+';
}

static bool my_isspace(int ch)
{
    return ch == ' ' || ch == '\t' || ch == '\r';
}

static bool my_iseol(int ch)
{
    return ch == '\n' || ch == '\0';
}

static bool my_issep(int ch)
{
    return my_isspace(ch) || my_iseol(ch);
}

static void con_runcommand() {
    int argc = int(argv.size());
	if (argc == 0) {
		return;
	}
	char firstChar = argv[0][0];
	if (firstChar == '-') {
		argv[0][0] = '+';
	}
    cfun_t fn = cfun_get(argv[0]);
    if (fn != NULL) {
		argv[0][0] = firstChar;
        fn();
        return;
    }
	argv[0][0] = firstChar;
    if (argc == 2) {
		cvar_t *cvar = cvar_get(argv[0]);
		if (cvar != NULL) {
			cvar_set(cvar, argv[1]);
			return;
        }
    }
    if (argc == 1) {
		cvar_t *cvar = cvar_get(argv[0]);
		if (cvar != NULL) {
            con_printf("\"%s\" is \"%s\"\n", argv[0], cvar->str);
			return;
        }
    }
    con_printf("Unknown command \"%s\"\n", argv[0]);
}

void con_addlines(const char *lines) {
	assert(lines != NULL);
	size_t bsize = runbuf.size();
	size_t ssize = strlen(lines);
	runbuf.resize(bsize + ssize);
	memcpy(&runbuf[bsize], lines, ssize);
}

void con_run() {
	size_t bsize = runbuf.size();
	if (bsize == 0) {
		return;
	}
	runbuf.resize(bsize + 1);
	runbuf[bsize] = '\0';
    char *st = my_strdup(&runbuf[0]);
	runbuf.clear();
    char *ss = st;
    for (;;) {
        while (my_isspace(*ss) || *ss == '\n') {
            ss++;
        }
        if (*ss == '\0') {
            break;
        }
        argv.clear();
        for (;;) {
            // now over word
            char quote = *ss;
            if (quote != '\"' && quote != '\'') {
                quote = 0;
            }
            if (quote) {
                ss++;
                argv.push_back(ss);
                while(*ss != quote && !my_iseol(*ss)) {
                    ss++;
                }
            } else {
                argv.push_back(ss);
                while (!my_issep(*ss)) {
                    ss++;
                }
            }
            // now just after word
            char sep = *ss;
            *ss = '\0';
            if (my_iseol(sep)) {
				if (sep != '\0') {
					ss++;
				}
                break;
            }
            ss++;
            // now over space or eol or new word
            while (my_isspace(*ss)) {
                ss++;
            }
            // now over eol or new word
            if (my_iseol(sep)) {
                break;
            }
            // now over word
        }
		con_runcommand();
    }
	delete[] st;
}

void con_type(const char *text) {
	assert(text != NULL);
	con_printf("$%s\n", text);
	con_addlines(text);
	//int len = strlen(text);
	//if (len > 0 && text[len - 1] != '\n') {
	con_addlines("\n");
	//}
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
