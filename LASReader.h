#pragma once
//
// Created by wuwei on 17-12-26.
//

#ifndef LASLIB_LASREADER_H
#define LASLIB_LASREADER_H


#include <stdio.h>
#include "LASHeader.h"
#include "LASPoint.h"

class ILASDataset;

/*��дlidar�ļ�����*/
class LASReader
{
public:
	LASReader() { m_isDatasetOpen = false; m_lasFile = nullptr; }
	~LASReader() {
		m_isDatasetOpen = false;
		if (m_lasFile != nullptr)
			fclose(m_lasFile);
	}

	//1.��las�ļ�
	virtual long LidarReader_Open(const char* pathLidar, ILASDataset* dataset) = 0;
	//2.��ȡlas�ļ�
	virtual long LidarReader_Read(bool inMemory, int skip, ILASDataset* dataset) = 0;
	//3.��las�ļ�д��
	virtual long LidarReader_Write(const char* pathLidar, ILASDataset* dataset) = 0;
	//4.��ĳһ������д��las�ļ���
	virtual long LidarReader_Write(const char* pathLidar, ILASDataset* dataset, eLASClassification classType) = 0;
	//4.���ý�����
	//virtual void LidarReader_SetProgress(GDALProcessBase* progress) { m_Progress = progress; }

public:
	//����Ϊlas�ļ����ݼ�
	FILE *m_lasFile;
	bool m_isDatasetOpen;
	//GDALProcessBase		*m_Progress;
};

//version 1.2
//author: Frank.Wu
class LidarMemReader : public  LASReader
{
public:
	//1.��las�ļ�
	/*
		�����ݲ���ȡ(������ȡͷ�ļ�)
		@param pathLidar:��������·��
		@param dataset:��ȡ�����ݼ�
	*/
	long LidarReader_Open(const char* pathLidar, _OUT_ ILASDataset* dataset);

	//2.��ȡlas�ļ�
	/*
		��ȡ���ݼ�(�����ݼ���ȡ���ļ���)
		@param inMemory���Ƿ��ȡ���ڴ��У�����ֻ��ȡ������
		@param skip��ѡ���ȡ��Ĳ���Ƶ��
		@param dataset�����ݼ�
	*/
	long LidarReader_Read(bool inMemory, int skip, _OUT_ ILASDataset* dataset);

	//3.��las�ļ�д��
	/*
		��LAS���ݼ�д���ļ���
		@param pathLidar:�ļ�·��
		@param dataset����д������ݼ�
	*/
	long LidarReader_Write(const char* pathLidar, ILASDataset* dataset);

	//4.��ĳһ��las�ļ�д��
	/*
		��ĳһ������ݼ�д���ļ���
		@param pathLidar:�ļ�·��
		@param dataset����д������ݼ�
		@param classType����д����ļ�������
	*/
	long LidarReader_Write(const char* pathLidar, ILASDataset* dataset, eLASClassification classType);
	
	//5.las�ļ���ͬһ����ɫд��
	/*
		�����е���������ĳһ����ɫд���ļ���
		@param pathLidar:�ļ�·��
		@param dataset����д������ݼ�
		@param classType����ɫ��Ϣ
	*/
	long LidarReader_WriteWithColor(const char* pathLidar, ILASDataset* dataset, LASColorExt color);
	
	//6.��LAS�ļ��ϲ���ȡ
	/*
		��ȡ���las�ļ���Ϣ��һ��dataset��
		@param lasFiles���ļ�����
		@param dataset����ȡ�������ݼ�
	*/
	long LidarReader_ReadMerge(std::vector<string> lasFiles, _OUT_ ILASDataset* dataset);

	//7.���ݷ��������������
	long LidarReader_Export(const char* pathLidar, ILASDataset* dataset, int classType);

protected:
	//�����ȸ�С��ʹ�ô��븴���Ը���
	/*
	*	@param fs:�ļ�ָ��
	*	@param dataset�����ݼ�
	*	@param widthPre��������
	*	@param heightPre������߶�
	*	@param widthNum����������
	*	@param heightNum����������
	*	@param skip����ȡ�ļ�������
	*	@param numPtsRect��ÿһ�������е�ĸ���
	*/
	long LidarReader_RectNumbers(FILE* fs, ILASDataset* dataset,double widthPre,double heightPre,
		                         int widthNum,int heightNum, int skip,_OUT_ int *numPtsRect);

	/*
	*   ��ȡÿ��Rect�еĵ������ݶ�ȡ��һ��ָ����
	*	@param fs:�ļ�ָ��
	*	@param dataset�����ݼ�
	*	@param widthPre��������
	*	@param heightPre������߶�
	*	@param widthNum����������
	*	@param heightNum����������
	*	@param skip����ȡ�ļ�������
	*	@param numPtsRect��ÿһ�������е�ĸ���
	*/
	long LidarReader_RectPoints(FILE* fs, ILASDataset* dataset, double widthPre, double heightPre,
								 int widthNum, int heightNum,bool inMemory, int skip, int &totallasPnts, _OUT_ int *pointsRect);


	/*
	*	��ȡÿ��Rect�еĵ������ݶ�ȡ��vector��
	*	@param fs:�ļ�ָ��
	*	@param dataset�����ݼ�
	*	@param widthPre��������
	*	@param heightPre������߶�
	*	@param widthNum����������
	*	@param heightNum����������
	*	@param skip����ȡ�ļ�������
	*/
	long LidarReader_RectPoints(FILE* fs, ILASDataset* dataset, double widthPre, double heightPre,
		int widthNum, int heightNum, bool inMemory, int skip, int &totallasPnts);
};

/*
	�ֿ��ȡ�Ĺ��ܣ�
	create by Frank.Wu
*/
class LidarPatchReader : public LidarMemReader
{
public:
	/**
	* ��ȡLASͷ
	* @param lf
	* @param lasHeader
	* @return
	*/
	long LidarReader_ReadHeader(FILE* lf, LASHeader &lasHeader);

	/**
	* дLASͷ
	* @param lf
	* @param lasHeader
	* @return
	*/
	long LidarReader_WriteHeader(FILE* lf, LASHeader lasHeader);

	/**
	* ��ȡһ��Patch
	* @param lf
	* @param lasHeader
	* @param points
	* @param number
	* @return patch�ķ�Χ
	*/
	Rect2D LidarReader_ReadPatch(FILE* lf, LASHeader lasHeader, LASPoint* points, int &number);

	/**
	* дһ��Patch
	* @param lf
	* @param lasHeader
	* @param points
	* @param number
	* @return patch�ķ�Χ
	*/
	long LidarReader_WritePatch(FILE* lf, LASHeader lasHeader, LASPoint* points, int number);
};

class LidarReaderTxt :public LidarMemReader {
	/**
	* careate LAS header
	* @param lf
	* @param lasHeader
	* @return
	*/
	long LidarReader_Open(const char* pathLidar, ILASDataset* dataset);

	/*
		Read las from txt file
	*/
	long LidarReader_Read(bool inMemory, int skip, ILASDataset* dataset);

};


#endif //LASLIB_LASREADER_H000
