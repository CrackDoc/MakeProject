#include "MakeProject.h"
#include <QtWidgets/QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	QTranslator* trans = new QTranslator;
	trans->load("D:/VsWorkspace/git/source/repos/MakeProject/MakeProject/China_zh.qm");
	a.installTranslator(trans);
    CMakeProject w;
    w.show();
    return a.exec();
}
