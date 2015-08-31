#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
    void initSignalsAndSlots();
    void pushButtonBrowseClicked();
    void pushButtonPreviewClicked();
    QString getErrorMessage(const QString&, const QString&, const QString&, const QString&);
    QStringList getMatchedFiles(const QString& fileNamePattern, const QString& directoryName);
    void getNewFileNames(const QStringList& matchedFiles, QString newFileNamePattern, int index);
    QString getFormattedNumber(int index, int maxNumber);
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
