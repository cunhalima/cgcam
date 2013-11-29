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
CFLAGS=-O2 -Wall -Wextra -g
CFLAGS+=-Wno-unused-parameter -Wno-unused-variable
CFLAGS+=-Wno-unused-but-set-variable
CFLAGS+=-Wno-unused-function
CFLAGS+=-Wno-strict-aliasing
CFLAGS+=-I$(GL_INCLUDE)
CFLAGS+=-I/home/alex/src/AntTweakBar/include
LDFLAGS=-g -L$(GL_LIB) -L/home/alex/src/AntTweakBar/lib \
        -lGL -lglfw -lGLEW -lm -lAntTweakBar
_MAINTEST=main.o
_MODULES=model.o shader.o video.o texture.o map.o client.o server.o unix.o net.o camera.o
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
	@$(CPP) -c $< $(CFLAGS) -o $@

$(EXE): | $(BDIR)

$(BDIR):
	@mkdir -p $(BDIR)

$(OBJS): | $(ODIR)

$(ODIR):
	@mkdir -p $(ODIR)

#.c.o:
#	@$(CC) -c $< $(CFLAGS) -o $@
#
run: $(EXE)
	@LD_LIBRARY_PATH=/home/alex/src/AntTweakBar/lib ./$(EXE)

net: $(EXE)
	@LD_LIBRARY_PATH=/home/alex/src/AntTweakBar/lib ./$(EXE) &
	@LD_LIBRARY_PATH=/home/alex/src/AntTweakBar/lib ./$(EXE) -c 127.0.0.1

val: $(EXE)
	@valgrind --tool=memcheck ./$(EXE)

tar: $(TARBALL)

$(TARBALL):
	tar -cvzf $@ README COPYING src doc \
	Makefile
