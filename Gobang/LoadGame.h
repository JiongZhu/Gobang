#ifndef LOADGAME_H
#define LOADGAME_H

#include <QMainWindow>
#include <QPushButton>
#include "DatabaseConnection.h"
#include <QSqlDatabase>

namespace Ui {
class LoadGame;
}

class LoadGame : public QMainWindow
{
    Q_OBJECT

public:
    explicit LoadGame(QWidget *parent = 0);
    ~LoadGame();

private slots:
    void dataInit();
    void setBtnQss(QPushButton *btn,QString normalColor, QString normalTextColor,
                            QString hoverColor, QString hoverTextColor,
                            QString pressedColor, QString pressedTextColor);

    void on_pushButton_load_clicked();

    void on_pushButton_delete_clicked();

    void on_pushButton_home_clicked();

private:
    Ui::LoadGame *ui;
    DatabaseConnection dbcon;
};

#endif // LOADGAME_H
