#include "mainwindow.h"
#include "sizedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QPixmap>
#include <QPainter>
#include <QMenu>
#include <QAction>
#include <QDesktopServices>
#include <QUrl>
#include <QCoreApplication>
#include <QDebug>
#include <QLocale>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(950, 750);
    setupUi();

    // Auto-detect system language
    QString sysLocale = QLocale::system().name().toLower();
    m_currentLocale = "en";
    if (sysLocale.startsWith("zh_tw") || sysLocale.startsWith("zh_hk") || sysLocale.startsWith("zh_mo")) {
        m_currentLocale = "zh_tw";
    } else if (sysLocale.startsWith("zh")) {
        m_currentLocale = "zh";
    } else if (sysLocale.startsWith("ja")) {
        m_currentLocale = "ja";
    } else if (sysLocale.startsWith("ru")) {
        m_currentLocale = "ru";
    }

    if (m_currentLocale != "en") {
        QString filename = QString("logobuilder_%1.qm").arg(m_currentLocale);
        QString path = QCoreApplication::applicationDirPath() + "/lang";
        if (!QFile::exists(path + "/" + filename)) {
            path = QCoreApplication::applicationDirPath();
        }
        if (m_translator.load(filename, path)) {
            qApp->installTranslator(&m_translator);
        } else {
            m_currentLocale = "en";
        }
    }

    retranslateUi();
}

MainWindow::~MainWindow()
{
}

QIcon MainWindow::drawVectorIcon(const QString& name)
{
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    if (name == "create") {
        painter.setBrush(QColor("#4CAF50")); // Green
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(2, 2, 28, 28);
        painter.setPen(QPen(Qt::white, 3, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(16, 9, 16, 23);
        painter.drawLine(9, 16, 23, 16);
    } else if (name == "open") {
        painter.setBrush(QColor("#FFC107")); // Yellow
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(2, 2, 28, 28);
        painter.setPen(QPen(Qt::white, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(16, 9, 16, 23);
        painter.drawLine(10, 15, 16, 9);
        painter.drawLine(22, 15, 16, 9);
    } else if (name == "boot_logo") {
        painter.setBrush(QColor("#F44336")); // Red
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(2, 2, 28, 28);
        painter.setBrush(Qt::white);
        painter.drawEllipse(11, 11, 10, 10);
    } else if (name == "globe") {
        painter.setBrush(QColor("#2196F3")); // Blue
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(2, 2, 28, 28);
        painter.setPen(QPen(Qt::white, 1.5));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(8, 8, 16, 16);
        painter.drawLine(16, 8, 16, 24);
        painter.drawLine(8, 16, 24, 16);
        painter.drawEllipse(12, 8, 8, 16);
    } else if (name == "info") {
        painter.setBrush(QColor("#00BCD4")); // Cyan
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(2, 2, 28, 28);
        painter.setPen(QPen(Qt::white, 3, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(16, 14, 16, 22);
        painter.setPen(QPen(Qt::white, 4, Qt::SolidLine, Qt::RoundCap));
        painter.drawPoint(16, 10);
    } else if (name == "folder") {
        painter.setBrush(QColor("#FFB300")); // Amber
        painter.setPen(Qt::NoPen);
        QPolygon poly;
        poly << QPoint(4, 6) << QPoint(14, 6) << QPoint(18, 10) << QPoint(28, 10)
             << QPoint(28, 26) << QPoint(4, 26);
        painter.drawPolygon(poly);
    } else if (name == "pack") {
        painter.setBrush(QColor("#3F51B5")); // Indigo/Blue
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(2, 2, 28, 28);
        painter.setPen(QPen(Qt::white, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(16, 9, 16, 23);
        painter.drawLine(10, 17, 16, 23);
        painter.drawLine(22, 17, 16, 23);
    }

    return QIcon(pixmap);
}

void MainWindow::setupUi()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    
    // Top bar (Toolbar style)
    QHBoxLayout *topLayout = new QHBoxLayout();
    
    m_btnCreate = new QPushButton(this);
    m_btnCreate->setIcon(drawVectorIcon("create"));
    m_btnCreate->setIconSize(QSize(24, 24));
    
    m_btnOpenProject = new QPushButton(this);
    m_btnOpenProject->setIcon(drawVectorIcon("open"));
    m_btnOpenProject->setIconSize(QSize(24, 24));
    
    m_btnBootLogo = new QPushButton(this);
    m_btnBootLogo->setIcon(drawVectorIcon("boot_logo"));
    m_btnBootLogo->setIconSize(QSize(24, 24));
    
    m_btnGlobe = new QPushButton(this);
    m_btnGlobe->setIcon(drawVectorIcon("globe"));
    m_btnGlobe->setIconSize(QSize(24, 24));
    
    QMenu *langMenu = new QMenu(this);
    QAction *actEn = langMenu->addAction("English");
    QAction *actRu = langMenu->addAction("Русский (Russian)");
    QAction *actZh = langMenu->addAction("中文 (简体) (Simplified Chinese)");
    QAction *actZhTw = langMenu->addAction("中文 (繁體) (Traditional Chinese)");
    QAction *actJa = langMenu->addAction("日本語 (Japanese)");
    m_btnGlobe->setMenu(langMenu);
    
    m_btnInfo = new QPushButton(this);
    m_btnInfo->setIcon(drawVectorIcon("info"));
    m_btnInfo->setIconSize(QSize(24, 24));
    
    m_btnLicense = new QPushButton(this);
    m_btnLicense->setIcon(drawVectorIcon("info"));
    m_btnLicense->setIconSize(QSize(24, 24));
    
    topLayout->addWidget(m_btnCreate);
    topLayout->addWidget(m_btnOpenProject);
    topLayout->addWidget(m_btnBootLogo);
    topLayout->addStretch();
    topLayout->addWidget(m_btnGlobe);
    topLayout->addWidget(m_btnInfo);
    topLayout->addWidget(m_btnLicense);
    
    mainLayout->addLayout(topLayout);
    
    // Scroll area for images
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setStyleSheet("background-color: #f7f7f9; border: 1px solid #e1e1e5; border-radius: 6px;");
    
    m_scrollWidget = new QWidget(m_scrollArea);
    m_gridLayout = new QGridLayout(m_scrollWidget);
    m_gridLayout->setContentsMargins(15, 15, 15, 15);
    m_gridLayout->setSpacing(15);
    m_scrollWidget->setLayout(m_gridLayout);
    m_scrollArea->setWidget(m_scrollWidget);
    
    mainLayout->addWidget(m_scrollArea);
    
    // Bottom Layout
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    
    m_btnOpenFolder = new QPushButton(this);
    m_btnOpenFolder->setIcon(drawVectorIcon("folder"));
    m_btnOpenFolder->setIconSize(QSize(24, 24));
    m_btnOpenFolder->setEnabled(false);
    
    m_btnPack = new QPushButton(this);
    m_btnPack->setObjectName("btnPack");
    m_btnPack->setIcon(drawVectorIcon("pack"));
    m_btnPack->setIconSize(QSize(24, 24));
    m_btnPack->setEnabled(false);
    m_btnPack->setMinimumWidth(150);
    
    bottomLayout->addWidget(m_btnOpenFolder);
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_btnPack);
    bottomLayout->addStretch();
    
    // Add tiny empty widget for layout balance
    QWidget *spacerWidget = new QWidget(this);
    spacerWidget->setMinimumWidth(80);
    bottomLayout->addWidget(spacerWidget);
    
    mainLayout->addLayout(bottomLayout);
    
    // Status
    m_progressBar = new QProgressBar(this);
    m_progressBar->setValue(0);
    m_progressBar->setFixedHeight(12);
    m_progressBar->setTextVisible(false);
    m_progressBar->setStyleSheet("QProgressBar { background-color: #e0e0e0; border-radius: 6px; } QProgressBar::chunk { background-color: #3F51B5; border-radius: 6px; }");
    mainLayout->addWidget(m_progressBar);
    
    connect(m_btnCreate, &QPushButton::clicked, this, &MainWindow::onCreateProject);
    connect(m_btnOpenProject, &QPushButton::clicked, this, &MainWindow::onOpenProject);
    connect(m_btnBootLogo, &QPushButton::clicked, this, &MainWindow::onCreateBootLogo);
    connect(m_btnOpenFolder, &QPushButton::clicked, this, &MainWindow::onOpenFolder);
    connect(m_btnPack, &QPushButton::clicked, this, &MainWindow::onPackProject);
    connect(m_btnInfo, &QPushButton::clicked, this, &MainWindow::onInfo);
    connect(m_btnLicense, &QPushButton::clicked, this, &MainWindow::onLicense);
    
    connect(actEn, &QAction::triggered, this, [this](){ switchLanguage("en"); });
    connect(actRu, &QAction::triggered, this, [this](){ switchLanguage("ru"); });
    connect(actZh, &QAction::triggered, this, [this](){ switchLanguage("zh"); });
    connect(actZhTw, &QAction::triggered, this, [this](){ switchLanguage("zh_tw"); });
    connect(actJa, &QAction::triggered, this, [this](){ switchLanguage("ja"); });
}

void MainWindow::retranslateUi()
{
    setWindowTitle(tr("LogoBuilder Qt - Linux Edition"));

    m_btnCreate->setText(tr("Create Project"));
    m_btnOpenProject->setText(tr("Open Project"));
    m_btnBootLogo->setText(tr("Create boot_logo"));
    m_btnGlobe->setText(tr("Language"));
    m_btnInfo->setText(tr("About"));
    m_btnLicense->setText(tr("License"));

    m_btnOpenFolder->setText(tr("Open Folder"));
    m_btnPack->setText(tr("Pack"));
    
    // Re-render images to update potential localized texts
    QVector<LogoImage> images = m_project.getImages();
    if (!images.isEmpty()) {
        clearImages();
        for (const auto& img : images) {
            addImagePreview(img.image, img.index, img.width, img.height);
        }
    }
}

void MainWindow::switchLanguage(const QString& locale)
{
    qApp->removeTranslator(&m_translator);

    if (locale == "en") {
        m_currentLocale = "en";
        retranslateUi();
        return;
    }

    QString filename = QString("logobuilder_%1.qm").arg(locale);
    QString path = QCoreApplication::applicationDirPath() + "/lang";
    if (!QFile::exists(path + "/" + filename)) {
        path = QCoreApplication::applicationDirPath();
    }

    if (m_translator.load(filename, path)) {
        qApp->installTranslator(&m_translator);
        m_currentLocale = locale;
        qDebug() << "Successfully loaded language:" << locale;
    } else {
        qWarning() << "Failed to load language:" << filename << "in" << path;
    }

    retranslateUi();
}

void MainWindow::clearImages()
{
    QLayoutItem *child;
    while ((child = m_gridLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
}

void MainWindow::addImagePreview(const QImage& img, int index, int w, int h)
{
    QVBoxLayout *vbox = new QVBoxLayout();
    vbox->setSpacing(6);
    
    QString infoText;
    if (w > 0 && h > 0) {
        infoText = QString("img%1\n%2 x %3").arg(index).arg(w).arg(h);
    } else {
        infoText = QString("img%1\n%2").arg(index).arg(tr("RAW / Unknown"));
    }
    
    QLabel *lblTitle = new QLabel(infoText);
    lblTitle->setAlignment(Qt::AlignCenter);
    lblTitle->setStyleSheet("font-weight: bold; color: #2c3e50; font-size: 11px;");
    
    ClickableLabel *lblImg = new ClickableLabel(index, this);
    if (!img.isNull()) {
        QPixmap pixmap = QPixmap::fromImage(img);
        pixmap = pixmap.scaled(150, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        lblImg->setPixmap(pixmap);
    } else {
        lblImg->setText(tr("RAW / Unknown\n(Double-click to adjust)"));
        lblImg->setStyleSheet("background-color: #ebedef; color: #7f8c8d; border: 2px dashed #bdc3c7; border-radius: 4px; padding: 10px;");
    }
    lblImg->setAlignment(Qt::AlignCenter);
    lblImg->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    lblImg->setCursor(Qt::PointingHandCursor);
    lblImg->setMinimumSize(150, 250);
    
    connect(lblImg, &ClickableLabel::doubleClicked, this, &MainWindow::onImageDoubleClicked);
    
    vbox->addWidget(lblImg);
    vbox->addWidget(lblTitle);
    
    QWidget *container = new QWidget();
    container->setLayout(vbox);
    
    int row = index / 4;
    int col = index % 4;
    m_gridLayout->addWidget(container, row, col);
}

void MainWindow::onCreateProject()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open logo.bin"), "", tr("BIN Files (*.bin);;All Files (*)"));
    if (fileName.isEmpty()) return;
    
    QFileInfo fi(fileName);
    m_projectDir = fi.absolutePath() + "/LogoProject_" + fi.baseName();
    
    clearImages();
    m_progressBar->setMaximum(100);
    m_progressBar->setValue(50);
    
    if (m_project.loadProject(fileName, m_projectDir)) {
        QVector<LogoImage> images = m_project.getImages();
        if (!images.isEmpty()) {
            const LogoImage& firstImg = images[0];
            int initW = firstImg.width > 0 ? firstImg.width : 1080;
            int initH = firstImg.height > 0 ? firstImg.height : 1920;
            int initDepth = firstImg.colorDepth > 0 ? firstImg.colorDepth : 32;
            
            SizeDialog dlg(firstImg.rawData, initW, initH, initDepth, this);
            dlg.setWindowTitle(tr("Adjust Global Resolution (Reference: img0)"));
            if (dlg.exec() == QDialog::Accepted) {
                int w = dlg.getSelectedWidth();
                int h = dlg.getSelectedHeight();
                int depth = dlg.getSelectedDepth();
                
                for (int i = 0; i < images.size(); ++i) {
                    if (images[i].rawData.size() == firstImg.rawData.size()) {
                        m_project.updateImageSize(i, w, h, depth, m_projectDir);
                    }
                }
            }
        }
        
        QVector<LogoImage> updatedImages = m_project.getImages();
        for (const auto& img : updatedImages) {
            addImagePreview(img.image, img.index, img.width, img.height);
        }
        
        m_progressBar->setValue(100);
        m_btnPack->setEnabled(true);
        m_btnOpenFolder->setEnabled(true);
        QMessageBox::information(this, tr("Success"), tr("Project unpacked successfully to:\n%1").arg(m_projectDir));
    } else {
        m_progressBar->setValue(0);
        QMessageBox::critical(this, tr("Error"), m_project.getError());
    }
}

void MainWindow::onOpenProject()
{
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("Open Project Folder"), "");
    if (dirPath.isEmpty()) return;
    
    m_projectDir = dirPath;
    
    clearImages();
    m_progressBar->setMaximum(100);
    m_progressBar->setValue(50);
    
    if (m_project.openProject(dirPath)) {
        QVector<LogoImage> images = m_project.getImages();
        for (const auto& img : images) {
            addImagePreview(img.image, img.index, img.width, img.height);
        }
        m_progressBar->setValue(100);
        m_btnPack->setEnabled(true);
        m_btnOpenFolder->setEnabled(true);
        QMessageBox::information(this, tr("Success"), tr("Project loaded successfully."));
    } else {
        m_progressBar->setValue(0);
        QMessageBox::critical(this, tr("Error"), m_project.getError());
    }
}

void MainWindow::onCreateBootLogo()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("Create boot_logo"));
    msgBox.setText(tr("Choose boot_logo conversion mode:"));
    QPushButton *btnToRaw = msgBox.addButton(tr("Image to boot_logo (Raw RGB565)"), QMessageBox::ActionRole);
    QPushButton *btnToImg = msgBox.addButton(tr("boot_logo (Raw RGB565) to Image"), QMessageBox::ActionRole);
    msgBox.addButton(QMessageBox::Cancel);
    
    msgBox.exec();
    
    if (msgBox.clickedButton() == btnToRaw) {
        QString imgFile = QFileDialog::getOpenFileName(this, tr("Open PNG/JPG Image"), "", tr("Images (*.png *.jpg *.jpeg)"));
        if (imgFile.isEmpty()) return;
        
        QImage img(imgFile);
        if (img.isNull()) {
            QMessageBox::critical(this, tr("Error"), tr("Failed to load source image."));
            return;
        }
        
        QString outFile = QFileDialog::getSaveFileName(this, tr("Save boot_logo raw file"), "boot_logo", tr("All Files (*)"));
        if (outFile.isEmpty()) return;
        
        if (LogoProject::imageToRawFile(img, 16, outFile)) {
            QMessageBox::information(this, tr("Success"), tr("boot_logo created successfully."));
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Failed to save boot_logo file."));
        }
    } else if (msgBox.clickedButton() == btnToImg) {
        QString rawFile = QFileDialog::getOpenFileName(this, tr("Open boot_logo raw file"), "", tr("All Files (*)"));
        if (rawFile.isEmpty()) return;
        
        QFile f(rawFile);
        if (!f.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, tr("Error"), tr("Failed to open raw file."));
            return;
        }
        QByteArray data = f.readAll();
        
        SizeDialog dlg(data, 1080, 1920, 16, this);
        dlg.setWindowTitle(tr("Set boot_logo resolution"));
        if (dlg.exec() == QDialog::Accepted) {
            int w = dlg.getSelectedWidth();
            int h = dlg.getSelectedHeight();
            int depth = dlg.getSelectedDepth();
            
            QImage img;
            if (depth == 32) {
                img = QImage((const uchar*)data.constData(), w, h, QImage::Format_RGBA8888);
            } else {
                img = QImage((const uchar*)data.constData(), w, h, QImage::Format_RGB16);
            }
            
            if (img.isNull()) {
                QMessageBox::critical(this, tr("Error"), tr("Invalid raw data for chosen resolution."));
                return;
            }
            
            QString outFile = QFileDialog::getSaveFileName(this, tr("Save image as PNG"), "boot_logo.png", tr("PNG Files (*.png)"));
            if (outFile.isEmpty()) return;
            
            if (img.save(outFile)) {
                QMessageBox::information(this, tr("Success"), tr("Image saved successfully."));
            } else {
                QMessageBox::critical(this, tr("Error"), tr("Failed to save output image."));
            }
        }
    }
}

void MainWindow::onOpenFolder()
{
    if (m_projectDir.isEmpty()) return;
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_projectDir));
}

void MainWindow::onPackProject()
{
    if (m_projectDir.isEmpty()) return;
    
    QString outFile = QFileDialog::getSaveFileName(this, tr("Save Packed logo.bin"), m_projectDir + "/logo_new.bin", tr("BIN Files (*.bin);;All Files (*)"));
    if (outFile.isEmpty()) return;
    
    m_progressBar->setValue(50);
    if (m_project.packProject(m_projectDir, outFile)) {
        m_progressBar->setValue(100);
        QMessageBox::information(this, tr("Success"), tr("Project packed successfully to:\n%1").arg(outFile));
    } else {
        m_progressBar->setValue(0);
        QMessageBox::critical(this, tr("Error"), m_project.getError());
    }
}

void MainWindow::onLicense()
{
    QMessageBox::information(this, tr("License"),
        tr("LogoBuilder Qt - Linux Edition\n\n"
           "Licensed under the GNU General Public License Version 3.\n"
           "Ported to C++/Qt for Linux systems."));
}

void MainWindow::onInfo()
{
    QMessageBox::about(this, tr("About LogoBuilder"),
        tr("<h3>LogoBuilder Qt</h3>"
           "<p>Version 1.3</p>"
           "<p>This is a native Qt port of LogoBuilder for editing and packing "
           "MediaTek (MTK) phone logo bin files.</p>"
           "<p>Original Windows Tool by: <b>kadan</b></p>"
           "<p>Ported to Linux C++/Qt by dismal002.</p>"));
}

void MainWindow::onImageDoubleClicked(int index)
{
    QVector<LogoImage> images = m_project.getImages();
    if (index < 0 || index >= images.size()) return;
    
    const LogoImage& logoImg = images[index];
    
    SizeDialog dlg(logoImg.rawData, logoImg.width, logoImg.height, logoImg.colorDepth, this);
    if (dlg.exec() == QDialog::Accepted) {
        int w = dlg.getSelectedWidth();
        int h = dlg.getSelectedHeight();
        int depth = dlg.getSelectedDepth();
        
        for (int i = 0; i < images.size(); ++i) {
            if (images[i].rawData.size() == logoImg.rawData.size()) {
                m_project.updateImageSize(i, w, h, depth, m_projectDir);
            }
        }
        
        clearImages();
        QVector<LogoImage> updatedImages = m_project.getImages();
        for (const auto& img : updatedImages) {
            addImagePreview(img.image, img.index, img.width, img.height);
        }
    }
}

