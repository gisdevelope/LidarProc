
//
// Created by wuwei on 17-12-26.
//

#include "LASReader.h"
#include <stdio.h>

#include "LASPoint.h"

#define LargestMemoryToRead 2147483648
#define BlockPointNumbers 2048

long LidarMemReader::LidarReader_Open(const char* pathLidar, ILASDataset* dataset)
{
	assert(dataset != nullptr);

	if (m_lasFile != nullptr)
		fclose(m_lasFile);
	m_lasFile = fopen(pathLidar, "rb");
	if (m_lasFile == nullptr)
		return -1;

	LASHeader &refHeader = dataset->m_lasHeader;
	refHeader.ReadHeader(m_lasFile);
	if (refHeader.number_of_variable_length_records == 0)
	{

	}
	else
	{
		dataset->m_lasvarHeader = new LASVariableRecord[refHeader.number_of_variable_length_records];
		for (int i = 0; i < refHeader.number_of_variable_length_records; ++i)
		{
			dataset->m_lasvarHeader[i].Read(m_lasFile);
		}
	}
	m_isDatasetOpen = true;
	return 0;
}

long LidarMemReader::LidarReader_Read(bool inMemory, int skip, ILASDataset* dataset)
{
	if (!m_isDatasetOpen)
	{
		printf("do not open file\n");
		exit(-1);
	}

	LASHeader &refLasHeader = dataset->m_lasHeader;
	fseek(m_lasFile, refLasHeader.offset_to_point_data, SEEK_SET);
	int totalLasNumber = 0;
	long long  memoryUsed = 0;
	totalLasNumber = refLasHeader.number_of_point_records / skip;
	memoryUsed = totalLasNumber*refLasHeader.point_data_record_length;

	if (inMemory&&memoryUsed > LargestMemoryToRead)
	{
		printf("too large memory to allocate\n");
		printf("advice to upper samples\n");
		return -1;
	}

	int *pointsRect = NULL;
	unsigned char* readOnce = NULL;

	try {
		double width = (refLasHeader.max_x - refLasHeader.min_x);
		double height = (refLasHeader.max_y - refLasHeader.min_y);
		double scaleWH = width / height;
		//ÿ����Ĵ�С
		double widthPer = width / (sqrt(totalLasNumber / BlockPointNumbers / scaleWH)) + 0.5;;
		double heightPer = height / (sqrt(totalLasNumber / BlockPointNumbers * scaleWH)) + 0.5;;
		int widthNum = ceil(width / widthPer);
		int heightNum = ceil(height / heightPer);

		//������ͳ����Ϣ
		//�����ڴ�
		dataset->LASDataset_AllocateMemory(widthNum*heightNum);
		//����R���ṹ
		for (int i = 0; i < widthNum; ++i)
			for (int j = 0; j < heightNum; ++j)
				dataset->m_lasRectangles[j*widthNum + i].m_Rectangle.Set(i*widthPer + refLasHeader.min_x, j*heightPer + refLasHeader.min_y, (i + 1)*widthPer + refLasHeader.min_x, (j + 1)*heightPer + refLasHeader.min_y);

		dataset->LASDataset_BuildTree();

		//ÿһ�����������еĵ������
		pointsRect = new int[widthNum*heightNum];
		memset(pointsRect, 0, sizeof(int)*widthNum*heightNum);
		int alread_read = 0;
		int read_once_max = BlockPointNumbers;
		int read_once = refLasHeader.number_of_point_records;
		long bytelen = read_once_max * refLasHeader.point_data_record_length;

		readOnce = new unsigned char[bytelen];
		totalLasNumber = 0;

		//��һ�α���
		while (alread_read<refLasHeader.number_of_point_records)
		{
			if (read_once>read_once_max)
				read_once = read_once_max;
			int readLen = read_once * refLasHeader.point_data_record_length;
			fread(readOnce, readLen, 1, m_lasFile);

			//�ȶ�ȡ��Ȼ����д���
			for (size_t i = 0; i < read_once; i += skip)
			{
				Point3D vex;
				const unsigned char* data = readOnce + i*refLasHeader.point_data_record_length;
				int size = sizeof(int);
				int x, y, z;
				memcpy(&x, data, size); memcpy(&y, data + size, size); memcpy(&z, data + size * 2, size);
				vex.x = x * refLasHeader.x_scale_factor + refLasHeader.x_offset;
				vex.y = y * refLasHeader.y_scale_factor + refLasHeader.y_offset;
				vex.z = z * refLasHeader.z_scale_factor + refLasHeader.z_offset;

				dataset->m_xrange[0] = max(dataset->m_xrange[0], vex.x); dataset->m_xrange[1] = min(dataset->m_xrange[1], vex.x);
				dataset->m_yrange[0] = max(dataset->m_yrange[0], vex.y); dataset->m_yrange[1] = min(dataset->m_yrange[1], vex.y);
				dataset->m_zrange[0] = max(dataset->m_zrange[0], vex.z); dataset->m_zrange[1] = min(dataset->m_zrange[1], vex.z);

				int widtnIdx = int((vex.x - refLasHeader.min_x) / widthPer);
				int heighIdx = int((vex.y - refLasHeader.min_y) / heightPer);

				//����ûһ��Rect���ж��ٸ���
				pointsRect[heighIdx*widthNum + widtnIdx]++;
				totalLasNumber++;
			}
			alread_read += read_once;
			read_once_max = min(read_once_max, int(refLasHeader.number_of_point_records - alread_read));
		}
		dataset->m_totalReadLasNumber = totalLasNumber;
		dataset->m_LASPointID = new LASIndex[totalLasNumber];
		//�ص��������
		fseek(m_lasFile, refLasHeader.offset_to_point_data, SEEK_SET);


		//�ڶ��������ȡ����
		for (int j = 0; j < widthNum*heightNum; ++j)
			dataset->m_lasRectangles[j].LASRect_AllocateMemory(pointsRect[j], inMemory, dataset->m_lasRectangles[j].m_Rectangle);

		memset(pointsRect, 0, sizeof(int)*widthNum*heightNum);
		int totallasPnts = 0;
		alread_read = 0;
		read_once_max = BlockPointNumbers;
		read_once = refLasHeader.number_of_point_records;

		while (alread_read<refLasHeader.number_of_point_records)
		{
			if (read_once>read_once_max)
				read_once = read_once_max;
			int readLen = read_once * refLasHeader.point_data_record_length;
			fread(readOnce, readLen, 1, m_lasFile);
			//�ȶ�ȡ��Ȼ����д���
			for (size_t i = 0; i < read_once; i += skip)
			{
				unsigned char* data = readOnce + i*refLasHeader.point_data_record_length;
				LASPoint lasPnts;
				lasPnts.ExtractFromBuffer(data, refLasHeader);

				int widtnIdx = int((lasPnts.m_vec3d.x - refLasHeader.min_x) / widthPer);
				int heighIdx = int((lasPnts.m_vec3d.y - refLasHeader.min_y) / heightPer);
				if (inMemory)
					memcpy(dataset->m_lasRectangles[heighIdx*widthNum + widtnIdx].m_lasPoints + pointsRect[heighIdx*widthNum + widtnIdx], &lasPnts, sizeof(LASPoint));
				dataset->m_LASPointID[totallasPnts].rectangle_idx = heighIdx*widthNum + widtnIdx;
				dataset->m_LASPointID[totallasPnts].point_idx_inRect = pointsRect[heighIdx*widthNum + widtnIdx];
				pointsRect[heighIdx*widthNum + widtnIdx]++;
				totallasPnts++;
			}
			alread_read += read_once;
			read_once_max = min(read_once_max, int(refLasHeader.number_of_point_records - alread_read));
		}

#ifdef  ANALYSIS
		FILE* statisticLog = fopen("statistics.log", "w+");
		for (int i = 0; i<widthNum; ++i)
			for (int j = 0; j<heightNum; ++j)
				fprintf(statisticLog, "%d %d %d\n", i, j, pointsRect[j*widthNum + i]);
		fclose(statisticLog);
#endif

	}
	catch (bad_alloc &e)
	{
		printf("%s\n", e.what());
		exit(-1);
	}

	dataset->LASDataset_Trim(inMemory);
	if (pointsRect != NULL)
		delete[]pointsRect;
	if (readOnce != NULL)
		delete[]readOnce;

	pointsRect = NULL;
	readOnce = NULL;

	return 0;
}

long LidarMemReader::LidarReader_Write(const char *pathLidar, ILASDataset* dataset)
{
	assert(dataset != nullptr);

	if (dataset->m_lasRectangles == nullptr)
	{
		printf("no las data\n");
		exit(-1);
	}
	//�½�һ��LASHeader
	LASHeader &refHeader = dataset->m_lasHeader;

	LASHeader lasHeader(refHeader);
	lasHeader.max_x = dataset->m_lasRectangles[0].m_lasPoints[0].m_vec3d.x;
	lasHeader.min_x = dataset->m_lasRectangles[0].m_lasPoints[0].m_vec3d.x;
	lasHeader.max_y = dataset->m_lasRectangles[0].m_lasPoints[0].m_vec3d.y;
	lasHeader.min_y = dataset->m_lasRectangles[0].m_lasPoints[0].m_vec3d.y;
	lasHeader.max_z = dataset->m_lasRectangles[0].m_lasPoints[0].m_vec3d.z;
	lasHeader.min_z = dataset->m_lasRectangles[0].m_lasPoints[0].m_vec3d.z;
	int totalPoints = 0;

	for (size_t i = 0; i < dataset->m_numRectangles; i++)
	{
		for (size_t j = 0; j < dataset->m_lasRectangles[i].m_lasPoints_numbers; ++j)
		{
			lasHeader.max_x = max(dataset->m_lasRectangles[i].m_lasPoints[j].m_vec3d.x, lasHeader.max_x);
			lasHeader.min_x = min(dataset->m_lasRectangles[i].m_lasPoints[j].m_vec3d.x, lasHeader.max_x);
			lasHeader.max_y = max(dataset->m_lasRectangles[i].m_lasPoints[j].m_vec3d.y, lasHeader.max_y);
			lasHeader.min_y = min(dataset->m_lasRectangles[i].m_lasPoints[j].m_vec3d.y, lasHeader.max_y);
			lasHeader.max_z = max(dataset->m_lasRectangles[i].m_lasPoints[j].m_vec3d.z, lasHeader.max_z);
			lasHeader.min_z = min(dataset->m_lasRectangles[i].m_lasPoints[j].m_vec3d.z, lasHeader.max_z);
		}
		totalPoints += dataset->m_lasRectangles[i].m_lasPoints_numbers;
	}
	FILE* fLasOut = fopen(pathLidar, "wb");
	if (fLasOut == nullptr)
		return -1;

	lasHeader.number_of_point_records = totalPoints;
	lasHeader.WriteHeader(fLasOut);
	//�м������� ��֪����ô�� �䳤�ֶλ�û�н��д���
	int sizeBuff = lasHeader.offset_to_point_data - sizeof(LASHeader);
	if (sizeBuff != 0)
	{
		char* buffer = new char[sizeBuff];
		memset(buffer, 0, sizeof(char) * sizeBuff);
		fwrite(buffer, 1, sizeBuff, fLasOut);
		delete[]buffer; buffer = NULL;
	}
	bool isGpsTime = lasHeader.HasGPSTime(), isColorEx = lasHeader.HasLASColorExt6();

	for (int k = 0; k<dataset->m_totalReadLasNumber; ++k)
	{
		int i = dataset->m_LASPointID[k].rectangle_idx;
		int j = dataset->m_LASPointID[k].point_idx_inRect;

		int x = dataset->m_lasRectangles[i].m_lasPoints[j].m_vec3d.x / lasHeader.x_scale_factor - lasHeader.x_offset;
		int y = dataset->m_lasRectangles[i].m_lasPoints[j].m_vec3d.y / lasHeader.y_scale_factor - lasHeader.y_offset;
		int z = dataset->m_lasRectangles[i].m_lasPoints[j].m_vec3d.z / lasHeader.z_scale_factor - lasHeader.z_offset;
		fwrite(&x, sizeof(int), 1, fLasOut);
		fwrite(&y, sizeof(int), 1, fLasOut);
		fwrite(&z, sizeof(int), 1, fLasOut);

		fwrite(&dataset->m_lasRectangles[i].m_lasPoints[j].m_intensity, sizeof(unsigned short), 1, fLasOut);
		fwrite(&dataset->m_lasRectangles[i].m_lasPoints[j].m_rnseByte, sizeof(unsigned char), 1, fLasOut);
		fwrite(&dataset->m_lasRectangles[i].m_lasPoints[j].m_classify, sizeof(unsigned char), 1, fLasOut);
		fwrite(&dataset->m_lasRectangles[i].m_lasPoints[j].m_scanAngle, sizeof(unsigned char), 1, fLasOut);
		fwrite(&dataset->m_lasRectangles[i].m_lasPoints[j].m_userdata, sizeof(unsigned char), 1, fLasOut);
		fwrite(&dataset->m_lasRectangles[i].m_lasPoints[j].m_flightID, sizeof(unsigned short), 1, fLasOut);
		if (isGpsTime)
			fwrite(&dataset->m_lasRectangles[i].m_lasPoints[j].m_gpsTime, sizeof(double), 1, fLasOut);
		if (isColorEx)
			fwrite(&dataset->m_lasRectangles[i].m_lasPoints[j].m_colorExt, sizeof(LASColorExt), 1, fLasOut);
	}
	fclose(fLasOut);
	return 0;
}

long LidarMemReader::LidarReader_Export(const char* pathLidar, ILASDataset* dataset, int classType)
{
	FILE* fw = fopen(pathLidar, "w+");
	if (fw == nullptr)
		return -1;

	bool color = false;
	if (dataset->m_lasHeader.HasLASColorExt4() || dataset->m_lasHeader.HasLASColorExt6())
	{
		color = true;
	}
	for (int k = 0; k<dataset->m_totalReadLasNumber; ++k)
	{
		int i = dataset->m_LASPointID[k].rectangle_idx;
		int j = dataset->m_LASPointID[k].point_idx_inRect;

		if (dataset->m_lasRectangles[i].m_lasPoints[j].m_classify == classType)
		{
			if (color)
			{
				double x = dataset->m_lasRectangles[i].m_lasPoints[j].m_vec3d.x,
					y = dataset->m_lasRectangles[i].m_lasPoints[j].m_vec3d.y,
					z = dataset->m_lasRectangles[i].m_lasPoints[j].m_vec3d.z;

				int r = dataset->m_lasRectangles[i].m_lasPoints[j].m_colorExt.Red,
					g = dataset->m_lasRectangles[i].m_lasPoints[j].m_colorExt.Green,
					b = dataset->m_lasRectangles[i].m_lasPoints[j].m_colorExt.Blue;
				fprintf(fw, "%8.4lf  %8.4lf  %8.4lf  %d  %d %d\n", x, y, z, r, g, b);
			}
			else {
				double x = dataset->m_lasRectangles[i].m_lasPoints[j].m_vec3d.x,
					y = dataset->m_lasRectangles[i].m_lasPoints[j].m_vec3d.y,
					z = dataset->m_lasRectangles[i].m_lasPoints[j].m_vec3d.z;
				fprintf(fw, "%8.4lf  %8.4lf  %8.4lf\n", x, y, z);
			}
		}
	}
	fclose(fw);
	return 0;
}

long LidarPatchReader::LidarReader_ReadHeader(FILE* lf, LASHeader &lasHeader)
{
	lasHeader.ReadHeader(lf);
	//�䳤ͷ
	for (int i = 0; i<lasHeader.number_of_variable_length_records; ++i)
	{
		LASVariableRecord variableRecord;
		variableRecord.Read(lf);
	}
	fseek(lf, lasHeader.offset_to_point_data, SEEK_SET);
	return 0;
}

long LidarPatchReader::LidarReader_WriteHeader(FILE *lf, LASHeader lasHeader)
{
	lasHeader.number_of_variable_length_records = 0;
	lasHeader.version_major = 1;
	lasHeader.version_minor = 2;
	lasHeader.point_data_format = 3;
	lasHeader.point_data_record_length = LASHeader::Data_Record_Length_of_Format3;
	lasHeader.WriteHeader(lf);

	int sizeBuff = lasHeader.offset_to_point_data - sizeof(LASHeader);
	if (sizeBuff != 0)
	{
		char* buffer = new char[sizeBuff];
		memset(buffer, 0, sizeof(char) * sizeBuff);
		fwrite(buffer, 1, sizeBuff, lf);
		delete[]buffer; buffer = NULL;
	}
	return 0;
}

Rect2D LidarPatchReader::LidarReader_ReadPatch(FILE *lf, LASHeader lasHeader, LASPoint *points, int &number)
{
	double dxMin = 99999999, dxMax = -9999999;
	double dyMin = 99999999, dyMax = -9999999;

	int numreader = 0;
	unsigned char* buffer = new unsigned char[lasHeader.point_data_record_length];
	while (!feof(lf)&&numreader<number) {
		fread(buffer, lasHeader.point_data_record_length, 1, lf);
		points[numreader].ExtractFromBuffer(buffer, lasHeader);

		dxMin = min(points[numreader].m_vec3d.x, dxMin);
		dxMax = max(points[numreader].m_vec3d.x, dxMax);

		dyMin = min(points[numreader].m_vec3d.y, dyMin);
		dyMax = max(points[numreader].m_vec3d.y, dyMax);

		++numreader;
	};
	delete[]buffer;

	Rect2D rect;
	rect.minx = dxMin; rect.maxx = dxMax;
	rect.miny = dyMin; rect.maxy = dyMax;
	number = numreader;
	return rect;
}

long LidarPatchReader::LidarReader_WritePatch(FILE* lf, LASHeader lasHeader, LASPoint* points, int number)
{
	lasHeader.number_of_variable_length_records = 0;
	lasHeader.version_major = 1;
	lasHeader.version_minor = 2;
	lasHeader.point_data_format = 3;
	lasHeader.point_data_record_length = LASHeader::Data_Record_Length_of_Format3;

	for (int i = 0; i<number; ++i)
	{
		//points[i].ExportToBuffer(data, lasHeader);
		int x = int((points[i].m_vec3d.x - lasHeader.x_offset) / lasHeader.x_scale_factor);
		int y = int((points[i].m_vec3d.y - lasHeader.y_offset) / lasHeader.y_scale_factor);
		int z = int((points[i].m_vec3d.z - lasHeader.z_offset) / lasHeader.z_scale_factor);
		fwrite(&x, 1,sizeof(int), lf);
		fwrite(&y, 1,sizeof(int), lf);
		fwrite(&z, 1,sizeof(int), lf);
		fwrite(&points[i].m_intensity, sizeof(unsigned short), 1, lf);
		fwrite(&points[i].m_rnseByte, sizeof(unsigned char), 1, lf);
		fwrite(&points[i].m_classify, sizeof(unsigned char), 1, lf);
		fwrite(&points[i].m_scanAngle, sizeof(unsigned char), 1, lf);
		fwrite(&points[i].m_userdata, sizeof(unsigned char), 1, lf);
		fwrite(&points[i].m_flightID, sizeof(unsigned short), 1, lf);
		if (lasHeader.HasGPSTime())
			fwrite(&points[i].m_gpsTime, sizeof(double), 1, lf);
		if (lasHeader.HasLASColorExt6())
		{
			int num = sizeof(LASColorExt);
			fwrite(&points[i].m_colorExt, sizeof(LASColorExt), 1, lf);
		}
	}
	return 0;
}

long LidarPatchReader::LidarReader_SplitPatch(const char* pathLas,const char* pathSplitDir,int splitSize/*MB*/)
{
    ILASDataset *dataset = new ILASDataset();
    LidarReader_Open(pathLas,dataset);

    const LASHeader &lasHeader = dataset->m_lasHeader;
    int numPntRecord = lasHeader.number_of_point_records;
    int numSplit     = splitSize*1024*1024/lasHeader.point_data_record_length;
    int numPart      = ceil((double)numPntRecord/(double)numSplit);
    int reserveNum   = numPntRecord;
    fseek(m_lasFile, lasHeader.offset_to_point_data, SEEK_SET);

    for(int i=0;i<numPart;++i)
    {
        string str(pathSplitDir);
        char name[20];
        sprintf(name,"%d.las",i);
        string path = str+string("/")+string(name);

        int realNum=0;
        if(numPart>numSplit)
            realNum = numSplit;
        else
            realNum = numSplit;

        //read points
        double dxMin = 99999999, dxMax = -9999999;
        double dyMin = 99999999, dyMax = -9999999;
        double dzMin = 99999999, dzMax = -9999999;
        LASPoint* points = new LASPoint[realNum];

        int numreader = 0;
        unsigned char* buffer = new unsigned char[lasHeader.point_data_record_length];
        for(int i=0;i<realNum;++i)
        {
            fread(buffer, lasHeader.point_data_record_length, 1, m_lasFile);
            points[numreader].ExtractFromBuffer(buffer, lasHeader);

            dxMin = min(points[numreader].m_vec3d.x, dxMin);
            dxMax = max(points[numreader].m_vec3d.x, dxMax);
            dyMin = min(points[numreader].m_vec3d.y, dyMin);
            dyMax = max(points[numreader].m_vec3d.y, dyMax);
            dzMin = min(points[numreader].m_vec3d.z, dzMin);
            dzMax = max(points[numreader].m_vec3d.z, dzMax);
        }
        delete[]buffer;buffer= nullptr;

        FILE* fs = fopen(path.c_str(),"wb");
        //construct header
        LASHeader splitHeader;
        splitHeader.number_of_point_records=realNum;
        splitHeader.min_x = dxMin;splitHeader.max_x=dxMax;
        splitHeader.min_y = dyMin;splitHeader.max_y=dyMax;
        splitHeader.min_z = dzMin;splitHeader.max_z=dzMax;

        LidarReader_WriteHeader(fs,splitHeader);
        LidarReader_WritePatch(fs,splitHeader,points,realNum);

        fclose(fs);
        delete[]points;points= nullptr;
    }
    delete dataset;
}
