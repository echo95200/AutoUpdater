#include "autoupdater.h"

AutoUpdater::AutoUpdater(QWidget *parent) : QWidget(parent)
{

    QLabel *pLabel = new QLabel(this);
    pLabel->setText("AutoUpdater");

    m_pExitPushPutton = new QPushButton(this);
    m_pUpdatePushButton = new QPushButton(this);
    m_pExitPushPutton->setText("Exit");
    m_pUpdatePushButton->setText("Update");
    QGridLayout *pLayout = new QGridLayout(this);
    pLayout->addWidget(pLabel,0,0);
    pLayout->addWidget(m_pUpdatePushButton,1,0);
    pLayout->addWidget(m_pExitPushPutton,1,1);

    this->setLayout(pLayout);

    this->getInfoFromDatabaseLocal();
    if (this->checkNewVersonExists()) {
        pLabel->setText("You have a new version to update!");
    } else {
        pLabel->setText("You don't have new version to update!");
        m_pUpdatePushButton->setEnabled(false);
    }


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
    } else {
        QMessageBox::about(NULL,"Info","Database file does not exist!");
    }
    return flag;
}

//Check the new version exists
bool AutoUpdater::checkNewVersonExists()
{
    bool flag = false;
    idVersionNew = idVersionOld = -1;
    QString databaseFilePath = "/home/echo/Database/autoUpdate.fdb";
    QFile dbFile(databaseFilePath);
    if (dbFile.exists()) {
        QSqlDatabase db = QSqlDatabase::database();
        db.setDatabaseName(dbFile.fileName());
        db.setUserName("SYSDBA");
        db.setPassword("masterkey");
        if (db.open()) {
            QString sql = "SELECT F_ID_PACKAGE FROM T_AUTHORIZATION "
                          "INNER JOIN T_CUSTOMER "
                          "ON T_AUTHORIZATION.F_ID_CUSTOMER = T_CUSTOMER.F_ID "
                          "WHERE T_CUSTOMER.F_SYS_CLIENT_REF = '";
            sql = sql + m_sClientRef +"'" ;
            QSqlQuery query1(sql,db);
            while (query1.next()) {
                idVersionNew = query1.value(0).toInt();
                qDebug() << idVersionNew;
            }

            sql = "SELECT F_ID FROM T_PACKAGE WHERE F_VERSION = '";
            sql = sql + m_sVersionOld + "'";
            QSqlQuery query2(sql,db);
            while (query2.next()) {
                idVersionOld = query2.value(0).toInt();
                qDebug() << idVersionOld;
            }
            if (idVersionNew > idVersionOld)
                flag = true;
        } else {
            QMessageBox::about(NULL,"ERROR","Database file cannot be opened!");
        }
    } else {
        QMessageBox::about(NULL,"Info","Database file does not exist!");
    }
    return flag;
}
























