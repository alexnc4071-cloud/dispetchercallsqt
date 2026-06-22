#include "norounddelegate.h"

NoRoundDelegate::NoRoundDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

QString NoRoundDelegate::displayText(const QVariant& value, const QLocale& locale) const
{
    if (value.canConvert<QString>())
    {
        return value.toString();
    }
    return QStyledItemDelegate::displayText(value, locale);
}
