#include "autoupdater.h"

AutoUpdater::AutoUpdater(QWidget *parent) : QWidget(parent)
{

    m_pLabel = new QLabel(this);
    m_pLabel->setText("AutoUpdater");

    m_pExitPushPutton = new QPushButton(this);
    m_pUpdatePushButton = new QPushButton(this);
    m_pExitPushPutton->setText("Exit");
    m_pUpdatePushButton->setText("Update");
    QGridLayout *pLayout = new QGridLayout(this);
    pLayout->addWidget(m_pLabel,0,0);
    pLayout->addWidget(m_pUpdatePushButton,1,0);
    pLayout->addWidget(m_pExitPushPutton,1,1);

    this->setLayout(pLayout);

    this->getInfoFromDatabaseLocal();
    if (this->initConnectDatabase()) {
        if (this->checkNewVersonExists()) {
            m_pLabel->setText("You have a new version to update!");
        } else {
            m_pUpdatePushButton->setEnabled(false);
            m_pLabel->setText("You don't have new version to update!");
        }
    } else {
        m_pLabel->setText("Please contact our customer service : ");
        m_pUpdatePushButton->setEnabled(false);
    }

    this->initDir();

    connect(m_pUpdatePushButton,SIGNAL(clicked(bool)),this,SLOT(updatePushButtonClickedSlot()));
    connect(m_pExitPushPutton,SIGNAL(clicked(bool)),this,SLOT(exitPushButtonClickedSlot()));
}

// To conncet with the database local
bool AutoUpdater::getInfoFromDatabaseLocal()
{
    bool flag = false;
    QString databaseFilePath = "/home/echo/ventap.fdb";
    QFile dbFile(databaseFilePath);
    if (dbFile.exists()) {
        QSqlDatabase db = QSqlDatabase::addDatabase("QIBASE");
        db.setDatabaseName(dbFile.fileName());
        db.setUserName("SYSDBA");
        db.setPassword("masterkey");
        if (db.open()) {
            flag = true;
            QSqlQuery queryRef("select pvalue from t_param where pkey='SYS_CLIENT_REF';",db);
            while (queryRef.next()) {
                m_sClientRef = queryRef.value(0).toString();
                qDebug() << m_sClientRef;
            }
            QSqlQuery queryVersion("select pvalue from t_param where pkey='VERSION';",db);
            while (queryVersion.next()) {
                m_sVersionOld = queryVersion.value(0).toString();
                qDebug() << m_sVersionOld;
            }
        } else {
            QMessageBox::about(NULL,"Info","Database file cannot be opened!");
        }
        db.close();
    } else {
        QMessageBox::about(NULL,"Info","Database file does not exist!");
    }
    return flag;
}

//Connect the serveur database
bool AutoUpdater::initConnectDatabase()
{
    bool flag = false;
    QString strFlagActive = "";
    m_iIdClient = -1;
    QString databaseFilePath = "/home/echo/Database/autoUpdate.fdb";
    QFile dbFile(databaseFilePath);
    if (dbFile.exists()) {
        QSqlDatabase db = QSqlDatabase::database();
        db.setDatabaseName(dbFile.fileName());
        db.setUserName("SYSDBA");
        db.setPassword("masterkey");
        if (db.open()) {
            QString sql = "SELECT F_ID,F_ACTIVE FROM T_CUSTOMER WHERE F_SYS_CLIENT_REF = ?";
            QSqlQuery query1(db);
            query1.prepare(sql);
            query1.bindValue(0,m_sClientRef);
            if (!query1.exec()) {
                QMessageBox::about(NULL,"ERROR",query1.lastError().text());
            } else {
                if (query1.next()) {
                    m_iIdClient = query1.value(0).toInt();
                    strFlagActive = query1.value(1).toString().trimmed();
                    qDebug() << m_iIdClient;
                    qDebug() << strFlagActive;
                }
            }
            flag = true;

            //If the customer does not exist in the database
            if (m_iIdClient == -1) {
                sql = "INSERT INTO T_CUSTOMER VALUES(NULL,?,'NO',?,'NO',NULL)";
                QSqlQuery query2(db);
                query2.prepare(sql);
                query2.bindValue(0,m_sClientRef);
                query2.bindValue(1,m_sVersionOld);
                if (!query2.exec()) {
                    QMessageBox::about(NULL,"ERROR",query2.lastError().text());
                } else {
                    //QMessageBox::about(NULL,"Info","Please contact our customer service : ");
                    m_pLabel->setText("Please contact our customer service : ");
                    flag = false;
                }
            }

            //If the customer exists in the database but is not actived
            if (strFlagActive == "NO") {
                m_pLabel->setText("Please contact our customer service : ");
                flag = false;
            }

            db.close();

        } else {
            QMessageBox::about(NULL,"Info","Database file cannot be opened!");
        }
    } else {
        QMessageBox::about(NULL,"Info","Database file does not exist!");
    }
    return flag;
}


//Check the new version exists
bool AutoUpdater::checkNewVersonExists()
{
    bool flag = false;
    m_iIdVersionNew = m_iIdVersionOld = -1;
    m_sPackageNew = "";
    m_sFileDownloadMD5 = "";
    QString databaseFilePath = "/home/echo/Database/autoUpdate.fdb";
    QFile dbFile(databaseFilePath);
    if (dbFile.exists()) {
        QSqlDatabase db = QSqlDatabase::database();
        db.setDatabaseName(dbFile.fileName());
        db.setUserName("SYSDBA");
        db.setPassword("masterkey");
        if (db.open()) {

            QString sql = "SELECT F_VERSION,F_NUMBER_MD5 FROM T_CUSTOMER "
                          "INNER JOIN T_PACKAGE "
                          "ON T_CUSTOMER.F_ID_PACKAGE = T_PACKAGE.F_ID "
                          "WHERE T_CUSTOMER.F_SYS_CLIENT_REF = '";
            sql = sql + m_sClientRef +"'" ;
            QSqlQuery query1(sql,db);
            while (query1.next()) {
                m_sPackageNew = query1.value(0).toString();
                m_sFileDownloadMD5 = query1.value(1).toString();
                qDebug() << m_sPackageNew;
            }

            //When the name of the package is not empty and is not different from the old one
            if (m_sPackageNew != m_sVersionOld && !m_sPackageNew.isEmpty())
                flag = true;
        } else {
            QMessageBox::about(NULL,"ERROR","Database file cannot be opened!");
        }
    } else {
        QMessageBox::about(NULL,"Info","Database file does not exist!");
    }
    return flag;
}

//Create the files of update and backup
bool AutoUpdater::initDir()
{
    bool flag = false;
    m_sVentapHomePath = "/home/echo/ventap";
    m_sVentapUPDPath = m_sVentapHomePath + QDir::separator() + ".update";
    m_sVentapBAKPath = m_sVentapHomePath + QDir::separator() + ".backup";

    m_sDate = QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm");
    m_sDateFilePath = m_sVentapBAKPath + QDir::separator() + m_sDate;

    QDir *temp = new QDir;
    // if the update file exits, delete it and create an new update file
    if (temp->exists(m_sVentapUPDPath)) {
        deleteDirectory(m_sVentapUPDPath);
    }
    temp->mkdir(m_sVentapUPDPath);

    //if the backup file doesn't exist, then create an new backup file
    // Do not delete backup file
    if (!temp->exists(m_sVentapBAKPath)) {
        temp->mkdir(m_sVentapBAKPath);
    }

    //create date file
    temp->mkdir(m_sDateFilePath);
    flag = temp->exists(m_sVentapUPDPath) && temp->exists(m_sVentapBAKPath) && temp->exists(m_sDateFilePath);
    if (!flag) {
        QMessageBox::about(NULL,"Info","Program initialization failure!");
    }

    return flag;
}

//Download the update file from FTP server
bool AutoUpdater::downloadPackage()
{
    //Set the ftp parameter
    m_sFTPLink = "ftp://192.168.0.84/data/";
    m_sFTPFileName = m_sPackageNew;
    m_sFTPUser = "ftpuser";
    m_sFTPPSW = "echo";
    m_iFTPPort = 21;

    m_pFile = new QFile(m_sVentapUPDPath + QDir::separator() + m_sFTPFileName);
    m_pFile->open(QIODevice::WriteOnly);

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    manager->setNetworkAccessible(QNetworkAccessManager::Accessible);
    QUrl url(m_sFTPLink + m_sFTPFileName);
    url.setPort(m_iFTPPort);
    url.setUserName(m_sFTPUser);
    url.setPassword(m_sFTPPSW);

    QNetworkRequest request(url);
    m_pReply = manager->get(request);

    connect(m_pReply,SIGNAL(readyRead()),this,SLOT(writeNetworkReplySlot()));
    connect(manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(replyFinishedSlot(QNetworkReply*)));
    connect(m_pReply,SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(loadErrorSlot(QNetworkReply::NetworkError)));

}

//SLOT to read the reply
void AutoUpdater::writeNetworkReplySlot()
{
    m_pFile->write(m_pReply->readAll());
}

//SLOT to close the update file
void AutoUpdater::replyFinishedSlot(QNetworkReply *)
{
    if (m_pReply->error() == QNetworkReply::NoError) {
        m_pReply->deleteLater();
        m_pFile->flush();
        m_pFile->close();

        m_pFile->open(QIODevice::ReadOnly);
        QByteArray ba = QCryptographicHash::hash(m_pFile->readAll(),QCryptographicHash::Md5);
        QString md5DownloadFile = ba.toHex().constData();
        m_pFile->close();

        qDebug() << md5DownloadFile;
        qDebug() << m_sFileDownloadMD5;

        if (md5DownloadFile == m_sFileDownloadMD5) {
            m_pLabel->setText("Download the file successfully");
        } else {
            m_pLabel->setText("Filed to download the file!");
        }

    } else {
        QMessageBox::about(NULL,"Info","Network problems!");
    }

}

//SLOT to input error informations
void AutoUpdater::loadErrorSlot(QNetworkReply::NetworkError)
{
    QMessageBox::about(NULL,"Info",m_pReply->errorString());
}

//When the client clicked the button update
void AutoUpdater::updatePushButtonClickedSlot()
{
    this->downloadPackage();
}

//When the client clicked the button update
void AutoUpdater::exitPushButtonClickedSlot()
{
    this->close();
}

//Delete the directory and the files
bool AutoUpdater::deleteDirectory(QString path)
{
    if (path.isEmpty())
        return false;
    QDir dir(path);
    if (!dir.exists())
        return true;

    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    QFileInfoList fileList = dir.entryInfoList();
    foreach (QFileInfo fi, fileList) {
        if (fi.isFile()) {
            fi.dir().remove(fi.fileName());
        } else {
            deleteDirectory(fi.absoluteFilePath());
        }
    }
    return dir.rmpath(dir.absolutePath());
}





















