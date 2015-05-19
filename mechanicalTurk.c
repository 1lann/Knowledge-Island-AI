/*
*  Mr Pass.  Brain the size of a planet!
*
*  Proundly Created by Richard Buckland
*  Share Freely Creative Commons SA-BY-NC 3.0.
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "Game.h"
#include "mechanicalTurk.h"

#define NUM_INT_VERTICES 54
#define NUM_INT_ARCS 73

// All paths to vertices including ARCs
#define ALL_PATHS {"RRLR", "RRLRL", "RRLRLL", "RRLRLLR", "RRLRLLRL",\
 "RRLRLLRLR", "RRLRLLRLRL", "RR", "RRL", "RRLL", "RRLLR", "RRLLRL", "RRLLRLR",\
 "RRLLRLRL", "RRLLRLRLR", "RRLLRLRLRL", "", "R", "RL", "RLR", "RLRL", "RLRLR",\
 "RLRLRL", "RLRLRLR", "RLRLRLRL", "RLRLRLRLR", "RLRLRLRLRL", "L", "LR", "LRR",\
 "LRRL", "LRRLR", "LRRLRL", "LRRLRLR", "LRRLRLRL", "LRRLRLRLR", "LRRLRLRLRL",\
 "LRRLRLRLRLR", "LRL", "LRLR", "LRLRR", "LRLRRL", "LRLRRLR", "LRLRRLRL",\
 "LRLRRLRLR", "LRLRRLRLRL", "LRLRRLRLRLR", "LRLRL", "LRLRLR", "LRLRLRR",\
 "LRLRLRRL", "LRLRLRRLR", "LRLRLRRLRL", "LRLRLRRLRLR"}

// Additional paths for ARCs
#define ARC_PATHS {"RRLRLLL", "RRLRLLRLL", "RRLRLLRLRLL",\
 "RRLLL", "RRLLRLL", "RRLLRLRLL", "RRLLRLRLRLL", "RLL",\
 "RLRLL", "RLRLRLL", "RLRLRLRLL", "RLRLRLRLRLL", "LRRLL",\
 "LRRLRLL", "LRRLRLRLL", "LRRLRLRLRLL", "LRLRRLL", "LRLRRLRLL",\
 "LRLRRLRLRLL"}

// // return the contents of the given vertex (ie campus code or
// // VACANT_VERTEX)
// int getCampus(Game g, path pathToVertex);

// // the contents of the given edge (ie ARC code or vacent ARC)
// int getARC(Game g, path pathToEdge);

typedef struct vertex {
	int object;
	char path[PATH_LIMIT];
};

typedef struct arc {
	int object;
	char path[PATH_LIMIT];
};

//
// The Mapping and Pathing Engine
//

int getColumnNum(int position) {
	int columnNum = -1;

	if (position >= 0 && position <= 6) {
		columnNum = 0;
	} else if (position >= 7 && position <= 15) {
		columnNum = 1;
	} else if (position >= 16 && position <= 26) {
		columnNum = 2;
	} else if (position >= 27 && position <= 37) {
		columnNum = 3;
	} else if (position >= 38 && position <= 46) {
		columnNum = 4;
	} else if (position >= 47 && position <= 53) {
		columnNum = 5;
	}

	return columnNum
}


int verifySameColumn(int first, int second) {
	int ok = FALSE;

	if (getColumnNum(first) == getColumnNum(second)) {
		ok = TRUE;
	}

	return ok;
}


int getJumpRightConversion(int position) {
	int conversion = 0;

	int column = getColumnNum(position);

	if (column == 0) {
		conversion = 8;
	} else if (column == 1) {
		conversion = 10;
	} else if (column == 2) {
		conversion = 11;
	} else if (column == 3) {
		conversion = 10;
	} else if (column == 4) {
		conversion = 8;
	} else if (column == 5) {
		conversion = 100;
	}

	return conversion;
}


int getJumpLeftConversion(int position) {
	int conversion = 0;

	int column = getColumnNum(position);

	if (column == 0) {
		conversion = -100;
	} else if (column == 1) {
		conversion = -8;
	} else if (column == 2) {
		conversion = -10;
	} else if (column == 3) {
		conversion = -11;
	} else if (column == 4) {
		conversion = -10;
	} else if (column == 5) {
		conversion = -8;
	}

	return conversion;
}


// Whether you can jump to the right at a position
int canJumpRight(int position) {
	int result = FALSE;

	int column = getColumnNum(position);

	if (column == 0 || column == 2 || column == 3 || column == 5) {
		if (position % 2 == 0) {
			result = TRUE;
		} else {
			result = FALSE:
		}
	} else if (column == 1 || column == 4) {
		if (position % 2 == 0) {
			result = FALSE;
		} else {
			result = TRUE:
		}
	}

	return modResult;
}


int positionBackFrom(int position, int relativeTo) {
	return relativeTo;
}


int positionLeftFrom(int position, int relativeTo) {
	int result = 0;
	int sameColumn = FALSE;

	if (relativeTo - position == 1) {
		// We are moving up the column
		if (canJumpRight(position)) {
			// You can move right, so on the left, you move up
			result = position - 1;
			sameColumn = TRUE;
		} else {
			// You can move left, so you jump
			result = position + getJumpLeftConversion(position)
		}

	} else if (relativeTo - position == -1) {
		// We are moving down the column
		if (canJumpRight(position)) {
			result = position + getJumpRightConversion(position);
		} else {
			result = position + 1;
			sameColumn = TRUE;
		}

	} else if (relativeTo < position) {
		// Moving left to right
		result = position - 1;
		sameColumn = TRUE;

	} else if (relativeTo > position) {
		// Moving right to left
		result = position + 1;
		sameColumn = TRUE;
	}

	if (sameColumn && !verifySameColumn(position, result)) {
		result = -100;
	}

	return result;
}


int positionRightFrom(int position, int relativeTo) {
	int result = 0;
	int sameColumn = FALSE;

	if (relativeTo - position == 1) {
		// We are moving up the column
		if (canJumpRight(position)) {
			// You can move right
			result = position + getJumpRightConversion(position)
		} else {
			// You can move left, so you jump
			resilt = position - 1;
			sameColumn = TRUE;
		}

	} else if (relativeTo - position == -1) {
		// We are moving down the column
		if (canJumpRight(position)) {
			result = position + 1;
			sameColumn = TRUE;
		} else {
			result = position + getJumpLeftConversion(position);
		}

	} else if (relativeTo < position) {
		// Moving left to right
		result = position + 1;
		sameColumn = TRUE;

	} else if (relativeTo > position) {
		// Moving right to left
		result = position - 1;
		sameColumn = TRUE;
	}

	if (sameColumn && !verifySameColumn(position, result)) {
		result = -100;
	}

	return result;
}


// Converts a path into a vertex position.
int pathToPosition(path p) {
	int pathLen = strlen(p);

	int lastPosition = 16;
	int currentPosition = -1;

	int i = 0;
	if (p[i] == 'L') {
		currentPosition = 27;
	} else if (p[i] == 'R') {
		currentPosition = 17;
	} else if (p[i] == 'B') {
		currentPosition = -1;
	}

	i++;

	while (i < pathLen && currentPosition >= 0 && currentPosition < NUM_VERTICES) {
		char instruction = p[i];
		int lastLastPosition = currentPosition;

		if (instruction == 'L') {
			currentPosition = positionLeftFrom(currentPosition, lastPosition);
		} else if (instruction == 'R') {
			currentPosition = positionRightFrom(currentPosition, lastPosition);
		} else if (instruction == 'B') {
			currentPosition = positionBackFrom(currentPosition, lastPosition);
		}

		lastPosition = lastLastPosition;

		i++;
	}

	return currentPosition;
}


action decideAction(Game g) {
    action nextAction;

    vertex vertices[NUM_INT_VERTICES];
    arc arcs[NUM_INT_ARCS];

    int currentPlayer = getWhoseTurn(g);

    int ourCampus;
    int ourGO8;
    int ourARC;

    if (currentPlayer == UNI_A) {
    	ourCampus = CAMPUS_A;
    	ourGO8 = GO8_A;
    	ourARC = ARC_A;
    } else if (currentPlayer == UNI_B) {
    	ourCampus = CAMPUS_B;
    	ourGO8 = GO8_B;
    	ourARC = ARC_C;
    } else if (currentPlayer == UNI_C) {
    	ourCampus = CAMPUS_C;
    	ourGO8 = GO8_C;
    	ourARC = ARC_C;
    }

    char allPaths[NUM_INT_VERTICES][PATH_LIMIT] = ALL_PATHS;
    char arcPaths[NUM_INT_ARCS - NUM_INT_VERTICES][PATH_LIMIT] = ARC_PATHS;

    // Populate database

    int i = 0;

    while (i < NUM_INT_VERTICES) {
    	vertex newVertex;
    	arc newArc;

    	newVertex.path = allPaths[i];
    	newArc.path = allPaths[i];

    	newVertex.object = getCampus(g, allPaths[i]);
    	newArc.object = getARC(g, allPaths[i]);

    	vertices[i] = newVertex;
    	arcs[i] = newArc;

    	i++;
    }

    while (i < NUM_INT_ARCS) {
    	arc newArc;

    	newArc.path = arcPaths[i - NUM_INT_VERTICES];
    	newArc.object = getARC(g, newArc.path);

    	arcs[i] = newArc;

    	i++;
    }

    // If we have more than 10 campuses, plan for building GO8s
    // If we have enough resources to build a GO8...
    //    Upgrade the most valued campus to a GO8


    // Note that this AI will only build ARCs and campuses together at the same time
    // If we have enouggh resources to build an ARC and campus...
    // Get a list of all the "edge" nodes
    // Iterate through each connecting edge nodes, and give them a value
    // based off the nodes they are connected to.
    // Their values is summed of the nodes they're connected to, too.
    // Cumalative values are halved for every node travelled to a maximum of 4 jumps
    // Then pick the highest scoring sub-node.
    // If there are multiple highest scoring sub-nodes, pick the one with the lowest index


    return nextAction;
}
