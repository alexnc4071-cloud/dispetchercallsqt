#include "dispatcherdate.h"
#include "ui_dispatcherdate.h"

Dispatcherdate::Dispatcherdate(QDialog* parent) : QDialog(parent), ui(new Ui::Dispatcherdate)
{
    ui->setupUi(this);
    emptyMsg = new QMessageBox;
    emptyMsg->setWindowTitle("Ошибка");
    emptyMsg->setText("У вас пустая строка");
    emptyMsg->setWindowIcon(QIcon(":/Images/IconWarning.png"));
    emptyMsg->setIconPixmap(QPixmap(":/Images/iconSign.png"));
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    connect(ui->comboBox_dispatcher_name_, &QComboBox::currentTextChanged, this, &Dispatcherdate::slot_name_selected);
}

Dispatcherdate::~Dispatcherdate()
{
    delete emptyMsg;
    delete ui;
}

void Dispatcherdate::on_pushButton_enter_dispatcher__clicked()
{
    QString strName = ui->lineEdit_dispatcher_name_->text();
    if (strName.isEmpty())
    {
        emptyMsg->exec();
        return;
    }
    int index = ui->comboBox_dispatcher_name_->findText(strName);
    if (index == -1)
    {
        ui->comboBox_dispatcher_name_->addItem(strName);
        ui->comboBox_dispatcher_name_->setCurrentIndex(ui->comboBox_dispatcher_name_->count() - 1);

        emit sig_send_dispatcher_name(strName, true);
    }
    emit sig_send_dispatcher_name(strName, false);
    this->close();
}

void Dispatcherdate::slot_name_selected(QString name)
{
    ui->lineEdit_dispatcher_name_->setText(name);
}

void Dispatcherdate::setModel(QSqlTableModel* model)
{
    int size = model->rowCount();
    for (int i = 0; i < size; i++)
    {
        QSqlRecord record = model->record(i);
        ui->comboBox_dispatcher_name_->addItem(record.value(1).toString());
    }
}

void Dispatcherdate::clearList()
{
    ui->comboBox_dispatcher_name_->clear();
}
