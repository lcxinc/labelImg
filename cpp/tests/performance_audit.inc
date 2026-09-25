// Opt-in Release benchmark; fixtures and saved annotations stay in a temporary directory.
void UiTests::performanceAudit() {
    if (!qEnvironmentVariableIsSet("LABELIMG_RUN_PERF")) QSKIP("Opt-in performance audit");
    resetTestSettings("performance-audit");
    QSettings().setValue("autosave", false);
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage fixtureImage(96, 64, QImage::Format_RGB32);
    fixtureImage.fill(QColor(90, 120, 160));
    QByteArray jpeg;
    QBuffer buffer(&jpeg);
    QVERIFY(buffer.open(QIODevice::WriteOnly));
    QVERIFY(fixtureImage.save(&buffer, "JPEG"));
    for (int i = 0; i < 6000; ++i) {
        const QString path = dir.filePath(QStringLiteral("image_%1.jpg").arg(i, 5, 10, QLatin1Char('0')));
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(jpeg), jpeg.size());
        file.close();
        if (i % 20 == 0) {
            AnnotationDocument doc;
            doc.imagePath = path;
            doc.imageSize = fixtureImage.size();
            doc.depth = 3;
            doc.shapes = {Shape::fromRect("defect", QRectF(10, 10, 30, 20), false)};
            QVERIFY(AnnotationIO::savePascalVoc(path.chopped(3) + "xml", doc));
        }
    }
    QJsonObject measurements;
    auto measure = [&](const QString &name, const auto &operation) {
        QElapsedTimer timer;
        timer.start();
        operation();
        const double ms = timer.nsecsElapsed() / 1000000.0;
        measurements.insert(name, ms);
        qInfo().noquote() << name << ms << "ms";
    };
    QElapsedTimer construction;
    construction.start();
    MainWindow window;
    measurements.insert("construction_ms", construction.nsecsElapsed() / 1000000.0);
    window.resize(1200, 800);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    measure("open_6000_ms", [&] { window.loadStartupArgs({"labelImgCpp", dir.path()}); });
    auto *files = window.findChild<QListWidget *>("fileList");
    auto *search = window.findChild<QLineEdit *>("fileSearchEdit");
    auto *thumbnails = window.findChild<QAction *>("thumbnailModeAction");
    auto *canvas = window.findChild<Canvas *>();
    QVERIFY(files && search && thumbnails && canvas);
    QCOMPARE(files->count(), 6000);
    measure("search_3_queries_ms", [&] {
        search->setText("image_01");
        search->setText("image_012");
        search->clear();
        QCoreApplication::processEvents();
    });
    QCOMPARE(files->count(), 6000);
    measure("navigate_12_ms", [&] {
        for (int i = 0; i < 6; ++i) {
            QMetaObject::invokeMethod(&window, "openNextImage");
            QMetaObject::invokeMethod(&window, "openPrevImage");
        }
    });
    measure("save_3_ms", [&] {
        for (int i = 0; i < 3; ++i) QMetaObject::invokeMethod(&window, "saveFile");
    });
    measure("enable_thumbnails_ms", [&] {
        thumbnails->trigger();
        QTRY_VERIFY_WITH_TIMEOUT(!files->item(0)->icon().isNull(), 60000);
    });
    QVERIFY(!files->item(0)->icon().isNull());
    measure("thumbnail_scroll_ms", [&] {
        files->scrollToBottom();
        QCoreApplication::processEvents();
    });
    thumbnails->trigger();
    QImage large(4000, 3000, QImage::Format_RGB32);
    for (int y = 0; y < large.height(); ++y) {
        auto *row = reinterpret_cast<QRgb *>(large.scanLine(y));
        for (int x = 0; x < large.width(); ++x) row[x] = qRgb((x * 13 + y) % 256, (y * 7 + x) % 256, (x + y * 3) % 256);
    }
    canvas->setPixmap(QPixmap::fromImage(large));
    canvas->setScale(0.2);
    canvas->setCreateMode(true);
    QCoreApplication::processEvents();
    measure("overview_500_ms", [&] {
        for (int i = 0; i < 500; ++i) QVERIFY(!canvas->overviewImage(200).isNull());
    });
    QVector<double> mouseSamples;
    measure("crosshair_200_ms", [&] {
        for (int i = 0; i < 200; ++i) {
            QElapsedTimer sample;
            sample.start();
            const QPointF p(50 + i % 300, 50 + i % 200);
            QMouseEvent event(QEvent::MouseMove, p, canvas->mapToGlobal(p.toPoint()), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
            QApplication::sendEvent(canvas, &event);
            QCoreApplication::processEvents();
            mouseSamples.append(sample.nsecsElapsed() / 1000000.0);
        }
    });
    std::sort(mouseSamples.begin(), mouseSamples.end());
    measurements.insert("crosshair_p95_ms", mouseSamples[189]);
    measurements.insert("crosshair_max_ms", mouseSamples.last());
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(30, 30));
    measure("draw_200_ms", [&] {
        for (int i = 0; i < 200; ++i) {
            const QPointF p(100 + i % 300, 100 + i % 200);
            QMouseEvent event(QEvent::MouseMove, p, canvas->mapToGlobal(p.toPoint()), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
            QApplication::sendEvent(canvas, &event);
            QCoreApplication::processEvents();
        }
    });
    QTest::keyClick(canvas, Qt::Key_Escape);
    canvas->setScale(0.5);
    QCoreApplication::processEvents();
    QTest::mousePress(canvas, Qt::MiddleButton, Qt::NoModifier, QPoint(300, 200));
    measure("pan_100_ms", [&] {
        for (int i = 0; i < 100; ++i) {
            const QPointF p(300 - i, 200 - i);
            QMouseEvent event(QEvent::MouseMove, p, canvas->mapToGlobal(p.toPoint()), Qt::NoButton, Qt::MiddleButton, Qt::NoModifier);
            QApplication::sendEvent(canvas, &event);
            QCoreApplication::processEvents();
        }
    });
    QTest::mouseRelease(canvas, Qt::MiddleButton, Qt::NoModifier, QPoint(200, 100));
    measure("zoom_12_ms", [&] {
        for (int i = 0; i < 12; ++i) {
            canvas->setScale(0.2 + i * 0.025);
            QCoreApplication::processEvents();
        }
    });
    canvas->setScale(0.2);
    measure("brightness_12mp_ms", [&] { canvas->setBrightness(65); canvas->setBrightness(50); });
    QVector<Shape> shapes;
    for (int i = 0; i < 1000; ++i) shapes.append(Shape::fromRect("defect", QRectF((i % 40) * 90, (i / 40) * 90, 70, 60), false));
    canvas->setShapes(shapes);
    QCoreApplication::processEvents();
    measure("paint_1000_shapes_30_ms", [&] {
        for (int i = 0; i < 30; ++i) { canvas->update(); QCoreApplication::processEvents(); }
    });
    QTest::qWait(100);
    QSignalSpy frames(canvas, &Canvas::frameRendered);
    QTest::qWait(300);
    measurements.insert("idle_frames_300ms", frames.count());
    measurements.insert("image_count", 6000);
    measurements.insert("qt_version", QT_VERSION_STR);
    measurements.insert("platform", QGuiApplication::platformName());
    const QString realDirectory = qEnvironmentVariable("LABELIMG_PERF_REAL_DIR");
    if (!realDirectory.isEmpty()) {
        // Explicit opt-in, read-only run. Never save or create real annotations.
        resetTestSettings("real-performance-audit");
        QSettings().setValue("autosave", false);
        MainWindow realWindow;
        realWindow.resize(1200, 800);
        realWindow.show();
        QVERIFY(QTest::qWaitForWindowExposed(&realWindow));
        measure("real_directory_open_ms", [&] { realWindow.loadStartupArgs({"labelImgCpp", realDirectory}); });
        auto *realFiles = realWindow.findChild<QListWidget *>("fileList");
        auto *realCanvas = realWindow.findChild<Canvas *>();
        QVERIFY(realFiles && realCanvas);
        measurements.insert("real_directory", realDirectory);
        measurements.insert("real_image_count", realFiles->count());
        measurements.insert("real_image_width", realCanvas->pixmapSize().width());
        measurements.insert("real_image_height", realCanvas->pixmapSize().height());
        measure("real_navigate_8_ms", [&] {
            for (int i = 0; i < 4; ++i) {
                QMetaObject::invokeMethod(&realWindow, "openNextImage");
                QMetaObject::invokeMethod(&realWindow, "openPrevImage");
                QCoreApplication::processEvents();
            }
        });
    }
    QFile output(qEnvironmentVariable("LABELIMG_PERF_OUTPUT", "performance-audit.json"));
    QVERIFY(output.open(QIODevice::WriteOnly));
    output.write(QJsonDocument(measurements).toJson(QJsonDocument::Indented));
}
