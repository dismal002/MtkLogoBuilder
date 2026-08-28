#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScrollArea>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QTranslator>
#include "logoproject.h"

class ClickableLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ClickableLabel(int index, QWidget *parent = nullptr) : QLabel(parent), m_index(index) {}
signals:
    void doubleClicked(int index);
protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override {
        Q_UNUSED(event);
        emit doubleClicked(m_index);
    }
private:
    int m_index;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onCreateProject();
    void onOpenProject();
    void onCreateBootLogo();
    void onOpenFolder();
    void onPackProject();
    void onLicense();
    void onInfo();
    void switchLanguage(const QString& locale);
    void onImageDoubleClicked(int index);

private:
    void setupUi();
    void retranslateUi();
    void clearImages();
    void addImagePreview(const QImage& img, int index, int w, int h);
    QIcon drawVectorIcon(const QString& name);

    LogoProject m_project;
    QString m_projectDir;
    QTranslator m_translator;
    QString m_currentLocale;
    
    QScrollArea *m_scrollArea;
    QWidget *m_scrollWidget;
    QGridLayout *m_gridLayout;
    
    QPushButton *m_btnCreate;
    QPushButton *m_btnOpenProject;
    QPushButton *m_btnBootLogo;
    QPushButton *m_btnGlobe;
    QPushButton *m_btnInfo;
    QPushButton *m_btnLicense;
    
    QPushButton *m_btnOpenFolder;
    QPushButton *m_btnPack;
    QProgressBar *m_progressBar;
};

#endif // MAINWINDOW_H
