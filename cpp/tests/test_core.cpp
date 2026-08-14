#include <QtCore>
#include <QtTest>

#include <QBuffer>
#include <QFile>
#include <QImage>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QtGui/qrgbafloat.h>

#include <cstring>

#include "core/AnnotationIO.h"
#include "core/AiAssistBridge.h"
#include "core/AiAssistSession.h"
#include "core/LabelListModel.h"
#include "core/LabelMeConfig.h"
#include "core/ImageIO.h"
#include "core/PerformanceMonitor.h"
#include "core/ResourcePaths.h"
#include "core/StringBundle.h"
#include "core/ShortcutRegistry.h"

class CoreTests : public QObject {
    Q_OBJECT

private slots:
    void stringBundleNormalizesLocales();
    void pascalVocRoundTripsChineseLabels();
    void yoloWritesClassesInEncounterOrder();
    void createMlUpdatesExistingImageEntry();
    void createMlRejectsMalformedJson();
    void labelMeRoundTripsRectangleJson();
    void labelMePreservesRectanglePointOrder();
    void labelMeUsesTwoSpaceIndentation();
    void labelMeDoesNotAppendTrailingJsonNewline();
    void labelMeNewFilesUseReferenceVersion();
    void labelMePreservesPolygonPoints();
    void labelMePreservesShapeMetadata();
    void labelMePreservesShapeOtherData();
    void labelMeWritesReferenceOptionalShapeFieldsWhenInputOmitsThem();
    void labelMePreservesNativePointLineCircleLinestripAndOrientedRectangleShapes();
    void labelMePreservesNativeMaskShapeData();
    void labelMePreservesImageDataWhenPresent();
    void labelMeRejectsEmbeddedImageDataDimensionMismatch();
    void labelMeReportsImageDataValidationError();
    void labelMeRepairsInvalidEmbeddedImageDataFromExternalFile();
    void labelMeRejectsSavingEmbeddedImageDataDimensionMismatch();
    void labelMeRejectsExternalImageDimensionMismatchWhenImageDataIsNull();
    void labelMeValidatesOptionalImageDimensionsIndependently();
    void labelMeWritesNullForUnknownImageDimensions();
    void labelMeAppliesExifOrientationToExternalImage();
    void labelMeRejectsMissingExternalImageWhenImageDataIsNull();
    void labelMeRejectsNonObjectShapeFlags();
    void labelMeRejectsNonBoolShapeFlags();
    void labelMeRejectsNonStringShapeDescription();
    void labelMeRejectsNonIntegerGroupId();
    void labelMeRejectsNonStringShapeMask();
    void labelMeRejectsInvalidShapeMaskPng();
    void labelMeRejectsShapeWithoutLabel();
    void labelMeRejectsMalformedShapePoints();
    void labelMeRejectsShapeWithoutShapeType();
    void labelMePreservesUnknownShapeType();
    void labelMeRejectsMissingImagePath();
    void labelMeRejectsMissingImageData();
    void labelMeRejectsMissingShapesArray();
    void labelMeRejectsNonStringImageData();
    void labelMeRejectsNonObjectTopLevelFlags();
    void labelMeRejectsNonBoolTopLevelFlags();
    void labelMePreservesTopLevelFlags();
    void labelMePreservesTopLevelOtherData();
    void labelMeRejectsReservedTopLevelOtherData();
    void labelMeRoundTripsReferencePrimitiveExampleVersion();
    void labelMeNormalizesNullShapeDescriptions();
    void labelMePreservesRelativeImagePath();
    void labelMeNormalizesWindowsImagePath();
    void labelMeWritesRelativeImagePathFromOutputDirectory();
    void labelMeRoundTripsAllReferenceExamples();
    void labelNavigationSelectsTopWhenEmptyAndWraps();
    void shapeRectUsesLabelMeDiagonalPoints();
    void shapeSupportsVertexEditingAndCopy();
    void shapeNearestVertexKeepsFirstEqualDistanceMatch();
    void shapeRotatesOnlyOrientedRectangles();
    void shapeMaskDoesNotExposeVertices();
    void shapeHitTestingMatchesLabelMeTolerance();
    void linestripHitTestingIncludesLabelMeClosingEdge();
    void linestripContainsIncludesLabelMeClosingEdge();
    void maskWithoutBitmapUsesLabelMeBoundingBoxHitTest();
    void shapePointHitTestingUsesScreenSpaceTolerance();
    void shapePointLabelsFollowVertexEdits();
    void shapeToMaskMatchesLabelMePrimitiveSemantics();
    void shapeToMaskRejectsUnsupportedShapeTypes();
    void shapeDefaultsUseLabelMeDraftFillColor();
    void shapeMaskContainsOnlyNonZeroPixels();
    void shapeOverlapScoreCatchesContainedShapes();
    void shapeOverlapUsesLabelMeInclusivePixelMasks();
    void shapeOverlapSkipsUnknownPluginTypesLikeLabelMe();
    void polygonShapeSupportsPointInsertionAndRemovalRules();
    void performanceMonitorReturnsCpuAndMemoryText();
    void shortcutRegistryNormalizesAndLimitsBindings();
    void shortcutRegistryRejectsConflicts();
    void shortcutRegistryPersistsOnlyOverrides();
    void imageIoNormalizesHighBitGrayscale();
    void imageIoReadsBoundedPreview();
    void imageIoPreviewPreservesExifDisplaySize();
    void imageIoNormalizesFloatingPointRgba();
    void imageIoReadsFloatTiffStackLikeLabelMe();
    void imageIoReadsUncompressedBigTiff();
    void imageIoReadsLzwFloatTiffLikeLabelMe();
    void imageIoReadsLzwPredictorTwoTiff();
    void imageIoReadsDeflateFloatTiffLikeLabelMe();
    void imageIoReadsDeflateFloatPredictorThreeTiff();
    void imageIoReadsTiledFloatTiff();
    void imageIoReadsPackedFourBitTiff();
    void resourcePathsFindSharedAssets();
    void aiAssistBridgeBuildsPromptAndParsesShapes();
    void aiAssistBridgeRejectsInvalidShapePointCounts();
    void aiAssistBridgeRejectsMaskWithoutPayload();
    void aiAssistBridgeRejectsMaskDimensionsThatDoNotMatchBoundingBox();
    void aiAssistSuppressesOverlappingShapes();
    void aiAssistKeepsPartialOverlapBelowContainmentThreshold();
    void aiAssistBridgeBuildsTextPrompt();
    void aiAssistSessionReusesLongLivedBridge();
    void aiAssistSessionReportsProgressEvents();
    void labelMeConfigParsesNestedOptionsAndLists();
    void labelMeConfigParsesInlineYamlText();
    void labelMeConfigParsesInlineFlowMapping();
    void labelMeConfigParsesFlowMappings();
    void labelMeConfigUpdatesDottedValueAtomically();
    void labelMeConfigPrunesDefaultsAndEmptyParents();
    void labelMeConfigPrunesDefaultShortcutLists();
    void labelMeConfigPreservesUntouchedComments();
    void labelMeConfigRejectsUnknownAndConflictingKeys();
    void labelMeConfigOverwritesTopLevelSequence();
    void labelMeConfigQuotesYamlBooleanLikeLabels();
    void labelMeConfigMigratesLegacyAiModelNames();
    void labelMeImageDataMatchesDisplayableEncodingRules();
    void labelMeImageDataUsesTiffFallbackForMultibandInput();
};

namespace {
QString repoRootPath() {
    QDir dir(QCoreApplication::applicationDirPath());
    for (int i = 0; i < 8; ++i) {
        if (dir.exists(QStringLiteral("refs/labelme/examples/primitives/primitives.json"))) {
            return dir.absolutePath();
        }
        if (!dir.cdUp()) {
            break;
        }
    }
    QDir current(QDir::currentPath());
    for (int i = 0; i < 8; ++i) {
        if (current.exists(QStringLiteral("refs/labelme/examples/primitives/primitives.json"))) {
            return current.absolutePath();
        }
        if (!current.cdUp()) {
            break;
        }
    }
    return {};
}

void compareJsonValue(const QJsonValue &actual, const QJsonValue &expected, const QString &context) {
    const QJsonDocument actualDoc(QJsonObject{{QStringLiteral("v"), actual}});
    const QJsonDocument expectedDoc(QJsonObject{{QStringLiteral("v"), expected}});
    const QByteArray actualJson = actualDoc.toJson(QJsonDocument::Compact);
    const QByteArray expectedJson = expectedDoc.toJson(QJsonDocument::Compact);
    QVERIFY2(actualJson == expectedJson,
             qPrintable(QStringLiteral("%1 actual=%2 expected=%3")
                            .arg(context,
                                 QString::fromUtf8(actualJson),
                                 QString::fromUtf8(expectedJson))));
}

void compareShapeField(const QJsonObject &actualShape, const QJsonObject &expectedShape, const QString &field, const QString &context) {
    if (field == QStringLiteral("description") || field == QStringLiteral("mask")) {
        QVERIFY2(actualShape.contains(field), qPrintable(context + QStringLiteral(" missing normalized ") + field));
        if (expectedShape.contains(field)) {
            QJsonValue expected = expectedShape.value(field);
            if (field == QStringLiteral("description") && expected.isNull()) {
                expected = QString();
            }
            compareJsonValue(actualShape.value(field), expected, context + QStringLiteral(".") + field);
        } else if (field == QStringLiteral("description")) {
            compareJsonValue(actualShape.value(field), QJsonValue(QString()), context + QStringLiteral(".") + field);
        } else {
            compareJsonValue(actualShape.value(field), QJsonValue(QJsonValue::Null), context + QStringLiteral(".") + field);
        }
        return;
    }
    QCOMPARE(actualShape.contains(field), expectedShape.contains(field));
    if (expectedShape.contains(field)) {
        compareJsonValue(actualShape.value(field), expectedShape.value(field), context + QStringLiteral(".") + field);
    }
}

bool writeSolidImage(const QString &path, const QSize &size) {
    QDir().mkpath(QFileInfo(path).absolutePath());
    QImage image(size, QImage::Format_RGB32);
    image.fill(Qt::white);
    return image.save(path);
}

bool writeFloatStackTiff(const QString &path) {
    constexpr int pageCount = 3;
    constexpr int pageHeight = 2;
    constexpr int pageWidth = 4;
    constexpr int entryCount = 10;
    constexpr quint32 firstIfdOffset = 8;
    constexpr quint32 ifdSize = 2 + entryCount * 12 + 4;
    constexpr quint32 dataStart = firstIfdOffset + pageCount * ifdSize;
    constexpr quint32 bytesPerPage = pageHeight * pageWidth * sizeof(float);

    QByteArray data;
    data.resize(static_cast<int>(dataStart + pageCount * bytesPerPage));
    data.fill('\0');
    data[0] = 'I';
    data[1] = 'I';
    auto put16 = [&data](quint32 offset, quint16 value) {
        data[static_cast<int>(offset)] = static_cast<char>(value & 0xff);
        data[static_cast<int>(offset + 1)] = static_cast<char>((value >> 8) & 0xff);
    };
    auto put32 = [&data](quint32 offset, quint32 value) {
        for (int byte = 0; byte < 4; ++byte) {
            data[static_cast<int>(offset + byte)] =
                static_cast<char>((value >> (byte * 8)) & 0xff);
        }
    };
    auto putFloat = [&put32](quint32 offset, float value) {
        quint32 raw = 0;
        static_assert(sizeof(raw) == sizeof(value));
        std::memcpy(&raw, &value, sizeof(raw));
        put32(offset, raw);
    };

    put16(2, 42);
    put32(4, firstIfdOffset);
    for (int page = 0; page < pageCount; ++page) {
        const quint32 ifd = firstIfdOffset + static_cast<quint32>(page) * ifdSize;
        const quint32 strip = dataStart + static_cast<quint32>(page) * bytesPerPage;
        put16(ifd, entryCount);
        auto entry = [&](int index, quint16 tag, quint16 type, quint32 count, quint32 value) {
            const quint32 offset = ifd + 2 + static_cast<quint32>(index) * 12;
            put16(offset, tag);
            put16(offset + 2, type);
            put32(offset + 4, count);
            put32(offset + 8, value);
        };
        entry(0, 256, 4, 1, pageWidth);
        entry(1, 257, 4, 1, pageHeight);
        entry(2, 258, 3, 1, 32);
        entry(3, 259, 3, 1, 1);
        entry(4, 262, 3, 1, 1);
        entry(5, 273, 4, 1, strip);
        entry(6, 277, 3, 1, 1);
        entry(7, 278, 4, 1, pageHeight);
        entry(8, 279, 4, 1, bytesPerPage);
        entry(9, 339, 3, 1, 3);
        put32(ifd + 2 + entryCount * 12,
              page + 1 < pageCount ? ifd + ifdSize : 0);

        for (int y = 0; y < pageHeight; ++y) {
            for (int x = 0; x < pageWidth; ++x) {
                const float value = static_cast<float>(page * 100 + y + x * 10);
                const quint32 pixel = strip + static_cast<quint32>((y * pageWidth + x) * sizeof(float));
                putFloat(pixel, value);
            }
        }
    }

    QFile file(path);
    return file.open(QIODevice::WriteOnly) && file.write(data) == data.size();
}

bool writeUncompressedBigTiff(const QString &path) {
    constexpr int width = 2;
    constexpr int height = 2;
    constexpr int entryCount = 10;
    constexpr quint64 firstIfdOffset = 16;
    constexpr quint64 ifdSize = 8 + entryCount * 20 + 8;
    constexpr quint64 dataOffset = firstIfdOffset + ifdSize;

    QByteArray data(static_cast<qsizetype>(dataOffset + width * height), '\0');
    data[0] = 'I';
    data[1] = 'I';
    auto put16 = [&data](quint64 offset, quint16 value) {
        data[static_cast<qsizetype>(offset)] = static_cast<char>(value & 0xff);
        data[static_cast<qsizetype>(offset + 1)] = static_cast<char>((value >> 8) & 0xff);
    };
    auto put64 = [&data](quint64 offset, quint64 value) {
        for (int byte = 0; byte < 8; ++byte) {
            data[static_cast<qsizetype>(offset + byte)] =
                static_cast<char>((value >> (byte * 8)) & 0xff);
        }
    };
    put16(2, 43);
    put16(4, 8);
    put16(6, 0);
    put64(8, firstIfdOffset);

    put64(firstIfdOffset, entryCount);
    auto entry = [&](int index, quint16 tag, quint16 type, quint64 count, quint64 value) {
        const quint64 offset = firstIfdOffset + 8 + static_cast<quint64>(index) * 20;
        put16(offset, tag);
        put16(offset + 2, type);
        put64(offset + 4, count);
        put64(offset + 12, value);
    };
    entry(0, 256, 4, 1, width);
    entry(1, 257, 4, 1, height);
    entry(2, 258, 3, 1, 8);
    entry(3, 259, 3, 1, 1);
    entry(4, 262, 3, 1, 1);
    entry(5, 273, 16, 1, dataOffset);
    entry(6, 277, 3, 1, 1);
    entry(7, 278, 16, 1, height);
    entry(8, 279, 16, 1, width * height);
    entry(9, 339, 3, 1, 1);
    put64(firstIfdOffset + 8 + static_cast<quint64>(entryCount) * 20, 0);

    const uchar pixels[] = {0, 64, 128, 255};
    for (int index = 0; index < 4; ++index) {
        data[static_cast<qsizetype>(dataOffset + index)] = static_cast<char>(pixels[index]);
    }
    QFile file(path);
    return file.open(QIODevice::WriteOnly) && file.write(data) == data.size();
}

QByteArray packTiffLzwLiteralBytes(const QByteArray &raw) {
    QByteArray packed;
    uchar current = 0;
    int bits = 0;
    int codeWidth = 9;
    int nextCode = 258;
    bool hasPrevious = false;
    auto writeCode = [&](int code) {
        for (int bit = codeWidth - 1; bit >= 0; --bit) {
            current = static_cast<uchar>((current << 1) | ((code >> bit) & 1));
            ++bits;
            if (bits == 8) {
                packed.append(static_cast<char>(current));
                current = 0;
                bits = 0;
            }
        }
    };

    writeCode(256); // clear
    for (const char byte : raw) {
        writeCode(static_cast<uchar>(byte));
        if (hasPrevious) {
            ++nextCode;
            if (nextCode == (1 << codeWidth) - 1 && codeWidth < 12) {
                ++codeWidth;
            }
        }
        hasPrevious = true;
    }
    writeCode(257); // end of information
    if (bits != 0) {
        packed.append(static_cast<char>(current << (8 - bits)));
    }
    return packed;
}

bool writeFloatCompressedTiff(const QString &path, int compression, bool predictor3 = false) {
    constexpr int width = 20;
    constexpr int height = 30;
    const int entryCount = predictor3 ? 11 : 10;
    constexpr quint32 firstIfdOffset = 8;
    const quint32 ifdSize = 2 + static_cast<quint32>(entryCount) * 12 + 4;
    const quint32 dataStart = firstIfdOffset + ifdSize;

    QByteArray raw;
    auto appendFloat = [&raw](float value) {
        quint32 bits = 0;
        std::memcpy(&bits, &value, sizeof(bits));
        for (int byte = 0; byte < 4; ++byte) {
            raw.append(static_cast<char>((bits >> (byte * 8)) & 0xff));
        }
    };
    for (int index = 0; index < width * height; ++index) {
        appendFloat(static_cast<float>(index % 4) / 3.0f);
    }
    QByteArray encodedRaw = raw;
    if (predictor3) {
        encodedRaw.clear();
        encodedRaw.reserve(raw.size());
        for (int y = 0; y < height; ++y) {
            const int rowOffset = y * width * 4;
            // TIFF floating-point Predictor=3 stores byte planes from the
            // most significant byte to the least significant byte, then
            // applies unsigned horizontal differencing to each plane.
            for (int plane = 3; plane >= 0; --plane) {
                for (int x = 0; x < width; ++x) {
                    const uchar current = static_cast<uchar>(raw.at(rowOffset + x * 4 + plane));
                    encodedRaw.append(static_cast<char>(current));
                }
            }
            const int rowStart = encodedRaw.size() - width * 4;
            for (int index = encodedRaw.size() - 1; index >= rowStart + 1; --index) {
                encodedRaw[index] = static_cast<char>(
                    static_cast<uchar>(encodedRaw.at(index)) -
                    static_cast<uchar>(encodedRaw.at(index - 1)));
            }
        }
    }
    const QByteArray compressed = compression == 5
                                      ? packTiffLzwLiteralBytes(encodedRaw)
                                      : qCompress(encodedRaw, 9).mid(4);

    QByteArray data(static_cast<int>(dataStart + compressed.size()), '\0');
    data[0] = 'I';
    data[1] = 'I';
    auto put16 = [&data](quint32 offset, quint16 value) {
        data[static_cast<int>(offset)] = static_cast<char>(value & 0xff);
        data[static_cast<int>(offset + 1)] = static_cast<char>((value >> 8) & 0xff);
    };
    auto put32 = [&data](quint32 offset, quint32 value) {
        for (int byte = 0; byte < 4; ++byte) {
            data[static_cast<int>(offset + byte)] =
                static_cast<char>((value >> (byte * 8)) & 0xff);
        }
    };

    put16(2, 42);
    put32(4, firstIfdOffset);
    put16(firstIfdOffset, entryCount);
    auto entry = [&](int index, quint16 tag, quint16 type, quint32 count, quint32 value) {
        const quint32 offset = firstIfdOffset + 2 + static_cast<quint32>(index) * 12;
        put16(offset, tag);
        put16(offset + 2, type);
        put32(offset + 4, count);
        put32(offset + 8, value);
    };
    entry(0, 256, 4, 1, width);
    entry(1, 257, 4, 1, height);
    entry(2, 258, 3, 1, 32);
    entry(3, 259, 3, 1, static_cast<quint32>(compression));
    entry(4, 262, 3, 1, 1);
    entry(5, 273, 4, 1, dataStart);
    entry(6, 277, 3, 1, 1);
    entry(7, 278, 4, 1, height);
    entry(8, 279, 4, 1, static_cast<quint32>(compressed.size()));
    if (predictor3) {
        entry(9, 317, 3, 1, 3);
        entry(10, 339, 3, 1, 3);
    } else {
        entry(9, 339, 3, 1, 3);
    }
    for (int index = 0; index < compressed.size(); ++index) {
        data[static_cast<int>(dataStart) + index] = compressed.at(index);
    }
    QFile file(path);
    return file.open(QIODevice::WriteOnly) && file.write(data) == data.size();
}

bool writeLzwFloatTiff(const QString &path) {
    return writeFloatCompressedTiff(path, 5);
}

bool writeDeflateFloatTiff(const QString &path) {
    return writeFloatCompressedTiff(path, 8);
}

bool writeDeflateFloatPredictorThreeTiff(const QString &path) {
    return writeFloatCompressedTiff(path, 8, true);
}

bool writeTiledFloatTiff(const QString &path) {
    constexpr int width = 5;
    constexpr int height = 4;
    constexpr int tileWidth = 3;
    constexpr int tileLength = 2;
    constexpr int tilesAcross = (width + tileWidth - 1) / tileWidth;
    constexpr int tilesDown = (height + tileLength - 1) / tileLength;
    constexpr int tileCount = tilesAcross * tilesDown;
    constexpr int entryCount = 11;
    constexpr quint32 firstIfdOffset = 8;
    constexpr quint32 ifdSize = 2 + entryCount * 12 + 4;
    constexpr quint32 tileOffsetsArray = firstIfdOffset + ifdSize;
    constexpr quint32 tileCountsArray = tileOffsetsArray + tileCount * 4;
    constexpr quint32 dataStart = tileCountsArray + tileCount * 4;
    constexpr quint32 tileBytes = tileWidth * tileLength * sizeof(float);

    QByteArray data(static_cast<int>(dataStart + tileCount * tileBytes), '\0');
    data[0] = 'I';
    data[1] = 'I';
    auto put16 = [&data](quint32 offset, quint16 value) {
        data[static_cast<int>(offset)] = static_cast<char>(value & 0xff);
        data[static_cast<int>(offset + 1)] = static_cast<char>((value >> 8) & 0xff);
    };
    auto put32 = [&data](quint32 offset, quint32 value) {
        for (int byte = 0; byte < 4; ++byte) {
            data[static_cast<int>(offset + byte)] =
                static_cast<char>((value >> (byte * 8)) & 0xff);
        }
    };
    auto putFloat = [&put32](quint32 offset, float value) {
        quint32 bits = 0;
        std::memcpy(&bits, &value, sizeof(bits));
        put32(offset, bits);
    };
    put16(2, 42);
    put32(4, firstIfdOffset);
    put16(firstIfdOffset, entryCount);
    auto entry = [&](int index, quint16 tag, quint16 type, quint32 count, quint32 value) {
        const quint32 offset = firstIfdOffset + 2 + static_cast<quint32>(index) * 12;
        put16(offset, tag);
        put16(offset + 2, type);
        put32(offset + 4, count);
        put32(offset + 8, value);
    };
    entry(0, 256, 4, 1, width);
    entry(1, 257, 4, 1, height);
    entry(2, 258, 3, 1, 32);
    entry(3, 259, 3, 1, 1);
    entry(4, 262, 3, 1, 1);
    entry(5, 277, 3, 1, 1);
    entry(6, 322, 4, 1, tileWidth);
    entry(7, 323, 4, 1, tileLength);
    entry(8, 324, 4, tileCount, tileOffsetsArray);
    entry(9, 325, 4, tileCount, tileCountsArray);
    entry(10, 339, 3, 1, 3);
    put32(firstIfdOffset + 2 + entryCount * 12, 0);

    for (int tileY = 0; tileY < tilesDown; ++tileY) {
        for (int tileX = 0; tileX < tilesAcross; ++tileX) {
            const int tileIndex = tileY * tilesAcross + tileX;
            const quint32 tileOffset = dataStart + static_cast<quint32>(tileIndex) * tileBytes;
            put32(tileOffsetsArray + static_cast<quint32>(tileIndex) * 4, tileOffset);
            put32(tileCountsArray + static_cast<quint32>(tileIndex) * 4, tileBytes);
            for (int row = 0; row < tileLength; ++row) {
                for (int column = 0; column < tileWidth; ++column) {
                    const int x = tileX * tileWidth + column;
                    const int y = tileY * tileLength + row;
                    const float value = (x < width && y < height)
                                            ? static_cast<float>(y * width + x)
                                            : 0.0f;
                    putFloat(tileOffset + static_cast<quint32>((row * tileWidth + column) * 4), value);
                }
            }
        }
    }
    QFile file(path);
    return file.open(QIODevice::WriteOnly) && file.write(data) == data.size();
}

bool writePackedFourBitTiff(const QString &path) {
    constexpr int width = 8;
    constexpr int height = 1;
    constexpr int entryCount = 10;
    constexpr quint32 firstIfdOffset = 8;
    constexpr quint32 ifdSize = 2 + entryCount * 12 + 4;
    constexpr quint32 dataStart = firstIfdOffset + ifdSize;
    constexpr quint32 dataBytes = (width * 4 + 7) / 8;

    QByteArray data(static_cast<int>(dataStart + dataBytes), '\0');
    data[0] = 'I';
    data[1] = 'I';
    auto put16 = [&data](quint32 offset, quint16 value) {
        data[static_cast<int>(offset)] = static_cast<char>(value & 0xff);
        data[static_cast<int>(offset + 1)] = static_cast<char>((value >> 8) & 0xff);
    };
    auto put32 = [&data](quint32 offset, quint32 value) {
        for (int byte = 0; byte < 4; ++byte) {
            data[static_cast<int>(offset + byte)] =
                static_cast<char>((value >> (byte * 8)) & 0xff);
        }
    };
    put16(2, 42);
    put32(4, firstIfdOffset);
    put16(firstIfdOffset, entryCount);
    auto entry = [&](int index, quint16 tag, quint16 type, quint32 count, quint32 value) {
        const quint32 offset = firstIfdOffset + 2 + static_cast<quint32>(index) * 12;
        put16(offset, tag);
        put16(offset + 2, type);
        put32(offset + 4, count);
        put32(offset + 8, value);
    };
    entry(0, 256, 4, 1, width);
    entry(1, 257, 4, 1, height);
    entry(2, 258, 3, 1, 4);
    entry(3, 259, 3, 1, 1);
    entry(4, 262, 3, 1, 1);
    entry(5, 273, 4, 1, dataStart);
    entry(6, 277, 3, 1, 1);
    entry(7, 278, 4, 1, height);
    entry(8, 279, 4, 1, dataBytes);
    entry(9, 339, 3, 1, 1);
    put32(firstIfdOffset + 2 + entryCount * 12, 0);
    for (int index = 0; index < width; index += 2) {
        data[static_cast<int>(dataStart + index / 2)] =
            static_cast<char>((index << 4) | (index + 1));
    }
    QFile file(path);
    return file.open(QIODevice::WriteOnly) && file.write(data) == data.size();
}

bool writeLzwPredictor16Tiff(const QString &path) {
    constexpr int width = 3;
    constexpr int height = 1;
    constexpr int entryCount = 11;
    constexpr quint32 firstIfdOffset = 8;
    constexpr quint32 ifdSize = 2 + entryCount * 12 + 4;
    constexpr quint32 dataStart = firstIfdOffset + ifdSize;

    const QVector<quint16> predictorBytes = {
        65000,
        static_cast<quint16>(100 - 65000),
        static_cast<quint16>(32750 - 100),
    };
    QByteArray raw;
    for (const quint16 value : predictorBytes) {
        raw.append(static_cast<char>(value & 0xff));
        raw.append(static_cast<char>((value >> 8) & 0xff));
    }
    const QByteArray compressed = packTiffLzwLiteralBytes(raw);

    QByteArray data(static_cast<int>(dataStart + compressed.size()), '\0');
    data[0] = 'I';
    data[1] = 'I';
    auto put16 = [&data](quint32 offset, quint16 value) {
        data[static_cast<int>(offset)] = static_cast<char>(value & 0xff);
        data[static_cast<int>(offset + 1)] = static_cast<char>((value >> 8) & 0xff);
    };
    auto put32 = [&data](quint32 offset, quint32 value) {
        for (int byte = 0; byte < 4; ++byte) {
            data[static_cast<int>(offset + byte)] =
                static_cast<char>((value >> (byte * 8)) & 0xff);
        }
    };
    put16(2, 42);
    put32(4, firstIfdOffset);
    put16(firstIfdOffset, entryCount);
    auto entry = [&](int index, quint16 tag, quint16 type, quint32 count, quint32 value) {
        const quint32 offset = firstIfdOffset + 2 + static_cast<quint32>(index) * 12;
        put16(offset, tag);
        put16(offset + 2, type);
        put32(offset + 4, count);
        put32(offset + 8, value);
    };
    entry(0, 256, 4, 1, width);
    entry(1, 257, 4, 1, height);
    entry(2, 258, 3, 1, 16);
    entry(3, 259, 3, 1, 5);
    entry(4, 262, 3, 1, 1);
    entry(5, 273, 4, 1, dataStart);
    entry(6, 277, 3, 1, 1);
    entry(7, 278, 4, 1, height);
    entry(8, 279, 4, 1, static_cast<quint32>(compressed.size()));
    entry(9, 339, 3, 1, 1);
    entry(10, 317, 3, 1, 2);
    for (int index = 0; index < compressed.size(); ++index) {
        data[static_cast<int>(dataStart) + index] = compressed.at(index);
    }
    QFile file(path);
    return file.open(QIODevice::WriteOnly) && file.write(data) == data.size();
}

bool writeExifOrientedJpeg(const QString &path) {
    QImage source(QSize(2, 3), QImage::Format_RGB32);
    source.fill(Qt::black);
    QByteArray jpeg;
    QBuffer buffer(&jpeg);
    if (!buffer.open(QIODevice::WriteOnly) || !source.save(&buffer, "JPEG")) {
        return false;
    }
    const QByteArray exifSegment = QByteArray::fromHex(
        "ffe100224578696600004d4d002a00000008000101120003000000010006000000000000");
    jpeg.insert(2, exifSegment);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    return file.write(jpeg) == jpeg.size();
}

QString tinyMaskBase64() {
    QImage mask(2, 2, QImage::Format_Grayscale8);
    mask.fill(0);
    mask.setPixelColor(0, 0, QColor(255, 255, 255));
    QByteArray pngData;
    QBuffer buffer(&pngData);
    if (!buffer.open(QIODevice::WriteOnly) || !mask.save(&buffer, "PNG")) {
        return {};
    }
    return QString::fromLatin1(pngData.toBase64());
}

QString tinyImageBase64() {
    QImage image(8, 6, QImage::Format_RGB32);
    image.fill(Qt::white);
    QByteArray imageData;
    QBuffer buffer(&imageData);
    if (!buffer.open(QIODevice::WriteOnly) || !image.save(&buffer, "PNG")) {
        return {};
    }
    return QString::fromLatin1(imageData.toBase64());
}

QString writeMinimalLabelMeFixture(QTemporaryDir &dir, QJsonObject shape, const QString &name) {
    if (!writeSolidImage(dir.filePath(QStringLiteral("image.jpg")), QSize(32, 24))) {
        return {};
    }
    QJsonObject root;
    root["version"] = QStringLiteral("labelme");
    root["flags"] = QJsonObject();
    root["imagePath"] = QStringLiteral("image.jpg");
    root["imageData"] = QJsonValue::Null;
    root["imageWidth"] = 32;
    root["imageHeight"] = 24;
    root["shapes"] = QJsonArray{shape};

    const QString path = dir.filePath(name + QStringLiteral(".json"));
    QFile input(path);
    if (!input.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return {};
    }
    input.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    input.close();
    return path;
}
}

void CoreTests::stringBundleNormalizesLocales() {
    QCOMPARE(StringBundle::normalizeLanguage("zh_CN"), QString("zh-CN"));
    QCOMPARE(StringBundle::normalizeLanguage("zh_TW.UTF-8"), QString("zh-TW"));
    QCOMPARE(StringBundle::normalizeLanguage("ja"), QString("ja-JP"));
    QCOMPARE(StringBundle::normalizeLanguage("UTF-8"), QString("en"));
}

void CoreTests::pascalVocRoundTripsChineseLabels() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AnnotationDocument doc;
    doc.imagePath = dir.filePath(QString::fromUtf8("图像.jpg"));
    doc.imageSize = QSize(1024, 768);
    doc.depth = 3;
    doc.verified = true;
    doc.shapes.push_back(Shape::fromRect(QString::fromUtf8("缺陷"), QRectF(10, 20, 90, 120), true));

    const QString xmlPath = dir.filePath(QString::fromUtf8("标注.xml"));
    QVERIFY2(AnnotationIO::savePascalVoc(xmlPath, doc), qPrintable(xmlPath));

    AnnotationDocument loaded;
    QVERIFY(AnnotationIO::loadPascalVoc(xmlPath, &loaded));
    QVERIFY(loaded.verified);
    QCOMPARE(loaded.shapes.size(), 1);
    QCOMPARE(loaded.shapes[0].label, QString::fromUtf8("缺陷"));
    QVERIFY(loaded.shapes[0].difficult);
    QCOMPARE(loaded.shapes[0].boundingRect().toRect(), QRect(10, 20, 90, 120));
}

void CoreTests::yoloWritesClassesInEncounterOrder() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AnnotationDocument doc;
    doc.imagePath = dir.filePath("image.jpg");
    doc.imageSize = QSize(100, 100);
    doc.depth = 3;
    doc.shapes.push_back(Shape::fromRect("b", QRectF(10, 10, 20, 20), false));
    doc.shapes.push_back(Shape::fromRect("a", QRectF(50, 50, 20, 20), false));

    const QString txtPath = dir.filePath("image.txt");
    QVERIFY(AnnotationIO::saveYolo(txtPath, doc, {}));

    QFile classes(dir.filePath("classes.txt"));
    QVERIFY(classes.open(QIODevice::ReadOnly | QIODevice::Text));
    QCOMPARE(QString::fromUtf8(classes.readAll()), QString("b\na\n"));
}

void CoreTests::createMlUpdatesExistingImageEntry() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AnnotationDocument first;
    first.imagePath = dir.filePath("image.jpg");
    first.imageSize = QSize(100, 100);
    first.shapes.push_back(Shape::fromRect("old", QRectF(0, 0, 10, 10), false));

    const QString jsonPath = dir.filePath("annotations.json");
    QVERIFY(AnnotationIO::saveCreateMl(jsonPath, first));

    AnnotationDocument second = first;
    second.shapes.clear();
    second.shapes.push_back(Shape::fromRect("new", QRectF(20, 20, 30, 30), false));
    QVERIFY(AnnotationIO::saveCreateMl(jsonPath, second));

    AnnotationDocument loaded;
    QVERIFY(AnnotationIO::loadCreateMl(jsonPath, first.imagePath, &loaded));
    QCOMPARE(loaded.shapes.size(), 1);
    QCOMPARE(loaded.shapes[0].label, QString("new"));
}

void CoreTests::createMlRejectsMalformedJson() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("broken.json"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write("[");
    file.close();

    AnnotationDocument document;
    QVERIFY(!AnnotationIO::loadCreateMl(path, QStringLiteral("image.jpg"), &document));
}

void CoreTests::labelMeConfigParsesNestedOptionsAndLists() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("labelmerc.yaml"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write("auto_save: false\n"
               "labels:\n"
               "  - \xE7\x8C\xAB\n"
               "  - dog\n"
               "shape:\n"
               "  line_color: [1, 2, 3, 128]\n"
               "canvas:\n"
               "  fill_drawing: true\n");
    file.close();

    QVariantMap values;
    QString error;
    QVERIFY2(LabelMeConfig::loadFile(path, &values, &error), qPrintable(error));
    QCOMPARE(values.value(QStringLiteral("auto_save")).toBool(), false);
    QCOMPARE(LabelMeConfig::stringList(values.value(QStringLiteral("labels"))),
             QStringList({QString::fromUtf8("猫"), QStringLiteral("dog")}));
    QCOMPARE(values.value(QStringLiteral("shape.line_color")).toList().size(), 4);
    QCOMPARE(values.value(QStringLiteral("canvas.fill_drawing")).toBool(), true);
}

void CoreTests::labelMeConfigParsesInlineYamlText() {
    QVariantMap values;
    QString error;
    QVERIFY2(LabelMeConfig::loadText(QStringLiteral("auto_save: false\n"
                                                     "labels: [cat, \xE7\x8C\xAB]\n"
                                                     "canvas:\n"
                                                     "  fill_drawing: true\n"),
                                    &values,
                                    &error),
             qPrintable(error));
    QCOMPARE(values.value(QStringLiteral("auto_save")).toBool(), false);
    QCOMPARE(LabelMeConfig::stringList(values.value(QStringLiteral("labels"))),
             QStringList({QStringLiteral("cat"), QString::fromUtf8("猫")}));
    QCOMPARE(values.value(QStringLiteral("canvas.fill_drawing")).toBool(), true);
}

void CoreTests::labelMeConfigParsesInlineFlowMapping() {
    QVariantMap values;
    QString error;
    QVERIFY2(LabelMeConfig::loadText(QStringLiteral("{auto_save: false, labels: [cat, dog]}"),
                                     &values,
                                     &error),
             qPrintable(error));
    QCOMPARE(values.value(QStringLiteral("auto_save")).toBool(), false);
    QCOMPARE(LabelMeConfig::stringList(values.value(QStringLiteral("labels"))),
             QStringList({QStringLiteral("cat"), QStringLiteral("dog")}));
}

void CoreTests::labelMeConfigParsesFlowMappings() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("labelmerc.yaml"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write("label_flags: {dog: [occluded, truncated], cat: [reviewed]}\n");
    file.close();

    QVariantMap values;
    QString error;
    QVERIFY2(LabelMeConfig::loadFile(path, &values, &error), qPrintable(error));
    const QVariantMap labelFlags = values.value(QStringLiteral("label_flags")).toMap();
    QVERIFY(labelFlags.contains(QStringLiteral("dog")));
    QCOMPARE(LabelMeConfig::stringList(labelFlags.value(QStringLiteral("dog"))),
             QStringList({QStringLiteral("occluded"), QStringLiteral("truncated")}));
    QCOMPARE(LabelMeConfig::stringList(labelFlags.value(QStringLiteral("cat"))),
             QStringList({QStringLiteral("reviewed")}));
}

void CoreTests::labelMeConfigUpdatesDottedValueAtomically() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("labelmerc.yaml"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write("auto_save: true\ncanvas:\n  fill_drawing: true\nlabels: [cat, dog]\n");
    file.close();

    QString error;
    QVERIFY2(LabelMeConfig::setFileValue(path, QStringLiteral("canvas.fill_drawing"), false, &error),
             qPrintable(error));
    QVERIFY2(LabelMeConfig::setFileValue(path, QStringLiteral("keep_prev"), true, &error),
             qPrintable(error));

    QVariantMap values;
    QVERIFY2(LabelMeConfig::loadFile(path, &values, &error), qPrintable(error));
    QCOMPARE(values.value(QStringLiteral("canvas.fill_drawing")).toBool(), false);
    QCOMPARE(values.value(QStringLiteral("keep_prev")).toBool(), true);
    QCOMPARE(LabelMeConfig::stringList(values.value(QStringLiteral("labels"))),
             QStringList({QStringLiteral("cat"), QStringLiteral("dog")}));
}

void CoreTests::labelMeConfigPrunesDefaultsAndEmptyParents() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("labelmerc.yaml"));

    QString error;
    QVERIFY2(LabelMeConfig::setFileValue(path, QStringLiteral("auto_save"), false, &error),
             qPrintable(error));
    QVERIFY2(LabelMeConfig::setFileValue(path, QStringLiteral("shape.point_size"), 12, &error),
             qPrintable(error));

    QVERIFY2(LabelMeConfig::setFileValue(path, QStringLiteral("shape.point_size"), 8, &error),
             qPrintable(error));
    QVERIFY2(LabelMeConfig::setFileValue(path, QStringLiteral("auto_save"), true, &error),
             qPrintable(error));

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QCOMPARE(file.readAll(), QByteArray());
}

void CoreTests::labelMeConfigPrunesDefaultShortcutLists() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("labelmerc.yaml"));
    QString error;
    const QVariantList customShortcuts = {QStringLiteral("Q"), QStringLiteral("R")};
    const QVariantList defaultShortcuts = {QStringLiteral("D"), QStringLiteral("Ctrl+Shift+D")};
    QVERIFY2(LabelMeConfig::setFileValue(path, QStringLiteral("shortcuts.open_next"), customShortcuts, &error),
             qPrintable(error));
    QVERIFY2(LabelMeConfig::setFileValue(path, QStringLiteral("shortcuts.open_next"), defaultShortcuts, &error),
             qPrintable(error));

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QCOMPARE(file.readAll(), QByteArray());
}

void CoreTests::labelMeConfigPreservesUntouchedComments() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("labelmerc.yaml"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write("# keep this note\nauto_save: false\n");
    file.close();

    QString error;
    QVERIFY2(LabelMeConfig::setFileValue(path, QStringLiteral("keep_prev"), true, &error),
             qPrintable(error));
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QByteArray content = file.readAll();
    QVERIFY(content.contains(QByteArray("# keep this note")));

    QVariantMap values;
    QVERIFY2(LabelMeConfig::loadFile(path, &values, &error), qPrintable(error));
    QCOMPARE(values.value(QStringLiteral("auto_save")).toBool(), false);
    QCOMPARE(values.value(QStringLiteral("keep_prev")).toBool(), true);
}

void CoreTests::labelMeConfigRejectsUnknownAndConflictingKeys() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("labelmerc.yaml"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write("shape: 42\n");
    file.close();

    QString error;
    QVERIFY(!LabelMeConfig::setFileValue(path, QStringLiteral("not_a_real_key"), 1, &error));
    QVERIFY(error.contains(QStringLiteral("Unknown config key")));
    QVERIFY(!LabelMeConfig::setFileValue(path, QStringLiteral("shape.point_size"), 12, &error));
    QVERIFY(error.contains(QStringLiteral("non-mapping")));
}

void CoreTests::labelMeConfigOverwritesTopLevelSequence() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("labelmerc.yaml"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write("# an old non-mapping config\n- one\n- two\n");
    file.close();

    QString error;
    QVERIFY2(LabelMeConfig::setFileValue(path, QStringLiteral("auto_save"), false, &error),
             qPrintable(error));

    QVariantMap values;
    QVERIFY2(LabelMeConfig::loadFile(path, &values, &error), qPrintable(error));
    QCOMPARE(values.value(QStringLiteral("auto_save")).toBool(), false);

    QFile rewritten(path);
    QVERIFY(rewritten.open(QIODevice::ReadOnly | QIODevice::Text));
    QCOMPARE(rewritten.readAll(), QByteArray("auto_save: false\n"));
}

void CoreTests::labelMeConfigQuotesYamlBooleanLikeLabels() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("labelmerc.yaml"));
    const QStringList labels = {
        QStringLiteral("yes"), QStringLiteral("no"), QStringLiteral("on"),
        QStringLiteral("off"), QStringLiteral("true"), QStringLiteral("false"),
        QStringLiteral("null"), QStringLiteral("~")};

    QString error;
    QVERIFY2(LabelMeConfig::setFileValue(path, QStringLiteral("labels"), labels, &error),
             qPrintable(error));

    QFile written(path);
    QVERIFY(written.open(QIODevice::ReadOnly | QIODevice::Text));
    const QByteArray content = written.readAll();
    QVERIFY(content.contains(QByteArray("labels: [\"yes\", \"no\", \"on\", \"off\", "
                                          "\"true\", \"false\", \"null\", \"~\"]")));

    QVariantMap values;
    QVERIFY2(LabelMeConfig::loadFile(path, &values, &error), qPrintable(error));
    QCOMPARE(LabelMeConfig::stringList(values.value(QStringLiteral("labels"))), labels);
}

void CoreTests::labelMeConfigMigratesLegacyAiModelNames() {
    QCOMPARE(LabelMeConfig::migrateAiModelName(QStringLiteral("SegmentAnything (balanced)")),
             QStringLiteral("Sam (balanced)"));
    QCOMPARE(LabelMeConfig::migrateAiModelName(QStringLiteral("SegmentAnything (tiny)")),
             QStringLiteral("Sam (tiny)"));
    QCOMPARE(LabelMeConfig::migrateAiModelName(QStringLiteral("Sam2 (balanced)")),
             QStringLiteral("Sam2 (balanced)"));
    QCOMPARE(LabelMeConfig::migrateAiModelName(QStringLiteral("custom-model")),
             QStringLiteral("custom-model"));
}

void CoreTests::labelMeImageDataMatchesDisplayableEncodingRules() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString jpgPath = dir.filePath(QStringLiteral("source.jpg"));
    QVERIFY(writeSolidImage(jpgPath, QSize(32, 24)));
    QFile originalJpeg(jpgPath);
    QVERIFY(originalJpeg.open(QIODevice::ReadOnly));
    const QByteArray expectedJpeg = originalJpeg.readAll();

    QByteArray encoded;
    QString error;
    QVERIFY2(AnnotationIO::loadImageData(jpgPath, &encoded, &error), qPrintable(error));
    QCOMPARE(encoded, expectedJpeg);

    const QString tiffPath = dir.filePath(QStringLiteral("source.tiff"));
    QImage tiffImage(32, 24, QImage::Format_RGB32);
    tiffImage.fill(Qt::white);
    if (!tiffImage.save(tiffPath, "TIFF")) {
        QSKIP("Qt TIFF image plugin is not available");
    }
    encoded.clear();
    if (!AnnotationIO::loadImageData(tiffPath, &encoded, &error)) {
        QSKIP(qPrintable(QStringLiteral("Qt TIFF decoder is unavailable: %1").arg(error)));
    }
    QVERIFY(encoded.startsWith(QByteArray("\xFF\xD8")));

    const QString alphaTiffPath = dir.filePath(QStringLiteral("alpha.tiff"));
    QImage alphaTiffImage(32, 24, QImage::Format_ARGB32);
    alphaTiffImage.fill(QColor(255, 255, 255, 96));
    QVERIFY(alphaTiffImage.save(alphaTiffPath, "TIFF"));
    encoded.clear();
    if (!AnnotationIO::loadImageData(alphaTiffPath, &encoded, &error)) {
        QSKIP(qPrintable(QStringLiteral("Qt TIFF decoder is unavailable: %1").arg(error)));
    }
    QVERIFY(encoded.startsWith(QByteArray("\x89PNG")));
}

void CoreTests::labelMeRoundTripsRectangleJson() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AnnotationDocument doc;
    doc.imagePath = dir.filePath(QString::fromUtf8("图像.jpg"));
    doc.imageSize = QSize(320, 240);
    doc.depth = 3;
    doc.verified = true;
    doc.shapes.push_back(Shape::fromRect(QString::fromUtf8("缺陷"), QRectF(12, 18, 40, 30), true));
    QVERIFY(writeSolidImage(doc.imagePath, doc.imageSize));

    const QString jsonPath = dir.filePath(QString::fromUtf8("图像.json"));
    QVERIFY(AnnotationIO::saveLabelMe(jsonPath, doc));

    QFile file(jsonPath);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
    QCOMPARE(root.value("imagePath").toString(), QString::fromUtf8("图像.jpg"));
    QVERIFY(root.value("imageData").isNull());
    QCOMPARE(root.value("imageWidth").toInt(), 320);
    QCOMPARE(root.value("imageHeight").toInt(), 240);
    QVERIFY(root.value("flags").toObject().value("verified").toBool());
    const QJsonObject shape = root.value("shapes").toArray().first().toObject();
    QCOMPARE(shape.value("label").toString(), QString::fromUtf8("缺陷"));
    QCOMPARE(shape.value("shape_type").toString(), QStringLiteral("rectangle"));
    QVERIFY(shape.contains(QStringLiteral("description")));
    QCOMPARE(shape.value(QStringLiteral("description")).toString(), QString());
    QVERIFY(shape.contains(QStringLiteral("mask")));
    QVERIFY(shape.value(QStringLiteral("mask")).isNull());
    QVERIFY(shape.value("flags").toObject().value("difficult").toBool());
    QCOMPARE(shape.value("points").toArray().size(), 2);

    AnnotationDocument loaded;
    QVERIFY(AnnotationIO::loadLabelMe(jsonPath, &loaded));
    QVERIFY(loaded.verified);
    QCOMPARE(loaded.imagePath, QFileInfo(doc.imagePath).fileName());
    QCOMPARE(loaded.imageSize, QSize(320, 240));
    QCOMPARE(loaded.shapes.size(), 1);
    QCOMPARE(loaded.shapes[0].label, QString::fromUtf8("缺陷"));
    QVERIFY(loaded.shapes[0].difficult);
    QCOMPARE(loaded.shapes[0].points, QVector<QPointF>({QPointF(12, 18), QPointF(52, 48)}));
    QCOMPARE(loaded.shapes[0].boundingRect().toRect(), QRect(12, 18, 40, 30));
}

void CoreTests::labelMePreservesRectanglePointOrder() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(writeSolidImage(dir.filePath(QStringLiteral("rectangle.jpg")), QSize(80, 60)));

    QJsonObject shape;
    shape[QStringLiteral("label")] = QStringLiteral("defect");
    // LabelMe stores the two diagonal corners as provided, including their order.
    shape[QStringLiteral("points")] = QJsonArray{
        QJsonArray{QJsonValue(42.0), QJsonValue(38.0)},
        QJsonArray{QJsonValue(8.0), QJsonValue(6.0)}};
    shape[QStringLiteral("group_id")] = QJsonValue::Null;
    shape[QStringLiteral("description")] = QString();
    shape[QStringLiteral("shape_type")] = QStringLiteral("rectangle");
    shape[QStringLiteral("flags")] = QJsonObject();
    shape[QStringLiteral("mask")] = QJsonValue::Null;

    QJsonObject root;
    root[QStringLiteral("version")] = QStringLiteral("5.7.0");
    root[QStringLiteral("flags")] = QJsonObject();
    root[QStringLiteral("imagePath")] = QStringLiteral("rectangle.jpg");
    root[QStringLiteral("imageData")] = QJsonValue::Null;
    root[QStringLiteral("imageWidth")] = 80;
    root[QStringLiteral("imageHeight")] = 60;
    root[QStringLiteral("shapes")] = QJsonArray{shape};

    const QString inputPath = dir.filePath(QStringLiteral("rectangle.json"));
    QFile input(inputPath);
    QVERIFY(input.open(QIODevice::WriteOnly | QIODevice::Text));
    input.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    input.close();

    AnnotationDocument loaded;
    QVERIFY(AnnotationIO::loadLabelMe(inputPath, &loaded));
    QCOMPARE(loaded.shapes.size(), 1);
    QCOMPARE(loaded.shapes.first().points,
             QVector<QPointF>({QPointF(42.0, 38.0), QPointF(8.0, 6.0)}));

    const QString outputPath = dir.filePath(QStringLiteral("rectangle-out.json"));
    QVERIFY(AnnotationIO::saveLabelMe(outputPath, loaded));
    QFile output(outputPath);
    QVERIFY(output.open(QIODevice::ReadOnly | QIODevice::Text));
    const QJsonArray writtenPoints = QJsonDocument::fromJson(output.readAll()).object()
                                         .value(QStringLiteral("shapes"))
                                         .toArray()
                                         .first()
                                         .toObject()
                                         .value(QStringLiteral("points"))
                                         .toArray();
    QCOMPARE(writtenPoints, shape.value(QStringLiteral("points")).toArray());
}

void CoreTests::labelMeUsesTwoSpaceIndentation() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AnnotationDocument document;
    document.imagePath = QStringLiteral("image.jpg");
    document.imageSize = QSize(32, 24);
    document.labelMeOtherData.insert(QStringLiteral("metadata"),
                                     QJsonObject{{QStringLiteral("camera"), QStringLiteral("line-1")}});

    const QString outputPath = dir.filePath(QStringLiteral("indent.json"));
    QVERIFY(AnnotationIO::saveLabelMe(outputPath, document));
    QFile output(outputPath);
    QVERIFY(output.open(QIODevice::ReadOnly | QIODevice::Text));
    const QByteArray content = output.readAll();
    QVERIFY(content.contains(QByteArray("\n  \"flags\"")));
    QVERIFY(content.contains(QByteArray("\n    \"camera\"")));
    QVERIFY(!content.contains(QByteArray("\n    \"flags\"")));
}

void CoreTests::labelMeDoesNotAppendTrailingJsonNewline() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AnnotationDocument document;
    document.imagePath = QStringLiteral("image.jpg");
    document.imageSize = QSize(8, 6);
    const QString path = dir.filePath(QStringLiteral("annotation.json"));
    QVERIFY(AnnotationIO::saveLabelMe(path, document));

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QByteArray content = file.readAll();
    QVERIFY2(!content.endsWith('\n') && !content.endsWith('\r'),
             "LabelMe JSON should match json.dump without a trailing newline");
}

void CoreTests::labelMeNewFilesUseReferenceVersion() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AnnotationDocument document;
    document.imagePath = QStringLiteral("new.jpg");
    document.imageSize = QSize(16, 12);

    const QString outputPath = dir.filePath(QStringLiteral("new.json"));
    QVERIFY(AnnotationIO::saveLabelMe(outputPath, document));
    QFile output(outputPath);
    QVERIFY(output.open(QIODevice::ReadOnly | QIODevice::Text));
    QCOMPARE(QJsonDocument::fromJson(output.readAll()).object()
                 .value(QStringLiteral("version"))
                 .toString(),
             QStringLiteral("5.7.0"));
}

void CoreTests::labelMePreservesPolygonPoints() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AnnotationDocument doc;
    doc.imagePath = dir.filePath("poly.jpg");
    doc.imageSize = QSize(200, 120);
    doc.shapes.push_back(Shape::fromPolygon(QStringLiteral("scratch"),
                                            {QPointF(10, 10), QPointF(80, 12), QPointF(70, 55), QPointF(20, 40)},
                                            false));
    QVERIFY(writeSolidImage(doc.imagePath, doc.imageSize));

    const QString jsonPath = dir.filePath("poly.json");
    QVERIFY(AnnotationIO::saveLabelMe(jsonPath, doc));

    QFile file(jsonPath);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
    const QJsonObject shape = root.value("shapes").toArray().first().toObject();
    QCOMPARE(shape.value("shape_type").toString(), QStringLiteral("polygon"));
    QCOMPARE(shape.value("points").toArray().size(), 4);

    AnnotationDocument loaded;
    QVERIFY(AnnotationIO::loadLabelMe(jsonPath, &loaded));
    QCOMPARE(loaded.shapes.size(), 1);
    QCOMPARE(loaded.shapes[0].shapeType, QStringLiteral("polygon"));
    QCOMPARE(loaded.shapes[0].points.size(), 4);
    QCOMPARE(loaded.shapes[0].points[0], QPointF(10, 10));
    QCOMPARE(loaded.shapes[0].points[2], QPointF(70, 55));
    QVERIFY(loaded.shapes[0].contains(QPointF(30, 25)));
    QVERIFY(!loaded.shapes[0].contains(QPointF(150, 90)));
}

void CoreTests::labelMePreservesShapeMetadata() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    Shape shape = Shape::fromPolygon(QStringLiteral("scratch"),
                                     {QPointF(10, 10), QPointF(80, 12), QPointF(70, 55)},
                                     false);
    shape.groupId = 7;
    shape.description = QString::fromUtf8("需要复检");
    shape.flags.insert(QStringLiteral("occluded"), true);
    shape.flags.insert(QStringLiteral("reviewed"), false);

    AnnotationDocument doc;
    doc.imagePath = dir.filePath("meta.jpg");
    doc.imageSize = QSize(160, 120);
    doc.shapes.push_back(shape);
    QVERIFY(writeSolidImage(doc.imagePath, doc.imageSize));

    const QString jsonPath = dir.filePath("meta.json");
    QVERIFY(AnnotationIO::saveLabelMe(jsonPath, doc));

    QFile file(jsonPath);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QJsonObject writtenShape = QJsonDocument::fromJson(file.readAll()).object()
                                         .value("shapes").toArray().first().toObject();
    QCOMPARE(writtenShape.value("group_id").toInt(), 7);
    QCOMPARE(writtenShape.value("description").toString(), QString::fromUtf8("需要复检"));
    QCOMPARE(writtenShape.value("flags").toObject().value("occluded").toBool(), true);
    QCOMPARE(writtenShape.value("flags").toObject().value("reviewed").toBool(), false);

    AnnotationDocument loaded;
    QVERIFY(AnnotationIO::loadLabelMe(jsonPath, &loaded));
    QCOMPARE(loaded.shapes.size(), 1);
    QCOMPARE(loaded.shapes[0].groupId, 7);
    QCOMPARE(loaded.shapes[0].description, QString::fromUtf8("需要复检"));
    QCOMPARE(loaded.shapes[0].flags.value(QStringLiteral("occluded")), true);
    QCOMPARE(loaded.shapes[0].flags.value(QStringLiteral("reviewed")), false);
}

void CoreTests::labelMePreservesShapeOtherData() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(writeSolidImage(dir.filePath(QStringLiteral("shape-extra.png")), QSize(64, 48)));

    QJsonObject shape;
    shape["label"] = QStringLiteral("defect");
    shape["points"] = QJsonArray{QJsonArray{4.0, 5.0}, QJsonArray{24.0, 25.0}};
    shape["group_id"] = QJsonValue::Null;
    shape["description"] = QJsonValue::Null;
    shape["shape_type"] = QStringLiteral("rectangle");
    shape["flags"] = QJsonObject();
    shape["mask"] = QJsonValue::Null;
    shape["score"] = 0.91;
    shape["attributes"] = QJsonObject{{QStringLiteral("source"), QStringLiteral("model")},
                                       {QStringLiteral("reviewed"), false}};

    QJsonObject root;
    root["version"] = QStringLiteral("labelme");
    root["flags"] = QJsonObject();
    root["imagePath"] = QStringLiteral("shape-extra.png");
    root["imageData"] = QJsonValue::Null;
    root["imageWidth"] = 64;
    root["imageHeight"] = 48;
    root["shapes"] = QJsonArray{shape};

    const QString jsonPath = dir.filePath(QStringLiteral("shape-extra.json"));
    QFile input(jsonPath);
    QVERIFY(input.open(QIODevice::WriteOnly | QIODevice::Text));
    input.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    input.close();

    AnnotationDocument loaded;
    QVERIFY(AnnotationIO::loadLabelMe(jsonPath, &loaded));

    const QString outputPath = dir.filePath(QStringLiteral("shape-extra-out.json"));
    QVERIFY(AnnotationIO::saveLabelMe(outputPath, loaded));
    QFile output(outputPath);
    QVERIFY(output.open(QIODevice::ReadOnly | QIODevice::Text));
    const QJsonObject writtenShape = QJsonDocument::fromJson(output.readAll()).object()
                                         .value(QStringLiteral("shapes")).toArray().first().toObject();
    compareJsonValue(writtenShape.value(QStringLiteral("score")), shape.value(QStringLiteral("score")), QStringLiteral("shape.score"));
    compareJsonValue(writtenShape.value(QStringLiteral("attributes")), shape.value(QStringLiteral("attributes")), QStringLiteral("shape.attributes"));
}

void CoreTests::labelMeWritesReferenceOptionalShapeFieldsWhenInputOmitsThem() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(writeSolidImage(dir.filePath(QStringLiteral("optional.jpg")), QSize(32, 24)));

    QJsonObject shape;
    shape[QStringLiteral("label")] = QStringLiteral("defect");
    shape[QStringLiteral("points")] = QJsonArray{QJsonArray{2.0, 3.0}, QJsonArray{14.0, 16.0}};
    shape[QStringLiteral("shape_type")] = QStringLiteral("rectangle");
    shape[QStringLiteral("flags")] = QJsonObject();

    QJsonObject root;
    root[QStringLiteral("version")] = QStringLiteral("5.7.0");
    root[QStringLiteral("flags")] = QJsonObject();
    root[QStringLiteral("imagePath")] = QStringLiteral("optional.jpg");
    root[QStringLiteral("imageData")] = QJsonValue::Null;
    root[QStringLiteral("imageWidth")] = 32;
    root[QStringLiteral("imageHeight")] = 24;
    root[QStringLiteral("shapes")] = QJsonArray{shape};

    const QString inputPath = dir.filePath(QStringLiteral("optional.json"));
    QFile input(inputPath);
    QVERIFY(input.open(QIODevice::WriteOnly | QIODevice::Text));
    input.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    input.close();

    AnnotationDocument loaded;
    QVERIFY(AnnotationIO::loadLabelMe(inputPath, &loaded));
    const QString outputPath = dir.filePath(QStringLiteral("optional-out.json"));
    QVERIFY(AnnotationIO::saveLabelMe(outputPath, loaded));

    QFile output(outputPath);
    QVERIFY(output.open(QIODevice::ReadOnly | QIODevice::Text));
    const QJsonObject writtenShape = QJsonDocument::fromJson(output.readAll()).object()
                                         .value(QStringLiteral("shapes")).toArray().first().toObject();
    QVERIFY(writtenShape.contains(QStringLiteral("description")));
    QCOMPARE(writtenShape.value(QStringLiteral("description")).toString(), QString());
    QVERIFY(writtenShape.contains(QStringLiteral("mask")));
    QVERIFY(writtenShape.value(QStringLiteral("mask")).isNull());
}

void CoreTests::labelMePreservesNativePointLineCircleLinestripAndOrientedRectangleShapes() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    auto pointArray = [](std::initializer_list<QPointF> points) {
        QJsonArray array;
        for (const QPointF &point : points) {
            QJsonArray item;
            item.append(point.x());
            item.append(point.y());
            array.append(item);
        }
        return array;
    };
    auto shapeObject = [&](const QString &label, const QString &shapeType, QJsonArray points) {
        QJsonObject shape;
        shape["label"] = label;
        shape["points"] = points;
        shape["group_id"] = QJsonValue::Null;
        shape["description"] = QString();
        shape["shape_type"] = shapeType;
        shape["flags"] = QJsonObject();
        shape["mask"] = QJsonValue::Null;
        return shape;
    };

    QJsonObject root;
    root["version"] = QStringLiteral("labelme");
    root["flags"] = QJsonObject();
    root["imagePath"] = QStringLiteral("native.jpg");
    root["imageData"] = QJsonValue::Null;
    root["imageWidth"] = 200;
    root["imageHeight"] = 120;
    QVERIFY(writeSolidImage(dir.filePath(QStringLiteral("native.jpg")), QSize(200, 120)));
    QJsonArray shapes;
    shapes.append(shapeObject(QStringLiteral("tip"), QStringLiteral("point"), pointArray({QPointF(15, 18)})));
    shapes.append(shapeObject(QStringLiteral("edge"), QStringLiteral("line"), pointArray({QPointF(20, 25), QPointF(70, 35)})));
    shapes.append(shapeObject(QStringLiteral("round"), QStringLiteral("circle"), pointArray({QPointF(90, 60), QPointF(110, 60)})));
    shapes.append(shapeObject(QStringLiteral("path"),
                              QStringLiteral("linestrip"),
                              pointArray({QPointF(25, 80), QPointF(55, 82), QPointF(75, 95)})));
    shapes.append(shapeObject(QStringLiteral("rotated"),
                              QStringLiteral("oriented_rectangle"),
                              pointArray({QPointF(30, 70), QPointF(80, 60), QPointF(88, 92), QPointF(38, 102)})));
    root["shapes"] = shapes;

    const QString jsonPath = dir.filePath(QStringLiteral("native.json"));
    QFile input(jsonPath);
    QVERIFY(input.open(QIODevice::WriteOnly | QIODevice::Text));
    input.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    input.close();

    AnnotationDocument loaded;
    QVERIFY(AnnotationIO::loadLabelMe(jsonPath, &loaded));
    QCOMPARE(loaded.shapes.size(), 5);
    QCOMPARE(loaded.shapes[0].shapeType, QStringLiteral("point"));
    QCOMPARE(loaded.shapes[0].points, QVector<QPointF>{QPointF(15, 18)});
    QCOMPARE(loaded.shapes[1].shapeType, QStringLiteral("line"));
    QCOMPARE(loaded.shapes[1].points, QVector<QPointF>({QPointF(20, 25), QPointF(70, 35)}));
    QCOMPARE(loaded.shapes[2].shapeType, QStringLiteral("circle"));
    QCOMPARE(loaded.shapes[2].points, QVector<QPointF>({QPointF(90, 60), QPointF(110, 60)}));
    QCOMPARE(loaded.shapes[3].shapeType, QStringLiteral("linestrip"));
    QCOMPARE(loaded.shapes[3].points, QVector<QPointF>({QPointF(25, 80), QPointF(55, 82), QPointF(75, 95)}));
    QCOMPARE(loaded.shapes[4].shapeType, QStringLiteral("oriented_rectangle"));
    QCOMPARE(loaded.shapes[4].points, QVector<QPointF>({QPointF(30, 70), QPointF(80, 60), QPointF(88, 92), QPointF(38, 102)}));

    const QString outputPath = dir.filePath(QStringLiteral("native-out.json"));
    QVERIFY(AnnotationIO::saveLabelMe(outputPath, loaded));
    QFile output(outputPath);
    QVERIFY(output.open(QIODevice::ReadOnly | QIODevice::Text));
    const QJsonArray written = QJsonDocument::fromJson(output.readAll()).object().value(QStringLiteral("shapes")).toArray();
    QCOMPARE(written.at(0).toObject().value(QStringLiteral("shape_type")).toString(), QStringLiteral("point"));
    QCOMPARE(written.at(0).toObject().value(QStringLiteral("points")).toArray().size(), 1);
    QCOMPARE(written.at(1).toObject().value(QStringLiteral("shape_type")).toString(), QStringLiteral("line"));
    QCOMPARE(written.at(1).toObject().value(QStringLiteral("points")).toArray().size(), 2);
    QCOMPARE(written.at(2).toObject().value(QStringLiteral("shape_type")).toString(), QStringLiteral("circle"));
    QCOMPARE(written.at(2).toObject().value(QStringLiteral("points")).toArray().size(), 2);
    QCOMPARE(written.at(3).toObject().value(QStringLiteral("shape_type")).toString(), QStringLiteral("linestrip"));
    QCOMPARE(written.at(3).toObject().value(QStringLiteral("points")).toArray().size(), 3);
    QCOMPARE(written.at(4).toObject().value(QStringLiteral("shape_type")).toString(), QStringLiteral("oriented_rectangle"));
    QCOMPARE(written.at(4).toObject().value(QStringLiteral("points")).toArray().size(), 4);
}

void CoreTests::labelMePreservesNativeMaskShapeData() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString maskPayload = tinyMaskBase64();
    QVERIFY(!maskPayload.isEmpty());
    QJsonArray points;
    for (const QPointF &point : {QPointF(12, 18), QPointF(32, 42)}) {
        QJsonArray item;
        item.append(point.x());
        item.append(point.y());
        points.append(item);
    }

    QJsonObject shape;
    shape["label"] = QStringLiteral("mask_shape");
    shape["points"] = points;
    shape["group_id"] = 3;
    shape["description"] = QStringLiteral("native mask");
    shape["shape_type"] = QStringLiteral("mask");
    shape["flags"] = QJsonObject{{QStringLiteral("occluded"), true}};
    shape["mask"] = maskPayload;

    QJsonObject root;
    root["version"] = QStringLiteral("labelme");
    root["flags"] = QJsonObject();
    root["imagePath"] = QStringLiteral("mask.jpg");
    root["imageData"] = QJsonValue::Null;
    root["imageWidth"] = 64;
    root["imageHeight"] = 48;
    QVERIFY(writeSolidImage(dir.filePath(QStringLiteral("mask.jpg")), QSize(64, 48)));
    root["shapes"] = QJsonArray{shape};

    const QString jsonPath = dir.filePath(QStringLiteral("mask.json"));
    QFile input(jsonPath);
    QVERIFY(input.open(QIODevice::WriteOnly | QIODevice::Text));
    input.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    input.close();

    AnnotationDocument loaded;
    QVERIFY(AnnotationIO::loadLabelMe(jsonPath, &loaded));
    QCOMPARE(loaded.shapes.size(), 1);
    QCOMPARE(loaded.shapes.first().shapeType, QStringLiteral("mask"));
    QCOMPARE(loaded.shapes.first().label, QStringLiteral("mask_shape"));
    QCOMPARE(loaded.shapes.first().points, QVector<QPointF>({QPointF(12, 18), QPointF(32, 42)}));
    QCOMPARE(loaded.shapes.first().groupId, 3);
    QCOMPARE(loaded.shapes.first().description, QStringLiteral("native mask"));
    QCOMPARE(loaded.shapes.first().flags.value(QStringLiteral("occluded")), true);
    QCOMPARE(loaded.shapes.first().maskData, maskPayload);

    const QString outputPath = dir.filePath(QStringLiteral("mask-out.json"));
    QVERIFY(AnnotationIO::saveLabelMe(outputPath, loaded));
    QFile output(outputPath);
    QVERIFY(output.open(QIODevice::ReadOnly | QIODevice::Text));
    const QJsonObject writtenShape = QJsonDocument::fromJson(output.readAll()).object()
                                         .value(QStringLiteral("shapes")).toArray().first().toObject();
    QCOMPARE(writtenShape.value(QStringLiteral("shape_type")).toString(), QStringLiteral("mask"));
    QCOMPARE(writtenShape.value(QStringLiteral("mask")).toString(), maskPayload);
    QCOMPARE(writtenShape.value(QStringLiteral("points")).toArray().size(), 2);
}

void CoreTests::labelMePreservesImageDataWhenPresent() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(12, 8, QImage::Format_RGB32);
    image.fill(Qt::white);
    QByteArray imageBytes;
    QBuffer buffer(&imageBytes);
    QVERIFY(buffer.open(QIODevice::WriteOnly));
    QVERIFY(image.save(&buffer, "PNG"));
    const QString imagePayload = QString::fromLatin1(imageBytes.toBase64());
    QJsonObject root;
    root["version"] = QStringLiteral("labelme");
    root["flags"] = QJsonObject();
    root["imagePath"] = QStringLiteral("embedded.jpg");
    root["imageData"] = imagePayload;
    root["imageWidth"] = 12;
    root["imageHeight"] = 8;
    root["shapes"] = QJsonArray();

    const QString jsonPath = dir.filePath(QStringLiteral("embedded.json"));
    QFile input(jsonPath);
    QVERIFY(input.open(QIODevice::WriteOnly | QIODevice::Text));
    input.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    input.close();

    AnnotationDocument loaded;
    QVERIFY(AnnotationIO::loadLabelMe(jsonPath, &loaded));
    QCOMPARE(loaded.imageData, imagePayload);

    const QString outputPath = dir.filePath(QStringLiteral("embedded-out.json"));
    QVERIFY(AnnotationIO::saveLabelMe(outputPath, loaded));
    QFile output(outputPath);
    QVERIFY(output.open(QIODevice::ReadOnly | QIODevice::Text));
    const QJsonObject written = QJsonDocument::fromJson(output.readAll()).object();
    QCOMPARE(written.value(QStringLiteral("imageData")).toString(), imagePayload);
}

void CoreTests::labelMeRejectsEmbeddedImageDataDimensionMismatch() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(12, 8, QImage::Format_RGB32);
    image.fill(Qt::white);
    QByteArray imageBytes;
    QBuffer buffer(&imageBytes);
    QVERIFY(buffer.open(QIODevice::WriteOnly));
    QVERIFY(image.save(&buffer, "PNG"));

    QJsonObject root;
    root["version"] = QStringLiteral("labelme");
    root["flags"] = QJsonObject();
    root["imagePath"] = QStringLiteral("embedded.png");
    root["imageData"] = QString::fromLatin1(imageBytes.toBase64());
    root["imageWidth"] = 13;
    root["imageHeight"] = 8;
    root["shapes"] = QJsonArray();

    const QString jsonPath = dir.filePath(QStringLiteral("embedded-mismatch.json"));
    QFile input(jsonPath);
    QVERIFY(input.open(QIODevice::WriteOnly | QIODevice::Text));
    input.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    input.close();

    AnnotationDocument loaded;
    QVERIFY(!AnnotationIO::loadLabelMe(jsonPath, &loaded));
}

void CoreTests::labelMeReportsImageDataValidationError() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(writeSolidImage(dir.filePath(QStringLiteral("external.png")), QSize(12, 8)));

    QJsonObject root;
    root[QStringLiteral("version")] = QStringLiteral("labelme");
    root[QStringLiteral("flags")] = QJsonObject();
    root[QStringLiteral("imagePath")] = QStringLiteral("external.png");
    root[QStringLiteral("imageData")] = QStringLiteral("not-a-valid-image");
    root[QStringLiteral("imageWidth")] = 12;
    root[QStringLiteral("imageHeight")] = 8;
    root[QStringLiteral("shapes")] = QJsonArray();

    const QString jsonPath = dir.filePath(QStringLiteral("invalid-image-data.json"));
    QFile input(jsonPath);
    QVERIFY(input.open(QIODevice::WriteOnly | QIODevice::Text));
    input.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    input.close();

    AnnotationDocument loaded;
    QString error;
    QVERIFY(!AnnotationIO::loadLabelMe(jsonPath, &loaded, &error));
    QVERIFY2(error.contains(QStringLiteral("imageData"), Qt::CaseInsensitive), qPrintable(error));
}

void CoreTests::labelMeRepairsInvalidEmbeddedImageDataFromExternalFile() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(writeSolidImage(dir.filePath(QStringLiteral("external.png")), QSize(12, 8)));

    QJsonObject shape;
    shape[QStringLiteral("label")] = QStringLiteral("defect");
    shape[QStringLiteral("points")] = QJsonArray{QJsonArray{2, 3}, QJsonArray{10, 7}};
    shape[QStringLiteral("shape_type")] = QStringLiteral("rectangle");
    shape[QStringLiteral("flags")] = QJsonObject();
    QJsonObject root;
    root[QStringLiteral("version")] = QStringLiteral("labelme");
    root[QStringLiteral("flags")] = QJsonObject();
    root[QStringLiteral("imagePath")] = QStringLiteral("external.png");
    root[QStringLiteral("imageData")] = QStringLiteral("not-a-valid-image");
    root[QStringLiteral("imageWidth")] = 99;
    root[QStringLiteral("imageHeight")] = 99;
    root[QStringLiteral("shapes")] = QJsonArray{shape};

    const QString jsonPath = dir.filePath(QStringLiteral("repairable-image-data.json"));
    QFile input(jsonPath);
    QVERIFY(input.open(QIODevice::WriteOnly | QIODevice::Text));
    input.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    input.close();

    AnnotationDocument loaded;
    QString error;
    QVERIFY(AnnotationIO::loadLabelMe(jsonPath, &loaded, &error, true));
    QVERIFY(loaded.imageDataRepaired);
    QVERIFY(loaded.imageData.isEmpty());
    QCOMPARE(loaded.imageSize, QSize(12, 8));
    QCOMPARE(loaded.shapes.size(), 1);
    QCOMPARE(loaded.shapes.first().label, QStringLiteral("defect"));
    QVERIFY2(!loaded.imageDataRepairMessage.isEmpty(), "repair message must be available");
}

void CoreTests::labelMeRejectsSavingEmbeddedImageDataDimensionMismatch() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(12, 8, QImage::Format_RGB32);
    image.fill(Qt::white);
    QByteArray imageBytes;
    QBuffer buffer(&imageBytes);
    QVERIFY(buffer.open(QIODevice::WriteOnly));
    QVERIFY(image.save(&buffer, "PNG"));

    AnnotationDocument doc;
    doc.imagePath = QStringLiteral("embedded.png");
    doc.imageSize = QSize(13, 8);
    doc.imageData = QString::fromLatin1(imageBytes.toBase64());

    const QString outputPath = dir.filePath(QStringLiteral("embedded-mismatch-out.json"));
    QVERIFY(!AnnotationIO::saveLabelMe(outputPath, doc));
    QVERIFY(!QFileInfo::exists(outputPath));

    doc.imageSize = QSize(13, 0);
    const QString partialOutputPath = dir.filePath(QStringLiteral("embedded-partial-mismatch-out.json"));
    QVERIFY(!AnnotationIO::saveLabelMe(partialOutputPath, doc));
    QVERIFY(!QFileInfo::exists(partialOutputPath));
}

void CoreTests::labelMeRejectsExternalImageDimensionMismatchWhenImageDataIsNull() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(12, 8, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(dir.filePath(QStringLiteral("external.png"))));

    QJsonObject root;
    root["version"] = QStringLiteral("labelme");
    root["flags"] = QJsonObject();
    root["imagePath"] = QStringLiteral("external.png");
    root["imageData"] = QJsonValue::Null;
    root["imageWidth"] = 13;
    root["imageHeight"] = 8;
    root["shapes"] = QJsonArray();

    const QString jsonPath = dir.filePath(QStringLiteral("external-mismatch.json"));
    QFile input(jsonPath);
    QVERIFY(input.open(QIODevice::WriteOnly | QIODevice::Text));
    input.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    input.close();

    AnnotationDocument loaded;
    QVERIFY(!AnnotationIO::loadLabelMe(jsonPath, &loaded));
}

void CoreTests::labelMeValidatesOptionalImageDimensionsIndependently() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString imagePath = dir.filePath(QStringLiteral("partial.png"));
    QVERIFY(writeSolidImage(imagePath, QSize(12, 8)));

    QJsonObject validRoot;
    validRoot[QStringLiteral("version")] = QStringLiteral("labelme");
    validRoot[QStringLiteral("flags")] = QJsonObject();
    validRoot[QStringLiteral("imagePath")] = QStringLiteral("partial.png");
    validRoot[QStringLiteral("imageData")] = QJsonValue::Null;
    validRoot[QStringLiteral("imageHeight")] = 8;
    validRoot[QStringLiteral("shapes")] = QJsonArray();

    const QString validPath = dir.filePath(QStringLiteral("partial-valid.json"));
    QFile validFile(validPath);
    QVERIFY(validFile.open(QIODevice::WriteOnly | QIODevice::Text));
    validFile.write(QJsonDocument(validRoot).toJson());
    validFile.close();

    AnnotationDocument loaded;
    QVERIFY(AnnotationIO::loadLabelMe(validPath, &loaded));
    QCOMPARE(loaded.imageSize, QSize(12, 8));

    validRoot[QStringLiteral("imageWidth")] = 13;
    const QString invalidPath = dir.filePath(QStringLiteral("partial-invalid.json"));
    QFile invalidFile(invalidPath);
    QVERIFY(invalidFile.open(QIODevice::WriteOnly | QIODevice::Text));
    invalidFile.write(QJsonDocument(validRoot).toJson());
    invalidFile.close();

    AnnotationDocument invalidLoaded;
    QVERIFY(!AnnotationIO::loadLabelMe(invalidPath, &invalidLoaded));
}

void CoreTests::labelMeWritesNullForUnknownImageDimensions() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AnnotationDocument document;
    document.imagePath = QStringLiteral("unknown-size.jpg");
    const QString outputPath = dir.filePath(QStringLiteral("unknown-size.json"));
    QVERIFY(AnnotationIO::saveLabelMe(outputPath, document));

    QFile output(outputPath);
    QVERIFY(output.open(QIODevice::ReadOnly | QIODevice::Text));
    const QJsonObject root = QJsonDocument::fromJson(output.readAll()).object();
    QVERIFY(root.value(QStringLiteral("imageWidth")).isNull());
    QVERIFY(root.value(QStringLiteral("imageHeight")).isNull());
}

void CoreTests::labelMeAppliesExifOrientationToExternalImage() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(writeExifOrientedJpeg(dir.filePath(QStringLiteral("oriented.jpg"))));

    QJsonObject root;
    root["version"] = QStringLiteral("labelme");
    root["flags"] = QJsonObject();
    root["imagePath"] = QStringLiteral("oriented.jpg");
    root["imageData"] = QJsonValue::Null;
    root["imageWidth"] = 3;
    root["imageHeight"] = 2;
    root["shapes"] = QJsonArray();

    const QString jsonPath = dir.filePath(QStringLiteral("oriented.json"));
    QFile input(jsonPath);
    QVERIFY(input.open(QIODevice::WriteOnly | QIODevice::Text));
    input.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    input.close();

    AnnotationDocument loaded;
    QVERIFY2(AnnotationIO::loadLabelMe(jsonPath, &loaded), "EXIF-oriented external image should load");
    QCOMPARE(loaded.imageSize, QSize(3, 2));
}

void CoreTests::labelMeRejectsMissingExternalImageWhenImageDataIsNull() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QJsonObject root;
    root["version"] = QStringLiteral("labelme");
    root["flags"] = QJsonObject();
    root["imagePath"] = QStringLiteral("missing.png");
    root["imageData"] = QJsonValue::Null;
    root["imageWidth"] = 12;
    root["imageHeight"] = 8;
    root["shapes"] = QJsonArray();

    const QString jsonPath = dir.filePath(QStringLiteral("missing-image.json"));
    QFile input(jsonPath);
    QVERIFY(input.open(QIODevice::WriteOnly | QIODevice::Text));
    input.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    input.close();

    AnnotationDocument loaded;
    QVERIFY(!AnnotationIO::loadLabelMe(jsonPath, &loaded));
}

void CoreTests::labelMeRejectsNonObjectShapeFlags() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(writeSolidImage(dir.filePath(QStringLiteral("image.jpg")), QSize(32, 24)));

    QJsonObject shape;
    shape["label"] = QStringLiteral("defect");
    shape["points"] = QJsonArray{QJsonArray{2, 3}, QJsonArray{12, 13}};
    shape["shape_type"] = QStringLiteral("rectangle");
    shape["flags"] = QStringLiteral("not-an-object");

    QJsonObject root;
    root["version"] = QStringLiteral("labelme");
    root["flags"] = QJsonObject();
    root["imagePath"] = QStringLiteral("image.jpg");
    root["imageData"] = QJsonValue::Null;
    root["imageWidth"] = 32;
    root["imageHeight"] = 24;
    root["shapes"] = QJsonArray{shape};

    const QString jsonPath = dir.filePath(QStringLiteral("invalid-shape-flags-type.json"));
    QFile input(jsonPath);
    QVERIFY(input.open(QIODevice::WriteOnly | QIODevice::Text));
    input.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    input.close();

    AnnotationDocument loaded;
    QVERIFY(!AnnotationIO::loadLabelMe(jsonPath, &loaded));
}

void CoreTests::labelMeRejectsNonBoolShapeFlags() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(writeSolidImage(dir.filePath(QStringLiteral("image.jpg")), QSize(32, 24)));

    QJsonObject shape;
    shape["label"] = QStringLiteral("defect");
    shape["points"] = QJsonArray{QJsonArray{2, 3}, QJsonArray{12, 13}};
    shape["shape_type"] = QStringLiteral("rectangle");
    shape["flags"] = QJsonObject{{QStringLiteral("difficult"), 1}};

    QJsonObject root;
    root["version"] = QStringLiteral("labelme");
    root["flags"] = QJsonObject();
    root["imagePath"] = QStringLiteral("image.jpg");
    root["imageData"] = QJsonValue::Null;
    root["imageWidth"] = 32;
    root["imageHeight"] = 24;
    root["shapes"] = QJsonArray{shape};

    const QString jsonPath = dir.filePath(QStringLiteral("invalid-shape-flag-value.json"));
    QFile input(jsonPath);
    QVERIFY(input.open(QIODevice::WriteOnly | QIODevice::Text));
    input.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    input.close();

    AnnotationDocument loaded;
    QVERIFY(!AnnotationIO::loadLabelMe(jsonPath, &loaded));
}

void CoreTests::labelMeRejectsNonStringShapeDescription() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QJsonObject shape;
    shape["label"] = QStringLiteral("defect");
    shape["points"] = QJsonArray{QJsonArray{2, 3}, QJsonArray{12, 13}};
    shape["shape_type"] = QStringLiteral("rectangle");
    shape["description"] = 42;

    const QString path = writeMinimalLabelMeFixture(dir, shape, QStringLiteral("invalid-description"));
    QVERIFY(!path.isEmpty());
    AnnotationDocument loaded;
    QVERIFY(!AnnotationIO::loadLabelMe(path, &loaded));
}

void CoreTests::labelMeRejectsNonIntegerGroupId() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QJsonObject shape;
    shape["label"] = QStringLiteral("defect");
    shape["points"] = QJsonArray{QJsonArray{2, 3}, QJsonArray{12, 13}};
    shape["shape_type"] = QStringLiteral("rectangle");
    shape["group_id"] = 1.5;

    const QString path = writeMinimalLabelMeFixture(dir, shape, QStringLiteral("invalid-group-id"));
    QVERIFY(!path.isEmpty());
    AnnotationDocument loaded;
    QVERIFY(!AnnotationIO::loadLabelMe(path, &loaded));
}

void CoreTests::labelMeRejectsNonStringShapeMask() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(writeSolidImage(dir.filePath(QStringLiteral("image.jpg")), QSize(32, 24)));

    QJsonObject shape;
    shape["label"] = QStringLiteral("mask");
    shape["points"] = QJsonArray{QJsonArray{2, 3}, QJsonArray{12, 13}};
    shape["shape_type"] = QStringLiteral("mask");
    shape["flags"] = QJsonObject();
    shape["mask"] = 42;

    QJsonObject root;
    root["version"] = QStringLiteral("labelme");
    root["flags"] = QJsonObject();
    root["imagePath"] = QStringLiteral("image.jpg");
    root["imageData"] = QJsonValue::Null;
    root["imageWidth"] = 32;
    root["imageHeight"] = 24;
    root["shapes"] = QJsonArray{shape};

    const QString jsonPath = dir.filePath(QStringLiteral("invalid-shape-mask-type.json"));
    QFile input(jsonPath);
    QVERIFY(input.open(QIODevice::WriteOnly | QIODevice::Text));
    input.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    input.close();

    AnnotationDocument loaded;
    QVERIFY(!AnnotationIO::loadLabelMe(jsonPath, &loaded));
}

void CoreTests::labelMeRejectsInvalidShapeMaskPng() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(writeSolidImage(dir.filePath(QStringLiteral("image.jpg")), QSize(32, 24)));

    QJsonObject shape;
    shape["label"] = QStringLiteral("mask");
    shape["points"] = QJsonArray{QJsonArray{2, 3}, QJsonArray{12, 13}};
    shape["shape_type"] = QStringLiteral("mask");
    shape["flags"] = QJsonObject();
    shape["mask"] = QStringLiteral("not-a-png");

    QJsonObject root;
    root["version"] = QStringLiteral("labelme");
    root["flags"] = QJsonObject();
    root["imagePath"] = QStringLiteral("image.jpg");
    root["imageData"] = QJsonValue::Null;
    root["imageWidth"] = 32;
    root["imageHeight"] = 24;
    root["shapes"] = QJsonArray{shape};

    const QString jsonPath = dir.filePath(QStringLiteral("invalid-shape-mask-png.json"));
    QFile input(jsonPath);
    QVERIFY(input.open(QIODevice::WriteOnly | QIODevice::Text));
    input.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    input.close();

    AnnotationDocument loaded;
    QVERIFY(!AnnotationIO::loadLabelMe(jsonPath, &loaded));
}

void CoreTests::labelMeRejectsShapeWithoutLabel() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QJsonObject shape;
    shape["points"] = QJsonArray{QJsonArray{2, 3}, QJsonArray{12, 13}};
    shape["shape_type"] = QStringLiteral("rectangle");
    shape["flags"] = QJsonObject();
    const QString jsonPath = writeMinimalLabelMeFixture(dir, shape, QStringLiteral("missing-label"));
    QVERIFY(!jsonPath.isEmpty());

    AnnotationDocument loaded;
    QVERIFY(!AnnotationIO::loadLabelMe(jsonPath, &loaded));
}

void CoreTests::labelMeRejectsMalformedShapePoints() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QJsonObject shape;
    shape["label"] = QStringLiteral("defect");
    shape["points"] = QJsonArray{QJsonArray{2}};
    shape["shape_type"] = QStringLiteral("rectangle");
    shape["flags"] = QJsonObject();
    const QString jsonPath = writeMinimalLabelMeFixture(dir, shape, QStringLiteral("malformed-points"));
    QVERIFY(!jsonPath.isEmpty());

    AnnotationDocument loaded;
    QVERIFY(!AnnotationIO::loadLabelMe(jsonPath, &loaded));
}

void CoreTests::labelMeRejectsShapeWithoutShapeType() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QJsonObject shape;
    shape["label"] = QStringLiteral("defect");
    shape["points"] = QJsonArray{QJsonArray{2, 3}, QJsonArray{12, 13}};
    shape["flags"] = QJsonObject();
    const QString jsonPath = writeMinimalLabelMeFixture(dir, shape, QStringLiteral("missing-shape-type"));
    QVERIFY(!jsonPath.isEmpty());

    AnnotationDocument loaded;
    QVERIFY(!AnnotationIO::loadLabelMe(jsonPath, &loaded));
}

void CoreTests::labelMePreservesUnknownShapeType() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QJsonObject shape;
    shape["label"] = QStringLiteral("plugin-shape");
    shape["points"] = QJsonArray{QJsonArray{2, 3}, QJsonArray{12, 13}, QJsonArray{6, 20}};
    shape["shape_type"] = QStringLiteral("triangle");
    shape["flags"] = QJsonObject();
    const QString jsonPath = writeMinimalLabelMeFixture(dir, shape, QStringLiteral("unknown-shape-type"));
    QVERIFY(!jsonPath.isEmpty());

    AnnotationDocument loaded;
    QVERIFY(AnnotationIO::loadLabelMe(jsonPath, &loaded));
    QCOMPARE(loaded.shapes.size(), 1);
    QCOMPARE(loaded.shapes.first().shapeType, QStringLiteral("triangle"));
    QCOMPARE(loaded.shapes.first().points,
             QVector<QPointF>({QPointF(2, 3), QPointF(12, 13), QPointF(6, 20)}));
    QVERIFY(loaded.shapes.first().closed);
    QVERIFY(loaded.shapes.first().contains(QPointF(6, 10)));
    QVERIFY(!loaded.shapes.first().contains(QPointF(11, 19)));

    const QString outputPath = dir.filePath(QStringLiteral("unknown-shape-type-out.json"));
    QVERIFY(AnnotationIO::saveLabelMe(outputPath, loaded));
    QFile output(outputPath);
    QVERIFY(output.open(QIODevice::ReadOnly | QIODevice::Text));
    const QJsonObject writtenShape = QJsonDocument::fromJson(output.readAll()).object()
                                         .value(QStringLiteral("shapes")).toArray().first().toObject();
    QCOMPARE(writtenShape.value(QStringLiteral("shape_type")).toString(), QStringLiteral("triangle"));
    QCOMPARE(writtenShape.value(QStringLiteral("points")).toArray(), shape.value(QStringLiteral("points")).toArray());
}

void CoreTests::labelMeRejectsMissingImagePath() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QJsonObject root;
    root["version"] = QStringLiteral("labelme");
    root["flags"] = QJsonObject();
    root["imageData"] = tinyImageBase64();
    root["imageWidth"] = 8;
    root["imageHeight"] = 6;
    root["shapes"] = QJsonArray();
    const QString jsonPath = dir.filePath(QStringLiteral("missing-image-path.json"));
    QFile input(jsonPath);
    QVERIFY(input.open(QIODevice::WriteOnly | QIODevice::Text));
    input.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    input.close();

    AnnotationDocument loaded;
    QVERIFY(!AnnotationIO::loadLabelMe(jsonPath, &loaded));
}

void CoreTests::labelMeRejectsMissingImageData() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(writeSolidImage(dir.filePath(QStringLiteral("image.jpg")), QSize(32, 24)));

    QJsonObject root;
    root["version"] = QStringLiteral("labelme");
    root["flags"] = QJsonObject();
    root["imagePath"] = QStringLiteral("image.jpg");
    root["imageWidth"] = 32;
    root["imageHeight"] = 24;
    root["shapes"] = QJsonArray();
    const QString jsonPath = dir.filePath(QStringLiteral("missing-image-data.json"));
    QFile input(jsonPath);
    QVERIFY(input.open(QIODevice::WriteOnly | QIODevice::Text));
    input.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    input.close();

    AnnotationDocument loaded;
    QVERIFY(!AnnotationIO::loadLabelMe(jsonPath, &loaded));
}

void CoreTests::labelMeRejectsMissingShapesArray() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(writeSolidImage(dir.filePath(QStringLiteral("image.jpg")), QSize(32, 24)));

    QJsonObject root;
    root["version"] = QStringLiteral("labelme");
    root["flags"] = QJsonObject();
    root["imagePath"] = QStringLiteral("image.jpg");
    root["imageData"] = QJsonValue::Null;
    root["imageWidth"] = 32;
    root["imageHeight"] = 24;
    const QString jsonPath = dir.filePath(QStringLiteral("missing-shapes.json"));
    QFile input(jsonPath);
    QVERIFY(input.open(QIODevice::WriteOnly | QIODevice::Text));
    input.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    input.close();

    AnnotationDocument loaded;
    QVERIFY(!AnnotationIO::loadLabelMe(jsonPath, &loaded));
}

void CoreTests::labelMeRejectsNonStringImageData() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(writeSolidImage(dir.filePath(QStringLiteral("image.jpg")), QSize(32, 24)));

    QJsonObject root;
    root["version"] = QStringLiteral("labelme");
    root["flags"] = QJsonObject();
    root["imagePath"] = QStringLiteral("image.jpg");
    root["imageData"] = 42;
    root["imageWidth"] = 32;
    root["imageHeight"] = 24;
    root["shapes"] = QJsonArray();
    const QString jsonPath = dir.filePath(QStringLiteral("non-string-image-data.json"));
    QFile input(jsonPath);
    QVERIFY(input.open(QIODevice::WriteOnly | QIODevice::Text));
    input.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    input.close();

    AnnotationDocument loaded;
    QVERIFY(!AnnotationIO::loadLabelMe(jsonPath, &loaded));
}

void CoreTests::labelMeRejectsNonObjectTopLevelFlags() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(writeSolidImage(dir.filePath(QStringLiteral("image.jpg")), QSize(32, 24)));

    QJsonObject root;
    root[QStringLiteral("version")] = QStringLiteral("labelme");
    root[QStringLiteral("flags")] = QJsonArray{true};
    root[QStringLiteral("imagePath")] = QStringLiteral("image.jpg");
    root[QStringLiteral("imageData")] = QJsonValue::Null;
    root[QStringLiteral("imageWidth")] = 32;
    root[QStringLiteral("imageHeight")] = 24;
    root[QStringLiteral("shapes")] = QJsonArray();

    const QString jsonPath = dir.filePath(QStringLiteral("non-object-top-level-flags.json"));
    QFile input(jsonPath);
    QVERIFY(input.open(QIODevice::WriteOnly | QIODevice::Text));
    input.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    input.close();

    AnnotationDocument loaded;
    QVERIFY(!AnnotationIO::loadLabelMe(jsonPath, &loaded));
}

void CoreTests::labelMeRejectsNonBoolTopLevelFlags() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(writeSolidImage(dir.filePath(QStringLiteral("image.jpg")), QSize(32, 24)));

    QJsonObject root;
    root[QStringLiteral("version")] = QStringLiteral("labelme");
    root[QStringLiteral("flags")] = QJsonObject{{QStringLiteral("verified"), 1}};
    root[QStringLiteral("imagePath")] = QStringLiteral("image.jpg");
    root[QStringLiteral("imageData")] = QJsonValue::Null;
    root[QStringLiteral("imageWidth")] = 32;
    root[QStringLiteral("imageHeight")] = 24;
    root[QStringLiteral("shapes")] = QJsonArray();

    const QString jsonPath = dir.filePath(QStringLiteral("non-bool-top-level-flags.json"));
    QFile input(jsonPath);
    QVERIFY(input.open(QIODevice::WriteOnly | QIODevice::Text));
    input.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    input.close();

    AnnotationDocument loaded;
    QVERIFY(!AnnotationIO::loadLabelMe(jsonPath, &loaded));
}

void CoreTests::labelMePreservesTopLevelFlags() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QJsonObject flags;
    flags["verified"] = true;
    flags["needs_review"] = true;
    flags["accepted"] = false;

    QJsonObject root;
    root["version"] = QStringLiteral("labelme");
    root["flags"] = flags;
    root["imagePath"] = QStringLiteral("flagged.jpg");
    root["imageData"] = QJsonValue::Null;
    root["imageWidth"] = 32;
    root["imageHeight"] = 24;
    QVERIFY(writeSolidImage(dir.filePath(QStringLiteral("flagged.jpg")), QSize(32, 24)));
    root["shapes"] = QJsonArray();

    const QString jsonPath = dir.filePath(QStringLiteral("flagged.json"));
    QFile input(jsonPath);
    QVERIFY(input.open(QIODevice::WriteOnly | QIODevice::Text));
    input.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    input.close();

    AnnotationDocument loaded;
    QVERIFY(AnnotationIO::loadLabelMe(jsonPath, &loaded));
    QVERIFY(loaded.verified);

    const QString outputPath = dir.filePath(QStringLiteral("flagged-out.json"));
    QVERIFY(AnnotationIO::saveLabelMe(outputPath, loaded));
    QFile output(outputPath);
    QVERIFY(output.open(QIODevice::ReadOnly | QIODevice::Text));
    const QJsonObject writtenFlags = QJsonDocument::fromJson(output.readAll()).object()
                                       .value(QStringLiteral("flags")).toObject();
    QCOMPARE(writtenFlags.value(QStringLiteral("verified")).toBool(), true);
    QCOMPARE(writtenFlags.value(QStringLiteral("needs_review")).toBool(), true);
    QCOMPARE(writtenFlags.value(QStringLiteral("accepted")).toBool(), false);
}

void CoreTests::labelMePreservesTopLevelOtherData() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QJsonObject root;
    root["version"] = QStringLiteral("labelme");
    root["flags"] = QJsonObject();
    root["imagePath"] = QStringLiteral("other.png");
    root["imageData"] = QJsonValue::Null;
    root["imageWidth"] = 20;
    root["imageHeight"] = 10;
    root["shapes"] = QJsonArray();
    root["metadata"] = QJsonObject{{QStringLiteral("source"), QStringLiteral("camera-a")},
                                    {QStringLiteral("reviewed"), false}};
    root["confidence"] = 0.75;
    QVERIFY(writeSolidImage(dir.filePath(QStringLiteral("other.png")), QSize(20, 10)));

    const QString jsonPath = dir.filePath(QStringLiteral("other.json"));
    QFile input(jsonPath);
    QVERIFY(input.open(QIODevice::WriteOnly | QIODevice::Text));
    input.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    input.close();

    AnnotationDocument loaded;
    QVERIFY(AnnotationIO::loadLabelMe(jsonPath, &loaded));

    const QString outputPath = dir.filePath(QStringLiteral("other-out.json"));
    QVERIFY(AnnotationIO::saveLabelMe(outputPath, loaded));
    QFile output(outputPath);
    QVERIFY(output.open(QIODevice::ReadOnly | QIODevice::Text));
    const QJsonObject written = QJsonDocument::fromJson(output.readAll()).object();
    compareJsonValue(written.value(QStringLiteral("metadata")), root.value(QStringLiteral("metadata")), QStringLiteral("metadata"));
    compareJsonValue(written.value(QStringLiteral("confidence")), root.value(QStringLiteral("confidence")), QStringLiteral("confidence"));
}

void CoreTests::labelMeRejectsReservedTopLevelOtherData() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AnnotationDocument document;
    document.imagePath = QStringLiteral("image.jpg");
    document.imageSize = QSize(32, 24);
    document.labelMeOtherData.insert(QStringLiteral("imagePath"), QStringLiteral("overwritten.jpg"));
    document.labelMeOtherData.insert(QStringLiteral("customField"), QStringLiteral("kept"));

    QVERIFY(!AnnotationIO::saveLabelMe(dir.filePath(QStringLiteral("reserved.json")), document));
}

void CoreTests::labelMeRoundTripsReferencePrimitiveExampleVersion() {
    const QString root = repoRootPath();
    QVERIFY2(!root.isEmpty(), "refs/labelme examples not found");
    const QString jsonPath = QDir(root).filePath(QStringLiteral("refs/labelme/examples/primitives/primitives.json"));

    QFile input(jsonPath);
    QVERIFY(input.open(QIODevice::ReadOnly | QIODevice::Text));
    const QJsonObject source = QJsonDocument::fromJson(input.readAll()).object();
    const QString sourceVersion = source.value(QStringLiteral("version")).toString();
    QCOMPARE(sourceVersion, QStringLiteral("5.7.0"));

    AnnotationDocument loaded;
    QVERIFY(AnnotationIO::loadLabelMe(jsonPath, &loaded));
    QCOMPARE(loaded.labelMeVersion, sourceVersion);
    QCOMPARE(loaded.shapes.size(), source.value(QStringLiteral("shapes")).toArray().size());
    QSet<QString> loadedTypes;
    for (const Shape &shape : loaded.shapes) {
        loadedTypes.insert(shape.shapeType);
    }
    QVERIFY(loadedTypes.contains(QStringLiteral("rectangle")));
    QVERIFY(loadedTypes.contains(QStringLiteral("polygon")));
    QVERIFY(loadedTypes.contains(QStringLiteral("circle")));
    QVERIFY(loadedTypes.contains(QStringLiteral("line")));
    QVERIFY(loadedTypes.contains(QStringLiteral("point")));
    QVERIFY(loadedTypes.contains(QStringLiteral("linestrip")));
    QVERIFY(loadedTypes.contains(QStringLiteral("mask")));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString outputPath = dir.filePath(QStringLiteral("primitives-out.json"));
    QVERIFY(AnnotationIO::saveLabelMe(outputPath, loaded));
    QFile output(outputPath);
    QVERIFY(output.open(QIODevice::ReadOnly | QIODevice::Text));
    const QJsonObject written = QJsonDocument::fromJson(output.readAll()).object();
    QCOMPARE(written.value(QStringLiteral("version")).toString(), sourceVersion);
}

void CoreTests::labelMeNormalizesNullShapeDescriptions() {
    const QString root = repoRootPath();
    QVERIFY2(!root.isEmpty(), "refs/labelme examples not found");
    const QString jsonPath = QDir(root).filePath(QStringLiteral("refs/labelme/examples/primitives/primitives.json"));

    QFile input(jsonPath);
    QVERIFY(input.open(QIODevice::ReadOnly | QIODevice::Text));
    const QJsonObject source = QJsonDocument::fromJson(input.readAll()).object();
    QVERIFY(source.value(QStringLiteral("shapes")).toArray().first().toObject()
                .value(QStringLiteral("description")).isNull());

    AnnotationDocument loaded;
    QVERIFY(AnnotationIO::loadLabelMe(jsonPath, &loaded));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString outputPath = dir.filePath(QStringLiteral("primitives-out.json"));
    QVERIFY(AnnotationIO::saveLabelMe(outputPath, loaded));
    QFile output(outputPath);
    QVERIFY(output.open(QIODevice::ReadOnly | QIODevice::Text));
    const QJsonObject writtenShape = QJsonDocument::fromJson(output.readAll()).object()
                                         .value(QStringLiteral("shapes")).toArray().first().toObject();
    QCOMPARE(writtenShape.value(QStringLiteral("description")).toString(), QString());

    AnnotationDocument newDocument;
    newDocument.imagePath = QStringLiteral("new.jpg");
    newDocument.imageSize = QSize(40, 30);
    Shape newShape = Shape::fromRect(QStringLiteral("new"), QRectF(2, 3, 10, 8), false);
    newShape.description = QStringLiteral("stale internal value");
    newShape.descriptionIsNull = true;
    newDocument.shapes.push_back(newShape);

    const QString newOutputPath = dir.filePath(QStringLiteral("new-out.json"));
    QVERIFY(AnnotationIO::saveLabelMe(newOutputPath, newDocument));
    QFile newOutput(newOutputPath);
    QVERIFY(newOutput.open(QIODevice::ReadOnly | QIODevice::Text));
    const QJsonObject newWrittenShape = QJsonDocument::fromJson(newOutput.readAll()).object()
                                            .value(QStringLiteral("shapes"))
                                            .toArray()
                                            .first()
                                            .toObject();
    QVERIFY(newWrittenShape.value(QStringLiteral("description")).isString());
    QCOMPARE(newWrittenShape.value(QStringLiteral("description")).toString(), QString());
}

void CoreTests::labelMePreservesRelativeImagePath() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QJsonObject root;
    root["version"] = QStringLiteral("5.7.0");
    root["flags"] = QJsonObject();
    root["shapes"] = QJsonArray();
    root["imagePath"] = QStringLiteral("images/nested.jpg");
    root["imageData"] = QJsonValue::Null;
    root["imageHeight"] = 24;
    root["imageWidth"] = 32;
    QVERIFY(writeSolidImage(dir.filePath(QStringLiteral("images/nested.jpg")), QSize(32, 24)));

    const QString jsonPath = dir.filePath(QStringLiteral("nested.json"));
    QFile input(jsonPath);
    QVERIFY(input.open(QIODevice::WriteOnly | QIODevice::Text));
    input.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    input.close();

    AnnotationDocument loaded;
    QVERIFY(AnnotationIO::loadLabelMe(jsonPath, &loaded));
    QCOMPARE(loaded.imagePath, QStringLiteral("images/nested.jpg"));

    const QString outputPath = dir.filePath(QStringLiteral("nested-out.json"));
    QVERIFY(AnnotationIO::saveLabelMe(outputPath, loaded));
    QFile output(outputPath);
    QVERIFY(output.open(QIODevice::ReadOnly | QIODevice::Text));
    const QJsonObject written = QJsonDocument::fromJson(output.readAll()).object();
    QCOMPARE(written.value(QStringLiteral("imagePath")).toString(), QStringLiteral("images/nested.jpg"));
}

void CoreTests::labelMeNormalizesWindowsImagePath() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("images")));
    QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("annotations")));

    const QString imagePath = dir.filePath(QStringLiteral("images/source.jpg"));
    const QString annotationPath = dir.filePath(QStringLiteral("annotations/source.json"));
    QVERIFY(writeSolidImage(imagePath, QSize(80, 60)));

    QJsonObject root;
    root.insert(QStringLiteral("version"), QStringLiteral("5.0"));
    root.insert(QStringLiteral("flags"), QJsonObject());
    root.insert(QStringLiteral("shapes"), QJsonArray());
    root.insert(QStringLiteral("imagePath"), QStringLiteral("..\\images\\source.jpg"));
    root.insert(QStringLiteral("imageData"), QJsonValue::Null);
    root.insert(QStringLiteral("imageHeight"), 60);
    root.insert(QStringLiteral("imageWidth"), 80);

    QFile file(annotationPath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();

    AnnotationDocument document;
    QString error;
    QVERIFY2(AnnotationIO::loadLabelMe(annotationPath, &document, &error), qPrintable(error));
    QCOMPARE(document.imagePath, QStringLiteral("../images/source.jpg"));
    QCOMPARE(document.imageSize, QSize(80, 60));
}

void CoreTests::labelMeWritesRelativeImagePathFromOutputDirectory() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("images")));
    QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("annotations")));

    const QString imagePath = dir.filePath(QStringLiteral("images/source.jpg"));
    QVERIFY(writeSolidImage(imagePath, QSize(32, 24)));

    AnnotationDocument document;
    document.imagePath = imagePath;
    document.imageSize = QSize(32, 24);
    const QString annotationPath = dir.filePath(QStringLiteral("annotations/source.json"));
    QVERIFY(AnnotationIO::saveLabelMe(annotationPath, document));

    QFile output(annotationPath);
    QVERIFY(output.open(QIODevice::ReadOnly | QIODevice::Text));
    const QJsonObject written = QJsonDocument::fromJson(output.readAll()).object();
    QCOMPARE(written.value(QStringLiteral("imagePath")).toString(), QStringLiteral("../images/source.jpg"));
}

void CoreTests::labelMeRoundTripsAllReferenceExamples() {
    const QString root = repoRootPath();
    QVERIFY2(!root.isEmpty(), "refs/labelme examples not found");
    const QDir examplesDir(QDir(root).filePath(QStringLiteral("refs/labelme/examples")));

    QStringList labelMeFiles;
    QDirIterator iterator(examplesDir.absolutePath(), QStringList{QStringLiteral("*.json")}, QDir::Files, QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        const QString path = iterator.next();
        QFile input(path);
        QVERIFY(input.open(QIODevice::ReadOnly | QIODevice::Text));
        const QJsonObject object = QJsonDocument::fromJson(input.readAll()).object();
        if (object.value(QStringLiteral("shapes")).isArray() && object.contains(QStringLiteral("imagePath"))) {
            labelMeFiles.append(path);
        }
    }
    labelMeFiles.sort();
    QCOMPARE(labelMeFiles.size(), 18);

    QTemporaryDir outputDir;
    QVERIFY(outputDir.isValid());

    for (const QString &path : labelMeFiles) {
        QFile input(path);
        QVERIFY(input.open(QIODevice::ReadOnly | QIODevice::Text));
        const QJsonObject source = QJsonDocument::fromJson(input.readAll()).object();

        AnnotationDocument loaded;
        QVERIFY2(AnnotationIO::loadLabelMe(path, &loaded), qPrintable(path));
        const QString outputPath = outputDir.filePath(QFileInfo(path).completeBaseName() + QStringLiteral("-out.json"));
        QVERIFY2(AnnotationIO::saveLabelMe(outputPath, loaded), qPrintable(path));

        QFile output(outputPath);
        QVERIFY(output.open(QIODevice::ReadOnly | QIODevice::Text));
        const QJsonObject written = QJsonDocument::fromJson(output.readAll()).object();
        const QString context = examplesDir.relativeFilePath(path);

        for (const QString &field : {QStringLiteral("version"),
                                     QStringLiteral("flags"),
                                     QStringLiteral("imagePath"),
                                     QStringLiteral("imageData"),
                                     QStringLiteral("imageHeight"),
                                     QStringLiteral("imageWidth")}) {
            compareJsonValue(written.value(field), source.value(field), context + QStringLiteral(".") + field);
        }

        const QJsonArray sourceShapes = source.value(QStringLiteral("shapes")).toArray();
        const QJsonArray writtenShapes = written.value(QStringLiteral("shapes")).toArray();
        QCOMPARE(writtenShapes.size(), sourceShapes.size());
        for (int i = 0; i < sourceShapes.size(); ++i) {
            const QJsonObject sourceShape = sourceShapes.at(i).toObject();
            const QJsonObject writtenShape = writtenShapes.at(i).toObject();
            const QString shapeContext = QStringLiteral("%1.shapes[%2]").arg(context).arg(i);
            for (const QString &field : {QStringLiteral("label"),
                                         QStringLiteral("points"),
                                         QStringLiteral("group_id"),
                                         QStringLiteral("shape_type"),
                                         QStringLiteral("flags"),
                                         QStringLiteral("description"),
                                         QStringLiteral("mask")}) {
                compareShapeField(writtenShape, sourceShape, field, shapeContext);
            }
        }
    }
}

void CoreTests::labelNavigationSelectsTopWhenEmptyAndWraps() {
    LabelListModel labels;
    labels.add(Shape::fromRect("first", QRectF(0, 0, 10, 10), false));
    labels.add(Shape::fromRect("second", QRectF(20, 20, 10, 10), false));
    labels.clearSelection();

    labels.selectAdjacent(1);
    QCOMPARE(labels.currentIndex(), 0);
    labels.selectAdjacent(1);
    QCOMPARE(labels.currentIndex(), 1);
    labels.selectAdjacent(1);
    QCOMPARE(labels.currentIndex(), 0);
    labels.selectAdjacent(-1);
    QCOMPARE(labels.currentIndex(), 1);

    labels.removeCurrent();
    QCOMPARE(labels.count(), 1);
    QCOMPARE(labels.currentIndex(), 0);
    QCOMPARE(labels.current().label, QString("first"));
}

void CoreTests::shapeRectUsesLabelMeDiagonalPoints() {
    const Shape shape = Shape::fromRect(QStringLiteral("box"), QRectF(10, 20, 30, 40), false);

    QCOMPARE(shape.shapeType, QStringLiteral("rectangle"));
    QCOMPARE(shape.points, QVector<QPointF>({QPointF(10, 20), QPointF(40, 60)}));
    QCOMPARE(shape.pointLabels, QVector<int>({1, 1}));
}

void CoreTests::shapeSupportsVertexEditingAndCopy() {
    Shape shape = Shape::fromRect("box", QRectF(10, 20, 30, 40), false);

    QCOMPARE(shape.nearestVertex(QPointF(10.5, 20.5), 2.0), 0);
    QCOMPARE(shape.nearestVertex(QPointF(80, 80), 2.0), -1);

    shape.moveVertexBy(0, QPointF(-5, -10));
    QCOMPARE(shape.boundingRect().toRect(), QRect(5, 10, 35, 50));
    QCOMPARE(shape.points[1].y(), 60.0);
    QCOMPARE(shape.points[0].x(), 5.0);

    Shape copy = shape.copy();
    copy.moveBy(QPointF(3, 4));
    QCOMPARE(shape.boundingRect().topLeft(), QPointF(5, 10));
    QCOMPARE(copy.boundingRect().topLeft(), QPointF(8, 14));

    shape.setVisible(false);
    QVERIFY(!shape.visible);
    QVERIFY(!shape.contains(QPointF(15, 15)));
}

void CoreTests::shapeNearestVertexKeepsFirstEqualDistanceMatch() {
    const Shape shape = Shape::fromPoints(QStringLiteral("points"),
                                          QStringLiteral("points"),
                                          {QPointF(10, 10), QPointF(14, 10)},
                                          false);

    // LabelMe uses numpy.argmin, so a midpoint hit resolves to the first
    // vertex rather than changing selection to the later equal-distance one.
    QCOMPARE(shape.nearestVertex(QPointF(12, 10), 2.0), 0);
}

void CoreTests::shapeRotatesOnlyOrientedRectangles() {
    Shape oriented = Shape::fromPoints(QStringLiteral("box"),
                                       QStringLiteral("oriented_rectangle"),
                                       {QPointF(0, 0), QPointF(10, 0), QPointF(10, 4), QPointF(0, 4)},
                                       false);
    QVERIFY(oriented.rotate(QPointF(0, 0), 1.5707963267948966));
    QCOMPARE(oriented.points[0], QPointF(0, 0));
    QCOMPARE(oriented.points[1].x(), 0.0);
    QCOMPARE(oriented.points[1].y(), 10.0);
    QCOMPARE(oriented.points[2].x(), -4.0);
    QCOMPARE(oriented.points[2].y(), 10.0);

    Shape rectangle = Shape::fromRect(QStringLiteral("box"), QRectF(0, 0, 10, 4), false);
    QVERIFY(!rectangle.rotate(QPointF(0, 0), 1.5707963267948966));
}

void CoreTests::shapeMaskDoesNotExposeVertices() {
    Shape mask = Shape::fromPoints(QStringLiteral("mask"),
                                   QStringLiteral("mask"),
                                   {QPointF(0, 0), QPointF(3, 3)},
                                   false);
    QVERIFY(mask.nearestVertex(QPointF(0, 0), 10.0) < 0);
    QVERIFY(mask.nearestVertex(QPointF(3, 3), 10.0) < 0);
}

void CoreTests::shapeHitTestingMatchesLabelMeTolerance() {
    const Shape line = Shape::fromPoints(QStringLiteral("line"),
                                         QStringLiteral("line"),
                                         {QPointF(10, 10), QPointF(90, 10)},
                                         false);
    QVERIFY(line.hitTest(QPointF(50, 19), 1.0, 10.0, 8));
    QVERIFY(!line.hitTest(QPointF(50, 19), 1.0, 8.0, 8));

    const Shape points = Shape::fromPoints(QStringLiteral("landmarks"),
                                           QStringLiteral("points"),
                                           {QPointF(20, 20), QPointF(60, 60)},
                                           false);
    QVERIFY(!points.hitTest(QPointF(20, 20), 1.0, 10.0, 8));

    const Shape point = Shape::fromPoints(QStringLiteral("point"),
                                          QStringLiteral("point"),
                                          {QPointF(20, 20)},
                                          false);
    QVERIFY(point.hitTest(QPointF(24, 20), 1.0, 10.0, 8));
    QVERIFY(!point.hitTest(QPointF(25, 20), 1.0, 10.0, 8));
}

void CoreTests::linestripHitTestingIncludesLabelMeClosingEdge() {
    const Shape linestrip = Shape::fromPoints(
        QStringLiteral("path"),
        QStringLiteral("linestrip"),
        {QPointF(10, 10), QPointF(30, 10), QPointF(30, 30)},
        false);

    // LabelMe rolls the point list for edge hit testing, so the closing
    // segment from the last vertex back to the first is also selectable.
    QVERIFY(linestrip.hitTest(QPointF(20, 20), 1.0, 2.0, 8));
}

void CoreTests::linestripContainsIncludesLabelMeClosingEdge() {
    const Shape linestrip = Shape::fromPoints(
        QStringLiteral("path"),
        QStringLiteral("linestrip"),
        {QPointF(10, 10), QPointF(30, 10), QPointF(30, 30)},
        false);

    // Keep the general Shape containment helper consistent with the canvas
    // hit test: the open rendering still exposes the closing edge to picks.
    QVERIFY(linestrip.contains(QPointF(20, 20)));
}

void CoreTests::maskWithoutBitmapUsesLabelMeBoundingBoxHitTest() {
    const Shape mask = Shape::fromPoints(QStringLiteral("mask"),
                                         QStringLiteral("mask"),
                                         {QPointF(10, 12), QPointF(30, 28)},
                                         false);
    QVERIFY(mask.hitTest(QPointF(20, 20), 1.0, 10.0, 8));
    QVERIFY(!mask.hitTest(QPointF(31, 20), 1.0, 10.0, 8));
}

void CoreTests::shapePointHitTestingUsesScreenSpaceTolerance() {
    const Shape point = Shape::fromPoints(QStringLiteral("point"),
                                          QStringLiteral("point"),
                                          {QPointF(20, 20)},
                                          false);

    // pointSize is measured in screen pixels, so a 3-image-pixel offset is
    // outside an 8px marker when the image is zoomed to 200%.
    QVERIFY(!point.hitTest(QPointF(23, 20), 2.0, 10.0, 8));
    // At 50% zoom, a 7-image-pixel offset is still only 3.5 screen pixels.
    QVERIFY(point.hitTest(QPointF(27, 20), 0.5, 10.0, 8));
}

void CoreTests::shapePointLabelsFollowVertexEdits() {
    Shape shape = Shape::fromPolygon(QStringLiteral("polygon"),
                                     {QPointF(0, 0), QPointF(10, 0), QPointF(10, 10)},
                                     false);
    QCOMPARE(shape.pointLabels, QVector<int>({1, 1, 1}));

    shape.pointLabels = {1, 0, 1};
    QVERIFY(shape.insertPoint(1, QPointF(5, 0), 0));
    QCOMPARE(shape.pointLabels, QVector<int>({1, 0, 0, 1}));
    QVERIFY(shape.removePoint(1));
    QCOMPARE(shape.pointLabels, QVector<int>({1, 0, 1}));
}

void CoreTests::shapeToMaskMatchesLabelMePrimitiveSemantics() {
    const QSize imageSize(32, 32);
    const Shape rectangle = Shape::fromPoints(QStringLiteral("box"),
                                              QStringLiteral("rectangle"),
                                              {QPointF(20, 20), QPointF(5, 5)},
                                              false);
    const QImage rectangleMask = rectangle.toMask(imageSize);
    QCOMPARE(rectangleMask.size(), imageSize);
    QCOMPARE(rectangleMask.format(), QImage::Format_Grayscale8);
    QVERIFY(rectangleMask.pixelColor(5, 5).value() > 0);
    QVERIFY(rectangleMask.pixelColor(12, 12).value() > 0);
    QVERIFY(rectangleMask.pixelColor(21, 21).value() == 0);

    const Shape oriented = Shape::fromPoints(QStringLiteral("oriented"),
                                             QStringLiteral("oriented_rectangle"),
                                             {QPointF(2, 2), QPointF(18, 2), QPointF(18, 8), QPointF(2, 8)},
                                             false);
    const QImage orientedMask = oriented.toMask(imageSize);
    QVERIFY(orientedMask.pixelColor(10, 5).value() > 0);
    QVERIFY(orientedMask.pixelColor(10, 15).value() == 0);

    const Shape point = Shape::fromPoints(QStringLiteral("point"),
                                          QStringLiteral("point"),
                                          {QPointF(24, 24)},
                                          false);
    const QImage pointMask = point.toMask(imageSize, 10, 3);
    QVERIFY(pointMask.pixelColor(24, 24).value() > 0);
    QVERIFY(pointMask.pixelColor(28, 24).value() == 0);
}

void CoreTests::shapeToMaskRejectsUnsupportedShapeTypes() {
    const Shape points = Shape::fromPoints(QStringLiteral("landmarks"),
                                           QStringLiteral("points"),
                                           {QPointF(1, 1), QPointF(2, 2)},
                                           false);
    QVERIFY(points.toMask(QSize(8, 8)).isNull());
}

void CoreTests::shapeMaskContainsOnlyNonZeroPixels() {
    QImage mask(4, 4, QImage::Format_Grayscale8);
    mask.fill(0);
    for (int y = 1; y <= 2; ++y) {
        for (int x = 1; x <= 2; ++x) {
            mask.setPixelColor(x, y, QColor(255, 255, 255));
        }
    }

    QByteArray pngData;
    QBuffer buffer(&pngData);
    QVERIFY(buffer.open(QIODevice::WriteOnly));
    QVERIFY(mask.save(&buffer, "PNG"));

    Shape shape = Shape::fromPoints(QStringLiteral("mask"),
                                    QStringLiteral("mask"),
                                    {QPointF(10, 20), QPointF(13, 23)},
                                    false);
    shape.maskData = QString::fromLatin1(pngData.toBase64());

    QVERIFY(shape.contains(QPointF(11, 21)));
    QVERIFY(shape.contains(QPointF(12, 22)));
    QVERIFY(!shape.contains(QPointF(10, 20)));
    QVERIFY(!shape.contains(QPointF(13, 23)));
    QVERIFY(!shape.contains(QPointF(9, 21)));
}

void CoreTests::shapeOverlapScoreCatchesContainedShapes() {
    const Shape outer = Shape::fromRect(QStringLiteral("outer"), QRectF(0, 0, 100, 100), false);
    const Shape inner = Shape::fromRect(QStringLiteral("inner"), QRectF(20, 20, 10, 10), false);
    const Shape separate = Shape::fromRect(QStringLiteral("separate"), QRectF(200, 200, 10, 10), false);

    QVERIFY(outer.overlapScore(inner) >= 0.99);
    QVERIFY(inner.overlapScore(outer) >= 0.99);
    QCOMPARE(outer.overlapScore(separate), 0.0);

    const Shape lowerLeft = Shape::fromPolygon(
        QStringLiteral("lower"),
        {QPointF(0, 0), QPointF(10, 0), QPointF(0, 10)},
        false);
    const Shape upperRight = Shape::fromPolygon(
        QStringLiteral("upper"),
        {QPointF(10, 10), QPointF(10, 0), QPointF(0, 10)},
        false);
    QVERIFY2(lowerLeft.overlapScore(upperRight) < 0.1,
             "opposite triangles sharing a bounding box must not overlap by bbox alone");

    QImage lowerMaskImage(11, 11, QImage::Format_Grayscale8);
    QImage upperMaskImage(11, 11, QImage::Format_Grayscale8);
    lowerMaskImage.fill(0);
    upperMaskImage.fill(0);
    for (int y = 0; y < 11; ++y) {
        for (int x = 0; x < 11; ++x) {
            if (x + y <= 10) {
                lowerMaskImage.setPixelColor(x, y, Qt::white);
            } else {
                upperMaskImage.setPixelColor(x, y, Qt::white);
            }
        }
    }
    QByteArray lowerMaskData;
    QByteArray upperMaskData;
    QBuffer lowerBuffer(&lowerMaskData);
    QBuffer upperBuffer(&upperMaskData);
    QVERIFY(lowerBuffer.open(QIODevice::WriteOnly));
    QVERIFY(upperBuffer.open(QIODevice::WriteOnly));
    QVERIFY(lowerMaskImage.save(&lowerBuffer, "PNG"));
    QVERIFY(upperMaskImage.save(&upperBuffer, "PNG"));
    Shape lowerMask = Shape::fromPoints(QStringLiteral("lower-mask"), QStringLiteral("mask"),
                                        {QPointF(0, 0), QPointF(10, 10)}, false);
    Shape upperMask = Shape::fromPoints(QStringLiteral("upper-mask"), QStringLiteral("mask"),
                                        {QPointF(0, 0), QPointF(10, 10)}, false);
    lowerMask.maskData = QString::fromLatin1(lowerMaskData.toBase64());
    upperMask.maskData = QString::fromLatin1(upperMaskData.toBase64());
    QVERIFY2(lowerMask.overlapScore(upperMask) < 0.1,
             "opposite masks sharing a bounding box must use mask pixels, not bbox IoU");
}

void CoreTests::shapeDefaultsUseLabelMeDraftFillColor() {
    const Shape shape = Shape::fromRect(QStringLiteral("object"), QRectF(1, 2, 10, 12), false);
    QCOMPARE(shape.fillColor, QColor(0, 0, 0, 64));
}

void CoreTests::shapeOverlapUsesLabelMeInclusivePixelMasks() {
    const Shape left = Shape::fromRect(QStringLiteral("left"), QRectF(0, 0, 10, 10), false);
    const Shape right = Shape::fromRect(QStringLiteral("right"), QRectF(5, 5, 10, 10), false);

    // LabelMe rasterizes [0, 10] and [5, 15] as inclusive 11x11 masks. The
    // 6x6 overlap therefore has containment 36 / 121, rather than the
    // continuous-area result 25 / 100.
    const double expectedContainment = 36.0 / 121.0;
    QVERIFY(qAbs(left.overlapScore(right) - expectedContainment) < 1e-6);
}

void CoreTests::shapeOverlapSkipsUnknownPluginTypesLikeLabelMe() {
    const Shape plugin = Shape::fromPoints(QStringLiteral("plugin"),
                                           QStringLiteral("custom_plugin_shape"),
                                           {QPointF(0, 0), QPointF(10, 0), QPointF(10, 10)},
                                           false);
    const Shape rectangle = Shape::fromRect(QStringLiteral("box"), QRectF(0, 0, 10, 10), false);
    QVERIFY(qFuzzyIsNull(plugin.overlapScore(rectangle)));
}

void CoreTests::polygonShapeSupportsPointInsertionAndRemovalRules() {
    Shape shape = Shape::fromPolygon(QStringLiteral("poly"),
                                     {QPointF(0, 0), QPointF(10, 0), QPointF(10, 10)},
                                     false);

    QVERIFY(shape.canInsertPoint());
    QVERIFY(!shape.canRemovePoint());
    QVERIFY(shape.insertPoint(1, QPointF(5, 0)));
    QCOMPARE(shape.points.size(), 4);
    QCOMPARE(shape.points[1], QPointF(5, 0));
    QVERIFY(shape.canRemovePoint());
    QVERIFY(shape.removePoint(1));
    QCOMPARE(shape.points.size(), 3);
    QCOMPARE(shape.points[1], QPointF(10, 0));
    QVERIFY(!shape.removePoint(1));
    QCOMPARE(shape.points.size(), 3);

    Shape rect = Shape::fromRect(QStringLiteral("rect"), QRectF(0, 0, 10, 10), false);
    QVERIFY(!rect.canInsertPoint());
    QVERIFY(!rect.insertPoint(1, QPointF(5, 0)));
    QVERIFY(!rect.canRemovePoint());
}

void CoreTests::performanceMonitorReturnsCpuAndMemoryText() {
    PerformanceMonitor monitor;
    const QString text = monitor.getPerformanceText();

    QVERIFY(text.contains("CPU"));
    QVERIFY(text.contains("MEM"));
    QVERIFY(!text.isEmpty());
}

void CoreTests::shortcutRegistryNormalizesAndLimitsBindings() {
    ShortcutRegistry registry;
    QVERIFY(registry.addCommand({QStringLiteral("create_rectangle"),
                                 QStringLiteral("annotation"),
                                 {QKeySequence(QStringLiteral("W")),
                                  QKeySequence(QStringLiteral("Ctrl+R"))}}));

    QCOMPARE(registry.command(QStringLiteral("create_rectangle")).shortcuts,
             QList<QKeySequence>({QKeySequence(QStringLiteral("W")),
                                  QKeySequence(QStringLiteral("Ctrl+R"))}));
    QVERIFY(registry.setShortcuts(QStringLiteral("create_rectangle"),
                                  {QKeySequence(QStringLiteral("v")),
                                   QKeySequence(QStringLiteral("Ctrl+Shift+R"))}));
    QCOMPARE(registry.command(QStringLiteral("create_rectangle")).shortcuts,
             QList<QKeySequence>({QKeySequence(QStringLiteral("V")),
                                  QKeySequence(QStringLiteral("Ctrl+Shift+R"))}));
    QVERIFY(!registry.setShortcuts(QStringLiteral("create_rectangle"),
                                   {QKeySequence(QStringLiteral("A")),
                                    QKeySequence(QStringLiteral("B")),
                                    QKeySequence(QStringLiteral("C"))}));
    QVERIFY(!registry.setShortcuts(QStringLiteral("create_rectangle"),
                                   {QKeySequence(QStringLiteral("Ctrl+K, Ctrl+C"))}));

    registry.reset(QStringLiteral("create_rectangle"));
    QCOMPARE(registry.command(QStringLiteral("create_rectangle")).shortcuts,
             registry.command(QStringLiteral("create_rectangle")).defaults);
}

void CoreTests::shortcutRegistryRejectsConflicts() {
    ShortcutRegistry registry;
    QVERIFY(registry.addCommand({QStringLiteral("view"), QStringLiteral("mode"),
                                 {QKeySequence(QStringLiteral("V"))}}));
    QVERIFY(registry.addCommand({QStringLiteral("verify"), QStringLiteral("annotation"),
                                 {QKeySequence(QStringLiteral("Space"))}}));
    QVERIFY(registry.setShortcuts(QStringLiteral("verify"),
                                  {QKeySequence(QStringLiteral("V")),
                                   QKeySequence(QStringLiteral("V"))}));

    const QVector<ShortcutConflict> conflicts = registry.conflicts();
    QCOMPARE(conflicts.size(), 1);
    QCOMPARE(conflicts.first().sequence, QKeySequence(QStringLiteral("V")));
    QCOMPARE(conflicts.first().locations.size(), 3);
    QCOMPARE(conflicts.first().locations.at(0).commandId, QStringLiteral("view"));
    QCOMPARE(conflicts.first().locations.at(1).commandId, QStringLiteral("verify"));
    QCOMPARE(conflicts.first().locations.at(1).slot, 0);
    QCOMPARE(conflicts.first().locations.at(2).slot, 1);
    QVERIFY(registry.hasConflicts());

    QVERIFY(registry.setShortcuts(QStringLiteral("verify"), {QKeySequence(QStringLiteral("Space"))}));
    QVERIFY(!registry.hasConflicts());
}

void CoreTests::shortcutRegistryPersistsOnlyOverrides() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString settingsPath = dir.filePath(QStringLiteral("shortcuts.ini"));

    ShortcutRegistry registry;
    QVERIFY(registry.addCommand({QStringLiteral("delete_shape"), QStringLiteral("edit"),
                                 {QKeySequence(QStringLiteral("Delete")),
                                  QKeySequence(QStringLiteral("X"))}}));
    QVERIFY(registry.addCommand({QStringLiteral("unused"), QStringLiteral("edit"),
                                 {QKeySequence(QStringLiteral("U"))}}));
    QVERIFY(registry.setShortcuts(QStringLiteral("delete_shape"),
                                  {QKeySequence(QStringLiteral("Backspace")),
                                   QKeySequence(QStringLiteral("D"))}));
    QVERIFY(registry.setShortcuts(QStringLiteral("unused"), {}));

    {
        QSettings settings(settingsPath, QSettings::IniFormat);
        registry.saveOverrides(settings);
        QVERIFY(settings.contains(QStringLiteral("shortcuts/delete_shape")));
        QVERIFY(settings.contains(QStringLiteral("shortcuts/unused")));
        QCOMPARE(settings.value(QStringLiteral("shortcuts/delete_shape")).toStringList(),
                 QStringList({QStringLiteral("Backspace"), QStringLiteral("D")}));
    }

    ShortcutRegistry restored;
    QVERIFY(restored.addCommand({QStringLiteral("delete_shape"), QStringLiteral("edit"),
                                 {QKeySequence(QStringLiteral("Delete")),
                                  QKeySequence(QStringLiteral("X"))}}));
    QVERIFY(restored.addCommand({QStringLiteral("unused"), QStringLiteral("edit"),
                                 {QKeySequence(QStringLiteral("U"))}}));
    {
        QSettings settings(settingsPath, QSettings::IniFormat);
        restored.loadOverrides(settings);
    }
    QCOMPARE(restored.command(QStringLiteral("delete_shape")).shortcuts,
             QList<QKeySequence>({QKeySequence(QStringLiteral("Backspace")),
                                  QKeySequence(QStringLiteral("D"))}));
    QVERIFY(restored.command(QStringLiteral("unused")).shortcuts.isEmpty());

    restored.resetAll();
    {
        QSettings settings(settingsPath, QSettings::IniFormat);
        restored.saveOverrides(settings);
        QVERIFY(!settings.contains(QStringLiteral("shortcuts/delete_shape")));
        QVERIFY(!settings.contains(QStringLiteral("shortcuts/unused")));
    }
}

void CoreTests::imageIoNormalizesHighBitGrayscale() {
    QImage source(3, 1, QImage::Format_Grayscale16);
    auto *pixels = reinterpret_cast<quint16 *>(source.scanLine(0));
    pixels[0] = 0;
    pixels[1] = 32768;
    pixels[2] = 65535;

    const QImage normalized = ImageIO::normalizeForDisplay(source);

    QCOMPARE(normalized.format(), QImage::Format_Grayscale8);
    QCOMPARE(normalized.pixelColor(0, 0).value(), 0);
    QCOMPARE(normalized.pixelColor(1, 0).value(), 127);
    QCOMPARE(normalized.pixelColor(2, 0).value(), 255);

    QImage constant(2, 1, QImage::Format_Grayscale16);
    auto *constantPixels = reinterpret_cast<quint16 *>(constant.scanLine(0));
    constantPixels[0] = 42;
    constantPixels[1] = 42;
    const QImage normalizedConstant = ImageIO::normalizeForDisplay(constant);
    QCOMPARE(normalizedConstant.pixelColor(0, 0).value(), 0);
    QCOMPARE(normalizedConstant.pixelColor(1, 0).value(), 0);

    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    const QString pngPath = QDir(temporary.path()).filePath(QStringLiteral("high-bit.png"));
    QVERIFY(source.save(pngPath, "PNG"));
    QString error;
    const QImage loaded = ImageIO::readForDisplay(pngPath, &error);
    QVERIFY2(!loaded.isNull(), qPrintable(error));
    QCOMPARE(loaded.format(), QImage::Format_Grayscale8);
    QCOMPARE(loaded.pixelColor(0, 0).value(), 0);
    QCOMPARE(loaded.pixelColor(1, 0).value(), 127);
    QCOMPARE(loaded.pixelColor(2, 0).value(), 255);
}

void CoreTests::imageIoReadsBoundedPreview() {
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());

    QImage source(400, 200, QImage::Format_RGB32);
    source.fill(QColor(30, 60, 90));
    const QString path = QDir(temporary.path()).filePath(QStringLiteral("preview.png"));
    QVERIFY(source.save(path, "PNG"));

    QSize sourceSize;
    QString error;
    const QImage preview = ImageIO::readPreview(path, 100, &sourceSize, &error);
    QVERIFY2(!preview.isNull(), qPrintable(error));
    QCOMPARE(sourceSize, QSize(400, 200));
    QVERIFY(qMax(preview.width(), preview.height()) <= 100);
    QCOMPARE(preview.size(), QSize(100, 50));
}

void CoreTests::imageIoPreviewPreservesExifDisplaySize() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("oriented-preview.jpg"));
    QVERIFY(writeExifOrientedJpeg(path));

    QSize sourceSize;
    QString error;
    const QImage preview = ImageIO::readPreview(path, 2, &sourceSize, &error);
    QVERIFY2(!preview.isNull(), qPrintable(error));
    QCOMPARE(sourceSize, QSize(3, 2));
    QCOMPARE(preview.size(), QSize(2, 1));
}

void CoreTests::imageIoNormalizesFloatingPointRgba() {
    QImage source(3, 1, QImage::Format_RGBA32FPx4);
    auto *pixels = reinterpret_cast<QRgbaFloat32 *>(source.scanLine(0));
    pixels[0] = {0.0f, 0.25f, 0.5f, 1.0f};
    pixels[1] = {0.5f, 0.5f, 0.25f, 1.0f};
    pixels[2] = {1.0f, 0.75f, 0.0f, 1.0f};

    const QImage normalized = ImageIO::normalizeForDisplay(source);

    QCOMPARE(normalized.format(), QImage::Format_RGBA8888);
    QCOMPARE(normalized.pixelColor(0, 0).red(), 0);
    QCOMPARE(normalized.pixelColor(1, 0).red(), 127);
    QCOMPARE(normalized.pixelColor(2, 0).red(), 255);
    QCOMPARE(normalized.pixelColor(0, 0).green(), 0);
    QCOMPARE(normalized.pixelColor(1, 0).green(), 127);
    QCOMPARE(normalized.pixelColor(2, 0).green(), 255);
    QCOMPARE(normalized.pixelColor(0, 0).blue(), 255);
    QCOMPARE(normalized.pixelColor(1, 0).blue(), 127);
    QCOMPARE(normalized.pixelColor(2, 0).blue(), 0);
    QCOMPARE(normalized.pixelColor(0, 0).alpha(), 255);
}

void CoreTests::imageIoReadsFloatTiffStackLikeLabelMe() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("stack.tiff"));
    QVERIFY(writeFloatStackTiff(path));

    QString error;
    const QImage image = ImageIO::readForDisplay(path, &error);
    QVERIFY2(!image.isNull(), qPrintable(error));
    QCOMPARE(image.size(), QSize(2, 3));
    QCOMPARE(image.format(), QImage::Format_RGB888);
    QCOMPARE(qRed(image.pixel(0, 0)), 0);
    QCOMPARE(qGreen(image.pixel(0, 0)), 0);
    QCOMPARE(qBlue(image.pixel(0, 0)), 0);
    QCOMPARE(qRed(image.pixel(1, 2)), 255);
    QCOMPARE(qGreen(image.pixel(1, 2)), 255);
    QCOMPARE(qBlue(image.pixel(1, 2)), 255);
}

void CoreTests::imageIoReadsUncompressedBigTiff() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("image.tif"));
    QVERIFY(writeUncompressedBigTiff(path));

    QString error;
    const QImage image = ImageIO::readForDisplay(path, &error);
    QVERIFY2(!image.isNull(), qPrintable(error));
    QCOMPARE(image.size(), QSize(2, 2));
    QCOMPARE(image.format(), QImage::Format_Grayscale8);
    QCOMPARE(image.pixelColor(0, 0).value(), 0);
    QCOMPARE(image.pixelColor(1, 0).value(), 64);
    QCOMPARE(image.pixelColor(0, 1).value(), 128);
    QCOMPARE(image.pixelColor(1, 1).value(), 255);
}

void CoreTests::imageIoReadsLzwFloatTiffLikeLabelMe() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("float-lzw.tif"));
    QVERIFY(writeLzwFloatTiff(path));

    QString error;
    const QImage image = ImageIO::readForDisplay(path, &error);
    QVERIFY2(!image.isNull(), qPrintable(error));
    QCOMPARE(image.size(), QSize(20, 30));
    QCOMPARE(image.format(), QImage::Format_Grayscale8);
    QCOMPARE(image.pixelColor(0, 0).value(), 0);
    QCOMPARE(image.pixelColor(1, 0).value(), 85);
    QCOMPARE(image.pixelColor(2, 0).value(), 170);
    QCOMPARE(image.pixelColor(3, 0).value(), 255);
}

void CoreTests::imageIoReadsLzwPredictorTwoTiff() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("predictor-lzw.tif"));
    QVERIFY(writeLzwPredictor16Tiff(path));

    QString error;
    const QImage image = ImageIO::readForDisplay(path, &error);
    QVERIFY2(!image.isNull(), qPrintable(error));
    QCOMPARE(image.size(), QSize(3, 1));
    QCOMPARE(image.format(), QImage::Format_Grayscale8);
    QCOMPARE(image.pixelColor(0, 0).value(), 255);
    QCOMPARE(image.pixelColor(1, 0).value(), 0);
    QCOMPARE(image.pixelColor(2, 0).value(), 128);
}

void CoreTests::imageIoReadsDeflateFloatTiffLikeLabelMe() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("float-deflate.tif"));
    QVERIFY(writeDeflateFloatTiff(path));

    QString error;
    const QImage image = ImageIO::readForDisplay(path, &error);
    QVERIFY2(!image.isNull(), qPrintable(error));
    QCOMPARE(image.size(), QSize(20, 30));
    QCOMPARE(image.format(), QImage::Format_Grayscale8);
    QCOMPARE(image.pixelColor(0, 0).value(), 0);
    QCOMPARE(image.pixelColor(1, 0).value(), 85);
    QCOMPARE(image.pixelColor(2, 0).value(), 170);
    QCOMPARE(image.pixelColor(3, 0).value(), 255);
}

void CoreTests::imageIoReadsDeflateFloatPredictorThreeTiff() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("float-deflate-predictor3.tif"));
    QVERIFY(writeDeflateFloatPredictorThreeTiff(path));

    QString error;
    const QImage image = ImageIO::readForDisplay(path, &error);
    QVERIFY2(!image.isNull(), qPrintable(error));
    QCOMPARE(image.size(), QSize(20, 30));
    QCOMPARE(image.format(), QImage::Format_Grayscale8);
    QCOMPARE(image.pixelColor(0, 0).value(), 0);
    QCOMPARE(image.pixelColor(1, 0).value(), 85);
    QCOMPARE(image.pixelColor(2, 0).value(), 170);
    QCOMPARE(image.pixelColor(3, 0).value(), 255);
}

void CoreTests::imageIoReadsTiledFloatTiff() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("tiled-float.tif"));
    QVERIFY(writeTiledFloatTiff(path));

    QString error;
    const QImage image = ImageIO::readForDisplay(path, &error);
    QVERIFY2(!image.isNull(), qPrintable(error));
    QCOMPARE(image.size(), QSize(5, 4));
    QCOMPARE(image.format(), QImage::Format_Grayscale8);
    QCOMPARE(image.pixelColor(0, 0).value(), 0);
    QCOMPARE(image.pixelColor(4, 0).value(), 53);
    QCOMPARE(image.pixelColor(0, 3).value(), 201);
    QCOMPARE(image.pixelColor(4, 3).value(), 255);
}

void CoreTests::imageIoReadsPackedFourBitTiff() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("packed-4bit.tif"));
    QVERIFY(writePackedFourBitTiff(path));

    QString error;
    const QImage image = ImageIO::readForDisplay(path, &error);
    QVERIFY2(!image.isNull(), qPrintable(error));
    QCOMPARE(image.size(), QSize(8, 1));
    QCOMPARE(image.format(), QImage::Format_Grayscale8);
    QCOMPARE(image.pixelColor(0, 0).value(), 0);
    QCOMPARE(image.pixelColor(1, 0).value(), 36);
    QCOMPARE(image.pixelColor(6, 0).value(), 218);
    QCOMPARE(image.pixelColor(7, 0).value(), 255);
}

void CoreTests::labelMeImageDataUsesTiffFallbackForMultibandInput() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("stack.tiff"));
    QVERIFY(writeFloatStackTiff(path));

    QByteArray encoded;
    QString error;
    QVERIFY2(AnnotationIO::loadImageData(path, &encoded, &error), qPrintable(error));
    QImage embedded;
    QVERIFY(embedded.loadFromData(encoded));
    QCOMPARE(embedded.size(), QSize(2, 3));
    QCOMPARE(embedded.format(), QImage::Format_RGB32);
}

void CoreTests::resourcePathsFindSharedAssets() {
    QVERIFY(QFileInfo::exists(ResourcePaths::filePath(QStringLiteral("resources/strings/strings.properties"))));
    QVERIFY(QFileInfo::exists(ResourcePaths::filePath(QStringLiteral("data/predefined_classes.txt"))));
}

void CoreTests::aiAssistBridgeBuildsPromptAndParsesShapes() {
    AiPrompt prompt;
    prompt.imagePath = QStringLiteral("C:/images/sample.png");
    prompt.modelName = QStringLiteral("sam2:latest");
    prompt.outputFormat = QStringLiteral("mask");
    prompt.points = {QPointF(10, 20), QPointF(40, 50)};
    prompt.pointLabels = {1, 0};

    const QJsonObject request = AiAssistBridge::requestObject(prompt);
    QCOMPARE(request.value(QStringLiteral("image_path")).toString(), prompt.imagePath);
    QCOMPARE(request.value(QStringLiteral("model")).toString(), prompt.modelName);
    QCOMPARE(request.value(QStringLiteral("output_format")).toString(), prompt.outputFormat);
    QCOMPARE(request.value(QStringLiteral("points")).toArray().size(), 2);
    QCOMPARE(request.value(QStringLiteral("point_labels")).toArray().at(1).toInt(), 0);
    QVERIFY(!AiAssistBridge::requestJson(prompt).isEmpty());

    const QByteArray response = QStringLiteral(R"json({
        "ok": true,
        "shapes": [{
            "label": "defect",
            "shape_type": "mask",
            "points": [[8, 18], [9, 19]],
            "mask_data": "%1",
            "group_id": 3,
            "description": "AI result",
            "flags": {"reviewed": true},
            "other_data": {"score": 0.92}
        }]
    })json").arg(tinyMaskBase64()).toUtf8();
    QVector<Shape> shapes;
    QString error;
    QVERIFY2(AiAssistBridge::parseResponse(response, &shapes, &error), qPrintable(error));
    QCOMPARE(shapes.size(), 1);
    QCOMPARE(shapes.first().shapeType, QStringLiteral("mask"));
    QVERIFY(shapes.first().closed);
    QCOMPARE(shapes.first().pointLabels, QVector<int>({1, 1}));
    QCOMPARE(shapes.first().label, QStringLiteral("defect"));
    QCOMPARE(shapes.first().groupId, 3);
    QCOMPARE(shapes.first().description, QStringLiteral("AI result"));
    QCOMPARE(shapes.first().flags.value(QStringLiteral("reviewed")), true);
    QCOMPARE(shapes.first().labelMeOtherData.value(QStringLiteral("score")).toDouble(), 0.92);

    QVERIFY(!AiAssistBridge::parseResponse(
        QByteArrayLiteral(R"({"ok":true,"shapes":[{"shape_type":"unknown","points":[[0,0]]}]})"),
        &shapes,
        &error));
    QVERIFY(error.contains(QStringLiteral("Unsupported AI shape type")));

    QVERIFY(!AiAssistBridge::parseResponse(
        QByteArrayLiteral(R"({"ok":true,"shapes":[{"shape_type":"mask","points":[[0,0],[2,2]],"mask_data":"aW52YWxpZA=="}]})"),
        &shapes,
        &error));
    QVERIFY(error.contains(QStringLiteral("PNG")));
}

void CoreTests::aiAssistBridgeRejectsInvalidShapePointCounts() {
    const QList<QPair<QString, QString>> invalidShapes = {
        {QStringLiteral("rectangle"), QStringLiteral("[[0,0]]")},
        {QStringLiteral("mask"), QStringLiteral("[[0,0],[4,4],[2,2]]")},
        {QStringLiteral("circle"), QStringLiteral("[[0,0]]")},
        {QStringLiteral("oriented_rectangle"), QStringLiteral("[[0,0],[4,0],[4,4]]")},
    };

    for (const auto &entry : invalidShapes) {
        const QByteArray response = QStringLiteral(
                                         R"({"ok":true,"shapes":[{"shape_type":"%1","points":%2}]})")
                                         .arg(entry.first, entry.second)
                                         .toUtf8();
        QVector<Shape> shapes;
        QString error;
        QVERIFY2(!AiAssistBridge::parseResponse(response, &shapes, &error),
                 qPrintable(entry.first + QStringLiteral(" should be rejected")));
        QVERIFY2(error.contains(QStringLiteral("point count")), qPrintable(error));
    }
}

void CoreTests::aiAssistBridgeRejectsMaskWithoutPayload() {
    const QList<QByteArray> payloads = {
        QByteArrayLiteral(R"({"shape_type":"mask","points":[[0,0],[1,1]]})"),
        QByteArrayLiteral(R"({"shape_type":"mask","points":[[0,0],[1,1]],"mask_data":null})"),
    };
    for (const QByteArray &shape : payloads) {
        const QByteArray response = QByteArrayLiteral(R"({"ok":true,"shapes":[)" ) +
                                                      shape + QByteArrayLiteral(R"(]})");
        QVector<Shape> shapes;
        QString error;
        QVERIFY2(!AiAssistBridge::parseResponse(response, &shapes, &error),
                 qPrintable(error));
        QVERIFY2(error.contains(QStringLiteral("mask_data")), qPrintable(error));
    }
}

void CoreTests::aiAssistBridgeRejectsMaskDimensionsThatDoNotMatchBoundingBox() {
    QJsonObject shape;
    shape[QStringLiteral("shape_type")] = QStringLiteral("mask");
    shape[QStringLiteral("points")] = QJsonArray{
        QJsonArray{0, 0},
        QJsonArray{2, 2},
    };
    shape[QStringLiteral("mask_data")] = tinyMaskBase64(); // 2x2, bbox is 3x3.
    const QJsonObject root{
        {QStringLiteral("ok"), true},
        {QStringLiteral("shapes"), QJsonArray{shape}},
    };

    QVector<Shape> shapes;
    QString error;
    QVERIFY2(!AiAssistBridge::parseResponse(
                 QJsonDocument(root).toJson(QJsonDocument::Compact), &shapes, &error),
             qPrintable(error));
    QVERIFY2(error.contains(QStringLiteral("dimensions")), qPrintable(error));
}

void CoreTests::aiAssistSuppressesOverlappingShapes() {
    const Shape existing = Shape::fromRect(QStringLiteral("existing"), QRectF(0, 0, 100, 100), false);
    const Shape first = Shape::fromRect(QStringLiteral("defect"), QRectF(120, 120, 40, 40), false);
    const Shape sameLabelOverlap =
        Shape::fromRect(QStringLiteral("defect"), QRectF(125, 125, 30, 30), false);
    const Shape differentLabelOverlap =
        Shape::fromRect(QStringLiteral("other"), QRectF(125, 125, 30, 30), false);
    const Shape existingOverlap =
        Shape::fromRect(QStringLiteral("defect"), QRectF(10, 10, 20, 20), false);

    const QVector<Shape> accepted = AiAssistBridge::suppressOverlappingShapes(
        {first, sameLabelOverlap, differentLabelOverlap, existingOverlap}, {existing}, 0.5);
    QCOMPARE(accepted.size(), 2);
    QCOMPARE(accepted.at(0).label, QStringLiteral("defect"));
    QCOMPARE(accepted.at(1).label, QStringLiteral("other"));
}

void CoreTests::aiAssistKeepsPartialOverlapBelowContainmentThreshold() {
    const Shape first = Shape::fromRect(QStringLiteral("defect"), QRectF(0, 0, 100, 100), false);
    const Shape partial =
        Shape::fromRect(QStringLiteral("defect"), QRectF(50, 0, 100, 100), false);

    const QVector<Shape> accepted = AiAssistBridge::suppressOverlappingShapes(
        {first, partial}, {}, 0.5);

    QCOMPARE(accepted.size(), 2);
}

void CoreTests::aiAssistBridgeBuildsTextPrompt() {
    AiPrompt prompt;
    prompt.imagePath = QStringLiteral("C:/images/text.png");
    prompt.modelName = QStringLiteral("yoloworld:latest");
    prompt.outputFormat = QStringLiteral("rectangle");
    prompt.textPrompt = true;
    prompt.texts = {QStringLiteral("person"), QStringLiteral("sofa")};
    prompt.scoreThreshold = 0.35;
    prompt.iouThreshold = 0.6;

    const QJsonObject request = AiAssistBridge::requestObject(prompt);
    QCOMPARE(request.value(QStringLiteral("prompt_type")).toString(), QStringLiteral("text"));
    QCOMPARE(request.value(QStringLiteral("texts")).toArray().size(), 2);
    QCOMPARE(request.value(QStringLiteral("texts")).toArray().at(1).toString(), QStringLiteral("sofa"));
    QCOMPARE(request.value(QStringLiteral("score_threshold")).toDouble(), 0.35);
    QCOMPARE(request.value(QStringLiteral("iou_threshold")).toDouble(), 0.6);
}

void CoreTests::aiAssistSessionReusesLongLivedBridge() {
    if (QStandardPaths::findExecutable(QStringLiteral("python")).isEmpty()) {
        QSKIP("python is required for the bridge session smoke test");
    }

    QTemporaryDir temp;
    QVERIFY(temp.isValid());
    const QString scriptPath = QDir(temp.path()).filePath(QStringLiteral("fake_bridge.py"));
    QFile script(scriptPath);
    QVERIFY(script.open(QIODevice::WriteOnly | QIODevice::Text));
    const QByteArray source =
        "import json, sys\n"
        "for line in sys.stdin:\n"
        "    if line.strip():\n"
        "        json.loads(line)\n"
        "        print('{\"ok\":true,\"shapes\":[]}', flush=True)\n";
    QCOMPARE(script.write(source), source.size());
    script.close();

    AiAssistSession session;
    QSignalSpy responseSpy(&session, &AiAssistSession::responseReady);
    QSignalSpy failureSpy(&session, &AiAssistSession::requestFailed);
    QVERIFY(session.request(scriptPath, QByteArrayLiteral("{}")));
    QTRY_COMPARE_WITH_TIMEOUT(responseSpy.count(), 1, 5000);
    QCOMPARE(failureSpy.count(), 0);
    QVERIFY(!session.busy());

    QVERIFY(session.request(scriptPath, QByteArrayLiteral("{\"second\":true}")));
    QTRY_COMPARE_WITH_TIMEOUT(responseSpy.count(), 2, 5000);
    QCOMPARE(failureSpy.count(), 0);
    session.stop();
}

void CoreTests::aiAssistSessionReportsProgressEvents() {
    if (QStandardPaths::findExecutable(QStringLiteral("python")).isEmpty()) {
        QSKIP("python is required for the bridge progress smoke test");
    }

    QTemporaryDir temp;
    QVERIFY(temp.isValid());
    const QString scriptPath = QDir(temp.path()).filePath(QStringLiteral("progress_bridge.py"));
    QFile script(scriptPath);
    QVERIFY(script.open(QIODevice::WriteOnly | QIODevice::Text));
    const QByteArray source =
        "import json, sys\n"
        "for line in sys.stdin:\n"
        "    if line.strip():\n"
        "        print(json.dumps({'event':'progress','model':'sam2:latest','file_index':0,'file_count':2,'filename':'weights.bin','bytes_done':128,'bytes_total':512}), flush=True)\n"
        "        print('{\"ok\":true,\"shapes\":[]}', flush=True)\n";
    QCOMPARE(script.write(source), source.size());
    script.close();

    AiAssistSession session;
    QSignalSpy responseSpy(&session, &AiAssistSession::responseReady);
    QSignalSpy progressSpy(&session, &AiAssistSession::progressUpdated);
    QSignalSpy failureSpy(&session, &AiAssistSession::requestFailed);
    QVERIFY(session.request(scriptPath, QByteArrayLiteral("{}")));
    QTRY_COMPARE_WITH_TIMEOUT(progressSpy.count(), 1, 5000);
    QTRY_COMPARE_WITH_TIMEOUT(responseSpy.count(), 1, 5000);
    QCOMPARE(failureSpy.count(), 0);
    const QList<QVariant> progress = progressSpy.first();
    QCOMPARE(progress.at(0).toString(), QStringLiteral("sam2:latest"));
    QCOMPARE(progress.at(1).toInt(), 0);
    QCOMPARE(progress.at(2).toInt(), 2);
    QCOMPARE(progress.at(3).toString(), QStringLiteral("weights.bin"));
    QCOMPARE(progress.at(4).toLongLong(), 128LL);
    QCOMPARE(progress.at(5).toLongLong(), 512LL);
    session.stop();
}

QTEST_MAIN(CoreTests)
#include "test_core.moc"
