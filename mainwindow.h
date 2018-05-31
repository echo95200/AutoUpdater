#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButtonUpdate_clicked();

    void getOutputProcess();
    void finishedProcess();

    void on_pushButtonClose_clicked();
    bool isFileExist(QString fileName);

private:
    Ui::MainWindow *ui;
    QProcess *shellProcess;
    QString shFile;

};

#endif // MAINWINDOW_H
