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
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <time.h>

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
#define START_BRIDGE_RIGHT {10, 19, 29, 40, 49}

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

typedef struct _fromToArc {
	int from;
	int to;
	int alreadyOwned;
	int valid;
} fromToArc;

typedef struct _weightedVertex {
	fromToArc arcPath[2];
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
		i++;
	}

	arcPairs[0].b = 8;
	arcPairs[7].b = 17;
	arcPairs[16].a = -1;
	arcPairs[16].b = -1;
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

		while (row < columnRows[column]) {
			arcPairs[i].a = startLeft[column] + (row * 2);
			arcPairs[i].b = startRight[column] + (row * 2);

			// printf("Linking %d to %d\n", arcPairs[i].a, arcPairs[i].b);

			row++;
			i++;
		}

		column++;
	}

	return arcPairs[arcId];
}


int getArcIdFromVertices(int firstId, int secondId) {
	int i = 0;
	int result = -1;
	while (i < NUM_INT_ARCS && result < 0) {
		pair vertexPair = getArcVertices(i);

		if (vertexPair.a == firstId && vertexPair.b == secondId) {
			result = i;
		} else if (vertexPair.a == secondId && vertexPair.b == firstId) {
			result = i;
		}

		i++;
	}

	return result;
}


trio getNeighbouringVertices(int vertexId) {
	int results[3];
	int resultsIterator = 0;
	int i = 0;

	while (i < NUM_INT_ARCS) {
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

	// printf("%d has the neighbours: ", vertexId);

	trio resultTrio;

	if (resultsIterator > 0) {
		resultTrio.a = results[0];
		// printf("%d, ", resultTrio.a);
	} else {
		resultTrio.a = -1;
	}

	if (resultsIterator > 1) {
		resultTrio.b = results[1];
		// printf("%d, ", resultTrio.b);
	} else {
		resultTrio.b = -1;
 	}

	if (resultsIterator > 2) {
		resultTrio.c = results[2];
		// printf("%d, ", resultTrio.c);
	} else {
		resultTrio.c = -1;
	}

	// printf("\n");

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

		i++;
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


int alreadyOwnVertex(int myVertices[NUM_INT_VERTICES], int queryVertex) {
	int i = 0;
	int match = FALSE;

	while (i < NUM_INT_VERTICES && !match) {
		if (myVertices[i] == queryVertex) {
			match = TRUE;
		}

		i++;
	}

	return match;
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

			i++;
		}

		layer++;
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

		layer++;
	}

	return (int)sum;
}


int canBuildCampusOn(vertex vertices[NUM_INT_VERTICES], int id) {
	int result = TRUE;

	trio neighbours = getNeighbouringVertices(id);

	if ((neighbours.a >= 0 && vertices[neighbours.a].object != VACANT_VERTEX) ||
		(neighbours.b >= 0 && vertices[neighbours.b].object != VACANT_VERTEX) ||
		(neighbours.c >= 0 && vertices[neighbours.c].object != VACANT_VERTEX)) {
		result = FALSE;
	}

	if (vertices[id].object != VACANT_VERTEX) {
		result = FALSE;
	}

	// printf("Result for: %d is %d\n", id, result);

	return result;
}


void sortWeights(weightedVertex *list, int arraySize) {
	int changesMade = TRUE;

	while (changesMade) {
		changesMade = FALSE;

		int i = 1;
		while (i < arraySize) {
			if (list[i].weight > list[i - 1].weight) {
				weightedVertex newFirst = list[i - 1];
				list[i - 1] = list[i];
				list[i] = newFirst;

				changesMade = TRUE;
			}

			i++;
		}
	}
}

void convertStudents(Game g, int studentFrom, int studentTo, int numToCreate){
	action a;
	a.actionCode = RETRAIN_STUDENTS;
	a.disciplineFrom = studentFrom;
	a.disciplineTo = studentTo;

	int i = 0;
	while (i < numToCreate) {
		if (isLegalAction(g, a)){
			makeAction(g, a);
		}
		i++;
	}

}

int enoughToBuildCampus(Game g, int playerId, fromToArc arcPath[2]) {
	int result = FALSE;

	int pathResources = 2;

	if (arcPath[0].alreadyOwned) {
		pathResources--;
	}

	if (arcPath[1].alreadyOwned) {
		pathResources--;
	}

	int conRateBPS = getExchangeRate(g, playerId, STUDENT_BPS, STUDENT_BPS);
	int conRateBQN = getExchangeRate(g, playerId, STUDENT_BPS, STUDENT_BQN);
	int conRateMJ = getExchangeRate(g, playerId, STUDENT_BPS, STUDENT_MJ);
	int conRateMTV = getExchangeRate(g, playerId, STUDENT_BPS, STUDENT_MTV);

	int numBPS = getStudents(g, playerId, STUDENT_BPS);
	int numBQN = getStudents(g, playerId, STUDENT_BQN);
	int numMJ = getStudents(g, playerId, STUDENT_MJ);
	int numMTV = getStudents(g, playerId, STUDENT_MTV);

	if (numMTV < 1) {
		if (numBPS - conRateBPS >= pathResources + 1) {
			convertStudents(g, STUDENT_BPS, STUDENT_MTV, 1);
			numBPS = numBPS - conRateBPS;
			numMTV++;
		} else if (numBQN - conRateBQN >= pathResources + 1) {
			convertStudents(g, STUDENT_BQN, STUDENT_MTV, 1);
			numBQN = numBQN - conRateBQN;
			numMTV++;
		} else if (numMJ - conRateMJ >= 1) {
			convertStudents(g, STUDENT_MJ, STUDENT_MTV, 1);
			numMJ = numMJ - conRateMJ;
			numMTV++;
		}
	}

	if (numMJ < 1) {
		if (numBPS - conRateBPS >= pathResources + 1) {
			convertStudents(g, STUDENT_BPS, STUDENT_MJ, 1);
			numBPS = numBPS - conRateBPS;
			numMJ++;
		}
		else if (numBQN - conRateBQN >= pathResources + 1) {
			convertStudents(g, STUDENT_BQN, STUDENT_MJ, 1);
			numBQN = numBQN - conRateBQN;
			numMJ++;
		}
		else if (numMTV - conRateMTV >= 1) {
			convertStudents(g, STUDENT_MTV, STUDENT_MJ, 1);
			numMTV = numMTV - conRateMTV;
			numMJ++;
		}
	}

	if (numBQN <= pathResources + 1) {
		int neededBQN = pathResources + 1 - numBQN;
		if (numBPS - (conRateBPS * (pathResources + 1)) >= pathResources + 1) {
			convertStudents(g, STUDENT_BPS, STUDENT_BQN, neededBQN);
			numBPS = numBPS - (conRateBPS * (pathResources + 1));
			numBQN += neededBQN;
		}
		else if (numMJ - (conRateMJ * (pathResources + 1)) >= 1) {
			convertStudents(g, STUDENT_MJ, STUDENT_BQN, neededBQN);
			numMJ = numMJ - (conRateBQN * (pathResources + 1));
			numBQN += neededBQN;
		}
		else if (numMTV - (conRateMTV * (pathResources + 1)) >= 1) {
			convertStudents(g, STUDENT_MTV, STUDENT_BQN, neededBQN);
			numMTV = numMTV - (conRateBQN * (pathResources + 1));
			numBQN += neededBQN;
		}
	}

	if (numBPS <= pathResources + 1) {
		int neededBPS = pathResources + 1 - numBPS;
		if (numBQN - (conRateBQN * (pathResources + 1)) >= pathResources + 1) {
			convertStudents(g, STUDENT_BQN, STUDENT_BPS, neededBPS);
			numBQN = numBQN - (conRateBQN * (pathResources + 1));
			numBPS += neededBPS;
		} else if (numMJ - (conRateBPS * (pathResources + 1)) >= 1) {
			convertStudents(g, STUDENT_BQN, STUDENT_BPS, neededBPS);
			numMJ = numMJ - (conRateMJ * (pathResources + 1));
			numBPS += neededBPS;
		} else if (numMTV - (conRateBPS * (pathResources + 1)) >= 1) {
			convertStudents(g, STUDENT_BQN, STUDENT_BPS, neededBPS);
			numMTV = numMTV - (conRateMTV * (pathResources + 1));
			numBPS += neededBPS;
		}
	}

	if (getStudents(g, playerId, STUDENT_BPS) >= pathResources + 1 &&
		getStudents(g, playerId, STUDENT_BQN) >= pathResources + 1 &&
		getStudents(g, playerId, STUDENT_MJ) >= 1 &&
		getStudents(g, playerId, STUDENT_MTV) >= 1) {
		result = TRUE;
	}

	return result;
}


int enoughToBuildPath(Game g, int playerId) {
	int result = FALSE;

	if (getStudents(g, playerId, STUDENT_BPS) >= 1 &&
		getStudents(g, playerId, STUDENT_BQN) >= 1) {
		result = TRUE;
	}

	return result;
}


action decideAction(Game g) {
	action nextAction;
	nextAction.actionCode = PASS;

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
		myARC = ARC_B;
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

	printf("Populated database\n");

	// If we have more than 5 campuses, plan for building GO8s
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
			// printf("%d\n", i);
		}
		i++;
	}

	printf("Found our vertices\n");

	// Now scan through each vertex and store neighbours

	// Array of vertices that are accessible and aren't owned
	fromToArc considerations[NUM_INT_VERTICES][2];

	i = 0;
	int numConsiderations = 0;

	while (i < numMyVertices) {
		trio neighbouring = getNeighbouringVertices(myVertices[i]);

		if (neighbouring.a >= 0 && vertices[neighbouring.a].object == VACANT_VERTEX) {
			int arcId = getArcIdFromVertices(myVertices[i], neighbouring.a);

			if (arcs[arcId].object == VACANT_ARC || arcs[arcId].object == myARC) {
				considerations[numConsiderations][0].from = myVertices[i];
				considerations[numConsiderations][0].to = neighbouring.a;
				considerations[numConsiderations][0].valid = TRUE;

				if (arcs[arcId].object == myARC) {
					considerations[numConsiderations][0].alreadyOwned = TRUE;
				} else {
					considerations[numConsiderations][0].alreadyOwned = FALSE;
				}

				numConsiderations++;
			}
		}
		if (neighbouring.b >= 0 && vertices[neighbouring.b].object == VACANT_VERTEX) {
			int arcId = getArcIdFromVertices(myVertices[i], neighbouring.b);

			if (arcs[arcId].object == VACANT_ARC || arcs[arcId].object == myARC) {
				considerations[numConsiderations][0].from = myVertices[i];
				considerations[numConsiderations][0].to = neighbouring.b;
				considerations[numConsiderations][0].valid = TRUE;

				if (arcs[arcId].object == myARC) {
					considerations[numConsiderations][0].alreadyOwned = TRUE;
				} else {
					considerations[numConsiderations][0].alreadyOwned = FALSE;
				}

				numConsiderations++;
			}
		}
		if (neighbouring.c >= 0 && vertices[neighbouring.c].object == VACANT_VERTEX) {
			int arcId = getArcIdFromVertices(myVertices[i], neighbouring.c);

			if (arcs[arcId].object == VACANT_ARC || arcs[arcId].object == myARC) {
				considerations[numConsiderations][0].from = myVertices[i];
				considerations[numConsiderations][0].to = neighbouring.c;

				if (arcs[arcId].object == myARC) {
					considerations[numConsiderations][0].alreadyOwned = TRUE;
				} else {
					considerations[numConsiderations][0].alreadyOwned = FALSE;
				}

				numConsiderations++;
			}
		}

		i++;
	}

	i = 0;

	while (i < numConsiderations) {
		trio neighbouring = getNeighbouringVertices(considerations[i][0].to);
		int matched = FALSE;

		if (neighbouring.a >= 0 && vertices[neighbouring.a].object == VACANT_VERTEX) {
			matched = TRUE;
			// Check that it's not within any other verticie
			if (canBuildCampusOn(vertices, neighbouring.a)) {
				int arcId = getArcIdFromVertices(considerations[i][0].to, neighbouring.a);

				if (arcs[arcId].object == VACANT_ARC || arcs[arcId].object == myARC) {
					considerations[i][1].from = considerations[i][0].to;
					considerations[i][1].to = neighbouring.a;

					if (arcs[arcId].object == myARC) {
						considerations[i][0].alreadyOwned = TRUE;
					} else {
						considerations[i][0].alreadyOwned = FALSE;
					}
				}
			} else {
				considerations[i][0].valid = FALSE;
			}
		}

		if (neighbouring.b >= 0 && vertices[neighbouring.b].object == VACANT_VERTEX) {
			matched = TRUE;
			// Check that it's not within any other verticie
			if (canBuildCampusOn(vertices, neighbouring.b)) {
				int arcId = getArcIdFromVertices(considerations[i][0].to, neighbouring.b);

				if (arcs[arcId].object == VACANT_ARC || arcs[arcId].object == myARC) {
					considerations[i][1].from = considerations[i][0].to;
					considerations[i][1].to = neighbouring.b;

					if (arcs[arcId].object == myARC) {
						considerations[i][0].alreadyOwned = TRUE;
					} else {
						considerations[i][0].alreadyOwned = FALSE;
					}
				}
			} else {
				considerations[i][0].valid = FALSE;
			}
		}

		if (neighbouring.c >= 0 && vertices[neighbouring.c].object == VACANT_VERTEX) {
			matched = TRUE;
			// Check that it's not within any other verticie
			if (canBuildCampusOn(vertices, neighbouring.c)) {
				int arcId = getArcIdFromVertices(considerations[i][0].to, neighbouring.c);

				if (arcs[arcId].object == VACANT_ARC || arcs[arcId].object == myARC) {
					considerations[i][1].from = considerations[i][0].to;
					considerations[i][1].to = neighbouring.c;

					if (arcs[arcId].object == myARC) {
						considerations[i][0].alreadyOwned = TRUE;
					} else {
						considerations[i][0].alreadyOwned = FALSE;
					}
				}
			} else {
				considerations[i][0].valid = FALSE;
			}
		}

		if (!matched) {
			considerations[i][0].valid = FALSE;
		}

		i++;
	}

	printf("Determined considerations\n");

	// Remove duplicate paths to the same vertex

	i = 0;
	int numPossibilities = 0;
	fromToArc possibilities[NUM_INT_VERTICES][2];

	while (i < numConsiderations) {
		// First result dominates others, unless it has an alreadyOwned flag
		// printf("%d: %d\n", i, considerations[i][1].to);

		if (considerations[i][0].valid) {
			int search = considerations[i][1].to;

			int j = 0;
			int removeAll = FALSE;

			possibilities[numPossibilities][0] = considerations[i][0];
			possibilities[numPossibilities][1] = considerations[i][1];

			while (j < numConsiderations) {
				if (considerations[j][0].valid) {
					if (considerations[j][1].to == search) {
						// Same
						if (!removeAll) {
							if (considerations[j][0].alreadyOwned ||
							considerations[j][1].alreadyOwned) {
								// Better option
								removeAll = TRUE;
								considerations[j][0].valid = FALSE;

								possibilities[numPossibilities][0] = considerations[j][0];
								possibilities[numPossibilities][1] = considerations[j][1];
								numPossibilities++;
							}
						}
						considerations[j][0].valid = FALSE;
					}
				}

				j++;
			}

			if (!removeAll) {
				numPossibilities++;
			}
		}

		i++;
	}

	// Now get the weight values of each consideration


	weightedVertex sortedWeights[1];

	i = 0;
	while (i < numPossibilities) {
		int weight = getRecursiveVertexWeight(vertices, myVertices, numMyVertices,
			possibilities[i][1].to);
		weightedVertex newVertex;
		newVertex.weight = weight;
		newVertex.arcPath[0] = possibilities[i][0];
		newVertex.arcPath[1] = possibilities[i][1];

		sortedWeights[i] = newVertex;

		i++;
	}

	sortWeights(sortedWeights, numPossibilities);

	printf("Weighted considerations\n");

	// Buy in order of weighting

	int attempt = 0;

	while (attempt < numPossibilities &&
		enoughToBuildCampus(g, currentPlayer, sortedWeights[attempt].arcPath)) {
		int firstArc = getArcIdFromVertices(sortedWeights[attempt].arcPath[0].from,
			sortedWeights[attempt].arcPath[0].to);

		if (firstArc < 0) {
			printf("Could not find path between %d and %d!\n",
				sortedWeights[attempt].arcPath[0].from,
				sortedWeights[attempt].arcPath[0].to);
		} else {
			if (!sortedWeights[attempt].arcPath[0].alreadyOwned) {
				action pathAction;

				pathAction.actionCode = OBTAIN_ARC;
				strcpy(pathAction.destination, arcs[firstArc].path);

				if (!isLegalAction(g, pathAction)) {
					printf("ARC between %d and %d is not legal?\n",
						sortedWeights[attempt].arcPath[0].from,
						sortedWeights[attempt].arcPath[0].to);
				} else {
					printf("Bought ARC between %d and %d\n",
					sortedWeights[attempt].arcPath[0].from,
					sortedWeights[attempt].arcPath[0].to);
					makeAction(g, pathAction);
				}
			}

			int secondArc = getArcIdFromVertices(sortedWeights[attempt].arcPath[1].from,
				sortedWeights[attempt].arcPath[1].to);

			if (secondArc < 0) {
				printf("Could not find path between %d and %d!\n",
					sortedWeights[attempt].arcPath[1].from,
					sortedWeights[attempt].arcPath[1].to);
			} else {
				if (!sortedWeights[attempt].arcPath[1].alreadyOwned) {
					action pathAction;

					pathAction.actionCode = OBTAIN_ARC;
					strcpy(pathAction.destination, arcs[secondArc].path);

					if (!isLegalAction(g, pathAction)) {
						printf("ARC between %d and %d is not legal?\n",
							sortedWeights[attempt].arcPath[1].from,
							sortedWeights[attempt].arcPath[1].to);
					} else {
						printf("Bought ARC between %d and %d\n",
						sortedWeights[attempt].arcPath[1].from,
						sortedWeights[attempt].arcPath[1].to);
						makeAction(g, pathAction);
					}
				}

				int vertexId = sortedWeights[attempt].arcPath[1].to;

				action campusAction;

				strcpy(campusAction.destination, vertices[vertexId].path);
				campusAction.actionCode = BUILD_CAMPUS;

				if (!isLegalAction(g, campusAction)) {
					printf("Campus at %d is not legal?\n", vertexId);
				} else {
					printf("Built campus at %d\n", vertexId);
					makeAction(g, campusAction);
				}
			}
		}

		attempt++;
	}

	printf("Campuses purchased\n");

	printf("I have:\n");
	printf("BPS: %d\n", getStudents(g, currentPlayer, STUDENT_BPS));
	printf("BQN: %d\n", getStudents(g, currentPlayer, STUDENT_BQN));
	printf("MJ: %d\n", getStudents(g, currentPlayer, STUDENT_MJ));
	printf("MTV: %d\n", getStudents(g, currentPlayer, STUDENT_MTV));
	printf("Points: %d\n", getKPIpoints(g, currentPlayer));


	// If there are any 3+ left over resources, start spin-offs. Check if legal.

	// If there is an outlier resource with 7+ left over, exchange them to
	// resource we have least have excluding THDs. Check if legal.

	return nextAction;
}

int main() {
	int defaultDis[] = DEFAULT_DISCIPLINES;
	int defaultDice[] = DEFAULT_DICE;

	srand(time(NULL));

	Game thisGame = newGame(defaultDis, defaultDice);

	action passAction;
	passAction.actionCode = PASS;

	int i = 0;
	while (i < 1000) {
		int r = rand();
		// throwDice(thisGame, (r % 11) + 2);
		throwDice(thisGame, 9);
		makeAction(thisGame, passAction);
		throwDice(thisGame, 9);
		printf("\n------------------------------------\n");
		printf("Turn #%d\n", i++);
		printf("This is %d's turn\n", getWhoseTurn(thisGame));
		action thisAction = decideAction(thisGame);
		throwDice(thisGame, 9);
		makeAction(thisGame, passAction);
	}
}
