#pragma once

#include"LASSimpleClassify.h"
#include<string>
using namespace std;

//method in my arcticle to extract tower points from points set
#ifdef _ARTICLE__USE_
/**
classify tower points from las dataset
author:wuwei
version:1.0.0.0
**/
class LASClassifyTower :public LASSimpleClassify
{
public:
	/*
	* set position of the tower
	* param x : coordinate x
	* param y : coordinate y
	* @return error code
	*/
	long LASSetTowerLocation(double x, double y);

	//TODO: Get tower position from las dataset
	/*
	* Get tower position from las dataset
	* @param dataset :las point dataset
	*/
	//long LASGetTowerLocation(ILASDataset *dataset);

		/*
	* get tower position 
	* @param pathLocate :path
	*/
	long LASGetTowerLocation(const char* pathLocate);

	/*
	get tower location by pre define
	*/
	long LASGetTowerLocation();


	/**
	* set tower range and in the range set the classify to elcTowerRange
	* @param dataset :lidar dataset(in & out)
	* @param range : classify color info (in)
	* @param rectTowers: rect towers
	* @return error code
	*/
	long LASTowerRect(ILASDataset* dataset, double range, vector<Rect2D> &rectTowers);

	/**
	* classify tower roughly(according to the average height) classify the point to elcTower
	* @param dataset :lidar dataset(in & out)
	* @param rectTowers : range tower
	* @param heightThreshod: height thresthod
	* @return  error code
	*/
	long LASTowerRough(ILASDataset* dataset, vector<Rect2D> rectTowers, float heightThreshold = 10);


	/**
	* calculate the plane of the ground in the tower range
	* @param dataset :lidar dataset(in & out)
	* @param rectTowers: rect towers
	* @param abc : params
	* @return  error code
	*/
	long LASTowerPlaneFit(ILASDataset* dataset, vector<Rect2D> &rectTowers, double *abc);

	/*
	* classify tower according to the plane and 
	  classify the tower point according to the 
	  relative height classify the point to elcTower
	* param dataset:las point dataset
	* param rectTowers:tower rect
	* param param:parameters of the plane
	* param heightThreshod:relatively height thresthold
	* @return  error code
	*/
	long LASTowerRoughPlane(ILASDataset* dataset, vector<Rect2D> rectTowers, double* param, float heightThreshold = 10);


	//�������refine���߼��ǣ�
	//���һ����Χһ����Χ��ͬ�����Ŀ����һ������Ŀ����Ϊ�����ķ�������ȷ�ģ�
	//����������Χһ����Χ��ͬ�����ĿС��һ������Ŀ����Ϊ���������������Ҫ���޳���
	//������ʹ�ù����з��ֲ�û��������
	/**
	* classify tower refine
	* @param dataset :lidar dataset(in & out)
	* @param range : range tower
	* @param cubeDis : range cube
	* @param cubePoints: point thresthod
	* @return
	*/
	long LASTowerRefine(ILASDataset* dataset, double range, float cubeDis = 0.5, float cubePoints = 20);

	/**
	* classify tower refine for classified points
	* @param dataset :lidar dataset(in & out)
	* @param range : range tower
	* @param cubeDis : range cube
	* @param cubePoints: point thresthod
	* @return
	*/
	long LASTowerRefineClassified(ILASDataset* dataset, double range, float cubeDis = 0.5, float cubePoints = 20);

	//------------------------------ͨ�����ϲ��������ϰ벿�ֵĵ㱻��ΪelcTowerUp----------------------------------
	/**
	* export the point classified elcTowerRange as  block
	* @param dataset :dataset(in & out)
	* @param rectTowers : range tower
	* @param cubeDis : range cube
	* @param stepDis: setp distance
	* constant directory "F:\\LAS\\test\\"
	* @return
	*/
	vector<string> LASTowerRectExport(ILASDataset* dataset, const char* pathDir, vector<Rect2D> rectTowers, float cubeDis, float stepDis);


	/**
	* calculate each block and if above the threshold output the block
	* @param pathBlockDir:block file directory
	* @param threshold : range tower
	* @param pathMerge : merge points file
	* @return error code
	*/
	long LASTowerEigenClassify(const char* pathBlockDir, double threshold, const char* pathMerge);

	/**
	* Filter the vegetation points and left tower points
	* param datasetMerge: merge dataset of the points
	* param pathSeedTower
	*/
	long LASTowerSegDBScan(ILASDataset *datasetMerge, double segDis, const char* pathSeedTower);

	/*
		�������ӵ�Ը�����������
	*/
	long LASTowerSeed(ILASDataset* dataset, double hDis, double vDis, Point3Ds seedPoints, const char* pathOut);
	long LASTowerSeed(ILASDataset* dataset, double hDis, double vDis, const char* seedPath, const char* pathOut);
	//------------------------����Ӧ����ȡ�˸��������������ˣ�Ȼ���ٽ�������--------------------------
	/**
	* merge the towerup and tower bottom to construct a whole tower
	* param datasetUp:points of tower up
	* param datasetDown:points of tower down
	* param pathOut:output points path 
	**/
	long LASTowerTotalMerge(ILASDataset *datasetUp, ILASDataset *datasetDown, const char* pathOut);
	
	/*
	get tower localtion test
	*/
	long LASTowerRoughTest();

private:
	/*
	* calculate the Eigen of each block point
	* param strFile:block point file
	* param eigen:eigen values
	*/
	long calculateEigen(string strFile, vector<double> &eigen);

	
	//tower position
	vector<Point2D> m_towerLocate;


};
#endif

