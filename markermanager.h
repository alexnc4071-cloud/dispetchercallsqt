#ifndef MARKERMANAGER_H
#define MARKERMANAGER_H
#include <QObject>
#include <QQmlContext>
#include <QQuickItem>
#include <QtQuickWidgets/QQuickWidget>
class MarkerManager : public QObject
{
    Q_OBJECT
  public:
    MarkerManager();
    ~MarkerManager();
    QWidget* getQuickWidget();
    void addMarker(QVariant lat, QVariant lon, QVariant address, QVariant description, QVariant status, QVariant number,
                   QVariant flag);
    void removeAllMarkers();
    void changeMarkerColor(QVariant index, QVariant status);
    void removeMarker(QVariant index);
  signals:
    void sig_markerAdded(QVariant lat, QVariant lon, QVariant address, QVariant description, QVariant status,
                         QVariant number, QVariant flag);
    void sig_removeAllMarkers();
    void sig_changeMarkerColor(QVariant index, QVariant status);
    void sig_removeMarker(QVariant index);

  private:
    QQuickWidget* mapWidget;
    bool existMarks = false;
};
#endif // MARKERMANAGER_H
