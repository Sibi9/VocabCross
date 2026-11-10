#pragma once
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// A small, dependency-free Arabic shaper + visual re-orderer.
//
// Raylib draws codepoints strictly left-to-right and does not apply Unicode
// joining or the bidi algorithm.  We therefore:
//   1. substitute the correct isolated/initial/medial/final presentation form,
//   2. collapse lam + alef into the official lam-alef ligature,
//   3. split the text into Arabic clusters and LTR runs,
//   4. emit the string in *visual* order so DrawTextEx produces correct RTL.
// ---------------------------------------------------------------------------
namespace ArabicShaper {

struct ArabicCharInfo {
    uint32_t isolated;
    uint32_t initial;
    uint32_t medial;
    uint32_t final_form;
    bool connects_next;
};

inline ArabicCharInfo getCharInfo(uint32_t cp) {
    switch (cp) {
        case 0x0621: return {0xFE80, 0xFE80, 0xFE80, 0xFE80, false}; // ء
        case 0x0622: return {0xFE81, 0xFE81, 0xFE82, 0xFE82, false}; // آ
        case 0x0623: return {0xFE83, 0xFE83, 0xFE84, 0xFE84, false}; // أ
        case 0x0624: return {0xFE85, 0xFE85, 0xFE86, 0xFE86, false}; // ؤ
        case 0x0625: return {0xFE87, 0xFE87, 0xFE88, 0xFE88, false}; // إ
        case 0x0626: return {0xFE89, 0xFE8B, 0xFE8C, 0xFE8A, true};  // ئ
        case 0x0627: return {0xFE8D, 0xFE8D, 0xFE8E, 0xFE8E, false}; // ا
        case 0x0628: return {0xFE8F, 0xFE91, 0xFE92, 0xFE90, true};  // ب
        case 0x0629: return {0xFE93, 0xFE93, 0xFE94, 0xFE94, false}; // ة
        case 0x062A: return {0xFE95, 0xFE97, 0xFE98, 0xFE96, true};  // ت
        case 0x062B: return {0xFE99, 0xFE9B, 0xFE9C, 0xFE9A, true};  // ث
        case 0x062C: return {0xFE9D, 0xFE9F, 0xFEA0, 0xFE9E, true};  // ج
        case 0x062D: return {0xFEA1, 0xFEA3, 0xFEA4, 0xFEA2, true};  // ح
        case 0x062E: return {0xFEA5, 0xFEA7, 0xFEA8, 0xFEA6, true};  // خ
        case 0x062F: return {0xFEA9, 0xFEA9, 0xFEAA, 0xFEAA, false}; // د
        case 0x0630: return {0xFEAB, 0xFEAB, 0xFEAC, 0xFEAC, false}; // ذ
        case 0x0631: return {0xFEAD, 0xFEAD, 0xFEAE, 0xFEAE, false}; // ر
        case 0x0632: return {0xFEAF, 0xFEAF, 0xFEB0, 0xFEB0, false}; // ز
        case 0x0633: return {0xFEB1, 0xFEB3, 0xFEB4, 0xFEB2, true};  // س
        case 0x0634: return {0xFEB5, 0xFEB7, 0xFEB8, 0xFEB6, true};  // ش
        case 0x0635: return {0xFEB9, 0xFEBB, 0xFEBC, 0xFEBA, true};  // ص
        case 0x0636: return {0xFEBD, 0xFEBF, 0xFEC0, 0xFEBE, true};  // ض
        case 0x0637: return {0xFEC1, 0xFEC3, 0xFEC4, 0xFEC2, true};  // ط
        case 0x0638: return {0xFEC5, 0xFEC7, 0xFEC8, 0xFEC6, true};  // ظ
        case 0x0639: return {0xFEC9, 0xFECB, 0xFECC, 0xFECA, true};  // ع
        case 0x063A: return {0xFECD, 0xFECF, 0xFED0, 0xFECE, true};  // غ
        case 0x0641: return {0xFED1, 0xFED3, 0xFED4, 0xFED2, true};  // ف
        case 0x0642: return {0xFED5, 0xFED7, 0xFED8, 0xFED6, true};  // ق
        case 0x0643: return {0xFED9, 0xFEDB, 0xFEDC, 0xFEDA, true};  // ك
        case 0x0644: return {0xFEDD, 0xFEDF, 0xFEE0, 0xFEDE, true};  // ل
        case 0x0645: return {0xFEE1, 0xFEE3, 0xFEE4, 0xFEE2, true};  // م
        case 0x0646: return {0xFEE5, 0xFEE7, 0xFEE8, 0xFEE6, true};  // ن
        case 0x0647: return {0xFEE9, 0xFEEB, 0xFEEC, 0xFEEA, true};  // ه
        case 0x0648: return {0xFEED, 0xFEED, 0xFEEE, 0xFEEE, false}; // و
        case 0x0649: return {0xFEEF, 0xFEEF, 0xFEF0, 0xFEF0, false}; // ى
        case 0x064A: return {0xFEF1, 0xFEF3, 0xFEF4, 0xFEF2, true};  // ي
        default: return {cp, cp, cp, cp, false};
    }
}

inline bool isArabicBase(uint32_t cp) {
    return (cp >= 0x0621 && cp <= 0x063A) || (cp >= 0x0641 && cp <= 0x064A) ||
           (cp >= 0x066E && cp <= 0x06D3);
}

inline bool isArabicMark(uint32_t cp) {
    return (cp >= 0x064B && cp <= 0x065F) || cp == 0x0670 ||
           (cp >= 0x06D6 && cp <= 0x06ED) || (cp >= 0x08D3 && cp <= 0x08FF);
}

inline bool isArabic(uint32_t cp) {
    return isArabicBase(cp) || isArabicMark(cp) ||
           (cp >= 0x0600 && cp <= 0x0605) || (cp >= 0x060C && cp <= 0x060D) ||
           (cp >= 0x0660 && cp <= 0x0669) || (cp >= 0x06F0 && cp <= 0x06F9) ||
           cp == 0x061F || cp == 0x0640 ||
           (cp >= 0xFB50 && cp <= 0xFDFF) || (cp >= 0xFE70 && cp <= 0xFEFF);
}

inline bool isLtr(uint32_t cp) {
    return (cp >= 'A' && cp <= 'Z') || (cp >= 'a' && cp <= 'z') ||
           (cp >= '0' && cp <= '9') || cp == '+' || cp == '-' || cp == '%' ||
           cp == '.' || cp == ',' || cp == ':' || cp == '\'' || cp == '/' ||
           cp == '#' || cp == '&';
}

inline std::vector<uint32_t> utf8ToCodepoints(const std::string& str) {
    std::vector<uint32_t> cps;
    size_t i = 0;
    while (i < str.size()) {
        uint8_t c = static_cast<uint8_t>(str[i]);
        uint32_t cp = 0;
        size_t len = 0;
        if (c < 0x80) { cp = c; len = 1; }
        else if ((c & 0xE0) == 0xC0) { cp = c & 0x1F; len = 2; }
        else if ((c & 0xF0) == 0xE0) { cp = c & 0x0F; len = 3; }
        else if ((c & 0xF8) == 0xF0) { cp = c & 0x07; len = 4; }
        else { ++i; continue; }
        if (i + len > str.size()) break;
        for (size_t j = 1; j < len; ++j) cp = (cp << 6) | (static_cast<uint8_t>(str[i + j]) & 0x3F);
        cps.push_back(cp);
        i += len;
    }
    return cps;
}

inline std::string codepointsToUtf8(const std::vector<uint32_t>& cps) {
    std::string res;
    for (uint32_t cp : cps) {
        if (cp < 0x80) {
            res += static_cast<char>(cp);
        } else if (cp < 0x800) {
            res += static_cast<char>(0xC0 | (cp >> 6));
            res += static_cast<char>(0x80 | (cp & 0x3F));
        } else if (cp < 0x10000) {
            res += static_cast<char>(0xE0 | (cp >> 12));
            res += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            res += static_cast<char>(0x80 | (cp & 0x3F));
        } else {
            res += static_cast<char>(0xF0 | (cp >> 18));
            res += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
            res += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            res += static_cast<char>(0x80 | (cp & 0x3F));
        }
    }
    return res;
}

struct Cluster {
    std::vector<uint32_t> glyphs; // base (one codepoint) + combining marks
};

inline std::vector<Cluster> buildClusters(const std::vector<uint32_t>& cps) {
    std::vector<Cluster> clusters;
    for (size_t i = 0; i < cps.size(); ++i) {
        uint32_t cp = cps[i];
        if (isArabicMark(cp) && !clusters.empty()) {
            clusters.back().glyphs.push_back(cp);
            continue;
        }
        Cluster c;
        c.glyphs.push_back(cp);
        clusters.push_back(c);
    }
    return clusters;
}

inline uint32_t lamAlefForm(uint32_t alef, bool prevJoins) {
    switch (alef) {
        case 0x0622: return prevJoins ? 0xFEF6 : 0xFEF5; // لآ
        case 0x0623: return prevJoins ? 0xFEF8 : 0xFEF7; // لأ
        case 0x0625: return prevJoins ? 0xFEFA : 0xFEF9; // لإ
        case 0x0627: return prevJoins ? 0xFEFC : 0xFEFB; // لا
        default: return 0;
    }
}

inline std::string reshape(const std::string& input) {
    auto cps = utf8ToCodepoints(input);
    if (cps.empty()) return "";

    bool hasArabic = false;
    for (uint32_t cp : cps) {
        if (isArabic(cp)) { hasArabic = true; break; }
    }
    if (!hasArabic) return input;

    auto clusters = buildClusters(cps);

    // Substitute presentation forms and collapse lam-alef ligatures.
    std::vector<Cluster> shaped;
    shaped.reserve(clusters.size());
    for (size_t i = 0; i < clusters.size(); ++i) {
        uint32_t base = clusters[i].glyphs[0];
        Cluster out = clusters[i];

        if (base == 0x0644 && i + 1 < clusters.size()) {
            uint32_t nextBase = clusters[i + 1].glyphs[0];
            bool prevJoins = !shaped.empty() &&
                             isArabicBase(shaped.back().glyphs[0]) &&
                             getCharInfo(shaped.back().glyphs[0]).connects_next;
            uint32_t lig = lamAlefForm(nextBase, prevJoins);
            if (lig != 0) {
                out.glyphs[0] = lig;
                // Marks that belonged to the alef follow the ligature.
                for (size_t k = 1; k < clusters[i + 1].glyphs.size(); ++k) {
                    out.glyphs.push_back(clusters[i + 1].glyphs[k]);
                }
                shaped.push_back(out);
                ++i;
                continue;
            }
        }

        if (!isArabicBase(base)) {
            shaped.push_back(out);
            continue;
        }

        bool prevJoins = !shaped.empty() &&
                         isArabicBase(shaped.back().glyphs[0]) &&
                         getCharInfo(shaped.back().glyphs[0]).connects_next;
        bool curConnects = getCharInfo(base).connects_next;
        bool nextArabic = (i + 1 < clusters.size()) && isArabicBase(clusters[i + 1].glyphs[0]);
        bool nextJoins = curConnects && nextArabic;

        auto info = getCharInfo(base);
        uint32_t form = info.isolated;
        if (prevJoins && nextJoins && curConnects) form = info.medial;
        else if (prevJoins) form = info.final_form;
        else if (nextJoins && curConnects) form = info.initial;
        out.glyphs[0] = form;
        shaped.push_back(out);
    }

    // Split shaped clusters into directional runs.  Every Arabic run is reversed
    // internally; for an RTL paragraph the order of the runs is reversed too.
    struct Run {
        int type; // 0 = Arabic, 1 = LTR, 2 = neutral
        std::vector<Cluster> items;
    };
    std::vector<Run> runs;
    auto classify = [](uint32_t b) {
        if (isArabicBase(b) || isArabicMark(b) || (b >= 0xFB50 && b <= 0xFEFF) || b == 0x0640)
            return 0;
        if (isLtr(b)) return 1;
        return 2;
    };
    for (const auto& c : shaped) {
        int t = classify(c.glyphs[0]);
        if (runs.empty() || runs.back().type != t) {
            runs.push_back(Run{t, {}});
        }
        runs.back().items.push_back(c);
    }

    for (auto& run : runs) {
        if (run.type == 0) std::reverse(run.items.begin(), run.items.end());
    }

    bool rtlParagraph = isArabic(cps.front()) || (cps.size() > 1 && isArabic(cps[1]));
    if (rtlParagraph) std::reverse(runs.begin(), runs.end());

    // Mirror brackets in an RTL paragraph.
    if (rtlParagraph) {
        for (auto& run : runs) {
            for (auto& c : run.items) {
                uint32_t& g = c.glyphs[0];
                if (g == '(') g = ')';
                else if (g == ')') g = '(';
                else if (g == '[') g = ']';
                else if (g == ']') g = '[';
                else if (g == '{') g = '}';
                else if (g == '}') g = '{';
            }
        }
    }

    std::vector<uint32_t> output;
    for (const auto& run : runs) {
        for (const auto& c : run.items) {
            for (uint32_t g : c.glyphs) output.push_back(g);
        }
    }
    return codepointsToUtf8(output);
}

} // namespace ArabicShaper
