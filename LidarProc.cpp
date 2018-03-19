// LidarProc.cpp: 定义控制台应用程序的入口点。
//

#include "LASVectorClassify.h"
#include "LASColorMap.h"
#include "LASReader.h"


int main()
{
	LASColorMap colorMap;
	vector<string> imgs;
	char* pathLas="/media/wuwei/My Passport/原始点云/L001-1-17110801-S1-C1_r.las";
	char* pathImgDir="/media/wuwei/My Passport/香港项目成果/dom-经纬度/tif/";
	colorMap.LASCorlorMap_ImageFind(pathLas,pathImgDir,imgs);
	colorMap.LASColorMap_Map(pathLas,imgs,"/home/wuwei/Data/LAS/L001-1-17110801-S1-C1_r_colorMap.las");

//    ILASDataset *dataset=new ILASDataset();
//    LASReader* reader = new LidarMemReader();
//    reader->LidarReader_Open("/home/wuwei/Data/LAS/L001-1-17110801-S1-C1_r_colorMap.las",dataset);
//    reader->LidarReader_Read(true,10,dataset);
//    reader->LidarReader_Write("/home/wuwei/Data/LAS/L001-1-17110801-S1-C1_r_colorMap_sample.las",dataset);
	return 0;
}

