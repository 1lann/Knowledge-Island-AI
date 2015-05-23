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
#include <strings.h>
#include <math.h>

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

#define NUM_DISCIPLINES 6

#define NUM_COLUMNS 5
#define ROWS_FOR_COLUMN {3, 4, 5, 4, 3}
#define START_LEFT_COLUMN {0, 7, 16, 28, 39}
#define START_RIGHT_COLUMN {8, 17, 27, 38, 47}

#define START_BRIDGE_LEFT {2, 9, 18, 30, 41}
#define START_BRIDGE_RIGHT {10, 19, 29, 40, 44}

// Order: THD, BPS, BQN, MJ, MTV, MMONEY
#define WEIGHTINGS {0, 30, 30, 50, 30, 40}

#define SEARCH_DEPTH 4
#define DEPTH_MULTIPLIER 0.5

// // return the contents of the given vertex (ie campus code or
// // VACANT_VERTEX)
// int getCampus(Game g, path pathToVertex);

// // the contents of the given edge (ie ARC code or vacent ARC)
// int getARC(Game g, path pathToEdge);

typedef struct _vertex {
	int object;
	char path[PATH_LIMIT];
	int disciplineA;
	int disciplineB;
	int disciplineC;
} vertex;

typedef struct _arc {
	int object;
	char path[PATH_LIMIT];
} arc;

typedef struct _pair {
	int a;
	int b;
} pair;

typedef struct _trio {
	int a;
	int b;
	int c;
} trio;

typedef struct _weightedVertex {
	int vertexId;
	int weight;
} weightedVertex;

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

	return columnNum;
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
			result = FALSE;
		}
	} else if (column == 1 || column == 4) {
		if (position % 2 == 0) {
			result = FALSE;
		} else {
			result = TRUE;
		}
	}

	return result;
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
			result = position + getJumpLeftConversion(position);
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
			result = position + getJumpRightConversion(position);
		} else {
			// You can move left, so you jump
			result = position - 1;
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

	while (i < pathLen && currentPosition >= 0 && currentPosition < NUM_INT_VERTICES) {
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


// Get connecting vertices of an arc
pair getArcVertices(int arcId) {
	pair arcPairs[NUM_INT_ARCS];

	int i = 0;

	while (i < NUM_INT_VERTICES) {
		pair newPair;
		newPair.a = i;
		newPair.b = i - 1;
		arcPairs[i] = newPair;
	}

	arcPairs[0].b = 8;
	arcPairs[7].b = 17;
	arcPairs[27].b = 16;
	arcPairs[38].b = 28;
	arcPairs[47].b = 39;

	int startLeft[] = START_BRIDGE_LEFT;
	int startRight[] = START_BRIDGE_RIGHT;
	int columnRows[] = ROWS_FOR_COLUMN;

	i = NUM_INT_VERTICES;
	int column = 0;

	while (column < NUM_COLUMNS) {
		int row = 0;

		while (row < columnRows[column] + 1) {
			arcPairs[i].a = startLeft[column] + (row * 2);
			arcPairs[i].b = startRight[column] + (row * 2);
			row++;
			i++;
		}


		column++;
	}

	return arcPairs[arcId];
}


trio getNeighbouringVertices(int vertexId) {
	int results[3];
	int resultsIterator = 0;
	int i = 0;

	while (i < NUM_INT_VERTICES) {
		pair result = getArcVertices(i);

		if (result.a == vertexId) {
			results[resultsIterator] = result.b;
			resultsIterator++;
		} else if (result.b == vertexId) {
			results[resultsIterator] = result.a;
			resultsIterator++;
		}

		i++;
	}

	trio resultTrio;

	if (resultsIterator >= 0) {
		resultTrio.a = results[0];
	}
	if (resultsIterator >= 1) {
		resultTrio.b = results[1];
	}
	if (resultsIterator >= 2) {
		resultTrio.c = results[2];
	}

	return resultTrio;
}


// Returns the disciplines of surrounding hexes of a vertices
trio getVertexHexes(int vertexId) {
	int results[3];
	int resultI = 0;

	results[0] = -1;
	results[1] = -1;
	results[2] = -1;

	int rowsForColumns[NUM_COLUMNS] = ROWS_FOR_COLUMN;
	int startLeftColumns[NUM_COLUMNS] = START_LEFT_COLUMN;
	int startRightColumns[NUM_COLUMNS] = START_RIGHT_COLUMN;

	// An array of regions, containg the vertices surrounding them.
	int regionVertices[NUM_REGIONS][6];

	int region = 0;

	int column = 0;
	while (column < NUM_COLUMNS && resultI < 2) {
		int row = 0;
		while (row < rowsForColumns[column]) {
			int left = startLeftColumns[column] + (row * 2);
			int right = startRightColumns[column] + (row * 2);

			if (vertexId - left <= 2 && vertexId - left >= 0) {
				results[resultI] = region;
				resultI++;
			}

			region++;
			row++;
		}

		column++;
	}

	trio resultTrio;
	int allDisciplines[NUM_REGIONS] = DEFAULT_DISCIPLINES;

	if (results[0] >= 0) {
		resultTrio.a = allDisciplines[results[0]];
	} else {
		resultTrio.a = -1;
	}

	if (results[1] >= 0) {
		resultTrio.b = allDisciplines[results[1]];
	} else {
		resultTrio.b = -1;
	}

	if (results[2] >= 0) {
		resultTrio.c = allDisciplines[results[2]];
	} else {
		resultTrio.c = -1;
	}

	return resultTrio;
}


// Gets value of vertex by itself
int getSingleVertexWeight(vertex vertices[NUM_INT_VERTICES],
	int myVertices[NUM_INT_VERTICES], int numMyVertices, int vertexId) {

	int weights[NUM_DISCIPLINES] = WEIGHTINGS;
	int subWeights[NUM_DISCIPLINES];

	int i = 0;

	while (i < NUM_DISCIPLINES) {
		subWeights[i] = 0;
		i++;
	}

	i = 0;

	while (i < numMyVertices) {
		trio myHexes = getVertexHexes(myVertices[i]);

		if (myHexes.a >= 0) {
			subWeights[myHexes.a] += 1;
		}

		if (myHexes.b >= 0) {
			subWeights[myHexes.b] += 1;
		}

		if (myHexes.c >= 0) {
			subWeights[myHexes.c] += 1;
		}
	}

	int sum = 0;
	trio hexes = getVertexHexes(vertexId);

	if (hexes.a >= 0) {
		sum += weights[hexes.a] - subWeights[hexes.a];
	}

	if (hexes.b >= 0) {
		sum += weights[hexes.b] - subWeights[hexes.b];
	}

	if (hexes.c >= 0) {
		sum += weights[hexes.c] - subWeights[hexes.c];
	}

	return sum;
}


int getRecursiveVertexWeight(vertex vertices[NUM_INT_VERTICES],
	int myVertices[NUM_INT_VERTICES], int numMyVertices, int vertexId) {

	int layerQueue[SEARCH_DEPTH + 1][NUM_INT_VERTICES];

	// Clear table
	int layer = 0;
	while (layer < SEARCH_DEPTH + 1) {
		int i = 0;
		while (i < NUM_INT_VERTICES) {
			layerQueue[layer][i] = -1;
		}
	}

	layerQueue[0][0] = vertexId;

	int seen[NUM_INT_VERTICES];

	int i = 0;
	while (i < NUM_INT_VERTICES) {
		seen[i] = FALSE;
		i++;
	}

	seen[vertexId] = TRUE;

	double sum = (double)getSingleVertexWeight(vertices,
		myVertices, numMyVertices, vertexId);

	layer = 0;
	while (layer < SEARCH_DEPTH) {
		int endOfQueue = FALSE;
		int i = 0;
		while (i < NUM_INT_VERTICES && !endOfQueue) {
			if (layerQueue[layer][i] < 0) {
				endOfQueue = TRUE;
			} else {
				trio neighbours = getNeighbouringVertices(layerQueue[layer][i]);
				int queuePusher = 0;

				if (neighbours.a >= 0 && !seen[neighbours.a] &&
					!alreadyOwnVertex(myVertices, neighbours.a)) {
					layerQueue[layer + 1][queuePusher] = neighbours.a;
					sum += (double)getSingleVertexWeight(vertices,
						myVertices, numMyVertices, neighbours.a) *
						pow(DEPTH_MULTIPLIER, layer);
					queuePusher++;
				}

				if (neighbours.b >= 0 && !seen[neighbours.b] &&
					!alreadyOwnVertex(myVertices, neighbours.b)) {
					layerQueue[layer + 1][queuePusher] = neighbours.b;
					sum += (double)getSingleVertexWeight(vertices,
						myVertices, numMyVertices, neighbours.b) *
						pow(DEPTH_MULTIPLIER, layer);
					queuePusher++;
				}

				if (neighbours.c >= 0 && !seen[neighbours.c] &&
					!alreadyOwnVertex(myVertices, neighbours.c)) {
					layerQueue[layer + 1][queuePusher] = neighbours.c;
					sum += (double)getSingleVertexWeight(vertices,
						myVertices, numMyVertices, neighbours.c) *
						pow(DEPTH_MULTIPLIER, layer);
					queuePusher++;
				}

				i++;
			}
		}
	}

	return (int)sum;
}


int alreadyOwnVertex(int myVertices[NUM_INT_VERTICES], int queryVertex) {
	int i = 0;
	int match = FALSE;

	while (i < NUM_INT_VERTICES && !match) {
		if (myVertices[i] == queryVertex) {
			match = TRUE;
		}
	}

	return match;
}


void sortWeights(weightedVertex *list, int arraySize) {
	int changesMade = TRUE;

	while (changesMade) {
		changesMade = FALSE;

		int i = 1;
		while (i < arraySize) {
			if (list[i].weight > list[i - 1].weight) {
				int newFirst = list[i - 1].weight;
				list[i - 1].weight = list[i].weight;
				list[i].weight = newFirst;

				changesMade = TRUE;
			}

			i++;
		}
	}
}


action decideAction(Game g) {
	action nextAction;

	vertex vertices[NUM_INT_VERTICES];
	arc arcs[NUM_INT_ARCS];

	int currentPlayer = getWhoseTurn(g);

	int myCampus;
	int myGO8;
	int myARC;

	if (currentPlayer == UNI_A) {
		myCampus = CAMPUS_A;
		myGO8 = GO8_A;
		myARC = ARC_A;
	} else if (currentPlayer == UNI_B) {
		myCampus = CAMPUS_B;
		myGO8 = GO8_B;
		myARC = ARC_C;
	} else if (currentPlayer == UNI_C) {
		myCampus = CAMPUS_C;
		myGO8 = GO8_C;
		myARC = ARC_C;
	}

	char allPaths[NUM_INT_VERTICES][PATH_LIMIT] = ALL_PATHS;
	char arcPaths[NUM_INT_ARCS - NUM_INT_VERTICES][PATH_LIMIT] = ARC_PATHS;

	// Populate database

	int i = 0;

	while (i < NUM_INT_VERTICES) {
		vertex newVertex;
		arc newArc;

		strcpy(newVertex.path, allPaths[i]);
		strcpy(newArc.path, allPaths[i]);

		newVertex.object = getCampus(g, allPaths[i]);
		newArc.object = getARC(g, allPaths[i]);

		vertices[i] = newVertex;
		arcs[i] = newArc;

		i++;
	}

	while (i < NUM_INT_ARCS) {
		arc newArc;

		strcpy(newArc.path, arcPaths[i - NUM_INT_VERTICES]);
		newArc.object = getARC(g, newArc.path);

		arcs[i] = newArc;

		i++;
	}

	// If we have more than 10 campuses, plan for building GO8s
	// If we have enough resources to build a GO8...
	//    Upgrade the most valued campus to a GO8


	// Note that this AI will only build ARCs and campuses together at the same time
	// If we have enough resources to build an ARC and campus...
	// Get a list of all the "edge" vertices
	// Iterate through each connecting edge vertices, and give them a value
	// based off the hexes they are connected to.
	// For every campus of already existing resource, a point is subtracted
	// Their values is summed of the vertices they're connected to, too.
	// Cumalative values are halved for every vertex travelled to a maximum of 4 jumps
	// Then pick the highest scoring sub-vertex.
	// If there are multiple highest scoring sub-vertices, pick the one with the lowest index

	// Find vertices we own

	int myVertices[NUM_INT_VERTICES]; // Array of vertices that we own

	i = 0;
	int numMyVertices = 0;

	while (i < NUM_INT_VERTICES) {
		myVertices[i] = -1;

		if (vertices[i].object == myCampus || vertices[i].object == myGO8) {
			myVertices[numMyVertices] = i;
			numMyVertices++;
		}
		i++;
	}

	// Now scan through each vertex and store neighbours

	int consideration[NUM_INT_VERTICES]; // Array of vertices that are accessible and aren't owned

	i = 0;
	while (i < NUM_INT_VERTICES) {
		consideration[i] = -1;
		i++;
	}

	i = 0;
	int numConsiderations = 0;

	while (i < numMyVertices) {
		trio neighbouring = getNeighbouringVertices(myVertices[i]);

		if (neighbouring.a >= 0 && vertices[neighbouring.a].object == VACANT_VERTEX) {
			consideration[numConsiderations] = neighbouring.a;
			numConsiderations++;
		}
		if (neighbouring.b >= 0 && vertices[neighbouring.b].object == VACANT_VERTEX) {
			consideration[numConsiderations] = neighbouring.b;
			numConsiderations++;
		}
		if (neighbouring.c >= 0 && vertices[neighbouring.c].object == VACANT_VERTEX) {
			consideration[numConsiderations] = neighbouring.c;
			numConsiderations++;
		}

		i++;
	}

	// Now get the weight values of each consideration

	weightedVertex sortedWeights[numConsiderations];

	i = 0;
	while (i < numConsiderations) {
		int weight = getRecursiveVertexWeight(vertices, myVertices, numMyVertices,
			consideration[i]);
		weightedVertex newVertex;
		newVertex.weight = weight;
		newVertex.vertexId = consideration[i];

		sortedWeights[i] = newVertex;

		i++;
	}

	sortWeights(sortedWeights, numConsiderations);

	// Buy in order of weighting


	// Attempt actions by weighted order, checking if they're legal


	// If there are any 3+ left over resources, start spin-offs. Check if legal.

	// If there is an outlier resource with 7+ left over, exchange them to
	// resource we have least have excluding THDs. Check if legal.

	printf("Done!\n");


	return nextAction;
}

int main() {
	int defaultDis[] = DEFAULT_DISCIPLINES;
	int defaultDice[] = DEFAULT_DICE;

	Game thisGame = newGame(defaultDis, defaultDice);
	throwDice(thisGame, 2);
	action thisAction = decideAction(thisGame);
}
