#include "ZipCoder.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <QString>
#include <QByteArray>
#include <QFileInfo>
#include "QDir"
#include "XDir.h"
#include "StlUtil.h"
#include <QFile>

using namespace std;

const QString strOutPut[]
= { "OUTPUT_TYPE dll","OUTPUT_TYPE exe","OUTPUT_TYPE lib" };
CZipCoder::CZipCoder()
	:m_eProjectType(e_Dll)
{

}

CZipCoder::~CZipCoder()
{

}
/*
* 函    数 :AddfiletoZip
* 函数功能 :添加文件到要打包的zipFile对象的文件里
* 参数备注 :参数zfile           表示zipFile文件对象
*			参数fileNameinZip   表示要压缩的文件路径
*			参数srcfile         表示要压缩的文件（为空表示为空目录）
*/
bool CZipCoder::AddfiletoZip(zipFile zfile, const std::string& fileNameinZip, const std::string& srcfile)
{
	if (NULL == zfile || fileNameinZip.empty()/* || srcfile.empty()为空代表空目录*/)
	{
		return 0;
	}
	int nErr = 0;
	zip_fileinfo zinfo = { 0 };
	tm_zip tmz = { 0 };
	zinfo.tmz_date = tmz;
	zinfo.dosDate = 0;
	zinfo.internal_fa = 0;
	zinfo.external_fa = 0;

	char sznewfileName[MAX_PATH] = { 0 };
	memset(sznewfileName, 0x00, sizeof(sznewfileName));
	strcat(sznewfileName, fileNameinZip.c_str());
	if (srcfile.empty())
	{
		strcat(sznewfileName, "\\");
	}

	nErr = zipOpenNewFileInZip(zfile, sznewfileName, &zinfo, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
	if (nErr != ZIP_OK)
	{
		return false;
	}
	if (!srcfile.empty())
	{
		//打开源文件
		FILE* srcfp = _fsopen(srcfile.c_str(), "rb", _SH_DENYNO);
		if (NULL == srcfp)
		{
			return false;
		}
		//读入源文件写入zip文件
		int numBytes = 0;
		char* pBuf = new char[1024 * 100];
		if (NULL == pBuf)
		{
			//std::cout << "new buffer failed." << std::endl;
			std::string strLog = "ZKTeco-AddfiletoZip--New buffer failed. error = ";
			int nErr = GetLastError();
			strLog += std::to_string(nErr);
			cout << strLog;
			return 0;
		}
		while (!feof(srcfp))
		{
			memset(pBuf, 0x00, sizeof(pBuf));
			numBytes = fread(pBuf, 1, sizeof(pBuf), srcfp);
			nErr = zipWriteInFileInZip(zfile, pBuf, numBytes);
			if (ferror(srcfp))
			{
				break;
			}
		}
		if (pBuf)
		{
			delete[] pBuf;
			pBuf = NULL;
		}
		if (srcfp)
		{
			fclose(srcfp);
			srcfp = NULL;
		}
	}
	zipCloseFileInZip(zfile);
	return true;
}
bool CZipCoder::UnzipFile(const std::string& strFilePath, const std::string& strTempPath)
{
	int nReturnValue;
	string tempFilePath;
	string srcFilePath(strFilePath);
	string destFilePath;

	std::cout << "Start unpacking the package... " << endl;
	
	if (!stlu::dirExist(strTempPath.c_str()))
	{
		stlu::dirCreate(strTempPath.c_str());
	}
	//打开zip文件
	unzFile unzfile = unzOpen(srcFilePath.c_str());
	if (unzfile == NULL)
	{
		return false;
	}

	//获取zip文件的信息
	unz_global_info* pGlobalInfo = new unz_global_info;
	
	nReturnValue = unzGetGlobalInfo(unzfile, pGlobalInfo);
	if (nReturnValue != UNZ_OK)
	{
		return false;
	}
	//解析zip文件
	unz_file_info* pFileInfo = new unz_file_info;
	char szZipFName[MAX_PATH] = { 0 };
	char szExtraName[MAX_PATH] = { 0 };
	char szCommName[MAX_PATH] = { 0 };
	//存放从zip中解析出来的内部文件名
	for (int i = 0; i < (int)pGlobalInfo->number_entry; i++)
	{
		//解析得到zip中的文件信息
		nReturnValue = unzGetCurrentFileInfo(unzfile, pFileInfo, szZipFName, MAX_PATH, szExtraName, MAX_PATH, szCommName, MAX_PATH);
		if (nReturnValue != UNZ_OK)
		{
			return false;
		}
		string strZipFName = szZipFName;
		if (0 == pFileInfo->uncompressed_size &&(strZipFName.rfind('/') == strZipFName.length() - 1))
		{
			destFilePath = strTempPath + "/" + szZipFName;
			stlu::dirCreate(destFilePath.c_str());
		}
		else
		{
			QString strFile = QString("%0/%1").arg(QString::fromLocal8Bit(strTempPath.c_str()))
				.arg(QString::fromLocal8Bit(szZipFName));
			QFileInfo info(strFile);
			stlu::dirCreate(info.absoluteDir().dirName().toLocal8Bit());
			nReturnValue = unzOpenCurrentFile(unzfile);
			if (nReturnValue != UNZ_OK)
			{
				return false;
			}
			QFile zipQFile(strFile);
			zipQFile.open(QFile::ReadWrite);
			//读取文件
			uLong BUFFER_SIZE = pFileInfo->uncompressed_size;;
			void* szReadBuffer = NULL;
			szReadBuffer = (char*)malloc(BUFFER_SIZE);
			if (NULL == szReadBuffer)
			{
				zipQFile.close();
				break;
			}

			while (TRUE)
			{
				memset(szReadBuffer, 0, BUFFER_SIZE);
				int nReadFileSize = 0;
				nReadFileSize = unzReadCurrentFile(unzfile, szReadBuffer, BUFFER_SIZE);

				if (nReadFileSize < 0)					 //读取文件失败
				{
					unzCloseCurrentFile(unzfile);
					zipQFile.close();
					printf("读取文件失败");
					break;
				}
				else if (nReadFileSize == 0)           //读取文件完毕
				{
					unzCloseCurrentFile(unzfile);
					zipQFile.close();
					printf("读取文件完毕");
					break;
				}
				else									//写入读取的内容
				{
					DWORD dWrite = 0;

					qint64 nSize = zipQFile.write((char*)szReadBuffer, BUFFER_SIZE);

					if (nSize == 0)
					{
						unzCloseCurrentFile(unzfile);
						zipQFile.close();
						break;
					}
				}
			}
			free(szReadBuffer);
		}
		unzGoToNextFile(unzfile);
	}

	if (pFileInfo)
	{
		delete pFileInfo;
		pFileInfo = NULL;
	}
	if (pGlobalInfo)
	{
		delete pGlobalInfo;
		pGlobalInfo = NULL;
	}

	//关闭
	if (unzfile)
	{
		unzClose(unzfile);
	}
	std::cout << "End unpacking the package... " << endl;
	return true;
}
bool CZipCoder::Build()
{
	std::string strFilePath = m_strFilePath;

	QFileInfo info00(QString::fromLocal8Bit(strFilePath.c_str()));

	QString qstrSrcZipName = info00.baseName();

	std::string strTempPath = m_strTempPath;
	int nReturnValue;
	string tempFilePath;
	string srcFilePath(strFilePath);
	string destFilePath;

	std::cout << "Start unpacking the package... " << endl;
	if (!stlu::dirExist(strTempPath.c_str()))
	{
		stlu::dirCreate(strTempPath.c_str());
	}

	//打开zip文件
	unzFile unzfile = unzOpen(srcFilePath.c_str());
	if (unzfile == NULL)
	{
		printf("unzOpen failed.");
		return false;
	}

	//获取zip文件的信息
	unz_global_info* pGlobalInfo = new unz_global_info;

	nReturnValue = unzGetGlobalInfo(unzfile, pGlobalInfo);
	if (nReturnValue != UNZ_OK)
	{
		return false;
	}
	//解析zip文件
	unz_file_info* pFileInfo = new unz_file_info;
	char szZipFName[MAX_PATH] = { 0 };
	char szExtraName[MAX_PATH] = { 0 };
	char szCommName[MAX_PATH] = { 0 };
	//存放从zip中解析出来的内部文件名
	bool isInitCMakeList = false;
	QString qstrModuleCmakePath;

	for (int i = 0; i < (int)pGlobalInfo->number_entry; i++)
	{
		//解析得到zip中的文件信息
		nReturnValue = unzGetCurrentFileInfo(unzfile, pFileInfo, szZipFName, MAX_PATH, szExtraName, MAX_PATH, szCommName, MAX_PATH);
		if (nReturnValue != UNZ_OK)
		{
			return false;
		}
		std::cout << "ZipName: " << szZipFName << "  Extra: " << szExtraName << "  Comm: " << szCommName << endl;

		string strZipFName = szZipFName;
		QString qstrZipFName = QString::fromLocal8Bit(strZipFName.c_str());

		if (!m_strModulName.empty())
		{
			qstrZipFName.replace(qstrSrcZipName, m_strModulName.c_str());

			strZipFName = qstrZipFName.toLocal8Bit().data();

			memset(szZipFName, '\0', sizeof(char) * MAX_PATH);
			strcat(szZipFName, strZipFName.c_str());
		}
		if (0 == pFileInfo->uncompressed_size && (strZipFName.rfind('/') == strZipFName.length() - 1))
		{
			destFilePath = strTempPath + "/" + szZipFName;
			QString qstrDestPath = QString::fromLocal8Bit(destFilePath.c_str());
			if (qstrModuleCmakePath.isEmpty()&& qstrDestPath.contains("CMakeModules"))
			{
				qstrModuleCmakePath = qstrDestPath;
			}
			stlu::dirCreate(destFilePath.c_str());
		}
		else
		{
			QString strFile = QString("%0/%1").arg(QString::fromLocal8Bit(strTempPath.c_str()))
				.arg(QString::fromLocal8Bit(szZipFName));
			QFileInfo info(strFile);
			stlu::dirCreate(info.absoluteDir().dirName().toLocal8Bit());

			nReturnValue = unzOpenCurrentFile(unzfile);

			if (nReturnValue != UNZ_OK)
			{
				return false;
			}

			if (strFile.contains("Model"))
			{
				QString strExternModuleName = QString::fromLocal8Bit(m_strModulName.c_str());
				strExternModuleName.remove("Module");
				strExternModuleName.append("Model");
				if (strFile.contains("IIxCMAKEModel"))
				{
					QString strIxModel = QString("I%0").arg(strExternModuleName);
					strFile.replace("IIxCMAKEModel", strIxModel);
				}
				else 
				{
					QString strIxModel = QString("%0").arg(strExternModuleName);
					strFile.replace("IxCMAKEModel", strIxModel);
				}
			}
			QFile zipQFile(strFile);
			zipQFile.open(QFile::ReadWrite);

			//读取文件
			uLong BUFFER_SIZE = pFileInfo->uncompressed_size;;
			void* szReadBuffer = NULL;
			szReadBuffer = (char*)malloc(BUFFER_SIZE);
			if (NULL == szReadBuffer)
			{
				break;
			}

			while (TRUE)
			{
				memset(szReadBuffer, 0, BUFFER_SIZE);
				int nReadFileSize = 0;
				nReadFileSize = unzReadCurrentFile(unzfile, szReadBuffer, BUFFER_SIZE);

				if (nReadFileSize < 0)					 //读取文件失败
				{
					unzCloseCurrentFile(unzfile);
					zipQFile.close();
					printf("读取文件失败");
					break;
				}
				else if (nReadFileSize == 0)           //读取文件完毕
				{
					unzCloseCurrentFile(unzfile);
					zipQFile.close();
					printf("读取文件完毕");
					break;
				}
				else									//写入读取的内容
				{
					DWORD dWrite = 0;
					qint64 nWirteSize;
					if (qstrZipFName.contains("include") || qstrZipFName.contains("src"))
					{
					
						QByteArray dataArray((char*)szReadBuffer, BUFFER_SIZE);						
						QString strExternModuleName = QString::fromLocal8Bit(m_strModulName.c_str());
						strExternModuleName.remove("Module");

						if (qstrZipFName.contains("Model"))
						{
							QString strExternModuleName = QString::fromLocal8Bit(m_strModulName.c_str());
							strExternModuleName.remove("Module");
							strExternModuleName.append("Model");
							if (dataArray.contains("IIxCMAKEeModel"))
							{
								QString strIxModel = QString("I%0").arg(strExternModuleName);
								dataArray.replace("IIxCMAKEModel", strIxModel.toLocal8Bit().data());
							}
							if (dataArray.contains("IxCMAKEModel"))
							{
								QString strIxModel = QString("%0").arg(strExternModuleName);
								dataArray.replace("IxCMAKEModel", strIxModel.toLocal8Bit().data());
							}
							if(dataArray.contains("CIxCMAKEModel"))
							{
								QString strIxModel = QString("C%0").arg(strExternModuleName);
								dataArray.replace("CIxCMAKEModel", strIxModel.toLocal8Bit().data());
							}
						}
						else
						{
							dataArray.replace(qstrSrcZipName, m_strModulName.c_str());
							dataArray.replace("__IxMdoule__", strExternModuleName.toStdString().c_str());
						}
						
						nWirteSize = zipQFile.write(dataArray);
					}
					else if (qstrZipFName.contains("CMakeLists.txt") && !isInitCMakeList && !m_strInstallPath.empty())
					{
						isInitCMakeList = true;

						QString qstrIDEPath = QString::fromLocal8Bit("\"D:/IDE/Utility/v142/x64\"");
						
						QByteArray dataArray((char*)szReadBuffer, BUFFER_SIZE);
						// repalce dll
						dataArray.replace(strOutPut[0], strOutPut[(int)m_eProjectType].toLocal8Bit().data());

						dataArray.replace(qstrIDEPath, m_strInstallPath.c_str());

						nWirteSize = zipQFile.write(dataArray);
					}
					else if (isInitCMakeList && qstrZipFName.contains("CMakeLists.txt") && !qstrZipFName.contains("UnitTest"))
					{
						QByteArray dataArray((char*)szReadBuffer, BUFFER_SIZE);
						_3PartyItr it0 = m_3PatryLibMap.begin();
						for (; it0 != m_3PatryLibMap.end(); ++it0)
						{
							std::string strName = it0->first;
							QString qstrName = QString::fromLocal8Bit(strName.c_str());
							QString qstrModule = QString::fromLocal8Bit("find_package(%0)\n"
								"if(%0_FOUND)\n"
								"  include_directories(${%0_DIR}/include)\n"
								"  link_directories(${%0_DIR}/lib)\n"
								"  target_link_libraries(${PROJECT_NAME} debug ${%0_LIBRARY_DEBUG})\n"
								"  target_link_libraries(${PROJECT_NAME} optimized ${%0_LIBRARY})\n"
								"endif(%0_FOUND)\n").arg(qstrName);

							dataArray.append(qstrModule);
						}
						nWirteSize = zipQFile.write(dataArray);
					}
					else 
					{
						nWirteSize = zipQFile.write((char*)szReadBuffer, BUFFER_SIZE);
					}
					if (nWirteSize == 0)
					{
						unzCloseCurrentFile(unzfile);
						zipQFile.close();
						break;
					}
				}
			}
			free(szReadBuffer);
		}
		unzGoToNextFile(unzfile);
	}
	// 处理添加的库FindModule.cmake
	QFileInfo info(qstrModuleCmakePath);

	_3PartyItr it0 = m_3PatryLibMap.begin();
	for (; it0 != m_3PatryLibMap.end(); ++it0)
	{
		std::string strName = it0->first;
		QString strFileName = QString("Find") + QString::fromLocal8Bit(strName.c_str()) + QString(".cmake");

		QString strNewFilePath = info.absoluteDir().absolutePath() + "/" + strFileName;

		if(!stlu::fileExist(strNewFilePath.toLocal8Bit().data()))
		{
			QString strIxProject = info.absoluteDir().absolutePath() + "/FindIxProject.cmake";

			QFile::copy(strIxProject, strNewFilePath);

			QFile newFile(strNewFilePath);

			newFile.open(QFile::ReadWrite);

			QByteArray array = newFile.readAll();

			newFile.close();

			QFile::remove(strNewFilePath);

			array.replace("IxProject", strName.c_str());

			QFile repaceFile(strNewFilePath);

			repaceFile.open(QFile::WriteOnly);

			repaceFile.write(array);

			repaceFile.close();
		}
	}
	if (pFileInfo)
	{
		delete pFileInfo;
		pFileInfo = NULL;
	}
	if (pGlobalInfo)
	{
		delete pGlobalInfo;
		pGlobalInfo = NULL;
	}

	//关闭
	if (unzfile)
	{
		unzClose(unzfile);
	}
	std::cout << "End unpacking the package... " << endl;
	return true;
}
void CZipCoder::SetModuleName(const std::string& strModuleName)
{
	m_strModulName = strModuleName;
}

void CZipCoder::SetInstallPath(const std::string& strInstallPath)
{
	m_strInstallPath = strInstallPath;
}

void CZipCoder::Push3PartyLibrary(const std::string& Key, const std::string& strFilePath)
{
	_3PartyItr it0 = m_3PatryLibMap.find(Key);
	if (it0 == m_3PatryLibMap.end())
	{
		m_3PatryLibMap.insert(std::map<std::string, std::string>::value_type(Key, strFilePath));
	}
}
void CZipCoder::SetExactInfo(const std::string& strFilePath, const std::string& strTempPath)
{
	m_strFilePath = strFilePath;
	m_strTempPath = strTempPath;
}

void CZipCoder::SetOutPutProjectType(E_ProjectType eType)
{
	m_eProjectType = eType;
}
