#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QFileDevice>
#include <QStringList>
#include <QDir>
#include <QMessageBox>
#include <QTextStream>
#include <QDateTime>
#include <QFileInfo>
#include <QGridLayout>
#include <QtSql/QSqlDatabase>
#include <QSqlQuery>
#include <QDateTime>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool isFileExist(QString fileName);
    bool connectDatabase();
    bool initDir();
    bool deleteDirectory(QString path);
    bool downloadPackage();
    bool backupCurrentVersion();
    bool updateNewVersion();
    bool copyFileToPath(QString sourceDir,QString toDir,bool coverFileIfExist);
    bool copyDirectoryFiles(QString fromDir,QString toDir,bool coverFileIfExist);

private slots:
    void on_pushButtonUpdate_clicked();
    void on_pushButtonClose_clicked();
    void getOutputProcess();
    void finishedProcess();

    void readContent();
    void replyFinished(QNetworkReply*);
    void loadError(QNetworkReply::NetworkError);

private:
    Ui::MainWindow *ui;
    QProcess *shellProcess;
    QString updateFile;

    QString m_client_ref;
    QString m_version_old;

    QFile* file;
    QNetworkReply* reply;
    QString ventapHomePath;
    QString ventapUPDPath;
    QString ventapBAKPath;
    QString date;
    QString dateFilePath;

    QString m_FTPLink;
    QString m_FTPUser;
    QString m_FTPPSW;
    QString m_FTPFileName;

};

#endif // MAINWINDOW_H
