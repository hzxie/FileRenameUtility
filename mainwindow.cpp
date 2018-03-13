#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <cmath>

#include <QDesktopServices>
#include <QFileDialog>
#include <QMap>
#include <QMessageBox>
#include <QStandardPaths>
#include <QStringList>
#include <QTreeWidgetItem>
#include <QUuid>

/*!
 * \brief MainWindow::MainWindow
 * \param parent
 */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);
    initSignalsAndSlots();
}

/*!
 * \brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow() {
    delete ui;
}

/*!
 * \brief MainWindow::initSignalsAndSlot
 */
void MainWindow::initSignalsAndSlots() {
    connect(ui->pushButtonBrowse, &QPushButton::clicked,
            this, &MainWindow::pushButtonBrowseClicked);
    connect(ui->pushButtonPreview, &QPushButton::clicked,
            this, &MainWindow::pushButtonPreviewClicked);
    connect(ui->pushButtonProcess, &QPushButton::clicked,
            this, &MainWindow::pushButtonProcessClicked);
    connect(ui->actionExit, &QAction::triggered,
            this, &MainWindow::actionExitTriggered);
    connect(ui->actionAboutUs, &QAction::triggered,
            this, &MainWindow::actionAboutUsTriggered);
}

/*!
 * \brief MainWindow::pushButtonBrowseClicked
 */
void MainWindow::pushButtonBrowseClicked() {
    QString defaultDirectory = ui->lineEditDirectoryName->text();
    if ( defaultDirectory.isEmpty() ) {
        defaultDirectory = QStandardPaths::PicturesLocation;
    }

    QString file = QFileDialog::getExistingDirectory(
                    this,
                    tr("Choose Directory"),
                    defaultDirectory,
                    QFileDialog::ShowDirsOnly |
                    QFileDialog::DontResolveSymlinks);
    if ( !file.isEmpty() ) {
        ui->lineEditDirectoryName->setText(file);
    }
}

/*!
 * \brief MainWindow::pushButtonPreviewClicked
 */
void MainWindow::pushButtonPreviewClicked() {
    QString fileNamePattern = ui->lineEditFileNamePattern->text();
    QString directoryName = ui->lineEditDirectoryName->text();
    QString newFileName = ui->lineEditNewFileName->text();
    QString startNumberString = ui->lineEditStartNumber->text();

    QString errorMessage = getErrorMessage(fileNamePattern, directoryName, newFileName, startNumberString);
    if ( !errorMessage.isEmpty() ) {
        QMessageBox::information(NULL, tr("Message"), errorMessage);
        return;
    }
    ui->pushButtonPreview->setEnabled(false);

    int startNumber = startNumberString.toInt();
    QStringList matchedFiles = getMatchedFiles(fileNamePattern, directoryName);
    getNewFileNames(matchedFiles, newFileName, startNumber);

    ui->tabWidget->setCurrentWidget(ui->tabPreview);
    ui->pushButtonPreview->setEnabled(true);
}

/*!
 * \brief MainWindow::getMatchedFiles - get matched file list filtered by the file name pattern
 * \param fileNamePattern - the pattern of the file name
 * \param directoryName - the directory where the files located
 * \return a list of string contains matched file names
 */
QStringList MainWindow::getMatchedFiles(const QString& fileNamePattern, const QString& directoryName) {
    QStringList files;
    QFileInfo directory(directoryName);
    if ( directory.isDir() && directory.exists() ) {
        QRegExp filenameExpression(fileNamePattern, Qt::CaseInsensitive, QRegExp::RegExp2);
        QDir workingDirectory(directoryName);
        QFileInfoList fileInfoList = workingDirectory.entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);

        for ( QFileInfo& file : fileInfoList ) {
            QString filename = file.fileName();
            if ( filenameExpression.indexIn(filename) != -1 ) {
                files.append(filename);
            }
        }
    }
    return files;
}

/*!
 * \brief MainWindow::getNewFileNames
 * \param matchedFiles - a list contains file names which matched the file name pattern
 * \param newFileNamePattern - the pattern of new file name
 * \param index - the ID in the new file name
 */
void MainWindow::getNewFileNames(const QStringList& matchedFiles, QString newFileNamePattern, int index) {
    int totalFiles = matchedFiles.length();
    int maxNumber = index + totalFiles;

    ui->treeWidget->clear();
    for ( const QString& originFileName : matchedFiles ) {
        QString fileExtension = QFileInfo(originFileName).suffix();
        QString formattedIndex = getFormattedNumber(index ++, maxNumber);
        QString newFileName = newFileNamePattern + "." + fileExtension;
        newFileName.replace(QString("<index>"), formattedIndex);

        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, originFileName);
        item->setText(1, newFileName);
    }
}

/*!
 * \brief MainWindow::getNewFileNames
 * \param matchedFiles - a list contains file names which matched the file name pattern
 * \param newFileNamePattern - the pattern of new file name
 * \param startNumber - the starting number of the new file name
 */
QString MainWindow::getFormattedNumber(int index, int maxNumber) {
    int numberOfLeadingZeros = std::ceil(std::log10(maxNumber));
    return QString("%1").arg(index, numberOfLeadingZeros, 10, QChar('0'));
}

/*!
 * \brief MainWindow::getErrorMessage
 * \param fileNamePattern - the pattern of original file name
 * \param directoryName - the directory where the files located
 * \param newFileName - the pattern of new file name
 * \param startNumber - the starting number of the file names
 * \return the error message
 */
QString MainWindow::getErrorMessage(const QString & fileNamePattern,
    const QString & directoryName, const QString & newFileName,
    const QString & startNumber) {
    bool isStartNumberNumberic = false;
    startNumber.toInt(&isStartNumberNumberic);

    if ( fileNamePattern.isEmpty() ) {
        return tr("Please fill the file name pattern.");
    }
    if ( directoryName.isEmpty() ) {
        return tr("Please choose the dicretory where the file located.");
    }
    if ( newFileName.isEmpty() ) {
        return tr("Please enter new file name.");
    } else if ( newFileName.indexOf("<index>") == -1 ) {
        return tr("The new file name MUST contains \"<index>\" field to identify every file.");
    }
    if ( startNumber.isEmpty() ) {
        return tr("Please fill the starting number of the new files.");
    } else if ( !isStartNumberNumberic ) {
        return tr("The starting number is not a number.");
    }
    return QString("");
}

/*!
 * \brief MainWindow::pushButtonProcessClicked
 */
void MainWindow::pushButtonProcessClicked() {
    ui->pushButtonProcess->setEnabled(false);

    QMap<QString, QString> finalNames;
    QString directoryName = ui->lineEditDirectoryName->text() + "/";

    for ( int i = 0; i < ui->treeWidget->topLevelItemCount(); ++ i ) {
        QTreeWidgetItem* item = ui->treeWidget->topLevelItem(i);
        QString originalName = directoryName + item->data(0, Qt::DisplayRole).toString();
        QString finalName = directoryName + item->data(1, Qt::DisplayRole).toString();
        QString tmpName = directoryName + QUuid::createUuid().toString();

        QFile file(originalName);
        if ( file.exists() ) {
            file.rename(tmpName);
            finalNames.insert(tmpName, finalName);
        }
    }
    for ( QMap<QString, QString>::const_iterator itr = finalNames.constBegin();
          itr != finalNames.constEnd(); ++ itr ) {
        QFile file(itr.key());
        file.rename(itr.value());
    }
    ui->treeWidget->clear();
    ui->pushButtonProcess->setEnabled(true);

    QMessageBox::information(NULL, tr("Message"), tr("Rename operation successfully completed."));
}

/*!
 * \brief MainWindow::actionExitTriggered
 */
void MainWindow::actionExitTriggered() {
    QApplication::quit();
}

/*!
 * \brief MainWindow::actionAboutUsTriggered
 */
void MainWindow::actionAboutUsTriggered() {
    QDesktopServices::openUrl(QString("https://github.com/hzxie/FileRenameUtility"));
}
