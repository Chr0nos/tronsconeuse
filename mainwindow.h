#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "spliter.h"

#include <QMainWindow>
#include <QThread>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
    Spliter* spliter;
    QThread* thread;

private slots:
    void on_actionQuitter_triggered();
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void cleanSpliter();
    void refreshEnabled();
    void debug(const QString message);
public slots:

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
