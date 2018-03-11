#pragma once
//
// Created by wuwei on 18-1-24.
//

#ifndef LASGUI_LASCORLORMAP_H
#define LASGUI_LASCORLORMAP_H

#include "gdal_priv.h"
#include "Geometry.h"

class ILASDataset;

/***
* 点云数据的颜色映射使用GDAL库读取影像
*/
class LASColorMap {
public:
	/**
	* 颜色映射
	* @param pathLas
	* @param pathImgs
	* @param pathOutLas
	* @return
	*/
	long LASColorMap_Map(const char* pathLas, vector<string> pathImgs, const char* pathOutLas);

	/**
	* 颜色映射
	* @param pathLas
	* @param pathImg
	* @param pathOutLas
	* @return
	*/
	long LASColorMap_Map(const char* pathLas, const char* pathImg, const char* pathOutLas);

private:
	/**
	* 获取影像范围
	* @param m_dataset
	* @return
	*/
	Rect2D LASColorMap_ImageRange(GDALDatasetH m_dataset);

	/**
	* 图像地理坐标和影像的行列号之间的转换
	* @param mx
	* @param my
	* @param adfGeoTransform
	* @param ix
	* @param iy
	* @return
	*/
	bool LASColorMap_MapToImage(double mx, double my, double adfGeoTransform[6], int &ix, int &iy);

	/**
	* 行列号转换到地理坐标
	* @param ix
	* @param iy
	* @param adfGeoTransform
	* @param mx
	* @param my
	* @return
	*/
	bool LASColorMap_ImageToMap(int ix, int iy, double *adfGeoTransform, double &mx, double &my);
};


#endif //LASGUI_LASCORLORMAP_H

