#include "mainwindow.h"
#include "ui_mainwindow.h"

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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->textEdit->hide();
    ui->pushButtonClose->hide();
    shellProcess = new QProcess();
    shellProcess->setProcessChannelMode(QProcess::MergedChannels);
//    QGridLayout *layout = new QGridLayout();
//    layout->addWidget(ui->pushButtonUpdate,0,0);
//    layout->addWidget(ui->textEdit,0,1);
//    layout->addWidget(ui->pushButtonClose,2,1);
//    this->setLayout(layout);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButtonUpdate_clicked()
{
    ui->pushButtonUpdate->hide();
    //ui->textEdit->setProperty("mandatoryField", true);
    ui->textEdit->show();
    ui->pushButtonClose->show();
    ui->pushButtonClose->setEnabled(false);

    shFile = QCoreApplication::applicationDirPath() + QDir::separator() + "temp.sh";
    //QFile::copy(":/shFile/Resources/test.sh",shFile);
    QString updateFile = QCoreApplication::applicationDirPath() + QDir::separator() + "update1.2.sh";
    if(!isFileExist(updateFile))
    {
        ui->textEdit->setText("Le fichier n'existe pas!");
        ui->pushButtonClose->setEnabled(true);
    }
    else
    {
        QFile::copy(updateFile,shFile);
        QFile::setPermissions(shFile,
                          QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                          QFileDevice::ExeOwner | QFileDevice::ReadUser |
                          QFileDevice::WriteUser | QFileDevice::ExeUser |
                          QFileDevice::ReadGroup | QFileDevice::WriteGroup |
                          QFileDevice::ExeGroup | QFileDevice::ReadOther |
                          QFileDevice::WriteOther | QFileDevice::ExeOther);

        connect(shellProcess,SIGNAL(readyRead()),this,SLOT(getOutputProcess()));
        connect(shellProcess,SIGNAL(finished(int)),this,SLOT(finishedProcess()));
        shellProcess->start("/bin/sh",QStringList(updateFile));
    }
}

void MainWindow::getOutputProcess()
{
    QByteArray output = shellProcess->readAllStandardOutput();
    QString outStr;
    outStr.append(output);
    qDebug() << outStr;
    ui->textEdit->append(output);

//    QByteArray error = shellProcess->readAllStandardError();
//    ui->textEdit->append(error);
}

void MainWindow::finishedProcess()
{
    shellProcess->close();
    QFile file(shFile);
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
