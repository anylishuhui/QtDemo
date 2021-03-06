#include "Models/model_include.h"


user_type_s userLogin;
//ATtoolConfig配置参数
QString WorkItem;           //当前测试项目
int WorkCurrent;            //工作电流
int WorkFrequency;          //测试次数
bool ReportCreat;           //创建报告


//运行中
QString devNumber;              //设备序列号
int devNumberCount = 0;
int Current=0;                  //当前测试电流
int Volt = 0;
bool SoundV;
QStringList ShowList;           //字符串列表：用来显示
QString savePath;                //测试序列保存为xml文件；测试结果记录保存为TXT文件，路径为序列文件所在路径，文件名为序列文件名

//特殊字符串标记，以便直接处理
QString AccKey;
QString BatKey;

QString testString;            //以此字符串传递action的执行字符串在主函数中运行

bool actIsRunning=false;

bool PauseState=false;


/*************************************************************
/函数功能：获得空按键资源信息：主要对数据信息进行赋值，字符串默认为空
/函数参数：无
/函数返回：按键信息：空
*************************************************************/
void getkeyControlNull(keyControl *keyInfo)
{
    keyInfo->isUse=false;
    keyInfo->type=HardACC;

    keyInfo->des="No Information!";
}

QString getKeyType(kType type)
{
    QString str;
    switch(type)
    {
    case HardACC:str="硬件ACC";break;
    case HardBAT:str="硬件BAT";break;
    case HardCCD:str="硬件CCD";break;
    case HardLamp:str="硬件LAMP";break;
    case HardBrake:str="硬件Brake";break;
    case HardCCDPower:str="CCD摄像头供电使能";break;
    case Can1_1:str="CAN1-单协议";break;
    case Can1_2:str="CAN1-双协议";break;
    case Can2_1:str="CAN2-单协议";break;
    case Can2_2:str="CAN2-双协议";break;
    case KEYOTHER:str="其他按键类型";break;
    }
    return str;
}


/*************************************************************
/函数功能：保存文件
/函数参数：SaveStr :保存字符串
/函数返回：无Text
/Append：在现有文件上添加   Text：文档从头编辑
*************************************************************/
bool appendTheResultToFile(QString SaveStr)
{
    QDir dir(savePath);
    if(!dir.exists())
    {
        if(dir.mkpath(savePath) == false) //创建多级目录
            return false;
    }

    QFile file(savePath+"/report.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        cout << "Cannot open file " << savePath+"/report.txt";
        return false;
    }
    QTextStream out(&file);
    out << SaveStr +"\r\n";

    file.close();
    return true;
}

/*************************************************************
/函数功能：保存文件:机器信息
/函数参数：SaveStr :保存字符串
/函数返回：无Text
/Append：在现有文件上添加   Text：文档从头编辑
*************************************************************/
bool appendThePropertiesToFile(QString path,QString SaveStr)
{
    QDir dir(path);
    if(!dir.exists())
    {
        if(dir.mkpath(path) == false) //创建多级目录
        {
            cout << "创建属性文件夹失败！创建路径为："<<path;
            return false;
        }
    }

    QFile file(path+"/properties.txt");
    bool isOK;

    //若是清数据，以Text保存，会将文档内数据先清除，再填写，否则，追加方式填写；
    if(SaveStr == "clear")
        isOK=file.open(QIODevice::WriteOnly | QIODevice::Text);
    else
        isOK=file.open(QIODevice::WriteOnly | QIODevice::Append);

    if (!isOK)
    {
        cout << "Cannot open file " << path+"/properties.txt";
        return false;
    }

    QTextStream out(&file);
    if(SaveStr == "clear")
        out << "";
    else
        out << SaveStr;

    file.close();
    return true;
}

void appendTheExecLogInfo(QString SaveStr)
{
    #if 1
    QStringList pathList = savePath.split("/");

    QString path ;

    //路径拆分方式增加处理："\\"
    if(pathList.length() == 1)
    {
        pathList.clear();
        pathList = savePath.split("\\");
    }

    //组合路径
    for(int i=0;i<pathList.length()-2;i++)
    {
        path += pathList.at(i) + "/";
    }

    QDir dir(path);
    if(!dir.exists())
    {
        if(dir.mkpath(path) == false) //创建多级目录
        {
            cout << "创建属性文件夹失败！创建路径为："<<path;
            return ;
        }
    }

    QFile file(path+"ExecLog.txt");

    if (!file.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        cout << "Cannot open file " << path+"properties.txt";
        return ;
    }

    QTextStream out(&file);
    out << SaveStr;
    file.close();
    #endif
}

/*************************************************************
/函数功能：在树上添加节点
/函数参数：  父节点  属性  list填充数据
/函数返回：添加的节点
//flags:0x01:可编辑  0x02:字体颜色为红色
*************************************************************/
QTreeWidgetItem *AddTreeNode(QTreeWidgetItem *parent, uint8_t flags,QStringList list)
{
    QTreeWidgetItem * item=new QTreeWidgetItem(list);

    if(flags & 0x01)
    {
        item->setFlags(item->flags()|Qt::ItemIsEditable);
    }

    if(flags & 0x02)
    {
        for(int i=0;i<list.length();i++)
            item->setTextColor(i,RED);
    }

    parent->addChild(item);
    return item;
}

/*************************************************************
/函数功能：开始执行动作
/函数参数：  执行字符串
/函数返回：无
/在运行时执行到动作执行时赋值字符串，主函数循环扫描到字符串时开始运行
*************************************************************/
void startAction(QString actStr)
{
    clearAction();
    testString = actStr;
}

/*************************************************************
/函数功能：清执行动作
/函数参数：  无
/函数返回：无
*************************************************************/
void clearAction()
{
    testString.clear();
}


/*************************************************************
/函数功能：获取运行时设备序列号
/函数参数：无
/函数返回：无
*************************************************************/
QString getDevNumber()
{
    return devNumber;
}

void setDevNumberCount(int count)
{
    devNumberCount = count;
}

/*************************************************************
/函数功能：设备列表中是否只有一台设备
/函数参数：无
/函数返回：判断结果
//any:用于若未选择设备列表时，值为1时处理，只有一台设备
*************************************************************/
bool NumberListIsSingle()
{
    if(devNumberCount == 1)
        return true;
    else
    {
        //if(devNumberCount)
        ShowList<<("<font color = red>警告：非单台设备或无设备，将不进行ADB命令执行！</font>");
            //QMessageBox::warning(NULL, tr("提示"), tr("非单台设备，将不进行ADB命令执行！"));
        return false;
    }
}



int getKeyNumber(QString key)
{
    QStringList theAct=key.split(':');
    int keyNum=-1;

    if(theAct.isEmpty()==false)
    {
        QString keyNumber = theAct.at(0);
        keyNum = keyNumber.remove("KEY").toInt();//KEY1 取"1"
    }


    return keyNum;
}

