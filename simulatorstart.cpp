#include "simulatorstart.h"

SimulatorStart::SimulatorStart(QWidget* parent) : QWidget{parent}
{
    timer = new QTimer(this);

    int realSize = storageClass.Vincidents.size();
    sizeProblems = realSize / 3;

    connect(timer, &QTimer::timeout, this, &SimulatorStart::slot_acceptTimeout);
}

void SimulatorStart::runSimulator(bool flag)
{
    if (flag)
    {
        timer->start(3000);
    }
    else
    {
        timer->stop();
    }
}

void SimulatorStart::slot_acceptTimeout()
{
    qDebug() << "коунтер: " << counter << " " << "коунтер проблемы: " << sizeProblems << Qt::endl;
    if (counter == sizeProblems)
    {
        counter = 0;
        timer->stop();
        qDebug() << "Симуляция закончилась, достигнуто максимальное количество точек";
        emit sig_sendMarksAllAppeared();
        return;
    }
    QString latitude;
    QString longitude;
    QString region;
    QString address;
    QString distance;
    QString problem;
    QString level;
    QString status;
    QString surname;
    QString phone;
    QString incident_enum;
    int counterStr = 0;
    int randomValue;
    while (true)
    {
        randomValue = QRandomGenerator::global()->bounded(sizeProblems);
        if (!digitsRandom.contains(randomValue))
        {
            digitsRandom.push_back(randomValue);
            break;
        }
    }
    latitude = storageClass.Vincidents[randomValue].at(counterStr++);
    longitude = storageClass.Vincidents[randomValue].at(counterStr++);
    region = storageClass.Vincidents[randomValue].at(counterStr++);
    address = storageClass.Vincidents[randomValue].at(counterStr++);
    distance = storageClass.Vincidents[randomValue].at(counterStr++);
    problem = storageClass.Vincidents[randomValue].at(counterStr++);
    level = storageClass.Vincidents[randomValue].at(counterStr++);
    status = storageClass.Vincidents[randomValue].at(counterStr++);
    surname = storageClass.Vincidents[randomValue].at(counterStr++);
    phone = storageClass.Vincidents[randomValue].at(counterStr++);
    incident_enum = storageClass.Vincidents[randomValue].at(counterStr++);
    QMap<QString, QString> mapDateOfCall;
    mapDateOfCall["latitude"] = latitude;
    mapDateOfCall["longitude"] = longitude;
    mapDateOfCall["region"] = region;
    mapDateOfCall["address"] = address;
    mapDateOfCall["distance"] = distance;
    mapDateOfCall["problem"] = problem;
    mapDateOfCall["level"] = level;
    mapDateOfCall["status"] = status;
    mapDateOfCall["surname"] = surname;
    mapDateOfCall["phone"] = phone;
    mapDateOfCall["incident_enum"] = incident_enum;
    emit sig_sendMapDateOfCall(mapDateOfCall);
}

void SimulatorStart::resetDigitsRandom()
{
    digitsRandom.clear();
}

int SimulatorStart::getSizeProblems()
{
    return sizeProblems;
}

void SimulatorStart::setCounter(int count)
{
    counter = count;
}

void SimulatorStart::counterPlusPlus()
{
    counter++;
}
