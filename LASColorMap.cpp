
// Created by wuwei on 18-1-24.


#include "LASColorMap.h"
#include <iostream>
#include "LASReader.h"
#include "tsmToUTM.h"
#include "GeometryAlgorithm.h"

#pragma comment(lib,"gdal_i.lib")

long LASColorMap::LASColorMap_Map(const char* pathLas, vector<string> pathImgs, const char* pathOutLas)
{
	FILE* fInLas = fopen(pathLas, "rb");
	FILE* fOutLas = fopen(pathOutLas, "wb");

	LidarPatchReader reader;
	LASHeader lasHeader;
	reader.LidarReader_ReadHeader(fInLas, lasHeader);
	reader.LidarReader_WriteHeader(fOutLas, lasHeader);

	const int numBlock = 100000;
	int realRead = numBlock;
	LASPoint* points = NULL;
	try {
		points = new LASPoint[numBlock];
	}
	catch (std::bad_alloc e)
	{
		std::cerr << e.what() << endl;
	}
	int totalNum = 0;
    int numLasDataset = lasHeader.number_of_point_records;
	while (realRead != 0)
	{
		Rect2D rect = reader.LidarReader_ReadPatch(fInLas, lasHeader, points, realRead);
		totalNum += realRead;
        printf("proces %d-%d \r",numLasDataset,totalNum);
//#ifdef PROCESS_OUTPUT_SCREEN
//        printf("proces %d-%d \r",numLasDataset,totalNum);
//#endif
		int realProcessPoints = 0;

		if (realRead == 0)
			break;

		char* labelPoint = new char[realRead];
		memset(labelPoint, 0, sizeof(char)*realRead);

		for (int i = 0; i<pathImgs.size(); ++i)
		{
			GDALAllRegister();
			GDALDatasetH m_dataset = GDALOpen(pathImgs[i].c_str(), GA_ReadOnly);
			double adfGeoTrans[6];
			GDALGetGeoTransform(m_dataset, adfGeoTrans);
			int xsize = GDALGetRasterXSize(m_dataset);
			int ysize = GDALGetRasterYSize(m_dataset);

#ifdef LATLONGITUDE
            double adfTransUTM[6]={0};
            int zone=49;
            tsmLatLongToUTM(adfGeoTrans[3],adfGeoTrans[0],&zone,&adfTransUTM[0],&adfTransUTM[3]);
            double txmax = adfGeoTrans[0]+xsize*adfGeoTrans[1]+ysize*adfGeoTrans[2];
            double tymax = adfGeoTrans[3]+xsize*adfGeoTrans[4]+ysize*adfGeoTrans[5];
            double xUTM,yUTM;
            tsmLatLongToUTM(tymax,txmax,&zone,&xUTM,&yUTM);
            adfTransUTM[1]=(xUTM-adfTransUTM[0])/xsize;
            adfTransUTM[5]=(xUTM-adfTransUTM[3])/ysize;
            memcpy(adfGeoTrans,adfTransUTM,sizeof(double)*6);
#endif

			Rect2D rectImg = LASColorMap_ImageRange(m_dataset);

			if (GeometryRelation::IsRectIntersectRect(rect, rectImg))
			{
				unsigned char *data = NULL;
				data = new unsigned char[3 * xsize*ysize];
				for (int nb = 1; nb <= 3; ++nb)
					GDALRasterIO(GDALGetRasterBand(m_dataset, nb), GF_Read, 0, 0, xsize, ysize, data + (nb - 1)*xsize*ysize, xsize, ysize, GDT_Byte, 0, 0);

				for (int i = 0; i<realRead; ++i)
				{
					if (!labelPoint[i]) {
						double x = points[i].m_vec3d.x;
						double y = points[i].m_vec3d.y;
						int ix, iy;
						LASColorMap_MapToImage(x, y, adfGeoTrans, ix, iy);
						if (ix < 0 || iy < 0 || ix >= xsize || iy >= ysize)
							continue;
						else {
							if (data[iy * xsize + ix] == 0 && data[iy * xsize + ix + xsize * ysize] == 0 && data[iy * xsize + ix + xsize * ysize] == 0)
								continue;
							points[i].m_colorExt.Red = data[iy * xsize + ix];
							points[i].m_colorExt.Green = data[iy * xsize + ix + xsize * ysize];
							points[i].m_colorExt.Blue = data[iy * xsize + ix + 2 * xsize * ysize];
							realProcessPoints++;
							labelPoint[i] = 1;
						}
					}
				}
				delete[]data; data = NULL;
			}
			GDALClose(m_dataset);
			if (realProcessPoints == realRead)
			{
				reader.LidarReader_WritePatch(fOutLas, lasHeader, points, realRead);
				break;
			}
		}
		delete[]labelPoint; labelPoint = NULL;
		if (realProcessPoints != realRead)
			reader.LidarReader_WritePatch(fOutLas, lasHeader, points, realRead);
	};
#ifdef PROCESS_OUTPUT_SCREEN
    printf("\n");
#endif
	delete[]points; points = NULL;
	fclose(fInLas);
	fclose(fOutLas);
	return 0;
}

long LASColorMap::LASColorMap_Map(const char* pathLas, const char* pathImg, const char* pathOutLas)
{
	FILE* fInLas = fopen(pathLas, "rb");
	FILE* fOutLas = fopen(pathOutLas, "wb");

	LidarPatchReader reader;
	LASHeader lasHeader;
	reader.LidarReader_ReadHeader(fInLas, lasHeader);
	reader.LidarReader_WriteHeader(fOutLas, lasHeader);

	const int numBlock = 100000;
	int realRead = numBlock;
	GDALAllRegister();
	GDALDatasetH m_dataset = GDALOpen(pathImg, GA_ReadOnly);
	double adfGeoTrans[6];
	GDALGetGeoTransform(m_dataset, adfGeoTrans);

	int xsize = GDALGetRasterXSize(m_dataset);
	int ysize = GDALGetRasterYSize(m_dataset);

#ifdef LATLONGITUDE
    double adfTransUTM[6]={0};
    int zone=49;
    tsmLatLongToUTM(adfGeoTrans[3],adfGeoTrans[0],&zone,&adfTransUTM[0],&adfTransUTM[3]);
    double txmax = adfGeoTrans[0]+xsize*adfGeoTrans[1]+ysize*adfGeoTrans[2];
    double tymax = adfGeoTrans[3]+xsize*adfGeoTrans[4]+ysize*adfGeoTrans[5];
    double xUTM,yUTM;
    tsmLatLongToUTM(tymax,txmax,&zone,&xUTM,&yUTM);
    adfTransUTM[1]=(xUTM-adfTransUTM[0])/xsize;
    adfTransUTM[5]=(xUTM-adfTransUTM[3])/ysize;
    memcpy(adfTransUTM,adfTransUTM,sizeof(double)*6);
#endif

	Rect2D rectImg = LASColorMap_ImageRange(m_dataset);

	Rect2D rangeLas;
	rangeLas.minx = lasHeader.min_x;    rangeLas.maxx = lasHeader.max_x;
	rangeLas.miny = lasHeader.min_y;    rangeLas.maxy = lasHeader.max_y;
	if (!GeometryRelation::IsRectIntersectRect(rectImg, rangeLas))
		return -1;


	LASPoint* points = NULL;
	unsigned char *data = NULL;
	try {
		data = new unsigned char[3 * xsize*ysize];
		points = new LASPoint[numBlock];
	}
	catch (std::bad_alloc e)
	{
		std::cerr << e.what() << endl;
	}

	for (int i = 0; i<3; ++i)
	{
		GDALRasterIO(GDALGetRasterBand(m_dataset, i + 1), GF_Read, 0, 0, xsize, ysize, data + i*xsize*ysize, xsize, ysize, GDT_Byte, 0, 0);
	}

	while (realRead != 0)
	{
		Rect2D rect = reader.LidarReader_ReadPatch(fInLas, lasHeader, points, realRead);
		for (int i = 0; i<realRead; ++i)
		{
			double x = points[i].m_vec3d.x;
			double y = points[i].m_vec3d.y;
			int ix, iy;
			LASColorMap_MapToImage(x, y, adfGeoTrans, ix, iy);
			if (ix<0 || iy<0 || ix>xsize || iy>ysize)
				continue;
			else {
				points[i].m_colorExt.Red = data[iy*xsize + ix];
				points[i].m_colorExt.Green = data[iy*xsize + ix + xsize*ysize];
				points[i].m_colorExt.Blue = data[iy*xsize + ix + 2 * xsize*ysize];
			}
		}
		reader.LidarReader_WritePatch(fOutLas, lasHeader, points, realRead);

	};

	delete[]points; points = NULL;
	delete[]data; data = NULL;

	fclose(fInLas);
	fclose(fOutLas);
	GDALClose(m_dataset);
	return 0;
}

long LASColorMap::LASCorlorMap_ImageFind(const char* pathLas,const char* pathImgDir,vector<string> &pImgs)
{
    ILASDataset *lasDataset = new ILASDataset();
    LASReader   *lasReader  = new LidarPatchReader();
    lasReader->LidarReader_Open(pathLas,lasDataset);
    Rect2D rectLas;
    rectLas.minx=lasDataset->m_lasHeader.min_x;
    rectLas.miny=lasDataset->m_lasHeader.min_y;
    rectLas.maxx=lasDataset->m_lasHeader.max_x;
    rectLas.maxy=lasDataset->m_lasHeader.max_y;

    //get image list
    pImgs.clear();
    vector<string> imgs;

    getFiles(pathImgDir,imgs);
    int numImages = imgs.size();
    GDALAllRegister();
    for(int i=0;i<numImages;++i){

#ifdef PROCESS_OUTPUT_SCREEN
        //printf("%d/%d\r",numImages,i+1);
#endif

        //Image Range
        GDALDatasetH dataset = GDALOpen(imgs[i].c_str(),GA_ReadOnly);
        int xsize = GDALGetRasterXSize(dataset);
        int ysize = GDALGetRasterYSize(dataset);

        double adfGeoTransform[6];
        GDALGetGeoTransform(dataset,adfGeoTransform);
#ifdef LATLONGITUDE
        double adfTransUTM[6]={0};
        int zone=49;
        tsmLatLongToUTM(adfGeoTransform[3],adfGeoTransform[0],&zone,&adfTransUTM[0],&adfTransUTM[3]);
        double txmax = adfGeoTransform[0]+xsize*adfGeoTransform[1]+ysize*adfGeoTransform[2];
        double tymax = adfGeoTransform[3]+xsize*adfGeoTransform[4]+ysize*adfGeoTransform[5];
        double xUTM,yUTM;
        tsmLatLongToUTM(tymax,txmax,&zone,&xUTM,&yUTM);
        adfTransUTM[1]=(xUTM-adfTransUTM[0])/xsize;
        adfTransUTM[5]=(xUTM-adfTransUTM[3])/ysize;

        memcpy(adfGeoTransform,adfTransUTM,sizeof(double)*6);
#endif
        //Get range
        double xmax = adfGeoTransform[0]+xsize*adfGeoTransform[1]+ysize*adfGeoTransform[2];
        double ymax = adfGeoTransform[3]+xsize*adfGeoTransform[4]+ysize*adfGeoTransform[5];

        Rect2D rectImg;
        rectImg.minx = adfGeoTransform[0];
        rectImg.miny = adfGeoTransform[3];
        rectImg.maxx = xmax;
        rectImg.maxy = ymax;

        if(GeometryRelation::IsRectIntersectRect(rectLas,rectImg))
        {
            pImgs.push_back(imgs[i]);
        }
        GDALClose(dataset);
    }
#ifdef PROCESS_OUTPUT_SCREEN
    //printf("\n");
#endif
    delete lasDataset;
    delete lasReader;
    return 0;
}

Rect2D LASColorMap::LASColorMap_ImageRange(GDALDatasetH m_dataset)
{
	double adfGeoTransform[6];
	GDALGetGeoTransform(m_dataset, adfGeoTransform);

	int xsize = GDALGetRasterXSize(m_dataset);
	int ysize = GDALGetRasterYSize(m_dataset);

	double mx1, my1;
	double mx2, my2;
	LASColorMap_ImageToMap(0, 0, adfGeoTransform, mx1, my1);
	LASColorMap_ImageToMap(xsize, ysize, adfGeoTransform, mx2, my2);

	Rect2D range;
#ifdef LATLONGITUDE
    double xUTM,yUTM;
    int zone=49;
    tsmLatLongToUTM(my1,mx1,&zone,&xUTM,&yUTM);
    range.minx=xUTM;range.maxy=yUTM;
    tsmLatLongToUTM(my2,mx2,&zone,&xUTM,&yUTM);
    range.maxx=xUTM;range.maxy=yUTM;
#else
    range.minx = min(mx1, mx2);
	range.maxx = max(mx1, mx2);
	range.miny = min(my1, my2);
	range.maxy = max(my1, my2);
#endif

	return  range;
}

bool LASColorMap::LASColorMap_MapToImage(double mx, double my, double adfGeoTransform[6], int &ix, int &iy)
{
//#ifdef LATLONGITUDE
//    //trans to lat long
//    double lx,ly;
//    tsmUTMToLatLong(50,mx,my,&ly,&lx);
//    ix = (lx - adfGeoTransform[0]) / adfGeoTransform[1];
//    iy = (ly - adfGeoTransform[3]) / adfGeoTransform[5];
//#else
    ix = (mx - adfGeoTransform[0]) / adfGeoTransform[1];
	iy = (my - adfGeoTransform[3]) / adfGeoTransform[5];
//#endif

	return true;
}

bool LASColorMap::LASColorMap_ImageToMap(int ix, int iy, double *adfGeoTransform, double &mx, double &my)
{
	mx = adfGeoTransform[0] + adfGeoTransform[1] * ix + adfGeoTransform[2] * iy;
	my = adfGeoTransform[3] + adfGeoTransform[4] * ix + adfGeoTransform[5] * iy;
	return true;
}