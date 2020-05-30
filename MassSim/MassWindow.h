#pragma once

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <vector>
#include "Mass.h"
#include "ThreadBarrier.h"

namespace Ui
{
    class MassWindow;
}

class MassWindow: public QDialog
{
    Q_OBJECT

public:
    static constexpr double gravity_constant = 1e-3;

    explicit MassWindow(double dt,
                        double graphics_interval,
                        size_t num_threads,
                        QWidget * parent = 0);

    void add_init_mass();
    void add_mass(Mass * m);

    void print_stats() const;

public slots:
    void advance();

private:
    Ui::MassWindow * ui;
    QGraphicsScene * scene;
    QTimer * timer;

    ThreadBarrier _barrier;

    std::vector<Mass *> _masses;
    std::vector<std::pair<Mass **, Mass **>> _collision_pairs;
    double _dt;

    double _sum_cycle_time = 0;
    double _max_cycle_time = 0;
    size_t _num_cycles = 0;

    void _init_mass(Mass * m,
                    bool at_edges) const;

    QVector2D _compute_accel(Mass ** m1,
                             QVector2D const & pos,
                             bool do_collisions);
};
