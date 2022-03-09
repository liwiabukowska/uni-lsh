#pragma once

char* find_char(char* const begin, char* const end, char c);

// jest potrzebny tylko do 2> bo jezeli uzyje szukaj > to jezeli sa > oraz 2>
// to znajdzie tylko ten ktory wystepuje pierwszy
// moge zalozyc ze 2> wystepuje zawsze po > i pominac > ale wtedy jezeli nie ma >
// to nie znajduje 2>
// a co jezeli beda w odwrotnej kolejnosci... robi sie syf
// dlatego potrzebuje substr aby parsowanie nie bylo zalezne od kolejnosci
// i przechwytywalo 2> ale nie >
char* find_substr(char* const begin, char* const end, char* const substr);