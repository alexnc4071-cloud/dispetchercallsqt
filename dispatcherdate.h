#ifndef DISPATCHERDATE_H
#define DISPATCHERDATE_H
#include <QDialog>
#include <QMessageBox>
#include <QSqlRecord>
#include <QSqlTableModel>
namespace Ui
{
class Dispatcherdate;
}

class Dispatcherdate : public QDialog
{
    Q_OBJECT
  public:
    explicit Dispatcherdate(QDialog* parent = nullptr);
    void setModel(QSqlTableModel* model);
    ~Dispatcherdate();
    void clearList();
    void checkLaunch(bool launch);

  private slots:
    void on_pushButton_enter_dispatcher__clicked();
    void slot_name_selected(QString name);

  signals:
    void sig_send_dispatcher_name(QString name, bool flag);

  private:
    QMessageBox* emptyMsg;
    bool emptyLine;
    QString currentDispatcher = "unknown";
    Ui::Dispatcherdate* ui;
};

#endif // DISPATCHERDATE_H
