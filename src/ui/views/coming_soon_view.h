#pragma once

#include "ui/tools.h"

#include <QWidget>

namespace drpdf {

class ComingSoonView : public QWidget {
    Q_OBJECT
public:
    explicit ComingSoonView(Tool tool, const QString& milestone, const QString& blurb,
                            QWidget* parent = nullptr);
};

} // namespace drpdf
