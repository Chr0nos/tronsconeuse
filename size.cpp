#include "size.h"
#include <QStringList>

Size::Size()
{
}

QString Size::getWsize(const quint64 size, const int steps) {
    //this function convert a bytes size into an human readble size
    double isize = size;
    QStringList keys = Size::getUnits();
    int n;
    for (n = 0;isize >= steps;n++) isize /= steps;
    if (n >= keys.count()) n = keys.count() -1;
    return QString::number(isize,10,2) + keys.at(n);
}
quint64 Size::getRsize(const QString wsize, const int steps) {
    //this method convert an humain size to a bytes size
    QStringList keys = Size::getUnits();
    for (int p = keys.count() -1;p;p--) {
        const int len = keys.at(p).length();
        const int sizeEndPos = wsize.length() - len;
        //prevent empty sizes like "b" (withous any numbers)
        if (!sizeEndPos) return 0;
        if (wsize.right(len) == keys.at(p)) {
            QString number = wsize.mid(0,sizeEndPos);
            double size = number.toDouble();
            while (p--) size *= steps;
            return size;
        }
    }

    return 0;
}
QStringList Size::getUnits() {
    QStringList keys;
    keys << "b" << "Kb" << "Mb" << "Gb" << "Tb" << "Pb" << "Eb" << "Zb" << "Yb";
    return keys;
}
