#include "score.h"


int sortTop10(HighScore *top10) {
	/*
	Regular bubble sort algorithm
	Values are stored alphabetically in the registry
	*/

	HighScore temp;
	int i, j;

	for (i = 0; i < 9; ++i)
		for (j = 0; j < 9 - i; ++j) {
			if (top10[j].score < top10[j + 1].score) {
				temp = top10[j + 1];
				top10[j + 1] = top10[j];
				top10[j] = temp;
			}
		}
	return 0;
}
int writeTop10ToReg(HighScore *top10) {

	HKEY	key;
	DWORD	dwDisposition;
	LONG	lResult;

	lResult = RegDeleteKeyEx(							//Deletes the HighScores subkey
		HKEY_CURRENT_USER,
		REG_SUBKEY,
		KEY_WOW64_32KEY,
		0);
	if (lResult != ERROR_SUCCESS) {
		_tprintf(TEXT("[Error] Deleting registry key (%d)\n"), GetLastError());
		return -1;
	}

	lResult = RegCreateKeyEx(							//Creates a new, blank subkey
		HKEY_CURRENT_USER,								//A handle to an open registry key.
		REG_SUBKEY,										//The name of a subkey that this function opens or creates.
		0,												//This parameter is reserved and must be zero.
		NULL,											//The user-defined class type of this key.
		REG_OPTION_NON_VOLATILE,						//Options: This key is not volatile, information is preserved after restart.
		KEY_SET_VALUE,									//A mask that specifies the access rights for the key to be created.
		NULL,											//Inheritance: NULL= Not inherited
		&key,											//A pointer to a variable that receives a handle to the opened or created key.
		&dwDisposition);								//A pointer to a variable that receives REG_CREATED_NEW_KEY or REG_OPENED_EXISTING_KEY
	if (lResult != ERROR_SUCCESS) {
		_tprintf(TEXT("[Error] Creating registry key (%d)\n"), GetLastError());
		return -1;
	}

	for (int i = 0; i < 10 && top10[i].score != 0; i++) {
		lResult = RegSetValueEx(							//Stores Username:score in the previously created key.
			key,										//A handle to an open registry key.
			top10[i].timestamp,							//The name of the value to be set.
			0,											//This parameter is reserved and must be zero.
			REG_DWORD,									//The type of data pointed to by the lpData parameter
			(LPBYTE)&top10[i].score,					//The data to be stored.
			sizeof(DWORD));								//The size of the information pointed to by the lpData parameter, in bytes.
		if (lResult != ERROR_SUCCESS) {
			_tprintf(TEXT("[Error] Creating registry key (%d)\n"), GetLastError());
			return -1;
		}
	}

	RegCloseKey(key);									//Closes the key

	return 0;
}

int readTop10FromReg(HighScore * top10) {

	HKEY	key;
	TCHAR	lpValueName[SMALL_BUFF];
	DWORD	lpData, lpType;
	DWORD	szValueNameRead, szValueName = SMALL_BUFF * sizeof(TCHAR);
	DWORD	szDataRead, szData = sizeof(DWORD);
	DWORD	nRegValues;
	LONG	lResult;
	DWORD	dwDisposition;

	//Populates top 10 with no scores
	for (int i = 0; i < 10; i++) {
		_tcscpy_s(top10[i].timestamp, szValueName, TEXT("NO RECORD"));
		top10[i].score = 0;
	}

	//Opens a key for writing, if non existant, creates a new one
	lResult = RegCreateKeyEx(
		HKEY_CURRENT_USER,									//A handle to an open registry key.
		REG_SUBKEY,											//The name of a subkey that this function opens or creates.
		0,													//This parameter is reserved and must be zero.
		NULL,												//The user-defined class type of this key.
		REG_OPTION_NON_VOLATILE,							//Options: This key is not volatile, information is preserved after restart.
		KEY_READ,											//A mask that specifies the access rights for the key to be created.
		NULL,												//Inheritance: NULL= Not inherited
		&key,												//A pointer to a variable that receives a handle to the opened or created key.
		&dwDisposition);									//A pointer to a variable that receives REG_CREATED_NEW_KEY or REG_OPENED_EXISTING_KEY
	if (lResult != ERROR_SUCCESS) {
		_tprintf(TEXT("[Error] Creating registry key (%d)\n"), GetLastError());
		return -1;
	}

	if (dwDisposition == REG_CREATED_NEW_KEY) {				//Registry was empty, nothing to copy
		RegCloseKey(key);
		return 0;											//Returns an empty top 10
	}

	//Query how many values the subkey has
	RegQueryInfoKey(
		key, NULL, NULL, NULL, NULL, NULL, NULL,			//Key to check
		&nRegValues, NULL, NULL, NULL, NULL);				//Pointer to be filled with the number of values stored

															//Reads nRegvalues from registry to Top10 
	for (int i = 0; lResult == ERROR_SUCCESS && i<(int)nRegValues; i++) {
		szValueNameRead = szValueName;
		szDataRead = szData;
		lResult = RegEnumValue(
			key,											//A handle to an open registry key.
			i,												//The index of the value to be retrieved.
			lpValueName, 									//A pointer to a buffer that receives the name of the value as a null-terminated string. 
			&szValueNameRead, 								//Specifies the size of the buffer pointed to by the lpValueName parameter, in characters.
			NULL, 											//Reserved
			&lpType, 										//A pointer to a variable that receives a code indicating the type of data
			(LPBYTE)&lpData,								//A pointer to a buffer that receives the data for the value entry.
			&szDataRead);									//A pointer to a variable that specifies the size of the buffer pointed to by the lpData parameter, in bytes.
		if (lpType != REG_DWORD) {							//Ignores all non DWORD values
			i--;
			nRegValues--;
		}
		else if (lResult == ERROR_SUCCESS) {				//Copy the value to Top10
			_tcscpy_s(top10[i].timestamp,
				szValueNameRead + 1,							//Adding 1 to szValueNameRead to account for NULL terminator
				lpValueName);
			top10[i].score = lpData;
		}
	}

	RegCloseKey(key);

	sortTop10(top10);										//Sorts the array by score

	return 0;
}

int SystemTimeString(TCHAR * timeString) {
	/*
	Creates a timestamp
	Used to display top 10
	*/
	SYSTEMTIME time;

	GetLocalTime(&time);									//Populates structure with local time
	_stprintf_s(timeString,
		SMALL_BUFF,
		TEXT("(%02d:%02d %02d/%02d/%d)"),					//Copies the values with the format (HH:MM DD/MM/YYY)
		time.wHour, time.wMinute,
		time.wDay, time.wMonth, time.wYear);

	return 0;
}

int addScoretoTop10(int score, HighScore * top10) {

	int i;

	for (i = 9; (i >= 0) && ((DWORD)score > top10[i].score); i--) {
		top10[i] = top10[i - 1];								//Finds a space for the new highscore
	}

	if (i < 9) {
		SystemTimeString(top10[i + 1].timestamp);			//Gets a timestamp for posterity
		top10[i + 1].score = score;							//Copies current score to top10
		writeTop10ToReg(top10);								//Write top10 to registry
		return 0;											//Return 0 if "New Highscore!"
	}
	else
		return 1;											//Return 1 if no new highscore
}