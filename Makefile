#===========================================================================
#
# CGCam
# Copyright (C) 2013 Alex Reimann Cunha Lima
#
# This file is part of the CGCam
#
# CGCam is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# CGCam is distributed in the hope that it will
# be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with MDB Modular Database System. If not, see
# <http://www.gnu.org/licenses/>.
#
#===========================================================================
GL_INCLUDE = /usr/X11R6/include
GL_LIB = /usr/X11R6/lib
BDIR=bin
ODIR=obj
SDIR=src
CPP=g++
CPPFLAGS=-O2 -Wall -Wextra -g
CPPFLAGS+=-Wno-unused-parameter -Wno-unused-variable
CPPFLAGS+=-Wno-unused-but-set-variable
CPPFLAGS+=-Wno-unused-function
CPPFLAGS+=-Wno-strict-aliasing
CPPFLAGS+=-I$(GL_INCLUDE)
CPPFLAGS+=-I/home/alex/src/AntTweakBar/include
LDFLAGS=-g -L$(GL_LIB) -L/home/alex/src/AntTweakBar/lib \
        -lGL -lglfw -lGLEW -lm -lAntTweakBar
_MAINTEST=main.o
_MODULES=cl_connect.o cl_game.o cl_input.o cl_main.o cl_parse.o cl_player.o cl_render.o \
         assets.o cfun.o console.o cvar.o math.o net.o szb.o \
         conunix.o timer.o \
         re_main.o re_map.o re_md2.o re_particle.o re_scene.o re_shader.o re_t01.o re_text.o re_texture.o re_vidcon.o \
         sv_clientstate.o sv_connect.o sv_entity.o sv_main.o sv_player.o sv_snapshot.o \
         snd_main.o \
         sys_glfw.o
MAINTEST=$(patsubst %, $(ODIR)/%, $(_MAINTEST))
MODULES=$(patsubst %, $(ODIR)/%, $(_MODULES))
OBJS=$(MAINTEST) $(MODULES)
EXE=$(BDIR)/cgcam
TARBALL=cgcam.tar.gz

.PHONY: all clean run val tar net

all: $(EXE)

clean:
	@rm -f $(EXE) $(OBJS)

$(EXE): $(MAINTEST) $(MODULES)
	@$(CPP) $^ $(LDFLAGS) -o $@

$(ODIR)/%.o: $(SDIR)/%.cpp
	@$(CPP) -c $< $(CPPFLAGS) -o $@

$(ODIR)/%.o: $(SDIR)/client/%.cpp
	@$(CPP) -c $< $(CPPFLAGS) -o $@

$(ODIR)/%.o: $(SDIR)/common/%.cpp
	@$(CPP) -c $< $(CPPFLAGS) -o $@

$(ODIR)/%.o: $(SDIR)/platform/%.cpp
	@$(CPP) -c $< $(CPPFLAGS) -o $@

$(ODIR)/%.o: $(SDIR)/ref/%.cpp
	@$(CPP) -c $< $(CPPFLAGS) -o $@

$(ODIR)/%.o: $(SDIR)/server/%.cpp
	@$(CPP) -c $< $(CPPFLAGS) -o $@

$(ODIR)/%.o: $(SDIR)/sound/%.cpp
	@$(CPP) -c $< $(CPPFLAGS) -o $@

$(ODIR)/%.o: $(SDIR)/sys/%.cpp
	@$(CPP) -c $< $(CPPFLAGS) -o $@

$(EXE): | $(BDIR)

$(BDIR):
	@mkdir -p $(BDIR)

$(OBJS): | $(ODIR)

$(ODIR):
	@mkdir -p $(ODIR)

#.c.o:
#	@$(CC) -c $< $(CPPFLAGS) -o $@
#
run: $(EXE)
	@LD_LIBRARY_PATH=/home/alex/src/AntTweakBar/lib ./$(EXE)

net: $(EXE)
	@./$(EXE) -0 &
	@./$(EXE) -1 -c 127.0.0.1 &

conn2: $(EXE)
	@./$(EXE) -c 192.168.0.2

val: $(EXE)
	@valgrind --tool=memcheck ./$(EXE)

tar: $(TARBALL)

$(TARBALL):
	tar -cvzf $@ README COPYING src doc \
	Makefile
