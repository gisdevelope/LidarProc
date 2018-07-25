#include"LASTowerClassify.h"
#include <Eigen/Dense>
#include"../LidarAlgorithm/Geometry.h"
#include"../LidarAlgorithm/GeometryAlgorithm.h"
#include"../LidarBase/tsmToUTM.h"
#include"../LidarBase/LASReader.h"

using namespace Eigen;
using namespace GeometryLas;

#ifdef _ARTICLE__USE_
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
		tsmLatLongToUTM(perTower.y, perTower.x, &zone, &mx, &my);
		perTower.x = mx;
		perTower.y = my;
		m_towerLocate.push_back(perTower);
	};
	fclose(plf);

	return 0;
}

long LASClassifyTower::LASGetTowerLocation()
{
	m_towerLocate.push_back(Point2D(182175.6228, 2481545.586));
	//m_towerLocate.push_back(Point2D(182232.7469, 2481205.475));
	//m_towerLocate.push_back(Point2D(182706.1623, 2481000.313));
	//m_towerLocate.push_back(Point2D(183021.9111, 2480776.1400));
	return 0;
}

long LASClassifyTower::LASTowerRoughTest()
{
	char* pathLas = "E:\\LidarData\\mergecolor.las";
	char* pathExport = "E:\\LidarData\\tower.txt";
	ILASDataset *lasDataset = new ILASDataset();
	LidarMemReader *reader = new LidarMemReader();
	reader->LidarReader_Open(pathLas, lasDataset);
	reader->LidarReader_Read(true, 1, lasDataset);

	vector<Rect2D> rectTowers;
	LASGetTowerLocation();
	LASTowerRect(lasDataset, 10, rectTowers);
	double params[3];
	//z=ax+by+c
	LASTowerPlaneFit(lasDataset, 10, rectTowers, params);
	LASTowerRoughPlane(lasDataset, rectTowers, params);
	LASTowerRefine(lasDataset, 20, 0.5, 25);
	LASTowerRefineClassified(lasDataset, 20, 0.5, 30);
	reader->LidarReader_Export(pathExport, lasDataset, elcDeletedPoint);
	return 0;
}

#endif

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

long LASClassifyTower::LASTowerPlaneFit(ILASDataset* dataset, double range, vector<Rect2D> &rectTowers, double *abc)
{
	int numPoints = 0;
	//去中心化，统计中心坐标
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

	//总点数
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
				if (lasPnt.m_vec3d.z - avgHeight>heightThreshod)
				{
					lasPnt.m_classify = elcDeletedPoint;
				}
			}
		}
	}
	return 0;
}

long LASClassifyTower::LASTowerRoughPlane(ILASDataset* dataset, vector<Rect2D> rectTowers, double* param, float heightThreshod /*= 10*/)
{
	//中心点
	int numPoints = 0;
	//去中心化，统计中心坐标
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

		//calculate dense on each point
		idPoint = 0;
		for (int j = 0; j < searchRect.size(); ++j)
		{
#ifdef _DEBUG
			printf("process rect:%d-%d\n", searchRect.size(), j + 1);
#endif // DEBUG

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
					if (GeometryRelation::IsRectIntersectRect(
						dataset->m_lasRectangles[idxRectIn].m_Rectangle,
						Rect2D(xmin, ymin, xmax, ymax)))
					{
						for (int m = 0; m < dataset->m_lasRectangles[idxRectIn].m_lasPoints_numbers; ++m)
						{
							Point3D &pntPart = dataset->m_lasRectangles[idxRectIn].m_lasPoints[m].m_vec3d;
							if (pntPart.x > xmin&&pntPart.x<xmax&&
								pntPart.y>ymin&&pntPart.y<ymax&&
								pntPart.z>zmin&&pntPart.z < zmax)
							{
								if (dataset->m_lasRectangles[idxRectIn].m_lasPoints[m].m_classify != elcDeletedPoint)
									idxpnt[idPoint]++;
							}
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
				if (dataset->m_lasRectangles[idxRectIn].m_lasPoints[k].m_classify != elcDeletedPoint && idxpnt[idPoint] > cubePoints)
				{
					dataset->m_lasRectangles[idxRectIn].m_lasPoints[k].m_classify = elcDeletedPoint;
					testNum++;
				}
				++idPoint;
			}
		}
		delete[]idxpnt;  idxpnt = NULL;
	}
	return 0;
}

long LASClassifyTower::LASTowerRefineClassified(ILASDataset* dataset, double range, float cubeDis/* = 0.5*/, float cubePoints/* = 20*/)
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

		//calculate dense on each point
		idPoint = 0;
		for (int j = 0; j < searchRect.size(); ++j)
		{
#ifdef _DEBUG
			printf("process rect:%d-%d\n", searchRect.size(), j + 1);
#endif // DEBUG

			int idxRect = searchRect[j];
			for (int k = 0; k < dataset->m_lasRectangles[idxRect].m_lasPoints_numbers; ++k)
			{
				if (dataset->m_lasRectangles[idxRect].m_lasPoints[k].m_classify != elcDeletedPoint)
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
					if (GeometryRelation::IsRectIntersectRect(
						dataset->m_lasRectangles[idxRectIn].m_Rectangle,
						Rect2D(xmin, ymin, xmax, ymax)))
					{
						for (int m = 0; m < dataset->m_lasRectangles[idxRectIn].m_lasPoints_numbers; ++m)
						{
							Point3D &pntPart = dataset->m_lasRectangles[idxRectIn].m_lasPoints[m].m_vec3d;
							if (pntPart.x > xmin&&pntPart.x<xmax&&
								pntPart.y>ymin&&pntPart.y<ymax&&
								pntPart.z>zmin&&pntPart.z < zmax)
							{
								if (dataset->m_lasRectangles[idxRectIn].m_lasPoints[m].m_classify == elcDeletedPoint)
									idxpnt[idPoint]++;
							}
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
				if (dataset->m_lasRectangles[idxRectIn].m_lasPoints[k].m_classify != elcDeletedPoint)
				{
					continue;
					++idPoint;
				}
				if (dataset->m_lasRectangles[idxRectIn].m_lasPoints[k].m_classify == elcDeletedPoint && idxpnt[idPoint] < cubePoints)
				{
					dataset->m_lasRectangles[idxRectIn].m_lasPoints[k].m_classify = elcTowerRange;
					testNum++;
				}
				++idPoint;
			}
		}
		delete[]idxpnt;  idxpnt = NULL;
	}
	return 0;
}

long LASClassifyTower::LASTowerRectExport(ILASDataset* dataset, vector<Rect2D> rectTowers, float cubeDis)
{
	//切块，并导出块中的点云
	double xmin = _MAX_LIMIT_, xmax = _MIN_LIMIT_, ymin = _MAX_LIMIT_, ymax = _MIN_LIMIT_, zmin = _MAX_LIMIT_, zmax = _MIN_LIMIT_;
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
	int zsize = (zmax - zmin) / cubeDis / 2 + 1;
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
				int idz = (lasPnt.m_vec3d.z - zmin) / cubeDis / 4;
				if (idx > xsize - 2 || idx<0 || idy>ysize - 2 || idy<0 || idz>zsize - 2 || idz < 0)
					continue;
				pnt[idz*xsize*ysize + idy * xsize + idx].push_back(lasPnt.m_vec3d);
			}
		}
	}

	for (int i = 0; i < pnt.size(); ++i)
	{
		char name[256];
		sprintf(name, "F:\\LAS\\test\\%d.txt", i);
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