#ifndef LATEXRENDERER_H
#define LATEXRENDERER_H

#include <QObject>
#include <QPixmap>
#include <QHash>
#include <QString>
#include <QMutex>

class LatexRenderer : public QObject
{
    Q_OBJECT

public:
    static LatexRenderer *instance();
    bool isAvailable() const;
    QPixmap render(const QString &formula, bool displayMode = false);
    void clearCache();

private:
    explicit LatexRenderer(QObject *parent = nullptr);
    QString generateTexContent(const QString &formula, bool displayMode) const;
    QPixmap renderToPixmap(const QString &formula, bool displayMode) const;
    QPixmap renderViaDvipng(const QString &texFilePath, const QString &baseName) const;
    QPixmap renderViaConvert(const QString &texFilePath, const QString &baseName) const;
    static QString cacheKey(const QString &formula, bool displayMode);

    bool m_latexAvailable{false};
    bool m_dvipngAvailable{false};
    bool m_pdflatexAvailable{false};
    bool m_convertAvailable{false};

    QHash<QString, QPixmap> m_cache;
    mutable QMutex m_cacheMutex;

    QString m_tempDir;
};

#endif // LATEXRENDERER_H
