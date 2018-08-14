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

class AutoUpdater : public QWidget
{
    Q_OBJECT
public:
    explicit AutoUpdater(QWidget *parent = nullptr);

    //Get the ref and version of the customer
    bool getInfoFromDatabaseLocal();

    //Check the new version exists
    bool checkNewVersonExists();

signals:

public slots:

private:
    QString m_sClientRef;
    QString m_sVersionOld;
    int idVersionOld;
    int idVersionNew;

    QPushButton *m_pUpdatePushButton;
    QPushButton *m_pExitPushPutton;
};

#endif // AUTOUPDATER_H
