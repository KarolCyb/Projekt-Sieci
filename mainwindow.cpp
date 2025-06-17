#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "oknoobiektarx.h"
#include <QDebug>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>

std::random_device srng;
std::mt19937 rng;

MainWindow::MainWindow(QWidget *parent, WarstwaUslug *prog)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , simulationTimer(new QTimer(this))
    , usluga(prog)
{
    usluga->setGUI(this);
    ui->setupUi(this);
    UstawienieWygladuGUI();
    UstawienieLayout();
    UstawienieOkienOrazSygnalowIslotow();
    ui->btnRozlacz->setEnabled(0);
    TCPpolaczenie = nullptr;
    TCPserver = nullptr;
    ui->lblPolaczenie->setVisible(0);
    ui->btnRozlacz->setVisible(0);
    ui->btnSendSignal->setVisible(0);
    ui->chkServer->setVisible(0);
    ui->letIP->setVisible(0);
    ui->sbxPort->setVisible(0);
    ui->chkObustronneTaktowanie->setVisible(0);
    ui->lblPort->setVisible(0);
    ui->lblAdresIp->setVisible(0);
    /*connect(simulationTimer, &QTimer::timeout, this, [=]() {
        wykres->WykresWartosciZadanej();
    });
    connect(simulationTimer, &QTimer::timeout, this, [=]() {
        wykres->WykresUchybu();
    });
    connect(simulationTimer, &QTimer::timeout, this, [=]() {
        wykres->WykresPID();
    });
    connect(simulationTimer, &QTimer::timeout, this, [=]() {
        wykres->WykresWartosciSterowania();
    });*/
    buferOneClock.resize(100);
    connect(simulationTimer, &QTimer::timeout, this, &MainWindow::dane_i_wykresy);
}
void MainWindow::zapiszPakietRegulator(QByteArray &dane){
    QDataStream out(&dane, QIODevice::WriteOnly);
    out<<(double)usluga->getSymulator()->getWartoscZadana();
    out<<(double)usluga->getSymulator()->getLastRegulatorValue();
    out<<(int)packet_number;
    out<<(int)interwalCzasowy;
    out<<(bool)run_str;

}
void MainWindow::zapiszPakietArx(QByteArray &dane){
    QDataStream out(&dane, QIODevice::WriteOnly);
    out<<(double)usluga->getSymulator()->getWartoscZadana();
    out<<(double)wykres->getSymulator()->getLastObjectOutput();
    out<<(int)packet_number;
    out<<(int)interwalCzasowy;
    out<<(bool)run_str;
}
void MainWindow::wczytajPakietOneClockRegulator(QByteArray &dane){
    double val_zad;
    double val_wyj;
    int time;
    int interwal;
    bool run_str_n;
    dane = TCPpolaczenie->readAll();
    QDataStream in(&dane, QIODevice::ReadOnly);
    in>>val_zad>>val_wyj>>time>>interwal>>run_str_n;
    end_m = std::chrono::high_resolution_clock::now();
    wykres->getSymulator()->setWyjscieObiektu(val_wyj);
    wykres->getSymulator()->setLastObjectOutput(val_wyj);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_m - start_m);
    wykres->getSymulator()->setFlag(true);
    ui->label_color->setStyleSheet("QLabel{background-color : green;}");
   /* wykres->WykresWartosciZadanej();
    wykres->AktualizujWykresy();
*/

    ui->ms_label->setText(QString::number(duration.count())+" ms");
}
void MainWindow::wczytajPakietOneClockArx(QByteArray &dane){

    double val_zad;
    double val;
    int time;
    int interwal;
    bool run_str_n;
    dane = TCPpolaczenie->readAll();
    QDataStream in(&dane, QIODevice::ReadOnly);
    in>>val_zad>>val>>time>>interwal>>run_str_n;
    wykres->getSymulator()->setLastRegulatorValue(val);
    wykres->getSymulator()->setLastGeneratorValue(val_zad);
    double wyjscie = wykres->getSymulator()->symuluj_wyjscie();
    wykres->widzialnoscWykresow(false);
    wykres->WykresWartosciZadanej_no_base();
    wykres->WykresWartosciSterowania();
    wykres->WykresUchybu();
    wykres->WykresPID();
    wykres->krok();
    wykres->AktualizujWykresy();
   // qDebug()<<packet_number<<" "<<time;
}
void MainWindow::wczytajPakietTwoClockRegulator(QByteArray &dane){
    double val_zad;
    double val_wyj;
    int time;
    int interwal;
    bool run_str;
    dane = TCPpolaczenie->readAll();
    QDataStream in(&dane, QIODevice::ReadOnly);
    in>>val_zad>>val_wyj>>time>>interwal>>run_str;

    end_m = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_m - start_m);
    wykres->getSymulator()->setFlag(true);
    ui->label_color->setStyleSheet("QLabel{background-color : green;}");
    wykres->getSymulator()->setWyjscieObiektu(val_wyj);
    wykres->getSymulator()->setLastObjectOutput(val_wyj);
    ui->ms_label->setText(QString::number(duration.count())+" ms");
}
void MainWindow::wczytajPakietTwoClockObiekt(QByteArray &dane){
     double val_zad;
    double val;
    int time;
    int interwal;
    bool run_str_n;
    dane = TCPpolaczenie->readAll();
    QDataStream in(&dane, QIODevice::ReadOnly);
    in>>val_zad>>val>>time>>interwal>>run_str_n;
    simulationTimer->setInterval(interwal);
    if(run_str_n && !simulationTimer->isActive()) simulationTimer->start();
    if(!run_str_n && simulationTimer->isActive()) simulationTimer->stop();
    wykres->getSymulator()->setLastRegulatorValue(val);
    wykres->getSymulator()->setLastGeneratorValue(val_zad);
}
void MainWindow::dane_i_wykresy()
{
    /*
    if(!blokada){
        if(TCPpolaczenie == nullptr)
        {
            wykres->getSymulator()->symulujKrok(wykres->getCzas());
            wykres->WykresWartosciZadanej();
        }
        if(TCPpolaczenie != nullptr && !ui->chkServer->isChecked()  && !ui->chkObustronneTaktowanie->isChecked())
        {
            if(wykres->getSymulator()->getFlag())
            {
                wykres->getSymulator()->symuluj_bez_wyjscia(wykres->getCzas());
                start_m = std::chrono::high_resolution_clock::now();
                wykres->getSymulator()->setFlag(false);
            }
            //przyklad
            QByteArray dane_siec;
            QDataStream out(&dane_siec, QIODevice::WriteOnly);
            out<<(double)usluga->getSymulator()->getLastRegulatorValue();
            TCPpolaczenie->write(dane_siec);
            TCPpolaczenie->flush();
            ui->label_color->setStyleSheet("QLabel{background-color : red;}");
        }
        if(TCPpolaczenie != nullptr && !ui->chkServer->isChecked() && ui->chkObustronneTaktowanie->isChecked())
        {

            if(wykres->getSymulator()->getFlag())
            {
                wykres->getSymulator()->symuluj_bez_wyjscia(wykres->getCzas());
                start_m = std::chrono::high_resolution_clock::now();
                wykres->getSymulator()->setFlag(false);
            }
            //przyklad

            QByteArray dane_siec;
            QDataStream out(&dane_siec, QIODevice::WriteOnly);
            out<<(double)usluga->getSymulator()->getLastRegulatorValue();
            out<<(int)packet_number;
            out<<(int)interwalCzasowy;
            out<<(bool)run_str;
            TCPpolaczenie->write(dane_siec);
            TCPpolaczenie->flush();
            packet_number++;
            wykres->WykresWartosciZadanej();
            ui->label_color->setStyleSheet("QLabel{background-color : red;}");

        }
        if(TCPpolaczenie != nullptr && ui->chkServer->isChecked() && ui->chkObustronneTaktowanie->isChecked())
        {

            wykres->getSymulator()->symuluj_w_tle(wykres->getCzas());
            //double wyjscie = wykres->getSymulator()->symuluj_wyjscie();

            QByteArray dane_siec;
            QDataStream out(&dane_siec, QIODevice::WriteOnly);
            out<<(double)wykres->getSymulator()->symuluj_wyjscie();
            out<<(int)packet_number;
            out<<(int)interwalCzasowy;
            out<<(bool)run_str;
            TCPpolaczenie->write(dane_siec);
            TCPpolaczenie->flush();
            packet_number++;
            wykres->WykresWartosciZadanej();
        }
        wykres->WykresUchybu();
        wykres->WykresPID();
        wykres->WykresWartosciSterowania();
        wykres->krok();
        wykres->AktualizujWykresy();
    }
    if(blokada && !ui->chkServer->isChecked())
    {
        wykres->AktualizujWykresy();
        QByteArray dane_siec;
        QDataStream out(&dane_siec, QIODevice::WriteOnly);
        out<<(double)usluga->getSymulator()->getLastRegulatorValue();
        out<<(int)packet_number;
        out<<(int)interwalCzasowy;
        out<<(bool)run_str;
        if(TCPpolaczenie != nullptr){
        TCPpolaczenie->write(dane_siec);
        TCPpolaczenie->flush();
        }
    }
    if(blokada && ui->chkServer->isChecked())
    {
        wykres->getSymulator()->symuluj_w_tle(wykres->getCzas());
        //double wyjscie = wykres->getSymulator()->symuluj_wyjscie();
        QByteArray dane_siec;
        QDataStream out(&dane_siec, QIODevice::WriteOnly);
        out<<(double)wykres->getSymulator()->symuluj_wyjscie();
        out<<(int)packet_number;
        out<<(int)interwalCzasowy;
        out<<(bool)run_str;
        if(TCPpolaczenie != nullptr){
            TCPpolaczenie->write(dane_siec);
            TCPpolaczenie->flush();
        }
    }
    qDebug()<<packet_number<<" "<<wykres->getCzas()<< " "<< korekta;
    //sinus po sieci nie dziala jak powinien
*/
    if(TCPpolaczenie == nullptr)
    {
        wykres->widzialnoscWykresow(true);
        wykres->getSymulator()->symulujKrok(wykres->getCzas());
        //wykres->WykresWartosciZadanej();
        wykres->WykresWartosciZadanej();
    }
    if(TCPpolaczenie != nullptr && !ui->chkServer->isChecked() && !ui->chkObustronneTaktowanie->isChecked())
    {

        if(wykres->getSymulator()->getFlag())
        {
            start_m = std::chrono::high_resolution_clock::now();
            wykres->getSymulator()->setFlag(false);
        }
        wykres->getSymulator()->symuluj_bez_wyjscia(wykres->getCzas());
        QByteArray dane_siec;
        zapiszPakietRegulator(dane_siec);
        TCPpolaczenie->write(dane_siec);
        TCPpolaczenie->flush();
        packet_number++;
        buferOneClock.push_front(usluga->getSymulator()->getLastRegulatorValue());
        buferOneClock.pop_back();
        ui->label_color->setStyleSheet("QLabel{background-color : red;}");
        wykres->WykresWartosciZadanej();

    }
    //reg
    if(TCPpolaczenie != nullptr && !ui->chkServer->isChecked() && ui->chkObustronneTaktowanie->isChecked())
    {
        if(wykres->getSymulator()->getFlag())
        {
            start_m = std::chrono::high_resolution_clock::now();
            wykres->getSymulator()->setFlag(false);
        }
        wykres->getSymulator()->symuluj_bez_wyjscia(wykres->getCzas());
         wykres->widzialnoscWykresow(true);
        start_m = std::chrono::high_resolution_clock::now();
        QByteArray dane_siec;
        zapiszPakietRegulator(dane_siec);
        TCPpolaczenie->write(dane_siec);
        TCPpolaczenie->flush();
        packet_number++;
        ui->label_color->setStyleSheet("QLabel{background-color : red;}");
        wykres->WykresWartosciZadanej();
    }
    //obiekt
    if(TCPpolaczenie != nullptr && ui->chkServer->isChecked() && ui->chkObustronneTaktowanie->isChecked())
    {
        double wyjscie = wykres->getSymulator()->symuluj_wyjscie();
        wykres->getSymulator()->symuluj_w_tle(wykres->getCzas());
        wykres->widzialnoscWykresow(false);
        QByteArray dane_siec;
        zapiszPakietArx(dane_siec);
        TCPpolaczenie->write(dane_siec);
        TCPpolaczenie->flush();
        packet_number++;
        wykres->WykresWartosciZadanej_no_base();
    }

    //wykres->WykresWartosciZadanej();
    wykres->WykresUchybu();
    wykres->WykresPID();
    wykres->WykresWartosciSterowania();
    wykres->krok();
    wykres->AktualizujWykresy();
    //qDebug()<<buferOneClock.front();
}

void MainWindow::odczyt()
{
    QByteArray dane_siec;
    if(!ui->chkObustronneTaktowanie->isChecked())
    {
        wczytajPakietOneClockArx(dane_siec);
        dane_siec.clear();
        zapiszPakietArx(dane_siec);
        TCPpolaczenie->write(dane_siec);
        TCPpolaczenie->flush();
        packet_number++;
    }

    if(ui->chkObustronneTaktowanie->isChecked()){
        wczytajPakietTwoClockObiekt(dane_siec);
    }

    /*
    if(!ui->chkObustronneTaktowanie->isChecked())
    {
        QByteArray dane_siec;
        double val;
        dane_siec = TCPpolaczenie->readAll();
        QDataStream in(&dane_siec, QIODevice::ReadOnly);
        in >> val;
        //dane_siec = TCPpolaczenie->read(8);
        //int time = dane_siec.toDouble();
        //TCPpolaczenie->flush();
        wykres->getSymulator()->setLastRegulatorValue(val);
        double wyjscie = wykres->getSymulator()->symuluj_wyjscie();
        wykres->WykresWartosciZadanej_no_base();
        wykres->krok();
        wykres->AktualizujWykresy();
        //wykres->WykresWartosciZadanej();
        //wykres->AktualizujWykresy();
        //wykres->krok();
        QByteArray dane_siec_wyj;
        QDataStream out(&dane_siec_wyj, QIODevice::WriteOnly);
        out<<(double)wyjscie;
        TCPpolaczenie->write(dane_siec_wyj);
        TCPpolaczenie->flush();
     }
    else if(ui->chkObustronneTaktowanie->isChecked())
    {

        QByteArray dane_siec;
        double val;
        int time;
        int interwal;
        bool run_str;
        dane_siec = TCPpolaczenie->readAll();
        QDataStream in(&dane_siec, QIODevice::ReadOnly);
        in>>val>>time>>interwal>>run_str;
        if(time == packet_number)
        {
            wykres->getSymulator()->setLastRegulatorValue(val);
            blokada = false;
        }
        /*double wyjscie = wykres->getSymulator()->symuluj_wyjscie();
        wykres->WykresWartosciZadanej_no_base();
        wykres->krok();
        wykres->AktualizujWykresy();

        if(interwal != interwalCzasowy )
        {
            interwalCzasowy=interwal;
            korekta = 1;
        }
        /*if(korekta > interwalCzasowy*0.5) korekta = interwalCzasowy*0.5;
        if(korekta < interwalCzasowy*(-0.5)) korekta = interwalCzasowy*(-0.5);
        ;*/
    /*
        if(korekta < 0.5) korekta = 0.5;
        double war =  interwalCzasowy*korekta;
        simulationTimer->setInterval(war);
        if(time < packet_number)
        {
             blokada = true;
            korekta = korekta - 0.02;
        }
        if(time >= packet_number)
        {
             blokada = false;
            korekta = korekta + 0.01;
        }
        if(run_str){
            if(!simulationTimer->isActive()) simulationTimer->start();
            /*
            QByteArray dane_siec_wyj;
            QDataStream out(&dane_siec_wyj, QIODevice::WriteOnly);
            out<<(double)wyjscie;
            out<<(int)packet_number;
            out<<(int)interwalCzasowy;
            out<<(bool)run_str;
            TCPpolaczenie->write(dane_siec_wyj);
            TCPpolaczenie->flush();
            packet_number++;
            */
    /*
        }
        if(!run_str){
            simulationTimer->stop();
        }
    }*/
}
void MainWindow::odczyt_klient()
{
    QByteArray dane_siec;
    if(!ui->chkObustronneTaktowanie->isChecked())
    {
        wczytajPakietOneClockRegulator(dane_siec);
    }
    if(ui->chkObustronneTaktowanie->isChecked()){
        wczytajPakietTwoClockRegulator(dane_siec);
    }
/*
    if(!ui->chkObustronneTaktowanie->isChecked())
    {
        QByteArray dane_siec;
        double val_wyj;
        dane_siec = TCPpolaczenie->readAll();
        QDataStream in(&dane_siec, QIODevice::ReadOnly);
        in>> val_wyj;
        end_m = std::chrono::high_resolution_clock::now();
        wykres->getSymulator()->setWyjscieObiektu(val_wyj);
        wykres->getSymulator()->setLastObjectOutput(val_wyj);
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_m - start_m);
        wykres->getSymulator()->setFlag(true);
        ui->label_color->setStyleSheet("QLabel{background-color : green;}");
        wykres->WykresWartosciZadanej();
        wykres->AktualizujWykresy();


        ui->ms_label->setText(QString::number(duration.count())+" ms");
    }
    else if(ui->chkObustronneTaktowanie->isChecked())
    {

        QByteArray dane_siec;
        double val_wyj;
        int time;
        int inter;
        bool run_str;
        dane_siec = TCPpolaczenie->readAll();
        QDataStream in(&dane_siec, QIODevice::ReadOnly);
        in >> val_wyj>>time>>inter>>run_str;
        end_m = std::chrono::high_resolution_clock::now();
        //qDebug()<<wykres->getCzas()<<" "<<time;
        if(packet_number == time)
        {
        wykres->getSymulator()->setWyjscieObiektu(val_wyj);
        wykres->getSymulator()->setLastObjectOutput(val_wyj);
         blokada = false;
        }
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_m - start_m);
        wykres->getSymulator()->setFlag(true);
        ui->label_color->setStyleSheet("QLabel{background-color : green;}");
       /* wykres->WykresWartosciZadanej();
        //wykres->krok();
        wykres->AktualizujWykresy();
        */
/*
        ui->ms_label->setText(QString::number(duration.count())+" ms");
        if( time < packet_number)
        {
            blokada = true;

        }
        if(time >= packet_number)
        {
            blokada = false;

        }

    }
*/

}
MainWindow::~MainWindow()
{
    delete ui;
    delete reg;
    reg=nullptr;
    delete gen;
    gen=nullptr;
}

void MainWindow::on_Start_clicked()
{
    usluga->SprawdzenieWszystkichDanych(interwalCzasowy);
    run_str = true;
}

void MainWindow::on_Stop_clicked()
{
    run_str = false;
    simulationTimer->stop();
    if(TCPpolaczenie != nullptr && !ui->chkServer->isChecked() && ui->chkObustronneTaktowanie->isChecked())
    {
        QByteArray dane_siec;
        QDataStream out(&dane_siec, QIODevice::WriteOnly);
        out<<(double)usluga->getSymulator()->getLastRegulatorValue();
        out<<(int)packet_number;
        out<<(int)interwalCzasowy;
        out<<(bool)run_str;
        TCPpolaczenie->write(dane_siec);
        TCPpolaczenie->flush();
    }
}

void MainWindow::PokazWykres() {


    simulationTimer->start(interwalCzasowy);

}
void MainWindow::Blad(){
    QMessageBox::warning(this, "Ostrzeżenie", "Nie uzupełniłeś Wszystkich Danych");
}

void MainWindow::on_UstawieniaObiektuARX_clicked()
{
    okno_obiekt->exec();
}

void MainWindow::bladUstawien()
{
    QMessageBox::information(this, "Informacja", "Sprawdź czy poprawnie uzupełniłeś wszystkie dane");
}

void MainWindow::on_Reset_clicked() {
    disconnect(simulationTimer, nullptr, nullptr, nullptr);
    simulationTimer->stop();
    wykres->ResetujWykresy();
    wykres->ResetCzas();
    usluga->ResetSymulacji();
    wykres->InicjalizujWykresy(layout);
}

void MainWindow::obslugaZapisu()
{
    QMessageBox::information(this, "Zapis konfiguracji", "Konfiguracja została zapisana.");
}

void MainWindow::on_Zapisz_clicked()
{
    usluga->zapiszKonfiguracje();
    QMessageBox::information(this, "Sukces", "Konfiguracja została zapisana.");
}

void MainWindow::on_Wczytaj_clicked()
{
    usluga->wczytajKonfiguracje();
    QMessageBox::information(this, "Sukces", "Konfiguracja została wczytana.");
}

void MainWindow::UstawienieOkienOrazSygnalowIslotow(){
    wykres = new Wykresy(this,usluga->getSymulator());
    okno_obiekt = new OknoObiektARX(this);
    okno_obiekt->setWarstwaUslug(usluga);
    reg = new Regulator;
    gen = new Generator;
    ui->Sposob->addAction("Stała w Sumie");
    ui->Sposob->addAction("Stała przed Sumą");
    ui->RodzajSygnalu->addAction("Skokowy");
    ui->RodzajSygnalu->addAction("Prostokątny");
    ui->RodzajSygnalu->addAction("Sinusoidalny");
    //Generator
    ui->Amplituda->setRange(-100, 1000);
    ui->Okres->setRange(0, 1000);
    ui->Wypelnienie->setRange(0, 1);
    ui->Amplituda->setSingleStep(0.1);
    ui->Wypelnienie->setSingleStep(0.05);
    ui->CzasAktywacji->setRange(0,500);
    ui->CzasAktywacji->setSingleStep(0.5);
    ui->Amplituda->setValue(66);
    ui->Okres->setValue(100);
    ui->Wypelnienie->setValue(0.6);
    ui->RodzajSygnalu->setText("Prostokątny");

    //Regulator
    ui->Wzmocnienie->setRange(0, 1000);
    ui->Wzmocnienie->setValue(0.3);
    ui->StalaI->setRange(0, 1000);
    ui->StalaI->setValue(2.5);
    ui->StalaD->setRange(0, 1000);
    ui->StalaD->setValue(0.05);
    ui->Wzmocnienie->setSingleStep(0.1);
    ui->StalaI->setSingleStep(0.1);
    ui->StalaD->setSingleStep(0.1);
    ui->Sposob->setText("Stała w Sumie");


    this->showMaximized();
    simulationTimer = new QTimer(this);
    wykres->InicjalizujWykresy(layout);
    connect(usluga, &WarstwaUslug::PoprawneDane, this, &MainWindow::PokazWykres);
    connect(usluga, &WarstwaUslug::BledneDane, this, &MainWindow::Blad);
    connect(usluga, &WarstwaUslug::sygnalZapisano, this, &MainWindow::obslugaZapisu);
    Wczytaj = new QPushButton("Wczytaj konfigurację", this);
    connect(Wczytaj, &QPushButton::clicked, this, &MainWindow::on_Wczytaj_clicked);
    connect(usluga, &WarstwaUslug::blad, this, &MainWindow::bladUstawien);
    ui->Interwal->setText("100");
    interwalCzasowy=100;
}

void MainWindow::UstawienieLayout(){
    for(int i=0;i<4;i++){
        layout[i] = new QVBoxLayout(this);
        layout[i]->setContentsMargins(0, 0, 0, 0);
    }
    ui->wykresWarZad->setLayout(layout[0]);
    ui->WykUchyb->setLayout(layout[1]);
    ui->WykPID->setLayout(layout[2]);
    ui->WykSter->setLayout(layout[3]);
}


void MainWindow::UstawienieWygladuGUI(){
    QString buttonStyleStart =
        "QPushButton {"
        "    background-color: white;"
        "    color: black;"
        "    border-radius: 10px;"
        "    border: 2px solid black;"
        "    padding: 5px;"
        "    transition: 0.5s ease-in-out;"
        "}"
        "QPushButton:hover {"
        "    background-color: green;"
        "    color: white;"
        "    padding: 8px;"
        "    border: 3px solid #fff;"
        "}";
    QString buttonStyleStop =
        "QPushButton {"
        "    background-color: white;"
        "    color: black;"
        "    border-radius: 10px;"
        "    border: 2px solid black;"
        "    padding: 5px;"
        "    transition: 0.5s ease-in-out;"
        "}"
        "QPushButton:hover {"
        "    background-color: red;"
        "    color: white;"
        "    padding: 8px;"
        "    border: 3px solid #fff;"
        "}";
    QString buttonStyleWczytaj =
        "QPushButton {"
        "    background-color: white;"
        "    color: black;"
        "    border-radius: 10px;"
        "    border: 2px solid black;"
        "    padding: 5px;"
        "    transition: 0.5s ease-in-out;"
        "}"
        "QPushButton:hover {"
        "    background-color: deepskyblue;"
        "    color: white;"
        "    padding: 8px;"
        "    border: 3px solid #fff;"
        "}";
    QString buttonStyleReset =
        "QPushButton {"
        "    background-color: white;"
        "    color: black;"
        "    border-radius: 10px;"
        "    border: 2px solid black;"
        "    padding: 5px;"
        "    transition: 0.5s ease-in-out;"
        "}"
        "QPushButton:hover {"
        "    background-color: orange;"
        "    color: white;"
        "    padding: 8px;"
        "    border: 3px solid #fff;"
        "}";
    QString buttonStyleReszta =
        "QPushButton {"
        "    background-color: white;"
        "    color: black;"
        "    border-radius: 10px;"
        "    border: 2px solid black;"
        "    padding: 5px;"
        "    transition: 0.5s ease-in-out;"
        "}"
        "QPushButton:hover {"
        "    background-color: grey;"
        "    color: white;"
        "    padding: 8px;"
        "    border: 3px solid #fff;"
        "}";
    QString GeneratorIregulatorNapis =
        "QLabel {"
        "    color: white;"
        "    border: 2px solid black;"
        "    background-color: grey;"

         "}";


    this->setStyleSheet(
        "QWidget { color: white; }"
        "QMainWindow { background-color: rgb(90,90,90); }"
        );
    this->setStyleSheet(
        "QMessageBox { color: black; "
        "background-color: grey;"
        "}"
        "QMainWindow { background-color: rgb(90,90,90); }"
        );
    ui->Interwal->setStyleSheet(
        "QLineEdit {"
        "    background-color: white;"
        "    color: black;"
        "}"
        );
    ui->RegulatorNapis->setStyleSheet(GeneratorIregulatorNapis);
    ui->GeneratorNapis->setStyleSheet(GeneratorIregulatorNapis);
    ui->Start->setStyleSheet(buttonStyleStart);
    ui->Stop->setStyleSheet(buttonStyleStop);
    ui->Reset->setStyleSheet(buttonStyleReset);
    ui->Wczytaj->setStyleSheet(buttonStyleWczytaj);
    ui->UstawieniaObiektuARX->setStyleSheet(buttonStyleReszta);
    ui->Zapisz->setStyleSheet(buttonStyleStart);
    ui->btnSendSignal->setStyleSheet(buttonStyleReszta);
    ui->btnRozlacz->setStyleSheet(buttonStyleReszta);


    ui->TytulWykres1->setStyleSheet(
        "QLabel {"
        "    background-color: rgb(130, 130, 130);"
        "    color: white;"
        "    border: 1px solid black;"
        "    padding: 5px;"
        "}"
        );

    ui->TytulWykres2->setStyleSheet(
        "QLabel {"
        "    background-color: rgb(130, 130, 130);"
        "    color: white;"
        "    border: 1px solid black;"
        "    padding: 5px;"
        "}"
        );

    ui->TytulWykres3->setStyleSheet(
        "QLabel {"
        "    background-color: rgb(130, 130, 130);"
        "    color: white;"
        "    border: 1px solid black;"
        "    padding: 5px;"
        "}"
        );

    ui->TytulWykres4->setStyleSheet(
        "QLabel {"
        "    background-color: rgb(130, 130, 130);"
        "    color: white;"
        "    border: 1px solid black;"
        "    padding: 5px;"
        "}"
        );

    ui->TytulGlowny->setStyleSheet(
        "QLabel {"
        "    background-color: rgb(130, 130, 130);"
        "    color: white;"
        "    border: 1px solid black;"
        "    padding: 5px;"
        "}"
        );
    ui->InterwalCzasowyLabel->setStyleSheet(
        "QLabel {"
        "    background-color: rgb(130, 130, 130);"
        "    color: white;"
        "    border: 1px solid black;"
        "    padding: 5px;"
        "}"
        );
    ui->lblPolaczenie->setStyleSheet(
        "QLabel {"
        "    background-color: rgb(130, 130, 130);"
        "    color: white;"
        "    border: 1px solid black;"
        "    padding: 5px;"
        "}"
        );

    QGraphicsDropShadowEffect *effect[8]; //

    for (int i = 0; i < 8; ++i) {
        effect[i] = new QGraphicsDropShadowEffect();
        effect[i]->setOffset(0, 0);
        effect[i]->setBlurRadius(10);
        effect[i]->setColor(Qt::black);
    }
    ui->Start->setGraphicsEffect(effect[0]);
    ui->Stop->setGraphicsEffect(effect[1]);
    ui->Reset->setGraphicsEffect(effect[2]);
    ui->UstawieniaObiektuARX->setGraphicsEffect(effect[4]);
    ui->Zapisz->setGraphicsEffect(effect[6]);
    ui->Wczytaj->setGraphicsEffect(effect[7]);



}

//Generator

void MainWindow::on_Amplituda_editingFinished()
{
    gen->setAmplituda(ui->Amplituda->value());
    usluga->getSymulator()->getGenerator().setAmplituda(ui->Amplituda->value());
}
void MainWindow::on_Wypelnienie_editingFinished()
{
    gen->setWypelnienie(ui->Wypelnienie->value());
    usluga->getSymulator()->getGenerator().setWypelnienie(ui->Wypelnienie->value());
}
void MainWindow::on_CzasAktywacji_editingFinished()
{
    gen->setCzasAktywacji(ui->CzasAktywacji->value());
    usluga->getSymulator()->getGenerator().setWypelnienie(ui->CzasAktywacji->value());
}
void MainWindow::on_Okres_editingFinished()
{
    gen->setOkres(ui->Okres->value());
    usluga->getSymulator()->getGenerator().setOkres(ui->Okres->value());
}

//Regulator

void MainWindow::on_Wzmocnienie_editingFinished()
{
    reg->setWzmocnienie(ui->Wzmocnienie->value());
    usluga->getSymulator()->getRegulator().setWzmocnienie(ui->Wzmocnienie->value());
}
void MainWindow::on_StalaI_editingFinished()
{
    reg->setStalaI(ui->StalaI->value());
    usluga->getSymulator()->getRegulator().setStalaI(ui->StalaI->value());
}
void MainWindow::on_StalaD_editingFinished()
{
    reg->setStalaD(ui->StalaD->value());
    usluga->getSymulator()->getRegulator().setStalaD(ui->StalaD->value());
}


void MainWindow::on_Interwal_editingFinished()
{
    int interwal = ui->Interwal->text().toInt();
    if(interwal>0){

        interwalCzasowy=interwal;
        simulationTimer->setInterval(interwalCzasowy);
    }
}
void MainWindow::on_RodzajSygnalu_clicked()
{
    ui->RodzajSygnalu->showMenu();
}
void MainWindow::on_RodzajSygnalu_triggered(QAction *arg1)
{
    QString wybor = arg1->text();
    if (wybor == "Skokowy") {
        gen->setRodzaj(RodzajSygnalu::Skok);
        usluga->getSymulator()->getGenerator().setRodzaj(RodzajSygnalu::Skok);
        ui->RodzajSygnalu->setText("Skokowy");
    } else if (wybor == "Sinusoidalny") {
        gen->setRodzaj(RodzajSygnalu::Sinusoida);
        usluga->getSymulator()->getGenerator().setRodzaj(RodzajSygnalu::Sinusoida);
        ui->RodzajSygnalu->setText("Sinusoidalny");
    } else if (wybor == "Prostokątny") {
        gen->setRodzaj(RodzajSygnalu::Prostokatny);
        usluga->getSymulator()->getGenerator().setRodzaj(RodzajSygnalu::Prostokatny);
        ui->RodzajSygnalu->setText("Prostokątny");
    }
}
void MainWindow::on_Sposob_clicked()
{
    ui->Sposob->showMenu();
}
void MainWindow::on_Sposob_triggered(QAction *arg1)
{
    QString wybor = arg1->text();
    if(wybor=="Stała przed Sumą"){
        reg->setCalkowanieWsumie(false);
        usluga->getSymulator()->getRegulator().setCalkowanieWsumie(false);
        ui->Sposob->setText("Stała przed Sumą");
    }else if(wybor=="Stała w Sumie"){
        reg->setCalkowanieWsumie(true);
        usluga->getSymulator()->getRegulator().setCalkowanieWsumie(true);
        ui->Sposob->setText("Stała w Sumie");
    }
}


void MainWindow::on_btnSendSignal_clicked()
{

    QString adres = ui->letIP->text();

    port = ui->sbxPort->value();

    if(!ui->chkServer->isChecked()){
        TCPpolaczenie = new QTcpSocket(this);

        ui->statusbar->showMessage("Łączenie...");

        address.setAddress(adres);

        if(!address.isNull()) {
           TCPpolaczenie->connectToHost(address,port);

                if(TCPpolaczenie->waitForConnected(3000)) {

                    //QMessageBox::information(this, "Informacja", "Połączono z " + address.toString());

                    ui->statusbar->showMessage("Otrzymano połączenie z serwerem " + address.toString() + " na porcie:  " +  QString::number(port));
                    ui->btnSendSignal->setEnabled(0);
                    ui->btnRozlacz->setEnabled(1);
                    connect(TCPpolaczenie, &QTcpSocket::readyRead, this, &MainWindow::odczyt_klient);
                    connect(TCPpolaczenie, &QTcpSocket::disconnected, this, &MainWindow::errorPolaczenie);
                    ui->cbxZmianaTrybu->setEnabled(false);
                    tryb_stac = false;
                }
                else    {
                    QMessageBox::information(this, "Informacja", "Błąd połączenia");
                    ui->statusbar->showMessage("Błąd połączenia " + TCPpolaczenie->errorString());
                }
        }
        else {
            ui->statusbar->showMessage("Błąd połączenia " + TCPpolaczenie->errorString());
            QMessageBox::information(this, "Informacja", "Sprawdź czy poprawnie uzupełniłeś adress ip oraz porty");
        }
    }
    else    {
        TCPserver = new QTcpServer(this);

        if(TCPserver->listen(QHostAddress::AnyIPv4, port))  {
            ui->statusbar->showMessage("Serwer nasłuchuje na porcie: " + QString::number(port));
            connect(TCPserver, SIGNAL(newConnection()), this, SLOT(Otrzymaj())); //jest git
            //connect(TCPserver, SIGNAL(disco))
            ui->btnSendSignal->setEnabled(0);
            ui->btnRozlacz->setEnabled(1);
            ui->cbxZmianaTrybu->setEnabled(false);
            tryb_stac = false;
        }
        else    {
            QMessageBox::information(this, "Informacja", "Błąd servera TCP");
        }

    }
}

void MainWindow::Otrzymaj() {

    TCPpolaczenie = TCPserver->nextPendingConnection();
    connect(TCPpolaczenie, &QTcpSocket::readyRead, this, &MainWindow::odczyt);
    connect(TCPpolaczenie, &QTcpSocket::disconnected, this, &MainWindow::errorPolaczenie);
    if(TCPserver != nullptr) {
        ui->statusbar->showMessage("Połączono z kilentem o adresie:" + TCPpolaczenie->peerAddress().toString() );
    }
}

void MainWindow::on_btnRozlacz_clicked()
{
    QMessageBox potwierdzenie(this);
    potwierdzenie.setWindowTitle("Rozłącz..");
    potwierdzenie.setText("Czy chcesz zakończyć połączenie?");
    potwierdzenie.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    potwierdzenie.setWindowModality(Qt::ApplicationModal);

    int wynik = potwierdzenie.exec();

    if( wynik == QMessageBox::No)    {
        return;
    }
    else    {


        ui->label_color->setStyleSheet("QLabel{background-color : transparent;}");
        //ui->ms_label->setText("");
        if(TCPpolaczenie != nullptr)
        {
            if(TCPpolaczenie->isOpen())
            {
                ui->statusbar->showMessage("Rozłączono z:  " + address.toString());
                //TCPpolaczenie->disconnectFromHost();
                TCPpolaczenie->close();
                ui->btnSendSignal->setEnabled(1);
                ui->btnRozlacz->setEnabled(0);
                //TCPpolaczenie = nullptr;
            }
        }
        if(TCPserver != nullptr)
        {
            if(TCPserver->isListening())
            {
                TCPserver->close();

                ui->statusbar->showMessage("Server zakończył prace");
                ui->btnSendSignal->setEnabled(1);
                ui->btnRozlacz->setEnabled(0);
                //TCPserver = nullptr;
            }
        }
         ui->cbxZmianaTrybu->setEnabled(true);
        tryb_stac = true;
    }
}


void MainWindow::on_chkServer_stateChanged(int arg1)
{
    if(ui->letIP->isVisible()) ui->letIP->setVisible(0);
    else ui->letIP->setVisible(1);


    //if(ui->chkObustronneTaktowanie->isVisible()) ui->chkObustronneTaktowanie->setVisible(0);
    //else ui->chkObustronneTaktowanie->setVisible(1);

    if(ui->chkServer->isChecked())
    {
        ui->UstawieniaObiektuARX->setEnabled(1);
        ui->Sposob->setEnabled(0);
        ui->Interwal->setEnabled(0);
        ui->RodzajSygnalu->setEnabled(0);
        ui->Amplituda->setEnabled(0);
        ui->Wypelnienie->setEnabled(0);
        ui->CzasAktywacji->setEnabled(0);
        ui->Okres->setEnabled(0);
        ui->Wzmocnienie->setEnabled(0);
        ui->StalaI->setEnabled(0);
        ui->StalaD->setEnabled(0);
        ui->Start->setEnabled(0);
        ui->Stop->setEnabled(0);
        ui->Reset->setEnabled(0);

        ui->lblAdresIp->setVisible(0);
    }
    if(!ui->chkServer->isChecked())
    {
        ui->UstawieniaObiektuARX->setEnabled(0);
        ui->Sposob->setEnabled(1);
        ui->Interwal->setEnabled(1);
        ui->RodzajSygnalu->setEnabled(1);
        ui->Amplituda->setEnabled(1);
        ui->Wypelnienie->setEnabled(1);
        ui->CzasAktywacji->setEnabled(1);
        ui->Okres->setEnabled(1);
        ui->Wzmocnienie->setEnabled(1);
        ui->StalaI->setEnabled(1);
        ui->StalaD->setEnabled(1);
        ui->Start->setEnabled(1);
        ui->Stop->setEnabled(1);
        ui->Reset->setEnabled(1);
        ui->lblPort->setVisible(1);
        ui->lblAdresIp->setVisible(1);

    }
}


void MainWindow::on_cbxZmianaTrybu_activated(int index)
{
    if(index == 0)  {

        ui->lblPolaczenie->setVisible(0);
        ui->btnRozlacz->setVisible(0);
        ui->btnSendSignal->setVisible(0);
        ui->chkServer->setVisible(0);
        ui->letIP->setVisible(0);
        ui->sbxPort->setVisible(0);
        ui->chkObustronneTaktowanie->setVisible(0);
        ui->lblPort->setVisible(0);
        ui->lblAdresIp->setVisible(0);

        ui->Sposob->setEnabled(1);
        ui->Interwal->setEnabled(1);
        ui->RodzajSygnalu->setEnabled(1);
        ui->Amplituda->setEnabled(1);
        ui->Wypelnienie->setEnabled(1);
        ui->CzasAktywacji->setEnabled(1);
        ui->Okres->setEnabled(1);
        ui->Wzmocnienie->setEnabled(1);
        ui->StalaI->setEnabled(1);
        ui->StalaD->setEnabled(1);
        ui->UstawieniaObiektuARX->setEnabled(1);
        ui->Zapisz->setEnabled(1);
        ui->Wczytaj->setEnabled(1);

    }
    else if(index == 1) {

        ui->lblPolaczenie->setVisible(1);
        ui->btnRozlacz->setVisible(1);
        ui->btnSendSignal->setVisible(1);
        ui->chkServer->setVisible(1);
        ui->letIP->setVisible(1);
        ui->sbxPort->setVisible(1);
        ui->lblPort->setVisible(1);
        ui->lblAdresIp->setVisible(1);
        if(ui->chkServer->isChecked()){
            ui->chkServer->setChecked(false);
            if(ui->letIP->isVisible()) ui->letIP->setVisible(0);
            else ui->letIP->setVisible(1);

            //if(ui->chkObustronneTaktowanie->isVisible()) ui->chkObustronneTaktowanie->setVisible(0);
            //else ui->chkObustronneTaktowanie->setVisible(1);
        }
        ui->chkObustronneTaktowanie->setVisible(1);

        if(ui->chkServer->isChecked())
        {
            ui->UstawieniaObiektuARX->setEnabled(1);
            ui->Sposob->setEnabled(0);
            ui->Interwal->setEnabled(0);
            ui->RodzajSygnalu->setEnabled(0);
            ui->Amplituda->setEnabled(0);
            ui->Wypelnienie->setEnabled(0);
            ui->CzasAktywacji->setEnabled(0);
            ui->Okres->setEnabled(0);
            ui->Wzmocnienie->setEnabled(0);
            ui->StalaI->setEnabled(0);
            ui->StalaD->setEnabled(0);
        }
        if(!ui->chkServer->isChecked())
        {
            ui->UstawieniaObiektuARX->setEnabled(0);
            ui->Sposob->setEnabled(1);
            ui->Interwal->setEnabled(1);
            ui->RodzajSygnalu->setEnabled(1);
            ui->Amplituda->setEnabled(1);
            ui->Wypelnienie->setEnabled(1);
            ui->CzasAktywacji->setEnabled(1);
            ui->Okres->setEnabled(1);
            ui->Wzmocnienie->setEnabled(1);
            ui->StalaI->setEnabled(1);
            ui->StalaD->setEnabled(1);
        }


        ui->Zapisz->setEnabled(0);
        ui->Wczytaj->setEnabled(0);
    }

}

void MainWindow::errorPolaczenie(){

    ui->cbxZmianaTrybu->setEnabled(true);
    ui->label_color->setStyleSheet("QLabel{background-color : transparent;}");
    //ui->ms_label->setText("");
    tryb_stac = true;
    ui->lblPolaczenie->setVisible(0);
    ui->btnRozlacz->setVisible(0);
    ui->btnSendSignal->setVisible(0);
    ui->chkServer->setVisible(0);
    ui->letIP->setVisible(0);
    ui->sbxPort->setVisible(0);
    ui->chkObustronneTaktowanie->setVisible(0);

    ui->Sposob->setEnabled(1);
    ui->Interwal->setEnabled(1);
    ui->RodzajSygnalu->setEnabled(1);
    ui->Amplituda->setEnabled(1);
    ui->Wypelnienie->setEnabled(1);
    ui->CzasAktywacji->setEnabled(1);
    ui->Okres->setEnabled(1);
    ui->Wzmocnienie->setEnabled(1);
    ui->StalaI->setEnabled(1);
    ui->StalaD->setEnabled(1);
    ui->UstawieniaObiektuARX->setEnabled(1);
    ui->Zapisz->setEnabled(1);
    ui->Wczytaj->setEnabled(1);
    ui->Start->setEnabled(1);
    ui->Stop->setEnabled(1);
    ui->Reset->setEnabled(1);
    ui->cbxZmianaTrybu->setCurrentIndex(0);

    ui->ms_label->setVisible(0);
    emit ui->cbxZmianaTrybu->activated(0);
    packet_number = 0;
    ui->btnSendSignal->setEnabled(1);
    ui->btnRozlacz->setEnabled(0);
    if(TCPpolaczenie != nullptr) TCPpolaczenie = nullptr;
    if(TCPserver != nullptr)
    {
        TCPserver->close();
        TCPserver = nullptr;
    }
    if(!simulationTimer->isActive()) simulationTimer->start(interwalCzasowy);
    blokada = false;
    this->simulationTimer->setInterval(ui->Interwal->text().toInt());
    //QMessageBox::information(this, "Informacja", "Nastąpiło nagłe utracenie połaczenia");
}


void MainWindow::on_chkObustronneTaktowanie_checkStateChanged(const Qt::CheckState &arg1)
{
    if(ui->chkServer->isChecked()) simulationTimer->stop();
}

