#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QDateTime>
#include <QTimer>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <cmath>
#include <QPainter>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintPreviewDialog>
#include "storage.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    double amount;
    QString time_in, selisih_waktu, time_out, j_kendaraan, j_parkir;


    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void showFullscreenWindows();
    void setListPayment();
    void setIcon();
    QString timeDiferent(const QString& time_in, QString &time_out, double *tarif);
    QString generateInvoiceNumber();
    void ParkingSummary(const QString time_in, const QString time_out, const QString duration, const QString plat, const QString j_kendaraan, const QString j_parkir, const QString Amount);
    void hideLabelSummary();
    void showLabelSummary();
    void resetPage();
    void creatstruck(QPrinter *printer, const QString plat, const QString time_in_, const QString time_out_, const QString duration, const QString j_kendaraan_, const QString j_parkir_, const QString amount_, const QString payment_);

private:
    bool kalkulasiTarif = false;
private slots:
    void setDateTime();
    void afterWindowShown();

protected:
    void keyPressEvent(QKeyEvent *event);
    void showEvent(QShowEvent *ev);

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    QTimer *timer_route;
    storage *database;
};
#endif // MAINWINDOW_H
