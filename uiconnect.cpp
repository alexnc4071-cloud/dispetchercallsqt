#include "uiconnect.h"
#include "ui_uiconnect.h"

UiConnect::UiConnect(QDialog* parent) : QDialog(parent), ui(new Ui::UiConnect)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/Images/iconMain.png"));
    setWindowTitle("Настройки соединения");
    myDataBase = new DataBase(this);
    connect(myDataBase, &QObject::destroyed, this, [&]() { qDebug() << "myDataBase  был удален"; });
    myDataBase->addDataBase(POSTGRE_DRIVER, DB_NAME);
    dataForConnect.resize(NUM_DATA_FOR_CONNECT_TO_DB);
    refreshConfig();
    msgConnectError = new QMessageBox(this);
    msgCascadeError = new QMessageBox(this);
    msgCascade = new QMessageBox(this);
    timer = new QTimer(this);
    msgCascadeError->setWindowTitle("Ошибка");
    msgCascadeError->setWindowIcon(QIcon(":/Images/IconWarning.png"));
    msgCascadeError->setText("Вы не подключены к базе данных!!!");
    msgCascadeError->setIcon(QMessageBox::Critical);
    msgCascadeError->setIconPixmap(QPixmap(":/Images/iconMistake.png"));
    msgCascade->setWindowTitle("Подтвердите действие");
    msgCascade->setWindowIcon(QIcon(":/Images/IconWarning.png"));
    msgCascade->setText("Вы точно хотите стереть данные?");
    msgCascade->setIcon(QMessageBox::Warning);
    msgCascade->setIconPixmap(QPixmap(":/Images/iconSign.png"));
    yesCascade = msgCascade->addButton("Подтвердить", QMessageBox::AcceptRole);
    cancelCascade = msgCascade->addButton("Отмена", QMessageBox::RejectRole);
    connect(myDataBase, &DataBase::sig_SendStatusConnection, this, &UiConnect::slot_statusConnection);
    connect(timer, &QTimer::timeout, this, &UiConnect::slot_refreshConnection);
    connect(myDataBase, &DataBase::sig_ChangeColor, this, &UiConnect::slot_accept_change_color);
    connect(myDataBase, &DataBase::sig_RemoveMarker, this, &UiConnect::slot_accept_remove_marker);
}

void UiConnect::refreshConfig()
{
    ui->lineEdit_name_host_->setText("localhost");
    ui->lineEdit_name_bd_->setText("postgres");
    ui->lineEdit_port_->setText("5432");
    ui->lineEdit_user_->setText("postgres");
    ui->lineEdit_user_password_->setText("123");
}

void UiConnect::slot_accept_change_color(int index, QString status)
{
    emit sig_changeColor(index, status);
}

void UiConnect::slot_accept_remove_marker(int index)
{
    emit sig_removeMarker(index);
}

void UiConnect::myConnect()
{
    if (ui->label_status_connect_->text() == "Отключено")
    {
        dataForConnect[HOST_NAME_] = ui->lineEdit_name_host_->text();
        dataForConnect[DBNAME_] = ui->lineEdit_name_bd_->text();
        dataForConnect[LOGIN_] = ui->lineEdit_user_->text();
        dataForConnect[PASS_] = ui->lineEdit_user_password_->text();
        dataForConnect[PORT_] = ui->lineEdit_port_->text();
        myDataBase->connectToDataBase(dataForConnect);
        if (connectionStatus)
        {
            ui->label_status_connect_->setText("Подключено");
            ui->label_status_connect_->setStyleSheet("QLabel {color: green}");
            ui->pushButton_connect_->setText("Отключится");
            myDataBase->startTimers();
            auto modelsMap = myDataBase->setModelForTableViews();
            emit sig_sendMapModels(modelsMap);
            dataBaseIsEpmtyMarks();
            dataBaseIsEpmtyDispatchers();
        }
    }
    else if (ui->label_status_connect_->text() == "Подключено")
    {
        ui->label_status_connect_->setText("Отключено");
        ui->label_status_connect_->setStyleSheet("QLabel {color: red}");
        ui->pushButton_connect_->setText("Подключится");
        myDataBase->stopTimers();
        myDataBase->disconnectFromDataBase();
    }
}

void UiConnect::slot_statusConnection(bool flag)
{
    connectionStatus = flag;
    if (!flag)
    {
        msgConnectError->setText(myDataBase->getLastError().text());
        msgConnectError->setIcon(QMessageBox::Critical);
        timer->start(5000);
        msgConnectError->show();
    }
    else
    {
        timer->stop();
    }
}

StatStruct UiConnect::getCountForStat()
{
    return myDataBase->getCountForStat();
}

QVector<QPair<QString, int>> UiConnect::getOftenBar()
{
    return myDataBase->getOftenBar();
}

void UiConnect::slot_refreshConnection()
{
    msgConnectError->close();
    myConnect();
}

UiConnect::~UiConnect()
{
    qDebug() << "Отключение в декструкторе";
    delete ui;
}

void UiConnect::on_pushButton_connect__clicked()
{
    myConnect();
    bool state = myDataBase->isOpen();
    if (state)
    {
        qDebug() << "Вы подлючились к базе данных" << Qt::endl;
        emit sig_sendStatusConnection(true);
    }
    else
    {
        qDebug() << "Вы отключились от базы данных" << Qt::endl;
        emit sig_sendStatusConnection(false);
    }
}

void UiConnect::on_pushButton_clear__clicked()
{
    ui->lineEdit_name_bd_->clear();
    ui->lineEdit_name_host_->clear();
    ui->lineEdit_port_->clear();
    ui->lineEdit_user_->clear();
    ui->lineEdit_user_password_->clear();
}

int UiConnect::setMapDateOfCall(QMap<QString, QString> mapDateOfCall)
{
    marksExist = true;
    return myDataBase->setMapDateOfCall(mapDateOfCall);
}

void UiConnect::on_pushButton_config__clicked()
{
    refreshConfig();
}

QString UiConnect::cancelButton(int num)
{
    return myDataBase->cancelButton(num);
}

void UiConnect::on_pushButton_cascade_delete__clicked()
{
    bool state = myDataBase->isOpen();
    if (!state)
    {
        msgCascadeError->exec();
    }
    else
    {
        msgCascade->exec();
        if (msgCascade->clickedButton() == yesCascade)
        {
            myDataBase->deleteCascade();
            emit sig_resetCounterMarker();
        }
    }
}

void UiConnect::dataBaseIsEpmtyMarks()
{
    emptyMarks = myDataBase->isEmptyMarks();
    if (!emptyMarks && !marksExist)
    {
        qDebug() << "сигнал: haveSomeMarks" << Qt::endl;
        emit sig_haveSomeMarks();
        marksExist = true;
    }
}

void UiConnect::dataBaseIsEpmtyDispatchers()
{
    emptyDispatchers = myDataBase->isEmptyDispatchers();
    if (!emptyDispatchers && !dispatchersExist)
    {
        qDebug() << "сигнал: haveSomeDispatchers" << Qt::endl;
        emit sig_haveSomeDispatchers();
        dispatchersExist = true;
    }
}

QMap<int, QString> UiConnect::getBeginStatus()
{
    return myDataBase->getBeginStatus();
}

int UiConnect::getSizeBrigades()
{
    return myDataBase->getSizeBrigades();
}

void UiConnect::setTimerCallFail(int idCall)
{
    myDataBase->setTimerCallFail(idCall);
}

bool UiConnect::getCheckIndexRight(int idCall)
{
    return myDataBase->getCheckIndexRight(idCall);
}

int UiConnect::getCountRowsCalls()
{
    return myDataBase->getCountRowsCalls();
}

void UiConnect::addDispatcher(QString name)
{
    myDataBase->addDispatcher(name);
}

void UiConnect::checkIndexs(int Call, int Brigade)
{
    myDataBase->checkIndexs(Call, Brigade);
}

void UiConnect::setCurrentDispatcher(QString name)
{
    myDataBase->setCurrentDispatcher(name);
}
