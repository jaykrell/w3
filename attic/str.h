/*
This file is part of a C pipe dream.
The real code is C++.
*/
#pragma once

// Vec<char> and maybe terminal nul
// Try the MFC trick where vec is at s - 1;

void Str_New(char**);
void Str_PushBackOne(char** s, char elem);
void Str_PushBackMultiple(char** s, char* elem, size_t count);
void Str_PopBack(char** s, char* elem);
//char* Str_At(char** s, size_t i);
size_t Str_Size(char** s);
size_t Str_ByteSize(char** s);
size_t Str_Resize(char** s);
size_t Str_Reserve(char** s);
