#ifndef LOGOPROJECT_H
#define LOGOPROJECT_H

#include <QString>
#include <QVector>
#include <QByteArray>
#include <QImage>
#include <QMap>

struct LogoImage {
    int index;
    int offset;
    int compressedSize;
    QByteArray rawData;       // Uncompressed raw data
    QImage image;             // Parsed image (if possible)
    int width;
    int height;
    int colorDepth;           // 16 or 32
};

class LogoProject {
public:
    struct SizeInfo {
        int width;
        int height;
        int depth;
    };

    LogoProject();
    
    bool loadProject(const QString& binFile, const QString& outDir);
    bool openProject(const QString& projectDir);
    bool packProject(const QString& inDir, const QString& binFile);
    bool updateImageSize(int index, int w, int h, int depth, const QString& projectDir);
    static bool imageToRawFile(const QImage& img, int depth, const QString& outFile);
    
    QVector<LogoImage> getImages() const;
    QString getError() const;

private:
    QVector<LogoImage> m_images;
    QString m_error;
    QByteArray m_header; // 512 bytes
    QMap<int, SizeInfo> m_globalSizes;
    
    void loadGlobalSizes();
    void saveGlobalSizes();
    
    bool guessResolution(int dataSize, int& w, int& h, int& depth);
    QImage rawToImage(const QByteArray& raw, int w, int h, int depth);
    static QByteArray imageToRaw(const QImage& img, int depth);
    
    // Zlib helpers
    QByteArray decompress(const QByteArray& compressed);
    QByteArray compress(const QByteArray& uncompressed);
};

#endif // LOGOPROJECT_H
