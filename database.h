#ifndef DATABASE_H
#define DATABASE_H
#include "datastorage.h"
#include <QDateTime>
#include <QMessageBox>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlTableModel>
#include <QTimer>
#include <QVector>
#define POSTGRE_DRIVER "QPSQL"
#define DB_NAME "MyDB"
#define NUM_DATA_FOR_CONNECT_TO_DB 5
#define FIRE_ "101"
#define POLICE_ "102"
#define AMBULANCE_ "103"
#define MCH_ "112"
#define POLICE_AND_AMBULANCE_ "102-103"
#define MCH_AND_AMBULANCE_ "103-112"
#define TIMER 10000
enum fieldsForConnect
{
    HOST_NAME_ = 0,
    DBNAME_ = 1,
    LOGIN_ = 2,
    PASS_ = 3,
    PORT_ = 4
};

struct StatStruct
{
    StatStruct(int done_, int cancel_, int notStart_) : done(done_), cancel(cancel_), notStart(notStart_) {}
    int done;
    int cancel;
    int notStart;
};

enum colorsPriority // Приоритет для цвета Мультивызова
{
    NOT_COLOR = -1,
    RED_ = 0, // Вызов свободен
    YELLOW_,  // Бригада Занята
    BLUE_,    // Бригада в пути
    ORANGE_,  // Бригада на месте
    GREEN_    // Бригада выполнила задачу
};

class DataBase : public QObject
{
    Q_OBJECT
  public:
    explicit DataBase(QObject* parent = nullptr);
    ~DataBase();
    void addDataBase(QString driver, QString nameDB = "");
    QSqlError getLastError(void);
    bool isOpen();
    bool isEmptyMarks();
    bool isEmptyDispatchers();

  signals:
    void sig_SendStatusConnection(bool);
    void sig_ChangeColor(int index, QString status);
    void sig_RemoveMarker(int index);

  public:
    bool getCheckIndexRight(int idCall);
    void setTimerCallFail(int idCall);
    void connectToDataBase(QVector<QString> dataForConnect);
    void disconnectFromDataBase();
    QMap<QString, QSqlTableModel*> setModelForTableViews();
    int setMapDateOfCall(QMap<QString, QString> mapDateOfCall);
    void deleteCascade();
    int getCountRowsCalls();
    void addDispatcher(QString name);
    void setCurrentDispatcher(QString name);
    void checkIndexs(int Call, int Brigade);
    int getSizeBrigades();
    void stopTimers();
    void startTimers();
    StatStruct getCountForStat();
    QVector<QPair<QString, int>> getOftenBar();
    QMap<int, QString> getBeginStatus();
    QString cancelButton(int numCall);

  private:
    void checkLaunch();
    void cancelBrigadeWhenUnsignedMulti(int idCall);
    void changeStatusLaunch(int Call, int Brigade, bool flag);
    void changeStatusBrigadeLaunch(int Call, int Brigade, bool flag);
    void changeStatusCallLaunch(int Call, int Brigade);
    void changeStatusCallMultiLaunch(int Call, int Brigade);
    void setFree(int Call, QVector<int> nums);
    void setFreeEdited(int Call, QVector<int> nums);
    void changeStatus(int Call, int Brigade, bool flag);
    void changeStatusBrigade(int Call, int Brigade, bool flag);
    void changeStatusCall(int Call, int Brigade);
    void changeStatusCallMulti(int Call, int Brigade);
    QString currentUser;
    QVector<bool> checkIndexUnsignedVector;
    QVector<bool> busyBrigades;
    QVector<bool> busyCalls;
    QVector<bool> multiDispatcher;
    QVector<int> multiFinishedCalls;
    QVector<QStringList> statesBrigade;
    QVector<QTimer*> timersVector;
    QVector<bool> activitedTimers;
    QVector<bool> checkStatusVector;
    QVector<colorsPriority> vecColorPriority;
    QVector<int> vecCallsPriority;
    QVector<int> vecCounterForDeleteMultiCalls;
    QMap<int, QString> mapBeginStatus;
    QMap<int, int> mapBrigadeStatus;
    QMap<int, int> mapBrigadeTime;
    QMap<QString, QString> mapTypeCode;
    QMap<int, QVector<bool>> mapBusyCallsTypes; // QVector{ 0 -police, 1-ambulance, 2-mch }
    QMap<int, QString> mapMultiTypesForCancel;
    void defaultSettings();
    void loadCalls();
    void loadDispatchers();
    void loadBrigades();
    Datastorage storageClass;
    int countRowsCalls;
    int countRowsDispatchers;
    int countRowsBrigades;
    bool firstLaunch = true;
    bool emptyMarks = false;
    bool emptyDispatchers = false;
    QSqlDatabase* dataBase = nullptr;
    QSqlTableModel* brigadesModel = nullptr;
    QSqlTableModel* dispatcherIdModel = nullptr;
    QSqlTableModel* brigadesOnCallModel = nullptr;
    QSqlTableModel* callsModel = nullptr;
    QMessageBox* msg;
    int sizeBrigades;
};
#endif // DATABASE_H
