#include "LatexRenderer.h"
#include <QStandardPaths>
#include <QDir>
#include <QProcess>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QDateTime>

LatexRenderer *LatexRenderer::instance()
{
    static LatexRenderer renderer;
    return &renderer;
}

LatexRenderer::LatexRenderer(QObject *parent)
    : QObject(parent)
{
    m_latexAvailable = !QStandardPaths::findExecutable("latex").isEmpty();
    m_dvipngAvailable = !QStandardPaths::findExecutable("dvipng").isEmpty();
    m_pdflatexAvailable = !QStandardPaths::findExecutable("pdflatex").isEmpty();
    m_convertAvailable = !QStandardPaths::findExecutable("convert").isEmpty();

    m_tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/knowledgecardgame_latex";
    QDir().mkpath(m_tempDir);

    // Clean up temp files older than 24 hours
    QDateTime cutoff = QDateTime::currentDateTime().addDays(-1);
    QDir dir(m_tempDir);
    for (const QFileInfo &fi : dir.entryInfoList(QDir::Files)) {
        if (fi.lastModified() < cutoff) {
            QFile::remove(fi.absoluteFilePath());
        }
    }
}

bool LatexRenderer::isAvailable() const
{
    return (m_latexAvailable && m_dvipngAvailable) ||
           (m_pdflatexAvailable && m_convertAvailable);
}

QString LatexRenderer::generateTexContent(const QString &formula, bool displayMode) const
{
    QString content = QString(
        "\\documentclass[preview,border=1pt]{standalone}\n"
        "\\usepackage{amsmath}\n"
        "\\usepackage{amssymb}\n"
        "\\usepackage{amsfonts}\n"
        "\\begin{document}\n"
    );

    if (displayMode) {
        content += "\\[\n" + formula + "\n\\]\n";
    } else {
        content += "$" + formula + "$\n";
    }

    content += "\\end{document}\n";
    return content;
}

QString LatexRenderer::cacheKey(const QString &formula, bool displayMode)
{
    QByteArray hash = QCryptographicHash::hash(
        (QString(displayMode ? "D" : "I") + formula).toUtf8(),
        QCryptographicHash::Md5
    );
    return QString::fromLatin1(hash.toHex());
}

QPixmap LatexRenderer::render(const QString &formula, bool displayMode)
{
    QString key = cacheKey(formula, displayMode);

    QMutexLocker locker(&m_cacheMutex);
    auto it = m_cache.find(key);
    if (it != m_cache.end() && !it->isNull())
        return *it;
    locker.unlock();

    QPixmap result = renderToPixmap(formula, displayMode);

    locker.relock();
    m_cache.insert(key, result);
    return result;
}

QPixmap LatexRenderer::renderToPixmap(const QString &formula, bool displayMode) const
{
    QString key = cacheKey(formula, displayMode);
    QString baseName = "formula_" + key;
    QString texPath = m_tempDir + "/" + baseName + ".tex";

    // Write .tex file
    QString texContent = generateTexContent(formula, displayMode);
    QFile texFile(texPath);
    if (!texFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return QPixmap();
    texFile.write(texContent.toUtf8());
    texFile.close();

    // Try dvipng pipeline first (faster)
    QPixmap result = renderViaDvipng(texPath, baseName);
    if (!result.isNull())
        return result;

    // Fallback to pdflatex + convert pipeline
    result = renderViaConvert(texPath, baseName);
    if (!result.isNull())
        return result;

    return QPixmap();
}

QPixmap LatexRenderer::renderViaDvipng(const QString &texFilePath, const QString &baseName) const
{
    if (!m_latexAvailable || !m_dvipngAvailable)
        return QPixmap();

    QString dir = QFileInfo(texFilePath).absolutePath();

    // latex -> DVI
    QProcess latex;
    latex.setWorkingDirectory(dir);
    latex.start("latex", {
        "-interaction=nonstopmode",
        "-halt-on-error",
        "-output-directory=" + dir,
        baseName + ".tex"
    });
    if (!latex.waitForFinished(10000) || latex.exitCode() != 0)
        return QPixmap();

    // DVI -> PNG
    QProcess dvipng;
    dvipng.setWorkingDirectory(dir);
    dvipng.start("dvipng", {
        "-T", "tight",
        "-D", "300",
        "-bg", "Transparent",
        "-o", baseName + ".png",
        baseName + ".dvi"
    });
    if (!dvipng.waitForFinished(10000) || dvipng.exitCode() != 0)
        return QPixmap();

    return QPixmap(dir + "/" + baseName + ".png");
}

QPixmap LatexRenderer::renderViaConvert(const QString &texFilePath, const QString &baseName) const
{
    if (!m_pdflatexAvailable || !m_convertAvailable)
        return QPixmap();

    QString dir = QFileInfo(texFilePath).absolutePath();

    // pdflatex -> PDF
    QProcess pdflatex;
    pdflatex.setWorkingDirectory(dir);
    pdflatex.start("pdflatex", {
        "-interaction=nonstopmode",
        "-halt-on-error",
        "-output-directory=" + dir,
        baseName + ".tex"
    });
    if (!pdflatex.waitForFinished(15000) || pdflatex.exitCode() != 0)
        return QPixmap();

    // PDF -> PNG
    QProcess convert;
    convert.setWorkingDirectory(dir);
    convert.start("convert", {
        "-density", "300",
        "-transparent", "white",
        baseName + ".pdf",
        baseName + ".png"
    });
    if (!convert.waitForFinished(15000) || convert.exitCode() != 0)
        return QPixmap();

    return QPixmap(dir + "/" + baseName + ".png");
}

void LatexRenderer::clearCache()
{
    QMutexLocker locker(&m_cacheMutex);
    m_cache.clear();
}
