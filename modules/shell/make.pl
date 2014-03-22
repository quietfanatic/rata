#!/usr/bin/perl
use lib do {__FILE__ =~ /^(.*)[\/\\]/; ($1||'.').'/../../tool'};
use MakePl;

my @objects = qw<
    tmp/main.o
    tmp/console.o
    tmp/editing.o
    tmp/menus.o
    tmp/room_editor.o
    ../core/tmp/commands.o
    ../core/tmp/opengl.o
    ../core/tmp/window.o
    ../ent/tmp/bipeds.o
    ../ent/tmp/control.o
    ../ent/tmp/items.o
    ../ent/tmp/various.o
    ../geo/tmp/camera.o
    ../geo/tmp/rooms.o
    ../geo/tmp/tiles.o
    ../geo/tmp/vision.o
    ../hacc/tmp/common.o
    ../hacc/tmp/documents.o
    ../hacc/tmp/dynamicism.o
    ../hacc/tmp/files.o
    ../hacc/tmp/haccable.o
    ../hacc/tmp/haccable_standard.o
    ../hacc/tmp/strings.o
    ../hacc/tmp/tree.o
    ../hacc/tmp/types.o
    ../phys/tmp/ground.o
    ../phys/tmp/phys.o
    ../snd/tmp/audio.o
    ../vis/tmp/common.o
    ../vis/tmp/color.o
    ../vis/tmp/images.o
    ../vis/tmp/light.o
    ../vis/tmp/models.o
    ../vis/tmp/text.o
    ../vis/tmp/tiles.o
    ../util/tmp/debug.o
    ../util/tmp/geometry.o
    ../util/tmp/integration.o
    ../../lib/libBox2D.a
>;

my @libs =
    `uname -s` eq "Darwin\n"
        ? qw(-framework OpenGL -lGLEW -lglfw -lSOIL -lSDL2 -framework Cocoa -L/opt/local/lib)
        : qw(-lGL -lGLEW -lglfw -lSOIL -lSDL2);

include '../..';

mkdir 'tmp';

objects(qw(main console editing menus room_editor));

 # Here's the main program
ld_rule('../../rata', [@objects], [@libs]);

clean_rule(glob 'tmp/*');

make;
