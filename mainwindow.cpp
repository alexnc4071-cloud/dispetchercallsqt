#include "mainwindow.h"

#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/Images/iconMain.png"));
    setWindowTitle("Окно действий");
    resize(500, 400);
    dispatcherLoginWindow = new Dispatcherdate;
    dispatcherLoginWindow->setFixedSize(400, 200);
    delegate = new NoRoundDelegate(this);
    msgAllMarks = new QMessageBox;
    msgAllMarks->setWindowTitle("Оповещение");
    msgAllMarks->setWindowIcon(QIcon(":/Images/IconWarning.png"));
    msgAllMarks->setText("Симуляция закончилась, достигнуто максимальное количество точек (очистите данные)");
    msgAllMarks->setIcon(QMessageBox::Information);
    msgAllMarks->setIconPixmap(QPixmap(":/Images/iconSign.png"));
    uiConnect = new UiConnect;
    simStart = new SimulatorStart(this);
    dispatcherWindow = new QWidget();
    dispatcherWindow->setWindowIcon(QIcon(":/Images/iconMain.png"));
    dispatcherWindow->setWindowTitle("Окно диспетчера");
    areaLayout = new QVBoxLayout(dispatcherWindow);
    boxOfCheckBoxs = new QGroupBox("Опции");
    QHBoxLayout* checkBoxLayout = new QHBoxLayout(boxOfCheckBoxs);
    checkBoxOpenALL = new QCheckBox("Открыть все окна", boxOfCheckBoxs);
    checkBoxOpenALL->setChecked(true);
    buttonTitleSet = new QPushButton("Расположить мозайкой", boxOfCheckBoxs);
    buttonTitleSet->setFixedSize(180, 27);
    checkBoxOpenBrigadesOnCall = new QCheckBox("Бригады на вызовах", boxOfCheckBoxs);
    checkBoxOpenCalls = new QCheckBox("Вызовы", boxOfCheckBoxs);
    checkBoxOpenMap = new QCheckBox("Карта", boxOfCheckBoxs);
    checkBoxOpenBrigades = new QCheckBox("Бригады", boxOfCheckBoxs);
    checkBoxOpenPanel = new QCheckBox("Панель диспетчера", boxOfCheckBoxs);
    checkBoxOpenDispatchers = new QCheckBox("Диспетчеры", boxOfCheckBoxs);
    checkBoxOpenCalls->setCheckState(Qt::Checked);
    checkBoxOpenCalls->setEnabled(false);
    checkBoxOpenBrigades->setCheckState(Qt::Checked);
    checkBoxOpenBrigades->setEnabled(false);
    checkBoxOpenBrigadesOnCall->setCheckState(Qt::Checked);
    checkBoxOpenBrigadesOnCall->setEnabled(false);
    checkBoxOpenMap->setCheckState(Qt::Checked);
    checkBoxOpenMap->setEnabled(false);
    checkBoxOpenPanel->setCheckState(Qt::Checked);
    checkBoxOpenPanel->setEnabled(false);
    checkBoxOpenDispatchers->setCheckState(Qt::Checked);
    checkBoxOpenDispatchers->setEnabled(false);
    checkBoxLayout->addWidget(checkBoxOpenALL);
    checkBoxLayout->addWidget(checkBoxOpenCalls);
    checkBoxLayout->addWidget(checkBoxOpenMap);
    checkBoxLayout->addWidget(checkBoxOpenBrigades);
    checkBoxLayout->addWidget(checkBoxOpenBrigadesOnCall);
    checkBoxLayout->addWidget(checkBoxOpenDispatchers);
    checkBoxLayout->addWidget(checkBoxOpenPanel);
    checkBoxLayout->addWidget(buttonTitleSet);
    areaLayout->addWidget(boxOfCheckBoxs);
    myArea = new QMdiArea();
    myArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    myArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    for (int i = 0; i < 6; i++)
    {
        QMdiSubWindow* subWindow = new QMdiSubWindow();
        subWindow->setWindowFlags(Qt::WindowTitleHint | Qt::CustomizeWindowHint);
        if (i == 0)
        {
            subWindow->setObjectName("panelDispatcher");
            subWindow->setWindowTitle("Панель диспетчера");
            QWidget* panelWidget = new QWidget(subWindow);
            QVBoxLayout* VlayoutPanel = new QVBoxLayout(panelWidget);
            connect(VlayoutPanel, &QObject::destroyed, this, [&]() { qDebug() << "VlayoutPanel был удален"; });
            panelDispatherGroup = new QGroupBox("Кнопки диспетчера");
            connect(panelDispatherGroup, &QObject::destroyed, this,
                    [&]() { qDebug() << "panelDispatherGroup был удален"; });
            VlayoutPanel->addWidget(panelDispatherGroup);
            QVBoxLayout* HlayoutGroup = new QVBoxLayout(panelDispatherGroup);
            HlayoutGroup->setSpacing(1);
            connect(HlayoutGroup, &QObject::destroyed, this, [&]() { qDebug() << "HlayoutGroup был удален"; });
            QFormLayout* callAndUnitLayout = new QFormLayout();
            callAndUnitLayout->setContentsMargins(100, 15, 100, 0);
            numUnitSpin = new QSpinBox;
            numUnitSpin->setMinimum(1);
            numUnitSpin->setMaximum(uiConnect->getSizeBrigades());
            connect(numUnitSpin, &QObject::destroyed, this, [&]() { qDebug() << "numUnitSpin был удален"; });
            numUnitSpin->setAlignment(Qt::AlignmentFlag::AlignCenter);
            numCallSpin = new QSpinBox;
            numCallSpin->setMinimum(1);
            connect(numCallSpin, &QObject::destroyed, this, [&]() { qDebug() << "numCallSpin был удален"; });
            numCallSpin->setAlignment(Qt::AlignmentFlag::AlignCenter);
            callAndUnitLayout->addRow("Введи номер вызова: ", numCallSpin);
            callAndUnitLayout->addRow("Введи номер бригады: ", numUnitSpin);
            QHBoxLayout* HlayoutButtons = new QHBoxLayout;
            connect(HlayoutButtons, &QObject::destroyed, this, [&]() { qDebug() << "HlayoutButtons  был удален"; });
            QPushButton* appointButton = new QPushButton("Назначить");
            connect(appointButton, &QObject::destroyed, this, [&]() { qDebug() << "appointButton был удален"; });
            connect(appointButton, &QPushButton::clicked, this, &MainWindow::slot_appointButton_clicked);
            QPushButton* cancelButton = new QPushButton("Отменить");
            cancelButton->setToolTip("Кнопка отмены освобождает вызов и все бригады связанные с ним");
            connect(cancelButton, &QObject::destroyed, this, [&]() { qDebug() << "cancelButton  был удален"; });
            QPushButton* statsButton = new QPushButton("Статистика");
            connect(statsButton, &QObject::destroyed, this, [&]() { qDebug() << "statsButton  был удален"; });
            QPushButton* barButton = new QPushButton("Частота");
            connect(barButton, &QObject::destroyed, this, [&]() { qDebug() << "barButton   был удален"; });
            QGroupBox* logAndInputGroupBox = new QGroupBox;
            logAndInputGroupBox->setObjectName("logAndInputGroupbox");
            logAndInputGroupBox->setStyleSheet("QGroupBox#logAndInputGroupbox { border: none; }");
            QVBoxLayout* logAndInputLayout = new QVBoxLayout(logAndInputGroupBox);
            QHBoxLayout* handLayout = new QHBoxLayout;
            browserLog = new QTextBrowser;
            QTextBrowser* browserHand = new QTextBrowser;
            browserHand->setAlignment(Qt::AlignmentFlag::AlignCenter);
            browserHand->append("Пожарные - 101");
            browserHand->append("Полиция - 102");
            browserHand->append("Скорая - 103");
            browserHand->append("Мчс - 112");
            browserHand->append("Полиция и Скорая - 102-103");
            browserHand->append("Мчс и Скорая - 103-112");
            connect(browserHand, &QObject::destroyed, this, [&]() { qDebug() << "browserHand  был удален"; });
            handLayout->addWidget(browserLog);
            handLayout->addWidget(browserHand);
            nameLabel = new QLabel("Диспетчер: ");
            nameLabel->setAlignment(Qt::AlignmentFlag::AlignRight);
            connect(nameLabel, &QObject::destroyed, this, [&]() { qDebug() << "nameLabel  был удален"; });
            currentUser = new QLabel();
            currentUser->setAlignment(Qt::AlignmentFlag::AlignLeft);
            QHBoxLayout* nameLayout = new QHBoxLayout;
            nameLayout->setContentsMargins(0, 0, 0, 10);
            nameLayout->addWidget(nameLabel);
            nameLayout->addWidget(currentUser);
            connect(nameLayout, &QObject::destroyed, this, [&]() { qDebug() << "nameLayout  был удален"; });
            QHBoxLayout* filterLayout = new QHBoxLayout;
            filterLayout->setAlignment(Qt::AlignmentFlag::AlignCenter);
            QRadioButton* filterAm = new QRadioButton("Скорая");
            QRadioButton* filterFire = new QRadioButton("Пожарные");
            QRadioButton* filterPolice = new QRadioButton("Полиция");
            QRadioButton* filterMch = new QRadioButton("Мчс");
            filterAll = new QRadioButton("Все");
            filterAll->setChecked(true);
            connect(filterAll, &QRadioButton::clicked, this,
                    [=]()
                    {
                        slot_accept_filter(FILTER_TYPE::ALL);
                        if (countLog++ > 8)
                        {
                            browserLog->clear();
                            countLog = 0;
                        }
                        browserLog->append("Фильтер на все активирован");
                    });
            connect(filterAm, &QRadioButton::clicked, this,
                    [=]()
                    {
                        slot_accept_filter(FILTER_TYPE::AMBULANCE);
                        if (countLog++ > 8)
                        {
                            browserLog->clear();
                            countLog = 0;
                        }
                        browserLog->append("Фильтер на скорую активирован");
                    });
            connect(filterFire, &QRadioButton::clicked, this,
                    [=]()
                    {
                        slot_accept_filter(FILTER_TYPE::FIRE);
                        if (countLog++ > 8)
                        {
                            browserLog->clear();
                            countLog = 0;
                        }
                        browserLog->append("Фильтер на пожарных активирован");
                    });
            connect(filterPolice, &QRadioButton::clicked, this,
                    [=]()
                    {
                        slot_accept_filter(FILTER_TYPE::POLICE);
                        if (countLog++ > 8)
                        {
                            browserLog->clear();
                            countLog = 0;
                        }
                        browserLog->append("Фильтер на полицию активирован");
                    });
            connect(filterMch, &QRadioButton::clicked, this,
                    [=]()
                    {
                        slot_accept_filter(FILTER_TYPE::MCH);
                        if (countLog++ > 8)
                        {
                            browserLog->clear();
                            countLog = 0;
                        }
                        browserLog->append("Фильтер на мчс активирован");
                    });
            filterLayout->addWidget(filterAll);
            filterLayout->addWidget(filterAm);
            filterLayout->addWidget(filterFire);
            filterLayout->addWidget(filterPolice);
            filterLayout->addWidget(filterMch);
            connect(filterLayout, &QObject::destroyed, this, [&]() { qDebug() << "filterLayout  был удален"; });
            connect(filterAm, &QObject::destroyed, this, [&]() { qDebug() << "filterAm  был удален"; });
            connect(filterFire, &QObject::destroyed, this, [&]() { qDebug() << "filterFire  был удален"; });
            connect(filterPolice, &QObject::destroyed, this, [&]() { qDebug() << "filterPolice  был удален"; });
            connect(filterMch, &QObject::destroyed, this, [&]() { qDebug() << "filterMch  был удален"; });
            filterLayout->setContentsMargins(1, 25, 1, 1);
            filterLayout->setSpacing(20);
            logAndInputLayout->setSpacing(1);
            logAndInputLayout->addLayout(nameLayout);
            logAndInputLayout->addLayout(handLayout);
            logAndInputLayout->addLayout(callAndUnitLayout);
            logAndInputLayout->addLayout(filterLayout);
            HlayoutButtons->addWidget(appointButton);
            HlayoutButtons->addWidget(cancelButton);
            HlayoutButtons->addWidget(statsButton);
            HlayoutButtons->addWidget(barButton);
            HlayoutGroup->addWidget(logAndInputGroupBox);
            HlayoutGroup->addLayout(HlayoutButtons);
            subWindow->setWidget(panelWidget);
            connect(subWindow, &QObject::destroyed, this, [&]() { qDebug() << "subWindow панель был удален"; });
            connect(cancelButton, &QPushButton::clicked, this, &MainWindow::slot_accept_cancel_button);
            connect(statsButton, &QPushButton::clicked, this, &MainWindow::slot_accept_stat_button);
            connect(barButton, &QPushButton::clicked, this, &MainWindow::slot_accept_bar_button);
        }
        else if (i == 1)
        {
            subWindow->setObjectName("brigades");
            subWindow->setWindowTitle("Бригады");
            connect(subWindow, &QObject::destroyed, this, [&]() { qDebug() << "subWindow Бригады был удален"; });
        }
        else if (i == 2)
        {
            subWindow->setObjectName("calls");
            subWindow->setWindowTitle("Вызовы");
            connect(subWindow, &QObject::destroyed, this, [&]() { qDebug() << "subWindow вызовы был удален"; });
        }
        else if (i == 3)
        {
            subWindow->setObjectName("dispatchers");
            subWindow->setWindowTitle("Диспетчеры");
            connect(subWindow, &QObject::destroyed, this, [&]() { qDebug() << "subWindow диспетчеры был удален"; });
        }
        else if (i == 4)
        {
            subWindow->setObjectName("brigadesOnCall");
            subWindow->setWindowTitle("Бригады на вызовах");
            connect(subWindow, &QObject::destroyed, this,
                    [&]() { qDebug() << "subWindow бригады на вызовах был удален"; });
        }
        else
        {
            subWindow->setObjectName("map");
            subWindow->setWindowTitle("Карта");
            connect(subWindow, &QObject::destroyed, this, [&]() { qDebug() << "subWindow карта был удален"; });
        }
        myArea->addSubWindow(subWindow);
    }
    auto List = myArea->subWindowList();
    for (auto& win : List)
    {
        if (win->objectName() == "calls")
        {
            callsView = new QTableView();
            callsView->setEditTriggers(QAbstractItemView::NoEditTriggers);
            callsView->setItemDelegateForColumn(12, delegate);
            callsView->setItemDelegateForColumn(13, delegate);
            win->setWidget(callsView);
        }
        else if (win->objectName() == "brigades")
        {
            brigadesView = new QTableView();
            brigadesView->setEditTriggers(QAbstractItemView::NoEditTriggers);
            win->setWidget(brigadesView);
        }
        else if (win->objectName() == "dispatchers")
        {
            dispatchesView = new QTableView();
            dispatchesView->setEditTriggers(QAbstractItemView::NoEditTriggers);
            win->setWidget(dispatchesView);
        }
        else if (win->objectName() == "brigadesOnCall")
        {
            brigadesOnCallView = new QTableView();
            brigadesOnCallView->setEditTriggers(QAbstractItemView::NoEditTriggers);
            win->setWidget(brigadesOnCallView);
        }
        else if (win->objectName() == "map")
        {
            win->setWidget(markerControl.getQuickWidget());
        }
    }
    areaLayout->addWidget(myArea);
    myArea->tileSubWindows();
    connect(uiConnect, &UiConnect::sig_sendStatusConnection, this, &MainWindow::slot_acceptStatusConnection);
    connect(uiConnect, &UiConnect::sig_sendMapModels, this, &MainWindow::slot_acceptMapModels);
    connect(simStart, &SimulatorStart::sig_sendMapDateOfCall, this, &MainWindow::slot_acceptMapDateOfCall);
    connect(uiConnect, &UiConnect::sig_resetCounterMarker, this, &MainWindow::slot_resetCounterMarker);
    connect(checkBoxOpenALL, &QCheckBox::stateChanged, this,
            [&](int state)
            {
                auto ListforAll = myArea->subWindowList();
                if (state == Qt::Checked)
                {
                    checkBoxOpenCalls->setCheckState(Qt::Checked);
                    checkBoxOpenCalls->setEnabled(false);
                    checkBoxOpenBrigades->setCheckState(Qt::Checked);
                    checkBoxOpenBrigades->setEnabled(false);
                    checkBoxOpenMap->setCheckState(Qt::Checked);
                    checkBoxOpenMap->setEnabled(false);
                    checkBoxOpenPanel->setCheckState(Qt::Checked);
                    checkBoxOpenPanel->setEnabled(false);
                    checkBoxOpenDispatchers->setCheckState(Qt::Checked);
                    checkBoxOpenDispatchers->setEnabled(false);
                    checkBoxOpenBrigadesOnCall->setCheckState(Qt::Checked);
                    checkBoxOpenBrigadesOnCall->setEnabled(false);
                    for (auto& win : ListforAll)
                    {
                        win->setWindowState(win->windowState() & ~Qt::WindowMinimized);
                    }
                }
                else if (state == Qt::Unchecked)
                {
                    checkBoxOpenCalls->setCheckState(Qt::Unchecked);
                    checkBoxOpenCalls->setEnabled(true);
                    checkBoxOpenBrigades->setCheckState(Qt::Unchecked);
                    checkBoxOpenBrigades->setEnabled(true);
                    checkBoxOpenMap->setCheckState(Qt::Unchecked);
                    checkBoxOpenMap->setEnabled(true);
                    checkBoxOpenPanel->setCheckState(Qt::Unchecked);
                    checkBoxOpenPanel->setEnabled(true);
                    checkBoxOpenDispatchers->setCheckState(Qt::Unchecked);
                    checkBoxOpenDispatchers->setEnabled(true);
                    checkBoxOpenBrigadesOnCall->setCheckState(Qt::Unchecked);
                    checkBoxOpenBrigadesOnCall->setEnabled(true);
                    for (auto& win : ListforAll)
                    {
                        win->setWindowState(win->windowState() | Qt::WindowMinimized);
                    }
                }
            });
    connect(checkBoxOpenCalls, &QCheckBox::stateChanged, this,
            [&](int state)
            {
                auto ListforAll = myArea->subWindowList();
                if (state == Qt::Checked)
                {
                    for (auto& win : ListforAll)
                    {
                        if (win->objectName() == "calls")
                        {
                            win->setWindowState(win->windowState() & ~Qt::WindowMinimized);
                            break;
                        }
                    }
                }
                else if (state == Qt::Unchecked)
                {
                    for (auto& win : ListforAll)
                    {
                        if (win->objectName() == "calls")
                        {
                            win->setWindowState(win->windowState() | Qt::WindowMinimized);
                            break;
                        }
                    }
                }
            });
    connect(checkBoxOpenBrigadesOnCall, &QCheckBox::stateChanged, this,
            [&](int state)
            {
                auto ListforAll = myArea->subWindowList();
                if (state == Qt::Checked)
                {
                    for (auto& win : ListforAll)
                    {
                        if (win->objectName() == "brigadesOnCall")
                        {
                            win->setWindowState(win->windowState() & ~Qt::WindowMinimized);
                            break;
                        }
                    }
                }
                else if (state == Qt::Unchecked)
                {
                    for (auto& win : ListforAll)
                    {
                        if (win->objectName() == "brigadesOnCall")
                        {
                            win->setWindowState(win->windowState() | Qt::WindowMinimized);
                            break;
                        }
                    }
                }
            });
    connect(checkBoxOpenPanel, &QCheckBox::stateChanged, this,
            [&](int state)
            {
                auto ListforAll = myArea->subWindowList();
                if (state == Qt::Checked)
                {
                    for (auto& win : ListforAll)
                    {
                        if (win->objectName() == "panelDispatcher")
                        {
                            win->setWindowState(win->windowState() & ~Qt::WindowMinimized);
                            break;
                        }
                    }
                }
                else if (state == Qt::Unchecked)
                {
                    for (auto& win : ListforAll)
                    {
                        if (win->objectName() == "panelDispatcher")
                        {
                            win->setWindowState(win->windowState() | Qt::WindowMinimized);
                            break;
                        }
                    }
                }
            });
    connect(checkBoxOpenBrigades, &QCheckBox::stateChanged, this,
            [&](int state)
            {
                auto ListforAll = myArea->subWindowList();
                if (state == Qt::Checked)
                {
                    for (auto& win : ListforAll)
                    {
                        if (win->objectName() == "brigades")
                        {
                            win->setWindowState(win->windowState() & ~Qt::WindowMinimized);
                            break;
                        }
                    }
                }
                else if (state == Qt::Unchecked)
                {
                    for (auto& win : ListforAll)
                    {
                        if (win->objectName() == "brigades")
                        {
                            win->setWindowState(win->windowState() | Qt::WindowMinimized);
                            break;
                        }
                    }
                }
            });
    connect(checkBoxOpenMap, &QCheckBox::stateChanged, this,
            [&](int state)
            {
                auto ListforAll = myArea->subWindowList();
                if (state == Qt::Checked)
                {
                    for (auto& win : ListforAll)
                    {
                        if (win->objectName() == "map")
                        {
                            win->setWindowState(win->windowState() & ~Qt::WindowMinimized);
                            break;
                        }
                    }
                }
                else if (state == Qt::Unchecked)
                {
                    for (auto& win : ListforAll)
                    {
                        if (win->objectName() == "map")
                        {
                            win->setWindowState(win->windowState() | Qt::WindowMinimized);
                            break;
                        }
                    }
                }
            });
    connect(checkBoxOpenDispatchers, &QCheckBox::stateChanged, this,
            [&](int state)
            {
                auto ListforAll = myArea->subWindowList();
                if (state == Qt::Checked)
                {
                    for (auto& win : ListforAll)
                    {
                        if (win->objectName() == "dispatchers")
                        {
                            win->setWindowState(win->windowState() & ~Qt::WindowMinimized);
                            break;
                        }
                    }
                }
                else if (state == Qt::Unchecked)
                {
                    for (auto& win : ListforAll)
                    {
                        if (win->objectName() == "dispatchers")
                        {
                            win->setWindowState(win->windowState() | Qt::WindowMinimized);
                            break;
                        }
                    }
                }
            });
    connect(buttonTitleSet, &QPushButton::clicked, this, [&]() { myArea->tileSubWindows(); });
    connect(simStart, &SimulatorStart::sig_sendMarksAllAppeared, this, &MainWindow::slot_acceptMarksAllAppeared);
    connect(uiConnect, &UiConnect::sig_haveSomeMarks, this, &MainWindow::slot_acceptHaveSomeMarks);
    connect(uiConnect, &UiConnect::sig_haveSomeDispatchers, this, &MainWindow::slot_acceptHaveSomeDispatchers);
    connect(dispatcherLoginWindow, &Dispatcherdate::sig_send_dispatcher_name, this,
            &MainWindow::slot_accept_dispatcher_name);
    callsView->verticalHeader()->setVisible(false);
    brigadesView->verticalHeader()->setVisible(false);
    dispatchesView->verticalHeader()->setVisible(false);
    brigadesOnCallView->verticalHeader()->setVisible(false);
    uiConnect->myConnect();
    connect(markerControl.getQuickWidget(), &QObject::destroyed, this, [&]() { qDebug() << "mapWidget был удален"; });
    connect(panelDispatherGroup, &QObject::destroyed, this,
            [&]() { qDebug() << "GroupBox panelDispatherGroup был удален"; });
    connect(dispatcherLoginWindow, &QObject::destroyed, this,
            [&]() { qDebug() << "GroupBox dispatcherLoginWindow был удален"; });
    connect(uiConnect, &UiConnect::sig_changeColor, this, &MainWindow::slot_accept_change_color);
    connect(uiConnect, &UiConnect::sig_removeMarker, this, &MainWindow::slot_accept_remove_marker);
    graficWidgetPie = new QWidget();
    graficWidgetPie->setFixedSize(600, 400);
    graficWidgetPie->setWindowTitle("Статистика");
    graficWidgetPie->setWindowModality(Qt::ApplicationModal);
    QHBoxLayout* layoutGraficPie = new QHBoxLayout(graficWidgetPie);
    chartPie = new QChart();
    chartViewPie = new QChartView(chartPie);
    seriesPie = new QPieSeries(this);
    chartPie->addSeries(seriesPie);
    chartPie->setTitle("Диаграмма работы диспетчера");
    chartViewPie->setRenderHint(QPainter::Antialiasing);
    layoutGraficPie->addWidget(chartViewPie);
    connect(chartPie, &QObject::destroyed, this, [&]() { qDebug() << "chartPie был удален"; });
    connect(chartViewPie, &QObject::destroyed, this, [&]() { qDebug() << "chartViewPie  был удален"; });
    connect(seriesPie, &QObject::destroyed, this, [&]() { qDebug() << "seriesPie  был удален"; });
    connect(layoutGraficPie, &QObject::destroyed, this, [&]() { qDebug() << "layoutGraficPie  был удален"; });
    connect(graficWidgetPie, &QObject::destroyed, this, [&]() { qDebug() << "graficWidgetPie  был удален"; });
    graficWidgetBar = new QWidget();
    graficWidgetBar->setFixedSize(700, 400);
    graficWidgetBar->setWindowTitle("Самые частые происшествия");
    graficWidgetBar->setWindowModality(Qt::ApplicationModal);
    QHBoxLayout* layoutGraficBar = new QHBoxLayout(graficWidgetBar);
    chartBar = new QChart();
    chartViewBar = new QChartView(chartBar);
    chartViewBar->setRenderHint(QPainter::Antialiasing);
    layoutGraficBar->addWidget(chartViewBar);
    seriesBar = new QBarSeries(this);
    seriesBar->setLabelsVisible(true);
    seriesBar->setLabelsPosition(QAbstractBarSeries::LabelsPosition::LabelsOutsideEnd);
    chartBar->setAnimationOptions(QChart::SeriesAnimations);
    chartBar->setTitle("Топ 5 просшествий по количеству");
    chartBar->legend()->setAlignment(Qt::AlignBottom);
    connect(chartBar, &QObject::destroyed, this, [&]() { qDebug() << "chartBar был удален"; });
    connect(chartViewBar, &QObject::destroyed, this, [&]() { qDebug() << "chartViewBar  был удален"; });
    connect(seriesBar, &QObject::destroyed, this, [&]() { qDebug() << "seriesBar  был удален"; });
    connect(graficWidgetBar, &QObject::destroyed, this, [&]() { qDebug() << "graficWidgetBar  был удален"; });
    connect(layoutGraficBar, &QObject::destroyed, this, [&]() { qDebug() << "layoutGraficBar  был удален"; });
}
MainWindow::~MainWindow()
{
    if (dispatcherLoginWindow)
    {
        delete dispatcherLoginWindow;
        dispatcherLoginWindow = nullptr;
    }
    if (chartPie)
    {
        delete chartPie;
        chartPie = nullptr;
    }

    if (chartBar)
    {
        delete chartBar;
        chartBar = nullptr;
    }

    if (graficWidgetPie)
    {
        delete graficWidgetPie;
        graficWidgetPie = nullptr;
    }

    if (graficWidgetBar)
    {
        delete graficWidgetBar;
        graficWidgetBar = nullptr;
    }

    if (msgAllMarks)
    {
        delete msgAllMarks;
        msgAllMarks = nullptr;
    }
    if (dispatcherWindow)
    {
        delete dispatcherWindow;
        dispatcherWindow = nullptr;
    }
    else
    {
        qDebug() << "dispatcherWindow уже был nullptr";
    }
    if (ui)
    {
        delete ui;
        ui = nullptr;
        qDebug() << "ui удалён";
    }
    else
    {
        qDebug() << "ui уже был nullptr";
    }
    if (uiConnect)
    {
        delete uiConnect;
        qDebug() << "uiConnect удалён";
    }
    else
    {
        qDebug() << "uiConnect уже был nullptr";
    }
    qDebug() << "MainWindow успешно разрушен";
}

void MainWindow::slot_accept_cancel_button()
{
    int num = numCallSpin->value();
    QString text = uiConnect->cancelButton(num);
    browserLog->append(text);
    qDebug() << "Слот для особождения вызова и бригады" << Qt::endl;
    if (text != "Ваш вызов уже выполнен" && text != "Ваш вызов и так свободен" &&
        text != "Ошибка, подробнее в отладке" && text != "Время назначения на вызов уже истекло")
    {

        markerControl.changeMarkerColor(QVariant(num - 1), "red");
        listOfTimers[num - 1]->start(TheCallIsUnsignedTime);
        finishedTimersUnsigned[num - 1].second = true;
    }
    else
    {
        qDebug() << "Упс отменить уже нельзя :)" << Qt::endl;
    }
}

void MainWindow::slot_accept_stat_button()
{
    static int counterStr = 0;
    auto Counts = uiConnect->getCountForStat();
    int done = Counts.done;
    int cancel = Counts.cancel;
    int notStart = Counts.notStart;
    if (done == 0 && cancel == 0 && notStart == 0)
    {
        qDebug() << "Упс, похоже данных для статистики нет :)" << Qt::endl;
        browserLog->append("Упс, похоже данных для статистики нет :)");
        if (counterStr++ == 5)
        {
            browserLog->clear();
            counterStr = 0;
        }
        return;
    }

    seriesPie->clear();
    seriesPie->append("Выполнено", done);
    seriesPie->append("Отмена", cancel);
    seriesPie->append("Не назначено", notStart);

    auto sliceDone = seriesPie->slices().at(0);
    auto sliceCancel = seriesPie->slices().at(1);
    auto sliceNotStart = seriesPie->slices().at(2);

    sliceDone->setBrush(Qt::green);
    sliceCancel->setBrush(Qt::red);
    sliceNotStart->setBrush(QColor(128, 0, 128));

    for (auto& slice : seriesPie->slices())
    {
        slice->setPen(Qt::NoPen);
        slice->setLabel(QString("%1: %2").arg(slice->label()).arg(slice->value()));
    }

    graficWidgetPie->show();
}

void MainWindow::slot_accept_bar_button()
{
    static int counterStr = 0;
    if (myMap["calls"]->rowCount() < 10)
    {
        qDebug() << "Нужно хотябы 10 вызовов для гистограммы :)" << Qt::endl;
        browserLog->append("Нужно хотябы 10 вызовов для гистограммы:)");
        if (counterStr++ == 5)
        {
            browserLog->clear();
            counterStr = 0;
        }
        return;
    }
    auto vec = uiConnect->getOftenBar();
    QStringList months;
    for (int i = 0; i < 5; i++)
    {
        months.append(vec.at(i).first);
    }
    if (!barSet)
    {
        barSet = new QBarSet("График вызовов", this);
        barSet->setLabelColor(Qt::black);
        for (int i = 0; i < 5; i++)
        {
            barSet->append(vec.at(i).second);
        }
        barSet->setColor(Qt::green);
        seriesBar->append(barSet);
        chartBar->addSeries(seriesBar);
    }
    else
    {
        for (int i = 0; i < 5; i++)
        {
            for (int i = 0; i < 5; i++)
            {
                barSet->replace(i, vec.at(i).second);
            }
        }
    }

    if (!barAxisY)
    {
        barAxisY = new QValueAxis(this);
        connect(barAxisY, &QObject::destroyed, this, [&]() { qDebug() << "barAxisY был удален"; });
        barAxisY->setTitleText("Количество");
        chartBar->addAxis(barAxisY, Qt::AlignLeft);
    }

    barAxisY->setRange(0, vec.at(0).second * 1.5);
    barAxisY->setLabelFormat("%.0f");
    barAxisY->setTickCount(5);

    if (!barAxisX)
    {
        barAxisX = new QBarCategoryAxis(this);
        connect(barAxisX, &QObject::destroyed, this, [&]() { qDebug() << "barAxisX был удален"; });
        barAxisX->append(months);
        chartBar->addAxis(barAxisX, Qt::AlignBottom);
    }

    seriesBar->attachAxis(barAxisX);
    seriesBar->attachAxis(barAxisY);
    graficWidgetBar->show();
}

void MainWindow::slot_accept_change_color(int index, QString status)
{
    markerControl.changeMarkerColor(QVariant(index), QVariant(status));
}

void MainWindow::slot_accept_remove_marker(int index)
{
    markerControl.removeMarker(QVariant(index));
}

void MainWindow::on_pushButton_start_simulator__clicked()
{
    if (ui->pushButton_start_simulator_->text() == "Запустить симуляцию")
    {
        if (simStart->getSizeProblems() == uiConnect->getCountRowsCalls())
        {
            msgAllMarks->exec();
            return;
        }

        if (allMarksAppeared)
        {
            msgAllMarks->exec();
            return;
        }
        ui->pushButton_start_simulator_->setText("Остановить симуляцию");
        simStart->runSimulator(true);
    }
    else
    {
        ui->pushButton_start_simulator_->setText("Запустить симуляцию");
        simStart->runSimulator(false);
    }
}

void MainWindow::on_pushButton_connection_settings__clicked()
{
    uiConnect->exec();
}

void MainWindow::slot_acceptStatusConnection(bool flag)
{
    connectionStatus = flag;
    int index = 0;
    if (!flag)
    {
        ui->pushButton_window_dispatcher_->setEnabled(false);
        ui->pushButton_dispatcher_date_->setEnabled(false);
        ui->pushButton_start_simulator_->setEnabled(false);
        simStart->runSimulator(false);
        ui->pushButton_start_simulator_->setText("Запустить симуляцию");
        for (auto& ptrTimer : listOfTimers)
        {
            if (ptrTimer != nullptr)
            {
                if (finishedTimersUnsigned[index].first == false && finishedTimersUnsigned[index].second == true)
                {
                    qDebug() << "Таймер для не назначенного вызова номер: " << index << " остановлен" << Qt::endl;
                    ptrTimer->stop();
                }
                else
                {
                    qDebug() << "Таймер для не назначенного вызова номер: " << index << " уже остановлен" << Qt::endl;
                }
            }
            else
            {
                qDebug() << "Таймер для не назначенного вызова номер: " << index << " равен nullptr" << Qt::endl;
            }
            index++;
        }
        return;
    }
    for (auto& ptrTimer : listOfTimers)
    {
        if (ptrTimer != nullptr)
        {
            if (finishedTimersUnsigned[index].first == false && finishedTimersUnsigned[index].second == true)
            {

                qDebug() << "Таймер для не назначенного вызова номер: " << index << " запущен" << Qt::endl;
                ptrTimer->start(TheCallIsUnsignedTime);
            }
            else
            {
                qDebug() << "Таймер для не назначенного вызова номер: " << index << " уже завершил работу" << Qt::endl;
            }
        }
        else
        {
            qDebug() << "Таймер для не назначенного вызова номер: " << index << " равен nullptr" << Qt::endl;
        }
        index++;
    }
    ui->pushButton_window_dispatcher_->setEnabled(true);
    ui->pushButton_dispatcher_date_->setEnabled(true);
    ui->pushButton_start_simulator_->setEnabled(true);
}

void MainWindow::on_pushButton_window_dispatcher__clicked()
{
    if (firstLaunch)
    {
        on_pushButton_dispatcher_date__clicked();
        firstLaunch = false;
    }
    dispatcherWindow->setWindowModality(Qt::ApplicationModal);
    dispatcherWindow->showMaximized();
}

void MainWindow::slot_acceptMapModels(QMap<QString, QSqlTableModel*> map)
{
    callsView->setModel(map["calls"]);
    callsView->setColumnWidth(0, 30);
    callsView->setColumnWidth(1, 230);
    callsView->setColumnWidth(2, 80);
    callsView->setColumnWidth(3, 70);
    callsView->setColumnWidth(4, 50);
    callsView->setColumnWidth(5, 80);
    callsView->setColumnWidth(6, 130);
    callsView->setColumnWidth(7, 130);
    callsView->setColumnWidth(8, 130);
    callsView->setColumnWidth(9, 130);
    callsView->setColumnWidth(10, 130);
    callsView->setColumnWidth(11, 130);
    callsView->setColumnWidth(12, 130);
    callsView->setColumnWidth(13, 130);
    callsView->setColumnWidth(14, 130);
    callsView->setColumnWidth(15, 130);
    dispatchesView->setModel(map["dispatchers"]);
    dispatchesView->setColumnWidth(0, 30);
    dispatchesView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    dispatchesView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    brigadesView->setModel(map["brigades"]);
    brigadesView->setColumnWidth(0, 30);
    brigadesView->setColumnWidth(1, 170);
    brigadesView->setColumnWidth(2, 170);
    brigadesView->setColumnWidth(3, 170);
    brigadesView->setColumnWidth(4, 170);
    brigadesView->setColumnWidth(5, 170);
    brigadesView->setColumnWidth(6, 170);
    brigadesView->setColumnWidth(7, 170);
    brigadesView->setColumnWidth(8, 170);
    brigadesView->setColumnWidth(9, 170);
    brigadesOnCallView->setModel(map["brigadesOnCall"]);
    brigadesOnCallView->setColumnWidth(0, 30);
    brigadesOnCallView->setColumnWidth(1, 60);
    brigadesOnCallView->setColumnWidth(2, 55);
    brigadesOnCallView->setColumnWidth(3, 100);
    brigadesOnCallView->setColumnWidth(4, 100);
    brigadesOnCallView->setColumnWidth(5, 100);
    brigadesOnCallView->setColumnWidth(6, 100);
    brigadesOnCallView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    brigadesOnCallView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    brigadesOnCallView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
    brigadesOnCallView->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Fixed);
    brigadesOnCallView->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Fixed);
    brigadesOnCallView->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Fixed);
    myMap = map;
}

void MainWindow::slot_acceptMapDateOfCall(QMap<QString, QString> mapDateOfCall)
{
    int codeResult = uiConnect->setMapDateOfCall(mapDateOfCall);
    int flag = 1;
    if (codeResult == 0)
    {
        markerControl.addMarker(QVariant(mapDateOfCall["latitude"]), QVariant(mapDateOfCall["longitude"]),
                                QVariant(mapDateOfCall["address"]), QVariant(mapDateOfCall["problem"]),
                                QVariant(mapDateOfCall["status"]), QVariant(counterMarker), QVariant(flag));

        listOfTimers.resize(listOfTimers.size() + 1);
        finishedTimersUnsigned.resize(finishedTimersUnsigned.size() + 1);
        listOfTimers[counterMarker] = new QTimer(this);
        finishedTimersUnsigned[counterMarker].first = false;
        finishedTimersUnsigned[counterMarker].second = true;
        qDebug() << "Выделяем память для таймера (не назначенного вызова) : " << counterMarker << Qt::endl;
        listOfTimers[counterMarker]->setObjectName(QString::number(counterMarker));
        connect(listOfTimers[counterMarker], &QObject::destroyed, this,
                [](QObject* obj) { qDebug() << "Таймер для не назначенного вызова был удалён:" << obj->objectName(); });
        listOfTimers[counterMarker]->setSingleShot(true);
        listOfTimers[counterMarker]->start(TheCallIsUnsignedTime);
        int tempCount = counterMarker;
        connect(listOfTimers[counterMarker], &QTimer::timeout, this, [=]() { slot_accept_list_timers(tempCount); });
        counterMarker++;
        simStart->counterPlusPlus();
    }
}

void MainWindow::slot_accept_list_timers(int call)
{
    qDebug() << "Меняем маркер номер: " << call + 1 << " " << "на фиолетовый цвет" << Qt::endl;
    finishedTimersUnsigned[call].first = true;
    markerControl.changeMarkerColor(QVariant(call), QVariant("Не назначен"));
    uiConnect->setTimerCallFail(call + 1);
}

void MainWindow::slot_resetCounterMarker()
{
    simStart->resetDigitsRandom();
    markerControl.removeAllMarkers();
    counterMarker = 0;
    allMarksAppeared = false;
    simStart->setCounter(0);
    dispatcherLoginWindow->clearList();
    firstLaunch = true;
    filterAll->setChecked(true);
    int index = 0;
    for (auto& ptrTimer : listOfTimers)
    {
        if (ptrTimer != nullptr)
        {
            delete ptrTimer;
            ptrTimer = nullptr;
        }
        else
        {
            qDebug() << "Таймер для не назначенного вызова  равен nullptr: " << index << Qt::endl;
        }
        index++;
    }
    listOfTimers.clear();
    finishedTimersUnsigned.clear();
}

void MainWindow::slot_acceptMarksAllAppeared()
{
    allMarksAppeared = true;
    ui->pushButton_start_simulator_->setText("Запустить симуляцию");
    msgAllMarks->show();
}

void MainWindow::slot_acceptHaveSomeMarks()
{
    int counter = 0;
    auto table = myMap["calls"];
    int countRows = table->rowCount();
    for (int i = 0; i < countRows; i++)
    {
        auto record = table->record(i);
        int flag = 1;
        if (record.value(2).toString() == "Выполнен")
        {
            flag = 2;
        }

        markerControl.addMarker(QVariant(record.value(12)), QVariant(record.value(13)), QVariant(record.value(6)),
                                QVariant(record.value(1)), QVariant(record.value(2)), QVariant(counter++),
                                QVariant(flag));
    }
    counterMarker = counter;
    simStart->setCounter(counter);
    auto map = uiConnect->getBeginStatus();
    QVector<int> checkStatusUnsignedVector;
    checkStatusUnsignedVector.resize(myMap["calls"]->rowCount());
    QMap<int, QString>::const_iterator i;
    int index = 0;
    for (i = map.constBegin(); i != map.constEnd(); ++i)
    {
        qDebug() << i.key() << ": " << i.value();
        if (i.value() == "Не назначен")
        {
            checkStatusUnsignedVector[index] = 2;
        }
        markerControl.changeMarkerColor(QVariant(i.key()), i.value());
        index++;
    }
    listOfTimers.resize(counterMarker, nullptr);
    finishedTimersUnsigned.resize(counterMarker);
    for (int i = 0; i < counterMarker; i++)
    {
        bool right = uiConnect->getCheckIndexRight(i);
        listOfTimers[i] = new QTimer(this);
        listOfTimers[i]->setObjectName(QString::number(i));
        listOfTimers[i]->setSingleShot(true);
        connect(listOfTimers[i], &QObject::destroyed, this,
                [](QObject* obj) { qDebug() << "Таймер для не назначенного вызова был удалён:" << obj->objectName(); });
        connect(listOfTimers[i], &QTimer::timeout, this, [=]() { slot_accept_list_timers(i); });
        if (right)
        {
            if (checkStatusUnsignedVector[i] != 2)
            {
                finishedTimersUnsigned[i].first = false;
                finishedTimersUnsigned[i].second = true;
                qDebug() << "Выделяем память для таймера (не назначенного вызова) : " << i << Qt::endl;
                listOfTimers[i]->start(TheCallIsUnsignedTime);
            }
        }
    }
}

void MainWindow::slot_acceptHaveSomeDispatchers()
{
    dispatcherLoginWindow->setModel(myMap["dispatchers"]);
}

void MainWindow::slot_appointButton_clicked()
{
    int callIndex = numCallSpin->value() - 1;
    int UnitIndex = numUnitSpin->value() - 1;
    auto calls = myMap["calls"];
    auto units = myMap["brigades"];
    int rowCalls = calls->rowCount();
    int rowUnits = units->rowCount();
    if (countLog++ > 8)
    {
        browserLog->clear();
        countLog = 0;
    }
    if (rowCalls == 0)
    {
        browserLog->append("Таблица с вызовами пустая");
        return;
    }
    if (rowUnits == 0)
    {
        browserLog->append("Таблица с бригадами пустая");
        return;
    }
    if (callIndex >= rowCalls)
    {
        browserLog->append("Вызова с таким индексом не существует");
        return;
    }
    uiConnect->checkIndexs(callIndex + 1, UnitIndex + 1);
    bool right = uiConnect->getCheckIndexRight(callIndex);
    if (!right)
    {
        listOfTimers[callIndex]->stop();
        finishedTimersUnsigned[callIndex].second = false;
    }
}

void MainWindow::on_pushButton_dispatcher_date__clicked()
{
    dispatcherLoginWindow->exec();
    firstLaunch = false;
}

void MainWindow::slot_accept_dispatcher_name(QString name, bool flag)
{
    if (flag)
    {
        uiConnect->addDispatcher(name);
    }
    currentUser->setText(name);
    uiConnect->setCurrentDispatcher(name);
}

void MainWindow::slot_accept_filter(int num)
{
    qDebug() << "Слот фильтра" << Qt::endl;
    if (num == FILTER_TYPE::ALL)
    {
        myMap["brigades"]->setFilter("");
    }
    else if (num == FILTER_TYPE::MCH)
    {
        myMap["brigades"]->setFilter(QString("type = '%1'").arg("Мчс"));
    }
    else if (num == FILTER_TYPE::AMBULANCE)
    {
        myMap["brigades"]->setFilter(QString("type = '%1'").arg("Скорая"));
    }
    else if (num == FILTER_TYPE::POLICE)
    {
        myMap["brigades"]->setFilter(QString("type = '%1'").arg("Полиция"));
    }
    else if (num == FILTER_TYPE::FIRE)
    {
        myMap["brigades"]->setFilter(QString("type = '%1'").arg("Пожарные"));
    }
    myMap["brigades"]->select();
}
