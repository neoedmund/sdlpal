#!/bin/bash -e
gcc   -o libpalthegame.so -shared -fPIC \
 main2.c \
 battle.c ending.c fight.c font.c game.c getopt.c global.c input.c itemmenu.c magicmenu.c  map.c palcommon.c palette.c play.c res.c  rngplay.c scene.c script.c sound.c text.c ui.c uibattle.c uigame.c util.c video.c yj1.c \
 rixplay.cpp \
 `pkg-config --cflags --libs sdl adplug` -lm  -lstdc++ -DENABLE_LOG -DPAL_CLASSIC
#adplug/binfile.cpp adplug/binio.cpp adplug/emuopl.cpp adplug/fmopl.c adplug/fprovide.cpp adplug/player.cpp adplug/rix.cpp   \
# `pkg-config --cflags --libs   sdl adplug` -lm  -lstdc++
#  nosound.c \
#cp pal.run Data/
#cd Data/
# exec ./pal.run
