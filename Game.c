
//
//  Game
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Game.h"


#define NUM_VERTICES 54
#define NUM_ARCS 72

#define START_OF_REGIONS {0, 2, 4, 7, 9, 11, 13, 16, 18, 20, 22, 24, 28, 30, \
	32, 34, 39, 41, 43}


typedef struct {
	int position;
	int owner;
	int isGO8;
} Campus;


typedef struct {
	int start;
	int end;
} ArcPosition;


typedef struct {
	ArcPosition position;
	int owner;
} Arc;


typedef struct {
	int disciplines[6];
	int ips;
	int publications;
} Player;


struct _game {
	int *regionDice;
	int *regionDisciplines;
	int turnNumber;

	Campus campuses[NUM_VERTICES];
	int campusCount;

	Arc arcs[NUM_ARCS];
	int arcCount;

	// Add 1 so we don't have to bother subtracting 1 off the
	// uni ID when we're passed it as an argument, cause we're
	// lazy and want to prevent bugs.
	Player players[NUM_UNIS + 1];
};


// Prototypes
void buildCampus(Game g, int position, int owner);



//
//  Initialisation
//

// Creates a new game object
Game newGame(int discipline[], int dice[]) {
	Game g = malloc(sizeof(struct _game));

	g->regionDisciplines = discipline;
	g->regionDice = dice;
	g->turnNumber = -1;
	g->arcCount = 0;
	g->campusCount = 0;

	buildCampus(g, 16, UNI_A);
	buildCampus(g, 37, UNI_A);
	buildCampus(g, 1, UNI_B);
	buildCampus(g, 52, UNI_B);
	buildCampus(g, 47, UNI_C);
	buildCampus(g, 6, UNI_C);

	for (int i = 1; i <= NUM_UNIS; i++) {
		Player *player = &g->players[i];
		player->ips = 0;
		player->publications = 0;

		player->disciplines[STUDENT_THD] = 0;
		player->disciplines[STUDENT_BPS] = 3;
		player->disciplines[STUDENT_BQN] = 3;
		player->disciplines[STUDENT_MJ] = 1;
		player->disciplines[STUDENT_MTV] = 1;
		player->disciplines[STUDENT_MMONEY] = 1;
	}

	return g;
}


// Disposes of the game's resources
void disposeGame(Game g) {
	free(g);
}



//
//  Path System
//

// Finds the position behind a vertex, given where the vertex
// was previously seen from.
int back(int current, int previous) {
	return previous;
}


// Returns the column number for the given position.
int vertexColumn(int position) {
	int column = -1;

	if (position >= 0 && position <= 6) {
		column = 0;
	} else if (position >= 7 && position <= 15) {
		column = 1;
	} else if (position >= 16 && position <= 26) {
		column = 2;
	} else if (position >= 27 && position <= 37) {
		column = 3;
	} else if (position >= 38 && position <= 46) {
		column = 4;
	} else if (position >= 47 && position <= 53) {
		column = 5;
	}

	return column;
}


// Returns true if the vertex at the given position has a vertex
// to the right of it, in absolute coordinates.
int hasRight(int position) {
	int result = FALSE;
	int column = vertexColumn(position);

	if (column == 0 || column == 2 || column == 3 || column == 5) {
		if (position % 2 == 0) {
			result = TRUE;
		}
	} else if (column == 1 || column == 4) {
		if (position % 2 == 1) {
			result = TRUE;
		}
	}

	return result;
}


// Returns the vertex to the right of the given vertex in
// absolute coordinates.
int absoluteRight(int vertex) {
	int modifier = 0;
	int column = vertexColumn(vertex);

	if (column == 0) {
		modifier = 8;
	} else if (column == 1) {
		modifier = 10;
	} else if (column == 2) {
		modifier = 11;
	} else if (column == 3) {
		modifier = 10;
	} else if (column == 4) {
		modifier = 8;
	} else if (column == 5) {
		return -1;
	}

	return vertex + modifier;
}


// Returns the vertex to the left of the given vertex in
// absolute coordinates.
int absoluteLeft(int vertex) {
	int modifier = 0;
	int column = vertexColumn(vertex);

	if (column == 0) {
		return -1;
	} else if (column == 1) {
		modifier = 8;
	} else if (column == 2) {
		modifier = 10;
	} else if (column == 3) {
		modifier = 11;
	} else if (column == 4) {
		modifier = 10;
	} else if (column == 5) {
		modifier = 8;
	}

	return vertex - modifier;
}


// Returns true if the two vertices are in the same column.
int inSameColumn(int one, int two) {
	return vertexColumn(one) == vertexColumn(two);
}


// Find the position to the left of a vertex, given where the
// vertex was seen from.
int left(int current, int previous) {
	int result = -1;

	if (previous - current == 1) {
		// We're facing upwards
		if (hasRight(current) && inSameColumn(current, current - 1)) {
			// Our choices are up and to the right (in absolute
			// coordinates), so move up
			result = current - 1;
		} else {
			// Our choices are (in absolute coordinates) moving
			// up or to the left, so move left
			result = absoluteLeft(current);
		}
	} else if (previous - current == -1) {
		// We're facing down
		if (hasRight(current)) {
			// Can go right or down
			result = absoluteRight(current);
		} else if (inSameColumn(current, current + 1)) {
			// Can go left or down
			result = current + 1;
		}
	} else if (previous < current && inSameColumn(current, current - 1)) {
		// Facing right
		result = current - 1;
	} else if (previous > current && inSameColumn(current, current + 1)) {
		// Facing left
		result = current + 1;
	}

	return result;
}


// Find the position to the right of a vertex, given where the
// vertex was seen from.
int right(int current, int previous) {
	int result = -1;

	if (previous - current == 1) {
		// We're facing upwards
		if (hasRight(current)) {
			// Our choices are (in absolute coordinates) moving
			// up or to the right, so move right
			result = absoluteRight(current);
		} else if (inSameColumn(current, current - 1)) {
			// Our choices are up and to the left (in absolute
			// coordinates), so move up
			result = current - 1;
		}
	} else if (previous - current == -1) {
		// We're facing down
		if (hasRight(current) && inSameColumn(current, current + 1)) {
			// Can go right or down
			result = current + 1;
		} else {
			// Can go left or down
			result = absoluteLeft(current);
		}
	} else if (previous < current && inSameColumn(current, current + 1)) {
		// Facing right
		result = current + 1;
	} else if (previous > current && inSameColumn(current, current - 1)) {
		// Facing left
		result = current - 1;
	}

	return result;
}


// Converts a path into a position.
int pathToPosition(path p) {
	int length = strlen(p);

	if (length == 0) {
		return 16;
	}

	int previous = 16;
	int current = -1;

	if (p[0] == 'L') {
		current = 27;
	} else if (p[0] == 'R') {
		current = 17;
	}

	int i = 1;
	while (i < length && current >= 0 && current <= NUM_VERTICES) {
		int saved = current;
		if (p[i] == 'L') {
			current = left(current, previous);
		} else if (p[i] == 'R') {
			current = right(current, previous);
		} else if (p[i] == 'B') {
			current = back(current, previous);
		}
		previous = saved;
		i++;
	}

	return current;
}


// Converts a path into a start and ending position for an arc.
ArcPosition pathToArcPosition(path p) {
	// Copy the path so when we transform it by adding a null
	// terminator, we don't apply a side effect to the argument
	path copy;
	strcpy(copy, p);

	int length = strlen(copy);
	ArcPosition result;
	result.end = pathToPosition(copy);
	copy[length - 1] = '\0';
	result.start = pathToPosition(copy);
	return result;
}



//
//  Campuses and Arcs
//

// Returns NULL if campus doesn't exist.
Campus * campusAt(Game g, int position) {
	for (int i = 0; i < g->campusCount; i++) {
		Campus *campus = &g->campuses[i];
		if (campus->position == position) {
			return campus;
		}
	}

	return NULL;
}


// Wrapper around `campusAt` to return the owner.
int campusOwnerAt(Game g, int position) {
	Campus *campus = campusAt(g, position);
	if (campus == NULL) {
		return NO_ONE;
	} else {
		return campus->owner;
	}
}


// Returns NULL if no arc exists.
Arc * arcAt(Game g, ArcPosition position) {
	for (int i = 0; i < g->arcCount; i++) {
		Arc *arc = &g->arcs[i];
		if (arc->position.start == position.start &&
				arc->position.end == position.end) {
			return arc;
		}
	}

	return NULL;
}


// Returns the number of campuses around the tile.
//
// Usage:
//   int campuses[6];
//   campusesAdjacentToRegion(g, position, campuses);
int campusesAdjacentToRegion(Game g, int region, Campus *campuses[6]) {
	int starts[] = START_OF_REGIONS;
	int start = starts[region];
	int right = absoluteRight(start);
	int vertices[] = {start, start + 1, start + 2, right, right + 1, right + 2};

	int count = 0;
	for (int i = 0; i < 6; i++) {
		Campus *campus = campusAt(g, vertices[i]);
		if (campus != NULL) {
			campuses[count] = campus;
			count++;
		}
	}

	return count;
}


// Calculates the up to 3 adjacent vertices to a vertex,
// returning the number of vertices.
int adjacentVertices(int vertex, int results[3]) {
	int count = 0;

	if (inSameColumn(vertex, vertex - 1)) {
		results[count++] = vertex - 1;
	}

	if (inSameColumn(vertex, vertex + 1)) {
		results[count++] = vertex + 1;
	}

	if (hasRight(vertex)) {
		results[count++] = absoluteRight(vertex);
	} else {
		results[count++] = absoluteLeft(vertex);
	}

	return count;
}



//
//  Actions
//

void reduceResources(Game g, int playerID, int thd, int bps, int bqn, int mj,
		int mtv, int mmoney) {
	Player *player = &g->players[playerID];
	player->disciplines[STUDENT_THD] -= thd;
	player->disciplines[STUDENT_BPS] -= bps;
	player->disciplines[STUDENT_BQN] -= bqn;
	player->disciplines[STUDENT_MJ] -= mj;
	player->disciplines[STUDENT_MTV] -= mtv;
	player->disciplines[STUDENT_MMONEY] -= mmoney;
}


// Builds a campus owned by `owner` at `position`.
void buildCampus(Game g, int position, int owner) {
	Campus *campus = &g->campuses[g->campusCount];
	g->campusCount++;

	campus->position = position;
	campus->owner = owner;
	campus->isGO8 = FALSE;
	reduceResources(g, owner, 0, 1, 1, 1, 1, 0);
}


// Upgrades the campus at `position` to a GO8 campus.
void buildGO8(Game g, int position, int owner) {
	g->campuses[position].isGO8 = TRUE;
	reduceResources(g, owner, 0, 0, 0, 2, 0, 3);
}


// Builds `arc`.
void buildArc(Game g, ArcPosition position, int owner) {
	Arc *arc = &g->arcs[g->arcCount];
	g->arcCount++;

	arc->position = position;
	arc->owner = owner;
	reduceResources(g, owner, 0, 1, 1, 0, 0, 0);
}


// Obtains a publication.
void obtainPublication(Game g, int player) {
	g->players[player].publications++;
	reduceResources(g, player, 0, 0, 0, 1, 1, 1);
}


// Obtains an IP patent.
void obtainIPPatent(Game g, int player) {
	g->players[player].ips++;
	reduceResources(g, player, 0, 0, 0, 1, 1, 1);
}


// Retrains students from the discipline `from` to the
// discipline `to`.
void retrainStudents(Game g, int player, int from, int to) {
	int rate = getExchangeRate(g, player, from, to);
	g->players[player].disciplines[from] -= rate;
	g->players[player].disciplines[to]++;
}


// make the specified action for the current player and update the
// game state accordingly.
// The function may assume that the action requested is legal.
// START_SPINOFF is not a legal action here
void makeAction(Game g, action a) {
	if (a.actionCode == PASS) {
		// Nothing
	} else if (a.actionCode == BUILD_CAMPUS) {
		buildCampus(g, pathToPosition(a.destination), getWhoseTurn(g));
	} else if (a.actionCode == BUILD_GO8) {
		buildGO8(g, pathToPosition(a.destination), getWhoseTurn(g));
	} else if (a.actionCode == OBTAIN_ARC) {
		buildArc(g, pathToArcPosition(a.destination), getWhoseTurn(g));
	} else if (a.actionCode == OBTAIN_PUBLICATION) {
		obtainPublication(g, getWhoseTurn(g));
	} else if (a.actionCode == OBTAIN_IP_PATENT) {
		obtainIPPatent(g, getWhoseTurn(g));
	} else if (a.actionCode == RETRAIN_STUDENTS) {
		retrainStudents(g, getWhoseTurn(g), a.disciplineFrom, a.disciplineTo);
	}
}



//
//  Dice
//

// advance the game to the next turn,
// assuming that the dice has just been rolled and produced diceScore
// the game starts in turn -1 (we call this state "Terra Nullis") and
// moves to turn 0 as soon as the first dice is thrown.
void throwDice(Game g, int dice) {
	g->turnNumber++;

	// Produce students for each region
	for (int region = 0; region < NUM_REGIONS; region++) {
		if (g->regionDice[region] != dice) {
			continue;
		}

		int studentType = getDiscipline(g, region);

		Campus *campuses[6];
		int count = campusesAdjacentToRegion(g, region, campuses);

		for (int i = 0; i < count; i++) {
			int owner = campuses[i]->owner;
			int amount = 1;
			if (campuses[i]->isGO8) {
				amount = 2;
			}

			g->players[owner].disciplines[studentType] += amount;
		}
	}

	// Convert all MTV and MMONEY students to THD
	if (dice == 7) {
		for (int i = 1; i <= NUM_UNIS; i++) {
			int mtv = g->players[i].disciplines[STUDENT_MTV];
			int mmoney = g->players[i].disciplines[STUDENT_MMONEY];
			g->players[i].disciplines[STUDENT_THD] += mtv + mmoney;

			g->players[i].disciplines[STUDENT_MTV] = 0;
			g->players[i].disciplines[STUDENT_MMONEY] = 0;
		}
	}
}



//
//  Getters
//

// what type of students are produced by the specified region?
// regionID is the index of the region in the newGame arrays (above)
// see discipline codes above
int getDiscipline(Game g, int regionID) {
	return g->regionDisciplines[regionID];
}


// what dice value produces students in the specified region?
// 2..12
int getDiceValue(Game g, int regionID) {
	return g->regionDice[regionID];
}


// which university currently has the prestige award for the most ARCs?
// this is NO_ONE until the first arc is purchased after the game
// has started.
int getMostARCs(Game g) {
	int mostOwner = NO_ONE;

	if (g->turnNumber > -1) {
		int most = 0;
		for (int i = 1; i <= NUM_UNIS; i++) {
			int arcs = getARCs(g, i);
			if (arcs > most) {
				most = arcs;
				mostOwner = i;
			}
		}

		// Check not more than one person has this number of
		// arcs
		for (int i = 1; i <= NUM_UNIS; i++) {
			int arcs = getARCs(g, i);
			if (arcs == most && i != mostOwner) {
				mostOwner = NO_ONE;
			}
		}
	}

	return mostOwner;
}


// which university currently has the prestige award for the most pubs?
// this is NO_ONE until the first publication is made.
int getMostPublications(Game g) {
	int mostOwner = NO_ONE;

	if (g->turnNumber > -1) {
		int most = 0;
		for (int i = 1; i <= NUM_UNIS; i++) {
			int publications = g->players[i].publications;
			if (publications > most) {
				most = publications;
				mostOwner = i;
			}
		}

		// Check there's only 1 person with this number of
		// publications
		for (int i = 1; i <= NUM_UNIS; i++) {
			int publications = g->players[i].publications;
			if (publications == most) {
				// More than one person has it
				mostOwner = NO_ONE;
			}
		}
	}

	return mostOwner;
}


// return the current turn number of the game -1, 0, 1, ...
int getTurnNumber(Game g) {
	return g->turnNumber;
}


// return the player id of the player whose turn it is
// the result of this function is NO_ONE during Terra Nullis
int getWhoseTurn(Game g) {
	int player = NO_ONE;
	if (g->turnNumber != -1) {
		player = (g->turnNumber % NUM_UNIS) + 1;
	}

	return player;
}


// return the contents of the given vertex (ie campus code or
// VACANT_VERTEX)
int getCampus(Game g, path pathToVertex) {
	int position = pathToPosition(pathToVertex);
	if (position == -1) {
		return NO_ONE;
	}

	Campus *campus = campusAt(g, position);
	if (campus == NULL) {
		return NO_ONE;
	}

	if (campus->isGO8) {
		return campus->owner + 3;
	}

	return campus->owner;
}


// the contents of the given edge (ie ARC code or vacent ARC)
int getARC(Game g, path pathToEdge) {
	ArcPosition position = pathToArcPosition(pathToEdge);
	if (position.start == -1) {
		return VACANT_ARC;
	}

	Arc *arc = arcAt(g, position);
	if (arc == NULL) {
		return VACANT_ARC;
	}

	return arc->owner;
}



//
//  Legality
//

int sufficientResources(Game g, int playerID, int thd, int bps, int bqn,
		int mj, int mtv, int mmoney) {
	Player *player = &g->players[playerID];
	return player->disciplines[STUDENT_THD] >= thd &&
		player->disciplines[STUDENT_BPS] >= bps &&
		player->disciplines[STUDENT_BQN] >= bqn &&
		player->disciplines[STUDENT_MJ] >= mj &&
		player->disciplines[STUDENT_MTV] >= mtv &&
		player->disciplines[STUDENT_MMONEY] >= mmoney;
}


int isBuildCampusLegal(Game g, int position, int player) {
	if (!sufficientResources(g, player, 0, 1, 1, 1, 1, 0)) {
		return FALSE;
	}

	// Check a campus at that location doesn't already exist
	if (campusOwnerAt(g, position) != NO_ONE) {
		return FALSE;
	}

	// Check is adjacent to arc
	int isNextToArc = FALSE;
	for (int i = 0; i < g->arcCount; i++) {
		Arc *arc = &g->arcs[i];
		if (arc->position.start == position || arc->position.end == position) {
			isNextToArc = TRUE;
			break;
		}
	}

	if (!isNextToArc) {
		return FALSE;
	}

	// Check isn't adjacent to another campus
	int adjacent[3];
	int count = adjacentVertices(position, adjacent);
	for (int i = 0; i < count; i++) {
		int vertex = adjacent[i];
		if (campusAt(g, vertex) != NULL) {
			return FALSE;
		}
	}

	return TRUE;
}


int isBuildGO8Legal(Game g, int position, int player) {
	// Check there are sufficient resources
	if (!sufficientResources(g, player, 0, 0, 0, 2, 0, 3)) {
		return FALSE;
	}

	// Check there are no more than 8 GO8 campuses
	int total = 0;
	for (int i = 0; i < g->campusCount; i++) {
		if (g->campuses[i].isGO8) {
			total++;
		}
	}

	if (total >= 8) {
		return FALSE;
	}

	// Check a campus the player owns exists at the position
	if (campusOwnerAt(g, position) != player) {
		return FALSE;
	}

	return TRUE;
}


int isBuildArcLegal(Game g, ArcPosition position, int player) {
	if (!sufficientResources(g, player, 0, 1, 1, 0, 0, 0)) {
		return FALSE;
	}

	// Check another arc doesn't already exist
	if (arcAt(g, position) != NULL) {
		return FALSE;
	}

	// Check connects to another arc
	int found = FALSE;
	for (int i = 0; i < g->arcCount; i++) {
		Arc *arc = &g->arcs[i];
		if (arc->owner == player && (arc->position.start == position.start ||
				arc->position.start == position.end ||
				arc->position.end == position.start ||
				arc->position.end == position.end)) {
			found = TRUE;
			break;
		}
	}

	// Check connects to another campus
	if (!found) {
		found = campusOwnerAt(g, position.start) == player ||
			campusOwnerAt(g, position.end) == player;
	}

	return found;
}


int isStartSpinoffLegal(Game g, int player) {
	if (!sufficientResources(g, player, 0, 0, 0, 1, 1, 1)) {
		return FALSE;
	}

	return TRUE;
}


int isRetrainStudentsLegal(Game g, int player, int from, int to) {
	int rate = getExchangeRate(g, player, from, to);

	if (g->players[player].disciplines[from] < rate) {
		return FALSE;
	}

	return TRUE;
}


// returns TRUE if it is legal for the current
// player to make the specified action, FALSE otherwise.
//
// "legal" means everything is legal:
//   * that the action code is a valid action code which is legal to
//     be made at this time
//   * that any path is well formed and legal ie consisting only of
//     the legal direction characters and of a legal length,
//     and which does not leave the island into the sea at any stage.
//   * that disciplines mentioned in any retraining actions are valid
//     discipline numbers, and that the university has sufficient
//     students of the correct type to perform the retraining
//
// eg when placing a campus consider such things as:
//   * is the path a well formed legal path
//   * does it lead to a vacent vertex?
//   * under the rules of the game are they allowed to place a
//     campus at that vertex? (eg is it adjacent to one of their ARCs?)
//   * does the player have the 4 specific students required to pay for
//     that campus?
// It is not legal to make any action during Terra Nullis ie
// before the game has started.
// It is not legal for a player to make the moves OBTAIN_PUBLICATION
// or OBTAIN_IP_PATENT(they can make the move START_SPINOFF)
// you can assume that any pths passed in are NULL terminated strings.
int isLegalAction(Game g, action a) {
	if (a.actionCode == PASS) {
		return TRUE;
	} else if (a.actionCode == BUILD_CAMPUS) {
		return isBuildCampusLegal(g, pathToPosition(a.destination), getWhoseTurn(g));
	} else if (a.actionCode == BUILD_GO8) {
		return isBuildGO8Legal(g, pathToPosition(a.destination), getWhoseTurn(g));
	} else if (a.actionCode == OBTAIN_ARC) {
		return isBuildArcLegal(g, pathToArcPosition(a.destination), getWhoseTurn(g));
	} else if (a.actionCode == START_SPINOFF) {
		return isStartSpinoffLegal(g, getWhoseTurn(g));
	} else if (a.actionCode == RETRAIN_STUDENTS) {
		return isRetrainStudentsLegal(g, getWhoseTurn(g), a.disciplineFrom,
			a.disciplineTo);
	} else {
		return FALSE;
	}
}



//
//  Player Data
//

// return the number of KPI points the specified player currently has
int getKPIpoints(Game g, int player) {
	int kpi = 0;

	kpi += getARCs(g, player) * 2;
	kpi += getGO8s(g, player) * 20;
	kpi += getCampuses(g, player) * 10;
	kpi += getIPs(g, player) * 10;

	if (getMostPublications(g) == player) {
		kpi += 10;
	}

	if (getMostARCs(g) == player) {
		kpi += 10;
	}

	return kpi;
}


// return the number of ARC grants the specified player currently has
int getARCs(Game g, int player) {
	int total = 0;
	for (int i = 0; i < g->arcCount; i++) {
		if (g->arcs[i].owner == player) {
			total++;
		}
	}

	return total;
}


// return the number of GO8 campuses the specified player currently has
int getGO8s(Game g, int player) {
	int total = 0;
	for (int i = 0; i < g->campusCount; i++) {
		if (g->campuses[i].isGO8 == TRUE && g->campuses[i].owner == player) {
			total++;
		}
	}

	return total;
}


// return the number of normal Campuses the specified player currently has
int getCampuses(Game g, int player) {
	int total = 0;
	for (int i = 0; i < g->campusCount; i++) {
		if (g->campuses[i].isGO8 == FALSE && g->campuses[i].owner == player) {
			total++;
		}
	}

	return total;
}


// return the number of IP Patents the specified player currently has
int getIPs(Game g, int player) {
	return g->players[player].ips;
}


// return the number of Publications the specified player currently has
int getPublications(Game g, int player) {
	return g->players[player].publications;
}


// return the number of students of the specified discipline type
// the specified player currently has
int getStudents(Game g, int player, int discipline) {
	return g->players[player].disciplines[discipline];
}


// return how many students of discipline type disciplineFrom
// the specified player would need to retrain in order to get one
// student of discipline type disciplineTo. This will depend
// on what retraining centers, if any, they have a campus at.
int getExchangeRate(Game g, int player, int disciplineFrom, int disciplineTo) {
	// Can't retrain THD students
	if (disciplineFrom == STUDENT_THD) {
		return 0;
	}

	// Check for a campus on each individual student position
	if (disciplineFrom == STUDENT_MTV) {
		if (campusOwnerAt(g, 7) == player || campusOwnerAt(g, 17) == player) {
			return 2;
		}
	} else if (disciplineFrom == STUDENT_MMONEY) {
		if (campusOwnerAt(g, 28) == player || campusOwnerAt(g, 38) == player) {
			return 2;
		}
	} else if (disciplineFrom == STUDENT_BPS) {
		if (campusOwnerAt(g, 14) == player || campusOwnerAt(g, 15) == player) {
			return 2;
		}
	} else if (disciplineFrom == STUDENT_MJ) {
		if (campusOwnerAt(g, 45) == player || campusOwnerAt(g, 46) == player) {
			return 2;
		}
	} else if (disciplineFrom == STUDENT_BQN) {
		if (campusOwnerAt(g, 50) == player || campusOwnerAt(g, 51) == player) {
			return 2;
		}
	}

	return 3;
}
