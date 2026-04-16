#include "LatexParser.h"

// Skip past a LaTeX command argument: \cmdname{...} including nested braces.
// Returns the position after the closing '}', or pos if not a brace command.
static int skipBraceArg(const QString &content, int pos)
{
    if (pos >= content.length() || content[pos] != '{')
        return pos;

    int depth = 0;
    while (pos < content.length()) {
        if (content[pos] == '\\' && pos + 1 < content.length() && content[pos + 1] == '{') {
            // Escaped brace: \{
            pos += 2;
            continue;
        }
        if (content[pos] == '\\' && pos + 1 < content.length() && content[pos + 1] == '}') {
            pos += 2;
            continue;
        }
        if (content[pos] == '{') {
            depth++;
        } else if (content[pos] == '}') {
            depth--;
            if (depth == 0)
                return pos + 1; // past the closing '}'
        }
        pos++;
    }
    return pos; // unmatched braces, return end
}

// Skip past a LaTeX command: \cmdname followed by optional brace argument.
// Returns the position after the command and its arguments.
static int skipLatexCommand(const QString &content, int pos)
{
    if (pos >= content.length() || content[pos] != '\\')
        return pos;

    int i = pos + 1;
    // Skip command name (letters only)
    while (i < content.length() && content[i].isLetter())
        i++;

    // If no letters were consumed, this is a symbol escape like \{ \} \$
    // Just skip the backslash and the next character, don't treat { as an argument.
    if (i == pos + 1)
        return (pos + 2 < content.length()) ? pos + 2 : content.length();

    // If there's a brace argument, skip it
    if (i < content.length() && content[i] == '{')
        return skipBraceArg(content, i);

    return i;
}

// Find the next unescaped '$' that is not inside a LaTeX command.
static int findNextDollar(const QString &content, int from)
{
    int pos = from;
    while (pos < content.length()) {
        // Escaped dollar: \$
        if (content[pos] == '\\' && pos + 1 < content.length() && content[pos + 1] == '$') {
            pos += 2;
            continue;
        }
        // Backslash commands: skip them so we don't match $ inside \text{...} etc.
        if (content[pos] == '\\') {
            pos = skipLatexCommand(content, pos);
            continue;
        }
        if (content[pos] == '$')
            return pos;
        pos++;
    }
    return -1;
}

// Find the next "$$" that is not inside a LaTeX command.
static int findNextDisplayDollar(const QString &content, int from)
{
    int pos = from;
    while (pos < content.length()) {
        // Escaped dollar: \$
        if (content[pos] == '\\' && pos + 1 < content.length() && content[pos + 1] == '$') {
            pos += 2;
            continue;
        }
        // Backslash commands: skip them
        if (content[pos] == '\\') {
            pos = skipLatexCommand(content, pos);
            continue;
        }
        if (content[pos] == '$' && pos + 1 < content.length() && content[pos + 1] == '$')
            return pos;
        // Single $ that is NOT part of $$ should not block us, but we should
        // not return here — only match $$
        pos++;
    }
    return -1;
}

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
            int endPos = findNextDisplayDollar(content, pos + 2);
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
            int endPos = findNextDollar(content, pos + 1);
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
                if (findNextDisplayDollar(content, i + 2) != -1)
                    return true;
            } else {
                // Inline math
                if (findNextDollar(content, i + 1) != -1)
                    return true;
            }
        }
    }
    return false;
}
