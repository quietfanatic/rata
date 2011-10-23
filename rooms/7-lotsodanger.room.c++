

BEGIN_ROOM

BEGIN_ROOM_TILES
  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
  1,  0,  0, -4, -2,  0,  0,  0,  0,  0,  5,  0,  0,  0,  5,  0,  0,  0,  0,  5,  0,  0,  0,  5,  0,  0,  0,  0,  0,  1,
  1,  0, -4, -3,  0,  0,  0,  0,  0,  0,  6,  0,  0,  0,  6,  0,  0,  0,  0,  6,  0,  0,  0,  6,  0,  0,  0,  0,  0,  1,
  1, -4, -3,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  6,  0,  0,  0,  6,  0,  0,  0,  0,  0,  1,
  1, -3,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0, -4,  1,  0,  0,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1,
  1,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0, -4, -3,  0,  0,  0,  0,  0,  0,  5,  0,  0,  0,  0,  0,  0,  0,  1,
  1,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0, -4, -3,  0,  0,  0,  0,  0,  0,  0,  6,  0,  0,  0,  0,  0,  0,  0,  1,
  1,  1,  1,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  6,  0,  0,  0,  0,  0,  1,  1,  1,
  0,  0,  0,  5,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  2,  4,  1,
  0,  0,  0,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  3,  1,
  0,  0,  0,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  0,  0,
  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0, -4, -2,  0,  1,  1,  1,
  1,  0,  0,  5,  0,  0,  0,  0,  0,  0,  0,  0,  5,  0,  0,  0,  5,  0,  0,  0,  0,  0,  0, -4, -3,  0,  0,  0,  1,  1,
  1,  0,  0,  6,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0, -4, -3,  0,  0,  0,  0,  0,  1,
  1,  0,  0,  6,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  1, -3,  0,  0,  0,  0,  0,  0,  1,
  1,  0,  0,  6,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0, -4,  1,  0,  0,  0,  0,  0,  0,  0,  1,
  1,  1,  1,  1,  0,  0,  0,  0,  0,  5,  0,  0,  5,  0,  0,  0,  5,  0,  0, -4, -3,  1,  0,  0,  0,  0,  0,  0,  0,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1
END_ROOM_TILES

BEGIN_ROOM_OBJECTS
ROOM_OBJECT(obj::entrance, 1.500000, 9.000000, 0.000000, 0.000000, 0, 0, 0)
ROOM_OBJECT(obj::exit, 30.000000, 6.000000, 1.000000, 4.000000, 0, room::lotsodanger, 0)
ROOM_OBJECT(obj::exit, -1.000000, 8.000000, 1.000000, 4.000000, 0, room::test4, 1)
ROOM_OBJECT(obj::flyer, 2.500000, 14.500000, 0.000000, 0.000000, 0, 0, 0)
ROOM_OBJECT(obj::flyer, 11.500000, 14.500000, 0.000000, 0.000000, 0, 0, 0)
ROOM_OBJECT(obj::patroller, 2.500000, 2.000000, 0.000000, 0.000000, 0, 0, 0)
ROOM_OBJECT(obj::patroller, 28.500000, 1.000000, 0.000000, 0.000000, 0, 0, 0)
END_ROOM_OBJECTS

ROOM_DEF(30, 20, 7, 127, 127, 127, 255, 0)

END_ROOM

