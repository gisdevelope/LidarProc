

//
// Created by wuwei on 18-1-14.
//

#include "LASSimpleClassify.h"
#include "tsmToUTM.h"
#include "Eigen/Dense"
#include "LASReader.h"
#include <string.h>
#ifdef linux
#include <unistd.h>
#include <dirent.h>
#endif
#ifdef WIN32
#include <direct.h>
#include <io.h>
#endif

using namespace std;
using namespace Eigen;

///////////////////////////////////////////////
long LASSimpleClassify::LASVegetationByEcho(ILASDataset *dataset)
{
	const int &numpoints = dataset->m_totalReadLasNumber;
	for (int i = 0; i<dataset->m_numRectangles; ++i)
	{
		LASRectBlock &block = dataset->m_lasRectangles[i];
		for (int j = 0; j < block.m_lasPoints_numbers; ++j) {
			LASPoint &point = block.m_lasPoints[j];
			if ((GetReturnNumber(point.m_rnseByte)>1))
			{
				point.m_classify = elcLowVegetation;
			}
		}
	}
	return 0;
}

long LASSimpleClassify::LASClassifyByElevation(ILASDataset *dataset, float elevation, bool direct,
	eLASClassification eclass)
{
	const int &numpoints = dataset->m_totalReadLasNumber;
	for (int i = 0; i<dataset->m_numRectangles; ++i)
	{
		LASRectBlock &block = dataset->m_lasRectangles[i];
		for (int j = 0; j < block.m_lasPoints_numbers; ++j) {
			LASPoint &point = block.m_lasPoints[j];
			if (direct)
			{
				point.m_classify = point.m_vec3d.z>elevation ? eclass : elcUnclassified;
			}
			else {
				point.m_classify = point.m_vec3d.z<elevation ? eclass : elcUnclassified;
			}
		}
	}

	return 0;
}

long LASSimpleClassify::LASClassifyByIntensity(ILASDataset *dataset, float intensity, bool direct,
	eLASClassification eclass)
{
	const int &numpoints = dataset->m_totalReadLasNumber;
	for (int i = 0; i<dataset->m_numRectangles; ++i)
	{
		LASRectBlock &block = dataset->m_lasRectangles[i];
		for (int j = 0; j < block.m_lasPoints_numbers; ++j) {
			LASPoint &point = block.m_lasPoints[j];
			if (direct)
			{
				point.m_classify = point.m_intensity>intensity ? eclass : elcUnclassified;
			}
			else {
				point.m_classify = point.m_intensity<intensity ? eclass : elcUnclassified;
			}
		}
	}

	return 0;
}

long LASSimpleClassify::LASClassifyByColor(ILASDataset *dataset, std::vector<ColoInfo> colorInfo)
{
	if (!dataset->m_lasHeader.HasLASColorExt4() && !dataset->m_lasHeader.HasLASColorExt6())
		return -1;

	const int &numpoints = dataset->m_totalReadLasNumber;
	for (int i = 0; i<dataset->m_numRectangles; ++i)
	{
		LASRectBlock &block = dataset->m_lasRectangles[i];
		for (int j = 0; j < block.m_lasPoints_numbers; ++j) {
			LASPoint &point = block.m_lasPoints[j];
			int minIdx = 0;
			double minDis = 9999999;

			for (int i = 0; i<colorInfo.size(); ++i)
			{
				double dis = sqrt((point.m_colorExt.Red - colorInfo[i].red)*(point.m_colorExt.Red - colorInfo[i].red) +
					(point.m_colorExt.Green - colorInfo[i].green)*(point.m_colorExt.Green - colorInfo[i].green) +
					(point.m_colorExt.Blue - colorInfo[i].blue)*(point.m_colorExt.Blue - colorInfo[i].blue));
				minIdx = minDis>dis ? i : minIdx;
				minDis = min(minDis, dis);
			}
			point.m_classify = colorInfo[i].classType;
		}
	}

	return 0;
}
/////////////////////////////////////


long LASClassifyMemLimited::LASExportClassifiedPoints(const char* pathLas,eLASClassification type,const char* pathExport)
{
    FILE* fLasIn = nullptr, *fLasOut = nullptr;
    fLasIn = fopen(pathLas, "rb");
    fLasOut = fopen(pathExport, "wb");

    if (fLasIn == nullptr || fLasOut == nullptr)
        return -1;

    LASHeader    headerLas;
    LidarPatchReader patchReaderLine;
    patchReaderLine.LidarReader_ReadHeader(fLasIn,headerLas);
    int curPos = ftell(fLasIn);

    int typesNumber=0;
    int pointReserved = headerLas.number_of_point_records;
    const int readPatch = 100000;
    LASPoint* lasPnt = nullptr;
    try
    {
        lasPnt = new LASPoint[readPatch];
    }
    catch (bad_alloc e) {
        printf("%s\n", e.what());
        return -1;
    }

    double xmin=99999999,xmax=-9999999;
    double ymin=99999999,ymax=-9999999;
    double zmin=99999999,zmax=-9999999;

    //get type number　and range first
    int realReadPoints = 0;
    while (!feof(fLasIn)) {
        if (pointReserved < readPatch)
            realReadPoints = pointReserved;
        else
            realReadPoints = readPatch;

        Rect2D rect = patchReaderLine.LidarReader_ReadPatch(fLasIn, headerLas, lasPnt, realReadPoints);
        for(int i=0;i<realReadPoints;++i)
        {
            if(lasPnt[i].m_classify==type) {
                typesNumber++;
                xmin = min(xmin, lasPnt[i].m_vec3d.x);
                xmax = max(xmax, lasPnt[i].m_vec3d.x);
                ymin = min(ymin, lasPnt[i].m_vec3d.y);
                ymax = max(ymax, lasPnt[i].m_vec3d.y);
                zmin = min(zmin, lasPnt[i].m_vec3d.z);
                zmax = max(zmax, lasPnt[i].m_vec3d.z);
            }
        }
        pointReserved=pointReserved-realReadPoints;
    };
    fseek(fLasIn,curPos,SEEK_SET);
    headerLas.min_x=xmin;headerLas.max_x=xmax;
    headerLas.min_y=ymin;headerLas.max_y=ymax;
    headerLas.min_z=zmin;headerLas.max_z=zmax;
    headerLas.number_of_point_records = typesNumber;
    //export
    patchReaderLine.LidarReader_WriteHeader(fLasOut,headerLas);

    realReadPoints = 0;
    while (!feof(fLasIn)) {
        if (pointReserved < readPatch)
            realReadPoints = pointReserved;
        else
            realReadPoints = readPatch;

        Rect2D rect = patchReaderLine.LidarReader_ReadPatch(fLasIn, headerLas, lasPnt, realReadPoints);
        for(int i=0;i<realReadPoints;++i)
        {
            if(lasPnt[i].m_classify==type) {
                patchReaderLine.LidarReader_WritePatch(fLasOut,headerLas,&lasPnt[i],1);
            }
        }
        pointReserved=pointReserved-realReadPoints;
    };
    delete[]lasPnt;lasPnt= nullptr;
    fclose(fLasIn);fLasIn= nullptr;
    fclose(fLasOut);fLasOut= nullptr;
}
//////////////////////////////////

long LASClassifyTower::LASGetTowerLocation(const char* pathLocate)
{
	FILE* plf = fopen(pathLocate, "r+");
	if (plf == nullptr)
		return -1;
	char buffer[2048];
	while (!feof(plf))
	{
		Point2D perTower;
		fgets(buffer, 2048, plf);
		sscanf(buffer, "%lf%lf", &perTower.x, &perTower.y);
		int zone = 50;
		double mx, my;
		tsmLatLongToUTM(perTower.y, perTower.x,&zone,&mx,&my);
		perTower.x = mx;
		perTower.y = my;
		m_towerLocate.push_back(perTower);
	};
	fclose(plf);

	return 0;
}

long LASClassifyTower::LASTowerRect(ILASDataset* dataset, double range, vector<Rect2D> &rectTowers)
{
	if (m_towerLocate.empty())
		return 0;
	long numPnt = 0;
	for (int i = 0; i < m_towerLocate.size(); ++i)
	{
		Rect2D towerRect;
		towerRect.minx = m_towerLocate[i].x - range;
		towerRect.maxx = m_towerLocate[i].x + range;
		towerRect.miny = m_towerLocate[i].y - range;
		towerRect.maxy = m_towerLocate[i].y + range;

		vector<int> searchRect;
		int idPoint = 0;
		dataset->LASDataset_Search(idPoint, towerRect, searchRect);

		for (int j = 0; j < searchRect.size(); ++j)
		{
			int rectInd = searchRect[j];
			if (dataset->m_lasRectangles[rectInd].m_lasPoints_numbers > 0)
			{
				//test
				for (int k = 0; k < dataset->m_lasRectangles[rectInd].m_lasPoints_numbers; ++k)
				{
					dataset->m_lasRectangles[rectInd].m_lasPoints[k].m_classify = elcTowerRange;
					numPnt++;
				}
			}
		}
		rectTowers.push_back(towerRect);
	}
	return numPnt;
}

long LASClassifyTower::LASTowerRough(ILASDataset* dataset, vector<Rect2D> rectTowers, float heightThreshod /*= 10*/)
{
	for (int i = 0; i < rectTowers.size(); ++i)
	{
		printf("%d\n", i);
		vector<int> searchRect;
		int idPoint = 0;
		dataset->LASDataset_Search(idPoint, rectTowers[i], searchRect);
		for (int j = 0; j < searchRect.size(); ++j)
		{
			int idxRect = searchRect[j];
			double avgHeight = 0;
			int num = 0;
			for (int k = 0; k < dataset->m_lasRectangles[idxRect].m_lasPoints_numbers; ++k)
			{
				LASPoint &lasPnt = dataset->m_lasRectangles[idxRect].m_lasPoints[k];
				if (lasPnt.m_classify != elcDeletedPoint)
				{
					++num;
					avgHeight = avgHeight + lasPnt.m_vec3d.z;
				}
			}
			avgHeight /= num;

			for (int k = 0; k < dataset->m_lasRectangles[idxRect].m_lasPoints_numbers; ++k)
			{
				LASPoint &lasPnt = dataset->m_lasRectangles[idxRect].m_lasPoints[k];
				if (lasPnt.m_vec3d.z- avgHeight>heightThreshod)
				{
					lasPnt.m_classify = elcDeletedPoint;
				}
			}
		}
	}
	return 0;
}

long LASClassifyTower::LASTowerRefine(ILASDataset* dataset, double range, float cubeDis/* = 0.5*/, float cubePoints/* = 20*/)
{
	for (int i = 0; i < m_towerLocate.size(); ++i)
	{
		Rect2D towerRect;
		towerRect.minx = m_towerLocate[i].x - range;
		towerRect.maxx = m_towerLocate[i].x + range;
		towerRect.miny = m_towerLocate[i].y - range;
		towerRect.maxy = m_towerLocate[i].y + range;

		vector<int> searchRect;
		int idPoint = 0;
		dataset->LASDataset_Search(idPoint, towerRect, searchRect);

		//get tower range point number
		int numPnt = 0;
		for (int j = 0; j < searchRect.size(); ++j)
		{
			int rectInd = searchRect[j];
			numPnt += dataset->m_lasRectangles[rectInd].m_lasPoints_numbers;
		}

		//allocate memory
		//Point3D *pntPart = new Point3D[numPnt];
		int *idxpnt = new int[numPnt];
		memset(idxpnt, 0, sizeof(int)*numPnt);
		idPoint = 0;
		for (int j = 0; j < searchRect.size(); ++j)
		{
			int idxRect = searchRect[j];
			for (int k = 0; k < dataset->m_lasRectangles[idxRect].m_lasPoints_numbers; ++k)
			{
				//pntPart[idPoint] = dataset->m_lasRectangles[idxRect].m_lasPoints[k].m_vec3d;
				++idPoint;
			}
		}

		//calculate dense on each point
		idPoint = 0;
		for (int j = 0; j < searchRect.size(); ++j)
		{
			int idxRect = searchRect[j];
			for (int k = 0; k < dataset->m_lasRectangles[idxRect].m_lasPoints_numbers; ++k)
			{
				if (dataset->m_lasRectangles[idxRect].m_lasPoints[k].m_classify == elcDeletedPoint)
				{
					++idPoint;
					continue;
				}

				double xmin = dataset->m_lasRectangles[idxRect].m_lasPoints[k].m_vec3d.x - cubeDis;
				double xmax = dataset->m_lasRectangles[idxRect].m_lasPoints[k].m_vec3d.x + cubeDis;
				double ymin = dataset->m_lasRectangles[idxRect].m_lasPoints[k].m_vec3d.y - cubeDis;
				double ymax = dataset->m_lasRectangles[idxRect].m_lasPoints[k].m_vec3d.y + cubeDis;
				double zmin = dataset->m_lasRectangles[idxRect].m_lasPoints[k].m_vec3d.z - cubeDis;
				double zmax = dataset->m_lasRectangles[idxRect].m_lasPoints[k].m_vec3d.z + cubeDis;

				for (int l = 0; l < searchRect.size(); ++l)
				{
					int idxRectIn = searchRect[l];
					for (int m = 0; m < dataset->m_lasRectangles[idxRectIn].m_lasPoints_numbers; ++m)
					{
						Point3D &pntPart = dataset->m_lasRectangles[idxRectIn].m_lasPoints[m].m_vec3d;
						if (pntPart.x > xmin&&pntPart.x<xmax&&pntPart.y>ymin&&pntPart.y<ymax&&pntPart.z>zmin&&pntPart.z < zmax)
						{
							if (dataset->m_lasRectangles[idxRectIn].m_lasPoints[m].m_classify != elcDeletedPoint)
								idxpnt[idPoint]++;
						}
					}
				}
				++idPoint;
			}
		}

		//classify
		idPoint = 0;
		int testNum = 0;
		for (int j = 0; j < searchRect.size(); ++j)
		{
			int idxRectIn = searchRect[j];
			for (int k = 0; k < dataset->m_lasRectangles[idxRectIn].m_lasPoints_numbers; ++k)
			{
				if (dataset->m_lasRectangles[idxRectIn].m_lasPoints[k].m_classify == elcDeletedPoint)
				{
					continue;
					++idPoint;
				}
				if (dataset->m_lasRectangles[idxRectIn].m_lasPoints[k].m_classify != elcDeletedPoint&&idxpnt[idPoint] < cubePoints)
				{
					dataset->m_lasRectangles[idxRectIn].m_lasPoints[k].m_classify = elcDeletedPoint;
					testNum++;
				}
				++idPoint;
			}
		}
		printf("%d\n", testNum);
		//delete[]pntPart; pntPart = NULL;
		delete[]idxpnt;  idxpnt = NULL;
	}
	return 0;
}

///////////////////////////////////
long LASClassifyPaperTower::LASTowerPlaneFit(ILASDataset* dataset, double range, vector<Rect2D> &rectTowers, double *abc)
{
	int numPoints = 0;
	//???????????????????
	double centerx = 0, centery = 0, centerz = 0;
	for (int i = 0; i < rectTowers.size(); ++i)
	{
		vector<int> searchRect;
		dataset->LASDataset_Search(0, rectTowers[i], searchRect);
		for (int j = 0; j < searchRect.size(); ++j)
		{
			int idxRect = searchRect[j];
			for (int k = 0; k < dataset->m_lasRectangles[idxRect].m_lasPoints_numbers; ++k)
			{
				LASPoint &lasPnt = dataset->m_lasRectangles[idxRect].m_lasPoints[k];
				centerx += lasPnt.m_vec3d.x;
				centery += lasPnt.m_vec3d.y;
				centerz += lasPnt.m_vec3d.z;
				++numPoints;
			}
		}
	}
	centerx /= numPoints;
	centery /= numPoints;
	centerz /= numPoints;

	//?????
	MatrixXd Z(numPoints, 1), A(numPoints, 3), X(3, 1);
	numPoints = 0;
	for (int i = 0; i < rectTowers.size(); ++i)
	{
		vector<int> searchRect;
		dataset->LASDataset_Search(0, rectTowers[i], searchRect);
		for (int j = 0; j < searchRect.size(); ++j)
		{
			int idxRect = searchRect[j];
			for (int k = 0; k < dataset->m_lasRectangles[idxRect].m_lasPoints_numbers; ++k)
			{
				LASPoint &lasPnt = dataset->m_lasRectangles[idxRect].m_lasPoints[k];
				Z(numPoints, 0) = lasPnt.m_vec3d.z - centerz;
				A(numPoints, 0) = lasPnt.m_vec3d.x - centerx;
				A(numPoints, 1) = lasPnt.m_vec3d.y - centery;
				A(numPoints, 2) = 1;
				++numPoints;
			}
		}
	}
	X = (A.transpose()*A).inverse()*(A.transpose()*Z);

	abc[0] = X(0, 0);
	abc[1] = X(1, 0);
	abc[2] = X(2, 0);

	return 0;
}

long LASClassifyPaperTower::LASTowerRectExport(ILASDataset* dataset, vector<Rect2D> rectTowers, float cubeDis)
{
	//?��??????????��????
	double xmin = 9999999, xmax = -9999999, ymin = 9999999, ymax = -9999999, zmin = 9999999, zmax = -9999999;
	for (int i = 0; i < rectTowers.size(); ++i)
	{
		vector<int> searchRect;
		dataset->LASDataset_Search(0, rectTowers[i], searchRect);
		for (int j = 0; j < searchRect.size(); ++j)
		{
			int idxRect = searchRect[j];
			for (int k = 0; k < dataset->m_lasRectangles[idxRect].m_lasPoints_numbers; ++k)
			{
				LASPoint &lasPnt = dataset->m_lasRectangles[idxRect].m_lasPoints[k];
				if (lasPnt.m_classify != elcDeletedPoint)
				{
					xmin = min(xmin, lasPnt.m_vec3d.x);
					xmax = max(xmax, lasPnt.m_vec3d.x);
					ymin = min(ymin, lasPnt.m_vec3d.y);
					ymax = max(ymax, lasPnt.m_vec3d.y);
					zmin = min(zmin, lasPnt.m_vec3d.z);
					zmax = max(zmax, lasPnt.m_vec3d.z);
				}
			}
		}
	}

	int xsize = (xmax - xmin) / cubeDis + 1;
	int ysize = (ymax - ymin) / cubeDis + 1;
	int zsize = (zmax - zmin) / cubeDis/2 + 1;
	vector<vector<Point3D>> pnt(xsize*ysize*zsize);

	//export
	for (int i = 0; i < rectTowers.size(); ++i)
	{
		vector<int> searchRect;
		dataset->LASDataset_Search(0, rectTowers[i], searchRect);
		for (int j = 0; j < searchRect.size(); ++j)
		{
			int idxRect = searchRect[j];
			for (int k = 0; k < dataset->m_lasRectangles[idxRect].m_lasPoints_numbers; ++k)
			{
				LASPoint &lasPnt = dataset->m_lasRectangles[idxRect].m_lasPoints[k];
				int idx = (lasPnt.m_vec3d.x - xmin) / cubeDis;
				int idy = (lasPnt.m_vec3d.y - ymin) / cubeDis;
				int idz = (lasPnt.m_vec3d.z - zmin) / cubeDis/4;
				if (idx > xsize-2 || idx<0 || idy>ysize-2 || idy<0 || idz>zsize-2 || idz < 0)
					continue;
				pnt[idz*xsize*ysize + idy*xsize + idx].push_back(lasPnt.m_vec3d);
			}
		}
	}

	for (int i = 0; i < pnt.size(); ++i)
	{
		char name[256];
		sprintf(name, "G:\\Ww\\LAS\\test\\%d.txt", i);
		if (pnt[i].size() > 10)
		{
			FILE* ef = fopen(name, "w+");
			for (int j = 0; j < pnt[i].size(); ++j)
			{
				fprintf(ef, "%lf  %lf  %lf\n", pnt[i][j].x, pnt[i][j].y, pnt[i][j].z);
			}
			fclose(ef); ef = NULL;
		}
	}

	return 0;
}

long LASClassifyPaperTower::LASTowerRoughPlane(ILASDataset* dataset, vector<Rect2D> rectTowers, double* param, float heightThreshod /*= 10*/)
{
	//?????
	int numPoints = 0;
	//???????????????????
	double centerx = 0, centery = 0, centerz = 0;
	for (int i = 0; i < rectTowers.size(); ++i)
	{
		vector<int> searchRect;
		dataset->LASDataset_Search(0, rectTowers[i], searchRect);
		for (int j = 0; j < searchRect.size(); ++j)
		{
			int idxRect = searchRect[j];
			for (int k = 0; k < dataset->m_lasRectangles[idxRect].m_lasPoints_numbers; ++k)
			{
				LASPoint &lasPnt = dataset->m_lasRectangles[idxRect].m_lasPoints[k];
				centerx += lasPnt.m_vec3d.x;
				centery += lasPnt.m_vec3d.y;
				centerz += lasPnt.m_vec3d.z;
			}
			numPoints += dataset->m_lasRectangles[idxRect].m_lasPoints_numbers;
		}
	}
	centerx /= numPoints;
	centery /= numPoints;
	centerz /= numPoints;


	for (int i = 0; i < rectTowers.size(); ++i)
	{
		vector<int> searchRect;
		int idPoint = 0;
		dataset->LASDataset_Search(idPoint, rectTowers[i], searchRect);
		for (int j = 0; j < searchRect.size(); ++j)
		{
			int idxRect = searchRect[j];
			for (int k = 0; k < dataset->m_lasRectangles[idxRect].m_lasPoints_numbers; ++k)
			{
				LASPoint &lasPnt = dataset->m_lasRectangles[idxRect].m_lasPoints[k];
				double z = param[0] * (lasPnt.m_vec3d.x - centerx) + param[1] * (lasPnt.m_vec3d.y - centery) + param[2];

				if (lasPnt.m_vec3d.z - centerz - z>heightThreshod)
				{
					lasPnt.m_classify = elcDeletedPoint;
				}
			}
		}
	}
	return 0;
}

long LASClassifyPaperTower::LASRegionPCAXY(const char* pathRegion, double *pca)
{
	FILE* fLAS = nullptr;
	fLAS = fopen(pathRegion, "r+");
	int num = 0; char line[2048];
	double xx=0, xy=0, yy=0;
	double mx=0, my=0;
	while (!feof(fLAS)) {
		num++;
		fgets(line, 2048, fLAS);
		double x, y, z;
		sscanf(line, "%lf%lf%lf", &x, &y, &z);
		mx += x; my += y;
	};
	fseek(fLAS, 0, SEEK_SET);

	mx /= num;
	my /= num;
	while (!feof(fLAS)) {
		fgets(line, 2048, fLAS);
		double x, y, z;
		sscanf(line, "%lf%lf%lf", &x, &y, &z);
		xx += (x - mx)*(x - mx) / num;
		yy += (y - my)*(y - my) / num;
		xy += (x - mx)*(y - my) / num;
	};
	fclose(fLAS);
	//xx = sqrt(xx);
	//yy = sqrt(yy);
	//xy = sqrt(xy);
	//cov matrix
	MatrixXd m1(2, 2);
	m1(0, 0) = xx; m1(0, 1) = xy;
	m1(1, 0) = xy; m1(1, 1) = yy;
	EigenSolver<Matrix2d> es(m1);
	Matrix2d D = es.pseudoEigenvalueMatrix();
	pca[0] = max(D(0, 0), D(1, 1));
	pca[1] = min(D(0, 0), D(1, 1));
	return 0;
}

long LASClassifyPaperTower::LASRegionPCA(const char* pathDir)
{
	vector<string> paths;
	getFiles(pathDir, paths,"tif");
	double *pca = new double[2 * paths.size()];
	for (int i = 0; i < paths.size(); ++i)
	{
		LASRegionPCAXY(paths[i].c_str(), pca + 2 * i);
	}
	FILE* flas = nullptr;
	flas = fopen("G:\\Ww\\LAS\\pca.csv", "w+");
	for (int i = 0; i < paths.size(); ++i)
	{
		int pos = paths[i].find_last_of('/');
		string str = (paths[i].substr(pos + 1));
		fprintf(flas, "%lf,%lf,%s,\n", pca[2 * i + 0], pca[2 * i + 1], str.c_str());
	}
	fclose(flas);
	delete[]pca; pca = nullptr;
	return 0;
}
