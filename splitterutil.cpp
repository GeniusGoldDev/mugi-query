#include "splitterutil.h"
#include <QSplitter>
#include <numeric>
namespace {
    double sum(const QList<double> &vs) {
        return std::accumulate(vs.begin(), vs.end(), 0.0);
    }
    double sum(const QList<int> &vs) {
        return std::accumulate(vs.begin(), vs.end(), 0);
    }
} // namespace
QList<int> SplitterUtil::proportional(const QList<int> &sizes, const QList<double> &ratios) {
    QList<int> result;
    double sizes_sum = sum(sizes);
    double ratios_sum = sum(ratios);
    for (int i = 0; i < sizes.size() - 1; i++) {
        result << (int)(sizes_sum * ratios[i] / ratios_sum);
    }
    result << (sizes_sum - sum(result));
    return result;
}
bool SplitterUtil::setRatio(QSplitter *splitter, const QList<double> &ratios) {
    QList<int> sizes = splitter->sizes();
    if (ratios.size() != sizes.size()) {
        return false;
    }
    splitter->setSizes(proportional(sizes, ratios));
    return true;
}
bool SplitterUtil::setRatio(QSplitter *splitter, double r0, double r1) {
    QList<double> ratios;
    ratios << r0 << r1;
    return setRatio(splitter, ratios);
}
bool SplitterUtil::setRatio(QSplitter *splitter, double r0, double r1, double r2) {
    QList<double> ratios;
    ratios << r0 << r1 << r2;
    return setRatio(splitter, ratios);
}
bool SplitterUtil::setRatio(QSplitter *splitter, double r0, double r1, double r2, double r3) {
    QList<double> ratios;
    ratios << r0 << r1 << r2 << r3;
    return setRatio(splitter, ratios);
}
bool SplitterUtil::setRatio(QSplitter *splitter, double r0, double r1, double r2, double r3,
                            double r4) {
    QList<double> ratios;
    ratios << r0 << r1 << r2 << r3 << r4;
    return setRatio(splitter, ratios);
}

#include <QDebug>

bool SplitterUtil::setFixed(QSplitter *splitter, int r0, int r1)
{
    QList<int> sizes = splitter->sizes();
    if (sizes.size() != 2) {
        return false;
    }
    double sizes_sum = sum(sizes);
    QList<int> result;
    if (r0 < 0) {
        result << (sizes_sum - r1) << r1;
    } else if (r1 < 0) {
        result << r0 << (sizes_sum - r0);
    }

    //qDebug() << "setFixed" << result;

    splitter->setSizes(result);
    return true;
}


