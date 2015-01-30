#include "spliter.h"
#include <QFile>
#include <QStringList>
#include <QDir>
#include <QDebug>

Spliter::Spliter(const QString sourceFile, const QString destination, const quint64 size, QThread *thread, QObject* parent) :
    QObject(parent)
{
    this->source = sourceFile;
    this->dest = destination;
    this->size = size;
    this->thread = thread;
    blockSize = 2097152; //2Mb;
}
Spliter::~Spliter() {
}

void Spliter::split() {
    QFile file(source);
    QString rawName = source.split('/').last();
    const quint64 fileSize = file.size();

    int counter = 0;
    QString destFilePath;
    QFile dfile;

    emit(debug("opening file"));
    file.open(QIODevice::ReadOnly);
    quint64 pos = 0;

    while (pos != fileSize) {
        destFilePath = dest + "/" + rawName + ".x" + QString::number(counter++).rightJustified(3,QChar('0'));
        emit(debug("Traitement de " + destFilePath));

        //écriture du bloc dans un nouveau fichier
        dfile.setFileName(destFilePath);
        dfile.open(QIODevice::WriteOnly);

        //on lis un block tant que la taille du fichier n'a pas été atteinte
        for (quint64 subpos = 0 ; subpos < size ; subpos += blockSize) {
            pos += dfile.write(file.read(blockSize));
            emit(position(pos / fileSize * (quint64) 10000));
        }
        dfile.close();
    }

    emit(debug("closing file"));
    file.close();

    emit(debug("fini."));
    emit(finished());
}
QString Spliter::getJoinRootFileName() {
    int endpos = source.length() - 3;
    if (endpos < 0) {
        return QString();
    }
    return source.mid(0,endpos);

}

void Spliter::join() {
    //fonction de recolage des morceaux de fichiers
    QString root = this->getJoinRootFileName();
    if (root.isEmpty()) {
        emit(finished());
        return;
    }

    //on déclare ici nos objets pour éviter les re-alocations inutilse
    QFile file;
    QFile fileDest(dest);
    if (!fileDest.open(QIODevice::WriteOnly)) {
        emit(debug("erreur: impossible d'ouvrir le fichier de destination"));
        return;
    }

    int readed = 0;
    //je passe par un buffer temporaire pour éviter les re-alocations de mémoires que causeraient un QByteArray à chaque lecture via QFile::read()
    char block[blockSize];

    //fileSize sera redéfinis à chaque fichier
    quint64 fileSize;

    //currentPos contiens la position écrite actuele dans le fichier de destination (incrémentation au write)
    quint64 currentPos = 0;

    //on récupere la liste des fichiers tronqués
    QStringList filesList = this->getDestinationFilesList();
    QStringList::Iterator i;
    qDebug() << filesList;

    //ici la taille totale des fichiers tronqués
    quint64 totalSize = getSplitedSize();
    emit(position(0));

    emit(totalBytes(totalSize));
    for (i = filesList.begin() ; i != filesList.end() ; i++) {
        file.setFileName(*i);
        emit(debug("Traitement de: " + file.fileName()));
        if (!file.open(QIODevice::ReadOnly)) emit(debug("erreur: impossible d'ouvrir le fichier: " + file.fileName()));
        fileSize = file.size();
        //ici on effectue une lecture par blocks (pour ne pas se retrouver à stoquer des fichier de 4gb+ en mémoire tampon)
        for (quint64 filePos = 0 ; filePos < fileSize ; filePos += blockSize) {

            //readed contiens le nombre de données lues sur le segment actuel
            readed = file.read(block,blockSize);
            currentPos += readed;

            //si une erreur de lecture est survenue
            if (readed <= 0) {
                emit(debug("erreur: lecture: " + file.errorString()));
                break;
            }

            //si les données écrites ne correspondent pas aux données lues
            else if (fileDest.write(block,blockSize) != readed) {
                emit(debug("erreur: écriture: " + file.errorString()));
                break;
            }

            //si aucune erreur n'a eut lieux: on envoi la position actuelle
            else {
                //apparement je n'arrive pas dans cette condition
                int bpos = (float) ((float) currentPos / (float) totalSize) * (quint64) 10000;
                qDebug() << bpos << currentPos << totalSize << file.fileName();
                emit(position(bpos));
            }

        }
        //ici on à finis la lecture d'un des fichier splité
        file.close();
    }
    fileDest.close();
    emit(debug("fini"));
    emit(finished());
}
qint64 Spliter::getSplitedSize() {
    emit(debug("calcul de la taille totale"));
    QStringList files = getDestinationFilesList();
    qint64 size = 0;
    QFile file(this);
    QStringList::Iterator i;
    for (i = files.begin() ; i != files.end() ; i++) {
        file.setFileName(*i);
        size += file.size();
    }
    return size;
}
QStringList Spliter::getDestinationFilesList() {
    emit(debug("recherche des fichiers tronqués"));
    QStringList files;
    QString root = getJoinRootFileName();
    QFile file(this);
    int counter = 0;
    if (root.isEmpty()) return files;
    do {
        //je n'utilise pas QDir::filesEntry pour une histoire d'optimisation et pour gerer plusieurs .x000 dans le même dossier
        file.setFileName(root + QString::number(counter++).rightJustified(3,QChar('0')));
        files << file.fileName();
    } while (file.exists());
    files.removeLast();
    return files;
}
