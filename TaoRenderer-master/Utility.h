#pragma once

#include <iostream>
#include <fstream>
#include <io.h>

using std::string;
using std::ifstream;

/*
	�ļ����ݣ�
	-�ļ��Ķ�ȡ����
	-���һ���޸����ڣ�2024.11.18
*/

#pragma region �ļ�����

// ��ȡ�ļ��ĺ�׺���磺 .obj
inline string GetFileExtension(const string& file_name) {
	return file_name.substr(file_name.find_last_of('.'));
}
// ��ȡ�ļ������ƣ��޺�׺
inline string GetFileNameWithoutExtension(const string& file_name) {
	// ���ҵ����һ�� �� / ����֮������ļ�������չ��
	string name;
	string::size_type start_pos = file_name.find_last_of('/') + 1;
	string name_with_extension = file_name.substr(start_pos);
	name = name_with_extension.substr(0, name_with_extension.find_last_of('.'));
	return name;
}

// ��ȡ�ļ����ڵ��ļ��У����û��"/"
inline string GetFileFolder(const std::string& file_name)
{
	const string::size_type end = file_name.find_last_of('/');
	string file_folder = file_name.substr(0, end);
	return file_folder;
}

// ��ָ���ļ����м��������а���file_name���ļ�������������·��
inline string GetFilePathByFileName(const string& file_folder, const string& file_name) {
	string complete_path;
	long long h_file; // �ļ����
	struct _finddata_t file_info; // �ļ�����
	string p;
	if ((h_file = _findfirst(p.assign(file_folder).append("/*").c_str(), &file_info)) != -1) {
		do
		{
			std::string temp_file_name = file_info.name;
			if (temp_file_name.find(file_name) != std::string::npos)
			{
				complete_path = (p.assign(file_folder).append("/").append(file_info.name));
				return complete_path;
			}
		} while (_findnext(h_file, &file_info) == 0);
		_findclose(h_file);
	}
	return complete_path;
}

// �ж��ļ��Ƿ����
inline bool DoesFileExist(const string& file_name)
{
	ifstream f(file_name.c_str()); // ����c_str�Ƿ��ܻ�ȡ���������������ļ�����
	return f.good();
}
#pragma endregion