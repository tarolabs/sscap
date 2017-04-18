#pragma once
#include <string>
using namespace std;

string NewVersion_GetVersionNo();
string NewVersion_GetVersionChange();
void EndGetNewVersionThread();
void StartGetNewVersionThread( void *pWnd );
