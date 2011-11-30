
ROOM_BEGIN

ROOM_POS(40, -10)
ROOM_WIDTH(30)
ROOM_HEIGHT(20)
ROOM_TILES(
  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
  1,  0,  0, -4, -2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
  1,  0, -4, -3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
  1, -4, -3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
  1, -3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  5,  0,  0,  0,  2,  1,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  0,  0,  0,  0,  0,
  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  5,  0,  5,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  0,  0,  0,  0,  0,
  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  6,  0,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  0,  0,  0,  0,  0,
  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  6,  0,  6,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
  1,  0,  0,  0,  0,  5,  0,  0,  0,  0,  6,  0,  6,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
  1,  0,  0,  0,  0,  6,  0,  0,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  5,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
  1,  0,  0,  0,  0,  6,  0,  0,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
  1,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
  0,  0,  0,  0,  0,  5,  0,  0,  0,  5,  0,  0,  0,  5,  0,  0,  0,  0,  0,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
  0,  0,  0,  0,  0,  6,  0,  0,  0,  6,  0,  0,  0,  6,  0,  0,  0,  0,  0,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
  0,  0,  0,  0,  0,  6,  0,  0,  0,  6,  0,  0,  0,  6,  0,  0,  0,  0,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1
)

ROOM_N_NEIGHBORS(1)
ROOM_NEIGHBORS(
	room::test1
)
ROOM_N_WALLS(7)
ROOM_WALLS(
	{{10, 7.5}, 0, false},
	{{40.5, -2.0}, 9.5, false},
	{{52.5, 5}, 2.5, true},
	{{60, 2.5}, 0, false},
	{{60, -2.5}, 0, false},
	{{50, -2.5}, 0, false},
	{{40.5, -2.0}, 9.5, true},
)

ROOM_INIT(
	new Tilemap(HERE);
	new BGColor(HERE, pos, vec(width, height), 0x7f7f7fff);
	new Flyer(HERE, pos+vec(16.5, 4));
)

ROOM_END

