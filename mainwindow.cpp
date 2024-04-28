#include "mainwindow.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QLocale>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showEvent(QShowEvent *ev) {
    QWidget::showEvent(ev);
    QMetaObject::invokeMethod(this, "afterWindowShown", Qt::ConnectionType::DirectConnection);
}

void MainWindow::afterWindowShown(){
    if((database = new storage()) != nullptr){
        if(database->initialize() == 0){
            // QMessageBox::information(this, "Initialize database", "Berhasil init database");
        }else{
            QMessageBox::critical(nullptr, "Error", "Database tidak ditemukan atau gagal diinisialisasi");
            QCoreApplication::exit(-1);
            return;
        }
    }
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(setDateTime()));
    timer->start(1000);
    showFullscreenWindows();
    setListPayment();
    setIcon();
}
void MainWindow::keyPressEvent(QKeyEvent *event)  {
    int key  = event->key();
    QString Textkey = QKeySequence(key).toString();

    if(key == Qt::Key_F7){
        QString noKendaraan = ui->plat_input->text();
        QString ticket = ui->ticket_input->text();

        noKendaraan = noKendaraan.toUpper();
        noKendaraan = noKendaraan.remove("");

        if(!noKendaraan.isEmpty() && !ticket.isEmpty()){
            ticket = ticket.remove("");
            ticket = "%" + ticket;
            int rv = database->getTimein(noKendaraan, ticket, time_in, j_kendaraan, j_parkir);
            if(rv == 0 ){
                selisih_waktu = timeDiferent(time_in, time_out, &amount);
                ui->label_plat->setText(noKendaraan);
                ui->label_duration->setText(selisih_waktu);
                QLocale indonesianLocale(QLocale::Indonesian, QLocale::Indonesia);
                QString formattedAmount = indonesianLocale.toCurrencyString(amount);
                ParkingSummary(time_in, time_out, selisih_waktu, noKendaraan, j_kendaraan, j_parkir, formattedAmount);
                kalkulasiTarif = true;
            }else if(rv == 1){
                 QMessageBox::warning(this, "Ticket Not Found", "Data ticket tidak ditemukan.");
            }else{
                 QMessageBox::critical(this, "Error", "Terjadi kesalahan saat mencari data.");
            }
        }else{
             QMessageBox::critical(this, "Error", "Data Plat dan Ticket Belum Diisi");
        }
    }

    if(key == Qt::Key_F1){
        if(!kalkulasiTarif){
            QMessageBox::warning(this, "Hitung Tarif", "Lakukan Kalkulasi Tarif Terlebih Dahulu");
        }else{
            QString noKendaraan = ui->plat_input->text();
            QString ticket = ui->ticket_input->text();
            QString paymentMethode = ui->payment_methode->currentText();
            QString Invoice = generateInvoiceNumber();
            noKendaraan = noKendaraan.toUpper();
            noKendaraan = noKendaraan.remove("");
            ticket = ticket.remove("");
            ticket = "%" + ticket;
            int rv = database->updateTransaction(noKendaraan, ticket, time_out, paymentMethode, Invoice, amount);
            if(rv == 0){
                QPrinter printer;
                QPrintPreviewDialog previewDialog(&printer, this);
//                QMessageBox::information(this, "Transaksi Berhasil", "Transaksi Berhasil");
                QLocale indonesianLocale(QLocale::Indonesian, QLocale::Indonesia);
                QString formattedAmount = indonesianLocale.toCurrencyString(amount);
                connect(&previewDialog, &QPrintPreviewDialog::paintRequested, this, [&](){
                    creatstruck(&printer,noKendaraan, time_in, time_out, selisih_waktu, j_kendaraan, j_parkir, formattedAmount, paymentMethode);
                });
                previewDialog.exec();
                resetPage();
            }else{
                QMessageBox::critical(this, "Error", "Transaksi Gagal");
            }
            kalkulasiTarif = false;
        }
    }

    if (event->key() == Qt::Key_Escape && event->modifiers() & Qt::AltModifier ) {
        QApplication::quit();
    }
    if(key == Qt::Key_F2){
        ui->plat_input->setFocus();
    }
    if(key == Qt::Key_F3){
        ui->ticket_input->setFocus();
    }
    if(key == Qt::Key_F4){
        ui->payment_methode->showPopup();
    }
    if(key == Qt::Key_F5){
       resetPage();
    }
}

void MainWindow::setDateTime(){
    ui->label_datetime->setText(QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss"));
}

void MainWindow::showFullscreenWindows(){

    ui->ticket_input->setPlaceholderText("Masukkan 3 Digit Terakhir Ticket");
    ui->plat_input->setPlaceholderText("Masukkan No Kendaraan");
    hideLabelSummary();
    setWindowFlag(Qt::FramelessWindowHint);
    showFullScreen();
    setCursor(Qt::BlankCursor);
}

void MainWindow::setListPayment(){
    QFile file(":/data/config/list_payment");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(nullptr, "Error", "Tidak bisa membuka file /data/config/list_kendaraan");
        return;
    }

    QTextStream in(&file);
    ui->payment_methode->clear();
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (!line.isEmpty()) {
            ui->payment_methode->addItem(line);
        }
    }
    file.close();
}

void MainWindow::setIcon(){
    QPixmap logoParkee(":/data/img/logo.png");
    int targetWidth = 680;
    int targetHeight = 100;

    QPixmap scaledlogoparkee = logoParkee.scaled(targetWidth,targetHeight);
    ui->label_logo->setPixmap(scaledlogoparkee);
}

QString MainWindow::timeDiferent(const QString &time_in, QString& time_out, double* tarif){
    QString timeDiferent_str;
    QDateTime time_in_ = QDateTime::fromString(time_in, "yyyy-MM-dd HH:mm:ss");
    QDateTime now = QDateTime::currentDateTime();
    time_out = now.toString("yyyy-MM-dd HH:mm:ss");

    qint64 total_seccond = time_in_.secsTo(now);
    qint64 days = std::floor(total_seccond / (24 * 3600));
    qint64 remaining_seconds = total_seccond % (24 * 3600);

    qint64 hours = std::floor(remaining_seconds / 3600);
    remaining_seconds %= 3600;

    qint64 minutes = std::floor(remaining_seconds / 60);

    qint64 total_hours = (days * 24) + hours;

    double tarif_per_jam = 3000.0;
    double total_cost = (total_hours * tarif_per_jam) + ((minutes/60.0) * tarif_per_jam);
    *tarif = total_cost;

    return timeDiferent_str = QString("%1 Hari %2 Jam %3 Menit").arg(days).arg(hours).arg(minutes);
}

QString MainWindow::generateInvoiceNumber(){
    QString Prefix = "PARKEE";
    QDateTime now = QDateTime::currentDateTime();
    QString datetime_str = now.toString("yyyyMMdd_HHmmss");
    QString invoice_number = Prefix + "_" + datetime_str;
    return invoice_number;
}

void MainWindow::ParkingSummary(const QString time_in, const QString time_out, const QString duration, const QString plat, const QString j_kendaraan, const QString j_parkir, const QString Amount){
    ui->label_in->setText(": " + time_in);
    ui->label_out->setText(": " + time_out);
    ui->label_durasi->setText(": " + duration);
    ui->label_plat_2->setText(": " + plat);
    ui->label_jenis->setText(": " + j_parkir);
    ui->label_kendaraan->setText(": " + j_kendaraan);
    ui->label_harga->setText(": " + Amount);
    showLabelSummary();
}

void MainWindow::hideLabelSummary(){
    ui->label_in->hide();
    ui->label_in_->hide();
    ui->label_out->hide();
    ui->label_out_->hide();
    ui->label_durasi->hide();
    ui->label_durasi_->hide();
    ui->label_plat_2->hide();
    ui->label_plat_k->hide();
    ui->label_jenis->hide();
    ui->label_jenis_->hide();
    ui->label_kendaraan->hide();
    ui->label_kendaraan_2->hide();
    ui->label_harga->hide();
    ui->label_harga_->hide();
    ui->label_summary->hide();
}

void MainWindow::showLabelSummary(){
    ui->label_in->show();
    ui->label_in_->show();
    ui->label_out->show();
    ui->label_out_->show();
    ui->label_durasi->show();
    ui->label_durasi_->show();
    ui->label_plat_2->show();
    ui->label_plat_k->show();
    ui->label_jenis->show();
    ui->label_jenis_->show();
    ui->label_kendaraan->show();
    ui->label_kendaraan_2->show();
    ui->label_harga->show();
    ui->label_harga_->show();
    ui->label_summary->show();
}

void MainWindow::resetPage(){
    ui->label_plat->clear();
    ui->plat_input->clear();
    ui->ticket_input->clear();
    ui->label_duration->clear();
    hideLabelSummary();
}

void MainWindow::creatstruck(QPrinter *printer, const QString plat, const QString time_in_, const QString time_out_, const QString duration, const QString j_kendaraan_, const QString j_parkir_, const QString amount_, const QString payment_){
    QPainter painter(printer);
    QFont font("Courier", 10);
    painter.setFont(font);

    int y = 100;
    int x = 0;
    painter.drawText(x,y, "PK 1 - PARKEE");
    y+=50;
    painter.drawText(0, y, QString(100, '-'));
     y+=50;
    painter.drawText(x,y, "Waktu Masuk      :" + time_in_);
     y+=50;
    painter.drawText(x,y, "Waktu Keluar     :" + time_out_);
     y+=50;
    painter.drawText(x,y, "Durasi Parkir    :" + duration);
     y+=50;
    painter.drawText(x,y, "No Kendaraan     :" + plat);
     y+=50;
    painter.drawText(x,y, "Jenis Kendaraan  :" + j_kendaraan_);
     y+=50;
    painter.drawText(x,y, "Jenis Parkir     :" + j_parkir_);
     y+=50;
    painter.drawText(x,y, "Tarif            :" + amount_);
     y+=50;
    painter.drawText(x,y, "Metode Pembayaran:" + payment_);
     y+=50;
    painter.drawText(0, y, QString(100, '-'));
     y+=50;
    painter.drawText(0,y, "Terimaksih Selamat Jalan");
}
