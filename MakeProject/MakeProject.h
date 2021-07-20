#pragma once

#include <QtWidgets/QWidget>
#include "ui_MakeProject.h"
#include "flowlayout.h"
#include <QMap>
#include <QPair>
#include "ZipCoder.h"

class CMakeProject : public QWidget
{
    Q_OBJECT

public:
    CMakeProject(QWidget *parent = Q_NULLPTR);

    /**
    * @fn       Initialize()
    * @author   Crack
    * @brief    初始化界面参数
    * @date     2021/7/12 8:53
    * @param
    * @return
    */
    void Initialize();
    /**
     * @fn       UpdateLibList(const QString &qstrLibDir)
     * @author   Crack
     * @brief
     * @date     2021/7/12 12:24
     * @param
     * @return
    */
    void UpdateLibModel(const QString& qstrLibDir);

    /**
     * @fn       UpdateLibView
     * @author   Crack
     * @brief
     * @date     2021/7/12 16:32
     * @param
     * @return
    */
    void UpdateLibView();

protected:
    virtual void resizeEvent(QResizeEvent* event);

public slots:
   /**
    * @fn       slotBuildProject
    * @author   Crack
    * @brief     构建工程  
    * @date     2021/7/11 16:22
    * @param    
    * @return   void
   */
    void SlotBuildProject();
    /**
     * @fn       SlotOpenSaveFile
     * @author   Crack
     * @brief       
     * @date     2021/7/12 10:34
     * @param    
     * @return   
    */
    void SlotOpenSaveFile();
    /**
     * @fn       SlotOpenInstallFile
     * @author   Crack
     * @brief       
     * @date     2021/7/12 10:35
     * @param    
     * @return   
    */
    void SlotOpenInstallFile();

    /**
     * @fn       SlotSelectCheckBox();
     * @author   Crack
     * @brief       
     * @date     2021/7/12 17:05
     * @param    
     * @return   
    */
    void SlotSelectCheckBox();

    /**
     * @fn       SlotUICheckBox();
     * @author   Crack
     * @brief       
     * @date     2021/7/13 14:04
     * @param    
     * @return   
    */
    void SlotUICheckBox();
    /**
     * @fn       SlotOutPutCheckBox
     * @author   Crack
     * @brief       
     * @date     2021/7/19 13:13
     * @param    
     * @return   
    */
    void SlotOutPutCheckBox();
    /**
     * @fn       SlotExportCheckBox
     * @author   Crack
     * @brief       
     * @date     2021/7/19 23:15
     * @param    
     * @return   
    */
    void SlotExportCheckBox();
private:
    Ui::CMakeProjectClass ui;
    FlowLayout* m_FlowLayOut;
    QMap<QString, QPair<QString,bool>> m_3dPatryMap;

    E_ProjectType m_OutPutType;

    bool m_bOutPut;
};
