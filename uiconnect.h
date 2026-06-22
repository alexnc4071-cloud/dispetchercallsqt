#ifndef UICONNECT_H
#define UICONNECT_H
#include "database.h"
#include <QDialog>
#include <QMessageBox>
#include <QTimer>
#include <QWidget>
namespace Ui
{
class UiConnect;
}

class UiConnect : public QDialog
{
    Q_OBJECT
  public:
    explicit UiConnect(QDialog* parent = nullptr);
    void myConnect();
    ~UiConnect();
    int setMapDateOfCall(QMap<QString, QString> mapDateOfCall);
    void dataBaseIsEpmtyMarks();
    void dataBaseIsEpmtyDispatchers();
    int getCountRowsCalls();
    void addDispatcher(QString name);
    void checkIndexs(int Call, int Brigade);
    void setCurrentDispatcher(QString name);
    void setTimerCallFail(int idCall);
    bool getCheckIndexRight(int idCall);
    QString cancelButton(int num);
    int getSizeBrigades();
    QMap<int, QString> getBeginStatus();
    StatStruct getCountForStat();
    QVector<QPair<QString, int>> getOftenBar();

  private slots:
    void on_pushButton_connect__clicked();
    void slot_statusConnection(bool flag);
    void slot_refreshConnection();
    void on_pushButton_clear__clicked();
    void on_pushButton_config__clicked();
    void on_pushButton_cascade_delete__clicked();
    void slot_accept_change_color(int index, QString status);
    void slot_accept_remove_marker(int index);
  signals:
    void sig_sendStatusConnection(bool flag);
    void sig_sendMapModels(QMap<QString, QSqlTableModel*> map);
    void sig_resetCounterMarker();
    void sig_haveSomeMarks();
    void sig_haveSomeDispatchers();
    void sig_changeColor(int index, QString status);
    void sig_removeMarker(int index);

  private:
    bool marksExist = false;
    bool emptyMarks;
    bool dispatchersExist = false;
    bool emptyDispatchers;
    void refreshConfig();
    bool connectionStatus = false;
    DataBase* myDataBase;
    QVector<QString> dataForConnect;
    QMessageBox* msgConnectError;
    QMessageBox* msgCascadeError;
    QMessageBox* msgCascade;
    QPushButton* yesCascade;
    QPushButton* cancelCascade;
    QTimer* timer;
    Ui::UiConnect* ui;
};
#endif // UICONNECT_H
