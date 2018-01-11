#include "LoadGame.h"
#include "MainWindow.h"
#include "ui_LoadGame.h"
#include <DatabaseConnection.h>
#include "GameWindow.h"
#include "NetGameWindow.h"
#include "PeopleVsComputer.h"
#include <memory>
#include <QSqlQuery>
#include <QDateTime>
#include <QIcon>
#include <QTextCodec>
#include <QHeaderView>
#include <QScrollBar>
#include <QDebug>

LoadGame::LoadGame(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LoadGame)
{
    ui->setupUi(this);
    this->setFixedSize(411,215);
    dbcon.connect();
    dbcon.db();
    dataInit();
}

LoadGame::~LoadGame()
{
    dbcon.close();
    delete ui;
}
void LoadGame::dataInit()
{
    //查询数据
    QSqlQuery query;
    query.exec("select (@i:=@i+1) i,gobang.chesspieces.* from gobang.chesspieces,(select @i:=0) as i");

    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    ui->tableWidget_LoadGame->setEditTriggers(QAbstractItemView::NoEditTriggers);//设置单元格不可编辑
    ui->tableWidget_LoadGame->setSelectionBehavior(QAbstractItemView::SelectRows);//设置选择行为时每次选择一行
    ui->tableWidget_LoadGame->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//设置为自动填充方式
    ui->tableWidget_LoadGame->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tableWidget_LoadGame->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);//显示滚动条
    ui->tableWidget_LoadGame->setFrameShape(QFrame::NoFrame);      //设置无边框
    ui->tableWidget_LoadGame->setShowGrid(false); //设置不显示格子线
    ui->tableWidget_LoadGame->setColumnCount(3);//设置列数
    ui->tableWidget_LoadGame->setRowCount(query.size());//设置行数
    //创建表头
    ui->tableWidget_LoadGame->horizontalHeader()->setDefaultSectionSize(100);
    ui->tableWidget_LoadGame->horizontalHeader()->setFixedHeight(25);         //设置表头的高度
    ui->tableWidget_LoadGame->verticalHeader()->setDefaultSectionSize(30);    //设置默认行高
    ui->tableWidget_LoadGame->verticalHeader()->setHidden(true);    //隐藏行号
    ui->tableWidget_LoadGame->setFocusPolicy(Qt::NoFocus); //失去焦点

    QFont font = ui->tableWidget_LoadGame->horizontalHeader()->font();        //设置表头字体加粗
    font.setBold(true);
    font.setFamily("Helvetica");
    ui->tableWidget_LoadGame->horizontalHeader()->setFont(font);
    ui->tableWidget_LoadGame->horizontalHeader()->setStyleSheet("QHeaderView::section{background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(134, 245, 99, 255),stop:0.5 rgba(134, 148, 99, 255),stop:1 rgba(115, 87, 128, 255));color:rgb(25, 70, 100);padding-left: 1px;border: 1px solid #FFFF00;}"); //设置表头背景色
    ui->tableWidget_LoadGame->horizontalHeader()->setStretchLastSection(true);      //设置充满表宽度
    QStringList heads;
    heads<<codec->toUnicode("棋局名称")<<codec->toUnicode("棋局类型")<<codec->toUnicode("存档时间");
    ui->tableWidget_LoadGame->setHorizontalHeaderLabels(heads);

    ui->tableWidget_LoadGame->setStyleSheet("selection-background-color:lightblue;");
    //设置水平、垂直滚动条样式
    ui->tableWidget_LoadGame->horizontalScrollBar()->setStyleSheet("QScrollBar{background:transparent; height:12px;}"
                                             "QScrollBar::handle{background:lightgray; border:2px solid transparent; border-radius:5px;}"
                                             "QScrollBar::handle:hover{background:gray;}"
                                             "QScrollBar::sub-line{background:transparent;}"
                                             "QScrollBar::add-line{background:transparent;}");

    ui->tableWidget_LoadGame->verticalScrollBar()->setStyleSheet("QScrollBar{background:transparent; width:12px;}"
                                           "QScrollBar::handle{background:lightgray; border:2px solid transparent; border-radius:5px;}"
                                           "QScrollBar::handle:hover{background:gray;}"
                                           "QScrollBar::sub-line{background:transparent;}"
                                           "QScrollBar::add-line{background:transparent;}");
    ui->tableWidget_LoadGame->horizontalHeader()->setHighlightSections(false);    //点击表时不对表头行光亮（获取焦点）
    //填充表格
    while(query.next()){
        bool ok;
        int cpid = query.value(0).toInt()-1;
        QTableWidgetItem *item1 = new QTableWidgetItem();
        QTableWidgetItem *item2 = new QTableWidgetItem();
        QTableWidgetItem *item3 = new QTableWidgetItem();
        if(query.value(3).toString()=="人人")
            item2->setIcon(QIcon(":/static/image/man.png"));
        else
            item2->setIcon(QIcon(":/static/image/robot.png"));
        item1->setText(query.value(2).toString());item1->setTextAlignment(Qt::AlignCenter);
        item2->setText(query.value(3).toString());
        item3->setText(QDateTime::fromTime_t(query.value(2).toString().toInt(&ok,16)).toString(Qt::ISODate));item3->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget_LoadGame->setItem(cpid,0,item1);
        ui->tableWidget_LoadGame->setItem(cpid,1,item2);
        ui->tableWidget_LoadGame->setItem(cpid,2,item3);
    }
    setBtnQss(ui->pushButton_home, "#3498DB", "#FFFFFF", "#5DACE4", "#E5FEEF", "#2483C7", "#A0DAFB");
    setBtnQss(ui->pushButton_load, "#34495E", "#FFFFFF", "#4E6D8C", "#F0F0F0", "#2D3E50", "#B8C6D1");
    setBtnQss(ui->pushButton_delete, "#1ABC9C", "#E6F8F5", "#2EE1C1", "#FFFFFF", "#16A086", "#A7EEE6");
}

void LoadGame::setBtnQss(QPushButton *btn,
                        QString normalColor, QString normalTextColor,
                        QString hoverColor, QString hoverTextColor,
                        QString pressedColor, QString pressedTextColor)
{
         QStringList qss;
         qss.append(QString("QPushButton{border-style:none;padding:5px;border-radius:3px;color:%1;background:%2;}").arg(normalTextColor).arg(normalColor));
         qss.append(QString("QPushButton:hover{color:%1;background:%2;}").arg(hoverTextColor).arg(hoverColor));
         qss.append(QString("QPushButton:pressed{color:%1;background:%2;}").arg(pressedTextColor).arg(pressedColor));
         btn->setStyleSheet(qss.join(""));
}

void LoadGame::on_pushButton_load_clicked()
{
    int rowIndex = ui->tableWidget_LoadGame->currentRow();
    QString cpname = ui->tableWidget_LoadGame->item(rowIndex,0)->text();
    QSqlQuery query;
    query.prepare(QString("SELECT * FROM chesspieces WHERE cpname = ?"));
    query.addBindValue(cpname);
    query.exec();
    QString cptype,cpdata;
    QChar* str_cpdata;
    int chess[15][15],player,currentX,currentY;
    if(query.next()){
        cptype = query.value(2).toString();
        cpdata = query.value(3).toString();
        currentX = query.value(4).toInt();
        currentY = query.value(5).toInt();
        player = query.value(8).toInt();
        str_cpdata = new QChar[225];
        str_cpdata = cpdata.data();
        for (int i = 0; i < 15; i++)
            for(int j = 0; j < 15; j++)
               chess[i][j] = str_cpdata[i*15+j].digitValue();
        if(cptype == "人人"){
            GameWindow* GW=new GameWindow;
            GW->whiteTimes = query.value(6).toInt();
            GW->blackTimes = query.value(7).toInt();
            GW->timerUpdate();
            memcpy(GW->game.chess,chess,sizeof(chess));
            GW->setXY(currentX,currentY);
            GW->game.player = player;
            GW->mouseflag = true;
            this->close();
            GW->show();
        }
        else{
            PeopleVsComputer* PVC = new PeopleVsComputer();
            memcpy(PVC->game.chess,chess,sizeof(chess));
            PVC->setXY(currentX,currentY);
            PVC->game.player = player;
            PVC->game.computerColor = query.value(9).toInt();
            PVC->mouseflag = true;
            this->close();
            PVC->show();
        }
    }
}

void LoadGame::on_pushButton_delete_clicked()
{
    int rowIndex = ui->tableWidget_LoadGame->currentRow();
    QString cpname = ui->tableWidget_LoadGame->item(rowIndex,0)->text();
    if (rowIndex != -1)
          ui->tableWidget_LoadGame->removeRow(rowIndex);
    QSqlQuery query;
    query.prepare(QString("DELETE FROM chesspieces WHERE cpname = ?"));
    query.addBindValue(cpname);
    query.exec();
}

void LoadGame::on_pushButton_home_clicked()
{
    this->close();
    MainWindow *MW = new MainWindow();
    MW->show();
}
