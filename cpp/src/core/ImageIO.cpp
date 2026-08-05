#include "core/ImageIO.h"

#include <QFile>
#include <QFileInfo>
#include <QImageIOHandler>
#include <QImageReader>
#include <QRgba64>
#include <QtGui/qrgbafloat.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

namespace {
uchar scaleChannel(quint16 value, quint16 minimum, quint16 maximum) {
    if (maximum <= minimum) {
        return 0;
    }
    const quint32 numerator = static_cast<quint32>(value - minimum) * 255u;
    return static_cast<uchar>(numerator / static_cast<quint32>(maximum - minimum));
}

uchar reduceAlpha(quint16 value) {
    return static_cast<uchar>((static_cast<quint32>(value) + 128u) / 257u);
}

uchar scaleFloatChannel(float value, float minimum, float maximum) {
    if (!std::isfinite(value) || !std::isfinite(minimum) || !std::isfinite(maximum) ||
        maximum <= minimum) {
        return 0;
    }
    const double normalized = std::clamp(
        (static_cast<double>(value) - minimum) /
            (static_cast<double>(maximum) - minimum),
        0.0,
        1.0);
    return static_cast<uchar>(normalized * 255.0);
}

uchar reduceFloatAlpha(float value) {
    if (!std::isfinite(value)) {
        return 0;
    }
    return static_cast<uchar>(std::clamp(static_cast<double>(value), 0.0, 1.0) * 255.0);
}

QImage normalizeGrayscale16(const QImage &image) {
    quint16 minimum = std::numeric_limits<quint16>::max();
    quint16 maximum = 0;
    for (int y = 0; y < image.height(); ++y) {
        const auto *line = reinterpret_cast<const quint16 *>(image.constScanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            minimum = std::min(minimum, line[x]);
            maximum = std::max(maximum, line[x]);
        }
    }

    QImage normalized(image.size(), QImage::Format_Grayscale8);
    for (int y = 0; y < image.height(); ++y) {
        const auto *source = reinterpret_cast<const quint16 *>(image.constScanLine(y));
        auto *target = normalized.scanLine(y);
        for (int x = 0; x < image.width(); ++x) {
            target[x] = scaleChannel(source[x], minimum, maximum);
        }
    }
    return normalized;
}

QImage normalizeRgba64(const QImage &image) {
    const QImage source = image.format() == QImage::Format_RGBA64_Premultiplied
                              ? image.convertToFormat(QImage::Format_RGBA64)
                              : image;
    quint16 minimum[3] = {std::numeric_limits<quint16>::max(),
                          std::numeric_limits<quint16>::max(),
                          std::numeric_limits<quint16>::max()};
    quint16 maximum[3] = {0, 0, 0};
    for (int y = 0; y < source.height(); ++y) {
        const auto *line = reinterpret_cast<const QRgba64 *>(source.constScanLine(y));
        for (int x = 0; x < source.width(); ++x) {
            const quint16 channels[3] = {line[x].red(), line[x].green(), line[x].blue()};
            for (int channel = 0; channel < 3; ++channel) {
                minimum[channel] = std::min(minimum[channel], channels[channel]);
                maximum[channel] = std::max(maximum[channel], channels[channel]);
            }
        }
    }

    const bool hasAlpha = image.format() == QImage::Format_RGBA64 ||
                          image.format() == QImage::Format_RGBA64_Premultiplied;
    QImage normalized(source.size(), hasAlpha ? QImage::Format_RGBA8888 : QImage::Format_RGB888);
    for (int y = 0; y < source.height(); ++y) {
        const auto *pixels = reinterpret_cast<const QRgba64 *>(source.constScanLine(y));
        auto *target = normalized.scanLine(y);
        const int stride = hasAlpha ? 4 : 3;
        for (int x = 0; x < source.width(); ++x) {
            const QRgba64 pixel = pixels[x];
            target[x * stride] = scaleChannel(pixel.red(), minimum[0], maximum[0]);
            target[x * stride + 1] = scaleChannel(pixel.green(), minimum[1], maximum[1]);
            target[x * stride + 2] = scaleChannel(pixel.blue(), minimum[2], maximum[2]);
            if (hasAlpha) {
                target[x * stride + 3] = reduceAlpha(pixel.alpha());
            }
        }
    }
    return normalized;
}

template <typename Pixel>
QImage normalizeFloatRgba(const QImage &image,
                          QImage::Format unpremultipliedFormat,
                          bool hasAlpha) {
    const QImage source = image.format() == QImage::Format_RGBA16FPx4_Premultiplied ||
                                  image.format() == QImage::Format_RGBA32FPx4_Premultiplied
                              ? image.convertToFormat(unpremultipliedFormat)
                              : image;
    float minimum[3] = {std::numeric_limits<float>::infinity(),
                        std::numeric_limits<float>::infinity(),
                        std::numeric_limits<float>::infinity()};
    float maximum[3] = {-std::numeric_limits<float>::infinity(),
                        -std::numeric_limits<float>::infinity(),
                        -std::numeric_limits<float>::infinity()};
    for (int y = 0; y < source.height(); ++y) {
        const auto *line = reinterpret_cast<const Pixel *>(source.constScanLine(y));
        for (int x = 0; x < source.width(); ++x) {
            const float channels[3] = {line[x].red(), line[x].green(), line[x].blue()};
            for (int channel = 0; channel < 3; ++channel) {
                if (!std::isfinite(channels[channel])) {
                    continue;
                }
                minimum[channel] = std::min(minimum[channel], channels[channel]);
                maximum[channel] = std::max(maximum[channel], channels[channel]);
            }
        }
    }

    QImage normalized(source.size(), hasAlpha ? QImage::Format_RGBA8888 : QImage::Format_RGB888);
    for (int y = 0; y < source.height(); ++y) {
        const auto *pixels = reinterpret_cast<const Pixel *>(source.constScanLine(y));
        auto *target = normalized.scanLine(y);
        const int stride = hasAlpha ? 4 : 3;
        for (int x = 0; x < source.width(); ++x) {
            const Pixel pixel = pixels[x];
            target[x * stride] = scaleFloatChannel(pixel.red(), minimum[0], maximum[0]);
            target[x * stride + 1] = scaleFloatChannel(pixel.green(), minimum[1], maximum[1]);
            target[x * stride + 2] = scaleFloatChannel(pixel.blue(), minimum[2], maximum[2]);
            if (hasAlpha) {
                target[x * stride + 3] = reduceFloatAlpha(pixel.alpha());
            }
        }
    }
    return normalized;
}

struct TiffPage {
    int width = 0;
    int height = 0;
    int bitsPerSample = 0;
    int sampleFormat = 1;
    int samplesPerPixel = 1;
    int compression = 1;
    int planarConfiguration = 1;
    int fillOrder = 1;
    int rowsPerStrip = 0;
    int predictor = 1;
    bool tiled = false;
    int tileWidth = 0;
    int tileLength = 0;
    QVector<quint64> stripOffsets;
    QVector<quint64> stripByteCounts;
    QVector<quint64> tileOffsets;
    QVector<quint64> tileByteCounts;
    QVector<double> values;
};

class TiffReader {
public:
    explicit TiffReader(const QByteArray &bytes)
        : m_bytes(bytes),
          m_littleEndian(bytes.size() >= 2 && bytes.at(0) == 'I' && bytes.at(1) == 'I') {}

    bool isValidHeader() const {
        quint16 magic = 0;
        if (m_bytes.size() < 8 ||
            !(m_littleEndian || (m_bytes.at(0) == 'M' && m_bytes.at(1) == 'M')) ||
            !readU16(2, &magic)) {
            return false;
        }
        if (magic == 43) {
            quint16 offsetSize = 0;
            quint16 reserved = 0;
            quint64 firstIfd = 0;
            m_bigTiff = readU16(4, &offsetSize) && readU16(6, &reserved) &&
                        offsetSize == 8 && reserved == 0 && readU64(8, &firstIfd) &&
                        firstIfd != 0;
            return m_bigTiff;
        }
        quint32 firstIfd = 0;
        return magic == 42 &&
               (m_littleEndian || (m_bytes.at(0) == 'M' && m_bytes.at(1) == 'M')) &&
               readU32(4, &firstIfd) && firstIfd != 0;
    }

    bool readPages(QVector<TiffPage> *pages, QString *error) const {
        if (!pages || !isValidHeader()) {
            return setError(error, QStringLiteral("unsupported TIFF header"));
        }
        pages->clear();
        quint64 ifdOffset = 0;
        if (m_bigTiff) {
            if (!readU64(8, &ifdOffset)) {
                return setError(error, QStringLiteral("invalid TIFF directory offset"));
            }
        } else {
            quint32 classicOffset = 0;
            if (!readU32(4, &classicOffset)) {
                return setError(error, QStringLiteral("invalid TIFF directory offset"));
            }
            ifdOffset = classicOffset;
        }
        QSet<quint64> visited;
        for (int pageIndex = 0; ifdOffset != 0 && pageIndex < 1024; ++pageIndex) {
            if (visited.contains(ifdOffset)) {
                return setError(error, QStringLiteral("cyclic TIFF directory chain"));
            }
            visited.insert(ifdOffset);

            QMap<quint16, QVector<quint64>> tags;
            quint64 nextIfd = 0;
            if (!readIfd(ifdOffset, &tags, &nextIfd, error)) {
                return false;
            }
            TiffPage page;
            if (!buildPage(tags, &page, error)) {
                return false;
            }
            if (!decodePage(&page, error)) {
                return false;
            }
            pages->push_back(std::move(page));
            ifdOffset = nextIfd;
        }
        if (pages->isEmpty()) {
            return setError(error, QStringLiteral("TIFF contains no image directories"));
        }
        if (ifdOffset != 0 && pages->size() >= 1024) {
            return setError(error, QStringLiteral("TIFF contains too many image directories"));
        }
        return true;
    }

private:
    static bool setError(QString *error, const QString &message) {
        if (error) {
            *error = message;
        }
        return false;
    }

    bool hasBytes(quint64 offset, quint64 count) const {
        return offset <= static_cast<quint64>(m_bytes.size()) &&
               count <= static_cast<quint64>(m_bytes.size()) - offset;
    }

    bool readU16(quint64 offset, quint16 *value) const {
        if (!value || !hasBytes(offset, 2)) {
            return false;
        }
        const uchar first = static_cast<uchar>(m_bytes.at(static_cast<int>(offset)));
        const uchar second = static_cast<uchar>(m_bytes.at(static_cast<int>(offset + 1)));
        *value = m_littleEndian ? static_cast<quint16>(first | (second << 8))
                                : static_cast<quint16>((first << 8) | second);
        return true;
    }

    bool readU32(quint64 offset, quint32 *value) const {
        if (!value || !hasBytes(offset, 4)) {
            return false;
        }
        quint32 result = 0;
        if (m_littleEndian) {
            for (int byte = 0; byte < 4; ++byte) {
                result |= static_cast<quint32>(static_cast<uchar>(m_bytes.at(static_cast<int>(offset + byte))))
                          << (byte * 8);
            }
        } else {
            for (int byte = 0; byte < 4; ++byte) {
                result = (result << 8) |
                         static_cast<quint32>(static_cast<uchar>(m_bytes.at(static_cast<int>(offset + byte))));
            }
        }
        *value = result;
        return true;
    }

    bool readU64(quint64 offset, quint64 *value) const {
        if (!value || !hasBytes(offset, 8)) {
            return false;
        }
        quint64 result = 0;
        if (m_littleEndian) {
            for (int byte = 0; byte < 8; ++byte) {
                result |= static_cast<quint64>(static_cast<uchar>(m_bytes.at(static_cast<int>(offset + byte))))
                          << (byte * 8);
            }
        } else {
            for (int byte = 0; byte < 8; ++byte) {
                result = (result << 8) |
                         static_cast<quint64>(static_cast<uchar>(m_bytes.at(static_cast<int>(offset + byte))));
            }
        }
        *value = result;
        return true;
    }

    static quint32 typeSize(quint16 type) {
        switch (type) {
        case 1: // BYTE
        case 2: // ASCII
            return 1;
        case 3: // SHORT
            return 2;
        case 4: // LONG
            return 4;
        case 5: // RATIONAL
            return 8;
        case 6: // SBYTE
        case 7: // UNDEFINED
        case 8: // SSHORT
            return type == 8 ? 2 : 1;
        case 9: // SLONG
            return 4;
        case 10: // SRATIONAL
            return 8;
        case 11: // FLOAT
            return 4;
        case 12: // DOUBLE
            return 8;
        case 16: // LONG8
        case 17: // SLONG8
        case 18: // IFD8
            return 8;
        default:
            return 0;
        }
    }

    bool readValues(quint64 entryOffset, quint16 type, quint64 count,
                    QVector<quint64> *values) const {
        if (!values || count == 0) {
            return false;
        }
        const quint32 size = typeSize(type);
        if (size == 0 || count > static_cast<quint64>(std::numeric_limits<int>::max()) ||
            static_cast<quint64>(size) * count > std::numeric_limits<int>::max()) {
            return false;
        }
        const quint64 byteCount = static_cast<quint64>(size) * count;
        const quint64 inlineBytes = m_bigTiff ? 8 : 4;
        quint64 valueOffset = entryOffset + (m_bigTiff ? 12 : 8);
        if (byteCount > inlineBytes) {
            if (m_bigTiff) {
                if (!readU64(entryOffset + 12, &valueOffset)) {
                    return false;
                }
            } else {
                quint32 classicOffset = 0;
                if (!readU32(entryOffset + 8, &classicOffset)) {
                    return false;
                }
                valueOffset = classicOffset;
            }
        }
        if (!hasBytes(valueOffset, byteCount)) {
            return false;
        }
        values->clear();
        values->reserve(static_cast<int>(count));
        for (quint64 index = 0; index < count; ++index) {
            const quint64 offset = static_cast<quint64>(valueOffset) + index * size;
            quint64 value = 0;
            if (type == 1 || type == 2 || type == 6 || type == 7) {
                value = static_cast<uchar>(m_bytes.at(static_cast<int>(offset)));
            } else if (type == 3) {
                quint16 shortValue = 0;
                if (!readU16(offset, &shortValue)) {
                    return false;
                }
                value = shortValue;
            } else if (type == 4 || type == 9 || type == 11) {
                quint32 longValue = 0;
                if (!readU32(offset, &longValue)) {
                    return false;
                }
                value = longValue;
            } else if (type == 8) {
                quint16 shortValue = 0;
                if (!readU16(offset, &shortValue)) {
                    return false;
                }
                value = shortValue;
            } else if (type == 16 || type == 17 || type == 18) {
                if (!readU64(offset, &value)) {
                    return false;
                }
            }
            values->push_back(value);
        }
        return true;
    }

    bool readIfd(quint64 offset, QMap<quint16, QVector<quint64>> *tags,
                 quint64 *nextIfd, QString *error) const {
        quint64 entryCount = 0;
        const quint64 entrySize = m_bigTiff ? 20 : 12;
        const quint64 nextSize = m_bigTiff ? 8 : 4;
        if (!tags || !nextIfd ||
            (m_bigTiff ? !readU64(offset, &entryCount)
                       : ([&]() {
                             quint16 count = 0;
                             const bool ok = readU16(offset, &count);
                             entryCount = count;
                             return !ok;
                         })()) ||
            entryCount > std::numeric_limits<quint64>::max() / entrySize ||
            !hasBytes(offset + (m_bigTiff ? 8 : 2),
                      entryCount * entrySize + nextSize)) {
            return setError(error, QStringLiteral("invalid TIFF image directory"));
        }
        tags->clear();
        for (quint64 index = 0; index < entryCount; ++index) {
            const quint64 entryOffset = offset + (m_bigTiff ? 8 : 2) + index * entrySize;
            quint16 tag = 0;
            quint16 type = 0;
            quint64 count = 0;
            if (!readU16(entryOffset, &tag) || !readU16(entryOffset + 2, &type) ||
                (m_bigTiff ? !readU64(entryOffset + 4, &count)
                           : ([&]() {
                                 quint32 classicCount = 0;
                                 const bool ok = readU32(entryOffset + 4, &classicCount);
                                 count = classicCount;
                                 return !ok;
                             })())) {
                return setError(error, QStringLiteral("invalid TIFF image directory entry"));
            }
            if (tag == 256 || tag == 257 || tag == 258 || tag == 259 || tag == 262 ||
                tag == 266 || tag == 273 || tag == 277 || tag == 278 || tag == 279 || tag == 284 ||
                tag == 317 || tag == 322 || tag == 323 || tag == 324 || tag == 325 ||
                tag == 339) {
                QVector<quint64> values;
                if (!readValues(entryOffset, type, count, &values)) {
                    return setError(error, QStringLiteral("invalid TIFF tag %1").arg(tag));
                }
                tags->insert(tag, values);
            }
        }
        if (m_bigTiff) {
            if (!readU64(offset + 8 + entryCount * entrySize, nextIfd)) {
                return setError(error, QStringLiteral("invalid TIFF next-directory offset"));
            }
        } else {
            quint32 classicNext = 0;
            if (!readU32(offset + 2 + entryCount * entrySize, &classicNext)) {
                return setError(error, QStringLiteral("invalid TIFF next-directory offset"));
            }
            *nextIfd = classicNext;
        }
        if (*nextIfd == std::numeric_limits<quint64>::max()) {
            return setError(error, QStringLiteral("invalid TIFF next-directory offset"));
        }
        return true;
    }

    static quint64 tagValue(const QMap<quint16, QVector<quint64>> &tags,
                            quint16 tag, quint64 fallback) {
        const QVector<quint64> values = tags.value(tag);
        return values.isEmpty() ? fallback : values.first();
    }

    bool buildPage(const QMap<quint16, QVector<quint64>> &tags,
                   TiffPage *page, QString *error) const {
        if (!page) {
            return setError(error, QStringLiteral("TIFF page output is null"));
        }
        const quint64 width = tagValue(tags, 256, 0);
        const quint64 height = tagValue(tags, 257, 0);
        const quint64 bits = tagValue(tags, 258, 8);
        const quint64 samples = tagValue(tags, 277, 1);
        const quint64 rows = tagValue(tags, 278, height);
        const bool tiled = tags.contains(322) || tags.contains(323) ||
                           tags.contains(324) || tags.contains(325);
        const bool validBits = bits == 1 || bits == 2 || bits == 4 || (bits % 8 == 0);
        if (width == 0 || height == 0 || width > std::numeric_limits<int>::max() ||
            height > std::numeric_limits<int>::max() || bits == 0 || !validBits ||
            bits > 32 || samples == 0 || samples > std::numeric_limits<int>::max() ||
            (!tiled && rows == 0)) {
            return setError(error, QStringLiteral("unsupported TIFF page dimensions or sample format"));
        }
        page->width = static_cast<int>(width);
        page->height = static_cast<int>(height);
        page->bitsPerSample = static_cast<int>(bits);
        page->sampleFormat = static_cast<int>(tagValue(tags, 339, 1));
        page->samplesPerPixel = static_cast<int>(samples);
        page->compression = static_cast<int>(tagValue(tags, 259, 1));
        page->planarConfiguration = static_cast<int>(tagValue(tags, 284, 1));
        page->fillOrder = static_cast<int>(tagValue(tags, 266, 1));
        page->rowsPerStrip = static_cast<int>(qMin<quint64>(rows, height));
        page->predictor = static_cast<int>(tagValue(tags, 317, 1));
        page->tiled = tiled;
        page->tileWidth = static_cast<int>(tagValue(tags, 322, 0));
        page->tileLength = static_cast<int>(tagValue(tags, 323, 0));
        page->stripOffsets = tags.value(273);
        page->stripByteCounts = tags.value(279);
        page->tileOffsets = tags.value(324);
        page->tileByteCounts = tags.value(325);
        const int expectedStrips = page->tiled
                                       ? 0
                                       : (page->height + page->rowsPerStrip - 1) / page->rowsPerStrip;
        const int expectedTiles = page->tiled
                                      ? ((page->tileWidth > 0 && page->tileLength > 0)
                                             ? ((page->width + page->tileWidth - 1) / page->tileWidth) *
                                                   ((page->height + page->tileLength - 1) / page->tileLength)
                                             : 0)
                                      : 0;
        const int expectedValues = page->planarConfiguration == 2
                                       ? (page->tiled ? expectedTiles : expectedStrips) * page->samplesPerPixel
                                       : (page->tiled ? expectedTiles : expectedStrips);
        const bool predictorThreeSupported = page->predictor != 3 ||
                                             (page->sampleFormat == 3 && page->bitsPerSample == 32);
        const bool storagePresent = page->tiled
                                        ? (page->tileWidth > 0 && page->tileLength > 0 &&
                                           page->tileOffsets.size() >= expectedValues &&
                                           page->tileByteCounts.size() >= expectedValues)
                                        : (page->rowsPerStrip > 0 &&
                                           page->stripOffsets.size() >= expectedValues &&
                                           page->stripByteCounts.size() >= expectedValues);
        if (!storagePresent ||
            (page->planarConfiguration != 1 && page->planarConfiguration != 2) ||
            (page->sampleFormat != 1 && page->sampleFormat != 2 && page->sampleFormat != 3) ||
            (page->predictor != 1 && page->predictor != 2 && page->predictor != 3) ||
            page->fillOrder != 1 ||
            (page->bitsPerSample < 8 && page->predictor != 1) ||
            (page->predictor == 2 && page->sampleFormat == 3) ||
            !predictorThreeSupported ||
            (page->compression != 1 && page->compression != 5 && page->compression != 8 &&
             page->compression != 32773)) {
            return setError(error, QStringLiteral("unsupported TIFF strips or compression"));
        }
        return true;
    }

    bool unpackPackBits(const QByteArray &compressed, int expectedSize,
                        QByteArray *decoded) const {
        if (!decoded || expectedSize < 0) {
            return false;
        }
        decoded->clear();
        decoded->reserve(expectedSize);
        int index = 0;
        while (index < compressed.size() && decoded->size() < expectedSize) {
            const qint8 count = static_cast<qint8>(static_cast<uchar>(compressed.at(index++)));
            if (count >= 0) {
                const int literalCount = count + 1;
                if (index + literalCount > compressed.size()) {
                    return false;
                }
                decoded->append(compressed.constData() + index, literalCount);
                index += literalCount;
            } else if (count != -128) {
                if (index >= compressed.size()) {
                    return false;
                }
                decoded->append(compressed.at(index), 1 - count);
                ++index;
            }
        }
        return decoded->size() == expectedSize;
    }

    bool unpackLzw(const QByteArray &compressed, int expectedSize,
                   QByteArray *decoded) const {
        if (!decoded || expectedSize < 0) {
            return false;
        }
        decoded->clear();
        decoded->reserve(expectedSize);
        QVector<QByteArray> dictionary(4096);
        int codeWidth = 9;
        int nextCode = 258;
        int bitPosition = 0;
        QByteArray previous;

        auto reset = [&]() {
            dictionary.fill(QByteArray());
            codeWidth = 9;
            nextCode = 258;
            previous.clear();
        };
        auto readCode = [&](int *code) {
            if (!code || bitPosition + codeWidth > compressed.size() * 8) {
                return false;
            }
            int value = 0;
            for (int bit = 0; bit < codeWidth; ++bit) {
                const int position = bitPosition + bit;
                const uchar byte = static_cast<uchar>(compressed.at(position / 8));
                value = (value << 1) | ((byte >> (7 - (position % 8))) & 1);
            }
            bitPosition += codeWidth;
            *code = value;
            return true;
        };

        reset();
        bool reachedEnd = false;
        while (true) {
            int code = 0;
            if (!readCode(&code)) {
                break;
            }
            if (code == 256) {
                reset();
                continue;
            }
            if (code == 257) {
                reachedEnd = true;
                break;
            }

            QByteArray entry;
            if (code < 256) {
                entry = QByteArray(1, static_cast<char>(code));
            } else if (code < nextCode && !dictionary.at(code).isEmpty()) {
                entry = dictionary.at(code);
            } else if (code == nextCode && !previous.isEmpty()) {
                entry = previous;
                entry.append(previous.at(0));
            } else {
                return false;
            }

            decoded->append(entry);
            if (decoded->size() > expectedSize) {
                return false;
            }
            if (!previous.isEmpty()) {
                if (nextCode >= dictionary.size()) {
                    return false;
                }
                QByteArray next = previous;
                next.append(entry.at(0));
                dictionary[nextCode++] = next;
                if (nextCode == (1 << codeWidth) - 1 && codeWidth < 12) {
                    ++codeWidth;
                }
            }
            previous = entry;
        }
        return reachedEnd && decoded->size() == expectedSize;
    }

    bool unpackDeflate(const QByteArray &compressed, int expectedSize,
                       QByteArray *decoded) const {
        if (!decoded || expectedSize < 0) {
            return false;
        }
        // TIFF Deflate strips contain the same zlib stream used by Qt's
        // public qCompress/qUncompress API, without qCompress's 4-byte size
        // prefix. Re-add that prefix instead of linking a private zlib copy.
        QByteArray framed;
        framed.reserve(compressed.size() + 4);
        framed.append(static_cast<char>((expectedSize >> 24) & 0xff));
        framed.append(static_cast<char>((expectedSize >> 16) & 0xff));
        framed.append(static_cast<char>((expectedSize >> 8) & 0xff));
        framed.append(static_cast<char>(expectedSize & 0xff));
        framed.append(compressed);
        *decoded = qUncompress(framed);
        return decoded->size() == expectedSize;
    }

    bool readSample(const char *bytes, int byteCount, double *value) const {
        if (!bytes || !value || byteCount != m_pageSampleBytes) {
            return false;
        }
        quint32 raw = 0;
        quint16 raw16 = 0;
        if (m_pageSampleBytes == 1) {
            raw = static_cast<uchar>(bytes[0]);
        } else if (m_pageSampleBytes == 2) {
            const uchar first = static_cast<uchar>(bytes[0]);
            const uchar second = static_cast<uchar>(bytes[1]);
            raw16 = m_littleEndian ? static_cast<quint16>(first | (second << 8))
                                   : static_cast<quint16>((first << 8) | second);
            raw = raw16;
        } else if (m_pageSampleBytes == 4) {
            if (m_littleEndian) {
                for (int byte = 0; byte < 4; ++byte) {
                    raw |= static_cast<quint32>(static_cast<uchar>(bytes[byte])) << (byte * 8);
                }
            } else {
                for (int byte = 0; byte < 4; ++byte) {
                    raw = (raw << 8) | static_cast<quint32>(static_cast<uchar>(bytes[byte]));
                }
            }
        }
        if (m_pageSampleFormat == 3) {
            if (m_pageSampleBytes == 2) {
                return false;
            }
            float floatValue = 0.0f;
            std::memcpy(&floatValue, &raw, sizeof(floatValue));
            *value = floatValue;
        } else if (m_pageSampleFormat == 2) {
            if (m_pageSampleBytes == 1) {
                *value = static_cast<qint8>(raw);
            } else if (m_pageSampleBytes == 2) {
                *value = static_cast<qint16>(raw16);
            } else {
                *value = static_cast<qint32>(raw);
            }
        } else {
            *value = static_cast<double>(raw);
        }
        return true;
    }

    bool readPackedSample(const QByteArray &bytes, qint64 bitOffset, int bits,
                          double *value) const {
        if (!value || bits <= 0 || bits > 8 || bitOffset < 0 ||
            bitOffset + bits > static_cast<qint64>(bytes.size()) * 8) {
            return false;
        }
        const int byteOffset = static_cast<int>(bitOffset / 8);
        const int inByte = static_cast<int>(bitOffset % 8);
        const int shift = 8 - inByte - bits;
        const uchar packed = static_cast<uchar>(bytes.at(byteOffset));
        const quint32 mask = (1u << bits) - 1u;
        *value = static_cast<double>((packed >> shift) & mask);
        if (m_pageSampleFormat == 2 && *value >= static_cast<double>(1u << (bits - 1))) {
            *value -= static_cast<double>(1u << bits);
        }
        return true;
    }

    bool decodePage(TiffPage *page, QString *error) const {
        if (!page) {
            return setError(error, QStringLiteral("TIFF page output is null"));
        }
        const bool packedSamples = page->bitsPerSample < 8;
        m_pageSampleBytes = packedSamples ? 0 : page->bitsPerSample / 8;
        m_pageSampleFormat = page->sampleFormat;
        if ((!packedSamples && m_pageSampleBytes != 1 && m_pageSampleBytes != 2 &&
             m_pageSampleBytes != 4) || (packedSamples && page->bitsPerSample != 1 &&
                                         page->bitsPerSample != 2 && page->bitsPerSample != 4)) {
            return setError(error, QStringLiteral("unsupported TIFF sample width"));
        }
        const qint64 valueCount = static_cast<qint64>(page->width) * page->height *
                                  page->samplesPerPixel;
        if (valueCount <= 0 || valueCount > 64LL * 1024LL * 1024LL) {
            return setError(error, QStringLiteral("TIFF page is too large"));
        }
        page->values.fill(0.0, static_cast<int>(valueCount));
        const int blocksAcross = page->tiled
                                  ? (page->width + page->tileWidth - 1) / page->tileWidth
                                  : 1;
        const int blocksDown = page->tiled
                                ? (page->height + page->tileLength - 1) / page->tileLength
                                : (page->height + page->rowsPerStrip - 1) / page->rowsPerStrip;
        const int blocksPerPlane = blocksAcross * blocksDown;
        const int blockCount = page->planarConfiguration == 2
                                   ? blocksPerPlane * page->samplesPerPixel
                                   : blocksPerPlane;
        for (int blockIndex = 0; blockIndex < blockCount; ++blockIndex) {
            const quint64 offset = page->tiled ? page->tileOffsets.at(blockIndex)
                                               : page->stripOffsets.at(blockIndex);
            const quint64 byteCount = page->tiled ? page->tileByteCounts.at(blockIndex)
                                                  : page->stripByteCounts.at(blockIndex);
            if (!hasBytes(offset, byteCount) || byteCount > std::numeric_limits<int>::max()) {
                return setError(error, QStringLiteral("TIFF block is outside the file"));
            }
            const QByteArray compressed = m_bytes.mid(static_cast<int>(offset), static_cast<int>(byteCount));
            const int plane = page->planarConfiguration == 2 ? blockIndex / blocksPerPlane : -1;
            const int localBlock = page->planarConfiguration == 2 ? blockIndex % blocksPerPlane : blockIndex;
            const int blockX = page->tiled ? localBlock % blocksAcross : 0;
            const int blockY = page->tiled ? localBlock / blocksAcross : localBlock;
            const int rowStart = page->tiled ? blockY * page->tileLength
                                             : blockY * page->rowsPerStrip;
            const int columnStart = page->tiled ? blockX * page->tileWidth : 0;
            const int blockWidth = page->tiled ? page->tileWidth : page->width;
            const int encodedRowCount = page->tiled ? page->tileLength :
                                                       qMin(page->rowsPerStrip,
                                                            page->height - rowStart);
            const int rowCount = qMin(encodedRowCount, page->height - rowStart);
            const int columnCount = qMin(blockWidth, page->width - columnStart);
            const int channels = page->planarConfiguration == 2 ? 1 : page->samplesPerPixel;
            const qint64 samplesInRow = static_cast<qint64>(blockWidth) * channels;
            const qint64 rowBytes = packedSamples
                                        ? (samplesInRow * page->bitsPerSample + 7) / 8
                                        : samplesInRow * m_pageSampleBytes;
            const qint64 expectedBytes = rowBytes * encodedRowCount;
            if (rowCount <= 0 || columnCount <= 0 || expectedBytes <= 0 ||
                expectedBytes > std::numeric_limits<int>::max()) {
                return setError(error, QStringLiteral("TIFF block is too large"));
            }
            QByteArray decoded;
            if (page->compression == 1) {
                if (compressed.size() < expectedBytes) {
                    return setError(error, QStringLiteral("TIFF block is truncated"));
                }
                decoded = compressed.left(static_cast<int>(expectedBytes));
            } else if (page->compression == 5) {
                if (!unpackLzw(compressed, static_cast<int>(expectedBytes), &decoded)) {
                    return setError(error, QStringLiteral("invalid TIFF LZW block"));
                }
            } else if (page->compression == 8) {
                if (!unpackDeflate(compressed, static_cast<int>(expectedBytes), &decoded)) {
                    return setError(error, QStringLiteral("invalid TIFF Deflate block"));
                }
            } else if (!unpackPackBits(compressed, static_cast<int>(expectedBytes), &decoded)) {
                return setError(error, QStringLiteral("invalid TIFF PackBits block"));
            }

            if (page->predictor == 3) {
                // TIFF floating-point Predictor=3 first stores each sample's
                // bytes in separate planes (most-significant plane first for
                // little-endian files), then applies byte-wise horizontal
                // differencing. Undo both stages per row before readSample().
                const int bytePlanes = m_pageSampleBytes;
                const int sampleCount = static_cast<int>(rowBytes / bytePlanes);
                const int stride = page->planarConfiguration == 1
                                       ? page->samplesPerPixel
                                       : 1;
                if (bytePlanes != 4 || sampleCount <= 0 || stride <= 0 ||
                    sampleCount % stride != 0) {
                    return setError(error, QStringLiteral("unsupported TIFF floating-point predictor"));
                }
                for (int row = 0; row < encodedRowCount; ++row) {
                    uchar *rowData = reinterpret_cast<uchar *>(decoded.data() +
                                                                  static_cast<qint64>(row) * rowBytes);
                    for (int index = stride; index < rowBytes; ++index) {
                        rowData[index] = static_cast<uchar>(rowData[index] +
                                                            rowData[index - stride]);
                    }
                    const QByteArray planes(reinterpret_cast<const char *>(rowData),
                                            static_cast<int>(rowBytes));
                    for (int sample = 0; sample < sampleCount; ++sample) {
                        for (int byte = 0; byte < bytePlanes; ++byte) {
                            const int plane = m_littleEndian ? bytePlanes - byte - 1 : byte;
                            rowData[sample * bytePlanes + byte] =
                                static_cast<uchar>(planes.at(plane * sampleCount + sample));
                        }
                    }
                }
            }

            for (int row = 0; row < rowCount; ++row) {
                QVector<double> previous(page->samplesPerPixel, 0.0);
                for (int x = 0; x < columnCount; ++x) {
                    for (int channel = 0; channel < channels; ++channel) {
                        const int outputChannel = page->planarConfiguration == 2 ? plane : channel;
                        const int rawChannel = page->planarConfiguration == 2 ? 0 : channel;
                        const qint64 sampleOffset = static_cast<qint64>(x) * channels + rawChannel;
                        double value = 0.0;
                        const bool readOk = packedSamples
                                                ? readPackedSample(decoded,
                                                                   static_cast<qint64>(row) * rowBytes * 8 +
                                                                       sampleOffset * page->bitsPerSample,
                                                                   page->bitsPerSample,
                                                                   &value)
                                                : readSample(decoded.constData() +
                                                                 static_cast<qint64>(row) * rowBytes +
                                                                     sampleOffset * m_pageSampleBytes,
                                                             m_pageSampleBytes,
                                                             &value);
                        if (!readOk) {
                            return setError(error, QStringLiteral("unsupported TIFF sample encoding"));
                        }
                        if (page->predictor == 2 && x > 0) {
                            value += previous.at(outputChannel);
                            const double modulus = std::ldexp(1.0, page->bitsPerSample);
                            value = std::fmod(value, modulus);
                            if (value < 0.0) {
                                value += modulus;
                            }
                            if (page->sampleFormat == 2 && value >= modulus / 2.0) {
                                value -= modulus;
                            }
                        }
                        previous[outputChannel] = value;
                        const int pixelIndex = (rowStart + row) * page->width + columnStart + x;
                        page->values[pixelIndex * page->samplesPerPixel + outputChannel] = value;
                    }
                }
            }
        }
        return true;
    }

    const QByteArray &m_bytes;
    bool m_littleEndian = true;
    mutable bool m_bigTiff = false;
    mutable int m_pageSampleBytes = 1;
    mutable int m_pageSampleFormat = 1;
};

uchar scaleTiffChannel(double value, double minimum, double maximum) {
    if (!std::isfinite(value) || !std::isfinite(minimum) || !std::isfinite(maximum) ||
        maximum <= minimum) {
        return 0;
    }
    const double normalized = std::clamp((value - minimum) / (maximum - minimum), 0.0, 1.0);
    return static_cast<uchar>(normalized * 255.0);
}

bool readTiffFallback(const QString &path, QImage *image, bool *prefer,
                      QString *errorMessage) {
    if (image) {
        *image = {};
    }
    if (prefer) {
        *prefer = false;
    }
    if (errorMessage) {
        errorMessage->clear();
    }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = file.errorString();
        }
        return false;
    }
    const QByteArray bytes = file.readAll();
    TiffReader reader(bytes);
    QVector<TiffPage> pages;
    if (!reader.readPages(&pages, errorMessage)) {
        return false;
    }
    const TiffPage &first = pages.first();
    const bool stack = pages.size() > 1 && first.samplesPerPixel == 1;
    for (const TiffPage &page : pages) {
        if (page.samplesPerPixel != 1 || page.width != first.width || page.height != first.height) {
            if (stack) {
                return false;
            }
        }
    }

    const int outputWidth = stack ? first.height : first.width;
    const int outputHeight = stack ? pages.size() : first.height;
    const int outputChannels = stack
                                   ? (first.width >= 3 ? 3 : 1)
                                   : (first.samplesPerPixel >= 3 ? 3 : 1);
    if (outputWidth <= 0 || outputHeight <= 0 ||
        static_cast<qint64>(outputWidth) * outputHeight > 64LL * 1024LL * 1024LL) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("TIFF display image is too large");
        }
        return false;
    }

    auto valueAt = [&](int x, int y, int channel) {
        if (stack) {
            const TiffPage &page = pages.at(y);
            const int sourceIndex = x * page.width + channel;
            return page.values.at(sourceIndex * page.samplesPerPixel);
        }
        const int sourceChannel = outputChannels == 1 ? 0 : channel;
        const int sourceIndex = (y * first.width + x) * first.samplesPerPixel + sourceChannel;
        return first.values.at(sourceIndex);
    };

    double minimum[3] = {std::numeric_limits<double>::infinity(),
                         std::numeric_limits<double>::infinity(),
                         std::numeric_limits<double>::infinity()};
    double maximum[3] = {-std::numeric_limits<double>::infinity(),
                         -std::numeric_limits<double>::infinity(),
                         -std::numeric_limits<double>::infinity()};
    for (int y = 0; y < outputHeight; ++y) {
        for (int x = 0; x < outputWidth; ++x) {
            for (int channel = 0; channel < outputChannels; ++channel) {
                const double value = valueAt(x, y, channel);
                if (!std::isfinite(value)) {
                    continue;
                }
                minimum[channel] = qMin(minimum[channel], value);
                maximum[channel] = qMax(maximum[channel], value);
            }
        }
    }

    *image = QImage(QSize(outputWidth, outputHeight),
                    outputChannels == 1 ? QImage::Format_Grayscale8 : QImage::Format_RGB888);
    for (int y = 0; y < outputHeight; ++y) {
        uchar *line = image->scanLine(y);
        for (int x = 0; x < outputWidth; ++x) {
            if (outputChannels == 1) {
                line[x] = scaleTiffChannel(valueAt(x, y, 0), minimum[0], maximum[0]);
            } else {
                for (int channel = 0; channel < 3; ++channel) {
                    line[x * 3 + channel] =
                        scaleTiffChannel(valueAt(x, y, channel), minimum[channel], maximum[channel]);
                }
            }
        }
    }
    if (prefer) {
        *prefer = stack || first.bitsPerSample != 8 || first.sampleFormat != 1 ||
                  (first.samplesPerPixel > 1 && first.samplesPerPixel < 3);
    }
    return true;
}
}

QImage ImageIO::readForDisplay(const QString &path, QString *errorMessage) {
    if (errorMessage) {
        errorMessage->clear();
    }
    QImageReader reader(path);
    reader.setAutoTransform(true);
    const QImage image = reader.read();
    if (QFileInfo(path).suffix().compare(QStringLiteral("tif"), Qt::CaseInsensitive) == 0 ||
        QFileInfo(path).suffix().compare(QStringLiteral("tiff"), Qt::CaseInsensitive) == 0) {
        QImage fallback;
        bool preferFallback = false;
        QString fallbackError;
        if (readTiffFallback(path, &fallback, &preferFallback, &fallbackError) &&
            (image.isNull() || preferFallback || image.size() != fallback.size())) {
            return fallback;
        }
    }
    if (image.isNull() && errorMessage) {
        *errorMessage = reader.errorString();
    }
    return image.isNull() ? QImage() : normalizeForDisplay(image);
}

QImage ImageIO::readPreview(const QString &path, int maxSide, QSize *sourceSize,
                            QString *errorMessage) {
    if (sourceSize) {
        *sourceSize = {};
    }
    if (errorMessage) {
        errorMessage->clear();
    }
    if (maxSide <= 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("preview size must be positive");
        }
        return {};
    }

    QImageReader reader(path);
    reader.setAutoTransform(true);
    const QSize encodedSize = reader.size();
    if (encodedSize.isValid() && !encodedSize.isEmpty()) {
        QSize displaySize = encodedSize;
        const auto transformation = reader.transformation();
        if (transformation & QImageIOHandler::TransformationRotate90) {
            displaySize = QSize(encodedSize.height(), encodedSize.width());
        }
        if (sourceSize) {
            *sourceSize = displaySize;
        }

        const double scale = qMin(1.0,
                                  qMin(static_cast<double>(maxSide) / displaySize.width(),
                                       static_cast<double>(maxSide) / displaySize.height()));
        const QSize targetSize(qMax(1, qRound(displaySize.width() * scale)),
                               qMax(1, qRound(displaySize.height() * scale)));
        const QSize decoderSize = (transformation & QImageIOHandler::TransformationRotate90)
                                      ? QSize(targetSize.height(), targetSize.width())
                                      : targetSize;
        reader.setScaledSize(decoderSize);
        QImage image = reader.read();
        if (!image.isNull()) {
            image = normalizeForDisplay(image);
            if (qMax(image.width(), image.height()) > maxSide) {
                image = image.scaled(targetSize, Qt::KeepAspectRatio, Qt::FastTransformation);
            }
            return image;
        }
        if (errorMessage) {
            *errorMessage = reader.errorString();
        }
    }

    // Some formats, notably the custom TIFF fallback, do not expose a useful
    // decoded size through QImageReader. Keep the fallback functional and
    // report the true image dimensions after decoding.
    QString fallbackError;
    QImage image = readForDisplay(path, &fallbackError);
    if (image.isNull()) {
        if (errorMessage && errorMessage->isEmpty()) {
            *errorMessage = fallbackError;
        }
        return {};
    }
    if (sourceSize) {
        *sourceSize = image.size();
    }
    if (qMax(image.width(), image.height()) > maxSide) {
        image = image.scaled(maxSide, maxSide, Qt::KeepAspectRatio, Qt::FastTransformation);
    }
    return image;
}

QImage ImageIO::normalizeForDisplay(const QImage &image) {
    if (image.isNull()) {
        return {};
    }
    switch (image.format()) {
    case QImage::Format_Grayscale16:
        return normalizeGrayscale16(image);
    case QImage::Format_RGBX64:
    case QImage::Format_RGBA64:
    case QImage::Format_RGBA64_Premultiplied:
        return normalizeRgba64(image);
    case QImage::Format_RGBX16FPx4:
        return normalizeFloatRgba<QRgbaFloat16>(image, QImage::Format_RGBA16FPx4, false);
    case QImage::Format_RGBA16FPx4:
    case QImage::Format_RGBA16FPx4_Premultiplied:
        return normalizeFloatRgba<QRgbaFloat16>(image, QImage::Format_RGBA16FPx4, true);
    case QImage::Format_RGBX32FPx4:
        return normalizeFloatRgba<QRgbaFloat32>(image, QImage::Format_RGBA32FPx4, false);
    case QImage::Format_RGBA32FPx4:
    case QImage::Format_RGBA32FPx4_Premultiplied:
        return normalizeFloatRgba<QRgbaFloat32>(image, QImage::Format_RGBA32FPx4, true);
    default:
        return image;
    }
}
