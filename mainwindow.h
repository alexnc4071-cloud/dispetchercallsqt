#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "QTextBrowser"
#include "dispatcherdate.h"
#include "markermanager.h"
#include "norounddelegate.h"
#include "simulatorstart.h"
#include "uiconnect.h"
#include <QCheckBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMainWindow>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QPair>
#include <QRadioButton>
#include <QSpinBox>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QtCharts>
// #define TheCallIsUnsignedTime     120000   // 2 минуты
// #define TheCallIsUnsignedTime 300000 // 5 минут
#define TheCallIsUnsignedTime 600000 // 10 минут
// #define TheCallIsUnsignedTime 30000 // 30 секунд
//  #define TheCallIsUnsignedTime 18000 // 18 секунд
QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

enum FILTER_TYPE
{
    ALL = 0,
    POLICE,
    AMBULANCE,
    MCH,
    FIRE
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
  public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

  private slots:
    void on_pushButton_start_simulator__clicked();
    void on_pushButton_connection_settings__clicked();
    void on_pushButton_window_dispatcher__clicked();
    void slot_acceptStatusConnection(bool flag);
    void slot_acceptMapModels(QMap<QString, QSqlTableModel*> map);
    void slot_acceptMapDateOfCall(QMap<QString, QString> mapDateOfCall);
    void slot_resetCounterMarker();
    void slot_acceptMarksAllAppeared();
    void slot_acceptHaveSomeMarks();
    void slot_acceptHaveSomeDispatchers();
    void slot_appointButton_clicked();
    void on_pushButton_dispatcher_date__clicked();
    void slot_accept_dispatcher_name(QString name, bool flag);
    void slot_accept_filter(int num);
    void slot_accept_cancel_button();
    void slot_accept_stat_button();
    void slot_accept_bar_button();
    void slot_accept_change_color(int index, QString status);
    void slot_accept_remove_marker(int index);
    void slot_accept_list_timers(int call);

  private:
    int countLog = 0;
    QLabel* nameLabel;
    bool firstLaunch = true;
    Dispatcherdate* dispatcherLoginWindow;
    NoRoundDelegate* delegate;
    QMessageBox* msgAllMarks;
    bool allMarksAppeared = false;
    QVector<QStringList> listOfMarks;
    QVector<QTimer*> listOfTimers;
    QVector<QPair<bool, bool>> finishedTimersUnsigned;
    int counterRowInCalls = 0;
    int counterMarker = 0;
    MarkerManager markerControl;
    QMap<QString, QSqlTableModel*> myMap;
    void myConnect();
    bool connectionStatus = false;
    Ui::MainWindow* ui;
    UiConnect* uiConnect;
    SimulatorStart* simStart;
    QMdiArea* myArea;
    QWidget* dispatcherWindow;
    QVBoxLayout* areaLayout;
    QCheckBox* checkBoxOpenALL;
    QCheckBox* checkBoxOpenCalls;
    QCheckBox* checkBoxOpenBrigades;
    QCheckBox* checkBoxOpenBrigadesOnCall;
    QCheckBox* checkBoxOpenMap;
    QCheckBox* checkBoxOpenPanel;
    QCheckBox* checkBoxOpenDispatchers;
    QPushButton* buttonTitleSet;
    QTableView* callsView;
    QTableView* brigadesView;
    QTableView* dispatchesView;
    QTableView* brigadesOnCallView;
    QGroupBox* boxOfCheckBoxs;
    QVBoxLayout* subwindowPanelLayout;
    QGroupBox* panelDispatherGroup;
    QSpinBox* numUnitSpin;
    QSpinBox* numCallSpin;
    QTextBrowser* browserLog;
    QLabel* currentUser;
    QRadioButton* filterAll;
    QChart* chartPie = nullptr;
    QChartView* chartViewPie = nullptr;
    QPieSeries* seriesPie = nullptr;
    QChart* chartBar = nullptr;
    QChartView* chartViewBar = nullptr;
    QBarSeries* seriesBar = nullptr;
    QBarSet* barSet = nullptr;
    QWidget* graficWidgetPie = nullptr;
    QWidget* graficWidgetBar = nullptr;
    QValueAxis* barAxisY = nullptr;
    QBarCategoryAxis* barAxisX = nullptr;
};

#endif // MAINWINDOW_H
