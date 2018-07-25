#pragma once
#include <vector>
using namespace std;
class FileHelper
{
public:
	FileHelper();
	~FileHelper();

	/*
		����Ŀ¼�������ļ�������Ŀ¼����Ŀ¼��
		@param dir������Ŀ¼�� \\����
	*/
	static void listFilesIncludeSubDir(const char * dir);  //����Ŀ¼�е������ļ�(������Ŀ¼)

	/*
		����Ŀ¼�µ������ļ�(��������Ŀ¼)
		@param dir�������ļ�Ŀ¼ ��\\����
		@param fileNameList�����ļ�Ŀ¼���浽������
	*/
	static void listFiles(const char * dir, vector<string>& fileNameList);  //����Ŀ¼�µ������ļ�(��������Ŀ¼)
	
	/*
		����Ŀ¼�µ������ļ�(��������Ŀ¼)
		@param dir�������ļ�Ŀ¼ ��\\����
		@param files�����ļ�Ŀ¼���浽������-�ļ�ȫ��·��
		@param ext����׺��.xxx
	*/
	static void listFiles(string cate_dir, vector<string> &files, string ext);//��ȡ�ļ����������ض���׺��Ŀ¼
	
	/*
		����Ŀ¼�µ������ļ�(��������Ŀ¼)
		@param dir�������ļ�Ŀ¼ ��\\����
		@param files�����ļ�Ŀ¼���浽������-�ļ���
		@param ext����׺��.xxx
	*/
	static void listNames(string cate_dir, vector<string> &files, string ext);//��ȡ�ļ���
};

