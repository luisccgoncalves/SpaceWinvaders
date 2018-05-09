#ifndef LOCALSTRUCTS_H
#define LOCALSTRUCTS_H

typedef struct {
	HANDLE			hTick;							//Handle to event. Warns gateway about updates in shared memory
	int				ThreadMustGoOn;
	HANDLE			*mhInvader;
}GTickStruct;

#endif /* LOCALSTRUCTS_H */