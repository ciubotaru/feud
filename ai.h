#ifndef AI_H
#define AI_H

#define INIT_STAGE 0
#define WAIT_STAGE 1
#define THINK_STAGE 2

int stage;
int side;
int ai_plays_for;
//char command[256];
//char line[256];

/* ADD EVALUATION CONSTANTS HERE */

float evaluate();

#endif /* AI_H */
