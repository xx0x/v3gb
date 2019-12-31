BOOLEAN object_mirrorline_locked = FALSE;


typedef enum {
	OBJECT_GRID,
	OBJECT_CHECKPOINT,
	OBJECT_MIRRORLINE,
	OBJECT_FLIPLINE
} OBJECT_TYPE;

typedef struct OBJECT{

	OBJECT_TYPE type;

	UINT8 x;
	UINT8 y;
	
	UINT8 width; // in tiles
	UINT8 height; // in tiles
	
	UINT8 min_x;
	UINT8 max_x;
	
	UINT8 min_y;
	UINT8 max_y;
	
	INT8 direction_x;
	INT8 direction_y;
	
} OBJECT;