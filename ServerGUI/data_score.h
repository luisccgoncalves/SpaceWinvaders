#ifndef DATA_SCORE_H
#define DATA_SCORE_DATA_H

#include "data_structs.h"

int writeTop10ToReg(HighScore *top10);
int readTop10FromReg(HighScore * top10);
int SystemTimeString(TCHAR * timeString);
int addScoretoTop10(int score, HighScore * top10);
int sortTop10(HighScore *top10);

#endif /* DATA_SCORE_DATA_H */