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

        friend bool operator==(const FmtRange& a, const FmtRange& b);
    };

    FmtRangeList();

    void clear() { m_vec.clear(); }
    void addRange(int offset, int length) { // TODO: Make this appendRange instead()
        if (length == 0 || offset < 0) return;

        // Find insertion position
        auto it = std::lower_bound(m_vec.begin(), m_vec.end(), offset, [](const FmtRange& n, int val){
            return val > n.end;
        });
        FmtRange r{offset, offset+length};
        m_vec.insert(it, r); // TODO: Do a range check
    }

    bool isInRange(int pos, int length=0) {
        auto it = std::find_if(m_vec.begin(), m_vec.end(), [pos, length](const FmtRange& r){
            return r.begin < pos && r.end > pos+length;
        });
        return it != m_vec.end();
    }

    const std::vector<FmtRange>& ranges() { return m_vec; }

private:
    friend bool operator==(const FmtRangeList& a, const FmtRangeList& b);

    std::vector<FmtRange> m_vec;
};

bool operator==(const FmtRangeList& a, const FmtRangeList& b);

#endif // FMTLRANGELIST_H
