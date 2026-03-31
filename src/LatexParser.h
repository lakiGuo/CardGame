#ifndef LATEXPARSER_H
#define LATEXPARSER_H

#include <QString>
#include <QList>

struct ContentSegment {
    enum Type { Text, InlineMath, DisplayMath };
    Type type;
    QString text;
};

class LatexParser
{
public:
    static QList<ContentSegment> parse(const QString &content);
    static bool containsLatex(const QString &content);
};

#endif // LATEXPARSER_H
