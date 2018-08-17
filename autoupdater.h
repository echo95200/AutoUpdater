#ifndef AUTOUPDATER_H
#define AUTOUPDATER_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>
#include <QPushButton>
#include <QGridLayout>
#include <QSqlError>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDir>
#include <QDateTime>
#include <QFileInfo>
#include <QFileInfoList>
#include <QByteArray>
#include <QCryptographicHash>

class AutoUpdater : public QWidget
{
    Q_OBJECT
public:
    explicit AutoUpdater(QWidget *parent = nullptr);

    //Get the ref and version of the customer
    bool getInfoFromDatabaseLocal();

    //Check the new version exists
    bool checkNewVersonExists();

    //Connect the serveur database
    bool initConnectDatabase();
    bool connectDatabase();

    //Create the files of update and backup
    bool initDir();
    //Download the update file from FTP server
    bool downloadPackage();

    //Delete the directory and the files
    bool deleteDirectory(QString);

signals:

public slots:

    //The slot for the FTP
    void writeNetworkReplySlot();
    void replyFinishedSlot(QNetworkReply*);
    void loadErrorSlot(QNetworkReply::NetworkError);

    void updatePushButtonClickedSlot();
    void exitPushButtonClickedSlot();

private:
    QString m_sClientRef;
    QString m_sVersionOld;
    int m_iIdClient;
    int m_iIdVersionOld;
    int m_iIdVersionNew;

    QLabel* m_pLabel;
    QPushButton *m_pUpdatePushButton;
    QPushButton *m_pExitPushPutton;

    QString m_sPackageNew;
    QFile *m_pFile;
    QString m_sFileDownloadMD5;

    //FTP server
    QString m_sFTPLink;
    QString m_sFTPUser;
    QString m_sFTPPSW;
    QString m_sFTPFileName;
    int m_iFTPPort;
    QNetworkReply *m_pReply;

    //The directory of the ventap
    QString m_sVentapHomePath;
    QString m_sVentapUPDPath;
    QString m_sVentapBAKPath;

    QString m_sDate;
    QString m_sDateFilePath;
};

#endif // AUTOUPDATER_H
