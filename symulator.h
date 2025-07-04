#ifndef SYMULATOR_H
#define SYMULATOR_H

#include "Generator.h"
#include "Regulator.h"
#include "ObiektARX.h"

class symulator
{
    Generator generator;
    Regulator regulator;
    ObiektARX obiekt;
    double poprzednieWyjscie = 0;
    double wyjscieObiektu = 0;
    double lastRegulatorValue = 0;
    double lastObjectOutput = 0 ;
    double lastGeneratorValue = 0;
    bool flaga = true;
    bool flaga_synchro = true;
public:
    symulator();
    symulator(Generator g, Regulator r, ObiektARX o);
    double symulujKrok(double czas);
    double symuluj_bez_wyjscia(double czas);
    double symuluj_wyjscie();
    double getWartoscZadana();
    double getZaklocenie();
    double getSterowanie();
    double getWyjscieObiektu();
    void setGenerator(Generator &g);
    void setRegulator(Regulator &r);
    void setObiektARX(ObiektARX &o);
    Generator &getGenerator();
    Regulator &getRegulator();
    ObiektARX getObiektARX();
    ObiektARX &getObiektARX_fun(){return this->obiekt;};
    void setLastRegulatorValue(double value) { lastRegulatorValue = value; }
    void setLastObjectOutput(double value) { lastObjectOutput = value; }
    void setLastGeneratorValue(double value) { lastGeneratorValue = value; }
    double getLastGeneratorValue()  { return lastGeneratorValue; }
    double getLastRegulatorValue()  { return lastRegulatorValue; }
    double getLastObjectOutput()  { return lastObjectOutput; }
    void setWyjscieObiektu(double wo){wyjscieObiektu=wo;};
    void setObiektARXzHistoria(ObiektARX &o);
    void setRegulatorzHistoria(Regulator &r);
    double symuluj_w_tle(double czas);
    double symuluj_w_tle_regulator(double czas);
    bool getFlag(){return flaga;}
    void setFlag(bool wart){flaga = wart;}
    bool getFlagSynchro(){return flaga_synchro;}
    void setFlagSynchro(bool wart){flaga_synchro = wart;}
    void setWyjscie(double wal){wyjscieObiektu = 0;}
    void resetSym(){
       poprzednieWyjscie = 0;
        wyjscieObiektu = 0;
        lastRegulatorValue = 0;
        lastObjectOutput = 0 ;
        lastGeneratorValue = 0;
    }
};

#endif // SYMULATOR_H

