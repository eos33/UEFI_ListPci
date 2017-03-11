/**
Author : Barry Liang 
Notes  : Please use it with EDK2 EADK LibC
**/
#include <stdio.h> 
#include <stdlib.h>
#include <wchar.h>
#include <string.h>
#include "AsciiFileAPI.h"

#define ASCIIFILEAPI_LINESTR_LENGTH 256 //Use long nage to avoid repeated definition

//EDK2 & EADK LibC don't have strlwr()/strupr(), build them here
char *strlwr(char *s) 
{
	int i, len;
	len = strlen(s);
	for(i=0; i<len; i++) {
		if( (s[i] >= 'A') && (s[i]<= 'Z') )
			s[i]= s[i] + ('a' - 'A');
	}
	return s;
}

//EDK2 & EADK LibC don't have strlwr()/strupr(), build them here
char *strupr(char *s)
{
	int i, len;
	len = strlen(s);
	for(i=0; i<len; i++) {
		if( (s[i] >= 'a') && (s[i]<= 'z') )
			s[i]= s[i] - ('a' - 'A');
	}
	return s;
}

void ReplaceChar(char *strSrc, char OldCh, char NewCh)
{
	int i;
	for(i=0; i<strlen(strSrc); i++) {
		if(strSrc[i]==OldCh)
			strSrc[i]=NewCh;
	}
}

void DelRChar(char *strSrc, char ch)
{
	int i;
	if(strlen(strSrc)==0)
		return;
	for(i=strlen(strSrc)-1; i>=0; i--) {
		if(strSrc[i]!=ch)
			break;
		else
			strSrc[i]='\0';
	}
}

char *strrev(char *strSrc) //Build strrev(), because Linux and EFI(EADK) does not support C function strrev()
{
	int i, j;
	char strTmp[ASCIIFILEAPI_LINESTR_LENGTH];
	memset(strTmp, 0x00, ASCIIFILEAPI_LINESTR_LENGTH);
	for(i=strlen(strSrc)-1,j=0 ; i>=0 ; i--,j++) {
		strTmp[j]=strSrc[i];
	}
	strTmp[j]='\0';
	sprintf(strSrc,"%s",strTmp);
	return strSrc;
}

void DelLChar(char *strSrc, char ch)
{
	DelRChar(strrev(strSrc),ch);
	strrev(strSrc);
}

char *AdjustString(char *strSrc) //Compatible more config file format,such as DOS/Windows/Linux
{
	if(strlen(strSrc)==0)
		return "";
	else {
		ReplaceChar(strSrc, 0xA1, ' '); //0xA1 : Full-width space
		ReplaceChar(strSrc, 0x09, ' '); //0x09 : Tab
		DelRChar(strSrc, '\n'); //Right LF, 0x0A
		DelRChar(strSrc, '\r'); //Right CR, 0x0D
		DelRChar(strSrc, ' ');  //Right Space
		DelLChar(strSrc, ' ');  //Left  Space
		//DelRChar(strSrc, 0x09); //Right Tab
		//DelLChar(strSrc, 0x09); //Left  Tab

		//For meeting most of the WITUL's code,WITUL use "[FanSpeed Test]\r\n"... to compare
		//Here,add "\r\n" to avoid more source code modification
		strcat(strSrc, "\r\n");
		
		return strSrc;
	}
}

//This function should be used after AdjustString()
void SkipBlankLines(FILE *fp, char *strSrc)
{
	char LineData[ASCIIFILEAPI_LINESTR_LENGTH];
	memset(LineData, 0x00, ASCIIFILEAPI_LINESTR_LENGTH);
	if( !strcmp(strSrc, "\r\n") ) {
		while( !feof(fp) ) {
			fgets(LineData, ASCIIFILEAPI_LINESTR_LENGTH, fp); //Get next line
			AdjustString(LineData);   //Fix string format
			if( !strcmp(LineData,"\r\n") ) //If this line is "blank line"
				continue;
			else {
				sprintf(strSrc, "%s", LineData);
				break;
			}
		}
	}
}

/*
----------> Likes MicroSoft VC, GetPrivateProfileString()
//DWORD GetPrivateProfileString(	LPCTSTR lpAppName,        // section name
//									LPCTSTR lpKeyName,        // key name
//									LPCTSTR lpDefault,        // default string
//									LPTSTR lpReturnedString,  // destination buffer
//									DWORD nSize,              // size of destination buffer
//									LPCTSTR lpFileName        // initialization file name
//								);
GetPrivateProfileString("SETUP", "Mode", "0", szINIReturn, 64, szINIpath);
GetPrivateProfileString("SETUP", "LocalIni", "0", szINIReturn, 64, szINIpath);

----------> *.ini as below
[SETUP]
Mode=0
LocalIni=C:\W240VA.ini
*/
//Return : 0 - OK, 1 - Error or fail
int GetPrivateProfileString(char FileName[], char SectionName[], char KeyName[], char *StrOut)
{
	FILE 	*fp;
	char	TmpStr[ASCIIFILEAPI_LINESTR_LENGTH];
	char	LineStr[ASCIIFILEAPI_LINESTR_LENGTH];
	int		i;
	int 	hit=0;

	memset(TmpStr, 0x00, ASCIIFILEAPI_LINESTR_LENGTH);
	memset(LineStr, 0x00, ASCIIFILEAPI_LINESTR_LENGTH);
	if( (fp = fopen(FileName, "r") ) == NULL ) {
		printf("Error, open file %s fail!\n", FileName);
		return 1;
	}

	while( (!feof(fp)) && (hit != 1) ) {
		fgets(LineStr, ASCIIFILEAPI_LINESTR_LENGTH, fp);
		AdjustString(LineStr);   //Fix string format
		if( !strcmp(LineStr, "\r\n") )   //If this line is "blank line"
			continue;
		if( !strncmp(LineStr, "//", 2) ) //Skip the line begining with "//"
			continue;
		if( !strncmp(LineStr, ";", 1) )  //Skip the line begining with ";"
			continue;
		if( !strncmp(LineStr, "#", 1) )  //Skip the line begining with "#"
			continue;
		if( !strncmp(LineStr, SectionName, strlen(SectionName)) ) {
			while( ((fgets(LineStr, ASCIIFILEAPI_LINESTR_LENGTH, fp)) != NULL)  && (hit != 1)) {
				AdjustString(LineStr);
				if( !strncmp(LineStr, KeyName, strlen(KeyName)) ) { 
					//printf("LineStr: %s---\n", LineStr);
					memset(TmpStr, 0x00, ASCIIFILEAPI_LINESTR_LENGTH);
					if( strstr(LineStr, "=") != NULL ) {
						hit=1;
						strcpy(StrOut, strstr(LineStr, "=") + 1); //Data after "="
						ReplaceChar(StrOut, 0x09, ' '); //0x09 : Tab
						DelRChar(StrOut, '\n'); //Right LF, 0x0A
						DelRChar(StrOut, '\r'); //Right CR, 0x0D
						DelRChar(StrOut, ' ');  //Right Space
						DelLChar(StrOut, ' ');  //Left  Space
						//printf("StrOut : %s---\n", StrOut);
						break;
					}
				}
			}
		}
	}
	fclose(fp);
	if(hit)
		return 0;
	else {
		printf("Error, can not find Section %s, Keyword %s \n", SectionName, KeyName);
		return 1;
	}
}

/*
----------> Likes MicroSoft VC, GetPrivateProfileString()
//WritePrivateProfileString("CONTROL240VAFPGALINK", "Port", G_V.strPort_240VABD, szINIpath); 
*/
//Update data to *.ini
//Return : 0 - OK, 1 - Error or fail
//Not ready now, 2016/04/12
int WritePrivateProfileString(char FileName[], char SectionName[], char KeyName[], char Str2Write[])
{
	FILE 	*fp;
	char	TmpStr[ASCIIFILEAPI_LINESTR_LENGTH];
	CHAR16	LineStr[ASCIIFILEAPI_LINESTR_LENGTH];
	int		i;
	int 	hit=0;

	memset(TmpStr, 0x00, ASCIIFILEAPI_LINESTR_LENGTH);
	memset(LineStr, 0x00, ASCIIFILEAPI_LINESTR_LENGTH);
	if( (fp = fopen(FileName, "a+") ) == NULL ) {
		printf("Error, open file %s fail!\n", FileName);
		return 1;
	}
	sprintf(LineStr,"[%s]\n%s=%s\n",SectionName,KeyName,Str2Write);
	fputs(LineStr, fp);
    fclose(fp);
	return 0;
}

int CleanFile(char FileName[])
{
	FILE 	*fp;

	if( (fp = fopen(FileName, "w+") ) == NULL ) {
		printf("Error, open file %s fail!\n", FileName);
		return 1;
	}
	fclose(fp);
	return 0;
} 

//Position start - begining with 0
//Return : 0  - OK, 
//         -1 - Error or fail
int StrMid(const char strSrc[], unsigned int start, unsigned int len, char *StrOut)
{
	int i;
	if( (start+len) > strlen(strSrc) )
		return -1;
	for(i=0; i<len; i++) {
		StrOut[i]=strSrc[start+i];
	}
	StrOut[i]=0x00; //End with '\0'
	return 0;
}

//Return : The index position in strSrc - begining with 0 
//         -1 if nothing found
int StrFindSubChIndex(const char strSrc[], unsigned int start, char chDelimitior)
{
	int i, hit;
	i=hit=0;
	if( start > strlen(strSrc) )
		return -1;
	for(i=0; (i+start)<strlen(strSrc); i++) {
		if( strSrc[i+start] == chDelimitior) {
			hit=1;
			return (i+start);
		}
	}
	if(hit==0)
		return -1;
}

//Return : 0 - OK, 1 - Error or fail
int StringSplit(const char strSrc[], int *cnt, char StrArray[][20], char chDelimitior)
{
	char strtmp[20];
	int i, j, len, nStart, nEnd;
	i = j = len = nStart = nEnd = 0;

	len = strlen(strSrc);
	while( nEnd < len ) {
		memset(strtmp, 0x00, sizeof(strtmp));
		nEnd = StrFindSubChIndex(strSrc, nStart, chDelimitior);
		if( -1 == nEnd )
			nEnd = len;
		
		if( -1 != StrMid(strSrc, nStart, nEnd - nStart, strtmp) ) {
			strcpy(StrArray[i], strtmp);
			i++;
		}
		nStart = nEnd + 1;
	}
	
	*cnt=i;
	if( i>256 ) {
		printf("Error, Overflow , just support total 256 sub-string!\n");
		return -1;
	} else {
		/*
		printf("********************\n");
		printf("%d\n", *cnt);
		for(j=0; j< (*cnt); j++) {
			printf("StrArray[%d]=[%s]\n", j, StrArray[j]);
		}
		printf("********************\n");
		*/
		return 0;
	}
}