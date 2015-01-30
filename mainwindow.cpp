#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "spliter.h"
#include "size.h"

#include <QFileDialog>
#include <QDir>
#include <QThread>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->radioButton,SIGNAL(clicked()),this,SLOT(refreshEnabled()));
    connect(ui->radioButton_2,SIGNAL(clicked()),this,SLOT(refreshEnabled()));
    spliter = NULL;
    thread = NULL;
}

MainWindow::~MainWindow()
{
    delete ui;
    if (spliter) {
        delete(spliter);
        spliter = NULL;
    }
}

void MainWindow::on_actionQuitter_triggered()
{
    exit(0);
}

void MainWindow::on_pushButton_clicked() {
    QFileDialog f(this);
    f.exec();
    f.show();

    if (!f.result()) return;
    ui->fileSource->setText(f.selectedFiles().first());
    //si le fichier selectioné est un fichier découpé, on passe automatiquement en mode recolage
    if (ui->fileSource->text().right(5) == ".x000") {
        ui->radioButton_2->setChecked(true);
    }
    //si on est en mode recolage: on remlis automatiquement le chemin du fichier de destination.
    if (ui->radioButton_2->isChecked()) {
        int end = ui->fileSource->text().length() - 5;
        if (end > 0) ui->fileDest->setText(ui->fileSource->text().mid(0,end));
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    //choix de la destination
    QFileDialog f(this);

    //pour le cas d'un découpage, on invite à choisir des fichiers et non des dossiers
    if (ui->radioButton->isChecked()) f.setFileMode(QFileDialog::DirectoryOnly);

    f.exec();
    f.show();

    if (!f.result()) return;
    ui->fileDest->setText(f.selectedFiles().first());
}

void MainWindow::on_pushButton_3_clicked() {
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(10000);

    QString sizeString = ui->lineEdit->text();
    if (sizeString.right(1) != "b") sizeString.append("b");
    const quint64 size = Size::getRsize(sizeString);

    ui->pushButton_3->setEnabled(false);

    QThread *thread = new QThread(this);
    this->thread = thread;

    //on créé le spliter
    spliter = new Spliter(ui->fileSource->text(),
                          ui->fileDest->text(),
                          size,
                          thread,
                          this);

    //connections des différents signaux/slots du spliter
    connect(spliter,SIGNAL(debug(QString)),ui->statusBar,SLOT(showMessage(QString)));
    connect(spliter,SIGNAL(finished()),this,SLOT(cleanSpliter()));
    connect(spliter,SIGNAL(position(int)),ui->progressBar,SLOT(setValue(int)));
    connect(spliter,SIGNAL(debug(QString)),this,SLOT(debug(QString)));

    //cas d'un découpage
    if (ui->radioButton->isChecked()) {
        if (!size) {
            ui->statusBar->showMessage("erreur: taille demandée invalide");
            ui->pushButton_3->setEnabled(true);
            return cleanSpliter();
        }
        else if (size < 2097152) {
            ui->pushButton_3->setEnabled(true);
            ui->statusBar->showMessage("erreur: taille demandée trop petite, minimul 2Mb");
            return cleanSpliter();
        }
        connect(thread,SIGNAL(started()),spliter,SLOT(split()));
    }

    //Cas d'un rassemblement
    else if (ui->radioButton_2->isChecked()) {
        //on lorsque le thread sera démaré on lance le rassemblement des morceaux.
        connect(thread,SIGNAL(started()),spliter,SLOT(join()));
    }
    //si aucun des deux radios n'est checké: on annule tout.
    else return cleanSpliter();

    //suppression du parent pour pouvoir le déplacacer dans un thread séparé
    spliter->setParent(NULL);
    spliter->moveToThread(thread);
    thread->start();
}
void MainWindow::cleanSpliter() {
    spliter->deleteLater();
    spliter = NULL;
    thread->deleteLater();
    thread = NULL;
    ui->pushButton_3->setEnabled(true);
}
void MainWindow::refreshEnabled() {
    if (ui->radioButton->isChecked()) ui->groupBox_3->show();
    if (ui->radioButton_2->isChecked()) ui->groupBox_3->hide();
}
void MainWindow::debug(const QString message) {
    qDebug() << message;
}
