#ifndef NETGAMEWINDOW_H
#define NETGAMEWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include "Game.h"
#include "DatabaseConnection.h"
namespace Ui {
class NetGameWindow;
}
class QUdpSocket;

// 枚举变量标志信息的类型，分别为消息，新用户加入，用户退出，准备，返回。。。
enum MessageType{Message,NewParticipant,ParticipantLeft,Ready,Back,Gamesite,returnInt,Giveup};
class NetGameWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit NetGameWindow(QWidget *parent = 0);
    ~NetGameWindow();
    virtual void paintEvent(QPaintEvent*);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent *event);
    Game game;
    int mouseflag;
private:
    Ui::NetGameWindow *ui;
    DatabaseConnection dbcon;
    QUdpSocket *udpSocket;
    qint16 port; bool isStart;
    int moveX;
    int moveY;
    int currentX;
    int currentY;
    int myReady;
    int otherReady;
    bool myColor;
    int returnint;
    int firstReady;
protected:
    void newParticipant(QString userName,QString localHostName,QString ipAddress);
    void sendMessage(MessageType type);
    void participantLeft(QString userName,QString localHostName, QString time);
    QString getIP();
    QString getUserName();
    QString getMessage();
    QPoint getGame();
private slots:
    void processPendingDatagrams();
    void on_sendButton_clicked();
    void on_readyBtn_clicked();
    void on_giveupBtn_clicked();
    void on_backBtn_clicked();
    void on_quitBtn_clicked();
    void on_actionNet_triggered();
    void on_actionHome_triggered();
    void on_actionScreenshot_triggered();
    void on_actionComputer_triggered();
    void on_actionPeople_triggered();
    void on_actionQuit_triggered();
    void on_actionBack_triggered();
};

#endif // NETGAMEWINDOW_H
