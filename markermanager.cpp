#include "markermanager.h"
#include <qobject.h>

MarkerManager::MarkerManager()
{
    mapWidget = new QQuickWidget;
    mapWidget->rootContext()->setContextProperty("markerManager", this);
    mapWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    mapWidget->setSource(QUrl(QStringLiteral("qrc:/map.qml")));
    QObject* qmlRoot = mapWidget->rootObject();
    QObject::connect(this, &MarkerManager::sig_markerAdded, qmlRoot,
                     [qmlRoot](QVariant lat, QVariant lon, QVariant address, QVariant description, QVariant status,
                               QVariant number, QVariant flag)
                     {
                         QMetaObject::invokeMethod(qmlRoot, "addMarker", Q_ARG(QVariant, lat), Q_ARG(QVariant, lon),
                                                   Q_ARG(QVariant, address), Q_ARG(QVariant, description),
                                                   Q_ARG(QVariant, status), Q_ARG(QVariant, number),
                                                   Q_ARG(QVariant, flag));
                     });
    QObject::connect(this, &MarkerManager::sig_removeAllMarkers, qmlRoot,
                     [qmlRoot]() { QMetaObject::invokeMethod(qmlRoot, "removeAllMarkers"); });
    QObject::connect(
        this, &MarkerManager::sig_changeMarkerColor, qmlRoot, [qmlRoot](QVariant index, QVariant status)
        { QMetaObject::invokeMethod(qmlRoot, "changeMarkerColor", Q_ARG(QVariant, index), Q_ARG(QVariant, status)); });
    QObject::connect(this, &MarkerManager::sig_removeMarker, qmlRoot, [qmlRoot](QVariant index)
                     { QMetaObject::invokeMethod(qmlRoot, "removeMarker", Q_ARG(QVariant, index)); });
}

void MarkerManager::changeMarkerColor(QVariant index, QVariant status)
{
    qDebug() << "Меняем цвет маркера номер: " << index.toInt() + 1 << Qt::endl;
    emit sig_changeMarkerColor(index, status);
}

void MarkerManager::removeMarker(QVariant index)
{
    qDebug() << "Удаляем маркер номер: " << index.toInt() + 1 << Qt::endl;
    emit sig_removeMarker(index);
}

void MarkerManager::addMarker(QVariant lat, QVariant lon, QVariant address, QVariant description, QVariant status,
                              QVariant number, QVariant flag)
{
    qDebug() << "Вызван метод addMarker с параметрами:";
    qDebug() << "  Широта (lat):" << lat;
    qDebug() << "  Долгота (lon):" << lon;
    qDebug() << "  Адрес:" << address;
    qDebug() << "  Описание:" << description;
    qDebug() << "  Номер:" << number;
    qDebug() << "  Статус:" << status;
    // Испускаем сигнал
    emit sig_markerAdded(lat, lon, address, description, status, number, flag);
    // Подтверждение отправки сигнала
    qDebug() << "Сигнал markerAdded успешно отправлен!" << Qt::endl;
    existMarks = true;
}

void MarkerManager::removeAllMarkers()
{
    if (existMarks)
    {
        emit sig_removeAllMarkers();
        qDebug() << "Сигнал sig_removeAllMarkers успешно отправлен!" << Qt::endl;
        existMarks = false;
    }
    else
    {
        qDebug() << "В базе нет меток для удаления!" << Qt::endl;
    }
}

MarkerManager::~MarkerManager()
{
    removeAllMarkers();
}

QWidget* MarkerManager::getQuickWidget()
{
    return mapWidget;
}
