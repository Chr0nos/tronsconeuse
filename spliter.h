#ifndef SLPITER_H
#define SLPITER_H

#include <QThread>
#include <QString>

class Spliter : public QObject
{
    Q_OBJECT
public:
    explicit Spliter(const QString sourceFile,const QString destination,const quint64 size,QThread* thread,QObject* parent = 0);
    ~Spliter();
    qint64 getSplitedSize();
private:
    QString source;
    QString dest;
    quint64 size;
    QThread* thread;
    QStringList getDestinationFilesList();
    QString getJoinRootFileName();
    int blockSize;

signals:
    void debug(const QString message);
    void finished();
    void totalBytes(const qint64 size);
    //emmet une position entre 0 et 10000 (pour éviter les quint64 sur les progressbar)
    void position(const int pos);

public slots:
    void split();
    void join();
};

#endif // SLPITER_H
