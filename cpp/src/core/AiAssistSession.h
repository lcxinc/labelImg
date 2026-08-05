#pragma once

#include <QObject>

class QProcess;

// Long-lived JSON-lines bridge used by consecutive LabelMe AI prompts. The
// session owns only the Python process lifecycle; shape validation remains in
// AiAssistBridge so the one-shot and server protocols share the same rules.
class AiAssistSession : public QObject {
    Q_OBJECT

public:
    explicit AiAssistSession(QObject *parent = nullptr);
    ~AiAssistSession() override;

    bool request(const QString &scriptPath, const QByteArray &payload);
    bool busy() const;
    void stop();

signals:
    void responseReady(const QByteArray &payload);
    void progressUpdated(const QString &modelName,
                         int fileIndex,
                         int fileCount,
                         const QString &fileName,
                         qint64 bytesDone,
                         qint64 bytesTotal);
    void requestFailed(const QString &message);

private:
    void sendPendingRequest();
    void readResponses();
    void failPending(const QString &message);

    QProcess *m_process = nullptr;
    QString m_scriptPath;
    QByteArray m_pendingPayload;
    QByteArray m_outputBuffer;
    bool m_pending = false;
    bool m_writeAfterStart = false;
};
