// LidarProc.cpp: 定义控制台应用程序的入口点。
//

#include "LASVectorClassify.h"
#include "LASFormatTransform.h"

int main()
{
    LASFormatTransform3DTiles trans3dTiles;
    Point3D pnt(0,0,0);
    trans3dTiles.LASFormatTransform_3DTilesJson(&pnt,"/home/wuwei/Data/LAS/test");
	return 0;
}

