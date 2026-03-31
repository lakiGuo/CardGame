#include "LatexParser.h"

QList<ContentSegment> LatexParser::parse(const QString &content)
{
    QList<ContentSegment> segments;
    int pos = 0;

    while (pos < content.length()) {
        // Escaped dollar sign: \$
        if (content[pos] == '\\' && pos + 1 < content.length() && content[pos + 1] == '$') {
            if (!segments.isEmpty() && segments.last().type == ContentSegment::Text) {
                segments.last().text += '$';
            } else {
                segments.append({ContentSegment::Text, "$"});
            }
            pos += 2;
            continue;
        }

        // Display math: $$...$$
        if (content[pos] == '$' && pos + 1 < content.length() && content[pos + 1] == '$') {
            int endPos = content.indexOf("$$", pos + 2);
            if (endPos != -1) {
                QString formula = content.mid(pos + 2, endPos - pos - 2).trimmed();
                if (!formula.isEmpty()) {
                    segments.append({ContentSegment::DisplayMath, formula});
                }
                pos = endPos + 2;
                continue;
            }
        }

        // Inline math: $...$
        if (content[pos] == '$') {
            int endPos = content.indexOf('$', pos + 1);
            if (endPos != -1) {
                // Make sure it's not the start of a display math delimiter
                if (endPos + 1 < content.length() && content[endPos + 1] == '$') {
                    // This $ is actually the start of $$, treat as plain text
                    if (!segments.isEmpty() && segments.last().type == ContentSegment::Text) {
                        segments.last().text += '$';
                    } else {
                        segments.append({ContentSegment::Text, "$"});
                    }
                    pos++;
                    continue;
                }
                QString formula = content.mid(pos + 1, endPos - pos - 1).trimmed();
                if (!formula.isEmpty()) {
                    segments.append({ContentSegment::InlineMath, formula});
                }
                pos = endPos + 1;
                continue;
            }
        }

        // Plain text character
        if (!segments.isEmpty() && segments.last().type == ContentSegment::Text) {
            segments.last().text += content[pos];
        } else {
            segments.append({ContentSegment::Text, content[pos]});
        }
        pos++;
    }

    return segments;
}

bool LatexParser::containsLatex(const QString &content)
{
    for (int i = 0; i < content.length(); i++) {
        if (content[i] == '\\' && i + 1 < content.length() && content[i + 1] == '$') {
            i++;
            continue;
        }
        if (content[i] == '$') {
            // Check if there's a matching closing $
            if (i + 1 < content.length() && content[i + 1] == '$') {
                // Display math
                if (content.indexOf("$$", i + 2) != -1)
                    return true;
            } else {
                // Inline math
                if (content.indexOf('$', i + 1) != -1)
                    return true;
            }
        }
    }
    return false;
}
