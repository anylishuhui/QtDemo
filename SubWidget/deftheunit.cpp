#include "deftheunit.h"
#include "ui_deftheunit.h"

defTheUnit::defTheUnit(tUnit *unit,QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::defTheUnit)
{
    ui->setupUi(this);

    unitH = new testUnit();

    //初始化窗口界面
    ui->toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    //tab标签栏优化
    ui->tabProperties->tabBar()->setStyle(new CustomTabStyle);
    ui->tabProperties->setCurrentWidget(ui->tabAct);
    ui->tabChkParam->setCurrentWidget(ui->tabCurrent);

    //获取文件中存储的测试单元
    Model_XMLFile xmlRead;
    int len = unitList.length();
    xmlRead.readSequenceXML(configPath("unitDefined.xml"),unitList);

    for(int i=len;i<unitList.length();i++)
    {
        if(userLogin.Permissions != OnlyUser)
            ui->listUnit->addItem(unitList.at(i).name);
    }

    ui->comboBoxActColInfo->appendItem("Interface:Front", false);
    ui->comboBoxActColInfo->appendItem("Interface:Back", false);
    ui->comboBoxActColInfo->appendItem("Picture:Front", false);
    ui->comboBoxActColInfo->appendItem("Picture:Back", false);

    //初始化按键列表
    xmlRead.readKeyInfoXML(WorkItem,&keyList);

    for(int i=0;i<keyList.length();i++)
    {
        if(keyList.at(i).isUse)
            ui->comboKeyList->addItem("KEY"+QString::number(i+1)+":"+keyList.at(i).name);
    }
    ui->comboKeyList->setCurrentIndex(-1);

    //初始化当前测试单元
    if((unit->name.isEmpty())&&(unit->actTest.isEmpty()))
    {
        this->setWindowTitle("Add-Unit");
        //创立之初为空
        on_actnew_triggered();

    }
    else
    {
        this->setWindowTitle("Edit-Unit");
        refreshUnitShow(*unit);
        ui->editUnitName->setEnabled(false);
    }

    //创建窗口组件信号槽函数
    connect(ui->editWaitMin,SIGNAL(textChanged(QString)),this,SLOT(editTimeDealSlot(QString)));
    connect(ui->editWaitMax,SIGNAL(textChanged(QString)),this,SLOT(editTimeDealSlot(QString)));
    connect(ui->editWaitStep,SIGNAL(textChanged(QString)),this,SLOT(editTimeDealSlot(QString)));
    connect(ui->editChkTime,SIGNAL(textChanged(QString)),this,SLOT(editTimeDealSlot(QString)));
    connect(ui->editOverTime,SIGNAL(textChanged(QString)),this,SLOT(editTimeDealSlot(QString)));

    connect(ui->lineEditMixVolt,SIGNAL(textChanged(QString)),this,SLOT(editBatVoltDealSlot(QString)));
    connect(ui->lineEditMaxVolt,SIGNAL(textChanged(QString)),this,SLOT(editBatVoltDealSlot(QString)));
    connect(ui->lineEditStepVolt,SIGNAL(textChanged(QString)),this,SLOT(editBatVoltDealSlot(QString)));

    connect(ui->editDelayMin,SIGNAL(textChanged(QString)),this,SLOT(editDelayTimeDealSlot(QString)));
    connect(ui->editDelayMax,SIGNAL(textChanged(QString)),this,SLOT(editDelayTimeDealSlot(QString)));
    connect(ui->editDelayStep,SIGNAL(textChanged(QString)),this,SLOT(editDelayTimeDealSlot(QString)));

    connect(ui->checkSetCurrent,SIGNAL(clicked(bool)),this,SLOT(editCheckDealSlot(bool)));
    connect(ui->checkSetVolt,SIGNAL(clicked(bool)),this,SLOT(editCheckDealSlot(bool)));
    connect(ui->checkSetSound,SIGNAL(clicked(bool)),this,SLOT(editCheckDealSlot(bool)));
    connect(ui->checkSetLogStr,SIGNAL(clicked(bool)),this,SLOT(editCheckDealSlot(bool)));
    connect(ui->checkSetInterface,SIGNAL(clicked(bool)),this,SLOT(editCheckDealSlot(bool)));
    connect(ui->checkSetPic,SIGNAL(clicked(bool)),this,SLOT(editCheckDealSlot(bool)));

    connect(ui->radioBtnNODeal,SIGNAL(clicked()),this,SLOT(editErrorDealSlot()));
    connect(ui->radioBtnOverTimeDeal,SIGNAL(clicked()),this,SLOT(editErrorDealSlot()));
    connect(ui->radioButtonchkErrorDeal,SIGNAL(clicked()),this,SLOT(editErrorDealSlot()));
    connect(ui->radioButtonchkPassDeal,SIGNAL(clicked()),this,SLOT(editErrorDealSlot()));

    if(userLogin.Permissions == OnlyUser)
    {
        ui->actSave->setEnabled(false);
        ui->actnew->setEnabled(false);
        ui->listUnit->setEnabled(false);
    }
}

defTheUnit::~defTheUnit()
{
    delete unitH;
    delete ui;
}

/*************************************************************
/函数功能：测试单元名修改
/函数参数：
/函数返回：wu
*************************************************************/
void defTheUnit::on_editUnitName_textChanged(const QString &arg1)
{
    unitDeal.name = arg1;
}

/*************************************************************
/函数功能：测试次数修改
/函数参数：
/函数返回：wu
*************************************************************/
void defTheUnit::on_spinUnitCycle_valueChanged(int arg1)
{
    unitDeal.cycleCount = arg1;
}

/*************************************************************
/函数功能：测试单元描述
/函数参数：
/函数返回：wu
*************************************************************/
void defTheUnit::on_editUnitDes_textChanged()
{
    unitDeal.unitDes = ui->editUnitDes->toPlainText();
}

/*************************************************************
/函数功能：tableAction右键创建菜单栏
/函数参数：位置
/函数返回：wu
*************************************************************/
void defTheUnit::on_tableAction_customContextMenuRequested(const QPoint &pos)
{
    Q_UNUSED(pos);
    QMenu *popMenu = new QMenu( this );
    QAction *deleteAct = new QAction(tr("delete"), this);
    QAction *upAct = new QAction(tr("上移"), this);
    QAction *downAct = new QAction(tr("下移"), this);
    QAction *ScriptAction = new QAction(tr("Add Script"), this);
    QAction *BatVoltAction = new QAction(tr("Add BatControl"), this);
    QAction *DelayAction = new QAction(tr("Add WaitTime"), this);

    QMenu *keyMenu = new QMenu("Add Key");
    //定制硬件操作：固定的处理模式，单独处理槽函数
    QAction *ACCONAction = new QAction(("ACCON"), this);
    QAction *ACCOFFAction = new QAction(("ACCOFF"), this);
    QAction *BATONAction = new QAction(("BATON"), this);
    QAction *BATOFFAction = new QAction(("BATOFF"), this);
    QAction *CCDONAction = new QAction(("CCDON"), this);
    QAction *CCDOFFAction = new QAction(("CCDOFF"), this);

    keyMenu->addAction(ACCONAction);
    keyMenu->addAction(ACCOFFAction);
    keyMenu->addAction(BATONAction);
    keyMenu->addAction(BATOFFAction);
    keyMenu->addAction(CCDONAction);
    keyMenu->addAction(CCDOFFAction);


    //添加其他按键：统一处理模式
    for(int i=3;i<keyList.length();i++)
    {
        if(keyList.at(i).isUse)
        {
            keyControl keyInfo = keyList.at(i);
            QAction *keyAction = new QAction(keyInfo.name, this);
            keyMenu->addAction(keyAction);
            connect( keyAction,        SIGNAL(triggered() ), this, SLOT( ActionAppendSlot()) );
        }
    }

    //添加到主菜单：
    if(userLogin.Permissions == Administrator)
    {
        popMenu->addMenu(keyMenu);
        popMenu->addAction(ScriptAction);
        popMenu->addAction(BatVoltAction);
        popMenu->addAction(DelayAction);
        popMenu->addSeparator();
        popMenu->addAction(upAct);
        popMenu->addAction(downAct);
        popMenu->addAction(deleteAct);
    }

    connect( ACCONAction,        SIGNAL(triggered() ), this, SLOT( ActionAppendSlot()) );
    connect( ACCOFFAction,        SIGNAL(triggered() ), this, SLOT( ActionAppendSlot()) );
    connect( BATONAction,        SIGNAL(triggered() ), this, SLOT( ActionAppendSlot()) );
    connect( BATOFFAction,        SIGNAL(triggered() ), this, SLOT( ActionAppendSlot()) );
    connect( CCDONAction,        SIGNAL(triggered() ), this, SLOT( ActionAppendSlot()) );
    connect( CCDOFFAction,        SIGNAL(triggered() ), this, SLOT( ActionAppendSlot()) );
    connect( ScriptAction,        SIGNAL(triggered() ), this, SLOT( ActionAppendSlot()) );
    connect( BatVoltAction,        SIGNAL(triggered() ), this, SLOT( ActionAppendSlot()) );
    connect( DelayAction,        SIGNAL(triggered() ), this, SLOT( ActionAppendSlot()) );

    connect( deleteAct,        SIGNAL(triggered() ), this, SLOT( deleteActionSlot()) );
    connect( upAct,        SIGNAL(triggered() ), this, SLOT( upActionSlot()) );
    connect( downAct,        SIGNAL(triggered() ), this, SLOT( downActionSlot()) );

    popMenu->exec( QCursor::pos() );

    delete popMenu;

}

/*************************************************************
/函数功能：上移
/函数参数：
/函数返回：无
*************************************************************/
void defTheUnit::upActionSlot()
{
    QTableWidgetItem * item = ui->tableAction->currentItem();
    if( item == NULL )
        return;

    int curIndex = ui->tableAction->row(item);

    moveRow(curIndex, curIndex-1 );
}

/*************************************************************
/函数功能：下移
/函数参数：
/函数返回：无
*************************************************************/
void defTheUnit::downActionSlot()
{
    QTableWidgetItem * item = ui->tableAction->currentItem();
    if( item == NULL )
        return;

    int curIndex = ui->tableAction->row(item);
    moveRow(curIndex, curIndex+1 );
}

/*************************************************************
/函数功能：移动行：表格显示及序列排序
/函数参数：
/函数返回：无
*************************************************************/
void defTheUnit::moveRow(int nFrom, int nTo )
{
    if( ui->tableAction == NULL )
        return;
    ui->tableAction->setFocus();

    if( nFrom == nTo )
        return;
    if( nFrom < 0 || nTo < 0 )
        return;
    int nRowCount = tableAction_RowCount();
    if( nFrom >= nRowCount  || nTo > nRowCount )
        return;
    if( nFrom >= unitDeal.actTest.length()  || nTo > unitDeal.actTest.length() )
        return;

    //交换序列
    tAction testAct=unitDeal.actTest.at(nFrom);
    tAction swopAct=unitDeal.actTest.at(nTo);

    for(int i=0;i<testAct.colInfoList.length();i++)
    {
        QString colInfoStr = testAct.colInfoList.at(i);
        colInfoStr.replace("ACT"+toStr(nFrom+1),"ACT"+toStr(nTo+1));
        testAct.colInfoList.replace(i,colInfoStr);
    }

    for(int i=0;i<swopAct.colInfoList.length();i++)
    {
        QString colInfoStr = swopAct.colInfoList.at(i);
        colInfoStr.replace("ACT"+toStr(nTo+1),"ACT"+toStr(nFrom+1));
        swopAct.colInfoList.replace(i,colInfoStr);
    }

    unitDeal.actTest.replace(nFrom,swopAct);
    unitDeal.actTest.replace(nTo,testAct);

    //交换表格显示表格显示
    if( nTo < nFrom )
        nFrom++;
    else
        nTo++;

    ui->tableAction->insertRow( nTo );//即在当前号增加一个空的行，原有数据下移

    //取消链接，避免设置项目时，跳转到处理项目改变槽函数，影响测试动作改变
   // disconnect(ui->tableAction,SIGNAL(itemChanged(QTableWidgetItem*)),this,SLOT(on_tableAction_itemChanged(QTableWidgetItem*)));
    for( int i=0; i<ui->tableAction->columnCount(); i++ )
    {
        ui->tableAction->setItem( nTo, i, ui->tableAction->takeItem( nFrom , i ) );
    }
   // connect(ui->tableAction,SIGNAL(itemChanged(QTableWidgetItem*)),this,SLOT(on_tableAction_itemChanged(QTableWidgetItem*)));

    if( nFrom < nTo  )
        nTo--;

    ui->tableAction->removeRow( nFrom );
    ui->tableAction->selectRow( nTo );
}

/*************************************************************
/函数功能：删除行
/函数参数：
/函数返回：无
*************************************************************/
void defTheUnit::deleteActionSlot()
{
    QTableWidgetItem * item = ui->tableAction->currentItem();
    if( item == NULL )
        return;

    int curIndex = ui->tableAction->row(item);
    ui->tableAction->removeRow(curIndex);          //delete item;因已移除行，无需再删除项目

    if((curIndex>=0)&&(curIndex<unitDeal.actTest.length()))
    {
        unitDeal.actTest.removeAt(curIndex);
        /*删除当前动作后，后面动作的采集标号修改*/
        for(uint8_t i=curIndex;i<unitDeal.actTest.length();i++)
        {
            tAction tempAct = unitDeal.actTest.at(i);
            if(tempAct.colInfoList.isEmpty()==false)
            {
                for(int j=0;j<tempAct.colInfoList.length();j++)
                {
                    QString colInfoStr = tempAct.colInfoList.at(j);
                    colInfoStr.replace("ACT"+toStr(i+2),"ACT"+toStr(i+1));
                    //cout << colInfoStr<<i;
                    tempAct.colInfoList.replace(j,colInfoStr);
                }
                unitDeal.actTest.replace(i,tempAct);
            }
        }
    }
}

/*************************************************************
/函数功能：按键动作添加槽函数
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::ActionAppendSlot()
{
    QAction *editor = qobject_cast<QAction *>(sender());
    QString editorName = editor->text();

    tAction addAct;
    int row=tableAction_RowCount();

    if(editorName == "ACCON")       unitH->actAppend_ACCON(row+1,&addAct,keyList);
    else if(editorName == "ACCOFF") unitH->actAppend_ACCOFF(row+1,&addAct,keyList);
    else if(editorName == "BATON")  unitH->actAppend_BATON(row+1,&addAct,keyList);
    else if(editorName == "BATOFF") unitH->actAppend_BATOFF(row+1,&addAct,keyList);
    else if(editorName == "CCDON")  unitH->actAppend_CCDON(row+1,&addAct,keyList);
    else if(editorName == "CCDOFF") unitH->actAppend_CCDOFF(row+1,&addAct,keyList);
    else if(editorName == "Add Script")unitH->actAppend_script(&addAct);
    else if(editorName == "Add BatControl")unitH->actAppend_batVolt(&addAct);
    else if(editorName == "Add WaitTime")unitH->actAppend_DelayTime(&addAct);
    else
    {
        unitH->actAppend_key(editorName,&addAct,keyList);
    }

    unitDeal.actTest.append(addAct);
    tableAction_Append(addAct);
}

/*************************************************************
/函数功能：行数
/函数参数：无
/函数返回：wu
*************************************************************/
int defTheUnit::tableAction_RowCount()
{
    return ui->tableAction->rowCount();
}

/*************************************************************
/函数功能：清空
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::tableAction_Clear()
{
    for(uint16_t i=ui->tableAction->rowCount();i>0;i--)
    {
        ui->tableAction->removeRow(i-1);
    }
}

/*************************************************************
/函数功能：将动作添加到table中
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::tableAction_Append(tAction act)
{
    int row=ui->tableAction->rowCount();
    ui->tableAction->setRowCount(row+1);

    ui->tableAction->setItem(row,Col_Name,new QTableWidgetItem(act.actName));
    ui->tableAction->setItem(row,Col_Str,new QTableWidgetItem(act.actStr));
    ui->tableAction->selectRow(row);

    refreshPropertiesParam(row,act);
}

/*************************************************************
/函数功能：将动作替换
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::tableAction_ActReplace(int row,tAction repAct)
{
    unitDeal.actTest.replace(row,repAct);
    ui->tableAction->setItem(row,Col_Name,new QTableWidgetItem(repAct.actName));
    ui->tableAction->setItem(row,Col_Str,new QTableWidgetItem(repAct.actStr));
}

/*************************************************************
/函数功能：点击table表格，刷新属性信息
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::on_tableAction_clicked(const QModelIndex &index)
{
    if(index.row()>=unitDeal.actTest.length())
        return ;

    tAction selAction = unitDeal.actTest.at(index.row());

    //刷新属性
    refreshPropertiesParam(index.row(),selAction);
}

/*************************************************************
/函数功能：取table选择行号
/函数参数：无
/函数返回：wu
*************************************************************/
int defTheUnit::tableAction_SelRanges()
{
    int selrow = 0xFFFF;
    //因设置了单选，因此只取一个参数即可
    QList <QTableWidgetSelectionRange>ranges = ui->tableAction->selectedRanges();

    if(ranges.isEmpty()==false)
        selrow = ranges.at(0).topRow();
    return selrow;
}

/*************************************************************
/函数功能：刷新测试单元
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::refreshUnitShow(tUnit unit)
{
    unitDeal = unit;

    ui->editUnitName->setText(unitDeal.name);
    ui->spinUnitCycle->setValue(unitDeal.cycleCount);
    ui->editUnitDes->setText(unitDeal.unitDes);

    tableAction_Clear();//clear

    for(int i=0;i<unitDeal.actTest.length();i++)
    {
        tableAction_Append(unitDeal.actTest.at(i));//append
    }
}

/*************************************************************
/函数功能：刷新属性信息
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::refreshPropertiesParam(int index,tAction act)
{
    //ui->editActName->setText(act.actName);
    refreshColInfo(act.colInfoList);

    //刷新动作属性
    if(act.actFlag == ACT_KEY)
    {
        ui->stackedWidget->setCurrentWidget(ui->pageKey);
        refreshKeyList(act.actStr);
    }
    else if(act.actFlag == ACT_SCRIPT)
    {
        ui->stackedWidget->setCurrentWidget(ui->pageScript);
        ui->editFilePath->setText(act.actStr);

        if((act.actStr.isEmpty())||(act.actStr.endsWith(".bat"))||(act.actStr.endsWith(".BAT")))
            ui->checkfileMore->setChecked(false);
        else
            ui->checkfileMore->setChecked(true);
    }
    else if(act.actFlag == ACT_DELAYTime)
    {
        ui->stackedWidget->setCurrentWidget(ui->pageDelay);
        ui->editDelayMin->setText(act.actStr);
        if(act.changedDeal.isEmpty()==false)
        {
            for(int i=0;i<act.changedDeal.length();i++)
            {
                changedParam cngParam = act.changedDeal.at(i);
                if(cngParam.changed==BatVolt)
                {
                    ui->editDelayMin->setText(toStr(cngParam.min));
                    ui->editDelayMax->setText(toStr(cngParam.max));
                    ui->editDelayStep->setText(toStr(cngParam.step));
                    break;
                }
            }
        }
    }
    else if(act.actFlag == ACT_BATVolt)
    {
        ui->stackedWidget->setCurrentWidget(ui->pageBatVolt);
        ui->lineEditMixVolt->setText(act.actStr.mid(9));
        if(act.changedDeal.isEmpty()==false)
        {
            for(int i=0;i<act.changedDeal.length();i++)
            {
                changedParam cngParam = act.changedDeal.at(i);
                if(cngParam.changed==BatVolt)
                {
                    ui->lineEditMixVolt->setText(toStr(cngParam.min));
                    ui->lineEditMaxVolt->setText(toStr(cngParam.max));
                    ui->lineEditStepVolt->setText(toStr(cngParam.step));
                    break;
                }
            }
        }
    }

    //刷新时间属性
    refreshTimeDeal(act);
    //刷新检测属性
    ui->comboChkParamFace->clear();
    ui->comboChkParamPic->clear();
    for(int i=0;i<index;i++)
    {
        tAction ActDeal = unitDeal.actTest.at(i);

        for(int j=0;j<ActDeal.colInfoList.length();j++)
        {
            if(ActDeal.colInfoList.at(j).contains("Interface"))
                ui->comboChkParamFace->addItem(ActDeal.colInfoList.at(j));
            else if(ActDeal.colInfoList.at(j).contains("Picture"))
                ui->comboChkParamPic->addItem(ActDeal.colInfoList.at(j));
        }
    }
    refreshCheckDeal(act.checkDeal);

    refreshErrorDeal(act.errorDeal);

}

#if 0  //列表不可编辑
/*************************************************************
/函数功能：表格内编辑动作
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::on_tableAction_itemChanged(QTableWidgetItem *item)
{
    int colNum = item->column();
    int selRow = item->row();

    if(selRow>=unitDeal.actTest.length())
        return ;


    if(colNum == Col_Name)
    {
        //修改列表信息
        tAction curAct = unitDeal.actTest.at(selRow);
        curAct.actName = item->text();
        unitDeal.actTest.replace(selRow,curAct);

        ui->editActName->setText(curAct.actName);
    }
    else if(colNum == Col_Str)
    {
        //修改列表信息
        tAction curAct = unitDeal.actTest.at(selRow);
        curAct.actStr = item->text();
        unitDeal.actTest.replace(selRow,curAct);

        if(curAct.actFlag == ACT_KEY)
        {
            ui->stackedWidget->setCurrentWidget(ui->pageKey);
            refreshKeyList(curAct.actStr);
        }
        else if(curAct.actFlag == ACT_SCRIPT)
        {
            ui->stackedWidget->setCurrentWidget(ui->pageScript);
            ui->editFilePath->setText(curAct.actStr);
            if((curAct.actStr.isEmpty())||(curAct.actStr.endsWith(".bat"))||(curAct.actStr.endsWith(".BAT")))
            {
                ui->checkfileMore->setChecked(false);
            }
            else
                ui->checkfileMore->setChecked(true);
        }
    }
}


/*************************************************************
/函数功能：名字编辑完成
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::on_editActName_textChanged(const QString &arg1)
{
    int selRow = tableAction_SelRanges();
    if((selRow>=unitDeal.actTest.length())||(selRow<0))
        return ;

    //修改列表信息
    tAction curAct = unitDeal.actTest.at(selRow);
    curAct.actName = arg1;

    tableAction_ActReplace(selRow,curAct);
}
#endif
/*************************************************************
/函数功能：刷新采集信息列表
/函数参数：采集列表
/函数返回：wu
*************************************************************/
void defTheUnit::refreshColInfo(QStringList infoDeal)
{
    for(int i=0;i<ui->comboBoxActColInfo->count();i++)
    {
        ui->comboBoxActColInfo->clickedItem(i,false);
    }

    for(int i=0;i<infoDeal.length();i++)
    {
        QString infoStr = infoDeal.at(i);
        QStringList infolist = infoStr.split(":");
        if(infolist.length() == 3)
        {
            if(infolist.at(1) == "Interface")
            {
                if(infolist.at(2) == "Front")
                    ui->comboBoxActColInfo->clickedItem(ACTFront_Interface,true);
                else if(infolist.at(2) == "Back")
                    ui->comboBoxActColInfo->clickedItem(ACTBack_Interface,true);
            }
            else if(infolist.at(1) == "Picture")
            {
                if(infolist.at(2) == "Front")
                    ui->comboBoxActColInfo->clickedItem(ACTFront_Picture,true);
                else if(infolist.at(2) == "Back")
                    ui->comboBoxActColInfo->clickedItem(ACTBack_Picture,true);
            }
        }
    }
}

/*************************************************************
/函数功能：状态改变
/函数参数：项目指针，及状态
/函数返回：wu anydeal
*************************************************************/
void defTheUnit::on_comboBoxActColInfo_checkedStateChange(int index, bool checked)
{
    int selRow = tableAction_SelRanges();
    if(selRow>=unitDeal.actTest.length())
        return ;

    //修改列表信息
    tAction curAct = unitDeal.actTest.at(selRow);
    QString infoStr;

    switch(index)
    {
    case ACTFront_Interface:infoStr ="ACT"+toStr(selRow+1)+":Interface:Front";break;
    case ACTBack_Interface:infoStr ="ACT"+toStr(selRow+1)+":Interface:Back";break;
    case ACTFront_Picture:infoStr ="ACT"+toStr(selRow+1)+":Picture:Front";break;
    case ACTBack_Picture:infoStr ="ACT"+toStr(selRow+1)+":Picture:Back";break;
    }

    if(checked)
    {
         curAct.colInfoList.append(infoStr);//加
    }
    else
    {
        curAct.colInfoList.removeOne(infoStr);//减
    }
    unitDeal.actTest.replace(selRow,curAct);
}

/*************************************************************
/函数功能：刷新按键列表
/函数参数：动作字符串
/函数返回：wu
*************************************************************/
void defTheUnit::refreshKeyList(QString actStr)
{
    //刷新按键属性
    int keyNum = getKeyNumber(actStr)-1;
    if((keyNum != -1)&&(actStr.isEmpty()==false))
    {
        ui->labelDescript->setText(keyList.at(keyNum).des);
        if((keyList.at(keyNum).type == Can1_1)||(keyList.at(keyNum).type == Can2_1))
        {
            ui->groupKeyONOFF->setEnabled(false);
            ui->groupKeyONOFF->setChecked(false);
            ui->comboKeyList->setCurrentText(actStr);
        }

        else
        {
            ui->groupKeyONOFF->setEnabled(true);
            if(actStr.contains(":on"))
            {
                ui->groupKeyONOFF->setChecked(true);
                ui->comboKeyList->setCurrentText(actStr.remove(":on"));
            }
            else if(actStr.contains(":off"))
            {
                ui->groupKeyONOFF->setChecked(false);
                ui->comboKeyList->setCurrentText(actStr.remove(":off"));
            }
        }
    }
    else
        ui->comboKeyList->setCurrentIndex(-1);
}

/*************************************************************
/函数功能：选择按键
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::on_comboKeyList_activated(const QString &arg1)
{
    int selRow = tableAction_SelRanges();
    if(selRow>=unitDeal.actTest.length())
        return ;

    //修改列表信息
    tAction curAct = unitDeal.actTest.at(selRow);
    int keyNum=getKeyNumber(arg1)-1;

    curAct.actStr = arg1;
    curAct.actName = keyList.at(keyNum).name;
    ui->labelDescript->setText(keyList.at(keyNum).des);

    if((keyList.at(keyNum).type == Can1_1)||(keyList.at(keyNum).type == Can2_1))
        ui->groupKeyONOFF->setEnabled(false);
    else
    {
        ui->groupKeyONOFF->setEnabled(true);
        if(ui->groupKeyONOFF->isChecked())
        {
            curAct.actStr += ":on";
            curAct.actName +="-ON";
        }
        else
        {
            curAct.actStr +=":off";
            curAct.actName +="-OFF";
        }
    }
    tableAction_ActReplace(selRow,curAct);
}

/*************************************************************
/函数功能：选择按键ONOFF
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::on_groupKeyONOFF_clicked(bool checked)
{
    int selRow = tableAction_SelRanges();
    if(selRow>=unitDeal.actTest.length())
        return ;

    //修改列表信息
    tAction curAct = unitDeal.actTest.at(selRow);
    if(checked)
    {
        if(curAct.actStr.endsWith(":on")==false)
        {
            if((curAct.actStr.endsWith(":off")))
                curAct.actStr.replace(":off",":on");
            else
                curAct.actStr += ":on";

            if((curAct.actName.endsWith("OFF")))
                curAct.actName.replace("OFF","ON");
            //else
            //    curAct.actName += "-ON";
        }
    }
    else
    {
        if(curAct.actStr.endsWith(":off")==false)
        {
            if(curAct.actStr.endsWith(":on"))
                curAct.actStr.replace(":on",":off");
            else
                curAct.actStr += ":off";

            if(curAct.actName.endsWith("ON"))
                curAct.actName.replace("ON","OFF");
            //else
            //    curAct.actName += "-OFF";
        }
    }
    tableAction_ActReplace(selRow,curAct);
}

/*************************************************************
/函数功能： 选择文件或文件夹
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::on_toolButtonSelFile_clicked()
{
    QString dirPath;
    if(ui->checkfileMore->isChecked())
        dirPath=QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this,tr("选择脚本文件"),QDir::currentPath()));//选择脚本文件夹
    else
        dirPath=QFileDialog::getOpenFileName(this , tr("选择脚本文件") , "" , tr("Text Files(*.bat)"));//选择脚本文件
    if(!dirPath.isEmpty())
        ui->editFilePath->setText(dirPath);
}

/*************************************************************
/函数功能：脚本是否批量处理
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::on_checkfileMore_clicked(bool checked)
{
    if(checked)
        ui->labelfileShow->setText(tr("文件夹："));
    else
        ui->labelfileShow->setText(tr("文件："));
}

/*************************************************************
/函数功能：脚本路径编辑
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::on_editFilePath_textChanged(const QString &arg1)
{
    int selRow = tableAction_SelRanges();
    if(selRow>=unitDeal.actTest.length())
        return ;

    //修改列表信息
    tAction curAct = unitDeal.actTest.at(selRow);
    curAct.actStr = arg1;
    tableAction_ActReplace(selRow,curAct);
}

/*************************************************************
/函数功能：电压调节编辑
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::editBatVoltDealSlot(QString volt)
{
    Q_UNUSED(volt)
    int selRow = tableAction_SelRanges();
    if(selRow>=unitDeal.actTest.length())
        return ;

    //修改列表信息
    tAction curAct = unitDeal.actTest.at(selRow);
    changedParam cngParam;

    cngParam.changed = BatVolt;

    cngParam.min = ui->lineEditMixVolt->text().toUInt();
    cngParam.max = ui->lineEditMaxVolt->text().toUInt();
    cngParam.step = ui->lineEditStepVolt->text().toUInt();

    if((cngParam.step!=0)&&(cngParam.min != cngParam.max))
    {
        int i;

        if(cngParam.max > cngParam.min)
            cngParam.dir = true;
        else
            cngParam.dir = false;

        for(i=0;i<curAct.changedDeal.length();i++)
        {
            if(curAct.changedDeal.at(i).changed == BatVolt)
            {
                curAct.changedDeal.replace(i,cngParam);//替换现有参数
                break;
            }
        }
        //添加变动处理
        if(i==curAct.changedDeal.length())
            curAct.changedDeal.append(cngParam);
    }
    else
    {
        //删除变动处理
        for(int i=0;i<curAct.changedDeal.length();i++)
        {
            if(curAct.changedDeal.at(i).changed == BatVolt)
            {
                curAct.changedDeal.removeAt(i);
                break;
            }
        }
    }
    curAct.actStr = "BAT:Volt:"+toStr(cngParam.min);
    tableAction_ActReplace(selRow,curAct);
}

void defTheUnit::editDelayTimeDealSlot(QString time)
{
    Q_UNUSED(time)
    int selRow = tableAction_SelRanges();
    if(selRow>=unitDeal.actTest.length())
        return ;

    //修改列表信息
    tAction curAct = unitDeal.actTest.at(selRow);
    changedParam cngParam;

    cngParam.changed = BatVolt;

    cngParam.min = ui->editDelayMin->text().toUInt();
    cngParam.max = ui->editDelayMax->text().toUInt();
    cngParam.step = ui->editDelayStep->text().toUInt();

    if((cngParam.step!=0)&&(cngParam.min != cngParam.max))
    {
        int i;

        if(cngParam.max > cngParam.min)
            cngParam.dir = true;
        else
            cngParam.dir = false;

        for(i=0;i<curAct.changedDeal.length();i++)
        {
            if(curAct.changedDeal.at(i).changed == BatVolt)
            {
                curAct.changedDeal.replace(i,cngParam);//替换现有参数
                break;
            }
        }
        //添加变动处理
        if(i==curAct.changedDeal.length())
            curAct.changedDeal.append(cngParam);
    }
    else
    {
        //删除变动处理
        for(int i=0;i<curAct.changedDeal.length();i++)
        {
            if(curAct.changedDeal.at(i).changed == BatVolt)
            {
                curAct.changedDeal.removeAt(i);
                break;
            }
        }
    }
    curAct.actStr = toStr(cngParam.min);
    tableAction_ActReplace(selRow,curAct);
}

/*************************************************************
/函数功能：刷新时间参数
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::refreshTimeDeal(tAction act)
{
    int i =0;
    //查询是否存在时间变动的处理
    if(act.changedDeal.isEmpty()==false)
    {
        for(i=0;i<act.changedDeal.length();i++)
        {
            changedParam cngParam = act.changedDeal.at(i);
            if(cngParam.changed==WaitTime)
            {
                ui->editWaitMin->setText(toStr(cngParam.min));
                ui->editWaitMax->setText(toStr(cngParam.max));
                ui->editWaitStep->setText(toStr(cngParam.step));
                break;
            }
        }
    }

    //无-初始化数据为0
    if(i==act.changedDeal.length())
    {
        ui->editWaitMin->setText(toStr(0));
        ui->editWaitMax->setText(toStr(0));
        ui->editWaitStep->setText(toStr(0));
    }

    //刷新时间参数
    ui->editWaitMin->setText(toStr(act.timeDeal.wait));
    ui->editChkTime->setText(toStr(act.timeDeal.check));
    ui->editOverTime->setText(toStr(act.timeDeal.end));
}

/*************************************************************
/函数功能：编辑时间处理槽函数
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::editTimeDealSlot(QString arg1)
{
    int selRow = tableAction_SelRanges();
    if(selRow>=unitDeal.actTest.length())
        return ;
    QObject *sender = QObject::sender();
    //修改列表信息
    tAction curAct = unitDeal.actTest.at(selRow);

    if(sender == ui->editChkTime)
        curAct.timeDeal.check = arg1.toUInt();
    else if(sender == ui->editOverTime)
        curAct.timeDeal.end = arg1.toUInt();
    else
    {
        changedParam cngParam;

        cngParam.changed = WaitTime;

        cngParam.min = ui->editWaitMin->text().toUInt();
        cngParam.max = ui->editWaitMax->text().toUInt();
        cngParam.step = ui->editWaitStep->text().toUInt();

        if((cngParam.step!=0)&&(cngParam.min != cngParam.max))
        {
            int i;

            if(cngParam.max > cngParam.min)
                cngParam.dir = true;
            else
                cngParam.dir = false;

            for(i=0;i<curAct.changedDeal.length();i++)
            {
                if(curAct.changedDeal.at(i).changed == WaitTime)
                {
                    curAct.changedDeal.replace(i,cngParam);//替换现有参数
                    break;
                }
            }
            //添加变动处理
            if(i==curAct.changedDeal.length())
                curAct.changedDeal.append(cngParam);
        }
        else
        {
            //删除变动处理
            for(int i=0;i<curAct.changedDeal.length();i++)
            {
                if(curAct.changedDeal.at(i).changed == WaitTime)
                {
                    curAct.changedDeal.removeAt(i);
                    break;
                }
            }
        }
        curAct.timeDeal.wait = cngParam.min;
    }

    unitDeal.actTest.replace(selRow,curAct);
}

/*************************************************************
/函数功能：刷新检测处理参数
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::refreshCheckDeal(QList<checkParam> chkDeal)
{
    ui->checkSetCurrent->setChecked(false);
    ui->comboCJudge->setCurrentIndex(0);
    ui->editCurMin->setText(toStr(0));
    ui->editCurMax->setText(toStr(0));
    ui->checkSetVolt->setChecked(false);
    ui->comboVJudge->setCurrentIndex(0);
    ui->editVoltMin->setText(toStr(0));
    ui->editVoltMax->setText(toStr(0));
    ui->checkSetSound->setChecked(false);
    ui->comboSoundChanged->setCurrentIndex(0);
    ui->checkSetLogStr->setChecked(false);
    ui->editScriptString->setText("OK (1 test)");
    ui->checkSetInterface->setChecked(false);
    ui->checkSetPic->setChecked(false);
    ui->radioPicSelf->setChecked(true);
    ui->radioFaceMemory->setChecked(true);

    ui->groupBox_cur->setEnabled(true);
    ui->groupBox_vol->setEnabled(true);
    ui->groupBox_sound->setEnabled(true);
    ui->groupBox_log->setEnabled(true);
    ui->groupBox_face->setEnabled(true);
    ui->groupBox_pic->setEnabled(true);
    ui->comboChkParamFace->setCurrentIndex(0);
    ui->comboChkParamPic->setCurrentIndex(0);


    if(chkDeal.isEmpty()==false)
    {
        for(int i=0;i<chkDeal.length();i++)
        {
            checkParam chkParam = chkDeal.at(i);

            switch(chkParam.check)
            {
            case CHKCurrent:{
                ui->checkSetCurrent->setChecked(true);
                ui->comboCJudge->setCurrentIndex(chkParam.range);
                ui->editCurMin->setText(toStr(chkParam.min));
                ui->editCurMax->setText(toStr(chkParam.max));
                ui->groupBox_cur->setEnabled(false);
                break;
            }
            case CHKVlot:{
                ui->checkSetVolt->setChecked(true);
                ui->comboVJudge->setCurrentIndex(chkParam.range);
                ui->editVoltMin->setText(toStr(chkParam.min));
                ui->editVoltMax->setText(toStr(chkParam.max));
                ui->groupBox_vol->setEnabled(false);
                break;
            }
            case CHKSound:{
                ui->checkSetSound->setChecked(true);
                ui->comboSoundChanged->setCurrentIndex(chkParam.sound);
                ui->groupBox_sound->setEnabled(false);
                break;
            }
            case CHKScript:{
                ui->checkSetLogStr->setChecked(true);
                ui->editScriptString->setText(chkParam.logContains);
                ui->groupBox_log->setEnabled(false);
                break;
            }
            case CHKInterface:{
                ui->checkSetInterface->setChecked(true);
                if(chkParam.infoCompare == NoCompare)
                    ui->radioFaceExist->setChecked(true);
                else if(chkParam.infoCompare == MemoryCompare)
                {
                    ui->radioFaceMemory->setChecked(true);
                    ui->comboChkParamFace->setCurrentText(chkParam.comTarget);
                }
                else if(chkParam.infoCompare == SelfCompare)
                    ui->radioFaceSelf->setChecked(true);
                ui->groupBox_face->setEnabled(false);
                break;
            }
            case CHKADBPIC:{
                ui->checkSetPic->setChecked(true);
                if(chkParam.infoCompare == NoCompare)
                    ui->radioPicExist->setChecked(true);
                else if(chkParam.infoCompare == MemoryCompare)
                {
                    ui->radioPicMemory->setChecked(true);
                    ui->comboChkParamPic->setCurrentText(chkParam.comTarget);
                }
                else if(chkParam.infoCompare == SelfCompare)
                    ui->radioPicSelf->setChecked(true);
                else if(chkParam.infoCompare == LocalCompare)
                    ui->radioPicLocal->setChecked(true);
                ui->groupBox_pic->setEnabled(false);
                break;
            }
            case CHKRES:{
                break;
            }
            }
        }
    }
}

/*************************************************************
/函数功能：编辑检测处理槽函数
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::editCheckDealSlot(bool checked)
{
    int selRow = tableAction_SelRanges();
    if(selRow>=unitDeal.actTest.length())
        return ;
    QObject *sender = QObject::sender();
    tAction curAct = unitDeal.actTest.at(selRow);
    //启用
    if(checked)
    {
        checkParam chkDeal ;
        initNullChkParam(&chkDeal);
        if(sender == ui->checkSetCurrent)
        {
            chkDeal.check = CHKCurrent;
            chkDeal.range = (range_type_e)ui->comboCJudge->currentIndex();
            chkDeal.min = ui->editCurMin->text().toUInt();
            chkDeal.max = ui->editCurMax->text().toUInt();
            curAct.checkDeal.append(chkDeal);

            curAct.colInfoList.append("ACT"+toStr(selRow+1)+":Current");

            ui->groupBox_cur->setEnabled(false);
        }
        else if(sender == ui->checkSetVolt)
        {
            chkDeal.check = CHKVlot;
            chkDeal.range = (range_type_e)ui->comboVJudge->currentIndex();
            chkDeal.min = ui->editVoltMin->text().toUInt();
            chkDeal.max = ui->editVoltMax->text().toUInt();
            curAct.checkDeal.append(chkDeal);

            curAct.colInfoList.append("ACT"+toStr(selRow+1)+":Volt");

            ui->groupBox_vol->setEnabled(false);
        }
        else if(sender == ui->checkSetSound)
        {
            chkDeal.check = CHKSound;
            chkDeal.sound = (sound_type_e)ui->comboSoundChanged->currentIndex();
            curAct.checkDeal.append(chkDeal);
            curAct.colInfoList.append("ACT"+toStr(selRow+1)+":Sound");

            ui->groupBox_sound->setEnabled(false);
        }
        else if(sender == ui->checkSetLogStr)
        {
            chkDeal.check = CHKScript;
            chkDeal.logContains = ui->editScriptString->text();
            curAct.checkDeal.append(chkDeal);

            ui->groupBox_log->setEnabled(false);
        }
        else if(sender == ui->checkSetInterface)
        {
            chkDeal.check = CHKInterface;

            if(ui->radioFaceExist->isChecked())
                chkDeal.infoCompare = NoCompare;
            else if(ui->radioFaceMemory->isChecked())
            {
                chkDeal.infoCompare = MemoryCompare;
                chkDeal.comTarget = ui->comboChkParamFace->currentText();
            }
            else if(ui->radioFaceSelf->isChecked())
                chkDeal.infoCompare = SelfCompare;

            curAct.checkDeal.append(chkDeal);

            if(ui->comboBoxActColInfo->isClickedItem(ACTBack_Interface)==false)
            {
                curAct.colInfoList.append("ACT"+toStr(selRow+1)+":Interface:Back");
                ui->comboBoxActColInfo->clickedItem(ACTBack_Interface,true);
            }

            ui->groupBox_face->setEnabled(false);
        }
        else if(sender == ui->checkSetPic)
        {
            chkDeal.check = CHKADBPIC;

            if(ui->radioPicExist->isChecked())
                chkDeal.infoCompare = NoCompare;
            else if(ui->radioPicLocal->isChecked())
                chkDeal.infoCompare = LocalCompare;
            else if(ui->radioPicMemory->isChecked())
            {
                chkDeal.infoCompare = MemoryCompare;
                chkDeal.comTarget = ui->comboChkParamPic->currentText();
            }
            else if(ui->radioPicSelf->isChecked())
                chkDeal.infoCompare = SelfCompare;

            curAct.checkDeal.append(chkDeal);

            if(ui->comboBoxActColInfo->isClickedItem(ACTBack_Picture)==false)
            {
                curAct.colInfoList.append("ACT"+toStr(selRow+1)+":Picture:Back");
                ui->comboBoxActColInfo->clickedItem(ACTBack_Picture,true);
            }

            ui->groupBox_pic->setEnabled(false);
        }
    }
    //删除
    else
    {
        if(sender == ui->checkSetCurrent)
        {
            ui->groupBox_cur->setEnabled(true);
            curAct.colInfoList.removeOne("ACT"+toStr(selRow+1)+":Current");
        }
        else if(sender == ui->checkSetVolt)
        {
            ui->groupBox_vol->setEnabled(true);
            curAct.colInfoList.removeOne("ACT"+toStr(selRow+1)+":Volt");
        }
        else if(sender == ui->checkSetSound)
        {
            ui->groupBox_sound->setEnabled(true);
            curAct.colInfoList.removeOne("ACT"+toStr(selRow+1)+":Sound");
        }
        else if(sender == ui->checkSetLogStr)
            ui->groupBox_log->setEnabled(true);
        else if(sender == ui->checkSetInterface)
        {
            ui->groupBox_face->setEnabled(true);
            curAct.colInfoList.removeOne("ACT"+toStr(selRow+1)+":Interface:Back");
            ui->comboBoxActColInfo->clickedItem(ACTBack_Interface,false);
        }
        else if(sender == ui->checkSetPic)
        {
            ui->groupBox_pic->setEnabled(true);
            curAct.colInfoList.removeOne("ACT"+toStr(selRow+1)+":Picture:Back");
            ui->comboBoxActColInfo->clickedItem(ACTBack_Picture,false);
        }

        for(int i=0;i<curAct.checkDeal.length();i++)
        {
            if(((sender == ui->checkSetCurrent)&&(curAct.checkDeal.at(i).check == CHKCurrent))
                    ||((sender == ui->checkSetVolt)&&(curAct.checkDeal.at(i).check == CHKVlot))
                    ||((sender == ui->checkSetSound)&&(curAct.checkDeal.at(i).check == CHKSound))
                    ||((sender == ui->checkSetLogStr)&&(curAct.checkDeal.at(i).check == CHKScript))
                    ||((sender == ui->checkSetInterface)&&(curAct.checkDeal.at(i).check == CHKInterface))
                    ||((sender == ui->checkSetPic)&&(curAct.checkDeal.at(i).check == CHKADBPIC)))
            {
                curAct.checkDeal.removeAt(i);
                break;
            }
        }
    }
    unitDeal.actTest.replace(selRow,curAct);
}

/*************************************************************
/函数功能：刷新错误处理标志
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::refreshErrorDeal(uint8_t errorFlag)
{
    if(errorFlag == NODealERROR)
        ui->radioBtnNODeal->setChecked(true);
    else if(errorFlag == OVERTIMEERR)
        ui->radioBtnOverTimeDeal->setChecked(true);
    else if(errorFlag == CHKERROR)
        ui->radioButtonchkErrorDeal->setChecked(true);
    else if(errorFlag == CHKPASS)
        ui->radioButtonchkPassDeal->setChecked(true);
}

/*************************************************************
/函数功能：编辑错误处理槽函数
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::editErrorDealSlot()
{
    int selRow = tableAction_SelRanges();
    if(selRow>=unitDeal.actTest.length())
        return ;
    QObject *sender = QObject::sender();
    //修改列表信息
    tAction curAct = unitDeal.actTest.at(selRow);

    if(sender == ui->radioBtnNODeal)
    {
        curAct.errorDeal = NODealERROR;
    }
    else if(sender == ui->radioBtnOverTimeDeal)
    {
        curAct.errorDeal = OVERTIMEERR;
    }
    else if(sender == ui->radioButtonchkErrorDeal)
    {
        curAct.errorDeal = CHKERROR;
    }
    else if(sender == ui->radioButtonchkPassDeal)
    {
        curAct.errorDeal = CHKPASS;
    }
    unitDeal.actTest.replace(selRow,curAct);
}

/*************************************************************
/函数功能：预览
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::on_actLook_triggered()
{
    if((unitDeal.actTest.isEmpty())||(unitDeal.name.isEmpty()))
    {
        QMessageBox::warning(NULL, tr("提示"), tr("请输入测试单元名/检测测试动作是否为空！"));
        return ;
    }
    else
    {
        CfgLookUnit lookUnit(&unitDeal);

        lookUnit.exec();//不可修改,该窗口即为配置窗口，无需再修改
    }
}

/*************************************************************
/函数功能：保存测试单元
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::on_actSave_triggered()
{
    if((unitDeal.actTest.isEmpty())||(unitDeal.name.isEmpty()))
    {
        QMessageBox::warning(NULL, tr("提示"), tr("请输入测试单元名/检测测试动作是否为空！"));
        return ;
    }

    QFile file(configPath("unitDefined.xml")); //关联文件名字
    Model_XMLFile xmlSave;
    QStringList infoList;

    if( false == file.exists() ) //如果存在不创建
    {
        xmlSave.createSequenceXML(configPath("unitDefined.xml"));
    }
    //查询是否已经存在该测试单元
    if(xmlSave.hasUnitInfomation(configPath("unitDefined.xml"),unitDeal.name))
    {
        if(QMessageBox::information(NULL, tr("提示"), tr("文件中该测试单元已存在，是否替换？？"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)==QMessageBox::No)
            return;
        else
        {
            xmlSave.removeUnitXML(configPath("unitDefined.xml"),unitDeal.name);
            for(int i=0;i<unitList.length();i++)
            {
                if(unitList.at(i).name==unitDeal.name)
                    unitList.replace(i,unitDeal);
            }
        }
    }
    else
    {
        ui->listUnit->addItem(unitDeal.name);
        unitList.append(unitDeal);
    }

    infoList.clear();
    infoList<<unitDeal.name<<QString::number(unitDeal.cycleCount)<<unitDeal.unitDes;//"this is test"

    for(int j=0;j<unitDeal.actTest.length();j++)
    {
        xmlSave.appendSequenceXML(configPath("unitDefined.xml"),infoList,unitDeal.actTest.at(j));
    }

}

/*************************************************************
/函数功能：应用测试单元
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::on_actApply_triggered()
{
    if((unitDeal.actTest.isEmpty())||(unitDeal.name.isEmpty()))
    {
        QMessageBox::warning(NULL, tr("提示"), tr("请输入测试单元名/检测测试动作是否为空！"));
        return ;
    }
    else
        applyTheUnit(unitDeal);
}

/*************************************************************
/函数功能：新建测试单元
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::on_actnew_triggered()
{
    if(tableAction_RowCount())
    {
        int selKey = QMessageBox::warning(NULL, tr("提示"),tr("是否已经保存当前测试单元?"),QMessageBox::Save | QMessageBox::Yes | QMessageBox::Cancel,QMessageBox::Yes);
        if (selKey == QMessageBox::Cancel )
            return ;
        else if (selKey == QMessageBox::Save )
            on_actSave_triggered();
    }

    //初始化测试单元基本信息
    unitDeal.name = "";
    unitDeal.cycleCount = 2;
    unitDeal.unitDes = "";
    unitDeal.actTest.clear();
    refreshUnitShow(unitDeal);
}

/*************************************************************
/函数功能：打开帮助文档
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::on_actHelp_triggered()
{
    QString pdfPath=QDir::currentPath()+"/Unit编辑窗口使用说明.pdf";
    if(QDesktopServices::openUrl(QUrl::fromLocalFile(pdfPath))==false)
    {
        QMessageBox::warning(NULL, tr("提示"), tr("该运行目录下无《Unit编辑窗口使用说明.pdf》文档！"));
    }
}

/*************************************************************
/函数功能：选择当前测试单元
/函数参数：指针
/函数返回：wu
*************************************************************/
void defTheUnit::on_listUnit_clicked(const QModelIndex &index)
{
    int listIndex = index.row();

    if(listIndex<unitList.length())
        refreshUnitShow(unitList.at(listIndex));
}

/*************************************************************
/函数功能：列表右键
/函数参数：指针
/函数返回：wu
*************************************************************/
void defTheUnit::on_listUnit_customContextMenuRequested(const QPoint &pos)
{
    Q_UNUSED(pos);
    QMenu *popMenu = new QMenu( this );
    QAction *deleteAct = new QAction(tr("删除"), this);
    QAction *clearAct = new QAction(tr("清空"), this);

    if(userLogin.Permissions == Administrator)
    {
        popMenu->addAction(deleteAct);
        popMenu->addAction(clearAct);
    }

    connect( deleteAct,        SIGNAL(triggered() ), this, SLOT( unitDeleteSlot()) );
    connect( clearAct,        SIGNAL(triggered() ), this, SLOT( unitClearSlot()) );
    popMenu->exec( QCursor::pos() );

    delete popMenu;
}

/*************************************************************
/函数功能：删除测试单元
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::unitDeleteSlot()
{
    QListWidgetItem *item = ui->listUnit->currentItem();
    if( item == NULL )
        return;

    int index = ui->listUnit->currentRow();
    QString delStr = item->text();
    Model_XMLFile xmlFile;
    if(xmlFile.hasUnitInfomation(configPath("unitDefined.xml"),delStr))
    {
        if (QMessageBox::warning(NULL, tr("提示"),tr("确定删除该测试测试单元?"),QMessageBox::Yes | QMessageBox::No,QMessageBox::No) != QMessageBox::Yes )
            return;

        xmlFile.removeUnitXML(configPath("unitDefined.xml"),delStr);

        QListWidgetItem* delitem = ui->listUnit->takeItem(index);
        delete delitem;

        if(index<unitList.length())
            unitList.removeAt(index);
    }
}

/*************************************************************
/函数功能：清空测试单元
/函数参数：无
/函数返回：wu
*************************************************************/
void defTheUnit::unitClearSlot()
{
    Model_XMLFile xmlFile;
    if (QMessageBox::warning(NULL, tr("提示"),tr("确定清空该测试单元?"),QMessageBox::Yes | QMessageBox::No,QMessageBox::No) != QMessageBox::Yes )
        return;

    for(int i=0;i<unitList.length();i++)
    {
        xmlFile.removeUnitXML(configPath("unitDefined.xml"),unitList.at(i).name);
    }


    ui->listUnit->clear();
    unitList.clear();
}


