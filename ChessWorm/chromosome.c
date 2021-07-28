#include "chromosome.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "evaluation.h";

Chromosome MakeChromosome() {
	Chromosome chromosome;
	chromosome.performance = 0;
	for (int i = 0; i < CHROMOSOME_POLINOM_DEGREE * CHROMOSOME_ARGUMENTS; i++)
		chromosome.params[i] = 0;
	return chromosome;
}

Chromosome ChildChromosome (Chromosome* father) {
	Chromosome chromosome = *father;
	for (int i = 0; i < CHROMOSOME_POLINOM_DEGREE * CHROMOSOME_ARGUMENTS; i++) {
		float r = (RAND_MAX / 2) - rand();
		r /= (RAND_MAX / 2);
		chromosome.params[i] += r * EVOLUTION_STEP * 
			(1 - chromosome.performance);
	}
	return chromosome;
}

float ChromosomeMoveEval (Chromosome* chromosome, Move move) {
	float eval = 0;
	float arg = 0;
	int paramIndex = 0;
	for (int i = 0; i < CHROMOSOME_ARGUMENTS; i++) {
		switch (i) {
		case 0:
			arg = PieceValue(move.pieceChar);
			break;
		case 1:
			arg = PieceValue(move.captureChar);
			break;
		case 2:
			arg = move.sourceFile;
			break;
		case 3:
			arg = move.sourceRank;
			break;
		case 4:
			arg = move.destinationFile;
			break;
		case 5:
			arg = move.destinationRank;
			break;
		case 6:
			arg = move.special;
		default:
			break;
		}
		float polinom = 0;
		for (int p = 0; p < CHROMOSOME_POLINOM_DEGREE; p++) {
			polinom += powf(arg, p) * chromosome->params[paramIndex];
			paramIndex++;
		}
		eval *= polinom;
	}
	return eval;
}

void CopyChromosome (Chromosome* original, Chromosome* copy) {
	copy->performance = original->performance;
	for (int i = 0; i < CHROMOSOME_ARGUMENTS * CHROMOSOME_POLINOM_DEGREE; i++)
		copy->params[i] = original->params[i];
}

void LogChromosome (Chromosome* chromosome) {
	printf("Chromosome:\nPreformance: %f", chromosome->performance);
	printf("\nParams:\n");
	for (int i = 0; i < CHROMOSOME_ARGUMENTS * CHROMOSOME_POLINOM_DEGREE; i++)
		printf("%f,", chromosome->params[i]);
}