#ifndef NOROUNDDELEGATE_H
#define NOROUNDDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>

class NoRoundDelegate : public QStyledItemDelegate
{
  public:
    NoRoundDelegate(QObject* parent = nullptr);
    QString displayText(const QVariant& value, const QLocale& locale) const override;
};

#endif // NOROUNDDELEGATE_H
