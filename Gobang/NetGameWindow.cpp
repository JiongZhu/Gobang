#include "NetGameWindow.h"
#include "ui_NetGameWindow.h"
#include <QPainter>
#include <QMessageBox>
#include <QUdpSocket>
#include <QHostInfo>
#include <QScrollBar>
#include <QDateTime>
#include <QNetworkInterface>
#include <QProcess>
#include <QSqlQuery>
#include "GameWindow.h"
#include "MainWindow.h"
#include "PeopleVsComputer.h"
#include <QFileDialog>
NetGameWindow::NetGameWindow(QWidget *parent) :
    QMainWindow(parent),game(),ui(new Ui::NetGameWindow)
{
    ui->setupUi(this);
    mouseflag=0;
    firstReady=0;
    myColor=false;
    myReady=0;
    otherReady=0;
    udpSocket=new QUdpSocket(this);  //创建一个QUdpSocket类对象，该类提供了Udp的许多相关操作
    port = 45454;
    //此处的bind是个重载函数，连接本机的port端口，采用ShareAddress模式(即允许其它的服务连接到相同的地址和端口，特别是
    //用在多客户端监听同一个服务器端口等时特别有效)，和ReuseAddressHint模式(重新连接服务器)
    udpSocket->bind(port,QUdpSocket::ShareAddress|QUdpSocket::ReuseAddressHint);
    //readyRead()信号是每当有新的数据来临时就被触发
    connect(udpSocket,SIGNAL(readyRead()),this,SLOT(processPendingDatagrams()));
    sendMessage(NewParticipant);//打开软件时就向外发射本地信息，让其他在线用户得到通知
    this->setFixedSize(850,720);
    this->setAutoFillBackground(true);
    QPalette palette;
    palette.setColor(QPalette::Background,QColor("#B1723C"));
    this->setPalette(palette);
    centralWidget()->setMouseTracking(true);
    setMouseTracking(true);
}

//使用UDP广播发送信息，MessageType是指头文件中的枚举数据类型
//sendMessage即把本机的主机名，用户名+（消息内容后ip地址）广播出去
void NetGameWindow::sendMessage(MessageType type)
{
    QByteArray data;    //字节数组
    //QDataStream类是将序列化的二进制数据送到io设备，因为其属性为只写
    QDataStream out(&data,QIODevice::WriteOnly);
    QString localHostName=QHostInfo::localHostName();//返回主机名，QHostInfo包含了一些关于主机的静态函数
    QString address=getIP();   //调用自己类中的getIP()函数
    //将type，getUserName()，localHostName按照先后顺序送到out数据流中，消息类型type在最前面
    out<<type<<getUserName()<<localHostName;
    switch(type)
    {
        case Message:
            if(ui->messageTextEdit->toPlainText()=="")
            {
                //将输入框里的文字转化成纯文本发送，当发送的文本为空时创建一个警告信息窗口，tr函数为译本函数，即译码后面的text内容
                QMessageBox::warning(0,"warning",QStringLiteral("文本不能为空"),QMessageBox::Ok);
                return;
            }
            out<<address<<getMessage();    //将ip地址和得到的消息内容输入out数据流
            ui->messageBrowser->verticalScrollBar() //返回垂直条
                    ->setValue(ui->messageBrowser->verticalScrollBar()->maximum());  //设置垂直滑动条的最大值
            break;
        case NewParticipant:
            out<<address;            //为什么此时只是输出地址这一项呢？因为此时不需要传递聊天内容
            break;
        case ParticipantLeft:
            break;
        case Ready:
            break;
        case Gamesite:
            out<<getGame();
            break;
        case Back:
            break;
        case returnInt:
            out<<returnint;
            if(returnint==QMessageBox::Yes)
            {
                game.backChess();
                mouseflag-=2;
                update();
            }
            break;
        case Giveup:
            break;
    }
    /*
    一个udpSocket已经于一个端口bind在一起了，这里的data是out流中的data，最多可以传送8192个字节，但是建议不要超过
    512个字节，因为这样虽然可以传送成功，但是这些数据需要在ip层分组，QHostAddress::Broadcast是指发送数据的目的地址
    这里为本机所在地址的广播组内所有机器，即局域网广播发送
    */
    udpSocket->writeDatagram(data,data.length(),QHostAddress::Broadcast,port);  //将data中的数据发送
}

// 处理用户离开
void NetGameWindow::participantLeft(QString userName, QString localHostName, QString time)
{
        //找到第一个对应的主机名
        int rowNum = ui->userTableWidget->findItems(localHostName, Qt::MatchExactly).first()->row();
        ui->userTableWidget->removeRow(rowNum);//此句执行完后，rowCount()内容会自动减1
        ui->messageBrowser->setTextColor(Qt::black);//设置文本颜色为黑色
        ui->messageBrowser->setCurrentFont(QFont("Times New Roman", 10));
        ui->messageBrowser->append(QStringLiteral("%1离开%2").arg(userName).arg(time));
}

// 接收UDP信息
void NetGameWindow::processPendingDatagrams()
{
    //hasPendingDatagrams返回true时表示至少有一个数据报在等待被读取
    while(udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        //pendingDatagramSize为返回第一个在等待读取报文的size，resize函数是把datagram的size归一化到参数size的大小一样
        datagram.resize(udpSocket->pendingDatagramSize());
        //将读取到的不大于datagram.size()大小数据输入到datagram.data()中，datagram.data()返回的是一个字节数组中存储数据位置的指针
        udpSocket->readDatagram(datagram.data(),datagram.size());
        QDataStream in(&datagram,QIODevice::ReadOnly);//因为其属性为只读，所以是输入
        int messageType;   //此处的int为qint32，在Qt中，qint8为char，qint16为uint
        in>>messageType;   //读取1个32位长度的整型数据到messageTyep中
        QString userName,localHostName,ipAddress,message;
        QPoint gamesite;
        QString time=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");  //将当前的时间转化到括号中的形式
        switch(messageType)
        {
            case Message:
                //in>>后面如果为Qstring，则表示读取一个直到出现'\0'的字符串
                in>>userName>>localHostName>>ipAddress>>message;
                ui->messageBrowser->setTextColor(Qt::blue);//设置文本颜色
                ui->messageBrowser->setCurrentFont(QFont("Times New Roman",12));//设置字体大小
                //ui->messageBrowser->append("[ " +userName+" ] "+ time);//输出的格式为用户名加时间显示
                //输出的格式为主机名加时间显示，但输出完后为什么会自动换行呢？
                ui->messageBrowser->append("["+localHostName+"]"+time);
                ui->messageBrowser->append(message);//消息输出
                break;
            case NewParticipant:
                in>>userName>>localHostName>>ipAddress;
                newParticipant(userName,localHostName,ipAddress);
                break;
            case ParticipantLeft:
                in>>userName>>localHostName;
                participantLeft(userName,localHostName,time);
                break;
            case Giveup:
                in>>userName>>localHostName;
                if(myColor)
                  QMessageBox::about(this,QStringLiteral(""),QStringLiteral("黑棋认输,白棋获胜"));
                else
                  QMessageBox::about(this,QStringLiteral(""),QStringLiteral("白棋认输,黑棋获胜"));
                mouseflag=0;
                break;
            case Back:
                in>>userName>>localHostName;
                if(localHostName!=QHostInfo::localHostName())
                {
                    returnint=QMessageBox::question(this,QStringLiteral("悔棋"),QStringLiteral("是否同意悔棋"),QMessageBox::Yes,QMessageBox::No);
                    sendMessage(returnInt);
                }
                break;
            case returnInt:
                in>>userName>>localHostName>>returnint;
                if(localHostName!=QHostInfo::localHostName())
                    if(returnint==QMessageBox::Yes)
                    {
                        game.backChess();
                        mouseflag-=2;
                        update();
                    }
                break;
            case Ready:
                in>>userName>>localHostName;
                firstReady++;
                if(localHostName==QHostInfo::localHostName())
                {
                    myReady=firstReady;
                }
                else
                {
                    otherReady=firstReady;
                }
                ui->messageBrowser->setTextColor(Qt::black);
                ui->messageBrowser->setCurrentFont(QFont("Times New Roman",10));
                ui->messageBrowser->append(QStringLiteral("%1准备").arg(userName));
                if(myReady!=0&&otherReady!=0)
                {
                    if(myReady<otherReady)
                    {

                        mouseflag=1;
                        myColor=true;
                    }
                    else
                    {
                        mouseflag=0;
                        myColor=false;
                    }
                    ui->messageBrowser->append(QStringLiteral("游戏开始！"));
                }
                break;
            case Gamesite:
                in>>userName>>localHostName>>gamesite;
                currentX=gamesite.x();
                currentY=gamesite.y();
                game.peoplePutDown(gamesite.x(),gamesite.y());
                update();
                mouseflag++;
                if(game.ifWin(gamesite.x(),gamesite.y()))
                {
                    if((game.player-1)%2)
                     QMessageBox::about(this,QStringLiteral("恭喜"),QStringLiteral("白棋获胜"));
                    else
                        QMessageBox::about(this,QStringLiteral("恭喜"),QStringLiteral("黑棋获胜"));
                    mouseflag=0;
                }

                update();
                break;
        }
    }
}

void NetGameWindow::on_sendButton_clicked()
{
    sendMessage(Message);

}
void NetGameWindow::newParticipant(QString userName, QString localHostName, QString ipAddress)
{
    bool isEmpty=ui->userTableWidget->findItems(localHostName,Qt::MatchExactly).isEmpty();
    if(isEmpty)
    {
        QTableWidgetItem *user = new QTableWidgetItem(userName);
        QTableWidgetItem *host = new QTableWidgetItem(localHostName);
        QTableWidgetItem *ip = new QTableWidgetItem(ipAddress);
        ui->userTableWidget->verticalHeader()->setVisible(false);
        ui->userTableWidget->horizontalHeader()->setStyleSheet("QHeaderView::Section{background:rgb(178, 113, 60);}");
        ui->userTableWidget->insertRow(0);
        ui->userTableWidget->setItem(0,0,user);
        ui->userTableWidget->setItem(0,1,host);
        ui->userTableWidget->setItem(0,2,ip);
        ui->messageBrowser->setTextColor(Qt::black);
        ui->messageBrowser->setCurrentFont(QFont("Times New Roman",10));
        ui->messageBrowser->append(QStringLiteral("%1在线").arg(userName));
        sendMessage(NewParticipant);
    }
}

// 获得要发送的消息
QString NetGameWindow::getMessage()
{
    QString msg=ui->messageTextEdit->toHtml();//转化成html语言进行发送
    ui->messageTextEdit->clear();//发送完后清空输入框
    ui->messageTextEdit->setFocus();//重新设置光标输入焦点，即焦点保持不变
    return msg;
}

QPoint NetGameWindow::getGame()
{
    return QPoint(currentX,currentY);
}

// 获取ip地址，获取本机ip地址(其协议为ipv4的ip地址)
QString NetGameWindow::getIP()
{
    //QList是Qt中一个容器模板类，是一个数组指针？
    QList<QHostAddress> list = QNetworkInterface::allAddresses();//此处的所有地址是指ipv4和ipv6的地址
    //foreach (variable, container),此处为按照容器list中条目的顺序进行迭代
    foreach (QHostAddress address, list)
    {
        if(address.protocol() == QAbstractSocket::IPv4Protocol)
            return address.toString();
    }
       return 0;
}

// 获取用户名
QString NetGameWindow::getUserName()
{
    QStringList envVariables;
    //将后面5个string存到envVariables环境变量中
    envVariables << "USERNAME.*" << "USER.*" << "USERDOMAIN.*"
                 << "HOSTNAME.*" << "DOMAINNAME.*";
    //系统中关于环境变量的信息存在environment中
    QStringList environment = QProcess::systemEnvironment();
    foreach (QString string, envVariables) {
        //indexOf为返回第一个匹配list的索引,QRegExp类是用规则表达式进行模式匹配的类
        int index = environment.indexOf(QRegExp(string));
        if (index != -1) {
            //stringList中存的是environment.at(index)中出现'='号前的字符串
            QStringList stringList = environment.at(index).split('=');
            if (stringList.size() == 2) {
                return stringList.at(1);//at(0)为文字"USERNAME."，at(1)为用户名
                break;
            }
        }
    }
    return "unknown";
}

NetGameWindow::~NetGameWindow()
{
    delete ui;
}

void NetGameWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing,true);
    QPen pen=painter.pen();
    pen.setColor(QColor("#8D5822"));
    pen.setWidth(7);
    painter.setPen(pen);
    QBrush brush;
    brush.setColor(QColor("#EEC085"));
    brush.setStyle(Qt::SolidPattern);
    painter.setBrush(brush);
    painter.drawRect(20,40,600,600);
    pen.setColor(Qt::black);
    pen.setWidth(1);
    painter.setPen(pen);
    for(int i=0;i<15;i++)
        {
            painter.drawLine(40+i*40,60,40+i*40,620);//纵线
            painter.drawLine(40,60+i*40,600,60+i*40);//横线
        }
    brush.setColor(Qt::black);
    painter.setBrush(brush);
    painter.drawRect(155,175,10,10);
    painter.drawRect(475,175,10,10);
    painter.drawRect(155,495,10,10);
    painter.drawRect(475,495,10,10);
    painter.drawRect(315,335,10,10);
    for (int i = 0; i < 15; i++)
        for (int j = 0; j < 15; j++)
            {
                if (game.chess[i][j] == 4)
                {
                    brush.setColor(Qt::black);
                    painter.setBrush(brush);
                    painter.drawEllipse(QPoint((j + 1) * 40,(i + 1) * 40+20),  18, 18);
                }
                else if (game.chess[i][j] == 5)
                {
                    brush.setColor(Qt::white);
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(brush);
                    painter.drawEllipse(QPoint((j + 1) * 40,(i + 1) * 40+20),  18, 18);
                }
            }
    brush.setColor(Qt::black);
    painter.setBrush(brush);
    painter.drawEllipse(650,80,50,50);
    brush.setColor(Qt::white);
    painter.setPen(Qt::NoPen);
    painter.setBrush(brush);
    painter.drawEllipse(750,80,50,50);
    brush.setColor(Qt::black);
    painter.setBrush(brush);
    if((game.player)%2)
    {
       QPoint points[3]={QPoint(765,160),QPoint(775,145),QPoint(785,160)};
       painter.drawPolygon(points,3);
    }
    else
    {
        QPoint points[3]={QPoint(665,160),QPoint(675,145),QPoint(685,160)};
        painter.drawPolygon(points,3);
    }
    pen.setColor(Qt::red);
    pen.setWidth(1);
    painter.setPen(pen);

    if((moveX*40+40)>=20&&(moveX*40+40)<=620&&(moveY*40+20)>=40&&(moveY*40+20)<=640)
    {
        painter.drawLine((moveY+1)*40-20,(moveX+1)*40,(moveY+1)*40-10,(moveX+1)*40);
        painter.drawLine((moveY+1)*40+20,(moveX+1)*40,(moveY+1)*40+10,(moveX+1)*40);
        painter.drawLine((moveY+1)*40-20,(moveX+1)*40+40,(moveY+1)*40-10,(moveX+1)*40+40);
        painter.drawLine((moveY+1)*40+20,(moveX+1)*40+40,(moveY+1)*40+10,(moveX+1)*40+40);
        painter.drawLine((moveY+1)*40-20,(moveX+1)*40,(moveY+1)*40-20,(moveX+1)*40+10);
        painter.drawLine((moveY+1)*40+20,(moveX+1)*40,(moveY+1)*40+20,(moveX+1)*40+10);
        painter.drawLine((moveY+1)*40-20,(moveX+1)*40+40,(moveY+1)*40-20,(moveX+1)*40+30);
        painter.drawLine((moveY+1)*40+20,(moveX+1)*40+40,(moveY+1)*40+20,(moveX+1)*40+30);
    }
        painter.drawLine((currentY+1)*40-1,(currentX+1)*40+20,(currentY+1)*40-6,(currentX+1)*40+20);
        painter.drawLine((currentY+1)*40+1,(currentX+1)*40+20,(currentY+1)*40+6,(currentX+1)*40+20);
        painter.drawLine((currentY+1)*40,(currentX+1)*40+19,(currentY+1)*40,(currentX+1)*40+14);
        painter.drawLine((currentY+1)*40,(currentX+1)*40+21,(currentY+1)*40,(currentX+1)*40+26);
}
void NetGameWindow::mouseMoveEvent(QMouseEvent *event)
{
    moveX=(event->y()-40)/40;
    moveY=(event->x()-20)/40;
    update();
}

void NetGameWindow::mouseReleaseEvent(QMouseEvent *event)
{
    int x,y;
    if(mouseflag%2)
    {
        x=(event->y()-40)/40;
        y=(event->x()-20)/40;
        if(event->x()>=20&&event->x()<=620&&event->y()>=40&&event->y()<=640)
        {
            if(game.judge(x,y))
            {
                currentX=x;
                currentY=y;
                sendMessage(Gamesite);
                update();
            }
            else
                QMessageBox::information(this,QStringLiteral("注意"),QStringLiteral("已存在棋子，请重下！"),QMessageBox::Ok);
        }
        else
            QMessageBox::information(this,QStringLiteral("注意"),QStringLiteral("不在棋盘内，请重下！"),QMessageBox::Ok);
        update();
    }
}




void NetGameWindow::on_readyBtn_clicked()
{
    sendMessage(Ready);
}
void NetGameWindow::on_giveupBtn_clicked()
{
    int ret=QMessageBox::question(this,QStringLiteral("认输"),QStringLiteral("确定认输?"),QMessageBox::Yes,QMessageBox::No);
    if(ret==QMessageBox::Yes)
    {
        sendMessage(Giveup);

    }
}

void NetGameWindow::on_backBtn_clicked()
{
    int ret=QMessageBox::question(this,QStringLiteral("悔棋"),QStringLiteral("是否请求悔棋"),QMessageBox::Yes,QMessageBox::No);
    if(ret==QMessageBox::Yes)
        sendMessage(Back);
}

void NetGameWindow::on_quitBtn_clicked()
{
    sendMessage(ParticipantLeft);
}

void NetGameWindow::on_actionNet_triggered()
{
    this->close();
    NetGameWindow* NG=new NetGameWindow;
    NG->show();
}

void NetGameWindow::on_actionHome_triggered()
{
    this->close();
    MainWindow* MW=new MainWindow;
    MW->show();
}

void NetGameWindow::on_actionScreenshot_triggered()
{    
    QPixmap screenshot=this->grab(QRect(20,40,600,620));
    QDir *dir = new QDir();
    if(!dir->exists("screenshots")){
        dir->mkdir("screenshots");//判断文件夹是否存在，若不存在新建
    }
    dir->cd("screenshots");
    dir->setCurrent("screenshots");
    long a=QDateTime::currentDateTime().toTime_t();
    QString t = QString::number(a, 16).toUpper();//获得1970-01-01至今的秒数并进行16进制转换，确保截图唯一
    QString filename = QFileDialog::getSaveFileName(this,QStringLiteral("保存图片"),t, "Images (*.png *.xpm *.jpg)");
        if(filename.length()>0)
            screenshot.save(filename);
}

void NetGameWindow::on_actionComputer_triggered()
{
    int ret;
    this->close();
    PeopleVsComputer* pvc=new PeopleVsComputer;
    pvc->show();
    ret= pvc->judgeWhoFirst();
    if(ret==QMessageBox::Yes)
        pvc->youFirst();
    else if(ret==QMessageBox::No)
        pvc->computerFirst();
}

void NetGameWindow::on_actionPeople_triggered()
{

    this->close();
    GameWindow* GW=new GameWindow;
    GW->show();
}

void NetGameWindow::on_actionQuit_triggered()
{
    this->close();
}

void NetGameWindow::on_actionBack_triggered()
{
    int ret=QMessageBox::question(this,QStringLiteral("悔棋"),QStringLiteral("是否确定悔棋"),QMessageBox::Yes,QMessageBox::No);
    if(ret==QMessageBox::Yes)
        sendMessage(Back);
}
