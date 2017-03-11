#ifndef _ASCII_FILE_API_H
#define _ASCII_FILE_API_H

#include <Uefi.h>                               //MdePky\Include\Uefi.h
#include <Library/UefiLib.h>                    //MdePkg\Include\Library\UefiLib.h
#include <Library/UefiApplicationEntryPoint.h>  //MdePkg\Include\Library\UefiApplicationEntryPoint.h
//#include <Library/IoLib.h>                      //MdePkg\Include\Library\IoLib.h
#include <stdio.h> 
#include <stdlib.h>
#include <wchar.h>
#include <string.h>

char *strlwr(char *s);
char *strupr(char *s);
void ReplaceChar(char *strSrc, char OldCh, char NewCh);
void DelRChar(char *strSrc, char ch);
char *strrev(char *strSrc);
void DelLChar(char *strSrc, char ch);
char *AdjustString(char *strSrc);
void SkipBlankLines(FILE *fp, char *strSrc);
int GetPrivateProfileString(char FileName[], char SectionName[], char KeyName[], char *StrOut);
int WritePrivateProfileString(char FileName[], char SectionName[], char KeyName[], char Str2Write[]);
int StrFindSubChIndex(const char strSrc[], unsigned int start, char chDelimitior);
int StringSplit(const char strSrc[], int *cnt, char StrArray[][128], char chDelimitior);
#endif
      