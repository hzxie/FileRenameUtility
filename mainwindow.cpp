#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <cmath>

#include <QFileDialog>
#include <QMessageBox>
#include <QStringList>
#include <QTreeWidgetItem>

#include <QtDebug>

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
}

/*!
 * \brief MainWindow::pushButtonBrowseClicked
 */
void MainWindow::pushButtonBrowseClicked() {
    QString file = QFileDialog::getExistingDirectory(
                    this,
                    tr("Choose Directory"),
                    "/",
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

    int startNumber = startNumberString.toInt();
    QStringList matchedFiles = getMatchedFiles(fileNamePattern, directoryName);
    getNewFileNames(matchedFiles, newFileName, startNumber);

    ui->tabWidget->setCurrentWidget(ui->tabPreview);
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
 * \param matchedFiles
 * \param newFileNamePattern
 * \param startNumber
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
