#include "fmtrangelist.h"

FmtRangeList::FmtRangeList() {}

bool operator==(const FmtRangeList& a, const FmtRangeList& b)
{
    return a.m_vec == b.m_vec;
}

bool operator==(const FmtRangeList::FmtRange& a, const FmtRangeList::FmtRange& b)
{
    return a.begin == b.begin && a.end == b.end;
}
