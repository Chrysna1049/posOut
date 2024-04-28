#ifndef STORAGE_H
#define STORAGE_H
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>


class storage
{
public:
    storage();
    storage(const storage& orig);
    ~storage();

    int  initialize();
    int getTimein(const QString &plat_kendaraan, const QString &ticket, QString &time_in, QString &j_kendaraan, QString &j_parkir);
    int updateTransaction(const QString &plat_kendaraan, const QString &ticket, const QString time_out, const QString payment_methode, const QString invoice, const int amout);
private:
    bool isInitialize = false;
};

#endif // STORAGE_H
