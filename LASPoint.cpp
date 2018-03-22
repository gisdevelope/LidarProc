
//
// Created by wuwei on 17-12-25.
//

#include "LASPoint.h"
/**
* �������ļ��н��������Ʋ���
* @param data
* @param info
*/
void LASPoint::ExtractFromBuffer(const unsigned char* data, const LASHeader& info)
{
	int xyz[3];
	memcpy(&xyz, data, sizeof(int) * 3);

	m_vec3d.x = xyz[0] * info.x_scale_factor + info.x_offset;
	m_vec3d.y = xyz[1] * info.y_scale_factor + info.y_offset;
	m_vec3d.z = xyz[2] * info.z_scale_factor + info.z_offset;
	data += sizeof(int) * 3;

	int size = sizeof(short)     //_intensity
		+ sizeof(unsigned char)
		+ sizeof(unsigned char)  // _classfication
		+ sizeof(char)           // _scanAngleRank
		+ sizeof(unsigned char)  // _userData
		+ sizeof(unsigned short);// _pointSourceID

	memcpy(&m_intensity, data, size);
	data += size;

	if (info.point_data_format == 1)
	{
		size = sizeof(double);
		memcpy(&m_gpsTime, data, size);
		data += size;
	}
	else
		m_gpsTime = 0;

	if (info.HasLASColorExt4())
	{
		size = sizeof(COLORREF);
		COLORREF color;
		memcpy(&color, data, size);
		data += size;

		m_colorExt.Red=(color<<24)>>24;
		m_colorExt.Green=(color<<16)>>24;
		m_colorExt.Blue=(color<<8)>>24;
	}
	if(info.HasLASColorExt6())
	{
		memcpy(&m_colorExt,data,sizeof(LASColorExt));
		data+=sizeof(LASColorExt);
	}

}

void LASPoint::ExportToBuffer(unsigned char* data, const LASHeader& info) const
{
	int x = int((m_vec3d.x - info.x_offset) / info.x_scale_factor);
	int y = int((m_vec3d.y - info.y_offset) / info.y_scale_factor);
	int z = int((m_vec3d.z - info.z_offset) / info.z_scale_factor);

	int size = sizeof(int);
	memcpy(data, &x, size);
	data += size;
	memcpy(data, &y, size);
	data += size;
	memcpy(data, &z, size);
	data += size;
	size = sizeof(short) //_intensity
		+ sizeof(unsigned char)
		+ sizeof(unsigned char)  // _classfication
		+ sizeof(char)  // _scanAngleRank
		+ sizeof(unsigned char)  // _userData
		+ sizeof(unsigned short);// _pointSourceID
	memcpy(data, &m_intensity, size);
	data += size;
	if (info.point_data_format == 1) //��GPSʱ��
	{
		size = sizeof(double);
		memcpy(data, &m_gpsTime, size);
		data += size;
	}
	if (info.HasLASColorExt6())
	{
		size = 3*sizeof(unsigned short);
		memcpy(data, &m_colorExt, size);
		data += size;
	}
}

/*****************************************************************************
* @brief : LAS����LAS���ݼ�
* @author : W.W.Frank
* @date : 2015/11/29 21:50
* @version : version 1.0
*****************************************************************************/
void LASRectBlock::LASRect_AllocateMemory(int lasPoints, bool inMemory, Rect2D rect)
{
	m_lasPoints_numbers = lasPoints;
	try
	{
		if (inMemory)
		{
			m_lasPoints = new LASPoint[lasPoints];
		}
		memcpy(&m_Rectangle, &rect, sizeof(Rect2D));
	}
	catch (bad_alloc &e)
	{
		printf("%s\n", e.what());
		exit(-1);
	}
}


ILASDataset::ILASDataset()
{
	m_lasRectangles = NULL;
	m_lasBlockTree.RemoveAll();
	m_numRectangles = 0;
	m_LASPointID = NULL;
	m_lasvarHeader = NULL;
	m_xrange[0] = -99999999; m_xrange[1] = 99999999;
	m_yrange[0] = -99999999; m_yrange[1] = 99999999;
	m_zrange[0] = -99999999; m_zrange[1] = 99999999;
}
ILASDataset::~ILASDataset()
{
	printf("destructor...\n");
	if (m_lasRectangles != NULL)
		delete[]m_lasRectangles;
	if (m_LASPointID != NULL)
		delete[]m_LASPointID;
	m_LASPointID = NULL;
	m_lasRectangles = NULL;
	m_lasBlockTree.RemoveAll();
}

//LASSet ���ݼ�����
inline bool SearchRectCallback(int id, void* arg)
{
	vector<int> *rect = (vector<int>*)arg;
	rect->push_back(id);
	return true;
}

long ILASDataset::LASDataset_BuildTree()
{
	if (m_lasRectangles == NULL || m_numRectangles == 0)
	{
		printf("no data to build tree\n");
		return -1;
	}
	//�����ݹ���R���ṹ
	for (int i = 0; i < m_numRectangles; ++i)
	{
		double minbound[] = { m_lasRectangles[i].m_Rectangle.minx,m_lasRectangles[i].m_Rectangle.miny };
		double maxbound[] = { m_lasRectangles[i].m_Rectangle.maxx,m_lasRectangles[i].m_Rectangle.maxy };
		m_lasBlockTree.Insert(minbound, maxbound, i);
	}

	return 0;
}

void ILASDataset::LASDataset_AllocateMemory(int lasRects)
{
	m_numRectangles = lasRects;
	try
	{
		m_lasRectangles = new LASRectBlock[lasRects];
	}
	catch (bad_alloc &e)
	{
		printf("%s\n", e.what());
		exit(-1);
	}
}

void ILASDataset::LASDataset_Trim(bool inMemory)
{
	//��ȡ���ݵ�����λ��
	if (inMemory)
	{
		Point3D pntLasSet;
		memset(&pntLasSet, 0, sizeof(Point3D));
		for (int i = 0; i < m_numRectangles; ++i)
		{

			Point3D pntRectSet;
			memset(&pntRectSet, 0, sizeof(Point3D));
			if (m_lasRectangles[i].m_lasPoints_numbers != 0)
			{
				for (int j = 0; j < m_lasRectangles[i].m_lasPoints_numbers; ++j)
				{
					pntRectSet.x += m_lasRectangles[i].m_lasPoints[j].m_vec3d.x / m_lasRectangles[i].m_lasPoints_numbers;
					pntRectSet.y += m_lasRectangles[i].m_lasPoints[j].m_vec3d.y / m_lasRectangles[i].m_lasPoints_numbers;
					pntRectSet.z += m_lasRectangles[i].m_lasPoints[j].m_vec3d.z / m_lasRectangles[i].m_lasPoints_numbers;
					memcpy(&m_lasRectangles[i].m_rectCenter, &pntRectSet, sizeof(Point3D));
				}
			}
			pntLasSet.x += pntRectSet.x / m_numRectangles;
			pntLasSet.y += pntRectSet.y / m_numRectangles;
			pntLasSet.z += pntRectSet.z / m_numRectangles;
		}
		memcpy(&m_SetCenter, &pntLasSet, sizeof(Point3D));
	}
}

//bool ILASDataset::LASDataset_Iterator(callback_operation_points_Ptr ptrFun)
//{
//    const int &numpoints = m_totalReadLasNumber;
//    for(int i=0;i<m_numRectangles;++i)
//    {
//        LASRectBlock &block =  m_lasRectangles[i];
//        for (int j = 0; j < block.m_lasPoints_numbers; ++j) {
//            LASPoint &point = block.m_lasPoints[j];
//            if(GetNumberOfReturns(point.m_rnseByte)>1)
//            {
//                ptrFun(&point);
//            }
//        }
//    }
//    return 0;
//}

bool ILASDataset::LASDataset_Search(int rectID, Rect2D searchRect, vector<int> &rects)
{
	if (m_lasBlockTree.Count() == 0)
		return false;
	else
	{
		double minbound[] = { searchRect.minx,searchRect.miny };
		double maxbound[] = { searchRect.maxx,searchRect.maxy };
		if (m_lasBlockTree.Search(minbound, maxbound, SearchRectCallback, &rects)>0)
			return true;
		return false;
	}
}
bool ILASDataset::LASDataset_Search(int rectID, Point3D searchPnt, vector<int> &rects)
{
	Rect2D searchRect(searchPnt.x, searchPnt.y, searchPnt.x, searchPnt.y);
	if (m_lasBlockTree.Count() == 0)
		return false;
	else
	{
		double minbound[] = { searchRect.minx,searchRect.miny };
		double maxbound[] = { searchRect.maxx,searchRect.maxy };
		if (m_lasBlockTree.Search(minbound, maxbound, SearchRectCallback, &rects)>0)
			return true;

	}
	return false;
}
//����˳�δ����ҵ���Ӧ�ĵ�
bool ILASDataset::LASDataset_Search(int pointID, Point3D &searchPnt)
{
	int pntBegIdz = 0, pntEndIdx = 0;
	for (int i = 0; i < m_numRectangles; ++i)
	{
		//��ʼ����ֹ���
		pntBegIdz = pntEndIdx;
		pntEndIdx += m_lasRectangles[i].m_lasPoints_numbers - 1;
		if (pointID >= pntBegIdz&&pointID <= pntEndIdx)
		{
			memcpy(&searchPnt, &m_lasRectangles[i].m_lasPoints[pointID - pntBegIdz].m_vec3d, sizeof(Point3D));
			return true;
		}
	}
	return false;
}