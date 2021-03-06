#ifndef SCORE_H
#define SCORE_H

#include "localStructs.h"

int writeTop10ToReg(HighScore *top10);
int readTop10FromReg(HighScore * top10);
int SystemTimeString(TCHAR * timeString);
int addScoretoTop10(int score, HighScore * top10);
int sortTop10(HighScore *top10);

#endif /* SCORE_H */