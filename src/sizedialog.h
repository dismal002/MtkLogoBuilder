#ifndef SIZEDIALOG_H
#define SIZEDIALOG_H

#include <QDialog>
#include <QSlider>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QByteArray>
#include <QImage>
#include <QVector>

struct PossibleSize {
    int width;
    int height;
};

class SizeDialog : public QDialog
{
    Q_OBJECT

public:
    SizeDialog(const QByteArray& rawData, int initialWidth, int initialHeight, int initialDepth, QWidget *parent = nullptr);
    
    int getSelectedWidth() const;
    int getSelectedHeight() const;
    int getSelectedDepth() const;

private slots:
    void onSliderChanged(int value);
    void onColorDepthChanged(int index);

private:
    void setupUi();
    void calculateSizes();
    void updatePreview();

    QByteArray m_rawData;
    int m_colorDepth; // 16 or 32
    
    QVector<PossibleSize> m_possibleSizes;
    int m_selectedWidth;
    int m_selectedHeight;

    QSlider *m_slider;
    QLabel *m_lblSize;
    QComboBox *m_cbColorDepth;
    QLabel *m_lblPreview;
    
    QPushButton *m_btnOk;
    QPushButton *m_btnCancel;
};

#endif // SIZEDIALOG_H
