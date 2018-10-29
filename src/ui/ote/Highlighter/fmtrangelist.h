#ifndef FMTLRANGELIST_H
#define FMTLRANGELIST_H

#include <vector>
#include <algorithm>

class FmtRangeList
{
public:
    struct FmtRange {
        int begin;
        int end;
        char type;

        friend bool operator==(const FmtRange& a, const FmtRange& b);
    };

    FmtRangeList();
    FmtRangeList& operator=(FmtRangeList&&) = default;

    void clear() { m_vec.clear(); }
    void append(int from, int to, char type) {
        if (from < 0 || to <= from) return;

        if (m_vec.empty()) {
            m_vec.push_back({from, to, type});
        } else {
            auto& last = m_vec.back();

            if (last.end+1 == from && last.type == type)
                last.end = to;
            else
                m_vec.push_back({from, to, type});
        }
    }

    bool isFormat(int from, int to, char type) const {
        auto it = std::find_if(m_vec.begin(), m_vec.end(), [from, to, type](const FmtRange& r){
            return r.begin < from && r.end > to && r.type == type;
        });
        return it != m_vec.end();
    }

    //const std::vector<FmtRange>& ranges() const { return m_vec; }

private:
    friend bool operator==(const FmtRangeList& a, const FmtRangeList& b);

    std::vector<FmtRange> m_vec;
};

bool operator==(const FmtRangeList& a, const FmtRangeList& b);

#endif // FMTLRANGELIST_H
