#include "MYASSERT.h"

void _Assert(char *strFile, unsigned uLine){
	char szBuffer[100];
	fstream assertFile("ASSERT.log", ios::out );
	sprintf(szBuffer, "Failled ASSERT:\n File %s, line %d ", strFile, uLine);
	assertFile<<szBuffer;
	assertFile.close();
	abort();
}