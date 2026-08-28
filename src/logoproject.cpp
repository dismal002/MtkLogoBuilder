#include "logoproject.h"
#include <QFile>
#include <QDataStream>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QCoreApplication>
#include <zlib.h>
#include <QDebug>
#include <algorithm>

LogoProject::LogoProject()
{
    loadGlobalSizes();
}

void LogoProject::loadGlobalSizes()
{
    m_globalSizes.clear();
    QString path = QCoreApplication::applicationDirPath() + "/sizes.txt";
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split(" ", Qt::SkipEmptyParts);
            if (parts.size() >= 4) {
                bool ok1, ok2, ok3, ok4;
                int size = parts[0].toInt(&ok1);
                int w = parts[1].toInt(&ok2);
                int h = parts[2].toInt(&ok3);
                int depth = parts[3].toInt(&ok4);
                if (ok1 && ok2 && ok3 && ok4) {
                    m_globalSizes[size] = SizeInfo{w, h, depth};
                }
            }
        }
    }
}

void LogoProject::saveGlobalSizes()
{
    QString path = QCoreApplication::applicationDirPath() + "/sizes.txt";
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (auto it = m_globalSizes.begin(); it != m_globalSizes.end(); ++it) {
            out << it.key() << " " << it.value().width << " " << it.value().height << " " << it.value().depth << "\n";
        }
    }
}

QVector<LogoImage> LogoProject::getImages() const
{
    return m_images;
}

QString LogoProject::getError() const
{
    return m_error;
}

QByteArray LogoProject::decompress(const QByteArray& compressed)
{
    // Try to guess a large enough buffer, e.g. 16MB
    uLongf uncompressedSize = 16 * 1024 * 1024;
    QByteArray uncompressed(uncompressedSize, 0);
    
    int res = ::uncompress((Bytef*)uncompressed.data(), &uncompressedSize, (const Bytef*)compressed.constData(), compressed.size());
    if (res == Z_OK) {
        uncompressed.resize(uncompressedSize);
        return uncompressed;
    }
    
    // If buffer was not large enough or other error
    if (res == Z_BUF_ERROR) {
        uncompressedSize = 64 * 1024 * 1024;
        uncompressed.resize(uncompressedSize);
        res = ::uncompress((Bytef*)uncompressed.data(), &uncompressedSize, (const Bytef*)compressed.constData(), compressed.size());
        if (res == Z_OK) {
            uncompressed.resize(uncompressedSize);
            return uncompressed;
        }
    }
    return QByteArray();
}

QByteArray LogoProject::compress(const QByteArray& uncompressed)
{
    uLongf compressedSize = ::compressBound(uncompressed.size());
    QByteArray compressed(compressedSize, 0);
    
    int res = ::compress((Bytef*)compressed.data(), &compressedSize, (const Bytef*)uncompressed.constData(), uncompressed.size());
    if (res == Z_OK) {
        compressed.resize(compressedSize);
        return compressed;
    }
    return QByteArray();
}

bool LogoProject::guessResolution(int dataSize, int& w, int& h, int& depth)
{
    // 1. Check loaded global sizes first
    if (m_globalSizes.contains(dataSize)) {
        w = m_globalSizes[dataSize].width;
        h = m_globalSizes[dataSize].height;
        depth = m_globalSizes[dataSize].depth;
        return true;
    }

    // 2. Battery percentage frames heuristic
    // Standard size for MTK logo battery percentage frames is 45x64 at 32 bpp (11520 bytes)
    if (dataSize == 45 * 64 * 4) {
        w = 45;
        h = 64;
        depth = 32;
        return true;
    }

    // 3. Battery water/wave frames heuristic
    // Size for MTK logo battery water frames is 163x29 at 32 bpp (18908 bytes)
    if (dataSize == 163 * 29 * 4) {
        w = 163;
        h = 29;
        depth = 32;
        return true;
    }

    // 4. Other MTK battery charging frame heuristics (digits, end numbers, and warning signs)
    if (dataSize == 57 * 64 * 4) {
        w = 57;
        h = 64;
        depth = 32;
        return true;
    }
    if (dataSize == 72 * 128 * 4) {
        w = 72;
        h = 128;
        depth = 32;
        return true;
    }
    if (dataSize == 84 * 121 * 4) {
        w = 84;
        h = 121;
        depth = 32;
        return true;
    }
    if (dataSize == 304 * 52 * 4) {
        w = 304;
        h = 52;
        depth = 32;
        return true;
    }

    // Heuristics based on data size
    if (dataSize == 1080 * 1920 * 4) { w = 1080; h = 1920; depth = 32; return true; }
    if (dataSize == 1080 * 1920 * 2) { w = 1080; h = 1920; depth = 16; return true; }
    if (dataSize == 1080 * 2400 * 4) { w = 1080; h = 2400; depth = 32; return true; }
    if (dataSize == 1080 * 2400 * 2) { w = 1080; h = 2400; depth = 16; return true; }
    if (dataSize == 720 * 1280 * 4) { w = 720; h = 1280; depth = 32; return true; }
    if (dataSize == 720 * 1280 * 2) { w = 720; h = 1280; depth = 16; return true; }
    
    // Fallback for smaller images, assume width = dataSize / something? 
    // Wait, some small images might be battery charging animations. 
    // Let's just default to a square if it's small, or just leave it empty.
    if (dataSize > 0 && dataSize % 4 == 0) {
        // try to find a nice rectangle
        int pixels = dataSize / 4;
        w = 0; h = 0;
        for (int i = 100; i < 2000; i++) {
            if (pixels % i == 0) {
                w = i;
                h = pixels / i;
                if (h >= w && h <= w * 3) {
                    depth = 32;
                    return true;
                }
            }
        }
    }
    
    return false;
}

QImage LogoProject::rawToImage(const QByteArray& raw, int w, int h, int depth)
{
    if (depth == 32) {
        // Assume RGBA or BGRA
        QImage img((const uchar*)raw.constData(), w, h, QImage::Format_RGBA8888);
        return img.copy(); // deep copy
    } else if (depth == 16) {
        QImage img((const uchar*)raw.constData(), w, h, QImage::Format_RGB16);
        return img.copy();
    }
    return QImage();
}

QByteArray LogoProject::imageToRaw(const QImage& img, int depth)
{
    if (depth == 32) {
        QImage converted = img.convertToFormat(QImage::Format_RGBA8888);
        return QByteArray((const char*)converted.constBits(), converted.sizeInBytes());
    } else if (depth == 16) {
        QImage converted = img.convertToFormat(QImage::Format_RGB16);
        return QByteArray((const char*)converted.constBits(), converted.sizeInBytes());
    }
    return QByteArray();
}

bool LogoProject::loadProject(const QString& binFile, const QString& outDir)
{
    m_images.clear();
    m_error.clear();
    
    QFile f(binFile);
    if (!f.open(QIODevice::ReadOnly)) {
        m_error = "Could not open logo.bin file.";
        return false;
    }
    
    QDataStream in(&f);
    in.setByteOrder(QDataStream::LittleEndian);
    
    m_header = f.read(512);
    if (m_header.size() != 512) {
        m_error = "Invalid logo.bin header.";
        return false;
    }
    
    QDir dir(outDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QFile headerFile(dir.absoluteFilePath("header.bin"));
    if (headerFile.open(QIODevice::WriteOnly)) {
        headerFile.write(m_header);
    }
    
    quint32 numImages;
    quint32 dataLen;
    in >> numImages >> dataLen;
    
    QVector<quint32> offsets;
    for (quint32 i = 0; i < numImages; ++i) {
        quint32 offset;
        in >> offset;
        offsets.push_back(offset);
    }
    
    QFile sizesFile(dir.absoluteFilePath("sizes.txt"));
    sizesFile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream sizesStream(&sizesFile);
    
    for (quint32 i = 0; i < numImages; ++i) {
        quint32 start = 512 + offsets[i];
        quint32 end = (i < numImages - 1) ? 512 + offsets[i+1] : 512 + dataLen;
        
        f.seek(start);
        QByteArray compressed = f.read(end - start);
        
        LogoImage logoImg;
        logoImg.index = i;
        logoImg.offset = offsets[i];
        logoImg.compressedSize = end - start;
        logoImg.rawData = decompress(compressed);
        
        if (!logoImg.rawData.isEmpty()) {
            int w = 0, h = 0, depth = 0;
            if (guessResolution(logoImg.rawData.size(), w, h, depth)) {
                logoImg.width = w;
                logoImg.height = h;
                logoImg.colorDepth = depth;
                logoImg.image = rawToImage(logoImg.rawData, w, h, depth);
                
                QString imgPath = dir.absoluteFilePath(QString("img%1.png").arg(i));
                if (!logoImg.image.isNull()) {
                    logoImg.image.save(imgPath);
                    sizesStream << i << " " << w << " " << h << " " << depth << "\n";
                }
            } else {
                // Could not guess resolution, save raw
                QString rawPath = dir.absoluteFilePath(QString("img%1.raw").arg(i));
                QFile rf(rawPath);
                if (rf.open(QIODevice::WriteOnly)) {
                    rf.write(logoImg.rawData);
                }
            }
        }
        m_images.push_back(logoImg);
    }
    
    return true;
}

bool LogoProject::packProject(const QString& inDir, const QString& binFile)
{
    m_error.clear();
    
    QDir dir(inDir);
    QFile sizesFile(dir.absoluteFilePath("sizes.txt"));
    QMap<int, QString> sizesInfo;
    
    if (sizesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&sizesFile);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split(" ", Qt::SkipEmptyParts);
            if (parts.size() >= 4) {
                sizesInfo[parts[0].toInt()] = line;
            }
        }
    }
    
    if (m_header.isEmpty()) {
        QFile headerFile(dir.absoluteFilePath("header.bin"));
        if (headerFile.open(QIODevice::ReadOnly)) {
            m_header = headerFile.readAll();
        } else {
            // Create a dummy header if we absolutely must, 
            // but usually we need the original for MTK devices.
            m_header = QByteArray(512, 0);
        }
    }
    
    QFile out(binFile);
    if (!out.open(QIODevice::WriteOnly)) {
        m_error = "Could not open output logo.bin for writing.";
        return false;
    }
    
    out.write(m_header);
    
    QDataStream outStream(&out);
    outStream.setByteOrder(QDataStream::LittleEndian);
    
    int maxIndex = -1;
    QFileInfoList entries = dir.entryInfoList(QStringList() << "img*.png" << "img*.raw", QDir::Files);
    for (const QFileInfo& fi : entries) {
        QString name = fi.baseName();
        name.remove("img");
        bool ok;
        int idx = name.toInt(&ok);
        if (ok && idx > maxIndex) {
            maxIndex = idx;
        }
    }
    
    quint32 numImages = maxIndex + 1;
    if (numImages == 0) {
        m_error = "No images found in project directory.";
        return false;
    }
    
    outStream << numImages;
    // placeholder for dataLen
    outStream << (quint32)0;
    
    QVector<QByteArray> compressedBlocks;
    quint32 currentOffset = 8 + numImages * 4; // size of numImages + dataLen + offsets array
    
    QVector<quint32> newOffsets;
    
    for (int i = 0; i < (int)numImages; ++i) {
        newOffsets.push_back(currentOffset);
        
        QByteArray uncompressed;
        
        QString imgPath = dir.absoluteFilePath(QString("img%1.png").arg(i));
        QString rawPath = dir.absoluteFilePath(QString("img%1.raw").arg(i));
        
        if (QFile::exists(imgPath)) {
            QImage img(imgPath);
            int depth = 32;
            if (sizesInfo.contains(i)) {
                depth = sizesInfo[i].split(" ").at(3).toInt();
            }
            uncompressed = imageToRaw(img, depth);
        } else if (QFile::exists(rawPath)) {
            QFile rf(rawPath);
            if (rf.open(QIODevice::ReadOnly)) {
                uncompressed = rf.readAll();
            }
        } else {
            // Fallback to original raw data if not found on disk
            if (i < m_images.size()) {
                uncompressed = m_images[i].rawData;
            }
        }
        
        QByteArray compressed = compress(uncompressed);
        compressedBlocks.push_back(compressed);
        currentOffset += compressed.size();
    }
    
    // Now write dataLen which is currentOffset
    out.seek(516);
    outStream << (quint32)currentOffset;
    
    // Write offsets
    out.seek(520);
    for (int i = 0; i < (int)numImages; ++i) {
        outStream << newOffsets[i];
    }
    
    // Write blocks
    for (int i = 0; i < (int)numImages; ++i) {
        out.write(compressedBlocks[i]);
    }
    
    return true;
}

bool LogoProject::updateImageSize(int index, int w, int h, int depth, const QString& projectDir)
{
    if (index < 0 || index >= m_images.size()) return false;
    
    m_images[index].width = w;
    m_images[index].height = h;
    m_images[index].colorDepth = depth;
    m_images[index].image = rawToImage(m_images[index].rawData, w, h, depth);
    
    QDir dir(projectDir);
    QString rawPath = dir.absoluteFilePath(QString("img%1.raw").arg(index));
    if (QFile::exists(rawPath)) {
        QFile::remove(rawPath);
    }
    
    QString imgPath = dir.absoluteFilePath(QString("img%1.png").arg(index));
    if (!m_images[index].image.isNull()) {
        m_images[index].image.save(imgPath);
    }
    
    QFile sizesFile(dir.absoluteFilePath("sizes.txt"));
    QMap<int, QString> sizesInfo;
    if (sizesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&sizesFile);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split(" ", Qt::SkipEmptyParts);
            if (parts.size() >= 4) {
                sizesInfo[parts[0].toInt()] = line;
            }
        }
        sizesFile.close();
    }
    
    sizesInfo[index] = QString("%1 %2 %3 %4").arg(index).arg(w).arg(h).arg(depth);
    
    if (sizesFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&sizesFile);
        for (auto it = sizesInfo.begin(); it != sizesInfo.end(); ++it) {
            out << it.value() << "\n";
        }
    }
    
    // Also save to global sizes database
    int dataSize = m_images[index].rawData.size();
    if (dataSize > 0) {
        m_globalSizes[dataSize] = SizeInfo{w, h, depth};
        saveGlobalSizes();
    }
    
    return true;
}

bool LogoProject::openProject(const QString& projectDir)
{
    m_images.clear();
    m_error.clear();
    
    QDir dir(projectDir);
    if (!dir.exists()) {
        m_error = "Project directory does not exist.";
        return false;
    }
    
    // Read sizes.txt
    QFile sizesFile(dir.absoluteFilePath("sizes.txt"));
    if (!sizesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_error = "Could not open sizes.txt in project directory.";
        return false;
    }
    
    QFile headerFile(dir.absoluteFilePath("header.bin"));
    if (headerFile.open(QIODevice::ReadOnly)) {
        m_header = headerFile.readAll();
    }
    
    QTextStream in(&sizesFile);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(" ", Qt::SkipEmptyParts);
        if (parts.size() >= 4) {
            LogoImage img;
            img.index = parts[0].toInt();
            img.width = parts[1].toInt();
            img.height = parts[2].toInt();
            img.colorDepth = parts[3].toInt();
            img.offset = 0;
            img.compressedSize = 0;
            
            QString imgPath = dir.absoluteFilePath(QString("img%1.png").arg(img.index));
            QString rawPath = dir.absoluteFilePath(QString("img%1.raw").arg(img.index));
            
            if (QFile::exists(imgPath)) {
                img.image = QImage(imgPath);
                img.rawData = imageToRaw(img.image, img.colorDepth);
            } else if (QFile::exists(rawPath)) {
                QFile rf(rawPath);
                if (rf.open(QIODevice::ReadOnly)) {
                    img.rawData = rf.readAll();
                }
                img.image = rawToImage(img.rawData, img.width, img.height, img.colorDepth);
            }
            
            m_images.push_back(img);
        }
    }
    
    std::sort(m_images.begin(), m_images.end(), [](const LogoImage& a, const LogoImage& b) {
        return a.index < b.index;
    });
    
    return true;
}

bool LogoProject::imageToRawFile(const QImage& img, int depth, const QString& outFile)
{
    QByteArray raw = imageToRaw(img, depth);
    if (raw.isEmpty()) return false;
    
    QFile f(outFile);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(raw);
        return true;
    }
    return false;
}
