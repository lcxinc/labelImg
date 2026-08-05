#include "core/AiAssistSession.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QProcess>

AiAssistSession::AiAssistSession(QObject *parent) : QObject(parent), m_process(new QProcess(this)) {
    connect(m_process, &QProcess::readyReadStandardOutput, this, &AiAssistSession::readResponses);
    connect(m_process, &QProcess::started, this, &AiAssistSession::sendPendingRequest);
    connect(m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) {
            failPending(QStringLiteral("Python bridge failed to start"));
        }
    });
    connect(m_process,
            qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this,
            [this](int exitCode, QProcess::ExitStatus) {
                if (m_pending) {
                    failPending(QStringLiteral("AI bridge exited with code %1").arg(exitCode));
                }
                m_outputBuffer.clear();
                m_writeAfterStart = false;
            });
}

AiAssistSession::~AiAssistSession() {
    stop();
}

bool AiAssistSession::request(const QString &scriptPath, const QByteArray &payload) {
    if (scriptPath.isEmpty() || payload.isEmpty() || m_pending) {
        return false;
    }

    if (m_process->state() != QProcess::NotRunning && m_scriptPath != scriptPath) {
        stop();
    }
    m_scriptPath = scriptPath;
    m_pendingPayload = payload;
    m_pending = true;

    if (m_process->state() == QProcess::NotRunning) {
        m_writeAfterStart = true;
        m_process->setProgram(QStringLiteral("python"));
        m_process->setArguments({m_scriptPath, QStringLiteral("--server")});
        m_process->start();
    } else {
        sendPendingRequest();
    }
    return true;
}

bool AiAssistSession::busy() const {
    return m_pending;
}

void AiAssistSession::stop() {
    m_pending = false;
    m_writeAfterStart = false;
    m_pendingPayload.clear();
    m_outputBuffer.clear();
    if (m_process->state() != QProcess::NotRunning) {
        m_process->terminate();
        if (!m_process->waitForFinished(500)) {
            m_process->kill();
            m_process->waitForFinished(500);
        }
    }
}

void AiAssistSession::sendPendingRequest() {
    if (!m_pending || m_process->state() != QProcess::Running) {
        return;
    }
    if (!m_writeAfterStart && m_process->bytesToWrite() > 0) {
        return;
    }
    m_writeAfterStart = false;
    m_process->write(m_pendingPayload);
    m_process->write("\n");
    m_process->waitForBytesWritten(1000);
}

void AiAssistSession::readResponses() {
    m_outputBuffer.append(m_process->readAllStandardOutput());
    while (true) {
        const int newline = m_outputBuffer.indexOf('\n');
        if (newline < 0) {
            return;
        }
        const QByteArray line = m_outputBuffer.left(newline).trimmed();
        m_outputBuffer.remove(0, newline + 1);
        if (line.isEmpty()) {
            continue;
        }

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(line, &parseError);
        // OSAM may emit diagnostic text on stdout in some deployments. Only
        // consume JSON objects as protocol responses; stderr remains visible
        // to the caller through QProcess diagnostics when needed.
        if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
            continue;
        }
        const QJsonObject object = document.object();
        if (object.value(QStringLiteral("event")).toString() == QStringLiteral("progress")) {
            const auto integerValue = [&object](const QString &key, qint64 fallback) {
                const QJsonValue value = object.value(key);
                return value.isDouble() ? static_cast<qint64>(value.toDouble()) : fallback;
            };
            emit progressUpdated(
                object.value(QStringLiteral("model")).toString(),
                static_cast<int>(integerValue(QStringLiteral("file_index"), 0)),
                static_cast<int>(integerValue(QStringLiteral("file_count"), 0)),
                object.value(QStringLiteral("filename")).toString(),
                integerValue(QStringLiteral("bytes_done"), 0),
                integerValue(QStringLiteral("bytes_total"), -1));
            continue;
        }
        if (!m_pending) {
            continue;
        }
        m_pending = false;
        m_pendingPayload.clear();
        emit responseReady(line);
        return;
    }
}

void AiAssistSession::failPending(const QString &message) {
    if (!m_pending) {
        return;
    }
    m_pending = false;
    m_pendingPayload.clear();
    emit requestFailed(message);
}
