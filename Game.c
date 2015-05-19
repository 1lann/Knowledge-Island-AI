// Game.c

//----------#include-----------//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "Game.h"

//----------#defines----------//

#define DEFAULT_PLAYERS {0, 3, 3, 1, 1, 1}
#define NUM_DISCIPLINES 6
#define NUM_HEXS 19
#define NUM_VERTS 53
#define HEX_BUILD_PRINT 14

//-----------Structs-----------//

typedef struct _game * Game;
typedef struct _hex * hex;
typedef struct _vert * vert;
typedef struct _edge * edge;
typedef struct _player player;

typedef struct _hex {
	int hexID; // 1-19 starting with 1 at top left
	int hexDiscipline;


	hex hexUp;
	hex hexDown;
	hex hexUpLeft;
	hex hexUpRight;
	hex hexDownLeft;
	hex hexDownRight;

	vert vertUpLeft;
	vert vertUpRight;
	vert vertLeft;
	vert vertRight;
	vert vertDownLeft;
	vert vertDownRight;

	edge edgeUp;
	edge edgeDown;
	edge edgeUpLeft;
	edge edgeUpRight;
	edge edgeDownLeft;
	edge edgeDownRight;

} * hex;

typedef struct _vert {
	int hasUni;
	int hasGO8;

	int playerID;


	hex hexUp;
	hex hexDown;
	hex hexSide;

	vert vertUp;
	vert vertDown;
	vert vertSide;

	edge edgeUp;
	edge edgeDown;
	edge edgeSide;
} *vert;

typedef struct _edge {
	int hasARC;

	int playerID;

	hex hexUp;
	hex hexDown;

	vert vertUp;//If level up == right
	vert vertDown;

} *edge;

typedef struct _player{
	int playerID;

	int numARCs;
	int numPubs;
	int numIPs;
	int students[NUM_DISCIPLINES];

	int kpiPoints;

} player;

typedef struct _game {
	int currentTurn;

	int disciplines[NUM_REGIONS];
	int dice[NUM_REGIONS];

	vert vertArray[NUM_VERTS];
	hex hexArray[NUM_HEXS];
	edge edgeArray[72];

	vert entryPoint;

	player playerArray[3];

} *Game;

//------------Local Functions--------------//

static player newPlayer(int playerID);
static void buildHexMap(Game game);
static void buildVerts(Game game);

//Range Checked
static vert getVert(Game game, int index);
static void linkVertOffsets(Game game, int vertNum, int up, int down, int side);

//------------Main-------------//


//------------Helper Functions-------------//

static vert getVert(Game game, int index){
	vert vertPtr = NULL;

	if (index > NUM_VERTS || index < 0){
		//Out of range - returns NULL
	} else {
		vertPtr = game->vertArray[index];
	}
	return vertPtr;
}

static void linkVertOffsets(Game game, int vertNum, int up, int down, int side){
	getVert(game, vertNum)->vertUp = getVert(game, vertNum + up);
	getVert(game, vertNum)->vertDown = getVert(game, vertNum + down);
	getVert(game, vertNum)->vertSide = getVert(game, vertNum + side);
}

//------------Building Map Functions--------//

static void buildHexMap(Game game){
	//Initing all hexs
	//Storing in game->hexArray;
	//Setting disciplines
	int hexNum = 0;
	while (hexNum < NUM_HEXS) {
		hex tempHex = malloc(sizeof(struct _hex));
		tempHex->hexDiscipline = game->disciplines[hexNum];
		tempHex->hexID = hexNum + 1;
		game->hexArray[hexNum] = tempHex;
		hexNum++;
	}

	int hexLink = 0;
	while (hexLink < NUM_HEXS) {
		if (hexLink > -1 && hexLink < 3) {//First col
			if (hexLink == 0){
				//Vertical
				game->hexArray[hexLink]->hexUp = NULL;
				game->hexArray[hexLink]->hexDown = game->hexArray[hexLink + 1];
				//Right
				game->hexArray[hexLink]->hexUpRight = game->hexArray[hexLink + 3];
				game->hexArray[hexLink]->hexDownRight = game->hexArray[hexLink + 4];
				//Left
				game->hexArray[hexLink]->hexUpLeft = NULL;
				game->hexArray[hexLink]->hexDownLeft = NULL;
			}
			else if (hexLink == 1){
				//Vertical
				game->hexArray[hexLink]->hexUp = game->hexArray[hexLink - 1];
				game->hexArray[hexLink]->hexDown = game->hexArray[hexLink + 1];
				//Right
				game->hexArray[hexLink]->hexUpRight = game->hexArray[hexLink + 3];
				game->hexArray[hexLink]->hexDownRight = game->hexArray[hexLink + 4];
				//Left
				game->hexArray[hexLink]->hexUpLeft = NULL;
				game->hexArray[hexLink]->hexDownLeft = NULL;
			}
			else if (hexLink == 2){
				//Vertical
				game->hexArray[hexLink]->hexUp = game->hexArray[hexLink - 1];
				game->hexArray[hexLink]->hexDown = NULL;
				//Right
				game->hexArray[hexLink]->hexUpRight = game->hexArray[hexLink + 3];
				game->hexArray[hexLink]->hexDownRight = game->hexArray[hexLink + 4];
				//Left
				game->hexArray[hexLink]->hexUpLeft = NULL;
				game->hexArray[hexLink]->hexDownLeft = NULL;
			}
		}
		else if (hexLink > 2 && hexLink < 7){
			if (hexLink == 3){
				//Vertical
				game->hexArray[hexLink]->hexUp = NULL;
				game->hexArray[hexLink]->hexDown = game->hexArray[hexLink + 1];
				//Right
				game->hexArray[hexLink]->hexUpRight = game->hexArray[hexLink + 3];
				game->hexArray[hexLink]->hexDownRight = game->hexArray[hexLink + 4];
				//Left
				game->hexArray[hexLink]->hexUpLeft = NULL;
				game->hexArray[hexLink]->hexDownLeft = game->hexArray[hexLink - 4];
			}
			else if (hexLink == 6){
				//Vertical
				game->hexArray[hexLink]->hexUp = game->hexArray[hexLink - 1];
				game->hexArray[hexLink]->hexDown = NULL;
				//Right
				game->hexArray[hexLink]->hexUpRight = game->hexArray[hexLink + 3];
				game->hexArray[hexLink]->hexDownRight = NULL;
				//Left
				game->hexArray[hexLink]->hexUpLeft = game->hexArray[hexLink - 5];
				game->hexArray[hexLink]->hexDownLeft = NULL;
			}
			else{
				//Vertical
				game->hexArray[hexLink]->hexUp = game->hexArray[hexLink - 1];
				game->hexArray[hexLink]->hexDown = game->hexArray[hexLink + 1];
				//Right
				game->hexArray[hexLink]->hexUpRight = game->hexArray[hexLink + 4];
				game->hexArray[hexLink]->hexDownRight = game->hexArray[hexLink + 5];
				//Left
				game->hexArray[hexLink]->hexUpLeft = game->hexArray[hexLink - 4];
				game->hexArray[hexLink]->hexDownLeft = game->hexArray[hexLink - 3];
			}
		}
		else if (hexLink > 6 && hexLink < 12){
			if (hexLink == 7){
				//Vertical
				game->hexArray[hexLink]->hexUp = NULL;
				game->hexArray[hexLink]->hexDown = game->hexArray[hexLink + 1];
				//Right
				game->hexArray[hexLink]->hexUpRight = game->hexArray[hexLink + 3];
				game->hexArray[hexLink]->hexDownRight = game->hexArray[hexLink + 4];
				//Left
				game->hexArray[hexLink]->hexUpLeft = NULL;
				game->hexArray[hexLink]->hexDownLeft = game->hexArray[hexLink - 4];
			}
			else if (hexLink == 11){
				//Vertical
				game->hexArray[hexLink]->hexUp = game->hexArray[hexLink - 1];
				game->hexArray[hexLink]->hexDown = NULL;
				//Right
				game->hexArray[hexLink]->hexUpRight = game->hexArray[hexLink + 4];
				game->hexArray[hexLink]->hexDownRight = NULL;
				//Left
				game->hexArray[hexLink]->hexUpLeft = game->hexArray[hexLink - 5];
				game->hexArray[hexLink]->hexDownLeft = NULL;
			}
			else {
				//Vertical
				game->hexArray[hexLink]->hexUp = game->hexArray[hexLink - 1];
				game->hexArray[hexLink]->hexDown = game->hexArray[hexLink + 1];
				//Right
				game->hexArray[hexLink]->hexUpRight = game->hexArray[hexLink + 4];
				game->hexArray[hexLink]->hexDownRight = game->hexArray[hexLink + 5];
				//Left
				game->hexArray[hexLink]->hexUpLeft = game->hexArray[hexLink - 5];
				game->hexArray[hexLink]->hexDownLeft = game->hexArray[hexLink - 4];
			}
		}
		else if (hexLink > 11 && hexLink < 16){
			if (hexLink == 12){
				//Vertical
				game->hexArray[hexLink]->hexUp = NULL;
				game->hexArray[hexLink]->hexDown = game->hexArray[hexLink + 1];
				//Right
				game->hexArray[hexLink]->hexUpRight = NULL;
				game->hexArray[hexLink]->hexDownRight = game->hexArray[hexLink + 4];
				//Left
				game->hexArray[hexLink]->hexUpLeft = game->hexArray[hexLink - 5];
				game->hexArray[hexLink]->hexDownLeft = game->hexArray[hexLink - 4];
			}
			else if (hexLink == 15){
				//Vertical
				game->hexArray[hexLink]->hexUp = game->hexArray[hexLink - 1];
				game->hexArray[hexLink]->hexDown = NULL;
				//Right
				game->hexArray[hexLink]->hexUpRight = game->hexArray[hexLink + 4];
				game->hexArray[hexLink]->hexDownRight = NULL;
				//Left
				game->hexArray[hexLink]->hexUpLeft = game->hexArray[hexLink - 5];
				game->hexArray[hexLink]->hexDownLeft = game->hexArray[hexLink - 4];
			}
			else {
				//Vertical
				game->hexArray[hexLink]->hexUp = game->hexArray[hexLink - 1];
				game->hexArray[hexLink]->hexDown = game->hexArray[hexLink + 1];
				//Right
				game->hexArray[hexLink]->hexUpRight = game->hexArray[hexLink + 3];
				game->hexArray[hexLink]->hexDownRight = game->hexArray[hexLink + 4];
				//Left
				game->hexArray[hexLink]->hexUpLeft = game->hexArray[hexLink - 5];
				game->hexArray[hexLink]->hexDownLeft = game->hexArray[hexLink - 4];
			}
		}
		else {
			if (hexLink == 16){
				//Vertical
				game->hexArray[hexLink]->hexUp = NULL;
				game->hexArray[hexLink]->hexDown = game->hexArray[hexLink + 1];
				//Right
				game->hexArray[hexLink]->hexUpRight = NULL;
				game->hexArray[hexLink]->hexDownRight = NULL;
				//Left
				game->hexArray[hexLink]->hexUpLeft = game->hexArray[hexLink - 4];
				game->hexArray[hexLink]->hexDownLeft = game->hexArray[hexLink - 3];
			}
			else if (hexLink == 18){
				//Vertical
				game->hexArray[hexLink]->hexUp = game->hexArray[hexLink - 1];
				game->hexArray[hexLink]->hexDown = NULL;
				//Right
				game->hexArray[hexLink]->hexUpRight = NULL;
				game->hexArray[hexLink]->hexDownRight = NULL;
				//Left
				game->hexArray[hexLink]->hexUpLeft = game->hexArray[hexLink - 4];
				game->hexArray[hexLink]->hexDownLeft = game->hexArray[hexLink - 3];
			}
			else {
				//Vertical
				game->hexArray[hexLink]->hexUp = game->hexArray[hexLink - 1];
				game->hexArray[hexLink]->hexDown = game->hexArray[hexLink + 1];
				//Right
				game->hexArray[hexLink]->hexUpRight = NULL;
				game->hexArray[hexLink]->hexDownRight = NULL;
				//Left
				game->hexArray[hexLink]->hexUpLeft = game->hexArray[hexLink - 4];
				game->hexArray[hexLink]->hexDownLeft = game->hexArray[hexLink - 3];
			}
		}
		hexLink++;
	}

	printf("Hex ID: %d,", game->hexArray[HEX_BUILD_PRINT]->hexID);

	if (game->hexArray[HEX_BUILD_PRINT]->hexUpLeft != NULL){
		printf(" upL: %d,", game->hexArray[HEX_BUILD_PRINT]->hexUpLeft->hexID);
	}
	if (game->hexArray[HEX_BUILD_PRINT]->hexUpRight != NULL){
		printf(" upR: %d,", game->hexArray[HEX_BUILD_PRINT]->hexUpRight->hexID);
	}
	if (game->hexArray[HEX_BUILD_PRINT]->hexDownLeft != NULL){
		printf(" downL: %d,", game->hexArray[HEX_BUILD_PRINT]->hexDownLeft->hexID);
	}
	if (game->hexArray[HEX_BUILD_PRINT]->hexDownRight != NULL){
		printf(" downR: %d,", game->hexArray[HEX_BUILD_PRINT]->hexDownRight->hexID);
	}
	if (game->hexArray[HEX_BUILD_PRINT]->hexUp != NULL){
		printf(" up: %d,", game->hexArray[HEX_BUILD_PRINT]->hexUp->hexID);
	}
	if (game->hexArray[HEX_BUILD_PRINT]->hexDown != NULL){
		printf(" down: %d", game->hexArray[HEX_BUILD_PRINT]->hexDown->hexID);
	}

	printf("\n");
}

static void buildVerts(Game game){
	//Initing all hexs
	//Storing in game->hexArray;
	//Setting disciplines
	int vertNum = 0;
	while (vertNum < NUM_VERTS) {
		vert tempVert = malloc(sizeof(struct _vert));
		tempVert->hasUni = FALSE;
		tempVert->hasGO8 = FALSE;
		tempVert->playerID = NO_ONE;
		game->vertArray[vertNum] = tempVert;
		vertNum++;
	}

	// Commented out while it's unfinished
	//int hexLink = 0;
	//while (hexLink < NUM_HEXS) {
	//	int vertNum = 0;
	//	if (hexLink > -1 && hexLink < 3) {//First col
	//		vertNum = hexLink + 0;
	//		game->hexArray[hexLink]->vertLeft = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexSide = game->hexArray[vertNum];
	//		linkVertOffsets(game, vertNum, 3, 4, -50);
	//		/*getVert(game, vertNum)->vertUp = getVert(game, vertNum + 3);
	//		getVert(game, vertNum)->vertDown = getVert(game, vertNum + 4);
	//		//getVert(game, vertNum)->vertSide = getVert(game, vertNum + 4);*/

	//		vertNum = hexLink + 3;
	//		game->hexArray[hexLink]->vertUpLeft = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexDown = game->hexArray[vertNum];
	//		linkVertOffsets(game, vertNum, -4, -3, 4);
	//		/*getVert(game, vertNum)->vertUp = getVert(game, vertNum - 4);
	//		getVert(game, vertNum)->vertDown = getVert(game, vertNum - 3);
	//		getVert(game, vertNum)->vertSide = getVert(game, vertNum + 4);*/

	//		vertNum = hexLink + 7;
	//		game->hexArray[hexLink]->vertUpRight = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexDown = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, 4, 5, -4);
	//		/*getVert(game, vertNum)->vertUp = getVert(game, vertNum + 4);
	//		getVert(game, vertNum)->vertDown = getVert(game, vertNum + 5);
	//		getVert(game, vertNum)->vertSide = getVert(game, vertNum - 4);*/

	//		vertNum = hexLink + 12;
	//		game->hexArray[hexLink]->vertRight = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexSide = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, -5, -4, 5);
	//		/*getVert(game, vertNum)->vertUp = getVert(game, vertNum - 5);
	//		getVert(game, vertNum)->vertDown = getVert(game, vertNum - 4);
	//		getVert(game, vertNum)->vertSide = getVert(game, vertNum + 5);*/

	//		vertNum = hexLink + 8;
	//		game->hexArray[hexLink]->vertDownRight = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexUp = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, 4, 5, 5);
	//		/*getVert(game, vertNum)->vertUp = getVert(game, vertNum + 4);
	//		getVert(game, vertNum)->vertDown = getVert(game, vertNum + 5);
	//		getVert(game, vertNum)->vertSide = getVert(game, vertNum + 5);*/

	//		vertNum = hexLink + 4;
	//		game->hexArray[hexLink]->vertDownLeft = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexUp = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, -4, -3, 4);
	//		/*getVert(game, vertNum)->vertUp = getVert(game, vertNum - 4);
	//		getVert(game, vertNum)->vertDown = getVert(game, vertNum - 3);
	//		getVert(game, vertNum)->vertSide = getVert(game, vertNum + 4);*/
	//	}
	//	else if (hexLink > 2 && hexLink < 7){
	//		vertNum = hexLink + 4;
	//		game->hexArray[hexLink]->vertLeft = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexSide = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, 4, 5, -4);
	//		/*getVert(game, vertNum)->vertUp = getVert(game, vertNum + 4);
	//		getVert(game, vertNum)->vertDown = getVert(game, vertNum + 5);
	//		getVert(game, vertNum)->vertSide = getVert(game, vertNum - 4);*/

	//		vertNum = hexLink + 8;
	//		game->hexArray[hexLink]->vertUpLeft = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexDown = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, -5, -4, 5);
	//		/*getVert(game, vertNum)->vertUp = getVert(game, vertNum - 5);
	//		getVert(game, vertNum)->vertDown = getVert(game, vertNum - 4);
	//		getVert(game, vertNum)->vertSide = getVert(game, vertNum + 5);*/

	//		vertNum = hexLink + 13;
	//		game->hexArray[hexLink]->vertUpRight = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexDown = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, 5, 6, -5);
	//		/*getVert(game, vertNum)->vertUp = getVert(game, vertNum + 5);
	//		getVert(game, vertNum)->vertDown = getVert(game, vertNum + 6);
	//		getVert(game, vertNum)->vertSide = getVert(game, vertNum - 5);*/

	//		vertNum = hexLink + 19;
	//		game->hexArray[hexLink]->vertRight = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexSide = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, -6, -5, 6);
	//		/*getVert(game, vertNum)->vertUp = getVert(game, vertNum - 6);
	//		getVert(game, vertNum)->vertDown = getVert(game, vertNum - 5);
	//		getVert(game, vertNum)->vertSide = getVert(game, vertNum + 6);*/

	//		vertNum = hexLink + 14;
	//		game->hexArray[hexLink]->vertDownRight = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexUp = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, 5, -5, 6);
	//		/*getVert(game, vertNum)->vertUp = getVert(game, vertNum + 5);
	//		getVert(game, vertNum)->vertDown = getVert(game, vertNum - 5);
	//		getVert(game, vertNum)->vertSide = getVert(game, vertNum + 6);*/

	//		vertNum = hexLink + 9;
	//		game->hexArray[hexLink]->vertDownLeft = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexUp = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, -5, -4, 5);
	//		/*getVert(game, vertNum)->vertUp = getVert(game, vertNum - 5);
	//		getVert(game, vertNum)->vertDown = getVert(game, vertNum - 4);
	//		getVert(game, vertNum)->vertSide = getVert(game, vertNum + 5);*/
	//	}
	//	else if (hexLink > 6 && hexLink < 12){
	//		vertNum = hexLink + 9;
	//		game->hexArray[hexLink]->vertLeft = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexSide = game->hexArray[hexLink];
	//		getVert(game, vertNum)->hexUp = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, 5, 6, -5);
	//		/*getVert(game, vertNum)->vertUp = getVert(game, vertNum + 5);
	//		getVert(game, vertNum)->vertDown = getVert(game, vertNum + 6);
	//		getVert(game, vertNum)->vertSide = getVert(game, vertNum - 5);*/

	//		vertNum = hexLink + 14;
	//		game->hexArray[hexLink]->vertUpLeft = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexDown = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, -5, -4, 6);
	//		/*getVert(game, vertNum)->vertUp = getVert(game, vertNum - 5);
	//		getVert(game, vertNum)->vertDown = getVert(game, vertNum - 4);
	//		getVert(game, vertNum)->vertSide = getVert(game, vertNum + 6);*/

	//		vertNum = hexLink + 20;
	//		game->hexArray[hexLink]->vertUpRight = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexDown = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, 5, 6, -6);
	//		/*getVert(game, vertNum)->vertUp = getVert(game, vertNum + 5);
	//		getVert(game, vertNum)->vertDown = getVert(game, vertNum + 6);
	//		getVert(game, vertNum)->vertSide = getVert(game, vertNum - 6);*/

	//		vertNum = hexLink + 26;
	//		game->hexArray[hexLink]->vertRight = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexSide = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, -6, -5, 5);
	//		/*getVert(game, vertNum)->vertUp = getVert(game, vertNum - 6);
	//		getVert(game, vertNum)->vertDown = getVert(game, vertNum - 5);
	//		getVert(game, vertNum)->vertSide = getVert(game, vertNum + 5);*/

	//		vertNum = hexLink + 21;
	//		game->hexArray[hexLink]->vertDownRight = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexUp = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, 5, 6, -6);
	//		/*getVert(game, vertNum)->vertUp = getVert(game, vertNum + 5);
	//		getVert(game, vertNum)->vertDown = getVert(game, vertNum + 6);
	//		getVert(game, vertNum)->vertSide = getVert(game, vertNum - 6);*/

	//		vertNum = hexLink + 15;
	//		game->hexArray[hexLink]->vertDownLeft = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexUp = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, -5, -4, 6);
	//	}
	//	else if (hexLink > 11 && hexLink < 16){
	//		vertNum = hexLink + 16;
	//		game->hexArray[hexLink]->vertLeft = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexSide = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, 5, 6, -6);

	//		vertNum = hexLink + 21;
	//		game->hexArray[hexLink]->vertUpLeft = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexDown = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, -5, -4, 5);

	//		vertNum = hexLink + 26;
	//		game->hexArray[hexLink]->vertUpRight = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexDown = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, 4, 5, -4);

	//		vertNum = hexLink + 31;
	//		game->hexArray[hexLink]->vertRight = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexSide = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, -5, -4, 4);

	//		vertNum = hexLink + 27;
	//		game->hexArray[hexLink]->vertDownRight = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexUp = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, 4, 5, -4);

	//		vertNum = hexLink + 22;
	//		game->hexArray[hexLink]->vertDownLeft = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexUp = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, -5, -4, 5);
	//	}
	//	else {
	//		vertNum = hexLink + 16;
	//		game->hexArray[hexLink]->vertLeft = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexSide = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, 5, 6, -6);

	//		vertNum = hexLink + 21;
	//		game->hexArray[hexLink]->vertUpLeft = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexDown = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, -5, -4, 5);

	//		vertNum = hexLink + 26;
	//		game->hexArray[hexLink]->vertUpRight = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexDown = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, 4, 5, -4);

	//		vertNum = hexLink + 31;
	//		game->hexArray[hexLink]->vertRight = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexSide = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, -5, -4, 4);

	//		vertNum = hexLink + 27;
	//		game->hexArray[hexLink]->vertDownRight = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexUp = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, 4, 5, -4);

	//		vertNum = hexLink + 22;
	//		game->hexArray[hexLink]->vertDownLeft = getVert(game, vertNum);
	//		getVert(game, vertNum)->hexUp = game->hexArray[hexLink];
	//		linkVertOffsets(game, vertNum, -5, -4, 5);
	//	}
	//	hexLink++;
	//}
}

//------------Interface functons------------//


Game newGame(int discipline[], int dice[]){
	Game game = (Game)malloc(sizeof(struct _game));

	//Setting disciplines and dice vals
	int i = 0;
	while (i < NUM_REGIONS){
		game->disciplines[i] = discipline[i];
		game->dice[i] = dice[i];
		i++;
	}

	//Setting inital turn
	game->currentTurn = -1;


	//Initing players and assigning
	int playerI = 0;
	while (playerI < 3){
		game->playerArray[playerI] = newPlayer(playerI + 1);
		playerI++;
	}

	buildHexMap(game);
	buildVerts(game);

	return game;
}

// Completed
void disposeGame(Game g) {
	//Free every thing in the hex, vert and edge arrays
	int hexLoop = 0;
	while (hexLoop < NUM_HEXS){
		free(g->hexArray[hexLoop]);
		g->hexArray[hexLoop] = NULL;
		hexLoop++;
	}
	free(g);
}

// Incomplete
void makeAction(Game g, action a) {
	player currentPlayer = g->playerArray[g->currentTurn % NUM_UNIS];

	if (a.actionCode == PASS) {
	        if (isLegalAction(g, a)) {
        	    g->currentTurn++;
       		}
	}
	else if (a.actionCode == BUILD_CAMPUS) {
		// check if there's enough students
		if (isLegalAction(g, a)) {
            // Add a campus and take the cost from the user
            // TODO - add the campus
            currentPlayer.students[STUDENT_BPS]--;
            currentPlayer.students[STUDENT_BQN]--;
            currentPlayer.students[STUDENT_MJ]--;
            currentPlayer.students[STUDENT_MTV]--;

            // also add 10 KPI points
            currentPlayer.kpiPoints += 10;
		}
	}
	else if (a.actionCode == BUILD_GO8) {
		// check if there's a campus by the player
		// check if there's enough students
        if (g->playerArray[g->currentTurn].students[STUDENT_MJ] < 2 ||
            currentPlayer.students[STUDENT_MMONEY] < 3) {
            // None
        } else {
            // TODO - Remove the campus, add a GO8 campus

            // take the cost from the user
            currentPlayer.students[STUDENT_MJ] -= 2;
            currentPlayer.students[STUDENT_MMONEY] -= 3;

            // add 10 KPI points (20 for building G08, -10 for removing a campus)
            currentPlayer.kpiPoints += 10;
        }
	}
	else if (a.actionCode == OBTAIN_ARC) {
		// check if the location of the player is connected to his/her ARC
		// check if there's enough students
		// Add arc and take the cost from the user
		// Add 2 KPI points
	}
	else if (a.actionCode == START_SPINOFF) {
		// check if it's a legal action
		// 1/3 of the chance will be OBTAIN_IP_PATENT
		// if not, then OBTAIN_PUBLICATION
		if (isLegalAction(g, a)) {
            // Create a sort-of-almost-random number
            srand((unsigned int)time(NULL));//Simpler
            int r = rand() % 3;
            if (r == 0) {
                a.actionCode = OBTAIN_IP_PATENT;
                makeAction(g, a);
            }
            else {
                a.actionCode = OBTAIN_PUBLICATION;
                makeAction(g, a);
            }
		}
	}
	else if (a.actionCode == OBTAIN_PUBLICATION) {
		// increase the number of publications by 1
		currentPlayer.numPubs++;
	}
	else if (a.actionCode == OBTAIN_IP_PATENT) {
		// increase the number of IP patents by 1
		// increase the KPI points by 10
		currentPlayer.numIPs++;
		currentPlayer.kpiPoints += 10;
	}
	else if (a.actionCode == RETRAIN_STUDENTS) {
        if (isLegalAction(g, a)) {
            currentPlayer.students[a.disciplineFrom] -= getExchangeRate(g, currentPlayer.playerID, a.disciplineFrom, a.disciplineTo);
            currentPlayer.students[a.disciplineTo]++;
        }
	}
};

void throwDice(Game g, int diceScore){
	//Adv turn
	g->currentTurn++;
	//Give resources

	if (diceScore == 7) {
		int i = 0;
		int temp = 0;
		while (i < NUM_UNIS) {
			temp = g->playerArray[i].students[STUDENT_MTV];
			g->playerArray[i].students[STUDENT_MTV] = 0;
			temp += g->playerArray[i].students[STUDENT_MMONEY];
			g->playerArray[i].students[STUDENT_MMONEY] = 0;
			g->playerArray[i].students[STUDENT_THD] += temp;

			i++;
		}
	}
}

// Completed
int getDiscipline(Game g, int regionID){
	return g->disciplines[regionID];
}

// Completed
int getDiceValue(Game g, int regionID){
	return g->dice[regionID];
}

// Completed
int getMostARCs(Game g){
	int uniWithARCs = NO_ONE;
	int mostARCs = 0;

	char allEqual = 1;

	int i = 1;
	while (i <= NUM_UNIS){
		if (g->playerArray[i - 1].numARCs > mostARCs){
			uniWithARCs = i;
			mostARCs = g->playerArray[i - 1].numARCs;
		}

		if (g->playerArray[i - 1].numARCs != mostARCs && i != 1) {
			allEqual = 0;
		}
		i++;
	}

	if (allEqual == 1) {
		uniWithARCs = NO_ONE;
	}

	return uniWithARCs;
}

// Completed
int getMostPublications(Game g){
	int uniWithPubs = NO_ONE;
	int mostPubs = 0;

	int i = 1;
	while (i <= 3){
		if (g->playerArray[i - 1].numPubs > mostPubs){
			uniWithPubs = i;
			mostPubs = g->playerArray[i - 1].numPubs;
		}
		i++;
	}

	return uniWithPubs;
}

// Completed
int getTurnNumber(Game g){
	return g->currentTurn;
}

// Completed
int getWhoseTurn(Game g){
	int returnValue;
	if (g->currentTurn == -1) {
		returnValue = NO_ONE;
	}
	else {
		returnValue = g->playerArray[g->currentTurn % NUM_UNIS].playerID;
	}
	return returnValue;
}

// Incomplete
int getCampus(Game g, path pathToVertex){
	return 0; // Placeholder so it compiles
}

// Incomplete
int getARC(Game g, path pathToEdge){
	return 0; // Placeholder
}

// Still incomplete
int isLegalAction(Game g, action a){
    if (a.actionCode == PASS) {
        return TRUE;
    }
    else if (a.actionCode == BUILD_CAMPUS) {
        // check if there's enough students
        if (g->playerArray[g->currentTurn].students[STUDENT_BPS] < 1 ||
            g->playerArray[g->currentTurn % NUM_UNIS].students[STUDENT_BQN] < 1 ||
            g->playerArray[g->currentTurn % NUM_UNIS].students[STUDENT_MJ] < 1 ||
            g->playerArray[g->currentTurn % NUM_UNIS].students[STUDENT_MTV] < 1) {
            return FALSE;
        // TODO - check if the campus is connected to an ARC grant
        } else if (FALSE) { // Placeholder
            return FALSE;
        }
        else {
            return TRUE;
        }
    }
    else if (a.actionCode == BUILD_GO8) {
        // check if there's enough students
        if (g->playerArray[g->currentTurn].students[STUDENT_MJ] < 2 ||
            g->playerArray[g->currentTurn % NUM_UNIS].students[STUDENT_MMONEY] < 3) {
            return FALSE;
        // TODO - check if there's a campus by the player
        } else if (FALSE) {//Placeholder
            return FALSE; // Placeholder
        } else {
            return TRUE;
        }
    }
    else if (a.actionCode == OBTAIN_ARC) {
        // check if the location of the player is connected to his/her ARC
        // check if there's enough students
        // Add arc and take the cost from the user
        // Add 2 KPI points
    }
    else if (a.actionCode == START_SPINOFF) {
        if (g->playerArray[g->currentTurn % NUM_UNIS].students[STUDENT_MJ] < 1 ||
            g->playerArray[g->currentTurn % NUM_UNIS].students[STUDENT_MTV] < 1 ||
            g->playerArray[g->currentTurn % NUM_UNIS].students[STUDENT_MMONEY] < 1) {
            return FALSE;
        }
        else {
            return TRUE;
        }
    }
    else if (a.actionCode == OBTAIN_PUBLICATION) {
        // OBTAIN_PUBLICATION and OBTAIN_IP_PATENT are always illegal unless called by START_SPINOFF
        return FALSE;
    }
    else if (a.actionCode == OBTAIN_IP_PATENT) {
        return FALSE;
    }
    else if (a.actionCode == RETRAIN_STUDENTS) {
        // see if (disciplineFrom != STUDENT_THD)
        if (a.disciplineFrom == STUDENT_THD) {
            return FALSE;
        } else if (g->playerArray[g->currentTurn % NUM_UNIS].students[a.disciplineFrom] < getExchangeRate(g, g->playerArray[g->currentTurn % NUM_UNIS].playerID, a.disciplineFrom, a.disciplineTo)) {
            return FALSE;
        }
        else {
            return TRUE;
        }
    }
	return 0; // Placeholder
}

// Completed
int getKPIpoints(Game g, int player){
	return g->playerArray[player - 1].kpiPoints;
}

// Completed
int getARCs(Game g, int player) {
	return g->playerArray[player - 1].numARCs;
}

// Incomplete - should we make a new player variable called numGO8s?
int getGO8s(Game g, int player){
	return 0; // Placeholder
}

// Incomplete - should we make a new player variable called numCampuses?
int getCampuses(Game g, int player){
	return 0; // Placeholder
}

// Completed
int getIPs(Game g, int player){
	return g->playerArray[g->currentTurn % NUM_UNIS].numIPs;
}

// Completed
int getPublications(Game g, int player){
	return g->playerArray[g->currentTurn % NUM_UNIS].numPubs;
}

// Completed
int getStudents(Game g, int player, int discipline){
	return g->playerArray[player - 1].students[discipline];
}

// Incomplete
int getExchangeRate(Game g, int player,
	int disciplineFrom, int disciplineTo){
	return 0; // Placeholder
}

// Completed
static player newPlayer(int playerID){
	player playerNew;

	playerNew.playerID = playerID;

	int buffer[NUM_DISCIPLINES] = DEFAULT_PLAYERS;
	int i = 0;
	while (i < NUM_DISCIPLINES) {
		playerNew.students[i] = buffer[i];
		i++;
	}

	playerNew.numARCs = 0;
	playerNew.numPubs = 0;
	playerNew.numIPs = 0;
	playerNew.kpiPoints = 0;

	return playerNew;
}

// Incomplete code here
int getOriginVertexId() {
	// PLACEHOLDER. ALBERT SMITH: DO NOT COMPLAIN.
	return 42;
}

// int resolvePathA(Game g, path pathToVertex) {
// 	int origin = getOriginVertexId();

// 	int currentVertex = vert;

// }

vert resolvePathB(Game g, path pathToVertex) {
	int origin = getOriginVertexId();

	vert currentVertex = g->vertArray[origin];

	int i = 0;
	int pathLen = strlen(pathToVertex);

	while (i < pathLen) {
		char currentChar = pathToVertex[i];
		if (currentChar == "L") {
			// currentVertex = *(currentVertex.hexLeft);
		} else if (currentChar == "R") {
			// currentVertex = *(currentVertex.hexRight);
		} else if (currentChar == "B") {
			// currentVertex = *(currentVertex.hexBack);
		} else {
			printf("Invalid path supplied: %s\n", pathToVertex);
			exit(1);
		}

		i++;
	}

	i = 0;

	// while (i < 48) { // Replace with num of paths

	// }

	return currentVertex;
}
