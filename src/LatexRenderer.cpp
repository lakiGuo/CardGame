#include "LatexRenderer.h"
#include <QStandardPaths>
#include <QDir>
#include <QProcess>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QDateTime>
#include <QSvgRenderer>
#include <QPainter>

LatexRenderer *LatexRenderer::instance()
{
    static LatexRenderer renderer;
    return &renderer;
}

LatexRenderer::LatexRenderer(QObject *parent)
    : QObject(parent)
{
    m_latexAvailable = !QStandardPaths::findExecutable("latex").isEmpty();
    m_dvisvgmAvailable = !QStandardPaths::findExecutable("dvisvgm").isEmpty();
    m_dvipngAvailable = !QStandardPaths::findExecutable("dvipng").isEmpty();

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
    return (m_latexAvailable && m_dvisvgmAvailable) ||
           (m_latexAvailable && m_dvipngAvailable);
}

QString LatexRenderer::getSvgPath(const QString &formula, bool displayMode)
{
    if (!m_latexAvailable || !m_dvisvgmAvailable)
        return QString();
    return compileToSvg(formula, displayMode);
}

QString LatexRenderer::generateTexContent(const QString &formula, bool displayMode) const
{
    bool cjk = containsCJK(formula);

    QString content = "\\documentclass[preview,border=0pt]{standalone}\n"
                      "\\usepackage{amsmath}\n"
                      "\\usepackage{amssymb}\n"
                      "\\usepackage{amsfonts}\n";

    if (cjk) {
        content += "\\usepackage[encapsulated]{CJK}\n";
    }

    content += "\\begin{document}\n";

    if (cjk) {
        content += "\\begin{CJK}{UTF8}{gbsn}\n";
    }

    if (displayMode) {
        content += "\\[\n" + formula + "\n\\]\n";
    } else {
        content += "$" + formula + "$\n";
    }

    if (cjk) {
        content += "\\end{CJK}\n";
    }

    content += "\\end{document}\n";
    return content;
}

bool LatexRenderer::containsCJK(const QString &text)
{
    for (int i = 0; i < text.length(); i++) {
        uint u = text.at(i).unicode();
        // CJK Unified Ideographs: U+4E00..U+9FFF
        // CJK Extension A: U+3400..U+4DBF
        // CJK Compatibility Ideographs: U+F900..U+FAFF
        // Hiragana: U+3040..U+309F
        // Katakana: U+30A0..U+30FF
        // CJK Radicals Supplement / Kangxi: U+2E80..U+2FDF, U+2F00..U+2FDF
        // Fullwidth forms: U+FF00..U+FFEF
        if ((u >= 0x4E00 && u <= 0x9FFF) ||
            (u >= 0x3400 && u <= 0x4DBF) ||
            (u >= 0xF900 && u <= 0xFAFF) ||
            (u >= 0x3040 && u <= 0x309F) ||
            (u >= 0x30A0 && u <= 0x30FF) ||
            (u >= 0x2E80 && u <= 0x2FDF) ||
            (u >= 0xFF00 && u <= 0xFFEF))
            return true;
    }
    return false;
}

QString LatexRenderer::cacheKey(const QString &formula, int width, int height, bool displayMode)
{
    QByteArray hash = QCryptographicHash::hash(
        (QString(displayMode ? "D" : "I") + formula +
         QString::number(width) + "x" + QString::number(height)).toUtf8(),
        QCryptographicHash::Md5
    );
    return QString::fromLatin1(hash.toHex());
}

QPixmap LatexRenderer::render(const QString &formula, int targetWidth, int targetHeight,
                                bool displayMode)
{
    QString key = cacheKey(formula, targetWidth, targetHeight, displayMode);

    QMutexLocker locker(&m_cacheMutex);
    auto it = m_cache.find(key);
    if (it != m_cache.end() && !it->isNull())
        return *it;
    locker.unlock();

    QPixmap result;

    // 优先使用 SVG 管线（矢量清晰）
    if (m_dvisvgmAvailable && m_latexAvailable) {
        QString svgPath = compileToSvg(formula, displayMode);
        if (!svgPath.isEmpty()) {
            result = svgToPixmap(svgPath, targetWidth, targetHeight);
        }
    }

    // 降级到 dvipng 管线
    if (result.isNull() && m_dvipngAvailable && m_latexAvailable) {
        QString key2 = cacheKey(formula, 0, 0, displayMode);
        QString baseName = "formula_" + key2;
        QString texPath = m_tempDir + "/" + baseName + ".tex";

        QString texContent = generateTexContent(formula, displayMode);
        QFile texFile(texPath);
        if (texFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            texFile.write(texContent.toUtf8());
            texFile.close();
            result = renderViaDvipng(texPath, baseName);
        }
    }

    locker.relock();
    m_cache.insert(key, result);
    return result;
}

QString LatexRenderer::compileToSvg(const QString &formula, bool displayMode) const
{
    QString key = cacheKey(formula, 0, 0, displayMode);
    QString baseName = "formula_" + key;
    QString texPath = m_tempDir + "/" + baseName + ".tex";
    QString svgPath = m_tempDir + "/" + baseName + ".svg";

    // 如果 SVG 已经存在，直接返回
    if (QFile::exists(svgPath))
        return svgPath;

    // 写入 .tex 文件
    QString texContent = generateTexContent(formula, displayMode);
    QFile texFile(texPath);
    if (!texFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return QString();
    texFile.write(texContent.toUtf8());
    texFile.close();

    QString dir = QFileInfo(texPath).absolutePath();

    // Step 1: latex -> DVI
    QProcess latex;
    latex.setWorkingDirectory(dir);
    latex.start("latex", {
        "-interaction=nonstopmode",
        "-halt-on-error",
        "-output-directory=" + dir,
        baseName + ".tex"
    });
    if (!latex.waitForFinished(10000) || latex.exitCode() != 0)
        return QString();

    // Step 2: DVI -> SVG (dvisvgm)
    QProcess dvisvgm;
    dvisvgm.setWorkingDirectory(dir);
    dvisvgm.start("dvisvgm", {
        "--no-fonts",
        "--exact",
        "-o", svgPath,
        baseName + ".dvi"
    });
    if (!dvisvgm.waitForFinished(10000) || dvisvgm.exitCode() != 0)
        return QString();

    return QFile::exists(svgPath) ? svgPath : QString();
}

QPixmap LatexRenderer::svgToPixmap(const QString &svgFilePath,
                                    int targetWidth, int targetHeight) const
{
    QSvgRenderer renderer(svgFilePath);
    if (!renderer.isValid())
        return QPixmap();

    QSizeF defaultSize = renderer.defaultSize();
    qreal svgAspect = defaultSize.width() / defaultSize.height();

    // 确定输出尺寸
    int outW, outH;
    if (targetWidth > 0 && targetHeight > 0) {
        outW = targetWidth;
        outH = targetHeight;
    } else if (targetWidth > 0) {
        outW = targetWidth;
        outH = static_cast<int>(targetWidth / svgAspect);
    } else if (targetHeight > 0) {
        outH = targetHeight;
        outW = static_cast<int>(targetHeight * svgAspect);
    } else {
        outW = static_cast<int>(defaultSize.width());
        outH = static_cast<int>(defaultSize.height());
    }

    QPixmap pixmap(outW, outH);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    renderer.render(&painter, QRectF(0, 0, outW, outH));
    painter.end();

    return pixmap;
}

QPixmap LatexRenderer::renderViaDvipng(const QString &texFilePath, const QString &baseName) const
{
    if (!m_latexAvailable || !m_dvipngAvailable)
        return QPixmap();

    QString dir = QFileInfo(texFilePath).absolutePath();

    QProcess dvipng;
    dvipng.setWorkingDirectory(dir);
    dvipng.start("dvipng", {
        "-T", "tight",
        "-D", "600",
        "-bg", "Transparent",
        "-gamma", "1.5",
        "-Q", "9",
        "-o", baseName + ".png",
        baseName + ".dvi"
    });
    if (!dvipng.waitForFinished(10000) || dvipng.exitCode() != 0)
        return QPixmap();

    QString pngPath = dir + "/" + baseName + ".png";
    QPixmap rawPng(pngPath);
    if (rawPng.isNull())
        return QPixmap();
    QImage converted = rawPng.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
    return QPixmap::fromImage(converted);
}

void LatexRenderer::clearCache()
{
    QMutexLocker locker(&m_cacheMutex);
    m_cache.clear();
}
