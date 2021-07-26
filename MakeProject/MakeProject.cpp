#include "MakeProject.h"
#include "QCoreApplication"
#include <iostream>
#include "XDir.h"
#include <QFileDialog>
#include <vector>
#include <QStringList>
#include <QCheckBox>
#include "StlUtil.h"
#include <QFileInfo>
#include <QDesktopServices>

using namespace std;
CMakeProject::CMakeProject(QWidget *parent)
    : QWidget(parent)
	, m_OutPutType(e_Dll)
	, m_ModuleType(e_None)
{
    ui.setupUi(this);

    this->setWindowTitle(tr("Project Guide"));

	m_FlowLayOut = new FlowLayout(ui.LibFrameWidget);
	ui.LibFrameWidget->setLayout(m_FlowLayOut);

	Initialize();

    connect(ui.OKBtn, &QPushButton::clicked, this, &CMakeProject::SlotBuildProject);
	connect(ui.SavePathBtn, &QPushButton::clicked, this, &CMakeProject::SlotOpenSaveFile);
	connect(ui.InstallPathBtn, &QPushButton::clicked, this, &CMakeProject::SlotOpenInstallFile);
	connect(ui.UIBox, &QCheckBox::clicked, this, &CMakeProject::SlotUICheckBox);

	connect(ui.QtBox, &QCheckBox::clicked, this, &CMakeProject::SlotQtCheckBox);
	connect(ui.DllCheckBox, &QCheckBox::clicked, this, &CMakeProject::SlotOutPutCheckBox);
	connect(ui.ExeCheckBox, &QCheckBox::clicked, this, &CMakeProject::SlotOutPutCheckBox);
	connect(ui.LibCheckBox, &QCheckBox::clicked, this, &CMakeProject::SlotOutPutCheckBox);

	connect(ui.ModuleCheckBox, &QCheckBox::clicked, this, &CMakeProject::SlotModuleCheckBox);

	connect(ui.FreshBtn, &QPushButton::clicked, this, &CMakeProject::SlotFreshCheckBox);


	connect(ui.PluginCheckBox,&QCheckBox::clicked, this, &CMakeProject::SlotPluginCheckBox);
}

void CMakeProject::resizeEvent(QResizeEvent* event)
{

}

void CMakeProject::SlotBuildProject()
{
	std::string strInstallPath = ui.InstallPathEdit->text().toLocal8Bit();
	IOx::XDir xDir(strInstallPath.c_str());

	if (!xDir.exist())
	{
		xDir.create();
	}
    QString qstrWorkDir = QCoreApplication::applicationDirPath();

    QString qstrTmpFilePath = ui.SavePathEdit->text();
    std::string  strDestDir = qstrTmpFilePath.toLocal8Bit();
	IOx::XDir xDir0(strDestDir.c_str());
	if (!xDir0.exist())
	{
		xDir0.create();
	}
	CZipCoder coder;
	QString qstrModuleName;
	qstrModuleName = ui.ModuleNameEdit->text();
	coder.SetModuleName(qstrModuleName.toLocal8Bit().data());
    coder.SetInstallPath(strInstallPath);

	QMap<QString, QPair<QString, bool>>::iterator it0 = m_3dPatryMap.begin();
	for (; it0 != m_3dPatryMap.end(); ++it0)
	{
		QPair<QString, bool>& pair = it0.value();
		if (pair.second)
		{
			coder.Push3PartyLibrary(it0.key().toLocal8Bit().data(), pair.first.toLocal8Bit().data());
		}
	}
	std::string strDLLName = "/IxCMAKE.dll";

	if (ui.QtBox->checkState() == Qt::Checked && ui.UIBox->checkState() == Qt::Unchecked)
	{
		strDLLName = "/QtCMAKE.dll";
	}
	else if (ui.QtBox->checkState() == Qt::Checked && ui.UIBox->checkState() == Qt::Checked)
	{
		strDLLName = "/QtUICMAKE.dll";
	}

	if (m_ModuleType == e_Module)
	{
		QString qstrOutput = QString::fromLocal8Bit(strDLLName.c_str());

		qstrOutput.replace(".dll", "Module.dll");

		strDLLName = qstrOutput.toLocal8Bit();

	}
	else if (m_ModuleType == e_Plugin)
	{
		QString qstrOutput = QString::fromLocal8Bit(strDLLName.c_str());

		qstrOutput.replace(".dll", "Plugin.dll");

		strDLLName = qstrOutput.toLocal8Bit();
	}
	else if (m_ModuleType == e_QtPlugin)
	{
		strDLLName = "/QT_PLUGIN.dll";
	}
	std::string strSrcFilePath = qstrWorkDir.toLocal8Bit();
	strSrcFilePath.append(strDLLName);
	coder.SetExactInfo(strSrcFilePath, strDestDir);
	coder.SetOutPutProjectType(m_OutPutType);
	coder.Build();

	QDesktopServices bs;
	bs.openUrl(QString::fromLocal8Bit(strDestDir.c_str()));

}

void CMakeProject::Initialize()
{
    QString qstrWorkDir = QCoreApplication::applicationDirPath();

    ui.SavePathEdit->setText(qstrWorkDir);

	QString qStrLibDir = QString::fromLocal8Bit("D:/IDE/Utility/v142/x64");
    ui.InstallPathEdit->setText(qStrLibDir);
	UpdateLibModel(qStrLibDir);
	UpdateLibView();
}

void CMakeProject::SlotOpenSaveFile()
{
	QString qstrWorkDir = QCoreApplication::applicationDirPath();

	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Files"),
		qstrWorkDir,
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);
	if (!dir.isEmpty())
	{
		ui.SavePathEdit->setText(dir);
	}
}

void CMakeProject::SlotOpenInstallFile()
{
	QString qstrWorkDir = QCoreApplication::applicationDirPath();

	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Files"),
		qstrWorkDir,
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);
	if (!dir.isEmpty())
	{
		ui.InstallPathEdit->setText(dir);
		m_3dPatryMap.clear();
		UpdateLibModel(dir);
		UpdateLibView();
	}
}

void CMakeProject::SlotSelectCheckBox()
{
	QCheckBox* senderCheckBox = qobject_cast<QCheckBox*>(sender());
	if (senderCheckBox)
	{
		QString keyName = senderCheckBox->text();
		if (m_3dPatryMap.contains(keyName))
		{
			QPair<QString, bool>& pair = m_3dPatryMap[keyName];
			pair.second = (senderCheckBox->checkState() == Qt::Checked ? true : false);
		}
	}
}

void CMakeProject::SlotUICheckBox()
{
	if (ui.UIBox->checkState() == Qt::Checked)
	{
		ui.QtBox->setCheckState(Qt::Checked);
	}
	else if (ui.UIBox->checkState() == Qt::Unchecked)
	{
		ui.QtBox->setCheckState(Qt::Unchecked);
	}

	Qt::CheckState checkSt = ui.UIBox->checkState();
	Qt::CheckState pluginSt = ui.PluginCheckBox->checkState();
	if (pluginSt == Qt::Checked && checkSt == Qt::Checked)
	{
		m_ModuleType = e_QtPlugin;
	}
}

void CMakeProject::SlotOutPutCheckBox()
{
	QCheckBox* senderCheckBox = qobject_cast<QCheckBox*>(sender());
	if (nullptr == senderCheckBox)
	{
		return;
	}
	Qt::CheckState checkSt = senderCheckBox->checkState();
	if (checkSt == Qt::Checked&&senderCheckBox == ui.DllCheckBox)
	{
		ui.LibCheckBox->setCheckState(Qt::Unchecked);
		ui.ExeCheckBox->setCheckState(Qt::Unchecked);
		m_OutPutType = e_Dll;
	}
	else if (checkSt == Qt::Checked && senderCheckBox == ui.ExeCheckBox)
	{
		ui.LibCheckBox->setCheckState(Qt::Unchecked);
		ui.DllCheckBox->setCheckState(Qt::Unchecked);
		m_OutPutType = e_Exe;
	}
	else if(checkSt == Qt::Checked && senderCheckBox == ui.LibCheckBox)
	{
		ui.DllCheckBox->setCheckState(Qt::Unchecked);
		ui.ExeCheckBox->setCheckState(Qt::Unchecked);
		m_OutPutType = e_Lib;
	}
}

void CMakeProject::SlotModuleCheckBox()
{
	Qt::CheckState checkSt = ui.ModuleCheckBox->checkState();
	if (checkSt == Qt::Checked)
	{
		m_OutPutType = e_Lib;
		m_ModuleType = e_Module;
		ui.DllCheckBox->setCheckState(Qt::Unchecked);
		ui.ExeCheckBox->setCheckState(Qt::Unchecked);
		ui.LibCheckBox->setCheckState(Qt::Checked);
		ui.PluginCheckBox->setCheckState(Qt::Unchecked);

		ui.DllCheckBox->setEnabled(false);
		ui.ExeCheckBox->setEnabled(false);
		ui.LibCheckBox->setEnabled(false);
	}
	else 
	{	
		m_OutPutType = e_Dll;
		m_ModuleType = e_None;
		ui.DllCheckBox->setCheckState(Qt::Checked);
		ui.ExeCheckBox->setCheckState(Qt::Unchecked);
		ui.LibCheckBox->setCheckState(Qt::Unchecked);

		ui.DllCheckBox->setEnabled(true);
		ui.ExeCheckBox->setEnabled(true);
		ui.LibCheckBox->setEnabled(true);
	}
}

void CMakeProject::SlotFreshCheckBox()
{
	QString qStrLibDir = ui.InstallPathEdit->text();
	UpdateLibModel(qStrLibDir);
	UpdateLibView();
}

void CMakeProject::SlotPluginCheckBox()
{
	Qt::CheckState checkSt = ui.PluginCheckBox->checkState();
	if (checkSt == Qt::Checked)
	{
		ui.ModuleCheckBox->setCheckState(Qt::Unchecked);
		ui.DllCheckBox->setEnabled(false);
		ui.ExeCheckBox->setEnabled(false);
		ui.LibCheckBox->setEnabled(false);
		m_ModuleType = e_Plugin;
	}
	else
	{
		m_ModuleType = e_None;
		ui.DllCheckBox->setEnabled(true);
		ui.ExeCheckBox->setEnabled(true);
		ui.LibCheckBox->setEnabled(true);

	}
	if (ui.QtBox->checkState() == Qt::Checked || ui.UIBox->checkState() == Qt::Checked)
	{
		m_ModuleType = e_QtPlugin;
	}
	m_OutPutType = e_Dll;
	ui.DllCheckBox->setCheckState(Qt::Checked);
	ui.ExeCheckBox->setCheckState(Qt::Unchecked);
	ui.LibCheckBox->setCheckState(Qt::Unchecked);
}

void CMakeProject::SlotQtCheckBox()
{
	Qt::CheckState checkSt = ui.QtBox->checkState();
	Qt::CheckState pluginSt = ui.PluginCheckBox->checkState();
	if (pluginSt == Qt::Checked && checkSt == Qt::Checked)
	{
		m_ModuleType = e_QtPlugin;
	}
}

void CMakeProject::SlotUpdateCheckBoxState()
{

}

void CMakeProject::UpdateLibView()
{
	QLayoutItem* child;
	while ((child = m_FlowLayOut->takeAt(0)) != 0)
	{
		//setParent为NULL，防止删除之后界面不消失
		if (child->widget())
		{
			child->widget()->setParent(NULL);
		}

		delete child;
	}
	QStringList qDirList;
	QMap<QString, QPair<QString, bool>>::iterator it0 = m_3dPatryMap.begin();
	for (; it0 != m_3dPatryMap.end(); ++it0)
	{
		QPair<QString, bool>& pair = it0.value();
		QString val0 = pair.first;
		QString key0 = it0.key();
		QCheckBox* checkBox = new QCheckBox(key0);
		connect(checkBox, &QCheckBox::clicked, this, &CMakeProject::SlotSelectCheckBox);
		m_FlowLayOut->addWidget(checkBox);
		qDirList << val0;
	}
	ui.LibCombbox->clear();
	ui.LibCombbox->addItems(qDirList);
}

void CMakeProject::UpdateLibModel(const QString& qstrLibDir)
{
	IOx::XDir xDir00(qstrLibDir.toLocal8Bit().data());

	std::vector<IOx::XDir> listXDir;

	xDir00.getOneLevelAllDir(&listXDir);

	for (int i = 0; i < listXDir.size(); ++i)
	{
		QFileInfo info(listXDir[i].dirName());
		QString qStrBaseName = info.fileName();
		if (!m_3dPatryMap.contains(qStrBaseName))
		{
			m_3dPatryMap[qStrBaseName] = QPair<QString, bool>(listXDir[i].dirName(), false);
		}
	}
}
