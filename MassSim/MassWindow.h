#pragma once

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <vector>
#include "Mass.h"

namespace Ui
{
    class MassWindow;
}

class MassWindow: public QDialog
{
    Q_OBJECT

public:
    explicit MassWindow(QWidget * parent = 0);

    void add_mass();

public slots:
    void advance();

private:
    Ui::MassWindow * ui;
    QGraphicsScene * scene;
    QTimer * timer;

    std::vector<Mass *> _masses;
    std::vector<std::pair<Mass **, Mass **>> _collision_pairs;
    double _dt;

    void _init_mass(Mass * m,
                    bool at_edges);
};
