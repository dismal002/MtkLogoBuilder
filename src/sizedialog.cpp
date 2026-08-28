#include "sizedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QPixmap>
#include <QDebug>

SizeDialog::SizeDialog(const QByteArray& rawData, int initialWidth, int initialHeight, int initialDepth, QWidget *parent)
    : QDialog(parent), m_rawData(rawData), m_colorDepth(initialDepth), m_selectedWidth(initialWidth), m_selectedHeight(initialHeight)
{
    setWindowTitle(tr("Adjust Image Size"));
    resize(700, 600);
    
    if (m_colorDepth != 16 && m_colorDepth != 32) {
        m_colorDepth = 32; // default
    }
    
    setupUi();
    calculateSizes();
    
    // Find initial size in list
    int initialIdx = 0;
    for (int i = 0; i < m_possibleSizes.size(); ++i) {
        if (m_possibleSizes[i].width == m_selectedWidth && m_possibleSizes[i].height == m_selectedHeight) {
            initialIdx = i;
            break;
        }
    }
    
    m_slider->setValue(initialIdx);
    onSliderChanged(initialIdx);
}

void SizeDialog::setupUi()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    
    // Left side: Controls
    QVBoxLayout *leftLayout = new QVBoxLayout();
    
    m_lblSize = new QLabel(tr("Size: 0 x 0 (32 bpp)"), this);
    leftLayout->addWidget(m_lblSize);
    
    m_slider = new QSlider(Qt::Horizontal, this);
    leftLayout->addWidget(m_slider);
    
    QHBoxLayout *depthLayout = new QHBoxLayout();
    depthLayout->addWidget(new QLabel(tr("Color Depth:"), this));
    m_cbColorDepth = new QComboBox(this);
    m_cbColorDepth->addItem(tr("32-bit (RGBA8888)"), 32);
    m_cbColorDepth->addItem(tr("16-bit (RGB565)"), 16);
    m_cbColorDepth->setCurrentIndex(m_colorDepth == 32 ? 0 : 1);
    depthLayout->addWidget(m_cbColorDepth);
    leftLayout->addLayout(depthLayout);
    
    leftLayout->addStretch();
    
    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_btnOk = new QPushButton(tr("OK"), this);
    m_btnCancel = new QPushButton(tr("Cancel"), this);
    btnLayout->addWidget(m_btnOk);
    btnLayout->addWidget(m_btnCancel);
    leftLayout->addLayout(btnLayout);
    
    mainLayout->addLayout(leftLayout, 1);
    
    // Right side: Preview Scroll Area
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    m_lblPreview = new QLabel(scrollArea);
    m_lblPreview->setAlignment(Qt::AlignCenter);
    m_lblPreview->setMinimumSize(400, 500);
    scrollArea->setWidget(m_lblPreview);
    
    mainLayout->addWidget(scrollArea, 2);
    
    connect(m_slider, &QSlider::valueChanged, this, &SizeDialog::onSliderChanged);
    connect(m_cbColorDepth, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SizeDialog::onColorDepthChanged);
    connect(m_btnOk, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void SizeDialog::calculateSizes()
{
    m_possibleSizes.clear();
    
    int bpp = m_colorDepth / 8;
    int totalPixels = m_rawData.size() / bpp;
    
    if (totalPixels <= 0) return;
    
    for (int w = 1; w <= totalPixels; ++w) {
        if (totalPixels % w == 0) {
            int h = totalPixels / w;
            if (w < 6000 && h < 6000) {
                PossibleSize sz;
                sz.width = w;
                sz.height = h;
                m_possibleSizes.push_back(sz);
            }
        }
    }
    
    m_slider->setRange(0, m_possibleSizes.size() - 1);
}

void SizeDialog::onSliderChanged(int value)
{
    if (value < 0 || value >= m_possibleSizes.size()) return;
    
    m_selectedWidth = m_possibleSizes[value].width;
    m_selectedHeight = m_possibleSizes[value].height;
    
    m_lblSize->setText(tr("Size: %1 x %2 (%3 bpp)")
                       .arg(m_selectedWidth)
                       .arg(m_selectedHeight)
                       .arg(m_colorDepth));
                       
    updatePreview();
}

void SizeDialog::onColorDepthChanged(int index)
{
    m_colorDepth = m_cbColorDepth->itemData(index).toInt();
    calculateSizes();
    
    // reset to middle or find closest
    int mid = m_possibleSizes.size() / 2;
    m_slider->setValue(mid);
    onSliderChanged(mid);
}

void SizeDialog::updatePreview()
{
    if (m_selectedWidth <= 0 || m_selectedHeight <= 0) return;
    
    QImage img;
    if (m_colorDepth == 32) {
        img = QImage((const uchar*)m_rawData.constData(), m_selectedWidth, m_selectedHeight, QImage::Format_RGBA8888);
    } else {
        img = QImage((const uchar*)m_rawData.constData(), m_selectedWidth, m_selectedHeight, QImage::Format_RGB16);
    }
    
    if (!img.isNull()) {
        QPixmap pixmap = QPixmap::fromImage(img);
        
        int targetW = m_lblPreview->width() - 10;
        int targetH = m_lblPreview->height() - 10;
        if (targetW <= 10 || targetH <= 10) {
            targetW = 400;
            targetH = 500;
        }
        
        pixmap = pixmap.scaled(targetW, targetH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_lblPreview->setPixmap(pixmap);
    }
}

int SizeDialog::getSelectedWidth() const { return m_selectedWidth; }
int SizeDialog::getSelectedHeight() const { return m_selectedHeight; }
int SizeDialog::getSelectedDepth() const { return m_colorDepth; }
