#ifndef ZipCoder_h__
#define ZipCoder_h__

#include <string>

#include "zlib.h"
#include "minizip/zip.h"
#include "minizip/unzip.h"
#include <map>

enum E_ProjectType
{
	e_Dll,
	e_Exe,
	e_Lib
};
class CZipCoder
{
public:
	CZipCoder();
	~CZipCoder();

	//����ļ����ļ��е�
	static bool AddfiletoZip(zipFile zfile, const std::string& fileNameinZip, const std::string& srcfile);

	static bool UnzipFile(const std::string& strFilePath, const std::string& strTempPath);

	bool Build();

	void SetModuleName(const std::string& strModuleName);

	void SetInstallPath(const std::string& strInstallPath);

	void Push3PartyLibrary(const std::string& Key, const std::string& strFilePath);

	void SetExactInfo(const std::string & strFilePath, const std::string& strTempPath);

	void SetOutPutProjectType(E_ProjectType eType);

private:
	
	std::string m_strModulName;
	std::string m_strInstallPath;

	std::string m_strFilePath;

	std::string m_strTempPath;

	// ��������
	std::map<std::string, std::string> m_3PatryLibMap;

	typedef std::map<std::string, std::string>::iterator _3PartyItr;

	E_ProjectType m_eProjectType;

};
#endif // ZipCoder_h__