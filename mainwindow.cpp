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
    , czas(0.0)
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
    connect(simulationTimer, &QTimer::timeout, this, &MainWindow::dane_i_wykresy);
}
void MainWindow::dane_i_wykresy()
{

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
        dane_siec = QByteArray::number(usluga->getSymulator()->getLastRegulatorValue());
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
        dane_siec = QByteArray::number(usluga->getSymulator()->getLastRegulatorValue());
        dane_siec.append(QByteArray::number(wykres->getCzas()));
        dane_siec.append(QByteArray::number(interwalCzasowy));
        TCPpolaczenie->write(dane_siec);
        TCPpolaczenie->flush();
        ui->label_color->setStyleSheet("QLabel{background-color : red;}");
    }
    wykres->WykresUchybu();
    wykres->WykresPID();
    wykres->WykresWartosciSterowania();
    wykres->krok();
    wykres->AktualizujWykresy();
    //sinus po sieci nie dziala jak powinien
}

void MainWindow::odczyt()
{
    if(!ui->chkObustronneTaktowanie->isChecked())
    {
        QByteArray dane_siec;
        dane_siec = TCPpolaczenie->read(8);
        double val = dane_siec.toDouble();
        //dane_siec = TCPpolaczenie->read(8);
        //int time = dane_siec.toDouble();
        //TCPpolaczenie->flush();
        wykres->getSymulator()->setLastRegulatorValue(val);
        double wyjscie = wykres->getSymulator()->symuluj_wyjscie();
        //wykres->WykresWartosciZadanej();
        //wykres->AktualizujWykresy();
        //wykres->krok();
        QByteArray dane_siec_wyj = QByteArray::number(wyjscie);
        TCPpolaczenie->write(dane_siec_wyj);
        TCPpolaczenie->flush();
    }
    else if(ui->chkObustronneTaktowanie->isChecked())
    {
        QByteArray dane_siec;
        dane_siec = TCPpolaczenie->readAll();
        QByteArray wynik = dane_siec.sliced(0,8);
        QByteArray czas = dane_siec.sliced(8,4);
        QByteArray interwal_z = dane_siec.sliced(12,4);
        double val = wynik.toDouble();
        int time = czas.toInt();
        double interwal = interwal_z.toInt();

        //TCPpolaczenie->flush();
        wykres->getSymulator()->setLastRegulatorValue(val);
        double wyjscie = wykres->getSymulator()->symuluj_wyjscie();
        //wykres->WykresWartosciZadanej();
        //wykres->AktualizujWykresy();
        //wykres->krok();
        QByteArray dane_siec_wyj = QByteArray::number(wyjscie);
        TCPpolaczenie->write(dane_siec_wyj);
        TCPpolaczenie->flush();
    }
}
void MainWindow::odczyt_klient()
{
    if(!ui->chkObustronneTaktowanie->isChecked())
    {
        QByteArray dane_siec;
        dane_siec = TCPpolaczenie->read(8);
        double val_wyj = dane_siec.toDouble();
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
        dane_siec = TCPpolaczenie->read(8);
        double val_wyj = dane_siec.toDouble();
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
}

void MainWindow::on_Stop_clicked()
{
    simulationTimer->stop();
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
    usluga->ResetSymulacji(czas);
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
    ui->Wzmocnienie->setValue(0.1);
    ui->StalaI->setRange(0, 1000);
    ui->StalaI->setValue(5);
    ui->StalaD->setRange(0, 1000);
    ui->StalaD->setValue(0.1);
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
            connect(TCPserver, SIGNAL(newConnection()), this, SLOT(Otrzymaj())); //Przerzucic do konstruktora
            //connect(TCPserver, SIGNAL(disco))
            ui->btnSendSignal->setEnabled(0);
            ui->btnRozlacz->setEnabled(1);
        }
        else    {
            QMessageBox::information(this, "Informacja", "Błąd servera TCP");
        }

    }
}

void MainWindow::Otrzymaj() {

    TCPpolaczenie = TCPserver->nextPendingConnection();
    connect(TCPpolaczenie, &QTcpSocket::readyRead, this, &MainWindow::odczyt);
    if(TCPserver != nullptr) {
        ui->statusbar->showMessage("Połączono z kilentem o adresie:" + TCPpolaczenie->peerAddress().toString() );
    }
}

void MainWindow::on_btnRozlacz_clicked()
{
    ui->label_color->setStyleSheet("QLabel{background-color : transparent;}");
    ui->ms_label->setText("");
    if(TCPpolaczenie != nullptr)
    {
        if(TCPpolaczenie->isOpen())
        {
            ui->statusbar->showMessage("Rozłączono z:  " + address.toString());
            TCPpolaczenie->disconnectFromHost();
            TCPpolaczenie->close();
            ui->btnSendSignal->setEnabled(1);
            ui->btnRozlacz->setEnabled(0);
            TCPpolaczenie = nullptr;
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
            TCPserver = nullptr;
        }
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
}


void MainWindow::on_cbxZmianaTrybu_activated(int index)
{
    if(index == 0)  {

        emit on_btnRozlacz_clicked();

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
    }
    else if(index == 1) {
        ui->lblPolaczenie->setVisible(1);
        ui->btnRozlacz->setVisible(1);
        ui->btnSendSignal->setVisible(1);
        ui->chkServer->setVisible(1);
        ui->letIP->setVisible(1);
        ui->sbxPort->setVisible(1);
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

