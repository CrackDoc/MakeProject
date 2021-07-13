#ifndef ZipCoder_h__
#define ZipCoder_h__

#include <string>

#include "zlib.h"
#include "minizip/zip.h"
#include "minizip/unzip.h"
#include <map>
class CZipCoder
{
public:
	CZipCoder();
	~CZipCoder();

	//添加文件和文件夹到
	static bool AddfiletoZip(zipFile zfile, const std::string& fileNameinZip, const std::string& srcfile);

	static bool UnzipFile(const std::string& strFilePath, const std::string& strTempPath);

	bool Build();

	void SetModuleName(const std::string& strModuleName);

	void SetInstallPath(const std::string& strInstallPath);

	void Push3PartyLibrary(const std::string& Key, const std::string& strFilePath);

	void SetExactInfo(const std::string & strFilePath, const std::string& strTempPath);
private:
	
	std::string m_strModulName;
	std::string m_strInstallPath;

	std::string m_strFilePath;

	std::string m_strTempPath;

	// 第三方库
	std::map<std::string, std::string> m_3PatryLibMap;

	typedef std::map<std::string, std::string>::iterator _3PartyItr;

};
#endif // ZipCoder_h__