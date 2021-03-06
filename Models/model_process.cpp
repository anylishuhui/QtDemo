#include "Model_Process.h"
/**************************************************************************************************************************
 * ***************功能描述：进程处理******************************************************************************************
 * *********************（1）根据USB端口建立进程，进程的执行即是端口执行**********************************************************
 * **********************（2）进程0为全局进程，可用来做全局命令执行，不参与USB升级操作**********************************************
 * ***********************（3）命令执行建立了命令链表，可按链表执行连续操作：命令时间较长时不建议使用**********************************
**************************************************************************************************************************/



/*************************************************************
/函数功能：构造函数：开启定时器，建立全局进程0
/函数参数：无
/函数返回：无
*************************************************************/
Model_Process::Model_Process()
{
    /*设置定时器*/
    m_timerID1=startTimer(10);//刷新进程执行

    /*建立全局进程-进程0*/
    Process_Add();
}

/*************************************************************
/函数功能：析构函数：杀死定时器
/函数参数：无
/函数返回：无
*************************************************************/
Model_Process::~Model_Process()
{
    killTimer(m_timerID1);
}

/*************************************************************
/函数功能：定时器事件：1.扫描执行进程；2.进程定时处理
/函数参数：event:定时器事件
/函数返回：无
*************************************************************/
void Model_Process::timerEvent(QTimerEvent *event)
{
    if(!P_List.length())
        return ;

    if(event->timerId()==m_timerID1)
    {
        /*扫描执行进程*/
        Process_Execute();
    }
}

/*************************************************************
/函数功能：进程添加：根据设备端口添加进程：1~...
/函数参数：无
/函数返回：无
*************************************************************/
void Model_Process::Process_Add()
{
    tProcess_t aProcess;
    aProcess.process=new QProcess();
    aProcess.cmdList.clear();
    aProcess.pNum=P_List.length();
    P_List.append(aProcess);
    //cout<< P_List.length();

    connect(P_List.at(P_List.length()-1).process,SIGNAL(readyRead()),this,SLOT(ProcessReadAll()));
    connect(P_List.at(P_List.length()-1).process,SIGNAL(readyReadStandardError()),this,SLOT(ProcessReadError()));
    connect(P_List.at(P_List.length()-1).process, SIGNAL(stateChanged(QProcess::ProcessState)),this, SLOT(ProcessshowState(QProcess::ProcessState)));

    pParam[aProcess.pNum].isRun=noRun;
    pParam[aProcess.pNum].rTime=0;
    pParam[aProcess.pNum].pBarValue=0;
    pParam[aProcess.pNum].CmdRead="";
    pParam[aProcess.pNum].CmdError="";
}

/*************************************************************
/函数功能：进程添加：根据设备端口添加进程：1~...
/函数参数：Workpath:进程的工作路径
/函数返回：无
*************************************************************/
void Model_Process::Process_Add(QString Workpath)
{
    Process_Add();

    P_List.at(P_List.length()-1).process->setWorkingDirectory(Workpath);
}

/*************************************************************
/函数功能：进程运行目录跳转
/函数参数：无
/函数返回：true:正在运行 false:未运行
*************************************************************/
void Model_Process::ProcessPathJump(QString path)
{
    for(uint8_t i=1;i<P_List.length();i++)
        P_List.at(i).process->setWorkingDirectory(path);
}

/*************************************************************
/函数功能：进程删除
/函数参数：pNum：进程序号
/函数返回：无
/系统进程不可删除
*************************************************************/
void Model_Process::Process_Del(uint8_t pNum)
{
    if((pNum>0)&&(pNum<P_List.length()))
    {
        tProcess_t tempProcess;
        P_List.removeAt(pNum);

        /*删除当前进程后，后面进程的序号修改*/
        for(uint8_t i=pNum;i<P_List.length();i++)
        {
            tempProcess=P_List.at(i);
            tempProcess.pNum=i;
            P_List.replace(i,tempProcess);
        }
    }
    else
        cout << "pNum is Over or SYSProcess.";
}

/*************************************************************
/函数功能：进程执行:遍历进程列表，执行置开始标志，且有进程命令的进程
/函数参数：无
/函数返回：无
//Process.process->waitForFinished();//阻塞，直到上一个进程执行完，再进行下一个进程执行
*************************************************************/
void Model_Process::Process_Execute()
{
    for(uint8_t i=0;i<P_List.length();i++)
    {
        /*进程开启了运行标志且进程命令不为空*/
        if((pParam[i].isRun == RunStart) && (P_List.at(i).cmdList.isEmpty()==false))
        {
            QString cmdStr = P_List.at(i).cmdList.first();

            ProcessOutDeal(i,"EXEC>>"+cmdStr);
            P_List.at(i).process->start("cmd.exe", QStringList() << "/c" << cmdStr);

            /*开启进程失败*/
            if (P_List.at(i).process->waitForStarted()==false)
            {
                cout << i <<"WARN:Failed to start";
                ProcessisOver(i);//进程结束
                pParam[i].isRun=noRun;
            }
        }
    }
}

/*************************************************************
/函数功能：进程开始执行：置进程运行标志位，添加进程执行命令
/函数参数：pNum:进程序号  cmdStr:命令字符串
/函数返回：无
*************************************************************/
void Model_Process::ProcessStart(uint8_t pNum,QString cmdStr)
{
    if(pNum<P_List.length())
    {
        /*若进程未进行，启动运行，否则只添加指令列表*/
        if(pParam[pNum].isRun == noRun)
            pParam[pNum].isRun=RunStart;

        /*进程只执行单个命令，带判断条件；
         * 若执行多个命令，将判断添加去掉，累加即可*/
        if(P_List.at(pNum).cmdList.isEmpty())
        {
            tProcess_t tempProcess=P_List.at(pNum);
            tempProcess.cmdList.append(cmdStr);
            P_List.replace(pNum,tempProcess);
        }
    }
    else
        cout << "pNum is Over.";
}

/*************************************************************
/函数功能：停止进程执行
/函数参数：devNum：进程号  appStr：命令
/函数返回：无
//备注：执行杀死进程的命令
adb -s 8495650c21bb13c shell "ps | grep logcat"
USER     PID   PPID  VSIZE  RSS     WCHAN    PC         NAME
root      2797  1754  2452   1780  c00f7e34 400b6124 S logcat
adb -s 8495650c21bb13c shell kill -9 PID
*************************************************************/
void Model_Process::stopProcess(QString devNum,QString appStr)
{
    //摘自ACCTool工具
    QProcess process;
    QString command;//"adb -s " + devNum +  " shell ps | grep " +appStr;
    if(devNum.isEmpty())
        command = PSGREP(appStr);
    else
        command = PSGREP_S(devNum,appStr);
    cout << (command);
    process.start(command);
    process.waitForStarted();
    process.waitForFinished();
    QByteArray byteArray = process.readAllStandardOutput();
    QString str = QString::fromLocal8Bit(byteArray);
    cout << (str);
    QStringList list = str.simplified().split(" ");
    for (int i=0;i<list.length();i++)
    {
        if(i%9 == 1){
            QString killcmd ;//"adb -s " + devNum + " shell kill -9 " + list.at(i);
            if(devNum.isEmpty())
                killcmd = KILL_9(list.at(i));
            else
                killcmd = KILL_9_S(devNum,list.at(i));
            cout << (killcmd);
            process.start(killcmd);
            process.waitForStarted();
            process.waitForFinished();
            QByteArray byteArray = process.readAllStandardOutput();
            QString str = QString::fromLocal8Bit(byteArray);
            cout << (str);
        }
    }
    process.close();
}

/*************************************************************
/函数功能：进程读信息槽函数
/函数参数：无
/函数返回：无
*************************************************************/
void Model_Process::ProcessReadAll()
{
    if(!P_List.length())
        return ;

    QObject *sender = QObject::sender();

    for(uint8_t i=0;i<P_List.length();i++)
    {
        if(sender==P_List.at(i).process)
        {
            ProcessOutDeal(i,"Out>>"+P_List.at(i).process->readAll());//QString::fromLocal8Bit();//any：最新版本的QT无需转换，可直接输出中文
            break;
        }
    }
}
/*************************************************************
/函数功能：进程读错误信息槽函数
/函数参数：无
/函数返回：无
*************************************************************/
void Model_Process::ProcessReadError()
{
    if(!P_List.length())
        return ;

    QObject *sender = QObject::sender();

    for(uint8_t i=0;i<P_List.length();i++)
    {
        if(sender==P_List.at(i).process)
        {
            ProcessOutDeal(i,"Error>>"+QString::fromLocal8Bit(P_List.at(i).process->readAllStandardError()));
            break;
        }
    }
}

/*************************************************************
/函数功能：进程判断状态槽函数
/函数参数：state:进程状态:Running-正在运行  Starting-开始运行  NotRunning-运行结束
/函数返回：无
*************************************************************/
void Model_Process::ProcessshowState(QProcess::ProcessState state)
{
    if(!P_List.length())
        return ;

    QObject *sender = QObject::sender();

    /*遍历进程*/
    for(uint8_t i=0;i<P_List.length();i++)
    {
        /*确认执行的进程*/
        if(sender == P_List.at(i).process)
        {
            switch(state)
            {
            case QProcess::Running:/*进程正在执行*/
                {
                    pParam[i].isRun=Runing;
                }
                break;
            case QProcess::Starting:/*进程开始执行*/
                {
                    pParam[i].rTime=1;
                    pParam[i].pBarValue=1;
                    ProcessBar(i-1,pParam[i].pBarValue,GREEN);
                }
                break;
            case QProcess::NotRunning:/*进程结束执行*/
                {
                    /*计时清空*/
                    pParam[i].rTime=0;

                    /*删除执行过的命令*/
                    tProcess_t tempProcess=P_List.at(i);
                    tempProcess.cmdList.removeFirst();
                    P_List.replace(i,tempProcess);

                    /*进程执行完毕，仍有命令，继续开启进程执行下一个命令*/
                    if(P_List.at(i).cmdList.isEmpty()==false)
                        pParam[i].isRun=RunStart;
                    else
                    {
                        pParam[i].isRun=noRun;//先转换进程执行状态，再去处理进程结束
                        ProcessisOver(i);//进程结束
                    }
                }
                break;
            }
            break;
        }
    }
}

/*************************************************************
/函数功能：获得某一进程运行状态
/函数参数：pNum:进程序号
/函数返回：sRun_e:进程状态
*************************************************************/
sRun_e Model_Process::GetProcessRunStatus(uint8_t pNum)
{
    return pParam[pNum].isRun;
}

/*************************************************************
/函数功能：是否有进程正在运行
/函数参数：无
/函数返回：true:正在运行 false:未运行
*************************************************************/
bool Model_Process::GetProcessIsRun()
{
    for(uint16_t i=P_List.length();i>0;i--)
    {
        if(pParam[i].isRun != noRun)
        {
            return true;
        }
    }
    return false;
}
