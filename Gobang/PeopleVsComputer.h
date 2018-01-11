#ifndef PEOPLEVSCOMPUTER_H
#define PEOPLEVSCOMPUTER_H

#include <QMainWindow>
#include <QMouseEvent>
#include "Game.h"
#include "DatabaseConnection.h"
namespace Ui {
class PeopleVsComputer;
}

class PeopleVsComputer : public QMainWindow
{
    Q_OBJECT

public:
    explicit PeopleVsComputer(QWidget *parent = 0);
    ~PeopleVsComputer();
    virtual void paintEvent(QPaintEvent*);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent *event);
    int judgeWhoFirst();
    void youFirst();
    void computerFirst();
    void setXY(int x,int y);
    Game game;
    bool mouseflag;
private slots:
    void on_actionBack_triggered();
    void on_actionPeople_triggered();
    void on_actionQuit_triggered();
    void on_actionNet_triggered();
    void on_actionHome_triggered();
    void on_actionScreenshot_triggered();
    void on_actionComputer_triggered();
    void on_actionOnFile_triggered();
private:
    Ui::PeopleVsComputer *ui;
    DatabaseConnection dbcon;
    int currentX;
    int currentY;
    int moveX;
    int moveY;
};

#endif // PEOPLEVSCOMPUTER_H
