#pragma once
//
// Created by wuwei on 18-1-24.
//

#ifndef LASGUI_LASCORLORMAP_H
#define LASGUI_LASCORLORMAP_H

#include "gdal/gdal_priv.h"
#include "Geometry.h"
#include "LASPoint.h"

class ILASDataset;

/***
* �������ݵ���ɫӳ��ʹ��GDAL���ȡӰ��
*/
class LASColorMap {
public:
	/**
	* ��ɫӳ��
	* @param pathLas
	* @param pathImgs
	* @param pathOutLas
	* @return
	*/
	long LASColorMap_Map(const char* pathLas, vector<string> pathImgs, const char* pathOutLas);

	/**
	* ��ɫӳ��
	* @param pathLas
	* @param pathImg
	* @param pathOutLas
	* @return
	*/
	long LASColorMap_Map(const char* pathLas, const char* pathImg, const char* pathOutLas);

	/**
	 * find images intersect with the range of the las
	 * @param pathLas
	 * @param pathImgDir
	 * @param pImgs
	 * @return
	 */
	long LASCorlorMap_ImageFind(const char* pathLas,const char* pathImgDir,vector<string> &pImgs);

private:
	/**
	* ��ȡӰ��Χ
	* @param m_dataset
	* @return
	*/
	Rect2D LASColorMap_ImageRange(GDALDatasetH m_dataset);

	/**
	* ͼ����������Ӱ������к�֮���ת��
	* @param mx
	* @param my
	* @param adfGeoTransform
	* @param ix
	* @param iy
	* @return
	*/
	bool LASColorMap_MapToImage(double mx, double my, double adfGeoTransform[6], int &ix, int &iy);

	/**
	* ���к�ת������������
	* @param ix
	* @param iy
	* @param adfGeoTransform
	* @param mx
	* @param my
	* @return
	*/
	bool LASColorMap_ImageToMap(int ix, int iy, double *adfGeoTransform, double &mx, double &my);

	/**match patch
	 * map match limited
	 * @param rectPatch
	 * @param lasPnts
	 * @param pImgs
	 * @return
	 */
	bool LASCorlorMap_MapPatch(Rect2D rectPatch,LASPoint* lasPnts,vector<string> &pImgs);
};


#endif //LASGUI_LASCORLORMAP_H

