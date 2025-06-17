#include "wykresy.h"

Wykresy::Wykresy(QWidget *parent,symulator* sym)
    : QObject(nullptr),czas(0.0),parent(parent),s(sym)
{

}

Wykresy::~Wykresy()
{
    for(int i = 0; i < 7; i++)
        if (seria[i]) delete seria[i];

    for(int i = 0; i < 4; i++) {
        if (wykres[i]) delete wykres[i];
        if (osY[i]) delete osY[i];
        if (osX[i]) delete osX[i];
        if (Widok[i]) delete Widok[i];
    }
}

void Wykresy::wyczyscLayout(QLayout* layout) {
    if (!layout) return;
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        if (QLayout* childLayout = item->layout()) {
            wyczyscLayout(childLayout);
        }
        delete item;
    }
}
void Wykresy::inicjalizacjaWykresuWartosciZadanej(QVBoxLayout *layout){
    wyczyscLayout(layout);
    seria[0] = new QLineSeries();
    seria[1] = new QLineSeries();
    seria[0]->setName("Wartość Regulowana");
    seria[1]->setName("Wartość Zadana");
    seria[1]->setColor(Qt::red);
    wykres[0] = new QChart();
    wykres[0]->addSeries(seria[0]);
    wykres[0]->addSeries(seria[1]);
    osX[0] = new QValueAxis();
    osX[0]->setRange(0, 1000);
    osY[0] = new QValueAxis();
    osY[0]->setRange(0, 10);
    wykres[0]->addAxis(osX[0], Qt::AlignBottom);
    wykres[0]->addAxis(osY[0], Qt::AlignLeft);
    seria[0]->attachAxis(osX[0]);
    seria[0]->attachAxis(osY[0]);
    seria[1]->attachAxis(osX[0]);
    seria[1]->attachAxis(osY[0]);
    Widok[0] = new QChartView(wykres[0]);
    Widok[0]->setRenderHint(QPainter::Antialiasing);
    Widok[0]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(Widok[0]);
    osX[0]->setTitleText("Czas [mS]");
    osY[0]->setTitleText("Wartości regulacji");
}
void Wykresy::inicjalizacjaWykresuUchybu(QVBoxLayout *layout){
    wyczyscLayout(layout);
    seria[2] = new QLineSeries();
    seria[2]->setName("Wartość Uchybu");
    wykres[1] = new QChart();
    wykres[1]->addSeries(seria[2]);
    osX[1] = new QValueAxis();
    osX[1]->setRange(0, 400);
    osY[1] = new QValueAxis();
    osY[1]->setRange(0, 10);
    wykres[1]->addAxis(osX[1], Qt::AlignBottom);
    wykres[1]->addAxis(osY[1], Qt::AlignLeft);
    seria[2]->attachAxis(osX[1]);
    seria[2]->attachAxis(osY[1]);
    Widok[1] = new QChartView(wykres[1]);
    Widok[1]->setRenderHint(QPainter::Antialiasing);
    Widok[1]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(Widok[1]);
    osX[1]->setTitleText("Czas [mS]");
    osY[1]->setTitleText("Wartość uchybu");
}
void Wykresy::inicjalizacjaWykresuPID(QVBoxLayout *layout){
    wyczyscLayout(layout);
    seria[3] = new QLineSeries();
    seria[3]->setName("Proporcjonalna");
    seria[4] = new QLineSeries();
    seria[4]->setColor(Qt::red);
    seria[4]->setName("Całkująca");
    seria[5] = new QLineSeries();
    seria[5]->setColor(Qt::black);
    seria[5]->setName("Różniczkująca");
    wykres[2] = new QChart();
    wykres[2]->addSeries(seria[3]);
    wykres[2]->addSeries(seria[4]);
    wykres[2]->addSeries(seria[5]);
    osX[2] = new QValueAxis();
    osX[2]->setRange(0, 400);
    osY[2] = new QValueAxis();
    osY[2]->setRange(0, 10);
    wykres[2]->addAxis(osX[2], Qt::AlignBottom);
    wykres[2]->addAxis(osY[2], Qt::AlignLeft);
    seria[3]->attachAxis(osX[2]);
    seria[3]->attachAxis(osY[2]);
    seria[4]->attachAxis(osX[2]);
    seria[4]->attachAxis(osY[2]);
    seria[5]->attachAxis(osX[2]);
    seria[5]->attachAxis(osY[2]);
    Widok[2] = new QChartView(wykres[2]);
    Widok[2]->setRenderHint(QPainter::Antialiasing);
    Widok[2]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(Widok[2]);
    osX[2]->setTitleText("Czas [mS]");
    osY[2]->setTitleText("Wartość ster PID");
}
void Wykresy::inicjalizacjaWykresuWartosciSterowania(QVBoxLayout *layout){
    wyczyscLayout(layout);
    seria[6] = new QLineSeries();
    seria[6]->setName("Wartość Sterująca");
    wykres[3] = new QChart();
    wykres[3]->addSeries(seria[6]);
    osX[3] = new QValueAxis();
    osX[3]->setRange(0, 400);
    osY[3] = new QValueAxis();
    osY[3]->setRange(0, 10);
    wykres[3]->addAxis(osX[3], Qt::AlignBottom);
    wykres[3]->addAxis(osY[3], Qt::AlignLeft);
    seria[6]->attachAxis(osX[3]);
    seria[6]->attachAxis(osY[3]);
    Widok[3] = new QChartView(wykres[3]);
    Widok[3]->setRenderHint(QPainter::Antialiasing);
    Widok[3]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(Widok[3]);
    osX[3]->setTitleText("Czas [mS]");
    osY[3]->setTitleText("Wartość Ster");
}
void Wykresy::widzialnoscWykresow(bool lol){
    seria[1]->setVisible(lol);
    seria[2]->setVisible(lol);
    seria[3]->setVisible(lol);
    seria[4]->setVisible(lol);
    seria[5]->setVisible(lol);
}
void Wykresy::WykresWartosciZadanej() {
    seria[0]->append(czas, s->getWyjscieObiektu());
    seria[1]->append(czas, s->getWartoscZadana());

    const int maxPoints = 1000;
    if (seria[0]->count() > maxPoints) {
        seria[0]->remove(0);
        seria[1]->remove(0);
    }

    if (czas > maxPoints) {
        osX[0]->setRange(czas - maxPoints, czas);
    }

    Generator generator = s->getGenerator();
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    for (int i = 0; i < seria[0]->count(); ++i) {
        double yValue1 = seria[0]->at(i).y();
        double yValue2 = seria[1]->at(i).y();
        minY = std::min({minY, yValue1, yValue2});
        maxY = std::max({maxY, yValue1, yValue2});
    }
    /*
    if (generator.getRodzaj() == RodzajSygnalu::Skok) {
        //minY = 0;
    }
    else if (generator.getRodzaj() == RodzajSygnalu::Sinusoida || generator.getRodzaj() == RodzajSygnalu::Prostokatny) {
        double amplituda = generator.getAmplituda();
        minY = -amplituda;
        maxY = amplituda;
    }
*/
    double margin = (maxY - minY) * 0.1;
    osY[0]->setRange(minY - margin, maxY + margin);

}
void Wykresy::WykresWartosciZadanej_no_base() {
    seria[0]->append(czas, s->getWyjscieObiektu());
    seria[1]->append(czas, s->getWartoscZadana());
    seria[1]->setVisible(false);
    const int maxPoints = 1000;
    if (seria[0]->count() > maxPoints) {
        seria[0]->remove(0);
        seria[1]->remove(0);
    }

    if (czas > maxPoints) {
        osX[0]->setRange(czas - maxPoints, czas);
    }

    Generator generator = s->getGenerator();
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    for (int i = 0; i < seria[0]->count(); ++i) {
        double yValue1 = seria[0]->at(i).y();
        //double yValue2 = seria[1]->at(i).y();
        minY = std::min({minY, yValue1});
        maxY = std::max({maxY, yValue1});
    }
    /*
    if (generator.getRodzaj() == RodzajSygnalu::Skok) {
        //minY = 0;
    }
    else if (generator.getRodzaj() == RodzajSygnalu::Sinusoida || generator.getRodzaj() == RodzajSygnalu::Prostokatny) {
        double amplituda = generator.getAmplituda();
        minY = -amplituda;
        maxY = amplituda;
    }*/
    double margin = (maxY - minY) * 0.1;
    osY[0]->setRange(minY - margin, maxY + margin);

}



void Wykresy::WykresUchybu() {
    double uchyb = s->getRegulator().getUchyb();
    seria[2]->append(czas, uchyb);
    const int maxPoints = 400;
    if (seria[2]->count() > maxPoints) {
        seria[2]->remove(0);
    }

    if (czas > maxPoints) {
        osX[1]->setRange(czas - maxPoints, czas);
    }

    double minY = uchyb;
    double maxY = uchyb;
    for (int i = 0; i < seria[2]->count(); ++i) {
        double yValue = seria[2]->at(i).y();
        minY = std::min(minY, yValue);
        maxY = std::max(maxY, yValue);
    }
    double margin = std::max(0.1 * (maxY - minY), 0.01);

    osY[1]->setRange(minY - margin, maxY + margin);

}

void Wykresy::WykresPID() {
    Regulator regulator = s->getRegulator();
    seria[3]->append(czas, regulator.getNastawaP());
    seria[4]->append(czas, regulator.getNastawaI());
    seria[5]->append(czas, regulator.getNastawaD());

    const int maxPoints = 400;
    if (seria[3]->count() > maxPoints) {
        seria[3]->remove(0);
        seria[4]->remove(0);
        seria[5]->remove(0);
    }

    if (czas > maxPoints) {
        osX[2]->setRange(czas - maxPoints, czas);
    }

    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    for (int i = 0; i < seria[3]->count(); ++i) {
        double yValueP = seria[3]->at(i).y();
        double yValueI = seria[4]->at(i).y();
        double yValueD = seria[5]->at(i).y();

        minY = std::min({minY, yValueP, yValueI, yValueD});
        maxY = std::max({maxY, yValueP, yValueI, yValueD});
    }

    double margin = maxY*0.1;
    minY -= margin;
    maxY += margin;

    if (osY[2]->min() != minY || osY[2]->max() != maxY) {
        osY[2]->setRange(minY, maxY);
    }
}
void Wykresy::WykresWartosciSterowania(){
    double Sterujaca = s->getLastRegulatorValue();
    seria[6]->append(czas, Sterujaca);

    const int maxPoints = 400;
    if (seria[6]->count() > maxPoints) {
        seria[6]->remove(0);
    }

    if (czas > maxPoints) {
        osX[3]->setRange(czas - maxPoints, czas);
    }

    double minY = Sterujaca;
    double maxY = Sterujaca;

    for (int i = 0; i < seria[6]->count(); ++i) {
        double yValue = seria[6]->at(i).y();
        minY = std::min(minY, yValue);
        maxY = std::max(maxY, yValue);
    }

    double margin = std::max(0.1 * (maxY - minY), 0.01);
    osY[3]->setRange(minY - margin, maxY + margin);
}

void Wykresy::ResetujWykresy(){
    for(int i=0;i<7;i++){
        if (seria[i]) {
            seria[i]->clear();
        }
    }
}

void Wykresy::InicjalizujWykresy(QVBoxLayout *layout[4] ){
    inicjalizacjaWykresuWartosciZadanej(layout[0]);
    inicjalizacjaWykresuUchybu(layout[1]);
    inicjalizacjaWykresuPID(layout[2]);
    inicjalizacjaWykresuWartosciSterowania(layout[3]);

}
void Wykresy::AktualizujWykresy(){
    for(int i=0;i<4;i++){
        Widok[i]->update();
    }
}
