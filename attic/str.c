/*
This file is part of some C pipe dream.
The real code is C++.
*/
#if 0
#include <stdlib.h>
#include "vec.h"

struct Str;
typedef struct Str Str;

struct Str
{
	Vec v;
	char* s;
};

// Vec<char> and maybe terminal nul
// Try the MFC trick where vec is at s - 1;

void Str_New(char** s)
{
	Str* a = (Str*)calloc(1, sizeof(*a));
	s = &a->s;
}

void Str_In(char** s, Vec** v)
{
	*v = (Vec*)CONTAINER(s, Str, s);
}

void Str_Out(char** s, Vec* v)
{
	*s = v->start;
}

int Str_PushBackOne(char** s, char elem)
{
	int err;
	Vec* v;
	Str_In(s, &v);
	err = Vec_PushBackOne(v, &elem);
	Str_Out(s, &v);
	return err;
}

int Str_PushBackMultiple(char** s, char* elem, size_t count)
{
	int err;
	Vec* v;
	Str_In(s, &v);
	err = Vec_PushBackOneMultiple(v, elem, count);
	Str_Out(s, &v);
	return err;
}

void Str_PopBack(char** s, char* elem);
//char* Str_At(char** s, size_t i);
size_t Str_Size(char** s);
size_t Str_ByteSize(char** s);
size_t Str_Resize(char** s);
size_t Str_Reserve(char** s);

#endif
