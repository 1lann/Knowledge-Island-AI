// mechanicalTurk.c
// Created by Albert Smith and Kohsuke Sato
// A file submission to "Mechanical Turk"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "albert.h"

#define ALL_PATHS {"RL", "RLR", "RLL", "RLRL", "RLRR", "RLLR",\
 "RLRRL", "RLRLR", "RLRLL", "RLLRL", "RLRRLL", "RLRLRL", "RLRLLR",\
 "RLLRLR", "RLLRLRR", "RLRLRLL", "RLRLRLR", "RLRRLLR", "RLLRLRRL",\
 "RLRLRLLR", "RLRLRLRL", "RLRLRLRR", "RLRLRLLRR", "RLLRLRRLR", "R",\
 "LR", "RRL", "RRLRLL", "LRLR", "LRLRLRR", "LRLRLRR", "RRLRLLRL",\
 "LRLRLRRLR", "LRLRRLRLRL", "RRLLRLRLR", "LRRLRLRLRL", "RLRLRLRLR", "",\
 "L", "LRL", "LRLRL", "LRLRLR", "LRLRLRRL", "LRLRLRRLRL",\
 "LRLRRLRLRLR", "LRRLRLRLRLR", "RLRLRLRLRL", "RRLLRLRLRL",\
 "RRLRLLRLRL", "RRLRLLRLR", "RRLRLLR", "RRLRL", "RRLR", "RR"}
#define NUM_PATHS 54

action decideActionB (Game g) {
    action testAction;
    action nextAction;

    char chosen = FALSE;
    int mostStudents;
    int leastStudents;

    // Mr Retrain - retrains students to even the numbers
    if (!chosen) {
        mostStudents = STUDENT_BPS;
        leastStudents = STUDENT_BPS;
        int i = STUDENT_BPS;
        while (i <= STUDENT_MMONEY) {
            if (getStudents(g, getWhoseTurn(g), i) <
                getStudents(g, getWhoseTurn(g), leastStudents)) {
                leastStudents = i;
            }
            if (getStudents(g, getWhoseTurn(g), i) >
                getStudents(g, getWhoseTurn(g), mostStudents)) {
                mostStudents = i;
            }
            i++;
        }
        testAction.actionCode = RETRAIN_STUDENTS;
        testAction.disciplineFrom = mostStudents;
        testAction.disciplineTo = leastStudents;
        if ((getStudents(g, getWhoseTurn(g), mostStudents) >
             getExchangeRate(g,getWhoseTurn(g),
                             mostStudents,leastStudents)) &&
            isLegalAction(g, testAction) &&
            getStudents(g, getWhoseTurn(g), leastStudents) == 0) {
            nextAction = testAction;
            chosen = TRUE;
        }
    }

    // Mr Spinoff
    if (!chosen) {
        testAction.actionCode = START_SPINOFF;
        if (isLegalAction(g, testAction)) {
            nextAction = testAction;
            chosen = TRUE;
        }
    }

    // Mr Campus
    if (!chosen) {
        path paths[] = ALL_PATHS;
        int i = 0;
        while ((i < NUM_PATHS) && !chosen) {
            testAction.actionCode = BUILD_CAMPUS;
            memcpy(testAction.destination, paths[i], sizeof(paths[i]));
            if (isLegalAction(g, testAction)) {
                nextAction = testAction;
                chosen = TRUE;
            }
            i++;
        }
    }

    // Mr ARC
    if (!chosen && getARCs(g, getWhoseTurn(g)) <
        getCampuses(g, getWhoseTurn(g)) + 10) {
        path arcs[] = ALL_PATHS;
        int i = 0;
        while ((i < NUM_PATHS) && !chosen) {
            testAction.actionCode = OBTAIN_ARC;
            memcpy(testAction.destination, arcs[i], sizeof(arcs[i]));
            if (isLegalAction(g, testAction)) {
                nextAction = testAction;
                chosen = TRUE;
            }
            i++;
        }
    }

    // Mr GO8
    if (!chosen) {
        if (getCampuses(g, getWhoseTurn(g)) > 0) {
            path paths[] = ALL_PATHS;
            int i = 0;
            while ((i < NUM_PATHS) & !chosen) {
                testAction.actionCode = BUILD_GO8;
                memcpy(testAction.destination, paths[i],
                       sizeof(paths[i]));
                if (isLegalAction(g, testAction)) {
                    nextAction = testAction;
                    chosen = TRUE;
                }
                i++;
            }
        }
    }

    // Mr Pass
    if (!chosen) {
        nextAction.actionCode = PASS;
    }

    return nextAction;
}
