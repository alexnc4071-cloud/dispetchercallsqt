#ifndef SIMULATORSTART_H
#define SIMULATORSTART_H
#include "datastorage.h"
#include <QRandomGenerator>
#include <QStringList>
#include <QTimer>
#include <QVector>
#include <QWidget>
class SimulatorStart : public QWidget
{
    Q_OBJECT
  public:
    explicit SimulatorStart(QWidget* parent = nullptr);
    void runSimulator(bool flag);
    void resetDigitsRandom();
    int getSizeProblems();
    void setCounter(int count);
    void counterPlusPlus();
  public slots:
    void slot_acceptTimeout();
  signals:
    void sig_sendMapDateOfCall(QMap<QString, QString> mapDateOfCall);
    void sig_sendMarksAllAppeared();

  private:
    Datastorage storageClass;
    int sizeProblems;
    QVector<int> digitsRandom;
    int counter = 0;
    QTimer* timer;
};

#endif // SIMULATORSTART_H
