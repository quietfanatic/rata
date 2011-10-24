

BEGIN_ROOM

BEGIN_ROOM_TILES
  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
  1,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  1,
  1,  0,  0,  0,  0,  0,  0,  5,  0,  5,  0,  5,  0,  5,  0,  5,  0,  0,  0,  1,
  1,  0,  0,  0,  0,  0,  0,  6,  0,  6,  0,  6,  0,  6,  0,  6,  0,  0,  0,  1,
  1,  0,  0,  0,  0,  0,  0,  6,  0,  6,  0,  6,  0,  6,  0,  6,  0,  0,  0,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1
END_ROOM_TILES

BEGIN_ROOM_OBJECTS
ROOM_OBJECT(obj::entrance, 1.500000, 1.000000, 0.000000, 0.000000, 0, 0, 0)
ROOM_OBJECT(obj::door, 8.500000, 1.000000, 0.500000, 0.000000, 0, room::test1, 0)
ROOM_OBJECT(obj::door, 10.500000, 1.000000, 0.500000, 0.000000, 0, room::test2, 0)
ROOM_OBJECT(obj::door, 12.500000, 1.000000, 0.500000, 0.000000, 0, room::test3, 0)
ROOM_OBJECT(obj::door, 14.500000, 1.000000, 0.500000, 0.000000, 0, room::test4, 0)
END_ROOM_OBJECTS

ROOM_DEF(20, 15, 5, 127, 127, 127, 255, -1, 1)

END_ROOM
