/****The ASSERT() macro Checks if a parameter is NULL****/
#ifndef MYASSERT_H
#define MYASSERT_H

#include <fstream>
#include <stdlib.h>
using namespace std;



	
#ifdef DEBUG
	void _Assert(char *strFile, unsigned uLine);
	#define ASSERT(f)	\
		if(f)			\
			NULL;		\
		else			\
		_Assert(__FILE__,__LINE__)	//if FALSE, the the function writes into:
#else									//ASSERT.log the macros __LINE__ and __FILE__ 
	#define ASSERT(f)	NULL
#endif



#endif