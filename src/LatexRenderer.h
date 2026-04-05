#ifndef LATEXRENDERER_H
#define LATEXRENDERER_H

#include <QObject>
#include <QPixmap>
#include <QHash>
#include <QString>
#include <QMutex>
#include <QSizeF>

class LatexRenderer : public QObject
{
    Q_OBJECT

public:
    static LatexRenderer *instance();
    bool isAvailable() const;

    /**
     * @brief 获取公式的 SVG 文件路径（自动编译并磁盘缓存）
     * @return SVG 文件路径，编译失败返回空字符串
     */
    QString getSvgPath(const QString &formula, bool displayMode = false);

    /**
     * @brief 渲染 LaTeX 公式为矢量清晰 Pixmap（位图降级方案）
     * @param formula LaTeX 公式
     * @param targetWidth 目标显示宽度 (px)，0 表示自动
     * @param targetHeight 目标显示高度 (px)，0 表示自动
     * @param displayMode 是否为 display math 模式
     */
    QPixmap render(const QString &formula, int targetWidth = 0, int targetHeight = 0,
                   bool displayMode = false);

    void clearCache();

private:
    explicit LatexRenderer(QObject *parent = nullptr);
    QString generateTexContent(const QString &formula, bool displayMode) const;
    QString compileToSvg(const QString &formula, bool displayMode) const;
    QPixmap svgToPixmap(const QString &svgFilePath, int targetWidth, int targetHeight) const;
    QPixmap renderViaDvipng(const QString &texFilePath, const QString &baseName) const;
    static QString cacheKey(const QString &formula, int width, int height, bool displayMode);

    bool m_latexAvailable{false};
    bool m_dvisvgmAvailable{false};
    bool m_dvipngAvailable{false};

    QHash<QString, QPixmap> m_cache;
    mutable QMutex m_cacheMutex;

    QString m_tempDir;
};

#endif // LATEXRENDERER_H
