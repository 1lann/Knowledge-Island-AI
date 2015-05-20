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
#define NUM_VERTS 54
#define NUM_EDGES 72
#define HEX_BUILD_PRINT 14

#define ORIGIN_VERT_ID 21
#define PREV_DIR_UP 1
#define PREV_DIR_DOWN 2
#define PREV_DIR_SIDE 3
#define LEFT 'L'
#define RIGHT 'R'
#define BACK 'B'
#define LEFT_I 0
#define RIGHT_I 1
#define BACK_I 2

#define VERT_A1_INDEX 21
#define VERT_A2_INDEX 32
#define VERT_B1_INDEX 0
#define VERT_B2_INDEX 50
#define VERT_C1_INDEX 6
#define VERT_C2_INDEX 47

#define VERT_CONV_MTV1_INDEX 11
#define VERT_CONV_MTV2_INDEX 16
#define VERT_CONV_MMONEY1_INDEX 33
#define VERT_CONV_MMONEY2_INDEX 38
#define VERT_CONV_BPS1_INDEX 10
#define VERT_CONV_BPS2_INDEX 15
#define VERT_CONV_MJ1_INDEX 42
#define VERT_CONV_MJ2_INDEX 46
#define VERT_CONV_BQN1_INDEX 52
#define VERT_CONV_BQN2_INDEX 49
#define VERT_CONV_ALL1_INDEX 0
#define VERT_CONV_ALL2_INDEX 3
#define VERT_CONV_ALL3_INDEX 1
#define VERT_CONV_ALL4_INDEX 5
#define VERT_CONV_ALL5_INDEX 26
#define VERT_CONV_ALL6_INDEX 32

//-----------Structs-----------//

typedef struct _game * Game;
typedef struct _hex * hex;
typedef struct _vert * vert;
typedef struct _edge * edge;
typedef struct _player * player;

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
//These will store the campus code or VACANT_VERTEX look at #defines in .h
    int contents;
    int hasGO8;
    int hasUni;

    int playerID;

    int vertIndex;

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
    int contents;

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
    int numGO8s;
    int numUnis;
    int students[NUM_DISCIPLINES];

    int kpiPoints;

} *player;

typedef struct _game {
    int currentTurn;

    int disciplines[NUM_REGIONS];
    int dice[NUM_REGIONS];

    int mostPubsUsed;
    player mostPubs;
    int mostARCsUsed;
    player mostARCs;

    vert vertArray[NUM_VERTS];
    hex hexArray[NUM_HEXS];
    edge edgeArray[72];

    vert entryPoint;

    vert startA1;
    vert startA2;
    vert startB1;
    vert startB2;
    vert startC1;
    vert startC2;

    player playerArray[3];

} *Game;

//------------Local Functions--------------//

static player newPlayer(int playerID);
static void buildHexMap(Game game);
static void buildVerts(Game game);
static void buildEdges(Game game);

//Range Checked
static vert getVert(Game game, int index);
static player getPlayer(Game game, int playerID);
static void linkVertOffsets(Game game, int vertNum, int up, int down, int side);

//Getting a vert or edge
static vert getVertAtPath(Game game, path pathToVert);
static edge getEdgeAtPath(Game game, path pathToEdge);
/*Finds the next vert knowing which vert is which direction. Updates prev direction*/
static vert getNextVert(Game game, vert verts[3], int tureDir[3], char letter, int *dir);

//------------Main-------------//


//------------Helper Functions-------------//

static vert getVert(Game game, int index) {
    vert vertPtr = NULL;

    if (index > NUM_VERTS || index < 0) {
        // Out of range - returns NULL
    } else {
        vertPtr = game->vertArray[index];
    }
    return vertPtr;
}

static player getPlayer(Game game, int playerID) {
    return game->playerArray[playerID - 1];
}

static void linkVertOffsets(Game game, int vertNum, int up, int down, int side) {
    getVert(game, vertNum)->vertUp = getVert(game, vertNum + up);
    getVert(game, vertNum)->vertDown = getVert(game, vertNum + down);
    getVert(game, vertNum)->vertSide = getVert(game, vertNum + side);
}

static player newPlayer(int playerID) {
    player playerNew = (player) malloc(sizeof(struct _player));

    playerNew->playerID = playerID;

    int buffer[NUM_DISCIPLINES] = DEFAULT_PLAYERS;
    int i = 0;
    while (i < NUM_DISCIPLINES) {
        playerNew->students[i] = buffer[i];
        i++;
    }

    playerNew->numARCs = 0;
    playerNew->numPubs = 0;
    playerNew->numIPs = 0;
	playerNew->numUnis = 2;
	playerNew->numGO8s = 0;
    playerNew->kpiPoints = 20;

    return playerNew;
}

static void addResourcesToPlayer(Game game, int playerID, int resources[NUM_DISCIPLINES]) {
    player playerToAdd = getPlayer(game, playerID);

    int discipline = 0;
    while (discipline < NUM_DISCIPLINES) {
        playerToAdd->students[discipline] += resources[discipline];
    }
}

static void addResourcesForHexAndVert(Game game, hex hexRolled, vert vertToCheck) {
    if (vertToCheck->hasUni == TRUE) {
        int resources[NUM_DISCIPLINES] = { 0, 0, 0, 0, 0, 0 };
        resources[hexRolled->hexDiscipline] = 1;
        addResourcesToPlayer(game, vertToCheck->playerID, resources);
    } else if (vertToCheck->hasGO8 == TRUE) {
        int resources[NUM_DISCIPLINES] = { 0, 0, 0, 0, 0, 0 };
        resources[hexRolled->hexDiscipline] = 2;
        addResourcesToPlayer(game, vertToCheck->playerID, resources);
    }
}

//------Getting vert/edge functions-------//

static vert getNextVert(Game game, vert verts[3], int tureDir[3], char letter, int *dir) {
    vert returnVert;
    if (letter == LEFT) {
        returnVert = verts[LEFT_I];
        *dir = tureDir[LEFT_I];
    } else if (letter == RIGHT) {
        returnVert = verts[RIGHT_I];
        *dir = tureDir[RIGHT_I];
    } else {
        returnVert = verts[BACK_I];
        *dir = tureDir[BACK_I];
    }
    return returnVert;
}

static vert getVertAtPath(Game game, path pathToVert) {
    vert prevVert = (vert)malloc(sizeof(struct _vert));
    vert toFree = prevVert;
    vert currVert;
    vert nextVert;
    int prevVertDir; // Last link taken

    currVert = getVert(game, ORIGIN_VERT_ID);
    prevVert->vertIndex = ORIGIN_VERT_ID - 1;
    prevVertDir = PREV_DIR_DOWN;

    //Loop over path
    char currentLetter;
    int pos = 0;

    currentLetter = pathToVert[pos];
    pos++;

    while (currentLetter != 0) {
        vert verts[3];
        int trueDir[3];

        if (prevVertDir == PREV_DIR_DOWN) {
            if (prevVert->vertIndex < currVert->vertIndex) {
                //Came from left, left = side, right = down, back == prev
                verts[LEFT_I] = currVert->vertSide;
                verts[RIGHT_I] = currVert->vertDown;

                trueDir[LEFT_I] = PREV_DIR_SIDE;
                trueDir[RIGHT_I] = PREV_DIR_DOWN;
                trueDir[BACK_I] = PREV_DIR_UP;
            } else {
                //Came from right; left = down, right = side, back == prev
                verts[LEFT_I] = currVert->vertDown;
                verts[RIGHT_I] = currVert->vertSide;

                trueDir[LEFT_I] = PREV_DIR_DOWN;
                trueDir[RIGHT_I] = PREV_DIR_SIDE;
                trueDir[BACK_I] = PREV_DIR_UP;
            }
        } else if (prevVertDir == PREV_DIR_SIDE) {
            if (prevVert->vertIndex < currVert->vertIndex) {
                //Came from left, left = up, right = down, back = prev
                verts[LEFT_I] = currVert->vertUp;
                verts[RIGHT_I] = currVert->vertDown;

                trueDir[LEFT_I] = PREV_DIR_UP;
                trueDir[RIGHT_I] = PREV_DIR_DOWN;
                trueDir[BACK_I] = PREV_DIR_SIDE;
            } else {
                //Came from right; left = down, right = up, back = prev
                verts[LEFT_I] = currVert->vertDown;
                verts[RIGHT_I] = currVert->vertSide;

                trueDir[LEFT_I] = PREV_DIR_DOWN;
                trueDir[RIGHT_I] = PREV_DIR_UP;
                trueDir[BACK_I] = PREV_DIR_SIDE;
            }
        } else if (prevVertDir == PREV_DIR_UP) {
            if (prevVert->vertIndex < currVert->vertIndex) {
                //Came from left, left = up, right = side, back = prev
                verts[LEFT_I] = currVert->vertUp;
                verts[RIGHT_I] = currVert->vertSide;

                trueDir[LEFT_I] = PREV_DIR_UP;
                trueDir[RIGHT_I] = PREV_DIR_SIDE;
                trueDir[BACK_I] = PREV_DIR_DOWN;
            } else {
                //Came from right; left = side, right = up, back = prev
                verts[LEFT_I] = currVert->vertSide;
                verts[RIGHT_I] = currVert->vertUp;

                trueDir[LEFT_I] = PREV_DIR_SIDE;
                trueDir[RIGHT_I] = PREV_DIR_UP;
                trueDir[BACK_I] = PREV_DIR_DOWN;
            }
        }

        verts[BACK_I] = prevVert;

        nextVert = getNextVert(game, verts, trueDir, currentLetter, &prevVertDir);
        /*printf("Curr vertID: %d, Next vertID: %d\n",
        currVert->vertIndex, nextVert->vertIndex);*/
        prevVert = currVert;
        currVert = nextVert;

        currentLetter = pathToVert[pos];
        pos++;

    }

    free(toFree);
    toFree = NULL;

    return currVert;
}

static edge getEdgeAtPath(Game game, path pathToEdge) {
    //------------NOTE SAME AS getVertAtPath() BUT FINDS THE EDGE AT THE END------------//
    vert prevVert = (vert)malloc(sizeof(struct _vert));
    vert toFree = prevVert;
    vert currVert;
    vert nextVert;
    int prevVertDir; // Last link taken

    currVert = getVert(game, ORIGIN_VERT_ID);
    prevVert->vertIndex = ORIGIN_VERT_ID - 1;
    prevVertDir = PREV_DIR_DOWN;

    //Loop over path
    char currentLetter;
    int pos = 0;

    currentLetter = pathToEdge[pos];
    pos++;

    while (currentLetter != 0) {
        vert verts[3];
        int trueDir[3];

        if (prevVertDir == PREV_DIR_DOWN) {
            if (prevVert->vertIndex < currVert->vertIndex) {
                //Came from left, left = side, right = down, back == prev
                verts[LEFT_I] = currVert->vertSide;
                verts[RIGHT_I] = currVert->vertDown;

                trueDir[LEFT_I] = PREV_DIR_SIDE;
                trueDir[RIGHT_I] = PREV_DIR_DOWN;
                trueDir[BACK_I] = PREV_DIR_UP;
            } else {
                //Came from right; left = down, right = side, back == prev
                verts[LEFT_I] = currVert->vertDown;
                verts[RIGHT_I] = currVert->vertSide;

                trueDir[LEFT_I] = PREV_DIR_DOWN;
                trueDir[RIGHT_I] = PREV_DIR_SIDE;
                trueDir[BACK_I] = PREV_DIR_UP;
            }
        } else if (prevVertDir == PREV_DIR_SIDE) {
            if (prevVert->vertIndex < currVert->vertIndex) {
                //Came from left, left = up, right = down, back = prev
                verts[LEFT_I] = currVert->vertUp;
                verts[RIGHT_I] = currVert->vertDown;

                trueDir[LEFT_I] = PREV_DIR_UP;
                trueDir[RIGHT_I] = PREV_DIR_DOWN;
                trueDir[BACK_I] = PREV_DIR_SIDE;
            } else {
                //Came from right; left = down, right = up, back = prev
                verts[LEFT_I] = currVert->vertDown;
                verts[RIGHT_I] = currVert->vertSide;

                trueDir[LEFT_I] = PREV_DIR_DOWN;
                trueDir[RIGHT_I] = PREV_DIR_UP;
                trueDir[BACK_I] = PREV_DIR_SIDE;
            }
        } else if (prevVertDir == PREV_DIR_UP) {
            if (prevVert->vertIndex < currVert->vertIndex) {
                //Came from left, left = up, right = side, back = prev
                verts[LEFT_I] = currVert->vertUp;
                verts[RIGHT_I] = currVert->vertSide;

                trueDir[LEFT_I] = PREV_DIR_UP;
                trueDir[RIGHT_I] = PREV_DIR_SIDE;
                trueDir[BACK_I] = PREV_DIR_DOWN;
            } else {
                //Came from right; left = side, right = up, back = prev
                verts[LEFT_I] = currVert->vertSide;
                verts[RIGHT_I] = currVert->vertUp;

                trueDir[LEFT_I] = PREV_DIR_SIDE;
                trueDir[RIGHT_I] = PREV_DIR_UP;
                trueDir[BACK_I] = PREV_DIR_DOWN;
            }
        }

        verts[BACK_I] = prevVert;

        nextVert = getNextVert(game, verts, trueDir, currentLetter, &prevVertDir);
        prevVert = currVert;
        currVert = nextVert;

        currentLetter = pathToEdge[pos];
        pos++;

    }

    free(toFree);
    toFree = NULL;

    //Last bit to get the edge
    edge edgeToReturn;
    if (prevVertDir == PREV_DIR_UP) {
        edgeToReturn = currVert->edgeUp;
    } else if (prevVertDir == PREV_DIR_SIDE) {
        edgeToReturn = currVert->edgeSide;
    } else {
        edgeToReturn = currVert->edgeDown;
    }
    return edgeToReturn;
}

//------------Building Map Functions--------//

static void buildHexMap(Game game) {
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
            if (hexLink == 0) {
                //Vertical
                game->hexArray[hexLink]->hexUp = NULL;
                game->hexArray[hexLink]->hexDown = game->hexArray[hexLink + 1];
                //Right
                game->hexArray[hexLink]->hexUpRight = game->hexArray[hexLink + 3];
                game->hexArray[hexLink]->hexDownRight = game->hexArray[hexLink + 4];
                //Left
                game->hexArray[hexLink]->hexUpLeft = NULL;
                game->hexArray[hexLink]->hexDownLeft = NULL;
            } else if (hexLink == 1) {
                //Vertical
                game->hexArray[hexLink]->hexUp = game->hexArray[hexLink - 1];
                game->hexArray[hexLink]->hexDown = game->hexArray[hexLink + 1];
                //Right
                game->hexArray[hexLink]->hexUpRight = game->hexArray[hexLink + 3];
                game->hexArray[hexLink]->hexDownRight = game->hexArray[hexLink + 4];
                //Left
                game->hexArray[hexLink]->hexUpLeft = NULL;
                game->hexArray[hexLink]->hexDownLeft = NULL;
            } else if (hexLink == 2) {
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
        } else if (hexLink > 2 && hexLink < 7) {
            if (hexLink == 3) {
                //Vertical
                game->hexArray[hexLink]->hexUp = NULL;
                game->hexArray[hexLink]->hexDown = game->hexArray[hexLink + 1];
                //Right
                game->hexArray[hexLink]->hexUpRight = game->hexArray[hexLink + 3];
                game->hexArray[hexLink]->hexDownRight = game->hexArray[hexLink + 4];
                //Left
                game->hexArray[hexLink]->hexUpLeft = NULL;
                game->hexArray[hexLink]->hexDownLeft = game->hexArray[hexLink - 4];
            } else if (hexLink == 6) {
                //Vertical
                game->hexArray[hexLink]->hexUp = game->hexArray[hexLink - 1];
                game->hexArray[hexLink]->hexDown = NULL;
                //Right
                game->hexArray[hexLink]->hexUpRight = game->hexArray[hexLink + 3];
                game->hexArray[hexLink]->hexDownRight = NULL;
                //Left
                game->hexArray[hexLink]->hexUpLeft = game->hexArray[hexLink - 5];
                game->hexArray[hexLink]->hexDownLeft = NULL;
            } else{
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
        } else if (hexLink > 6 && hexLink < 12) {
            if (hexLink == 7) {
                //Vertical
                game->hexArray[hexLink]->hexUp = NULL;
                game->hexArray[hexLink]->hexDown = game->hexArray[hexLink + 1];
                //Right
                game->hexArray[hexLink]->hexUpRight = game->hexArray[hexLink + 3];
                game->hexArray[hexLink]->hexDownRight = game->hexArray[hexLink + 4];
                //Left
                game->hexArray[hexLink]->hexUpLeft = NULL;
                game->hexArray[hexLink]->hexDownLeft = game->hexArray[hexLink - 4];
            } else if (hexLink == 11) {
                //Vertical
                game->hexArray[hexLink]->hexUp = game->hexArray[hexLink - 1];
                game->hexArray[hexLink]->hexDown = NULL;
                //Right
                game->hexArray[hexLink]->hexUpRight = game->hexArray[hexLink + 4];
                game->hexArray[hexLink]->hexDownRight = NULL;
                //Left
                game->hexArray[hexLink]->hexUpLeft = game->hexArray[hexLink - 5];
                game->hexArray[hexLink]->hexDownLeft = NULL;
            } else {
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
        } else if (hexLink > 11 && hexLink < 16) {
            if (hexLink == 12) {
                //Vertical
                game->hexArray[hexLink]->hexUp = NULL;
                game->hexArray[hexLink]->hexDown = game->hexArray[hexLink + 1];
                //Right
                game->hexArray[hexLink]->hexUpRight = NULL;
                game->hexArray[hexLink]->hexDownRight = game->hexArray[hexLink + 4];
                //Left
                game->hexArray[hexLink]->hexUpLeft = game->hexArray[hexLink - 5];
                game->hexArray[hexLink]->hexDownLeft = game->hexArray[hexLink - 4];
            } else if (hexLink == 15) {
                //Vertical
                game->hexArray[hexLink]->hexUp = game->hexArray[hexLink - 1];
                game->hexArray[hexLink]->hexDown = NULL;
                //Right
                game->hexArray[hexLink]->hexUpRight = game->hexArray[hexLink + 4];
                game->hexArray[hexLink]->hexDownRight = NULL;
                //Left
                game->hexArray[hexLink]->hexUpLeft = game->hexArray[hexLink - 5];
                game->hexArray[hexLink]->hexDownLeft = game->hexArray[hexLink - 4];
            } else {
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
        } else {
            if (hexLink == 16) {
                //Vertical
                game->hexArray[hexLink]->hexUp = NULL;
                game->hexArray[hexLink]->hexDown = game->hexArray[hexLink + 1];
                //Right
                game->hexArray[hexLink]->hexUpRight = NULL;
                game->hexArray[hexLink]->hexDownRight = NULL;
                //Left
                game->hexArray[hexLink]->hexUpLeft = game->hexArray[hexLink - 4];
                game->hexArray[hexLink]->hexDownLeft = game->hexArray[hexLink - 3];
            } else if (hexLink == 18) {
                //Vertical
                game->hexArray[hexLink]->hexUp = game->hexArray[hexLink - 1];
                game->hexArray[hexLink]->hexDown = NULL;
                //Right
                game->hexArray[hexLink]->hexUpRight = NULL;
                game->hexArray[hexLink]->hexDownRight = NULL;
                //Left
                game->hexArray[hexLink]->hexUpLeft = game->hexArray[hexLink - 4];
                game->hexArray[hexLink]->hexDownLeft = game->hexArray[hexLink - 3];
            } else {
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
   /* printf("Hex ID: %d,", game->hexArray[HEX_BUILD_PRINT]->hexID);

    if (game->hexArray[HEX_BUILD_PRINT]->hexUpLeft != NULL) {
        printf(" upL: %d,", game->hexArray[HEX_BUILD_PRINT]->hexUpLeft->hexID);
    }
    if (game->hexArray[HEX_BUILD_PRINT]->hexUpRight != NULL) {
        printf(" upR: %d,", game->hexArray[HEX_BUILD_PRINT]->hexUpRight->hexID);
    }
    if (game->hexArray[HEX_BUILD_PRINT]->hexDownLeft != NULL) {
        printf(" downL: %d,", game->hexArray[HEX_BUILD_PRINT]->hexDownLeft->hexID);
    }
    if (game->hexArray[HEX_BUILD_PRINT]->hexDownRight != NULL) {
        printf(" downR: %d,", game->hexArray[HEX_BUILD_PRINT]->hexDownRight->hexID);
    }
    if (game->hexArray[HEX_BUILD_PRINT]->hexUp != NULL) {
        printf(" up: %d,", game->hexArray[HEX_BUILD_PRINT]->hexUp->hexID);
    }
    if (game->hexArray[HEX_BUILD_PRINT]->hexDown != NULL) {
        printf(" down: %d", game->hexArray[HEX_BUILD_PRINT]->hexDown->hexID);
    }

    printf("\n");*/
}

static void buildVerts(Game game) {
    //Initing all hexs
    //Storing in game->hexArray;
    //Setting disciplines
    int vertNum = 0;
    while (vertNum < NUM_VERTS) {
        vert tempVert = malloc(sizeof(struct _vert));
        tempVert->contents = VACANT_VERTEX;
        tempVert->playerID = NO_ONE;
        tempVert->vertIndex = vertNum;
        game->vertArray[vertNum] = tempVert;
        vertNum++;
    }

    // Commented out while it's unfinished
    int hexLink = 0;
    while (hexLink < NUM_HEXS) {
        int vertNum = 0;
        if (hexLink > -1 && hexLink < 3) {//First col
            vertNum = hexLink + 0;
            game->hexArray[hexLink]->vertLeft = getVert(game, vertNum);
            getVert(game, vertNum)->hexSide = game->hexArray[vertNum];
            linkVertOffsets(game, vertNum, 3, 4, -50);
            getVert(game, vertNum)->vertUp = getVert(game, vertNum + 3);
            getVert(game, vertNum)->vertDown = getVert(game, vertNum + 4);
            //getVert(game, vertNum)->vertSide = getVert(game, vertNum + 4);

            vertNum = hexLink + 3;
            game->hexArray[hexLink]->vertUpLeft = getVert(game, vertNum);
            getVert(game, vertNum)->hexDown = game->hexArray[vertNum];
            linkVertOffsets(game, vertNum, -4, -3, 4);
            /*getVert(game, vertNum)->vertUp = getVert(game, vertNum - 4);
            getVert(game, vertNum)->vertDown = getVert(game, vertNum - 3);
            getVert(game, vertNum)->vertSide = getVert(game, vertNum + 4);*/

            vertNum = hexLink + 7;
            game->hexArray[hexLink]->vertUpRight = getVert(game, vertNum);
            getVert(game, vertNum)->hexDown = game->hexArray[hexLink];
            linkVertOffsets(game, vertNum, 4, 5, -4);
            /*getVert(game, vertNum)->vertUp = getVert(game, vertNum + 4);
            getVert(game, vertNum)->vertDown = getVert(game, vertNum + 5);
            getVert(game, vertNum)->vertSide = getVert(game, vertNum - 4);*/

            vertNum = hexLink + 12;
            game->hexArray[hexLink]->vertRight = getVert(game, vertNum);
            getVert(game, vertNum)->hexSide = game->hexArray[hexLink];
            linkVertOffsets(game, vertNum, -5, -4, 5);
            /*getVert(game, vertNum)->vertUp = getVert(game, vertNum - 5);
            getVert(game, vertNum)->vertDown = getVert(game, vertNum - 4);
            getVert(game, vertNum)->vertSide = getVert(game, vertNum + 5);*/

            vertNum = hexLink + 8;
            game->hexArray[hexLink]->vertDownRight = getVert(game, vertNum);
            getVert(game, vertNum)->hexUp = game->hexArray[hexLink];
            linkVertOffsets(game, vertNum, 4, 5, 5);
            /*getVert(game, vertNum)->vertUp = getVert(game, vertNum + 4);
            getVert(game, vertNum)->vertDown = getVert(game, vertNum + 5);
            getVert(game, vertNum)->vertSide = getVert(game, vertNum + 5);*/

            vertNum = hexLink + 4;
            game->hexArray[hexLink]->vertDownLeft = getVert(game, vertNum);
            getVert(game, vertNum)->hexUp = game->hexArray[hexLink];
            linkVertOffsets(game, vertNum, -4, -3, 4);
            /*getVert(game, vertNum)->vertUp = getVert(game, vertNum - 4);
            getVert(game, vertNum)->vertDown = getVert(game, vertNum - 3);
            getVert(game, vertNum)->vertSide = getVert(game, vertNum + 4);*/
        } else if (hexLink > 2 && hexLink < 7) {
            vertNum = hexLink + 4;
            game->hexArray[hexLink]->vertLeft = getVert(game, vertNum);
            getVert(game, vertNum)->hexSide = game->hexArray[hexLink];
            linkVertOffsets(game, vertNum, 4, 5, -4);
            /*getVert(game, vertNum)->vertUp = getVert(game, vertNum + 4);
            getVert(game, vertNum)->vertDown = getVert(game, vertNum + 5);
            getVert(game, vertNum)->vertSide = getVert(game, vertNum - 4);*/

            vertNum = hexLink + 8;
            game->hexArray[hexLink]->vertUpLeft = getVert(game, vertNum);
            getVert(game, vertNum)->hexDown = game->hexArray[hexLink];
            linkVertOffsets(game, vertNum, -5, -4, 5);
            /*getVert(game, vertNum)->vertUp = getVert(game, vertNum - 5);
            getVert(game, vertNum)->vertDown = getVert(game, vertNum - 4);
            getVert(game, vertNum)->vertSide = getVert(game, vertNum + 5);*/

            vertNum = hexLink + 13;
            game->hexArray[hexLink]->vertUpRight = getVert(game, vertNum);
            getVert(game, vertNum)->hexDown = game->hexArray[hexLink];
            linkVertOffsets(game, vertNum, 5, 6, -5);
            /*getVert(game, vertNum)->vertUp = getVert(game, vertNum + 5);
            getVert(game, vertNum)->vertDown = getVert(game, vertNum + 6);
            getVert(game, vertNum)->vertSide = getVert(game, vertNum - 5);*/

            vertNum = hexLink + 19;
            game->hexArray[hexLink]->vertRight = getVert(game, vertNum);
            getVert(game, vertNum)->hexSide = game->hexArray[hexLink];
            linkVertOffsets(game, vertNum, -6, -5, 6);
            /*getVert(game, vertNum)->vertUp = getVert(game, vertNum - 6);
            getVert(game, vertNum)->vertDown = getVert(game, vertNum - 5);
            getVert(game, vertNum)->vertSide = getVert(game, vertNum + 6);*/

            vertNum = hexLink + 14;
            game->hexArray[hexLink]->vertDownRight = getVert(game, vertNum);
            getVert(game, vertNum)->hexUp = game->hexArray[hexLink];
            linkVertOffsets(game, vertNum, 5, -5, 6);
            /*getVert(game, vertNum)->vertUp = getVert(game, vertNum + 5);
            getVert(game, vertNum)->vertDown = getVert(game, vertNum - 5);
            getVert(game, vertNum)->vertSide = getVert(game, vertNum + 6);*/

            vertNum = hexLink + 9;
            game->hexArray[hexLink]->vertDownLeft = getVert(game, vertNum);
            getVert(game, vertNum)->hexUp = game->hexArray[hexLink];
            linkVertOffsets(game, vertNum, -5, -4, 5);
            /*getVert(game, vertNum)->vertUp = getVert(game, vertNum - 5);
            getVert(game, vertNum)->vertDown = getVert(game, vertNum - 4);
            getVert(game, vertNum)->vertSide = getVert(game, vertNum + 5);*/
        } else if (hexLink > 6 && hexLink < 12) {
            vertNum = hexLink + 9;
            game->hexArray[hexLink]->vertLeft = getVert(game, vertNum);
            getVert(game, vertNum)->hexSide = game->hexArray[hexLink];
            linkVertOffsets(game, vertNum, 5, 6, -5);
            /*getVert(game, vertNum)->vertUp = getVert(game, vertNum + 5);
            getVert(game, vertNum)->vertDown = getVert(game, vertNum + 6);
            getVert(game, vertNum)->vertSide = getVert(game, vertNum - 5);*/

            vertNum = hexLink + 14;
            game->hexArray[hexLink]->vertUpLeft = getVert(game, vertNum);
            getVert(game, vertNum)->hexDown = game->hexArray[hexLink];
            linkVertOffsets(game, vertNum, -6, -5, 6);
            /*getVert(game, vertNum)->vertUp = getVert(game, vertNum - 5);
            getVert(game, vertNum)->vertDown = getVert(game, vertNum - 4);
            getVert(game, vertNum)->vertSide = getVert(game, vertNum + 6);*/

            vertNum = hexLink + 20;
            game->hexArray[hexLink]->vertUpRight = getVert(game, vertNum);
            getVert(game, vertNum)->hexDown = game->hexArray[hexLink];
            linkVertOffsets(game, vertNum, 5, 6, -6);
            /*getVert(game, vertNum)->vertUp = getVert(game, vertNum + 5);
            getVert(game, vertNum)->vertDown = getVert(game, vertNum + 6);
            getVert(game, vertNum)->vertSide = getVert(game, vertNum - 6);*/

            vertNum = hexLink + 26;
            game->hexArray[hexLink]->vertRight = getVert(game, vertNum);
            getVert(game, vertNum)->hexSide = game->hexArray[hexLink];
            linkVertOffsets(game, vertNum, -6, -5, 5);
            /*getVert(game, vertNum)->vertUp = getVert(game, vertNum - 6);
            getVert(game, vertNum)->vertDown = getVert(game, vertNum - 5);
            getVert(game, vertNum)->vertSide = getVert(game, vertNum + 5);*/

            vertNum = hexLink + 21;
            game->hexArray[hexLink]->vertDownRight = getVert(game, vertNum);
            getVert(game, vertNum)->hexUp = game->hexArray[hexLink];
            linkVertOffsets(game, vertNum, 5, 6, -6);
            /*getVert(game, vertNum)->vertUp = getVert(game, vertNum + 5);
            getVert(game, vertNum)->vertDown = getVert(game, vertNum + 6);
            getVert(game, vertNum)->vertSide = getVert(game, vertNum - 6);*/

            vertNum = hexLink + 15;
            game->hexArray[hexLink]->vertDownLeft = getVert(game, vertNum);
            getVert(game, vertNum)->hexUp = game->hexArray[hexLink];
            linkVertOffsets(game, vertNum, -6, -5, 6);
        } else if (hexLink > 11 && hexLink < 16) {
            vertNum = hexLink + 16;
            game->hexArray[hexLink]->vertLeft = getVert(game, vertNum);
            getVert(game, vertNum)->hexSide = game->hexArray[hexLink];
            linkVertOffsets(game, vertNum, 5, 6, -6);

            vertNum = hexLink + 21;
            game->hexArray[hexLink]->vertUpLeft = getVert(game, vertNum);
            getVert(game, vertNum)->hexDown = game->hexArray[hexLink];
			linkVertOffsets(game, vertNum, -6, -5, 5);

            vertNum = hexLink + 26;
            game->hexArray[hexLink]->vertUpRight = getVert(game, vertNum);
            getVert(game, vertNum)->hexDown = game->hexArray[hexLink];
            linkVertOffsets(game, vertNum, 4, 5, -5);

            vertNum = hexLink + 31;
            game->hexArray[hexLink]->vertRight = getVert(game, vertNum);
            getVert(game, vertNum)->hexSide = game->hexArray[hexLink];
            linkVertOffsets(game, vertNum, -5, -4, 4);

            vertNum = hexLink + 27;
            game->hexArray[hexLink]->vertDownRight = getVert(game, vertNum);
            getVert(game, vertNum)->hexUp = game->hexArray[hexLink];
            linkVertOffsets(game, vertNum, 4, 5, -4);

            vertNum = hexLink + 22;
            game->hexArray[hexLink]->vertDownLeft = getVert(game, vertNum);
            getVert(game, vertNum)->hexUp = game->hexArray[hexLink];
            linkVertOffsets(game, vertNum, -6, -5, 5);
        } else {
            vertNum = hexLink + 23;
            game->hexArray[hexLink]->vertLeft = getVert(game, vertNum);
            getVert(game, vertNum)->hexSide = game->hexArray[hexLink];
            linkVertOffsets(game, vertNum, 4, 5, -5);

            vertNum = hexLink + 27;
            game->hexArray[hexLink]->vertUpLeft = getVert(game, vertNum);
            getVert(game, vertNum)->hexDown = game->hexArray[hexLink];
            linkVertOffsets(game, vertNum, -5, -4, 4);

            vertNum = hexLink + 31;
            game->hexArray[hexLink]->vertUpRight = getVert(game, vertNum);
            getVert(game, vertNum)->hexDown = game->hexArray[hexLink];
            linkVertOffsets(game, vertNum, 3, 4, -4);

            vertNum = hexLink + 35;
            game->hexArray[hexLink]->vertRight = getVert(game, vertNum);
            getVert(game, vertNum)->hexSide = game->hexArray[hexLink];
            linkVertOffsets(game, vertNum, -4, -3, 50);

            vertNum = hexLink + 32;
            game->hexArray[hexLink]->vertDownRight = getVert(game, vertNum);
            getVert(game, vertNum)->hexUp = game->hexArray[hexLink];
            linkVertOffsets(game, vertNum, 3, 4, -4);

            vertNum = hexLink + 28;
            game->hexArray[hexLink]->vertDownLeft = getVert(game, vertNum);
            getVert(game, vertNum)->hexUp = game->hexArray[hexLink];
            linkVertOffsets(game, vertNum, -5, -4, 4);
        }
        hexLink++;
    }
}

/*
static void buildEdges(Game game){
	int edgeNum = 0;
	while (edgeNum < NUM_EDGES) {
		edge tempEdge = malloc(sizeof(struct _edge));
		tempEdge->contents = VACANT_ARC;
		game->edgeArray[edgeNum] = tempEdge;
		edgeNum++;
	}

	int hexLink = 0;
	while (hexLink < NUM_HEXS) {
		hex currHex =
		if (hexLink > -1 && hexLink < 3) {//First col

		}
		else if (hexLink > 2 && hexLink < 7) {

		}
		else if (hexLink > 6 && hexLink < 12) {

		}
		else if (hexLink > 11 && hexLink < 16) {

		}
		else {

		}
		hexLink++;
	}

}*/

//------------Interface functons------------//

// Incomplete
Game newGame(int discipline[], int dice[]) {
    Game game = (Game)malloc(sizeof(struct _game));

    //Setting disciplines and dice vals
    int i = 0;
    while (i < NUM_REGIONS) {
        game->disciplines[i] = discipline[i];
        game->dice[i] = dice[i];
        game->mostPubs = NULL;
        game->mostPubsUsed = FALSE;
        i++;
    }

    //Setting inital turn
    game->currentTurn = -1;


    //Initing players and assigning
    int playerI = 0;
    while (playerI < 3) {
        game->playerArray[playerI] = newPlayer(playerI + 1);
        playerI++;
    }

    buildHexMap(game);
    buildVerts(game);

    game->startA1 = getVert(game, VERT_A1_INDEX);
    game->startA2 = getVert(game, VERT_A2_INDEX);
    game->startB1 = getVert(game, VERT_B1_INDEX);
    game->startB2 = getVert(game, VERT_B2_INDEX);
    game->startC1 = getVert(game, VERT_C1_INDEX);
    game->startC2 = getVert(game, VERT_C2_INDEX);

    game->startA1->contents = CAMPUS_A;
    game->startA1->hasUni = TRUE;
    game->startA1->playerID = CAMPUS_A - 1;
    game->startA2->contents = CAMPUS_A;
    game->startA2->hasUni = TRUE;
    game->startA2->playerID = CAMPUS_A - 1;

    game->startB1->contents = CAMPUS_B;
    game->startB1->hasUni = TRUE;
    game->startB1->playerID = CAMPUS_B - 1;
    game->startB2->contents = CAMPUS_B;
    game->startB2->hasUni = TRUE;
    game->startB2->playerID = CAMPUS_B - 1;

    game->startC1->contents = CAMPUS_C;
    game->startC1->hasUni = TRUE;
    game->startC1->playerID = CAMPUS_C - 1;
    game->startC2->contents = CAMPUS_C;
    game->startC2->hasUni = TRUE;
    game->startC2->playerID = CAMPUS_C - 1;

	printf("numIPs : %d\n", getPlayer(game, UNI_A)->numIPs);
	printf("numIPs : %d\n", getPlayer(game, UNI_B)->numIPs);
	printf("numIPs : %d\n", getPlayer(game, UNI_C)->numIPs);

    //Set in newPlayer()

    return game;
}

// Incomplete - ish
void disposeGame(Game g) {
    //Free every thing in the hex, vert and edge arrays
    int playerLoop = 0;
    while (playerLoop < NUM_UNIS) {
        free(g->hexArray[playerLoop]);
        g->playerArray[playerLoop] = NULL;
        playerLoop++;
    }

    int hexLoop = 0;
    while (hexLoop < NUM_HEXS) {
        free(g->hexArray[hexLoop]);
        g->hexArray[hexLoop] = NULL;
        hexLoop++;
    }

    int vertLoop = 0;
    while (vertLoop < NUM_VERTS) {
        free(g->vertArray[vertLoop]);
        g->vertArray[vertLoop] = NULL;
        hexLoop++;
    }

    free(g);

//Dispose of edges

}

//Complete
void makeAction(Game g, action a) {

    player currentPlayer = g->playerArray[g->currentTurn % NUM_UNIS];

    if (a.actionCode == PASS) {
        // Do nothing - runGame.c via throwDice() increments the turn number
    } else if (a.actionCode == BUILD_CAMPUS) {
        // Take the cost from the user
        currentPlayer->students[STUDENT_BPS]--;
        currentPlayer->students[STUDENT_BQN]--;
        currentPlayer->students[STUDENT_MJ]--;
        currentPlayer->students[STUDENT_MTV]--;

        // Add a campus
        vert campus = getVertAtPath(g, a.destination);
        currentPlayer->numUnis++;
        campus->playerID = currentPlayer->playerID;

        campus->hasUni = TRUE;
        campus->hasGO8 = FALSE;
        // If there's a better way to do this, let me know.
        campus->contents = (g->currentTurn % NUM_UNIS) + 1;

        // also add 10 KPI points
        currentPlayer->kpiPoints += 10;
    } else if (a.actionCode == BUILD_GO8) {
        // remove the campus, add a GO8 campus
        vert go8 = getVertAtPath(g, a.destination);
        // I also want a better way to do this.
        go8->contents += NUM_UNIS;
        go8->hasUni = FALSE;
        go8->hasGO8 = TRUE;
        currentPlayer->numUnis--;
        currentPlayer->numGO8s++;
        // take the cost from the user
        currentPlayer->students[STUDENT_MJ] -= 2;
        currentPlayer->students[STUDENT_MMONEY] -= 3;

        // add 10 KPI points (20 for building G08, -10 for removing a campus)
        currentPlayer->kpiPoints += 10;
    } else if (a.actionCode == OBTAIN_ARC) {

        player mostARCs = g->mostARCs;

        // Add arc
        edge arc = getEdgeAtPath(g, a.destination);
        arc->contents = currentPlayer->playerID;
        currentPlayer->numARCs++;

        // Take the cost from the user
        currentPlayer->students[STUDENT_BQN]--;
        currentPlayer->students[STUDENT_BPS]--;

        // Add 2 KPI points
        currentPlayer->kpiPoints += 2;


        if (g->mostARCs->playerID == currentPlayer->playerID) {
            //Player now has same Pubs as other player OR more
            if (currentPlayer->numPubs > mostARCs->numPubs) {
                //Player actually has more
                currentPlayer->kpiPoints += 10;
                if (g->mostARCsUsed == TRUE) {
                    mostARCs->kpiPoints -= 10;
                } else {
                    g->mostARCsUsed = TRUE;
                }
                g->mostARCs = currentPlayer;
            }
        }
    } else if (a.actionCode == OBTAIN_PUBLICATION) {

        player mostPubs = g->mostPubs;

        // increase the number of publications by 1
        currentPlayer->numPubs++;

        if (g->mostARCs->playerID == currentPlayer->playerID) {
            //Player now has same Pubs as other player OR more
            if (currentPlayer->numPubs > mostPubs->numPubs) {
                //Player actually has more
                currentPlayer->kpiPoints += 10;
                if (g->mostPubsUsed == TRUE) {
                    mostPubs->kpiPoints -= 10;
                } else {
                    g->mostPubsUsed = TRUE;
                }
                g->mostPubs = currentPlayer;
            }
        }
    } else if (a.actionCode == OBTAIN_IP_PATENT) {
        // increase the number of IP patents by 1
        // increase the KPI points by 10
        currentPlayer->numIPs++;
        currentPlayer->kpiPoints += 10;
    } else if (a.actionCode == RETRAIN_STUDENTS) {
        currentPlayer->students[a.disciplineFrom] -= getExchangeRate(g, currentPlayer->playerID, a.disciplineFrom, a.disciplineTo);
        currentPlayer->students[a.disciplineTo]++;
    }
}


//Complete
void throwDice(Game g, int diceScore) {
    //Adv turn
    g->currentTurn++;
    //Give resources

    //Give resources
    //Find hexs that have dice value == diceScore
    //Added rescourese to players that have unis/GO8s in on the adjacant verts
    int iter = 0;
    while (iter < NUM_HEXS) {
        if (g->dice[iter] == diceScore) {
            //Hex iter has been rolled
            hex hexRolled = g->hexArray[iter];

            addResourcesForHexAndVert(g, hexRolled, hexRolled->vertLeft);
            addResourcesForHexAndVert(g, hexRolled, hexRolled->vertUpLeft);
            addResourcesForHexAndVert(g, hexRolled, hexRolled->vertUpRight);
            addResourcesForHexAndVert(g, hexRolled, hexRolled->vertRight);
            addResourcesForHexAndVert(g, hexRolled, hexRolled->vertDownRight);
            addResourcesForHexAndVert(g, hexRolled, hexRolled->vertDownLeft);
        }

        iter++;
    }

    if (diceScore == 7) {
        int i = 0;
        int temp = 0;
        while (i < NUM_UNIS) {
            temp = g->playerArray[i]->students[STUDENT_MTV];
            g->playerArray[i]->students[STUDENT_MTV] = 0;
            temp += g->playerArray[i]->students[STUDENT_MMONEY];
            g->playerArray[i]->students[STUDENT_MMONEY] = 0;
            g->playerArray[i]->students[STUDENT_THD] += temp;

            i++;
        }
    }
}

// Completed
int getDiscipline(Game g, int regionID) {
    return g->disciplines[regionID];
}

// Completed
int getDiceValue(Game g, int regionID) {
    return g->dice[regionID];
}

// Completed
int getMostARCs(Game g) {
    int uniWithARCs = NO_ONE;
    int mostARCs = 0;

    char allEqual = 1;

    int i = 1;
    while (i <= NUM_UNIS) {
        if (g->playerArray[i - 1]->numARCs > mostARCs) {
            uniWithARCs = i;
            mostARCs = g->playerArray[i - 1]->numARCs;
        }

        if (g->playerArray[i - 1]->numARCs != mostARCs && i != 1) {
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
int getMostPublications(Game g) {
    int uniWithPubs = NO_ONE;
    int mostPubs = 0;

    int i = 1;
    while (i <= 3) {
        if (g->playerArray[i - 1]->numPubs > mostPubs) {
            uniWithPubs = i;
            mostPubs = g->playerArray[i - 1]->numPubs;
        }
        i++;
    }

    return uniWithPubs;
}

// Completed
int getTurnNumber(Game g) {
    return g->currentTurn;
}

// Completed
int getWhoseTurn(Game g) {
    int returnValue;
    if (g->currentTurn == -1) {
        returnValue = NO_ONE;
    } else {
        returnValue = g->playerArray[g->currentTurn % NUM_UNIS]->playerID;
    }
    return returnValue;
}

// Completed
int getCampus(Game g, path pathToVertex) {
    //Get vert using local function
    vert vertToReturn = getVertAtPath(g, pathToVertex);

    //Return val from vert
    return vertToReturn->contents; // Placeholder so it compiles
}

// Completed - ish
// Needs edges to be initalised
int getARC(Game g, path pathToEdge) {
    /*
    edge edgeToReturn = getEdgeAtPath(g, pathToEdge);

    return edgeToReturn->contents;*/
    return 0;
}

// Completed
int isLegalAction(Game g, action a) {
    int isLegal = FALSE;

    player currentPlayer = g->playerArray[g->currentTurn % NUM_UNIS];

    if (a.actionCode == PASS) {
        isLegal = TRUE;
    } else if (a.actionCode == BUILD_CAMPUS) {
        vert campus = getVertAtPath(g, a.destination);
        // check if there's enough students
        if (currentPlayer->students[STUDENT_BPS] < 1 ||
            currentPlayer->students[STUDENT_BQN] < 1 ||
            currentPlayer->students[STUDENT_MJ] < 1 ||
            currentPlayer->students[STUDENT_MTV] < 1) {
            isLegal = FALSE;
        // Check if the campus is connected to an ARC grant
        } else if ((campus->edgeUp->contents != currentPlayer->playerID) &&
        (campus->edgeDown->contents != currentPlayer->playerID) &&
        (campus->edgeSide->contents != currentPlayer->playerID)) { // Placeholder
            isLegal = FALSE;
        } else {
            isLegal = TRUE;
        }
    } else if (a.actionCode == BUILD_GO8) {
        vert campus = getVertAtPath(g, a.destination);
        // check if there's enough students
        if (currentPlayer->students[STUDENT_MJ] < 2 ||
            currentPlayer->students[STUDENT_MMONEY] < 3) {
            isLegal = FALSE;
        // Check if there's a campus by the player
        } else if (campus->contents != currentPlayer->playerID) {
            isLegal = FALSE;
        } else {
            isLegal = TRUE;
        }
    } else if (a.actionCode == OBTAIN_ARC) {
        edge arc = getEdgeAtPath(g, a.destination);
        // check if the location of the player is connected to his/her ARC
        if ((arc->vertUp->edgeUp->contents != currentPlayer->playerID) &&
            (arc->vertUp->edgeDown->contents != currentPlayer->playerID) &&
            (arc->vertUp->edgeSide->contents != currentPlayer->playerID) &&
            (arc->vertDown->edgeUp->contents != currentPlayer->playerID) &&
            (arc->vertDown->edgeDown->contents != currentPlayer->playerID) &&
            (arc->vertDown->edgeSide->contents != currentPlayer->playerID)) {
            isLegal = FALSE;
        // check if there's enough students
        } else if (currentPlayer->students[STUDENT_BPS] < 1 || currentPlayer->students[STUDENT_BQN] < 1) {
            isLegal = FALSE;
        } else {
            isLegal = TRUE;
        }
    } else if (a.actionCode == START_SPINOFF) {
        if (currentPlayer->students[STUDENT_MJ] < 1 ||
            currentPlayer->students[STUDENT_MTV] < 1 ||
            currentPlayer->students[STUDENT_MMONEY] < 1) {
            isLegal = FALSE;
        } else {
            isLegal = TRUE;
        }
    } else if (a.actionCode == OBTAIN_PUBLICATION) {
        // OBTAIN_PUBLICATION and OBTAIN_IP_PATENT are always illegal unless called by START_SPINOFF
        isLegal = FALSE;
    } else if (a.actionCode == OBTAIN_IP_PATENT) {
        isLegal = FALSE;
    } else if (a.actionCode == RETRAIN_STUDENTS) {
        // see if (disciplineFrom != STUDENT_THD)
        if (a.disciplineFrom == STUDENT_THD) {
            isLegal = FALSE;
        } else if (currentPlayer->students[a.disciplineFrom] <
            getExchangeRate(g, currentPlayer->playerID,
                a.disciplineFrom, a.disciplineTo)) {
            isLegal = FALSE;
        } else {
            isLegal = TRUE;
        }
    }

    return isLegal; // Placeholder
}

// Completed
int getKPIpoints(Game g, int player) {
    return g->playerArray[player - 1]->kpiPoints;
}

// Completed
int getARCs(Game g, int player) {
    return g->playerArray[player - 1]->numARCs;
}

// Completed
int getGO8s(Game g, int player) {
    return g->playerArray[player - 1]->numGO8s;
}

// Completed
int getCampuses(Game g, int player) {
    return g->playerArray[player - 1]->numUnis;
}

// Completed
int getIPs(Game g, int player) {
	return g->playerArray[player - 1]->numIPs;
    //return getPlayer(g, player);
}

// Completed
int getPublications(Game g, int player) {
	return g->playerArray[player - 1]->numPubs;
}

// Completed
int getStudents(Game g, int player, int discipline) {
    return g->playerArray[player - 1]->students[discipline];
}

// Completed
int getExchangeRate(Game g, int player, int disciplineFrom, int disciplineTo) {
    int hasBPSExchange = FALSE;
    int hasMJExchange = FALSE;
    int hasBQNExchange = FALSE;
    int hasMMONEYExchange = FALSE;
    int hasMTVExchange = FALSE;

        //Check which verts the player owns
    if (getVert(g, VERT_CONV_BPS1_INDEX)->playerID == player ||
        getVert(g, VERT_CONV_BPS2_INDEX)->playerID == player) {
        hasBPSExchange = TRUE;
    }

    if (getVert(g, VERT_CONV_MMONEY1_INDEX)->playerID == player ||
        getVert(g, VERT_CONV_MMONEY2_INDEX)->playerID == player) {
        hasMMONEYExchange = TRUE;
    }

    if (getVert(g, VERT_CONV_MTV1_INDEX)->playerID == player ||
        getVert(g, VERT_CONV_MTV2_INDEX)->playerID == player) {
        hasMTVExchange = TRUE;
    }

    if (getVert(g, VERT_CONV_BQN1_INDEX)->playerID == player ||
        getVert(g, VERT_CONV_BQN2_INDEX)->playerID == player) {
        hasMTVExchange = TRUE;
    }

    if (getVert(g, VERT_CONV_MJ1_INDEX)->playerID == player ||
        getVert(g, VERT_CONV_MJ2_INDEX)->playerID == player) {
        hasMTVExchange = TRUE;
    }

	/*printf("BPS:%d, MJ:%d, BQN:%d, MTV:%d, MMONEY:%d\n",
		hasBPSExchange, hasMJExchange, 
		hasBQNExchange, hasMTVExchange, hasMMONEYExchange);*/

    int exchangeRate = 3;

    if (disciplineFrom == STUDENT_BPS) {
        if (hasBPSExchange == TRUE) {
            exchangeRate = 2;
        }
    } else if (disciplineFrom == STUDENT_MJ) {
        if (hasMJExchange == TRUE) {
            exchangeRate = 2;
        }
    } else if (disciplineFrom == STUDENT_BQN) {
        if (hasBQNExchange == TRUE) {
            exchangeRate = 2;
        }
    } else if (disciplineFrom == STUDENT_MTV) {
        if (hasMTVExchange == TRUE) {
            exchangeRate = 2;
        }
    } else if (disciplineFrom == STUDENT_MMONEY) {
        if (hasMMONEYExchange == TRUE) {
            exchangeRate = 2;
        }
    }


    return exchangeRate;
}
