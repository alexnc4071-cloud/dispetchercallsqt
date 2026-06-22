#include "database.h"

DataBase::DataBase(QObject* parent) : QObject{parent}
{
    dataBase = new QSqlDatabase();
    mapTypeCode["Полиция"] = POLICE_;
    mapTypeCode["Мчс"] = MCH_;
    mapTypeCode["Скорая"] = AMBULANCE_;
    mapTypeCode["Пожарные"] = FIRE_;
    mapTypeCode["Полиция и скорая"] = POLICE_AND_AMBULANCE_;
    mapTypeCode["Мчс и скорая"] = MCH_AND_AMBULANCE_;
    msg = new QMessageBox;
    msg->setIcon(QMessageBox::Information);
    msg->setWindowIcon(QIcon(":/Images/IconWarning.png"));
    msg->setIconPixmap(QPixmap(":/Images/iconSign.png"));
    msg->setWindowTitle("Код вызова не соответствует с бригадой");
    sizeBrigades = storageClass.getSizeBrigade();
    qDebug() << "Количестов бригад: " << sizeBrigades << Qt::endl;
    statesBrigade.resize(sizeBrigades);
    for (int i = 0; i < sizeBrigades; i++)
    {
        statesBrigade[i] << (QStringList() << "Занята" << "В пути" << "На месте" << "Команда выполнела задачу");
    }
    activitedTimers.resize(sizeBrigades);
    timersVector.resize(sizeBrigades);
    for (int i = 0; i < sizeBrigades; i++)
    {
        timersVector[i] = nullptr;
        activitedTimers[i] = false;
    }
    busyBrigades.resize(sizeBrigades);
    QVector<bool> vec(sizeBrigades, false);
    checkStatusVector = vec;
    vecColorPriority.resize(5);
    vecColorPriority[0] = colorsPriority::RED_;
    vecColorPriority[1] = colorsPriority::YELLOW_;
    vecColorPriority[2] = colorsPriority::BLUE_;
    vecColorPriority[3] = colorsPriority::ORANGE_;
    vecColorPriority[4] = colorsPriority::GREEN_;
}

int DataBase::getSizeBrigades()
{
    return sizeBrigades;
}

DataBase::~DataBase()
{
    delete msg;
    if (dataBase)
    {
        if (callsModel)
        {
            delete callsModel;
            callsModel = nullptr;
        }
        if (brigadesModel)
        {
            delete brigadesModel;
            brigadesModel = nullptr;
        }
        if (dispatcherIdModel)
        {
            delete dispatcherIdModel;
            dispatcherIdModel = nullptr;
        }
        if (brigadesOnCallModel)
        {
            delete brigadesOnCallModel;
            brigadesOnCallModel = nullptr;
        }
        int i = 1;
        for (auto& ptr : timersVector)
        {
            if (ptr != nullptr)
            {
                delete ptr;
                qDebug() << "Таймер" << i++ << " удалён";
                ptr = nullptr;
            }
            else
            {
                qDebug() << "Таймер" << i++ << " уже удалён";
            }
        }
        disconnectFromDataBase();
        delete dataBase;
    }
}

void DataBase::addDataBase(QString driver, QString nameDB)
{
    *dataBase = QSqlDatabase::addDatabase(driver, nameDB);
}

void DataBase::connectToDataBase(QVector<QString> data)
{
    dataBase->setHostName(data[HOST_NAME_]);
    dataBase->setDatabaseName(data[DBNAME_]);
    dataBase->setUserName(data[LOGIN_]);
    dataBase->setPassword(data[PASS_]);
    dataBase->setPort(data[PORT_].toInt());
    bool status = dataBase->open();
    if (!status)
    {
        qDebug() << "Не удалось подключиться к базе данных!";
        emit sig_SendStatusConnection(false);
        return;
    }
    defaultSettings();
    qDebug() << "Успешное подключение к базе данных!";
    if (firstLaunch)
    {
        qDebug() << "Первый запуск" << Qt::endl;
        firstLaunch = false;
        if (callsModel->rowCount() != 0)
        {
            checkLaunch();
        }
        else
        {
            qDebug() << "Вызовы пустые" << Qt::endl;
        }
    }
    else
    {
        qDebug() << "Запуск явно не первый" << Qt::endl;
    }
    emit sig_SendStatusConnection(true);
    loadCalls();
    loadBrigades();
    loadDispatchers();
}

QMap<int, QString> DataBase::getBeginStatus()
{
    return mapBeginStatus;
}

void DataBase::checkLaunch()
{
    int size = callsModel->rowCount();
    multiFinishedCalls.resize(size, 1);
    multiDispatcher.resize(size);
    busyCalls.resize(size);
    vecCallsPriority.resize(size, colorsPriority::NOT_COLOR);
    vecCounterForDeleteMultiCalls.resize(size, 1);
    checkIndexUnsignedVector.resize(size, true);
    for (int i = 0; i < size; i++)
    {
        mapBusyCallsTypes[i].resize(3, false);
    }
    QSqlQuery checkColors(*dataBase);
    checkColors.prepare("SELECT id_brigade, id_call FROM brigades_on_call WHERE flag != ?");
    checkColors.addBindValue(2);
    if (!checkColors.exec())
    {
        qDebug() << "Ошибка выполнения запроса: " << checkColors.lastError().text() << Qt::endl;
        return;
    }
    while (checkColors.next())
    {
        int idBrig = checkColors.record().value(0).toInt();
        int idCal = checkColors.record().value(1).toInt();
        QSqlQuery checkStatus(*dataBase);
        checkStatus.prepare("SELECT status FROM brigades WHERE id =?");
        checkStatus.addBindValue(idBrig);
        if (!checkStatus.exec())
        {
            qDebug() << "Ошибка выполнения запроса: " << checkStatus.lastError().text() << Qt::endl;
            return;
        }
        while (checkStatus.next())
        {
            QString stat = checkStatus.record().value(0).toString();
            if (stat == "Занята")
            {

                if (vecCallsPriority[idCal - 1] < colorsPriority::YELLOW_)
                {
                    vecCallsPriority[idCal - 1] = colorsPriority::YELLOW_;
                    mapBeginStatus[idCal - 1] = "Занята";
                }
            }
            else if (stat == "В пути")
            {

                if (vecCallsPriority[idCal - 1] < colorsPriority::BLUE_)
                {
                    vecCallsPriority[idCal - 1] = colorsPriority::BLUE_;
                    mapBeginStatus[idCal - 1] = "В пути";
                }
            }
            else if (stat == "На месте")
            {
                if (vecCallsPriority[idCal - 1] < colorsPriority::ORANGE_)
                {
                    vecCallsPriority[idCal - 1] = colorsPriority::ORANGE_;
                    mapBeginStatus[idCal - 1] = "На месте";
                }
            }
            else if (stat == "Команда выполнела задачу")
            {
                if (vecCallsPriority[idCal - 1] < colorsPriority::GREEN_)
                {
                    vecCallsPriority[idCal - 1] = colorsPriority::GREEN_;
                    mapBeginStatus[idCal - 1] = "Команда выполнела задачу";
                }
            }
            else if (stat == "Свободна")
            {
                if (vecCallsPriority[idCal - 1] < colorsPriority::GREEN_)
                {
                    vecCallsPriority[idCal - 1] = colorsPriority::GREEN_;
                    mapBeginStatus[idCal - 1] = "Свободна";
                    vecCounterForDeleteMultiCalls[idCal - 1]++;
                }
            }
        }
    }
    QSqlQuery checkUnsigned(*dataBase);
    checkUnsigned.prepare("SELECT id FROM calls WHERE status = ?");
    checkUnsigned.addBindValue("В работе");
    if (!checkUnsigned.exec())
    {
        qDebug() << "Ошибка выполнения запроса: " << checkUnsigned.lastError().text() << Qt::endl;
        return;
    }
    int idCallUnsigned = 0;
    while (checkUnsigned.next())
    {
        idCallUnsigned = checkUnsigned.record().value(0).toInt();
        checkIndexUnsignedVector[idCallUnsigned - 1] = false;
    }

    QSqlQuery checkUnsignedFinished(*dataBase);
    checkUnsignedFinished.prepare("SELECT id FROM calls WHERE status = ?");
    checkUnsignedFinished.addBindValue("Выполнен");
    if (!checkUnsignedFinished.exec())
    {
        qDebug() << "Ошибка выполнения запроса: " << checkUnsignedFinished.lastError().text() << Qt::endl;
        return;
    }
    int idCallUnsignedFinished = 0;
    while (checkUnsignedFinished.next())
    {
        idCallUnsignedFinished = checkUnsignedFinished.record().value(0).toInt();
        checkIndexUnsignedVector[idCallUnsignedFinished - 1] = false;
    }

    QSqlQuery checkUnsignedStatus(*dataBase);
    checkUnsignedStatus.prepare("SELECT id FROM calls WHERE status = ?");
    checkUnsignedStatus.addBindValue("Не назначен");
    if (!checkUnsignedStatus.exec())
    {
        qDebug() << "Ошибка выполнения запроса: " << checkUnsignedStatus.lastError().text() << Qt::endl;
        return;
    }
    int idCallStatusUn = 0;
    while (checkUnsignedStatus.next())
    {
        idCallStatusUn = checkUnsignedStatus.record().value(0).toInt();
        mapBeginStatus[idCallStatusUn - 1] = "Не назначен";
        checkIndexUnsignedVector[idCallStatusUn - 1] = true;
    }

    QSqlQuery checkfinished(*dataBase);
    checkfinished.prepare("SELECT id, incident_enum FROM calls WHERE status = ?");
    checkfinished.addBindValue("Выполнен");
    if (!checkfinished.exec())
    {
        qDebug() << "Ошибка выполнения запроса: " << checkfinished.lastError().text() << Qt::endl;
        return;
    }
    int busyCall;
    QString codeType;
    while (checkfinished.next())
    {
        busyCall = checkfinished.record().value(0).toInt();
        codeType = checkfinished.record().value(1).toString();
        busyCalls[busyCall - 1] = true;
        if (codeType == "102-103")
        {
            mapBusyCallsTypes[busyCall - 1][1] = true;
            mapBusyCallsTypes[busyCall - 1][0] = true;
        }
        else if (codeType == "103-112")
        {
            mapBusyCallsTypes[busyCall - 1][2] = true;
            mapBusyCallsTypes[busyCall - 1][1] = true;
        }
    }
    QSqlQuery query(*dataBase);
    query.prepare("SELECT id_call, id_brigade, id, flag FROM brigades_on_call WHERE flag IN (?, ?)");
    query.addBindValue(0);
    query.addBindValue(1);
    if (!query.exec())
    {
        qDebug() << "Ошибка выполнения запроса: " << query.lastError().text() << Qt::endl;
        return;
    }
    int idCall;
    int idBrigade;
    int id;
    int flagInt;
    while (query.next())
    {
        qDebug() << "Первый запуск вызовы бригады" << Qt::endl;
        idCall = query.record().value(0).toInt();
        idBrigade = query.record().value(1).toInt();
        id = query.record().value(2).toInt();
        flagInt = query.record().value(3).toInt();
        qDebug() << "Вызов: " << idCall << Qt::endl;
        qDebug() << "Бригада " << idBrigade << Qt::endl;
        QSqlQuery queryCheckType(*dataBase);
        queryCheckType.prepare("SELECT incident_enum FROM calls WHERE id = ?");
        queryCheckType.addBindValue(idCall);
        if (!queryCheckType.exec())
        {
            qDebug() << "Ошибка выполнения запроса: " << queryCheckType.lastError().text() << Qt::endl;
            return;
        }
        QString codeCall;
        while (queryCheckType.next())
        {
            codeCall = queryCheckType.record().value(0).toString();
        }
        bool flag = false;
        if (codeCall == POLICE_AND_AMBULANCE_ || codeCall == MCH_AND_AMBULANCE_)
        {
            flag = true;
            QString typeBrigade;
            QSqlQuery checkMultiBrigades(*dataBase);
            checkMultiBrigades.prepare("SELECT type FROM brigades WHERE id = ?");
            checkMultiBrigades.addBindValue(idBrigade);
            if (!checkMultiBrigades.exec())
            {
                qDebug() << "Ошибка выполнения запроса: " << checkMultiBrigades.lastError().text() << Qt::endl;
                return;
            }
            while (checkMultiBrigades.next())
            {
                typeBrigade = checkMultiBrigades.record().value(0).toString();
            }
            if (typeBrigade == "Скорая")
            {
                mapBusyCallsTypes[idCall - 1][1] = true;
                mapMultiTypesForCancel[idBrigade - 1] = AMBULANCE_;
            }
            else if (typeBrigade == "Мчс")
            {
                mapBusyCallsTypes[idCall - 1][2] = true;
                mapMultiTypesForCancel[idBrigade - 1] = MCH_;
            }
            else if (typeBrigade == "Полиция")
            {
                mapBusyCallsTypes[idCall - 1][0] = true;
                mapMultiTypesForCancel[idBrigade - 1] = POLICE_;
            }
            qDebug() << mapBusyCallsTypes[idCall - 1][0] << Qt::endl;
            qDebug() << mapBusyCallsTypes[idCall - 1][1] << Qt::endl;
            qDebug() << mapBusyCallsTypes[idCall - 1][2] << Qt::endl;
            if (mapBusyCallsTypes[idCall - 1][1] && mapBusyCallsTypes[idCall - 1][0] ||
                mapBusyCallsTypes[idCall - 1][1] && mapBusyCallsTypes[idCall - 1][2])
            {
                busyCalls[idCall - 1] = true;
                checkIndexUnsignedVector[idCall - 1] = false;
            }
            else
            {
                checkIndexUnsignedVector[idCall - 1] = true;
            }
            qDebug() << "Мульти вызов" << Qt::endl;
        }
        if (timersVector[idBrigade - 1] == nullptr)
        {
            if (flagInt != 1)
            {
                qDebug() << "Выделяем память в первом запуске под таймер: " << idBrigade - 1 << Qt::endl;
                timersVector[idBrigade - 1] = new QTimer();
                activitedTimers[idBrigade - 1] = true;
            }
            else
            {
                multiFinishedCalls[idCall - 1]++;
            }
            if (!flag)
            {
                busyCalls[idCall - 1] = true;
            }
            if (flagInt != 1)
            {
                busyBrigades[idBrigade - 1] = true;
                mapBrigadeTime[idBrigade - 1] = id - 1;
                connect(timersVector[idBrigade - 1], &QTimer::timeout, this,
                        [=]() { changeStatusLaunch(idCall - 1, idBrigade - 1, flag); });
            }
        }
    }
}

void DataBase::disconnectFromDataBase()
{
    *dataBase = QSqlDatabase::database(DB_NAME);
    qDebug() << dataBase->isOpen();
    if (callsModel)
    {
        callsModel->clear();
    }
    if (brigadesModel)
    {
        brigadesModel->clear();
    }
    if (dispatcherIdModel)
    {
        dispatcherIdModel->clear();
    }
    if (brigadesOnCallModel)
    {
        brigadesOnCallModel->clear();
    }
    dataBase->close();
}

bool DataBase::isEmptyMarks()
{
    return emptyMarks;
}

bool DataBase::isEmptyDispatchers()
{
    return emptyDispatchers;
}

QSqlError DataBase::getLastError()
{
    return dataBase->lastError();
}

bool DataBase::isOpen()
{
    return dataBase->isOpen();
}

QMap<QString, QSqlTableModel*> DataBase::setModelForTableViews()
{
    QMap<QString, QSqlTableModel*> data;
    data["calls"] = callsModel;
    data["brigades"] = brigadesModel;
    data["dispatchers"] = dispatcherIdModel;
    data["brigadesOnCall"] = brigadesOnCallModel;
    return data;
}

int DataBase::setMapDateOfCall(QMap<QString, QString> mapDateOfCall)
{
    if (!callsModel->database().isOpen())
    {
        qDebug() << "Соединение с базой данных закрыто.";
        return -1;
    }
    for (int i = 0; i < callsModel->rowCount(); i++)
    {
        QSqlRecord record = callsModel->record(i);
        if (record.value(12).toString() == mapDateOfCall["latitude"] &&
            record.value(13).toString() == mapDateOfCall["longitude"])
        {
            qDebug() << "Совпадение найдено" << Qt::endl;
            qDebug() << record.value(12).toString() << Qt::endl;
            qDebug() << mapDateOfCall["latitude"] << Qt::endl;
            qDebug() << record.value(13).toString() << Qt::endl;
            qDebug() << mapDateOfCall["longitude"] << Qt::endl;
            return -1;
        }
    }
    int row = callsModel->rowCount(); // Получаем индекс новой строки
    if (!callsModel->insertRow(row))
    {
        qDebug() << "Ошибка при добавлении строки: " << callsModel->lastError().text();
        return -1;
    }
    int requiredUnits = 0;
    if (mapDateOfCall["level"] == "Сложный")
    {
        requiredUnits = 5;
    }
    else if (mapDateOfCall["level"] == "Средний")
    {
        requiredUnits = 4;
    }
    else
    {
        requiredUnits = 3;
    }
    callsModel->setData(callsModel->index(row, callsModel->fieldIndex("incident_type")), mapDateOfCall["problem"]);
    callsModel->setData(callsModel->index(row, callsModel->fieldIndex("address")), mapDateOfCall["address"]);
    callsModel->setData(callsModel->index(row, callsModel->fieldIndex("priority")), mapDateOfCall["level"]);
    callsModel->setData(callsModel->index(row, callsModel->fieldIndex("required_units")), QVariant(requiredUnits));
    callsModel->setData(callsModel->index(row, callsModel->fieldIndex("status")), mapDateOfCall["status"]);
    callsModel->setData(callsModel->index(row, callsModel->fieldIndex("distance_km")),
                        QVariant(mapDateOfCall["distance"]));
    callsModel->setData(callsModel->index(row, callsModel->fieldIndex("region")), mapDateOfCall["region"]);
    callsModel->setData(callsModel->index(row, callsModel->fieldIndex("latitude")), mapDateOfCall["latitude"]);
    callsModel->setData(callsModel->index(row, callsModel->fieldIndex("longitude")), mapDateOfCall["longitude"]);
    callsModel->setData(callsModel->index(row, callsModel->fieldIndex("surname")), mapDateOfCall["surname"]);
    callsModel->setData(callsModel->index(row, callsModel->fieldIndex("incident_enum")),
                        mapDateOfCall["incident_enum"]);
    callsModel->setData(callsModel->index(row, callsModel->fieldIndex("phone")), mapDateOfCall["phone"]);
    if (!callsModel->submitAll())
    {
        qDebug() << "Ошибка при сохранении данных: " << callsModel->lastError().text();
        callsModel->revertAll();
        return -1;
    }
    else
    {
        qDebug() << "Данные успешно добавлены в таблицу.";
        callsModel->setSort(callsModel->fieldIndex("id"), Qt::AscendingOrder);
        callsModel->select();
    }
    return 0;
}

void DataBase::deleteCascade()
{
    QString strQuery = "TRUNCATE brigades, calls, dispatchers, brigades_on_call  RESTART IDENTITY CASCADE";
    QSqlQuery query(*dataBase);

    if (!query.exec(strQuery))
    {
        qDebug() << "Ошибка выполнения запроса:" << query.lastError().text();
    }
    else
    {
        qDebug() << "Запрос успешно выполнен!";
    }
    callsModel->setSort(callsModel->fieldIndex("id"), Qt::AscendingOrder);
    callsModel->select();
    brigadesModel->setSort(brigadesModel->fieldIndex("id"), Qt::AscendingOrder);
    brigadesModel->select();
    dispatcherIdModel->select();
    brigadesOnCallModel->setSort(brigadesOnCallModel->fieldIndex("id"), Qt::AscendingOrder);
    brigadesOnCallModel->select();
    countRowsCalls = 0;
    countRowsDispatchers = 0;
    busyCalls.clear();
    busyBrigades.clear();
    int i = 0;
    for (auto& ptr : timersVector)
    {
        if (ptr != nullptr)
        {
            delete ptr;
            qDebug() << "Таймер" << i++ << " удалён";
            ptr = nullptr;
        }
        else
        {
            qDebug() << "Таймер" << i++ << " уже удалён";
        }
    }
    timersVector.clear();
    activitedTimers.clear();
    statesBrigade.clear();
    mapBusyCallsTypes.clear();
    multiDispatcher.clear();
    multiFinishedCalls.clear();
    mapMultiTypesForCancel.clear();
    vecCallsPriority.clear();
    vecCounterForDeleteMultiCalls.clear();
    checkIndexUnsignedVector.clear();
}

int DataBase::getCountRowsCalls()
{
    return countRowsCalls;
}

void DataBase::loadCalls()
{
    QSqlQuery checkRowsCalls(*dataBase);
    if (!checkRowsCalls.exec("SELECT COUNT(*) AS RowCnt FROM calls"))
    {
        qDebug() << "Ошибка выполнения запроса SELECT COUNT: " << checkRowsCalls.lastError().text();
    }
    else
    {
        if (checkRowsCalls.next())
        {
            countRowsCalls = checkRowsCalls.value(0).toInt(); // Берем значение из первой (и единственной) строки
            if (countRowsCalls == 0)
            {
                emptyMarks = true;
            }
            else
            {
                emptyMarks = false;
            }
        }
        else
        {
            qDebug() << "Запрос выполнен, но данных нет.";
        }
    }
}

void DataBase::loadDispatchers()
{
    QSqlQuery checkRowsDispatchers(*dataBase);
    if (!checkRowsDispatchers.exec("SELECT COUNT(*) AS RowCnt FROM dispatchers"))
    {
        qDebug() << "Ошибка выполнения запроса SELECT COUNT: " << checkRowsDispatchers.lastError().text();
    }
    else
    {
        if (checkRowsDispatchers.next())
        {
            countRowsDispatchers =
                checkRowsDispatchers.value(0).toInt(); // Берем значение из первой (и единственной) строки
            if (countRowsDispatchers == 0)
            {
                emptyDispatchers = true;
            }
            else
            {
                emptyDispatchers = false;
            }
        }
        else
        {
            qDebug() << "Запрос выполнен, но данных нет.";
        }
    }
}

void DataBase::defaultSettings()
{
    if (!callsModel)
    {
        qDebug() << "Выделяем память для callModel" << Qt::endl;
        callsModel = new QSqlTableModel(nullptr, *dataBase);
        connect(callsModel, &QObject::destroyed, this, [&]() { qDebug() << "callsModel был удален"; });
    }
    callsModel->setTable("calls");
    callsModel->setSort(callsModel->fieldIndex("id"), Qt::AscendingOrder);
    if (!callsModel->select())
    {
        qDebug() << "Ошибка загрузки данных(calls):" << callsModel->lastError().text();
    }
    callsModel->setHeaderData(0, Qt::Horizontal, QObject::tr("№"));
    callsModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Происшествие"));
    callsModel->setHeaderData(2, Qt::Horizontal, QObject::tr("Статус"));
    callsModel->setHeaderData(3, Qt::Horizontal, QObject::tr("Код"));
    callsModel->setHeaderData(4, Qt::Horizontal, QObject::tr("Силы"));
    callsModel->setHeaderData(5, Qt::Horizontal, QObject::tr("Диспетчер"));
    callsModel->setHeaderData(6, Qt::Horizontal, QObject::tr("Адресс"));
    callsModel->setHeaderData(7, Qt::Horizontal, QObject::tr("Уровень"));
    callsModel->setHeaderData(8, Qt::Horizontal, QObject::tr("Регион"));
    callsModel->setHeaderData(9, Qt::Horizontal, QObject::tr("Номер"));
    callsModel->setHeaderData(10, Qt::Horizontal, QObject::tr("Поступил от"));
    callsModel->setHeaderData(11, Qt::Horizontal, QObject::tr("Расстояние"));
    callsModel->setHeaderData(12, Qt::Horizontal, QObject::tr("Широта"));
    callsModel->setHeaderData(13, Qt::Horizontal, QObject::tr("Долгота"));
    callsModel->setHeaderData(14, Qt::Horizontal, QObject::tr("Поступление вызова"));
    callsModel->setHeaderData(15, Qt::Horizontal, QObject::tr("Завершение вызова"));
    if (!brigadesModel)
    {
        qDebug() << "Выделяем память для brigadesModel" << Qt::endl;
        brigadesModel = new QSqlTableModel(nullptr, *dataBase);
        connect(brigadesModel, &QObject::destroyed, this, [&]() { qDebug() << "brigadesModel был удален"; });
    }
    brigadesModel->setTable("brigades");
    brigadesModel->setSort(brigadesModel->fieldIndex("id"), Qt::AscendingOrder);
    if (!brigadesModel->select())
    {
        qDebug() << "Ошибка загрузки данных(brigades):" << brigadesModel->lastError().text();
    }
    brigadesModel->setHeaderData(0, Qt::Horizontal, QObject::tr("№"));
    brigadesModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Тип службы"));
    brigadesModel->setHeaderData(2, Qt::Horizontal, QObject::tr("Статус"));
    brigadesModel->setHeaderData(3, Qt::Horizontal, QObject::tr("Количество персонала"));
    brigadesModel->setHeaderData(4, Qt::Horizontal, QObject::tr("Начальник"));
    brigadesModel->setHeaderData(5, Qt::Horizontal, QObject::tr("Зам. начальника"));
    brigadesModel->setHeaderData(6, Qt::Horizontal, QObject::tr("Водитель"));
    brigadesModel->setHeaderData(7, Qt::Horizontal, QObject::tr("Номер машины"));
    brigadesModel->setHeaderData(8, Qt::Horizontal, QObject::tr("Время начала работы"));
    brigadesModel->setHeaderData(9, Qt::Horizontal, QObject::tr("Время конца работы"));
    if (!dispatcherIdModel)
    {
        qDebug() << "Выделяем памят для dispatcherIdModel" << Qt::endl;
        dispatcherIdModel = new QSqlTableModel(nullptr, *dataBase);
        connect(dispatcherIdModel, &QObject::destroyed, this, [&]() { qDebug() << "dispatcherIdModel был удален"; });
    }
    dispatcherIdModel->setTable("dispatchers");
    if (!dispatcherIdModel->select())
    {
        qDebug() << "Ошибка загрузки данных(dispatchers):" << dispatcherIdModel->lastError().text();
    }
    dispatcherIdModel->setHeaderData(0, Qt::Horizontal, QObject::tr("№"));
    dispatcherIdModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Имя"));
    if (!brigadesOnCallModel)
    {
        qDebug() << "Выделяем памят для brigadesOnCallModel" << Qt::endl;
        brigadesOnCallModel = new QSqlTableModel(nullptr, *dataBase);
        connect(brigadesOnCallModel, &QObject::destroyed, this,
                [&]() { qDebug() << "brigadesOnCallModel был удален"; });
    }
    brigadesOnCallModel->setTable("brigades_on_call");
    brigadesOnCallModel->setSort(brigadesOnCallModel->fieldIndex("id"), Qt::AscendingOrder);
    if (!brigadesOnCallModel->select())
    {
        qDebug() << "Ошибка загрузки данных(brigades_on_call):" << brigadesOnCallModel->lastError().text();
    }
    brigadesOnCallModel->setHeaderData(0, Qt::Horizontal, QObject::tr("№"));
    brigadesOnCallModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Вызов"));
    brigadesOnCallModel->setHeaderData(2, Qt::Horizontal, QObject::tr("Бригада"));
    brigadesOnCallModel->setHeaderData(3, Qt::Horizontal, QObject::tr("Старт работы"));
    brigadesOnCallModel->setHeaderData(4, Qt::Horizontal, QObject::tr("Прибытие"));
    brigadesOnCallModel->setHeaderData(5, Qt::Horizontal, QObject::tr("Решение"));
    brigadesOnCallModel->setHeaderData(6, Qt::Horizontal, QObject::tr("Статус"));
    brigadesOnCallModel->setHeaderData(7, Qt::Horizontal, QObject::tr("Индификатор"));
}

void DataBase::loadBrigades()
{
    QSqlQuery checkRowsUn(*dataBase);
    if (!checkRowsUn.exec("SELECT COUNT(*) AS RowCnt FROM brigades"))
    {
        qDebug() << "Ошибка выполнения запроса SELECT COUNT: " << checkRowsUn.lastError().text();
    }
    else
    {
        if (checkRowsUn.next())
        {
            int countRowsUn = checkRowsUn.value(0).toInt(); // Берем значение из первой (и единственной) строки
            if (countRowsUn == 0)
            {
                int countAm = storageClass.Vambulance.size();
                int countRealAm = 0;
                int countMch = storageClass.Vmch.size();
                int countRealMch = 0;
                int countPolice = storageClass.Vpolice.size();
                int countRealPolice = 0;
                int countFire = storageClass.Vfire.size();
                int countRealFire = 0;
                int counter = 1;
                while (true)
                {
                    int row = brigadesModel->rowCount(); // Получаем индекс новой строки
                    if (!brigadesModel->insertRow(row))
                    {
                        qDebug() << "Ошибка при добавлении строки: " << brigadesModel->lastError().text();
                    }
                    if (counter == 1)
                    {
                        int tempCount = 0;
                        brigadesModel->setData(brigadesModel->index(row, brigadesModel->fieldIndex("type")), "Скорая");
                        brigadesModel->setData(brigadesModel->index(row, brigadesModel->fieldIndex("status")),
                                               "Свободна");
                        brigadesModel->setData(brigadesModel->index(row, brigadesModel->fieldIndex("number_car")),
                                               storageClass.Vambulance[countRealAm].at(tempCount++));
                        brigadesModel->setData(brigadesModel->index(row, brigadesModel->fieldIndex("surname_leader")),
                                               storageClass.Vambulance[countRealAm].at(tempCount++));
                        brigadesModel->setData(
                            brigadesModel->index(row, brigadesModel->fieldIndex("surname_deputy_leader")),
                            storageClass.Vambulance[countRealAm].at(tempCount++));
                        brigadesModel->setData(brigadesModel->index(row, brigadesModel->fieldIndex("surname_driver")),
                                               storageClass.Vambulance[countRealAm].at(tempCount++));
                        brigadesModel->setData(brigadesModel->index(row, brigadesModel->fieldIndex("number_of_staff")),
                                               storageClass.Vambulance[countRealAm++].at(tempCount++));
                        if (countAm == countRealAm)
                        {
                            counter++;
                        }
                    }
                    else if (counter == 2)
                    {
                        int tempCount = 0;
                        brigadesModel->setData(brigadesModel->index(row, brigadesModel->fieldIndex("type")), "Мчс");
                        brigadesModel->setData(brigadesModel->index(row, brigadesModel->fieldIndex("status")),
                                               "Свободна");
                        brigadesModel->setData(brigadesModel->index(row, brigadesModel->fieldIndex("number_car")),
                                               storageClass.Vmch[countRealMch].at(tempCount++));
                        brigadesModel->setData(brigadesModel->index(row, brigadesModel->fieldIndex("surname_leader")),
                                               storageClass.Vmch[countRealMch].at(tempCount++));
                        brigadesModel->setData(
                            brigadesModel->index(row, brigadesModel->fieldIndex("surname_deputy_leader")),
                            storageClass.Vmch[countRealMch].at(tempCount++));
                        brigadesModel->setData(brigadesModel->index(row, brigadesModel->fieldIndex("surname_driver")),
                                               storageClass.Vmch[countRealMch].at(tempCount++));
                        brigadesModel->setData(brigadesModel->index(row, brigadesModel->fieldIndex("number_of_staff")),
                                               storageClass.Vmch[countRealMch++].at(tempCount++));
                        if (countMch == countRealMch)
                        {
                            counter++;
                        }
                    }
                    else if (counter == 3)
                    {
                        int tempCount = 0;
                        brigadesModel->setData(brigadesModel->index(row, brigadesModel->fieldIndex("type")), "Полиция");
                        brigadesModel->setData(brigadesModel->index(row, brigadesModel->fieldIndex("status")),
                                               "Свободна");
                        brigadesModel->setData(brigadesModel->index(row, brigadesModel->fieldIndex("number_car")),
                                               storageClass.Vpolice[countRealPolice].at(tempCount++));
                        brigadesModel->setData(brigadesModel->index(row, brigadesModel->fieldIndex("surname_leader")),
                                               storageClass.Vpolice[countRealPolice].at(tempCount++));
                        brigadesModel->setData(
                            brigadesModel->index(row, brigadesModel->fieldIndex("surname_deputy_leader")),
                            storageClass.Vpolice[countRealPolice].at(tempCount++));
                        brigadesModel->setData(brigadesModel->index(row, brigadesModel->fieldIndex("surname_driver")),
                                               storageClass.Vpolice[countRealPolice].at(tempCount++));
                        brigadesModel->setData(brigadesModel->index(row, brigadesModel->fieldIndex("number_of_staff")),
                                               storageClass.Vpolice[countRealPolice++].at(tempCount++));
                        if (countPolice == countRealPolice)
                        {
                            counter++;
                        }
                    }
                    else if (counter == 4)
                    {
                        int tempCount = 0;
                        brigadesModel->setData(brigadesModel->index(row, brigadesModel->fieldIndex("type")),
                                               "Пожарные");
                        brigadesModel->setData(brigadesModel->index(row, brigadesModel->fieldIndex("status")),
                                               "Свободна");
                        brigadesModel->setData(brigadesModel->index(row, brigadesModel->fieldIndex("number_car")),
                                               storageClass.Vfire[countRealFire].at(tempCount++));
                        brigadesModel->setData(brigadesModel->index(row, brigadesModel->fieldIndex("surname_leader")),
                                               storageClass.Vfire[countRealFire].at(tempCount++));
                        brigadesModel->setData(
                            brigadesModel->index(row, brigadesModel->fieldIndex("surname_deputy_leader")),
                            storageClass.Vfire[countRealFire].at(tempCount++));
                        brigadesModel->setData(brigadesModel->index(row, brigadesModel->fieldIndex("surname_driver")),
                                               storageClass.Vfire[countRealFire].at(tempCount++));
                        brigadesModel->setData(brigadesModel->index(row, brigadesModel->fieldIndex("number_of_staff")),
                                               storageClass.Vfire[countRealFire++].at(tempCount++));
                        if (countFire == countRealFire)
                        {
                            counter++;
                        }
                    }
                    if (!brigadesModel->submitAll())
                    {
                        qDebug() << "Ошибка при сохранении данных: " << brigadesModel->lastError().text();
                        brigadesModel->revertAll();
                    }
                    else
                    {
                        qDebug() << "Данные успешно добавлены в таблицу.";
                        brigadesModel->select();
                    }

                    if (counter == 5)
                    {
                        break;
                    }
                }
            }
        }
    }
}

void DataBase::addDispatcher(QString name)
{

    if (!dispatcherIdModel->database().isOpen())
    {
        qDebug() << "Соединение с базой данных закрыто.";
        return;
    }
    int row = dispatcherIdModel->rowCount(); // Получаем индекс новой строки
    if (!dispatcherIdModel->insertRow(row))
    {
        qDebug() << "Ошибка при добавлении строки: " << dispatcherIdModel->lastError().text();
        return;
    }
    dispatcherIdModel->setData(dispatcherIdModel->index(row, dispatcherIdModel->fieldIndex("name")), name);
    if (!dispatcherIdModel->submitAll())
    {
        qDebug() << "Ошибка при сохранении данных: " << dispatcherIdModel->lastError().text();
        dispatcherIdModel->revertAll();
    }
    else
    {
        qDebug() << "Данные успешно добавлены в таблицу.";
        dispatcherIdModel->select();
    }
}

void DataBase::checkIndexs(int Call, int Brigade)
{
    if (!dataBase->isOpen())
    {
        qDebug() << "Соединение с базой данной закрыто!" << Qt::endl;
        return;
    }
    QString type = "Неверный индекс";
    QString unitsBrigade;
    QSqlQuery codeBrigade(*dataBase);
    codeBrigade.prepare("SELECT type, number_of_staff FROM brigades WHERE id = ?");
    codeBrigade.addBindValue(Brigade);
    if (!codeBrigade.exec())
    {
        qDebug() << "Ошибка выполнения запроса SELECT: " << codeBrigade.lastError().text();
        return;
    }
    while (codeBrigade.next())
    {
        type = codeBrigade.value(0).toString();
        qDebug() << "Тип:" << type;
        unitsBrigade = codeBrigade.value(1).toString();
        qDebug() << "Юниты в бригаде:" << unitsBrigade;
    }
    QString code;
    QString unitsCall;
    QSqlQuery codeCall(*dataBase);
    codeCall.prepare("SELECT incident_enum, required_units FROM calls WHERE id = ?");
    codeCall.addBindValue(Call);
    if (!codeCall.exec())
    {
        qDebug() << "Ошибка выполнения запроса SELECT: " << codeCall.lastError().text();
        return;
    }
    while (codeCall.next())
    {
        code = codeCall.value(0).toString();
        qDebug() << "Код вызова:" << code;
        unitsCall = codeCall.value(1).toString();
        qDebug() << "Юниты в вызове:" << unitsCall;
    }
    int size = callsModel->rowCount();
    if (busyBrigades.size() == 0)
    {
        busyBrigades.resize(sizeBrigades);
    }
    if (busyCalls.size() == 0 || Call >= busyCalls.size())
    {
        busyCalls.resize(size, false);
    }
    if (mapBusyCallsTypes.size() == 0 || Call >= mapBusyCallsTypes.size())
    {
        for (int i = 0; i < size; i++)
        {
            mapBusyCallsTypes[i].resize(3, false);
        }
    }
    if (vecCallsPriority.size() == 0 || Call >= vecCallsPriority.size())
    {
        vecCallsPriority.resize(size, colorsPriority::NOT_COLOR);
    }
    if (checkIndexUnsignedVector.size() == 0 || Call >= checkIndexUnsignedVector.size())
    {
        checkIndexUnsignedVector.resize(size, true);
    }
    if (multiFinishedCalls.size() == 0 || Call >= multiFinishedCalls.size())
    {
        multiFinishedCalls.resize(size, 1);
    }
    if (multiDispatcher.size() == 0 || Call >= multiDispatcher.size())
    {
        multiDispatcher.resize(size, false);
    }
    if (vecCounterForDeleteMultiCalls.size() == 0 || Call >= vecCounterForDeleteMultiCalls.size())
    {
        vecCounterForDeleteMultiCalls.resize(size, 1);
    }
    if (timersVector.size() == 0)
    {
        activitedTimers.resize(sizeBrigades);
        timersVector.resize(sizeBrigades);
        for (int i = 0; i < sizeBrigades; i++)
        {
            timersVector[i] = nullptr;
            activitedTimers[i] = false;
        }
    }
    if (statesBrigade.size() == 0)
    {
        statesBrigade.resize(sizeBrigades);
        for (int i = 0; i < sizeBrigades; i++)
        {
            statesBrigade[i] << (QStringList() << "Занята" << "В пути" << "На месте" << "Команда выполнела задачу");
        }
    }
    QString strCode = code;
    bool flagTwo = false;
    if (strCode == POLICE_AND_AMBULANCE_ || strCode == MCH_AND_AMBULANCE_)
    {
        QSqlQuery stat(*dataBase);
        stat.prepare("Select status from calls where id = ?");
        stat.addBindValue(Call);
        if (!stat.exec())
        {
            qDebug() << "Ошибка выполнения запроса SELECT: " << stat.lastError().text();
            return;
        }
        QString status;
        while (stat.next())
        {
            status = stat.record().value(0).toString();
        }
        if (status == "Не назначен")
        {
            qDebug() << "Упс время назначения на вызов уже истекло" << Qt::endl;
            msg->setWindowTitle("Упс, время истекло.....");
            msg->setText("Вы не успели назначить на вызов достаточное количество бригад");
            msg->setIconPixmap(QPixmap(":/Images/iconTimeout.png"));
            msg->exec();
            return;
        }

        qDebug() << status << Qt::endl;

        flagTwo = true;
        if (strCode == POLICE_AND_AMBULANCE_)
        {
            if (mapTypeCode[type] == POLICE_)
            {
                if (!mapBusyCallsTypes[Call - 1][0] && busyCalls[Call - 1] == false)
                {
                    if (unitsCall.toInt() != unitsBrigade.toInt())
                    {
                        msg->setWindowTitle("Количество персонала не подходит");
                        msg->setText("Количество которое требуется:     " + unitsCall +
                                     "     Твоё количество:     " + unitsBrigade);
                        msg->setIconPixmap(QPixmap(":/Images/iconSign.png"));
                        msg->exec();
                        return;
                    }
                }
                if (!mapBusyCallsTypes[Call - 1][0])
                {
                    if (busyBrigades[Brigade - 1] == true)
                    {
                        qDebug() << "Эта бригада уже занята" << Qt::endl;
                        msg->setWindowTitle("Бригада в работе");
                        msg->setText("Эта бригада уже занята");
                        msg->setIconPixmap(QPixmap(":/Images/iconSign.png"));
                        msg->exec();
                    }
                    else
                    {
                        if (timersVector[Brigade - 1] == nullptr)
                        {
                            timersVector[Brigade - 1] = new QTimer();
                            qDebug() << "Выделяем память для таймера: " << Brigade - 1 << Qt::endl;
                            connect(timersVector[Brigade - 1], &QTimer::timeout, this,
                                    [=]() { changeStatus(Call - 1, Brigade - 1, true); });
                        }
                        else
                        {
                            disconnect(timersVector[Brigade - 1], nullptr, nullptr, nullptr);
                            connect(timersVector[Brigade - 1], &QTimer::timeout, this,
                                    [=]() { changeStatus(Call - 1, Brigade - 1, true); });
                            qDebug() << "Память для тамера уже выделена, просто запускаем1" << Qt::endl;
                        }
                        mapBrigadeStatus[Brigade - 1] = 0;
                        busyBrigades[Brigade - 1] = true;
                        mapBusyCallsTypes[Call - 1][0] = true;
                        mapMultiTypesForCancel[Brigade - 1] = POLICE_;
                        if (multiDispatcher[Call - 1] == false)
                        {
                            multiDispatcher[Call - 1] = true;
                            changeStatusCallMulti(Call - 1, Brigade - 1);
                        }
                        changeStatusBrigade(Call - 1, Brigade - 1, true);

                        timersVector[Brigade - 1]->start(TIMER);
                    }
                }
                else
                {
                    if (mapBusyCallsTypes[Call - 1][0] == true && mapBusyCallsTypes[Call - 1][1] == false)
                    {
                        QString recBrigade;
                        for (auto it = mapTypeCode.begin(); it != mapTypeCode.end(); ++it)
                        {
                            if (strCode == it.value())
                            {
                                recBrigade = it.key();
                            }
                        }
                        msg->setWindowTitle("Ты уже назначил полицию");
                        msg->setText("Осталось назначить скорую");
                        msg->setIconPixmap(QPixmap(":/Images/iconSign.png"));
                        msg->exec();
                    }
                }
            }
            else if (mapTypeCode[type] == AMBULANCE_)
            {
                if (!mapBusyCallsTypes[Call - 1][1] && busyCalls[Call - 1] == false)
                {
                    if (unitsCall.toInt() != unitsBrigade.toInt())
                    {
                        msg->setWindowTitle("Количество персонала не подходит");
                        msg->setText("Количество которое требуется:     " + unitsCall +
                                     "     Твоё количество:     " + unitsBrigade);
                        msg->setIconPixmap(QPixmap(":/Images/iconSign.png"));
                        msg->exec();
                        return;
                    }
                }
                if (!mapBusyCallsTypes[Call - 1][1])
                {
                    if (busyBrigades[Brigade - 1] == true)
                    {
                        qDebug() << "Эта бригада уже занята" << Qt::endl;
                        msg->setWindowTitle("Бригада в работе");
                        msg->setText("Эта бригада уже занята");
                        msg->setIconPixmap(QPixmap(":/Images/iconSign.png"));
                        msg->exec();
                    }
                    else
                    {
                        if (timersVector[Brigade - 1] == nullptr)
                        {
                            timersVector[Brigade - 1] = new QTimer();
                            qDebug() << "Выделяем память для таймера: " << Brigade - 1 << Qt::endl;
                            connect(timersVector[Brigade - 1], &QTimer::timeout, this,
                                    [=]() { changeStatus(Call - 1, Brigade - 1, true); });
                        }
                        else
                        {
                            disconnect(timersVector[Brigade - 1], nullptr, nullptr, nullptr);
                            connect(timersVector[Brigade - 1], &QTimer::timeout, this,
                                    [=]() { changeStatus(Call - 1, Brigade - 1, true); });
                            qDebug() << "Память для тамера уже выделена, просто запускаем1" << Qt::endl;
                        }
                        mapBrigadeStatus[Brigade - 1] = 0;
                        busyBrigades[Brigade - 1] = true;
                        mapBusyCallsTypes[Call - 1][1] = true;
                        mapMultiTypesForCancel[Brigade - 1] = AMBULANCE_;
                        if (multiDispatcher[Call - 1] == false)
                        {
                            multiDispatcher[Call - 1] = true;
                            changeStatusCallMulti(Call - 1, Brigade - 1);
                        }
                        changeStatusBrigade(Call - 1, Brigade - 1, true);
                        timersVector[Brigade - 1]->start(TIMER);
                    }
                }
                else
                {
                    if (mapBusyCallsTypes[Call - 1][1] == true && mapBusyCallsTypes[Call - 1][0] == false)
                    {
                        QString recBrigade;
                        for (auto it = mapTypeCode.begin(); it != mapTypeCode.end(); ++it)
                        {
                            if (strCode == it.value())
                            {
                                recBrigade = it.key();
                            }
                        }
                        msg->setWindowTitle("Ты уже назначил скорую");
                        msg->setText("Осталось назначить полицию");
                        msg->setIconPixmap(QPixmap(":/Images/iconSign.png"));
                        msg->exec();
                    }
                }
            }
            else
            {
                QString recBrigade;
                for (auto it = mapTypeCode.begin(); it != mapTypeCode.end(); ++it)
                {
                    if (strCode == it.value())
                    {
                        recBrigade = it.key();
                    }
                }
                msg->setWindowTitle("Код вызова не соответствует с бригадой");
                msg->setText("Твой код вызова:     " + code + "     Назначенная бригада:     " + type +
                             "\n\nРекомендуемая бригада:     " + recBrigade);
                msg->setIconPixmap(QPixmap(":/Images/iconSign.png"));
                msg->exec();
                return;
            }
            if (busyCalls[Call - 1] == true)
            {
                if (status == "В работе")
                {
                    qDebug() << "На этот вызов уже назначена бригада" << Qt::endl;
                    msg->setWindowTitle("Вызов в работе");
                    msg->setIconPixmap(QPixmap(":/Images/iconSign.png"));
                    msg->setText("На этот вызов уже назначены бригады");
                    msg->exec();
                }
                else if (status == "Выполнен")
                {
                    qDebug() << "Этот вызов уже выполнен" << Qt::endl;
                    msg->setWindowTitle("Вызов решен");
                    msg->setIconPixmap(QPixmap(":/Images/iconGalo4ka.png"));
                    msg->setText("Этот вызов уже выполнен");
                    msg->exec();
                }
                return;
            }
            if (mapBusyCallsTypes[Call - 1][1] && mapBusyCallsTypes[Call - 1][0])
            {
                busyCalls[Call - 1] = true;
                checkIndexUnsignedVector[Call - 1] = false;
            }
        }
        else if (strCode == MCH_AND_AMBULANCE_)
        {
            if (!mapBusyCallsTypes[Call - 1][2] && busyCalls[Call - 1] == false)
            {
                if (unitsCall.toInt() != unitsBrigade.toInt())
                {
                    msg->setWindowTitle("Количество персонала не подходит");
                    msg->setText("Количество которое требуется:     " + unitsCall +
                                 "     Твоё количество:     " + unitsBrigade);
                    msg->setIconPixmap(QPixmap(":/Images/iconSign.png"));
                    msg->exec();
                    return;
                }
            }
            if (mapTypeCode[type] == MCH_)
            {
                if (!mapBusyCallsTypes[Call - 1][2])
                {
                    if (busyBrigades[Brigade - 1] == true)
                    {
                        qDebug() << "Эта бригада уже занята" << Qt::endl;
                        msg->setWindowTitle("Бригада в работе");
                        msg->setText("Эта бригада уже занята");
                        msg->setIconPixmap(QPixmap(":/Images/iconSign.png"));
                        msg->exec();
                    }
                    else
                    {
                        if (timersVector[Brigade - 1] == nullptr)
                        {
                            timersVector[Brigade - 1] = new QTimer();
                            qDebug() << "Выделяем память для таймера: " << Brigade - 1 << Qt::endl;
                            connect(timersVector[Brigade - 1], &QTimer::timeout, this,
                                    [=]() { changeStatus(Call - 1, Brigade - 1, true); });
                        }
                        else
                        {
                            disconnect(timersVector[Brigade - 1], nullptr, nullptr, nullptr);
                            connect(timersVector[Brigade - 1], &QTimer::timeout, this,
                                    [=]() { changeStatus(Call - 1, Brigade - 1, true); });
                            qDebug() << "Память для тамера уже выделена, просто запускаем1" << Qt::endl;
                        }

                        mapBrigadeStatus[Brigade - 1] = 0;
                        busyBrigades[Brigade - 1] = true;
                        mapBusyCallsTypes[Call - 1][2] = true;
                        mapMultiTypesForCancel[Brigade - 1] = MCH_;
                        if (multiDispatcher[Call - 1] == false)
                        {
                            multiDispatcher[Call - 1] = true;
                            changeStatusCallMulti(Call - 1, Brigade - 1);
                        }
                        changeStatusBrigade(Call - 1, Brigade - 1, true);
                        timersVector[Brigade - 1]->start(TIMER);
                    }
                }
                else
                {
                    if (mapBusyCallsTypes[Call - 1][2] == true && mapBusyCallsTypes[Call - 1][1] == false)
                    {
                        QString recBrigade;
                        for (auto it = mapTypeCode.begin(); it != mapTypeCode.end(); ++it)
                        {
                            if (strCode == it.value())
                            {
                                recBrigade = it.key();
                            }
                        }
                        msg->setWindowTitle("Ты уже назначил мчс");
                        msg->setText("Осталось назначить скорую");
                        msg->setIconPixmap(QPixmap(":/Images/iconSign.png"));
                        msg->exec();
                        return;
                    }
                }
            }
            else if (mapTypeCode[type] == AMBULANCE_)
            {
                if (!mapBusyCallsTypes[Call - 1][1] && busyCalls[Call - 1] == false)
                {
                    if (unitsCall.toInt() != unitsBrigade.toInt())
                    {
                        msg->setWindowTitle("Количество персонала не подходит");
                        msg->setText("Количество которое требуется:     " + unitsCall +
                                     "     Твоё количество:     " + unitsBrigade);
                        msg->setIconPixmap(QPixmap(":/Images/iconSign.png"));
                        msg->exec();
                        return;
                    }
                }
                if (!mapBusyCallsTypes[Call - 1][1])
                {
                    if (busyBrigades[Brigade - 1] == true)
                    {
                        qDebug() << "Эта бригада уже занята" << Qt::endl;
                        msg->setWindowTitle("Бригада в работе");
                        msg->setIconPixmap(QPixmap(":/Images/iconSign.png"));
                        msg->setText("Эта бригада уже занята");
                        msg->exec();
                    }
                    else
                    {
                        if (timersVector[Brigade - 1] == nullptr)
                        {
                            timersVector[Brigade - 1] = new QTimer();
                            qDebug() << "Выделяем память для таймера: " << Brigade - 1 << Qt::endl;
                            connect(timersVector[Brigade - 1], &QTimer::timeout, this,
                                    [=]() { changeStatus(Call - 1, Brigade - 1, true); });
                        }
                        else
                        {
                            disconnect(timersVector[Brigade - 1], nullptr, nullptr, nullptr);
                            connect(timersVector[Brigade - 1], &QTimer::timeout, this,
                                    [=]() { changeStatus(Call - 1, Brigade - 1, true); });
                            qDebug() << "Память для тамера уже выделена, просто запускаем1" << Qt::endl;
                        }
                        mapBrigadeStatus[Brigade - 1] = 0;
                        busyBrigades[Brigade - 1] = true;
                        mapBusyCallsTypes[Call - 1][1] = true;
                        mapMultiTypesForCancel[Brigade - 1] = AMBULANCE_;
                        if (multiDispatcher[Call - 1] == false)
                        {
                            multiDispatcher[Call - 1] = true;
                            changeStatusCallMulti(Call - 1, Brigade - 1);
                        }
                        changeStatusBrigade(Call - 1, Brigade - 1, true);
                        timersVector[Brigade - 1]->start(TIMER);
                    }
                }
                else
                {
                    if (mapBusyCallsTypes[Call - 1][1] == true && mapBusyCallsTypes[Call - 1][2] == false)
                    {
                        QString recBrigade;
                        for (auto it = mapTypeCode.begin(); it != mapTypeCode.end(); ++it)
                        {
                            if (strCode == it.value())
                            {
                                recBrigade = it.key();
                            }
                        }
                        msg->setWindowTitle("Ты уже назначил скорую");
                        msg->setIconPixmap(QPixmap(":/Images/iconSign.png"));
                        msg->setText("Осталось назначить мчс");
                        msg->exec();
                    }
                }
            }
            else
            {
                QString recBrigade;
                for (auto it = mapTypeCode.begin(); it != mapTypeCode.end(); ++it)
                {
                    if (strCode == it.value())
                    {
                        recBrigade = it.key();
                    }
                }
                msg->setWindowTitle("Код вызова не соответствует с бригадой");
                msg->setText("Твой код вызова:     " + code + "     Назначенная бригада:     " + type +
                             "\n\nРекомендуемая бригада:     " + recBrigade);
                msg->setIconPixmap(QPixmap(":/Images/iconSign.png"));
                msg->exec();
                return;
            }
            if (busyCalls[Call - 1] == true)
            {
                if (status == "В работе")
                {
                    qDebug() << "На этот вызов уже назначена бригада" << Qt::endl;
                    msg->setWindowTitle("Вызов в работе");
                    msg->setIconPixmap(QPixmap(":/Images/iconSign.png"));
                    msg->setText("На этот вызов уже назначены бригады");
                    msg->exec();
                }
                else if (status == "Выполнен")
                {
                    qDebug() << "Этот вызов уже выполнен" << Qt::endl;
                    msg->setWindowTitle("Вызов решен");
                    msg->setIconPixmap(QPixmap(":/Images/iconGalo4ka.png"));
                    msg->setText("Этот вызов уже выполнен");
                    msg->exec();
                }
                return;
            }
            if (mapBusyCallsTypes[Call - 1][1] && mapBusyCallsTypes[Call - 1][2])
            {
                busyCalls[Call - 1] = true;
                checkIndexUnsignedVector[Call - 1] = false;
            }
        }
    }
    if (!flagTwo)
    {
        QSqlQuery stat(*dataBase);
        stat.prepare("Select status from calls where id = ?");
        stat.addBindValue(Call);
        if (!stat.exec())
        {
            qDebug() << "Ошибка выполнения запроса SELECT: " << stat.lastError().text();
            return;
        }
        QString status;
        while (stat.next())
        {
            status = stat.record().value(0).toString();
        }

        if (status == "Не назначен")
        {
            qDebug() << "Упс время назначения на вызов уже истекло" << Qt::endl;
            msg->setWindowTitle("Упс, время истекло.....");
            msg->setText("Вы не успели назначить на вызов бригаду");
            msg->setIconPixmap(QPixmap(":/Images/iconTimeout.png"));
            msg->exec();
            return;
        }

        if (strCode == mapTypeCode[type])
        {
            if (busyCalls[Call - 1] == false && busyBrigades[Brigade - 1] == false)
            {
                if (unitsCall.toInt() != unitsBrigade.toInt())
                {
                    msg->setWindowTitle("Количество персонала не подходит");
                    msg->setText("Количество которое требуется:     " + unitsCall +
                                 "     Твоё количество:     " + unitsBrigade);
                    msg->exec();
                    return;
                }
            }
            bool flag = false;
            if (busyCalls[Call - 1] == true)
            {
                flag = true;
                qDebug() << status << Qt::endl;
                if (status == "В работе")
                {
                    qDebug() << "На этот вызов уже назначена бригада" << Qt::endl;
                    msg->setWindowTitle("Вызов в работе");
                    msg->setIconPixmap(QPixmap(":/Images/iconSign.png"));
                    msg->setText("На этот вызов уже назначена бригада");
                    msg->exec();
                }
                else if (status == "Выполнен")
                {
                    qDebug() << "Этот вызов уже выполнен" << Qt::endl;
                    msg->setWindowTitle("Вызов решен");
                    msg->setIconPixmap(QPixmap(":/Images/iconGalo4ka.png"));
                    msg->setText("Этот вызов уже выполнен");
                    msg->exec();
                }
            }
            else
            {
                if (busyBrigades[Brigade - 1] != true)
                {
                    busyCalls[Call - 1] = true;
                }
            }
            if (!flag)
            {
                if (busyBrigades[Brigade - 1] == true)
                {
                    qDebug() << "Эта бригада уже занята" << Qt::endl;
                    msg->setWindowTitle("Бригада в работе");
                    msg->setText("Эта бригада уже занята");
                    msg->exec();
                }
                else
                {
                    busyBrigades[Brigade - 1] = true;
                    checkIndexUnsignedVector[Call - 1] = false;
                    if (timersVector[Brigade - 1] == nullptr)
                    {
                        timersVector[Brigade - 1] = new QTimer();
                        qDebug() << "Выделяем память для таймера: " << Brigade - 1 << Qt::endl;
                        connect(timersVector[Brigade - 1], &QTimer::timeout, this,
                                [=]() { changeStatus(Call - 1, Brigade - 1, false); });
                    }
                    else
                    {
                        disconnect(timersVector[Brigade - 1], nullptr, nullptr, nullptr);
                        connect(timersVector[Brigade - 1], &QTimer::timeout, this,
                                [=]() { changeStatus(Call - 1, Brigade - 1, false); });
                        qDebug() << "Память для тамера уже выделена, просто запускаем" << Qt::endl;
                    }
                    mapBrigadeStatus[Brigade - 1] = 0;
                    changeStatus(Call - 1, Brigade - 1, false);
                    timersVector[Brigade - 1]->start(TIMER);
                }
            }
        }
        else
        {
            QString recBrigade;
            for (auto it = mapTypeCode.begin(); it != mapTypeCode.end(); ++it)
            {
                if (strCode == it.value())
                {
                    recBrigade = it.key();
                }
            }
            msg->setWindowTitle("Код вызова не соответствует с бригадой");
            msg->setText("Твой код вызова:     " + code + "     Назначенная бригада:     " + type +
                         "\n\nРекомендуемая бригада:     " + recBrigade);
            msg->exec();
        }
    }
}

bool DataBase::getCheckIndexRight(int idCall)
{
    return checkIndexUnsignedVector[idCall];
}

void DataBase::setTimerCallFail(int idCall)
{
    qDebug() << "Айди вызова: " << idCall << Qt::endl;
    int size = callsModel->rowCount();
    qDebug() << "Индекс: " << idCall << Qt::endl;

    if (checkIndexUnsignedVector.size() == 0 || idCall >= checkIndexUnsignedVector.size())
    {
        checkIndexUnsignedVector.resize(size, true);
    }
    qDebug() << "Размер: " << size << Qt::endl;
    checkIndexUnsignedVector[idCall - 1] = true;
    QSqlQuery query(*dataBase);
    query.prepare("SELECT update_call_status_timeout(:id)");
    query.bindValue(":id", idCall);
    if (!query.exec())
    {
        qDebug() << "Ошибка в запросе: " << query.lastError().text() << Qt::endl;
        return;
    }
    callsModel->setSort(callsModel->fieldIndex("id"), Qt::AscendingOrder);
    callsModel->select();
    cancelBrigadeWhenUnsignedMulti(idCall);
}

void DataBase::changeStatus(int Call, int Brigade, bool flag)
{
    if (!dataBase->isOpen())
    {
        qDebug() << "Соединение с базой данной закрыто!" << Qt::endl;
        return;
    }
    qDebug() << "changeStatus" << Qt::endl;
    if (!flag)
    {
        changeStatusCall(Call, Brigade);
    }
    else
    {
        changeStatusCallMulti(Call, Brigade);
    }
    changeStatusBrigade(Call, Brigade, flag);
}

void DataBase::changeStatusLaunch(int Call, int Brigade, bool flag)
{
    if (!dataBase->isOpen())
    {
        qDebug() << "Соединение с базой данной закрыто!" << Qt::endl;
        return;
    }
    qDebug() << "changeStatusLaunch" << Qt::endl;
    if (checkStatusVector[Brigade] == false)
    {
        QSqlQuery query(*dataBase);
        query.prepare("SELECT status FROM brigades WHERE id =?");
        query.addBindValue(Brigade + 1);
        if (!query.exec())
        {
            qDebug() << "Ошибка в запросе: " << query.lastError().text() << Qt::endl;
            return;
        }
        QString status;
        while (query.next())
        {
            status = query.record().value(0).toString();
        }
        if (status == "Занята")
        {
            mapBrigadeStatus[Brigade] = 0;
        }
        else if (status == "В пути")
        {
            mapBrigadeStatus[Brigade] = 1;
        }
        else if (status == "На месте")
        {
            mapBrigadeStatus[Brigade] = 2;
        }
        else if (status == "Команда выполнела задачу")
        {
            mapBrigadeStatus[Brigade] = 3;
        }
        checkStatusVector[Brigade] = true;
    }

    if (!flag)
    {
        changeStatusCallLaunch(Call, Brigade);
    }
    else
    {
        changeStatusCallMultiLaunch(Call, Brigade);
    }
    changeStatusBrigadeLaunch(Call, Brigade, flag);
}

void DataBase::setCurrentDispatcher(QString name)
{
    currentUser = name;
}

QString DataBase::cancelButton(int numCall)
{
    QSqlQuery queryCalls(*dataBase);
    queryCalls.prepare("SELECT status FROM calls WHERE id = ?");
    queryCalls.addBindValue(numCall);
    if (!queryCalls.exec())
    {
        qDebug() << "Ошибка выполнения запроса SELECT: " << queryCalls.lastError().text();
        return "Ошибка, подробнее в отладке";
    }
    QString status;
    while (queryCalls.next())
    {
        status = queryCalls.record().value(0).toString();
    }
    if (status == "Принят")
    {
        return QString("Ваш вызов и так свободен");
    }
    else if (status == "Выполнен")
    {
        return QString("Ваш вызов уже выполнен");
    }
    else if (status == "Не назначен")
    {
        return QString("Время назначения на вызов уже истекло");
    }
    QString statWork = "В работе";
    QSqlQuery queryBrigadesOnCall2(*dataBase);
    queryBrigadesOnCall2.prepare("SELECT id_brigade FROM brigades_on_call WHERE id_call = ? AND status = ?");
    queryBrigadesOnCall2.addBindValue(numCall);
    queryBrigadesOnCall2.addBindValue(statWork);
    if (!queryBrigadesOnCall2.exec())
    {
        qDebug() << "Ошибка выполнения запроса SELECT: " << queryBrigadesOnCall2.lastError().text();
        return "Ошибка, подробнее в отладке";
    }

    int idBrigadeNum;
    int index = 1;
    QVector<int> nums;
    bool flag = false;
    QString statCanlcel = "Отмена";
    while (queryBrigadesOnCall2.next())
    {
        flag = true;
        idBrigadeNum = queryBrigadesOnCall2.record().value(0).toInt();
        qDebug() << "№ " << index++;
        qDebug() << "Бригада номер: " << idBrigadeNum << Qt::endl;
        QSqlQuery queryBrigadesOnCall3(*dataBase);
        queryBrigadesOnCall3.prepare("SELECT update_brigades_on_set_flag(:id, :flag)");
        queryBrigadesOnCall3.bindValue(":id", idBrigadeNum);
        queryBrigadesOnCall3.bindValue(":flag", 2);
        if (!queryBrigadesOnCall3.exec())
        {
            qDebug() << "Ошибка выполнения запроса SELECT: " << queryBrigadesOnCall3.lastError().text();
            return "Ошибка, подробнее в отладке";
        }
        QSqlQuery queryBrigadesOnCall1(*dataBase);
        queryBrigadesOnCall1.prepare("SELECT update_brigades_on_cancel_status(:id, :status)");
        queryBrigadesOnCall1.bindValue(":id", idBrigadeNum);
        queryBrigadesOnCall1.bindValue(":status", statCanlcel);
        if (!queryBrigadesOnCall1.exec())
        {
            qDebug() << "Ошибка выполнения запроса UPDATE: " << queryBrigadesOnCall1.lastError().text();
            return "Ошибка, подробнее в отладке";
        }
        nums.push_back(idBrigadeNum - 1);
    }
    brigadesOnCallModel->setSort(brigadesOnCallModel->fieldIndex("id"), Qt::AscendingOrder);
    brigadesOnCallModel->select();
    if (!flag)
    {
        return QString("Бригады уже свободны");
    }

    setFree(numCall - 1, nums);
    return QString("Вызов - " + QString::number(numCall) + " снят с задачи и бригада(ы) c ним");
}

void DataBase::setFree(int Call, QVector<int> nums)
{
    int size = nums.size();
    bool multi = false;
    if (size > 1)
    {
        qDebug() << "Multi" << Qt::endl;
    }
    for (int i = 0; i < size; i++)
    {
        int brigade = nums[i];
        QString status = "Свободна";
        QSqlQuery queryStatus(*dataBase);
        queryStatus.prepare("SELECT update_brigade_status(:id, :status)");
        queryStatus.bindValue(":id", brigade + 1);
        queryStatus.bindValue(":status", status);
        if (!queryStatus.exec())
        {
            qDebug() << "Ошибка выполнения запроса UPDATE: " << queryStatus.lastError().text();
            return;
        }
        QSqlQuery querySetStartTimeNull(*dataBase);
        querySetStartTimeNull.prepare("SELECT update_brigade_start_time_set_null(:id)");
        querySetStartTimeNull.bindValue(":id", brigade + 1);
        if (!querySetStartTimeNull.exec())
        {
            qDebug() << "Ошибка выполнения запроса UPDATE: " << querySetStartTimeNull.lastError().text();
            return;
        }

        if (i == 0)
        {
            QSqlQuery queryCallDispatcher(*dataBase);
            queryCallDispatcher.prepare("SELECT update_call_dispatcher_status(:id)");
            queryCallDispatcher.bindValue(":id", Call + 1);
            if (!queryCallDispatcher.exec())
            {
                qDebug() << "Ошибка выполнения запроса UPDATE: " << queryCallDispatcher.lastError().text();
                return;
            }
            busyCalls[Call] = false;
            qDebug() << "Вызов номер: " << Call + 1 << "стал принятым";

            multiDispatcher[Call] = false;
        }
        if (mapMultiTypesForCancel[brigade] == AMBULANCE_)
        {
            mapBusyCallsTypes[Call][1] = false;
        }
        else if (mapMultiTypesForCancel[brigade] == POLICE_)
        {
            mapBusyCallsTypes[Call][0] = false;
        }
        else if (mapMultiTypesForCancel[brigade] == MCH_)
        {
            mapBusyCallsTypes[Call][2] = false;
        }
        qDebug() << brigade << Qt::endl;
        qDebug() << timersVector.size() << Qt::endl;
        timersVector[brigade]->stop();
        qDebug() << "Таймер для бригады: " << brigade + 1 << " остановлен";
        busyBrigades[brigade] = false;
        qDebug() << "Бригада номер: " << brigade + 1 << "стала свободна";
        mapBrigadeStatus[brigade] = 0;
    }
    vecCallsPriority[Call] = colorsPriority::NOT_COLOR;
    callsModel->setSort(callsModel->fieldIndex("id"), Qt::AscendingOrder);
    callsModel->select();
    qDebug() << "Данные успешно добавлены в таблицу.";
    brigadesModel->setSort(brigadesModel->fieldIndex("id"), Qt::AscendingOrder);
    brigadesModel->select();
}

void DataBase::setFreeEdited(int Call, QVector<int> nums)
{
    int size = nums.size();
    bool multi = false;
    if (size > 1)
    {
        qDebug() << "Multi" << Qt::endl;
    }
    for (int i = 0; i < size; i++)
    {
        int brigade = nums[i];
        QString status = "Свободна";
        QSqlQuery queryStatus(*dataBase);
        queryStatus.prepare("SELECT update_brigade_status(:id, :status)");
        queryStatus.bindValue(":id", brigade + 1);
        queryStatus.bindValue(":status", status);
        if (!queryStatus.exec())
        {
            qDebug() << "Ошибка выполнения запроса UPDATE: " << queryStatus.lastError().text();
            return;
        }
        QSqlQuery querySetStartTimeNull(*dataBase);
        querySetStartTimeNull.prepare("SELECT update_brigade_start_time_set_null(:id)");
        querySetStartTimeNull.bindValue(":id", brigade + 1);
        if (!querySetStartTimeNull.exec())
        {
            qDebug() << "Ошибка выполнения запроса UPDATE: " << querySetStartTimeNull.lastError().text();
            return;
        }

        if (i == 0)
        {
            QSqlQuery queryCallDispatcher(*dataBase);
            queryCallDispatcher.prepare("SELECT update_call_dispatcher_statusEdited(:id)");
            queryCallDispatcher.bindValue(":id", Call + 1);
            if (!queryCallDispatcher.exec())
            {
                qDebug() << "Ошибка выполнения запроса UPDATE: " << queryCallDispatcher.lastError().text();
                return;
            }
            busyCalls[Call] = false;
            multiDispatcher[Call] = false;
        }
        if (mapMultiTypesForCancel[brigade] == AMBULANCE_)
        {
            mapBusyCallsTypes[Call][1] = false;
        }
        else if (mapMultiTypesForCancel[brigade] == POLICE_)
        {
            mapBusyCallsTypes[Call][0] = false;
        }
        else if (mapMultiTypesForCancel[brigade] == MCH_)
        {
            mapBusyCallsTypes[Call][2] = false;
        }
        qDebug() << brigade << Qt::endl;
        qDebug() << timersVector.size() << Qt::endl;
        timersVector[brigade]->stop();
        qDebug() << "Таймер для бригады: " << brigade + 1 << " остановлен";
        busyBrigades[brigade] = false;
        qDebug() << "Бригада номер: " << brigade + 1 << "стала свободна";
        mapBrigadeStatus[brigade] = 0;
    }
    vecCallsPriority[Call] = colorsPriority::NOT_COLOR;
    callsModel->setSort(callsModel->fieldIndex("id"), Qt::AscendingOrder);
    callsModel->select();
    qDebug() << "Данные успешно добавлены в таблицу.";
    brigadesModel->setSort(brigadesModel->fieldIndex("id"), Qt::AscendingOrder);
    brigadesModel->select();
}

void DataBase::cancelBrigadeWhenUnsignedMulti(int idCall)
{
    QSqlQuery queryCalls(*dataBase);
    queryCalls.prepare("SELECT status FROM calls WHERE id = ?");
    queryCalls.addBindValue(idCall);
    if (!queryCalls.exec())
    {
        qDebug() << "Ошибка выполнения запроса SELECT: " << queryCalls.lastError().text();
        return;
    }
    QString status;
    while (queryCalls.next())
    {
        status = queryCalls.record().value(0).toString();
    }

    QString statWork = "В работе";
    QSqlQuery queryBrigadesOnCall2(*dataBase);
    queryBrigadesOnCall2.prepare("SELECT id_brigade FROM brigades_on_call WHERE id_call = ? AND status = ?");
    queryBrigadesOnCall2.addBindValue(idCall);
    queryBrigadesOnCall2.addBindValue(statWork);
    if (!queryBrigadesOnCall2.exec())
    {
        qDebug() << "Ошибка выполнения запроса SELECT: " << queryBrigadesOnCall2.lastError().text();
        return;
    }

    int idBrigadeNum;
    int index = 1;
    QVector<int> nums;
    bool flag = false;
    QString statCanlcel = "Отмена";
    while (queryBrigadesOnCall2.next())
    {
        flag = true;
        idBrigadeNum = queryBrigadesOnCall2.record().value(0).toInt();
        qDebug() << "№ " << index++;
        qDebug() << "Бригада номер: " << idBrigadeNum << Qt::endl;
        QSqlQuery queryBrigadesOnCall3(*dataBase);
        queryBrigadesOnCall3.prepare("SELECT update_brigades_on_set_flag(:id, :flag)");
        queryBrigadesOnCall3.bindValue(":id", idBrigadeNum);
        queryBrigadesOnCall3.bindValue(":flag", 2);
        if (!queryBrigadesOnCall3.exec())
        {
            qDebug() << "Ошибка выполнения запроса SELECT: " << queryBrigadesOnCall3.lastError().text();
            return;
        }
        QSqlQuery queryBrigadesOnCall1(*dataBase);
        queryBrigadesOnCall1.prepare("SELECT update_brigades_on_cancel_status(:id, :status)");
        queryBrigadesOnCall1.bindValue(":id", idBrigadeNum);
        queryBrigadesOnCall1.bindValue(":status", statCanlcel);
        if (!queryBrigadesOnCall1.exec())
        {
            qDebug() << "Ошибка выполнения запроса UPDATE: " << queryBrigadesOnCall1.lastError().text();
            return;
        }
        nums.push_back(idBrigadeNum - 1);
    }
    brigadesOnCallModel->setSort(brigadesOnCallModel->fieldIndex("id"), Qt::AscendingOrder);
    brigadesOnCallModel->select();
    if (!flag)
    {
        return;
    }

    setFreeEdited(idCall - 1, nums);
}

void DataBase::changeStatusBrigade(int Call, int Brigade, bool flag)
{
    qDebug() << "Номер бригады: " << Brigade + 1 << Qt::endl;
    QString dateTimeString = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    if (mapBrigadeStatus[Brigade] == 4)
    {
        QSqlQuery queryStatus1(*dataBase);
        mapBrigadeStatus[Brigade] = 0;
        QString status = "Свободна";
        queryStatus1.prepare("SELECT update_brigade_status(:id, :status)");
        queryStatus1.bindValue(":id", Brigade + 1);
        queryStatus1.bindValue(":status", status);
        if (!queryStatus1.exec())
        {
            qDebug() << "Ошибка выполнения запроса UPDATE: " << queryStatus1.lastError().text();
            return;
        }
        timersVector[Brigade]->stop();
        busyBrigades[Brigade] = false;
        qDebug() << "Данные успешно добавлены в таблицу.";
        brigadesModel->setSort(brigadesModel->fieldIndex("id"), Qt::AscendingOrder);
        brigadesModel->select();
        brigadesOnCallModel->setData(
            brigadesOnCallModel->index(mapBrigadeTime[Brigade], brigadesOnCallModel->fieldIndex("flag")), 1);
        if (!brigadesOnCallModel->submitAll())
        {
            qDebug() << "Ошибка при сохранении данных: " << brigadesOnCallModel->lastError().text();
            brigadesOnCallModel->revertAll();
            return;
        }
        else
        {
            qDebug() << "Данные успешно добавлены в таблицу.";
            brigadesOnCallModel->setSort(brigadesOnCallModel->fieldIndex("id"), Qt::AscendingOrder);
            brigadesOnCallModel->select();
        }
        if (flag)
        {
            if (vecCounterForDeleteMultiCalls[Call] == 2)
            {
                emit sig_RemoveMarker(Call);
            }
            vecCounterForDeleteMultiCalls[Call]++;
        }
        else
        {
            emit sig_RemoveMarker(Call);
        }

        return;
    }
    QString status = statesBrigade[Brigade].at(mapBrigadeStatus[Brigade]);
    QSqlQuery queryStatus(*dataBase);
    queryStatus.prepare("SELECT update_brigade_status(:id, :status)");
    queryStatus.bindValue(":id", Brigade + 1);
    queryStatus.bindValue(":status", status);
    if (!queryStatus.exec())
    {
        qDebug() << "Ошибка выполнения запроса UPDATE: " << queryStatus.lastError().text();
        return;
    }
    if (mapBrigadeStatus[Brigade] == 0)
    {
        QSqlQuery queryStart(*dataBase);
        queryStart.prepare("SELECT update_brigade_start_time(:id, :start_time)");
        queryStart.bindValue(":id", Brigade + 1);
        queryStart.bindValue(":start_time", dateTimeString);
        if (!queryStart.exec())
        {
            qDebug() << "Ошибка выполнения запроса UPDATE: " << queryStart.lastError().text();
            return;
        }
    }
    else if (mapBrigadeStatus[Brigade] == 3)
    {
        QSqlQuery queryEnd(*dataBase);
        queryEnd.prepare("SELECT update_brigade_end_time(:id, :end_time)");
        queryEnd.bindValue(":id", Brigade + 1);
        queryEnd.bindValue(":end_time", dateTimeString);
        if (!queryEnd.exec())
        {
            qDebug() << "Ошибка выполнения запроса UPDATE: " << queryEnd.lastError().text();
            return;
        }
    }
    qDebug() << "Данные успешно добавлены в таблицу.";
    brigadesModel->setSort(brigadesModel->fieldIndex("id"), Qt::AscendingOrder);
    brigadesModel->select();
    if (mapBrigadeStatus[Brigade] == 0 || mapBrigadeStatus[Brigade] == 2 || mapBrigadeStatus[Brigade] == 3)
    {
        int onCallCount = brigadesOnCallModel->rowCount();
        if (mapBrigadeStatus[Brigade] == 0)
        {
            if (!brigadesOnCallModel->insertRow(onCallCount))
            {
                qDebug() << "Ошибка при добавлении строки: " << brigadesOnCallModel->lastError().text();
                return;
            }
            brigadesOnCallModel->setData(
                brigadesOnCallModel->index(onCallCount, brigadesOnCallModel->fieldIndex("id_call")), Call + 1);
            brigadesOnCallModel->setData(
                brigadesOnCallModel->index(onCallCount, brigadesOnCallModel->fieldIndex("id_brigade")), Brigade + 1);
            brigadesOnCallModel->setData(
                brigadesOnCallModel->index(onCallCount, brigadesOnCallModel->fieldIndex("accept_time")),
                dateTimeString);
            brigadesOnCallModel->setData(
                brigadesOnCallModel->index(onCallCount, brigadesOnCallModel->fieldIndex("flag")), 0);
            if (!brigadesOnCallModel->submitAll())
            {
                qDebug() << "Ошибка при сохранении данных: " << brigadesOnCallModel->lastError().text();
                brigadesOnCallModel->revertAll();
                return;
            }
            else
            {
                qDebug() << "Данные успешно добавлены в таблицу.";
                brigadesOnCallModel->setSort(brigadesOnCallModel->fieldIndex("id"), Qt::AscendingOrder);
                brigadesOnCallModel->select();
            }
            QSqlQuery queryBrigadesOnCall2(*dataBase);
            queryBrigadesOnCall2.prepare("SELECT update_brigades_on_set_status(:id, :status)");
            queryBrigadesOnCall2.bindValue(":id", Brigade + 1);
            queryBrigadesOnCall2.bindValue(":status", "В работе");
            if (!queryBrigadesOnCall2.exec())
            {
                qDebug() << "Ошибка выполнения запроса UPDATE: " << queryBrigadesOnCall2.lastError().text();
                return;
            }
            mapBrigadeTime[Brigade] = onCallCount;
        }
        else if (mapBrigadeStatus[Brigade] == 2)
        {

            brigadesOnCallModel->setData(
                brigadesOnCallModel->index(mapBrigadeTime[Brigade], brigadesOnCallModel->fieldIndex("arrival_time")),
                dateTimeString);
        }
        else if (mapBrigadeStatus[Brigade] == 3)
        {
            brigadesOnCallModel->setData(
                brigadesOnCallModel->index(mapBrigadeTime[Brigade], brigadesOnCallModel->fieldIndex("finish_time")),
                dateTimeString);
            QString stat = "Выполнила";
            QSqlQuery queryBrigadesOnCall1(*dataBase);
            queryBrigadesOnCall1.prepare("SELECT update_brigades_on_set_status(:id, :status)");
            queryBrigadesOnCall1.bindValue(":id", Brigade + 1);
            queryBrigadesOnCall1.bindValue(":status", stat);
            if (!queryBrigadesOnCall1.exec())
            {
                qDebug() << "Ошибка выполнения запроса UPDATE: " << queryBrigadesOnCall1.lastError().text();
                return;
            }
        }
        if (!brigadesOnCallModel->submitAll())
        {
            qDebug() << "Ошибка при сохранении данных: " << brigadesOnCallModel->lastError().text();
            brigadesOnCallModel->revertAll();
            return;
        }
        else
        {
            qDebug() << "Данные успешно добавлены в таблицу.";
            brigadesOnCallModel->setSort(brigadesOnCallModel->fieldIndex("id"), Qt::AscendingOrder);
            brigadesOnCallModel->select();
        }
    }
    if (flag)
    {
        if (vecCallsPriority[Call] == colorsPriority::NOT_COLOR)
        {
            emit sig_ChangeColor(Call, status);
            vecCallsPriority[Call] = vecColorPriority[mapBrigadeStatus[Brigade]];
        }
        else
        {
            if (vecCallsPriority[Call] < vecColorPriority[mapBrigadeStatus[Brigade]])
            {
                emit sig_ChangeColor(Call, status);
                vecCallsPriority[Call] = vecColorPriority[mapBrigadeStatus[Brigade]];
            }
            else
            {
                qDebug() << "У цвета ниже приоритет или такой же" << Qt::endl;
            }
        }
    }
    else
    {
        emit sig_ChangeColor(Call, status);
    }

    mapBrigadeStatus[Brigade]++;
}

void DataBase::changeStatusCall(int Call, int Brigade)
{
    if (mapBrigadeStatus[Brigade] == 0 || mapBrigadeStatus[Brigade] == 3)
    {
        QString codeDispatcher;
        int numberDispatcher = 1;
        QSqlQuery dispatcherCode(*dataBase);
        dispatcherCode.prepare("SELECT id FROM dispatchers WHERE name = ?");
        dispatcherCode.addBindValue(currentUser);
        if (!dispatcherCode.exec())
        {
            qDebug() << "Ошибка выполнения запроса SELECT: " << dispatcherCode.lastError().text();
            return;
        }
        while (dispatcherCode.next())
        {
            codeDispatcher = dispatcherCode.value(0).toString();
            numberDispatcher = dispatcherCode.value(0).toInt();
        }
        if (mapBrigadeStatus[Brigade] == 0)
        {
            callsModel->setData(callsModel->index(Call, callsModel->fieldIndex("status")), "В работе");
            callsModel->setData(callsModel->index(Call, callsModel->fieldIndex("id_dispatcher")), numberDispatcher);
            qDebug() << "Диспетчер номер: " << codeDispatcher << Qt::endl;
        }
        else
        {
            qDebug() << "Выполнил" << Qt::endl;
            QString dateTimeString = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
            callsModel->setData(callsModel->index(Call, callsModel->fieldIndex("status")), "Выполнен");
            callsModel->setData(callsModel->index(Call, callsModel->fieldIndex("resolved_at")), dateTimeString);
        }
        if (!callsModel->submitAll())
        {
            qDebug() << "Ошибка при сохранении данных: " << callsModel->lastError().text();
            callsModel->revertAll();
            return;
        }
        else
        {
            qDebug() << "Данные успешно добавлены в таблицу.";
            callsModel->setSort(callsModel->fieldIndex("id"), Qt::AscendingOrder);
            callsModel->select();
        }
    }
}

void DataBase::changeStatusCallMulti(int Call, int Brigade)
{
    qDebug() << mapBrigadeStatus[Brigade] << Qt::endl;
    if (mapBrigadeStatus[Brigade] == 0 || mapBrigadeStatus[Brigade] == 3)
    {
        QString codeDispatcher;
        int numberDispatcher = 1;
        QSqlQuery dispatcherCode(*dataBase);
        dispatcherCode.prepare("SELECT id FROM dispatchers WHERE name = ?");
        dispatcherCode.addBindValue(currentUser);
        if (!dispatcherCode.exec())
        {
            qDebug() << "Ошибка выполнения запроса SELECT: " << dispatcherCode.lastError().text();
            return;
        }
        while (dispatcherCode.next())
        {
            codeDispatcher = dispatcherCode.value(0).toString();
            numberDispatcher = dispatcherCode.value(0).toInt();
        }
        if (mapBrigadeStatus[Brigade] == 0)
        {
            callsModel->setData(callsModel->index(Call, callsModel->fieldIndex("status")), "В работе");
            callsModel->setData(callsModel->index(Call, callsModel->fieldIndex("id_dispatcher")), numberDispatcher);
            qDebug() << "Диспетчер номер: " << codeDispatcher << Qt::endl;
        }
        else if (mapBrigadeStatus[Brigade] == 3)
        {
            if (multiFinishedCalls[Call]++ == 2)
            {
                QString dateTimeString = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
                callsModel->setData(callsModel->index(Call, callsModel->fieldIndex("status")), "Выполнен");
                callsModel->setData(callsModel->index(Call, callsModel->fieldIndex("resolved_at")), dateTimeString);
            }
        }
        if (!callsModel->submitAll())
        {
            qDebug() << "Ошибка при сохранении данных: " << callsModel->lastError().text();
            callsModel->revertAll();
            return;
        }
        else
        {
            qDebug() << "Данные успешно добавлены в таблицу.";
            callsModel->setSort(callsModel->fieldIndex("id"), Qt::AscendingOrder);
            callsModel->select();
        }
    }
}

void DataBase::changeStatusBrigadeLaunch(int Call, int Brigade, bool flag)
{
    qDebug() << "Номер бригады: " << Brigade + 1 << Qt::endl;
    QString dateTimeString = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    if (mapBrigadeStatus[Brigade] == 4)
    {
        QSqlQuery queryStatus1(*dataBase);
        mapBrigadeStatus[Brigade] = 0;
        QString status = "Свободна";
        queryStatus1.prepare("SELECT update_brigade_status(:id, :status)");
        queryStatus1.bindValue(":id", Brigade + 1);
        queryStatus1.bindValue(":status", status);
        if (!queryStatus1.exec())
        {
            qDebug() << "Ошибка выполнения запроса UPDATE: " << queryStatus1.lastError().text();
            return;
        }
        timersVector[Brigade]->stop();
        busyBrigades[Brigade] = false;
        qDebug() << "Данные успешно добавлены в таблицу.";
        brigadesModel->setSort(brigadesModel->fieldIndex("id"), Qt::AscendingOrder);
        brigadesModel->select();
        brigadesOnCallModel->setData(
            brigadesOnCallModel->index(mapBrigadeTime[Brigade], brigadesOnCallModel->fieldIndex("flag")), 1);
        if (!brigadesOnCallModel->submitAll())
        {
            qDebug() << "Ошибка при сохранении данных: " << brigadesOnCallModel->lastError().text();
            brigadesOnCallModel->revertAll();
            return;
        }
        else
        {
            qDebug() << "Данные успешно добавлены в таблицу.";
            brigadesOnCallModel->setSort(brigadesOnCallModel->fieldIndex("id"), Qt::AscendingOrder);
            brigadesOnCallModel->select();
        }
        if (flag)
        {
            if (vecCounterForDeleteMultiCalls[Call] == 2)
            {
                emit sig_RemoveMarker(Call);
            }
            vecCounterForDeleteMultiCalls[Call]++;
        }
        else
        {
            emit sig_RemoveMarker(Call);
        }
        return;
    }
    QString status = statesBrigade[Brigade].at(mapBrigadeStatus[Brigade]);
    QSqlQuery queryStatus(*dataBase);
    queryStatus.prepare("SELECT update_brigade_status(:id, :status)");
    queryStatus.bindValue(":id", Brigade + 1);
    queryStatus.bindValue(":status", status);
    if (!queryStatus.exec())
    {
        qDebug() << "Ошибка выполнения запроса UPDATE: " << queryStatus.lastError().text();
        return;
    }
    if (mapBrigadeStatus[Brigade] == 3)
    {
        QSqlQuery queryEnd(*dataBase);
        queryEnd.prepare("SELECT update_brigade_end_time(:id, :end_time)");
        queryEnd.bindValue(":id", Brigade + 1);
        queryEnd.bindValue(":end_time", dateTimeString);
        if (!queryEnd.exec())
        {
            qDebug() << "Ошибка выполнения запроса UPDATE: " << queryEnd.lastError().text();
            return;
        }
    }
    qDebug() << "Данные успешно добавлены в таблицу.";
    brigadesModel->setSort(brigadesModel->fieldIndex("id"), Qt::AscendingOrder);
    brigadesModel->select();
    if (mapBrigadeStatus[Brigade] == 2 || mapBrigadeStatus[Brigade] == 3)
    {
        if (mapBrigadeStatus[Brigade] == 2)
        {
            brigadesOnCallModel->setData(
                brigadesOnCallModel->index(mapBrigadeTime[Brigade], brigadesOnCallModel->fieldIndex("arrival_time")),
                dateTimeString);
        }
        else if (mapBrigadeStatus[Brigade] == 3)
        {
            brigadesOnCallModel->setData(
                brigadesOnCallModel->index(mapBrigadeTime[Brigade], brigadesOnCallModel->fieldIndex("finish_time")),
                dateTimeString);
            QString stat = "Выполнила";
            QSqlQuery queryBrigadesOnCall1(*dataBase);
            queryBrigadesOnCall1.prepare("SELECT update_brigades_on_set_status(:id, :status)");
            queryBrigadesOnCall1.bindValue(":id", Brigade + 1);
            queryBrigadesOnCall1.bindValue(":status", stat);
            if (!queryBrigadesOnCall1.exec())
            {
                qDebug() << "Ошибка выполнения запроса UPDATE: " << queryBrigadesOnCall1.lastError().text();
                return;
            }
        }
        if (!brigadesOnCallModel->submitAll())
        {
            qDebug() << "Ошибка при сохранении данных: " << brigadesOnCallModel->lastError().text();
            brigadesOnCallModel->revertAll();
            return;
        }
        else
        {
            qDebug() << "Данные успешно добавлены в таблицу.";
            brigadesOnCallModel->setSort(brigadesOnCallModel->fieldIndex("id"), Qt::AscendingOrder);
            brigadesOnCallModel->select();
        }
    }
    if (flag)
    {
        if (vecCallsPriority[Call] == colorsPriority::NOT_COLOR)
        {
            emit sig_ChangeColor(Call, status);
            vecCallsPriority[Call] = vecColorPriority[mapBrigadeStatus[Brigade]];
        }
        else
        {
            if (vecCallsPriority[Call] < vecColorPriority[mapBrigadeStatus[Brigade]])
            {
                emit sig_ChangeColor(Call, status);
                vecCallsPriority[Call] = vecColorPriority[mapBrigadeStatus[Brigade]];
            }
            else
            {
                qDebug() << "У цвета ниже приоритет или такой же" << Qt::endl;
            }
        }
    }
    else
    {
        emit sig_ChangeColor(Call, status);
    }
    mapBrigadeStatus[Brigade]++;
}

void DataBase::changeStatusCallLaunch(int Call, int Brigade)
{
    if (mapBrigadeStatus[Brigade] == 3)
    {
        qDebug() << "Выполнил" << Qt::endl;
        QString dateTimeString = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        callsModel->setData(callsModel->index(Call, callsModel->fieldIndex("status")), "Выполнен");
        callsModel->setData(callsModel->index(Call, callsModel->fieldIndex("resolved_at")), dateTimeString);
        if (!callsModel->submitAll())
        {
            qDebug() << "Ошибка при сохранении данных: " << callsModel->lastError().text();
            callsModel->revertAll();
            return;
        }
        else
        {
            qDebug() << "Данные успешно добавлены в таблицу.";
            callsModel->setSort(callsModel->fieldIndex("id"), Qt::AscendingOrder);
            callsModel->select();
        }
    }
}

void DataBase::changeStatusCallMultiLaunch(int Call, int Brigade)
{
    qDebug() << mapBrigadeStatus[Brigade] << Qt::endl;
    if (mapBrigadeStatus[Brigade] == 3)
    {
        if (multiFinishedCalls[Call]++ == 2)
        {
            QString dateTimeString = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
            callsModel->setData(callsModel->index(Call, callsModel->fieldIndex("status")), "Выполнен");
            callsModel->setData(callsModel->index(Call, callsModel->fieldIndex("resolved_at")), dateTimeString);
        }
        if (!callsModel->submitAll())
        {
            qDebug() << "Ошибка при сохранении данных: " << callsModel->lastError().text();
            callsModel->revertAll();
            return;
        }
        else
        {
            qDebug() << "Данные успешно добавлены в таблицу.";
            callsModel->setSort(callsModel->fieldIndex("id"), Qt::AscendingOrder);
            callsModel->select();
        }
    }
}

void DataBase::stopTimers()
{
    int size = timersVector.size();
    for (int i = 0; i < size; i++)
    {
        if (timersVector[i] != nullptr)
        {
            if (timersVector[i]->isActive())
            {
                qDebug() << "Таймер смены статуса вызова номер: " << i << " остановлен" << Qt::endl;
                activitedTimers[i] = true;
                timersVector[i]->stop();
            }
        }
    }
}

void DataBase::startTimers()
{
    bool exist = false;
    int size = timersVector.size();
    for (int i = 0; i < size; i++)
    {
        if (timersVector[i] != nullptr)
        {
            if (activitedTimers[i] == true)
            {
                exist = true;
                qDebug() << "Таймер смены статуса вызова номер: " << i << " запущен" << Qt::endl;
                timersVector[i]->start(TIMER);
                activitedTimers[i] = false;
            }
        }
    }
    if (!exist)
    {
        qDebug() << "Таймеров для запуска нет" << Qt::endl;
    }
}

StatStruct DataBase::getCountForStat()
{
    int done = 0;
    int cancel = 0;
    int notStart = 0;

    QSqlQuery getCountsNotStart(*dataBase);
    getCountsNotStart.prepare("SELECT COUNT(*) FROM calls WHERE status = ?");
    getCountsNotStart.addBindValue("Не назначен");
    if (!getCountsNotStart.exec())
    {
        qDebug() << "Ошибка в выполнении запроса: " << getCountsNotStart.lastError().text() << Qt::endl;
        return StatStruct(0, 0, 0);
    }
    while (getCountsNotStart.next())
    {
        notStart = getCountsNotStart.value(0).toInt();
    }

    QSqlQuery getCountsDone(*dataBase);
    getCountsDone.prepare("SELECT COUNT(*) FROM calls WHERE status = ?");
    getCountsDone.addBindValue("Выполнен");
    if (!getCountsDone.exec())
    {
        qDebug() << "Ошибка в выполнении запроса: " << getCountsDone.lastError().text() << Qt::endl;
        return StatStruct(0, 0, 0);
    }
    while (getCountsDone.next())
    {
        done = getCountsDone.value(0).toInt();
    }

    QSqlQuery getCountsCancel(*dataBase);
    getCountsCancel.prepare("SELECT COUNT(*) FROM brigades_on_call WHERE status = ?");
    getCountsCancel.addBindValue("Отмена");
    if (!getCountsCancel.exec())
    {
        qDebug() << "Ошибка в выполнении запроса: " << getCountsCancel.lastError().text() << Qt::endl;
        return StatStruct(0, 0, 0);
    }
    while (getCountsCancel.next())
    {
        cancel = getCountsCancel.value(0).toInt();
    }

    qDebug() << "Выполнено: " << done << " " << "Отменено: " << cancel << " " << "Не назначено: " << notStart
             << Qt::endl;
    return StatStruct(done, cancel, notStart);
}

QVector<QPair<QString, int>> DataBase::getOftenBar()
{
    QSqlQuery getTopIncidentTypes(*dataBase);

    // SQL-запрос: выбираем типы инцидентов и их количество, сортируем по убыванию количества, берем топ-5
    getTopIncidentTypes.prepare("SELECT incident_type, COUNT(*) as cnt "
                                "FROM calls "
                                "GROUP BY incident_type "
                                "ORDER BY cnt DESC "
                                "LIMIT 5");

    QVector<QPair<QString, int>> topIncidentTypes;

    if (!getTopIncidentTypes.exec())
    {
        qDebug() << "Ошибка при выполнении запроса:" << getTopIncidentTypes.lastError().text();
    }
    else
    {
        while (getTopIncidentTypes.next())
        {
            QString incidentType = getTopIncidentTypes.value(0).toString();
            int count = getTopIncidentTypes.value(1).toInt();
            topIncidentTypes.append(qMakePair(incidentType, count));
        }
    }

    for (const auto& pair : topIncidentTypes)
    {
        qDebug() << pair.first << ":" << pair.second;
    }
    return topIncidentTypes;
}
