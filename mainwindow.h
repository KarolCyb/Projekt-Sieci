#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "wykresy.h"
#include <QMainWindow>
#include <QTimer>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include "warstwauslug.h"
#include "oknoobiektarx.h"
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpServer>
#include <chrono>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE
class WarstwaUslug;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr,
               WarstwaUslug* program = nullptr);
    ~MainWindow();
    void UstawienieWygladuGUI();
    void UstawienieOkienOrazSygnalowIslotow();
    bool getWyslijInterwal() {return wyslij_interwal;}
    void setWyslijInterwal(bool set){wyslij_interwal = set;}
private slots:
    void on_Start_clicked();
    void on_Stop_clicked();
    void on_UstawieniaObiektuARX_clicked();
    void PokazWykres();
    void Blad();
    void on_Reset_clicked();
    void on_Zapisz_clicked();
    void on_Wczytaj_clicked();
    void bladUstawien();
    void on_Amplituda_editingFinished();
    void on_Wypelnienie_editingFinished();
    void on_CzasAktywacji_editingFinished();
    void on_Okres_editingFinished();
    void on_Wzmocnienie_editingFinished();
    void on_StalaI_editingFinished();
    void on_StalaD_editingFinished();
    void on_Interwal_editingFinished();
    void on_RodzajSygnalu_clicked();
    void on_RodzajSygnalu_triggered(QAction *arg1);
    void on_Sposob_clicked();
    void on_Sposob_triggered(QAction *arg1);
    void on_btnSendSignal_clicked();
    void Otrzymaj();

    void dane_i_wykresy();
    void odczyt();
    void odczyt_klient();
    void on_btnRozlacz_clicked();

    void on_chkServer_stateChanged(int arg1);

    void on_cbxZmianaTrybu_activated(int index);

    void errorPolaczenie();

    void on_chkObustronneTaktowanie_checkStateChanged(const Qt::CheckState &arg1);

private:
    Ui::MainWindow *ui;
    QTimer *simulationTimer;
    WarstwaUslug* usluga ;
    OknoObiektARX* okno_obiekt;
    //int czas;
    int interwalCzasowy=0;
    Wykresy* wykres;
    void UstawienieLayout();
    QVBoxLayout *layout[4];
    void obslugaZapisu();

    void zapiszPakietRegulator(QByteArray &dane);
    void zapiszPakietArx(QByteArray &dane);
    void wczytajPakietOneClockArx(QByteArray &dane);
    void wczytajPakietOneClockRegulator(QByteArray &dane);
    void wczytajPakietTwoClockRegulator(QByteArray &dane);
    void wczytajPakietTwoClockObiekt(QByteArray &dane);
    QPushButton *Wczytaj;
    Regulator* reg;
    Generator* gen;
    QTcpSocket* TCPpolaczenie;
    QHostAddress address;
    int port;
    QTcpServer* TCPserver;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_m;
    std::chrono::time_point<std::chrono::high_resolution_clock> end_m;
    int packet_number = 0;
    float korekta = 1;
    bool wyslij_interwal = true;
    bool blokada = false;
    bool tryb_stac = true;
    bool run_str = true;
    int error = -1;
    std::deque<double> buferOneClock;
    std::deque<double> buferTwoClockRegulator;
    std::deque<double> buferTwoClockObiekt;
};
#endif // MAINWINDOW_H
