#ifndef DATASTORAGE_H
#define DATASTORAGE_H
#include <QStringList>

class Database;
class SimulatorStart;
class Datastorage
{
  public:
    Datastorage();
    QVector<QStringList> Vpolice;
    QVector<QStringList> Vmch;
    QVector<QStringList> Vfire;
    QVector<QStringList> Vambulance;
    QVector<QStringList> Vincidents;
    friend class DataBase;
    friend class SimulatorStart;
    int getSizeBrigade();

  private:
    int sizeBrigades;
};

#endif // DATASTORAGE_H
