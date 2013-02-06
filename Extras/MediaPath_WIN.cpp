#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "MediaPath.h"

static const char* searchPathArray[] = 
{
	"Samples\\compiler\\vc8win32\\",
	"..\\..\\Media\\",
	".\\Samples\\Media\\",
	"..\\..\\Samples\\Media\\",
	"..\\..\\..\\..\\..\\Samples\\Media\\",
	"..\\..\\..\\..\\..\\..\\Samples\\Media\\",
	"..\\..\\..\\TrainingPrograms\\Programs\\Data\\Media\\",
	"..\\..\\..\\..\\TrainingPrograms\\Programs\\Data\\Media\\",
	"..\\..\\..\\..\\..\\TrainingPrograms\\Programs\\Data\\Media\\",
	"..\\TrainingPrograms\\Programs\\Data\\SoftBody\\",
	"..\\..\\TrainingPrograms\\Programs\\Data\\SoftBody\\",
	"..\\..\\..\\TrainingPrograms\\Programs\\Data\\SoftBody\\",
	"..\\..\\..\\..\\TrainingPrograms\\Programs\\Data\\SoftBody\\",
	"..\\..\\..\\..\\..\\TrainingPrograms\\Programs\\Data\\SoftBody\\",
	"..\\..\\..\\Data\\Core_Dump\\",
	"..\\..\\..\\..\\Data\\Core_Dump\\",
	"TrainingPrograms\\Programs\\Data\\Core_Dump\\",
	"..\\..\\TrainingPrograms\\Programs\\Data\\Core_Dump\\",
	0 //TERMINATOR
};


static bool MediaFileExists(const char *fname)
{
	FILE *fp=fopen(fname,"rb");
	if(fp == NULL)
	{
		return false;
	}
	else
	{
		fclose(fp);
		return true;
	}
}


char* FindMediaFile(const char *fname,char *buff)
{
	char curPath[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, curPath);
	strcat(curPath, "\\");
	strcpy(buff, curPath);
	strcat(buff,fname);
	if(MediaFileExists(buff))
		return buff;

	char basePath[MAX_PATH];
	GetModuleFileNameA(NULL, basePath, MAX_PATH);
	char* pTmp = strrchr(basePath, '\\');
	if(pTmp != NULL){
		*pTmp = 0;
		SetCurrentDirectoryA(basePath);
		pTmp = strrchr(basePath, '\\');
		if(pTmp != NULL){
			*pTmp = 0;
		}
		pTmp = strrchr(basePath, '\\');
		if(pTmp != NULL){
			*(pTmp + 1) = 0;
		}
	}
	// printf("base path is: %s\n", basePath);

	int i = 0;
	while(searchPathArray[i])
	{
		strcpy(buff, basePath);
		strcat(buff,searchPathArray[i]);
		strcat(buff,fname);
		if(MediaFileExists(buff))
			return buff;
		i++;
	}

	strcpy(buff,fname);
	if(MediaFileExists(buff))
		return buff;

	printf("Can't find media file: %s\n", fname);

	strcpy(buff,fname);
	return buff;
}


char* GetTempFilePath(char *buff)
{
	strcpy(buff,"");
	return buff;
}
