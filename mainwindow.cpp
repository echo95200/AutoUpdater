#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    if(connectDatabase())
    {
        if(initDir())
        {
            ui->textEdit->hide();
            ui->pushButtonClose->hide();
            shellProcess = new QProcess();
            shellProcess->setProcessChannelMode(QProcess::MergedChannels);
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

// To conncet with the database
bool MainWindow::connectDatabase()
{
    bool flag = false;
    QString databaseFilePath = "/home/echo/ventap.fdb";
    QFile dbFile(databaseFilePath);
    if(dbFile.exists())
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QIBASE");
        db.setDatabaseName(dbFile.fileName());
        db.setUserName("SYSDBA");
        db.setPassword("masterkey");
        if(db.open())
        {
            flag = true;
            QSqlQuery queryRef("select pvalue from t_param where pkey='SYS_CLIENT_REF';",db);
            while (queryRef.next()) {
                m_client_ref = queryRef.value(0).toString();
            }
            QSqlQuery queryVersion("select pvalue from t_param where pkey='VERSION';",db);
            while (queryVersion.next()) {
                m_version_old = queryVersion.value(0).toString();
            }
        }
        else
        {
            QMessageBox::about(NULL,"Info","Database file cannot be opened!");
        }
    }
    else
    {
        QMessageBox::about(NULL,"Info","Database file does not exist!");
    }
    return flag;
}

//Create the files of update and backup
bool MainWindow::initDir()
{
    bool flag = false;
    ventapHomePath = "/home/echo/ventap";
    ventapUPDPath = ventapHomePath + QDir::separator() + ".update";
    ventapBAKPath = ventapHomePath + QDir::separator() + ".backup";

    date = QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm");
    dateFilePath = ventapBAKPath + QDir::separator() + date;
    QDir *temp = new QDir;
    // if the update file exits, delete it and create an new update file
    if(temp->exists(ventapUPDPath))
    {
        deleteDirectory(ventapUPDPath);
    }
    temp->mkdir(ventapUPDPath);

    //if the backup file doesn't exist, then create an new backup file
    // Do not delete backup file
    if(!temp->exists(ventapBAKPath))
    {
        temp->mkdir(ventapBAKPath);
    }

    //create date file
    temp->mkdir(dateFilePath);

    flag = temp->exists(ventapUPDPath) && temp->exists(ventapBAKPath) && temp->exists(dateFilePath);
    if(!flag)
    {
        QMessageBox::about(NULL,"Info","Program initialization failure!");
    }
    return flag;
}

// Get the update file from ftp server
bool MainWindow::downloadPackage()
{
    m_FTPLink = "ftp://cloud.arcsolu.fr/";
    m_FTPUser = "update";
    m_FTPPSW = "update";
    m_FTPFileName = "ventap_1.2.0_180515.tar.gz";

    file = new QFile(ventapUPDPath + QDir::separator() + m_FTPFileName);
    file->open(QIODevice::WriteOnly);

    QNetworkAccessManager *accessManager = new QNetworkAccessManager(this);
    accessManager->setNetworkAccessible(QNetworkAccessManager::Accessible);
    QUrl url(m_FTPLink + m_FTPFileName);
    url.setPort(21);
    url.setUserName(m_FTPUser);
    url.setPassword(m_FTPPSW);

    QNetworkRequest request(url);
    reply = accessManager->get(request);

    connect(reply,SIGNAL(readyRead()),this,SLOT(readContent()));
    connect(accessManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(replyFinished(QNetworkReply*)));
    connect(reply,SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(loadError(QNetworkReply::NetworkError)));

}

//SLOT to read content
void MainWindow::readContent()
{
    file->write(reply->readAll());
    ui->textEdit->append("Download from FTP server...");
}

//SLOT to close the update file
void MainWindow::replyFinished(QNetworkReply *)
{
    if(reply->error() == QNetworkReply::NoError)
    {
        reply->deleteLater();
        file->flush();
        file->close();
        ui->textEdit->append("Download finished...");

        //Start the process of update
        //this->updateNewVersion();
    }
    else
    {
        QMessageBox::about(NULL,"Info","File transfer error!");
    }

}

//SLOT to input error informations
void MainWindow::loadError(QNetworkReply::NetworkError)
{
    ui->textEdit->append(reply->errorString());
}

//Delete the directory and the files
bool MainWindow::deleteDirectory(QString path)
{
    if(path.isEmpty())
    {
        return false;
    }
    QDir dir(path);
    if(!dir.exists())
    {
        return true;
    }

    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    QFileInfoList fileList = dir.entryInfoList();
    foreach (QFileInfo fi, fileList)
    {
        if(fi.isFile())
        {
            fi.dir().remove(fi.fileName());
        }
        else
        {
            deleteDirectory(fi.absoluteFilePath());
        }
    }
    return dir.rmpath(dir.absolutePath());
}

// create backup file
bool MainWindow::backupCurrentVersion()
{
    // lib file
    QString sourceDirLib = ventapHomePath + QDir::separator() + "lib";
    QString targetDirLib = dateFilePath + QDir::separator() + "lib";
    copyDirectoryFiles(sourceDirLib,targetDirLib,true);

    // plugins file
    QString sourceDirPlugins = ventapHomePath + QDir::separator() + "plugins";
    QString targetDirPlugins = dateFilePath + QDir::separator() + "plugins";
    copyDirectoryFiles(sourceDirPlugins,targetDirPlugins,true);

    //VtMain
    QString sourceVtMain = ventapHomePath + QDir::separator() + "VtMain";
    QString targetVtMain = dateFilePath + QDir::separator() + "VtMain";
    copyFileToPath(sourceVtMain,targetVtMain,true);

    QString targetTarFile = "./" + date + ".tar.gz";
    QString targetTarPath = "./" + date;

    QProcess process;
    process.setWorkingDirectory(ventapBAKPath);
    QString command = "tar -zcf " + targetTarFile + " " + targetTarPath;
    process.start(command);
    process.waitForFinished();
    process.close();
    ui->textEdit->append("Compress end...");
}

bool MainWindow::updateNewVersion()
{
    ui->textEdit->append("********************\n");
    ui->textEdit->append("Update begins");
    QProcess process;
    process.setWorkingDirectory(ventapUPDPath);
    QString command = "tar -zxf " + m_FTPFileName;
    process.start(command);
    process.waitForFinished();
    process.close();

    // lib file
    QString sourceDirLib = ventapUPDPath + QDir::separator() + "lib";
    QString targetDirLib = ventapHomePath + QDir::separator() + "lib";
    copyDirectoryFiles(sourceDirLib,targetDirLib,true);

    // plugins file
    QString sourceDirPlugins = ventapUPDPath + QDir::separator() + "plugins";
    QString targetDirPlugins = ventapHomePath + QDir::separator() + "plugins";
    copyDirectoryFiles(sourceDirPlugins,targetDirPlugins,true);

    //VtMain
    QString sourceVtMain = ventapUPDPath + QDir::separator() + "VtMain";
    QString targetVtMain = ventapHomePath + QDir::separator() + "VtMain";
    copyFileToPath(sourceVtMain,targetVtMain,true);

    ui->textEdit->append("Update ends...");
}

//copy file to target path
bool MainWindow::copyFileToPath(QString sourceDir, QString toDir, bool coverFileIfExist)
{
    toDir.replace("\\","/");
    if(sourceDir == toDir){
        return true;
    }
    if(!QFile::exists(sourceDir)){
        return false;
    }
    QDir* createFile = new QDir;
    bool exist = createFile->exists(toDir);
    if(exist)
    {
        if(coverFileIfExist)
        {
            createFile->remove(toDir);
        }
    }

    if(!QFile::copy(sourceDir,toDir))
    {
        return false;
    }
    else
    {
        ui->textEdit->append("Copy " + sourceDir + " To " +  toDir);
    }
    return true;
}

// copy directory to target path
bool MainWindow::copyDirectoryFiles(QString fromDir, QString toDir, bool coverFileIfExist)
{
    QDir sourceDir(fromDir);
    QDir targetDir(toDir);
    if(!targetDir.exists()){
        if(!targetDir.mkdir(targetDir.absolutePath()))
            return false;
    }

    QFileInfoList fileInfoList = sourceDir.entryInfoList();
    foreach (QFileInfo fileInfo, fileInfoList) {
        if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;
        //If the file is an directory
        if(fileInfo.isDir()){
            if(!copyDirectoryFiles(fileInfo.filePath(),
                                   targetDir.filePath(fileInfo.fileName()),
                                   coverFileIfExist))
                return false;
        }else {
            if(coverFileIfExist && targetDir.exists(fileInfo.fileName())){
                targetDir.remove(fileInfo.fileName());
            }

            if(!QFile::copy(fileInfo.filePath(),
                            targetDir.filePath(fileInfo.fileName()))){
                return false;
            }
            else {
                ui->textEdit->append("Copy " + fileInfo.filePath() + " To " + targetDir.filePath(fileInfo.fileName()));
            }
        }
    }
    return true;
}

void MainWindow::on_pushButtonUpdate_clicked()
{
    ui->pushButtonUpdate->hide();
    ui->textEdit->show();
    ui->pushButtonClose->show();
    ui->pushButtonClose->setEnabled(false);

    this->downloadPackage();
//    ui->textEdit->append("Backup begins...");
//    this->backupCurrentVersion();
//    ui->textEdit->append("Backup ends...");
//    ui->pushButtonClose->setEnabled(true);


//    updateFile = QCoreApplication::applicationDirPath() + QDir::separator() + "update1.2.sh";
//    if(!isFileExist(updateFile))
//    {
//        ui->textEdit->setText("Le fichier n'existe pas!");
//        ui->pushButtonClose->setEnabled(true);
//    }
//    else
//    {
//        connect(shellProcess,SIGNAL(readyRead()),this,SLOT(getOutputProcess()));
//        connect(shellProcess,SIGNAL(finished(int)),this,SLOT(finishedProcess()));
//        shellProcess->start("/bin/bash",QStringList(updateFile));
//    }

}

void MainWindow::getOutputProcess()
{
    QByteArray output = shellProcess->readAllStandardOutput();
    QString outStr;
    outStr.append(output);
    ui->textEdit->append(output);
}

void MainWindow::finishedProcess()
{
    shellProcess->close();
    QFile file(updateFile);
    if(file.exists())
    {
        file.remove();
    }
    ui->pushButtonClose->setEnabled(true);
}

void MainWindow::on_pushButtonClose_clicked()
{
    // create the file of log
    QString logPath = QCoreApplication::applicationDirPath() +QDir::separator() + "log.txt";
    QFile logFile(logPath);
    logFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream logStream(&logFile);
    QDateTime time = QDateTime::currentDateTime();
    QString timeStr = time.toString();
    int timeT = time.toTime_t();
    logStream << "\n";
    logStream << timeT;
    logStream << "   ";
    logStream << timeStr << "\n";
    logStream << ui->textEdit->toPlainText() << "\n";
    logFile.flush();
    logFile.close();
    this->close();
}

bool MainWindow::isFileExist(QString fileName)
{
    QFileInfo fileInfo(fileName);
    if(fileInfo.isFile())
    {
        return true;
    }
    return false;
}
