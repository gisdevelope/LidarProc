#include "LASHeader.h"
//
// Created by wuwei on 17-12-25.
//

#include <stdio.h>
#include <cstdlib>
#include <string.h>
#pragma warning(disable:4996)
const unsigned short LASHeader::Data_Record_Length_of_Format0 = 20;
const unsigned short LASHeader::Data_Record_Length_of_Format1 = 28;
//��������Ϊ1.2��ʽ��Ӧ�ĵ��ʽ
const unsigned short LASHeader::Data_Record_Length_of_Format2 = 26;
const unsigned short LASHeader::Data_Record_Length_of_Format3 = 34;
const unsigned short LASHeader::HeaderSize_Def = 227;
// const char LASHeader::ErrorPointCnt[] = "number of point is more than LasFileHead.Number_of_point_records";
// const char LASHeader::ErrorVarLenRecordCnt[] = "number of variable Length Record is more than LasFileHead.Number_of_variable_length_records";
// const char LASHeader::ErrorOffsettoData[] = "number of bytes of Variable Length Record is more than LasFileHead.Offset_to_data";

/*****************************************************************************
* @brief : LAS�ļ�ͷ�ļ���д����
* @author : W.W.Frank
* @date : 2015/11/29 18:18
* @version : version 1.0
* @inparam : FILE*
* @outparam :NULL
*****************************************************************************/
LASHeader::LASHeader()
{
	const char tmp[] = "0000";
	for (int i = 0; i < 4; i++)
		file_signature[i] = tmp[i];
	//strcpy_s(file_signature,"LASF");
	// File_Source_ID = 0;
	reserved = 0;
	project_ID_GUID_data_1 = 0;
	project_ID_GUID_data_2 = 0;
	project_ID_GUID_data_3 = 0;
	memset(project_ID_GUID_data_4, 0, 8);
	version_major = 1;
	version_minor = 0;

	memset(system_id, 0, 32);
	strcpy(generating_software, "LidarLab");

	memset(generating_software, 0, 32);
	strcpy(generating_software, "RAMS"); // ��չColor�����к����generating_software����RAMS����TerraSolid��ȡ��Ȼ����

										 //DateTime dt = DateTime::Now();
	file_creation_day = 1;
	file_creation_year = 2015;

	header_size = HeaderSize_Def;
	offset_to_point_data = header_size + 2; // 1.0�汾ͨ���ڿɱ䳤��¼����һ��������0xccdd��1.1ȡ�������������
	number_of_variable_length_records = 0;
	point_data_format = 0;
	point_data_record_length = Data_Record_Length_of_Format0;// + 4;
	number_of_point_records = 0;

	for (int i = 0; i < 5; i++)
		number_of_points_by_return[i] = 0;

	x_scale_factor = 0.01;
	y_scale_factor = 0.01;
	z_scale_factor = 0.001;
	x_offset = 0;
	y_offset = 0;
	z_offset = 0;
	min_x = 0;
	max_x = 0;
	min_y = 0;
	max_y = 0;
	min_z = 0;
	max_z = 0;
}
LASHeader::LASHeader(const LASHeader& header)
{
	memcpy(this, &header, HeaderSize_Def);
}
LASHeader& LASHeader::operator=(const LASHeader& header)
{
	memcpy(this, &header, HeaderSize_Def);
	return *this;
}
void LASHeader::SetFile_Source_ID(unsigned short id)
{
	unsigned int t = id;
	if (version_major == 1 && version_minor == 1)
	{
		reserved &= 0x0000FFFF;
		reserved |= (t << 16);
	}
}
void LASHeader::Setreserved(unsigned int reservedi)
{
	if (version_major == 1 && version_minor == 1)
	{
		reserved &= 0xFFFF0000;
		reserved |= (reserved & 0x0000FFFF);
	}
	else
		reserved = reservedi;
}
bool LASHeader::HasGPSTime() const
{
	if (version_major == 1 && (version_minor == 0 || version_minor == 1))
	{
		if (point_data_format == 1)
			return true;
		else
			return false;
	}
	else if (version_major == 1 && version_minor == 2)//1.2
	{
		if (point_data_format == 1 || point_data_format == 3)
			return true;
		else
			return false;
	}
	else
		exit(1);
}
bool LASHeader::HasLASColorExt4() const
{
	// color: 4 unsigned char
	// lyf modified [20100522] �����˶�1.2��ʽ��֧��
	// ����Ҫ������������ǣ�1��1.0����1.1��ʽ��չ������ɫ��Ϣ(4���ֽ�)
	// 2.1.2��ʽ�Դ�rgb������ɫֵ(3���ֽ�)
	if (version_major == 1 && (version_minor == 0 || version_minor == 1))//1.0 1.1��ʽ
	{
		if (point_data_format == 0)
			return point_data_record_length == Data_Record_Length_of_Format0 + 4 ? true : false;
		else if (point_data_format == 1)
			return point_data_record_length == Data_Record_Length_of_Format1 + 4 ? true : false;
		else{
			printf("format error!");
            exit(0);
        }
	}
	else if (version_major == 1 && version_minor == 2)//1.2��ʽ
	{
		if (point_data_format == 0)
			return point_data_record_length == Data_Record_Length_of_Format0 + 4 ? true : false;
		else if (point_data_format == 1)
			return point_data_record_length == Data_Record_Length_of_Format1 + 4 ? true : false;
		else if (point_data_format == 2)
			return point_data_record_length == Data_Record_Length_of_Format2 + 4 ? true : false;
		else if (point_data_format == 3)
			return point_data_record_length == Data_Record_Length_of_Format3 + 4 ? true : false;
		else{
            printf("format error!");
            exit(0);
        }
	}
	return false;
}
bool LASHeader::HasLASColorExt6() const
{
	if (version_major == 1 && version_minor == 2)
	{
		if (point_data_format == 2 || point_data_format == 3)
			return true;
		else if (point_data_format == 0 || point_data_format == 1)
			return false;
		else{
            printf("format error!");
            exit(0);
        }
	}
	else
		return false;
}
int  LASHeader::HasLASPointExt() const
{
	int len = 0;
	if (point_data_format == 0)
		len = point_data_record_length - LASHeader::Data_Record_Length_of_Format0;
	else if (point_data_format == 1)
		len = point_data_record_length - LASHeader::Data_Record_Length_of_Format1;
	// lyf modified [20100522] �����˶�1.2��ʽ��֧��
	else if (point_data_format == 2)
		len = point_data_record_length - LASHeader::Data_Record_Length_of_Format2;
	else if (point_data_format == 3)
		len = point_data_record_length - LASHeader::Data_Record_Length_of_Format3;
	else{
        printf("format error!");
        exit(0);
    }
	if (len >= 0)
		return len;
	else{
        printf("data point length error");
        exit(0);
    }
}
void LASHeader::ReadHeader(FILE *fs)
{
	//��ȡ��׼�ļ�ͷ����
    //fread(this->file_signature,4,1,fs);
	fread((void*)(this), HeaderSize_Def, 1, fs);
	strcpy(generating_software, "wuwei");

	//��ȡ�ļ�����
	int curPos = ftell(fs);
	fseek(fs, 0, SEEK_END);
	int size = ftell(fs);
	fseek(fs, curPos, SEEK_SET);

	// �����ļ�ͷ��¼�ĵ�����ʵ�ʵ�������
	unsigned int ptsCnt;
	int len = size;
	if ((unsigned int)len <= offset_to_point_data)
		ptsCnt = 0;
	else
		ptsCnt = (len - offset_to_point_data);

	ptsCnt /= point_data_record_length;
	number_of_point_records = ptsCnt;
	if (point_data_record_length == Data_Record_Length_of_Format0)
	{
		point_data_format = 0;
	}
	else if (point_data_record_length == Data_Record_Length_of_Format1)
	{
		point_data_format = 1;
	}
	else if (point_data_record_length == Data_Record_Length_of_Format2)
	{
		point_data_format = 2;
	}
	else if (point_data_record_length == Data_Record_Length_of_Format3)
	{
		point_data_format = 3;
	}
	else
		exit(1);
}
void LASHeader::WriteHeader(FILE *fs) const
{
	fwrite((void*)(this), HeaderSize_Def, 1, fs);
}

/*****************************************************************************
* @brief : LAS�䳤�����ļ���д����
* @author : W.W.Frank
* @date : 2015/11/29 19:33
* @version : version 1.0
* @inparam : FILE*
* @outparam : NULL
*****************************************************************************/
const unsigned short LASVariableRecord::Min_Record_Length_After_Header = min_las_record_after_header;
const short LASVariableRecord::Point_Data_Start_Signature = (short)0xccdd;
void LASVariableRecord::Read(FILE* fs)
{
	LASVariableRecord& record = (*this);
	fread(&record.reserved, sizeof(unsigned short), 1, fs);
	fread(record.user_id, sizeof(char), 16, fs);
	fread(&record.record_id, sizeof(unsigned short), 1, fs);
	fread(&record.record_length_after_header, sizeof(unsigned short), 1, fs);
	fread(record.description, sizeof(char), 32, fs);

	if (record.record_buffer != NULL)
	{
		delete[] record.record_buffer;
		record.record_buffer = NULL;
	}
	int len = record.record_length_after_header;
	if (len > 0)
	{
		record.record_buffer = new unsigned char[len];
		fread(record.record_buffer, sizeof(char), len, fs);
	}
}
void LASVariableRecord::Write(FILE* fs) const
{
	fwrite(&reserved, sizeof(unsigned short), 1, fs);
	fwrite(user_id, sizeof(char), 16, fs);
	fwrite(&record_id, sizeof(unsigned short), 1, fs);
	fwrite(&record_length_after_header, sizeof(unsigned short), 1, fs);
	fwrite(description, sizeof(char), 32, fs);

	int len = record_length_after_header;
	if (len > 0)
	{
		if (record_buffer == NULL)
			exit(1);
		else
			fwrite(record_buffer, sizeof(char), len, fs);
	}
}