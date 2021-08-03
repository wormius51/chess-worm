#ifndef MULTITHREAD_EVAL_H
#define MULTITHREAD_EVAL_H

#include "types.h"
#include <Windows.h>

EvalState globalEvalState;
EvalState evalStates[MAX_CANDIDATE_MOVES];

EvalState MakeEvalState ();
int BestEvalStateIndex ();
DWORD WINAPI ThreadEval (LPVOID param);
DWORD WINAPI MultiThreadEval (LPVOID param);


#endif // !MULTITHREAD_EVAL_H
