#include "GameWindow.h"
#include "ui_GameWindow.h"
#include <QPainter>
#include <QMessageBox>
#include "NetGameWindow.h"
#include "PeopleVsComputer.h"
#include "MainWindow.h"
#include <QSqlQuery>
#include <QDateTime>
#include <QTimer>
#include <QFileDialog>
GameWindow::GameWindow(QWidget *parent):
    QMainWindow(parent),game(),
    ui(new Ui::GameWindow)
{
    ui->setupUi(this);
    mouseflag=true;
    whiteTimes=300;
    blackTimes=300;
    this->setFixedSize(850,660);
    this->setAutoFillBackground(true);
    QPalette palette;
    palette.setColor(QPalette::Background,QColor("#B1723C"));
    this->setPalette(palette);
    centralWidget()->setMouseTracking(true);
    setMouseTracking(true);
    btimerId=new QTimer(this);
    wtimerId=new QTimer(this);
    connect(btimerId,SIGNAL(timeout()),this,SLOT(btimerUpdate()));
    connect(wtimerId,SIGNAL(timeout()),this,SLOT(wtimerUpdate()));
}

GameWindow::~GameWindow()
{
    disconnect(btimerId,SIGNAL(timeout()),this,SLOT(btimerUpdate()));
    disconnect(wtimerId,SIGNAL(timeout()),this,SLOT(wtimerUpdate()));
    delete ui;
}

void GameWindow::setXY(int x,int y)
{
    currentX = x;
    currentY = y;
}

void GameWindow::paintEvent(QPaintEvent *)
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
    painter.drawEllipse(700,120,50,50);
    brush.setColor(Qt::white);
    painter.setPen(Qt::NoPen);
    painter.setBrush(brush);
    painter.drawEllipse(700,400,50,50);
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

void GameWindow::mouseReleaseEvent(QMouseEvent* event)
{
    int x,y;
    if(mouseflag)
    {
        x=(event->y()-40)/40;
        y=(event->x()-20)/40;
        if(event->x()>=20&&event->x()<=620&&event->y()>=40&&event->y()<=640)
        {
            if(game.judge(x,y))
            {
                game.peoplePutDown(x,y);
                if((game.player-1)%2)
                {
                    wtimerId->stop();
                    btimerId->start(1000);
                }
                else
                {
                    btimerId->stop();
                    wtimerId->start(1000);
                }
                currentX=x;
                currentY=y;
                update();
                if(game.ifWin(x,y))
                {
                    if((game.player-1)%2)
                        QMessageBox::about(this,QStringLiteral("恭喜"),QStringLiteral("白棋获胜"));
                    else
                        QMessageBox::about(this,QStringLiteral("恭喜"),QStringLiteral("黑棋获胜"));
                    disconnect(btimerId,SIGNAL(timeout()),this,SLOT(btimerUpdate()));
                    disconnect(wtimerId,SIGNAL(timeout()),this,SLOT(wtimerUpdate()));
                    mouseflag=false;
                }
            }
            else
                QMessageBox::information(this,QStringLiteral("注意"),QStringLiteral("已存在棋子，请重下！"),QMessageBox::Ok);
        }
        else
            QMessageBox::information(this,QStringLiteral("注意"),QStringLiteral("不在棋盘内，请重下！"),QMessageBox::Ok);
        update();
    }
}

void GameWindow::mouseMoveEvent(QMouseEvent *event)
{
    moveX=(event->y()-40)/40;
    moveY=(event->x()-20)/40;
    update();
}

void GameWindow::timerUpdate()
{
    QString wtimeString=QString("%1-%2").arg(whiteTimes/60,2,10,QChar('0')).arg(whiteTimes%60,2,10,QChar('0'));
    ui->whiteTime->setText(wtimeString);
    QString btimeString=QString("%1-%2").arg(blackTimes/60,2,10,QChar('0')).arg(blackTimes%60,2,10,QChar('0'));
    ui->blackTime->setText(btimeString);
}

void GameWindow::btimerUpdate()
{
    QString timeString=QString("%1-%2").arg(whiteTimes/60,2,10,QChar('0')).arg(whiteTimes%60,2,10,QChar('0'));
    ui->whiteTime->setText(timeString);
    if(!whiteTimes)
    {
        QMessageBox::about(this,QStringLiteral("恭喜"),QStringLiteral("黑棋超时,白棋获胜!"));
        disconnect(btimerId,SIGNAL(timeout()),this,SLOT(btimerUpdate()));
        disconnect(wtimerId,SIGNAL(timeout()),this,SLOT(wtimerUpdate()));
        mouseflag=false;
    }
    else
        whiteTimes--;
}

void GameWindow::wtimerUpdate()
{
    QString timeString=QString("%1-%2").arg(blackTimes/60,2,10,QChar('0')).arg(blackTimes%60,2,10,QChar('0'));
    ui->blackTime->setText(timeString);
    if(!blackTimes)
    {
        QMessageBox::about(this,QStringLiteral("恭喜"),QStringLiteral("白棋超时,黑棋获胜!"));
        disconnect(btimerId,SIGNAL(timeout()),this,SLOT(btimerUpdate()));
        disconnect(wtimerId,SIGNAL(timeout()),this,SLOT(wtimerUpdate()));
        mouseflag=false;
    }
    else
        blackTimes--;
}

void GameWindow::on_actionBack_triggered()
{
    if(game.player<2)
        QMessageBox::warning(this,QStringLiteral("警告"),QStringLiteral("当前下棋数小于两手，无法悔棋!"),QMessageBox::Yes,QMessageBox::NoButton);
    else
        game.backChess();
    mouseflag = true;
}

void GameWindow::on_actionHome_triggered()
{
    this->close();
    MainWindow* MW=new MainWindow;
    MW->show();
}

void GameWindow::on_actionNet_triggered()
{

    this->close();
    NetGameWindow* NGW=new NetGameWindow;
    NGW->show();
}

void GameWindow::on_actionQuit_triggered()
{
    this->close();
}

void GameWindow::on_actionPeople_triggered()
{
    game.newChess();
    blackTimes=300;
    whiteTimes=300;
    currentX=0;
    currentY=0;
    mouseflag=true;
}

void GameWindow::on_actionComputer_triggered()
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

void GameWindow::on_actionScreenshot_triggered()
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

void GameWindow::on_actionOnFile_triggered()
{
    dbcon.connect();
    dbcon.db();
    long a=QDateTime::currentDateTime().toTime_t();
    QString cptime = QString::number(a, 16).toUpper();
    QString cpdata;
    for (int i = 0; i < 15; i++)
        for (int j = 0; j < 15; j++)
            {
                if (game.chess[i][j] == 0)
                    cpdata+='0';
                else if (game.chess[i][j] == 4)
                    cpdata+='4';
                else
                    cpdata+='5';
            }
    QSqlQuery query;
    query.prepare("INSERT INTO chesspieces(cpname,cptype,cpdata,currentX,currentY,whitetimes,blacktimes,player) VALUES(:cpname,:cptype,:cpdata,:currentX,:currentY,:whitetimes,:blacktimes,:player)");
    query.bindValue(":cpname",cptime);
    query.bindValue(":cptype","人人");
    query.bindValue(":cpdata",cpdata);
    query.bindValue(":currentX",currentX);
    query.bindValue(":currentY",currentY);
    query.bindValue(":whitetimes",whiteTimes);
    query.bindValue(":blacktimes",blackTimes);
    query.bindValue(":player",game.player);
    query.exec();
    dbcon.close();
}
