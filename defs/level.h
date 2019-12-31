typedef struct LEVEL{

	char *name;
	
	INT16 start_x;
	INT16 start_y;
	
	struct LEVEL *north;
	struct LEVEL *east;
	struct LEVEL *south;
	struct LEVEL *west;
	UINT8 objects_count;
	OBJECT *objects;
	unsigned char map[360];
} LEVEL;