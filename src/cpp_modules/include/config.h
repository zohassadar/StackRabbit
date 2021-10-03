#ifndef CONFIG
#define CONFIG

#define LOGGING_ENABLED 0
#define PLAYOUT_LOGGING_ENABLED 0

#define USE_RANKS 0
#define CAN_TUCK 1

#define PLAY_SAFE 0

#define DEPTH_2_PRUNING_BREADTH 12
#define TUCK_SETUP_HOLE_PROPORTION 0.81f
#define SEQUENCE_LENGTH 20

#define NUM_PLAYOUTS_LONG 0
#define PLAYOUT_LENGTH_LONG 6
#define NUM_PLAYOUTS_SHORT 100
#define PLAYOUT_LENGTH_SHORT 5

#endif
