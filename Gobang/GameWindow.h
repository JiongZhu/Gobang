#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include "Game.h"
#include "DatabaseConnection.h"
namespace Ui {
class GameWindow;
}

class GameWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GameWindow(QWidget *parent = 0);
    ~GameWindow();
    virtual void paintEvent(QPaintEvent*);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent *event);
    void timerUpdate();
    void setXY(int x, int y);
    QTimer* wtimerId;
    QTimer* btimerId;
    Game game;
    int whiteTimes;
    int blackTimes;
    bool mouseflag;
private:
    Ui::GameWindow *ui;
    DatabaseConnection dbcon;
    int currentX;
    int currentY;
    int moveX;
    int moveY;
private slots:
    void btimerUpdate();
    void wtimerUpdate();
    void on_actionBack_triggered();
    void on_actionHome_triggered();
    void on_actionNet_triggered();
    void on_actionQuit_triggered();
    void on_actionPeople_triggered();
    void on_actionComputer_triggered();
    void on_actionScreenshot_triggered();
    void on_actionOnFile_triggered();
};

#endif // GAMEWINDOW_H
