// LidarProc.cpp: 定义控制台应用程序的入口点。
//

#include "LASReader.h"
#include "LASPoint.h"

int main()
{
	LidarPatchReader patchReader;
	char* pathLas="/home/wuwei/Data/LAS/2477.00-184.75.las";
	char* pathDir="/home/wuwei/Data/LAS/vegterian";
	patchReader.LidarReader_SplitPatch(pathLas,pathDir,1);

	return 0;
}

