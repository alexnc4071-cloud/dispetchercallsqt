import QtQuick 2.15
import QtQuick.Controls 2.15
import QtLocation 6.1
import QtPositioning 6.1
import QtQuick.Window 2.0

Item {

    Plugin {
        id: mapPlugin
        name: "osm"
    }
    property var markers: []
    Map {
        id: mapWidget
        anchors.fill: parent
        plugin: mapPlugin
        center: QtPositioning.coordinate(51.5532, 45.9998)
        zoomLevel: 5

        MouseArea {
            anchors.fill: parent
            drag.target: null
            property var previousCenter: mapWidget.center
            property bool isDragging: false
            property var startPoint: Qt.point(0, 0)
            property real dragSensitivity: 0.03

            onPressed: (mouse) => {
                           isDragging = true
                           previousCenter = mapWidget.center
                           startPoint = Qt.point(mouse.x, mouse.y)
                       }
            onReleased: {
                isDragging = false
            }
            onPositionChanged: (mouse) => {
                                   if (isDragging) {
                                       var deltaX = startPoint.x - mouse.x
                                       var deltaY = startPoint.y - mouse.y
                                       deltaX *= dragSensitivity
                                       deltaY *= dragSensitivity

                                       var metersPerPixel = 156543.03392 * Math.cos(mapWidget.center.latitude * Math.PI / 180) / Math.pow(2, mapWidget.zoomLevel)
                                       var deltaLon = deltaX * metersPerPixel / (111320 * Math.cos(mapWidget.center.latitude * Math.PI / 180))
                                       var deltaLat = -deltaY * metersPerPixel / 111320

                                       mapWidget.center = QtPositioning.coordinate(
                                           previousCenter.latitude + deltaLat,
                                           previousCenter.longitude + deltaLon
                                           )
                                   }
                               }
        }

        WheelHandler {
            onWheel: (wheelEvent) => {
                         if (wheelEvent.angleDelta.y > 0) {
                             if (mapWidget.zoomLevel < 20)
                             mapWidget.zoomLevel += 1
                         } else {
                             if (mapWidget.zoomLevel > 1)
                             mapWidget.zoomLevel -= 1
                         }
                     }
        }

    }

    function checkColor(status){

        var color = "red";
        if (status === "Занята") {
            color = "yellow";
        } else if (status === "В пути") {
            color = "blue";
        } else if (status === "На месте") {
            color = "orange";
        } else if (status === "Команда выполнела задачу") {
            color = "lightgreen";
        } else if (status === "Свободна") {
            color = "lightgreen";
        } else if (status === "Не назначен") {
            color = "purple";
        }
        return color;
    }

    function addMarker(lat, lon, address, description, status, number, flag) {
        number = number + 1;
        var color = "red";
        var dotName = "dotRect_" + number; // Уникальное имя

        var marker = Qt.createQmlObject(
                    'import QtLocation 6.1; import QtQuick 2.15; import QtPositioning 6.1;\
            MapQuickItem { \
                property string dotObjectName: "' + dotName + '";
                anchorPoint.x: 0.5; anchorPoint.y: 1.0; \
                coordinate: QtPositioning.coordinate(' + lat + ', ' + lon + '); \
                sourceItem: Item { \
                    width: 120; height: 60; \
                    Rectangle { \
                        objectName: "' + dotName + '"; \
                        width: 10; height: 10; color: "' + color + '"; \
                        border.color: "black"; border.width: 1; \
                        radius: width / 2; \
                        anchors.verticalCenter: parent.verticalCenter; \
                        anchors.left: parent.left; \
                    } \
                    Column { \
                        anchors.left: parent.left; \
                        anchors.leftMargin: 15; \
                        anchors.verticalCenter: parent.verticalCenter; \
                        spacing: 1; \
                        Text { text: "' + address + '"; color: "black"; font.pixelSize: 8; } \
                        Text { text: "' + description + '"; color: "green"; font.pixelSize: 8; } \
                        Text { text: "' + number + '"; color: "green"; font.pixelSize: 8; } \
                    } \
                } \
            }',
                    mapWidget
                    );

        marker.dotObjectName = dotName; // запоминаем имя внутри объекта
        if (flag !== 2) {
            mapWidget.addMapItem(marker);}
        markers.push(marker);
    }



    function changeMarkerColor(index, status) {
        var newColor = checkColor(status);
        if (index >= 0 && index < markers.length) {
            var marker = markers[index];
            if (marker && marker.sourceItem && marker.dotObjectName) {
                var children = marker.sourceItem.children;
                for (var i = 0; i < children.length; i++) {
                    if (children[i].objectName === marker.dotObjectName) {
                        children[i].color = newColor;
                        console.log("Изменён цвет на:", children[i].color);
                        return;
                    }
                }
                console.warn("Не найден объект с objectName:", marker.dotObjectName);
            }
        }
    }

    function removeMarker(index) {
        var expectedDotName = "dotRect_" + (index + 1);  // формируем имя
        for (var i = 0; i < markers.length; i++) {
            var marker = markers[i];
            if (marker.dotObjectName === expectedDotName) {
                console.log("Удаляется маркер с именем:", expectedDotName);
                mapWidget.removeMapItem(marker);
                return;  // удалили — выходим
            }
        }
        console.warn("Маркер с именем", expectedDotName, "не найден");
    }

    function removeAllMarkers() {
        var count = markers.length;
        if (count > 0) {
            for (var i = count - 1; i >= 0; i--) {
                mapWidget.removeMapItem(markers[i]);
                markers[i].destroy();
                markers.splice(i, 1);
            }
        }
    }

    // Кнопки увеличения/уменьшения масштаба
    Button {
        text: "+"
        width: 50
        height: 50
        anchors.right: parent.right
        anchors.top: parent.top
        onClicked: {
            if (mapWidget.zoomLevel < 20)
                mapWidget.zoomLevel += 1;
        }
    }

    Button {
        text: "-"
        width: 50
        height: 50
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: 70
        onClicked: {
            if (mapWidget.zoomLevel > 1)
                mapWidget.zoomLevel -= 1;
        }
    }
}
