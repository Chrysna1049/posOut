#include "storage.h"
#include <QCoreApplication>
#include <QDebug>
#include <QSettings>
#include <QMessageBox>
#include <QFile>

storage::storage()
{

}

storage::storage(const storage& orig){

}
storage::~storage() {
}

bool checkConfigFileExists(const QString& configFilePath) {
    QFile file(configFilePath);
    return file.exists();
}

int storage::initialize(){
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    QString appDirPath = QCoreApplication::applicationDirPath();
    QString fileConfig = appDirPath +"/config.ini";
    if (!checkConfigFileExists(fileConfig)) {
        qDebug() << "Config file not found at" << fileConfig;
        QMessageBox::critical(nullptr, "Error", "File konfigurasi tidak ditemukan." + fileConfig);

        return -1;
    }
    QSettings settings(fileConfig, QSettings::IniFormat);

    QString hostName = settings.value("Database/HostName").toString();
    QString databaseName = settings.value("Database/DatabaseName").toString();
    QString userName = settings.value("Database/UserName").toString();
    QString password = settings.value("Database/Password").toString();

    db.setHostName(hostName);
    db.setDatabaseName(databaseName);
    db.setUserName(userName);
    db.setPassword(password);

    if (!db.open()) {
         qDebug() << "Error: Could not open database:" << db.lastError().text();
        isInitialize = false;
        return -1;
    }
    isInitialize = true;
    return 0;
}

int storage::getTimein(const QString& plat_kendaraan, const QString& ticket, QString& time_in, QString& j_kendaraan, QString& j_parkir){
    if(isInitialize){
        QSqlQuery query;
//        query.prepare("SELECT data_datetime_in FROM transaction WHERE data_plat_kendaraan = :plat_kendaraan");
        query.prepare("SELECT data_datetime_in, data_jenis_kendaraan, data_jenis_parkir FROM transaction WHERE data_plat_kendaraan = :plat_kendaraan AND data_ticket LIKE :ticket AND data_datetime_out = '' ORDER BY data_datetime_in DESC LIMIT 1");
//        query.prepare("SELECT data_datetime_in FROM transaction WHERE data_plat_kendaraan = :plat_kendaraan AND RIGHT(data_ticket, 3) = :ticket AND (data_datetime_out IS NULL) ORDER BY data_datetime_in DESC LIMIT 1");
        query.bindValue(":plat_kendaraan", plat_kendaraan);
        query.bindValue(":ticket", ticket);
        if(query.exec()){
            if (query.next()) {
                 time_in = query.value(0).toString();
                 j_kendaraan = query.value(1).toString();
                 j_parkir = query.value(2).toString();
                 if (time_in.isEmpty()) {
                     return -1;
                 }
                 return 0;
             } else {
                 return 1;
             }
        }
    }
}

int storage::updateTransaction(const QString &plat_kendaraan, const QString &ticket, const QString time_out, const QString payment_methode, const QString invoice, const int amout){
    if(isInitialize){
        QSqlQuery query;
        query.prepare("UPDATE transaction SET data_datetime_out = :time_out, data_payment_methode = :payment_methode, data_payment_invoice = :invoice, data_amount = :amount WHERE data_plat_kendaraan = :plat AND data_ticket LIKE :ticket");
        query.bindValue(":time_out", time_out);
        query.bindValue(":payment_methode", payment_methode);
        query.bindValue(":invoice", invoice);
        query.bindValue(":amount", amout);
        query.bindValue(":plat", plat_kendaraan);
        query.bindValue(":ticket", ticket);
        if (!query.exec()) {
             qDebug() << "Error updating transaction:" << query.lastError().text();
             return -1;
         }
        return 0;
    }
}
