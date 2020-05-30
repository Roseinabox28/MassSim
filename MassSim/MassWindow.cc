
#include <iostream>
#include <QGraphicsEllipseItem>
#include <math.h>
#include <stdlib.h>
#include "MassWindow.h"
#include "ui_MassWindow.h"

double drand48()
{
    return (rand() / (RAND_MAX + 1.0));
}


MassWindow::MassWindow(double dt,
                       double graphics_interval,
                       size_t num_threads,
                       QWidget * parent):
    QDialog(parent),
    ui(new Ui::MassWindow),
    _barrier(num_threads + 1),
    _dt(dt)
{
    ui->setupUi(this);

    scene = new QGraphicsScene(this);

    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHints(QPainter::Antialiasing);
    ui->graphicsView->setBackgroundBrush(QBrush(Qt::black, Qt::SolidPattern));

    int width = 1000;
    int height = 800;
    scene->setSceneRect(-width*0.5, -height*0.5, width, height);

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(advance()));
    timer->start(graphics_interval*1e3);

    resize(width*1.1, height*1.1);
}

void MassWindow::_init_mass(Mass * m,
                            bool at_edges) const
{
    double mass = 0.001;
    double radius = 0.3;

    QVector2D position;

    bool good_pos = false;
    while (!good_pos)
    {
        if (at_edges)
        {
            switch(int(4*drand48()))
            {
            case 0:
                position = QVector2D(radius,
                                     radius + (scene->sceneRect().height() - 2*radius)*drand48());
                break;
            case 1:
                position = QVector2D(radius + (scene->sceneRect().width() - 2*radius),
                                     radius + (scene->sceneRect().height() - 2*radius)*drand48());
                break;
            case 2:
                position = QVector2D(radius + (scene->sceneRect().width() - 2*radius)*drand48(),
                                     radius);
                break;
            case 3:
                position = QVector2D(radius + (scene->sceneRect().width() - 2*radius)*drand48(),
                                     radius + (scene->sceneRect().height() - 2*radius));
                break;
            }
        }
        else
        {
            position = QVector2D(radius + (scene->sceneRect().width() - 2*radius)*drand48(),
                                 radius + (scene->sceneRect().height() - 2*radius)*drand48());
        }

        position -= QVector2D(scene->sceneRect().width()/2, scene->sceneRect().height()/2);

        good_pos = true;
        for (auto m2 : _masses)
        {
            double dist = ((m2->position() - position).length() -
                           m2->radius());
            if (dist < 1.1*radius)
            {
                good_pos = false;
                break;
            }
        }
    }

    double r = position.length();
    double v = 0.9*sqrt(gravity_constant*_masses[0]->mass()/r);
    QVector2D velocity = v*QVector2D(position.y(), -position.x())/r;
    //QVector2D velocity = 0*0.0001*QVector2D(position.y(), -position.x());

    m->reset(position,
             velocity,
             M_PI*drand48(),
             0.0,
             mass,
             radius);
}

void MassWindow::add_init_mass()
{
    auto m = new Mass();
    _init_mass(m, false);
    add_mass(m);
}

void MassWindow::add_mass(Mass * m)
{
    _masses.push_back(m);
    scene->addItem(m);
    //std::cout << "ADD " << m << std::endl;
}

inline double delta_time(const struct timespec &start,
                         const struct timespec &end)
{
   return (end.tv_sec - start.tv_sec +
	   (end.tv_nsec - start.tv_nsec)*1e-9);
}

inline
double cross(QVector2D const & a,
             QVector2D const & b)
{
    return (a.x()*b.y() - a.y()*b.x());
}

QVector2D MassWindow::_compute_accel(Mass ** mass1,
                                     QVector2D const & pos,
                                     bool do_collisions)
{
    Mass * m1 = *mass1;
    QVector2D total_force(0.0, 0.0);
    for (auto & m2 : _masses)
    {
        if (m2 == m1)
        {
            continue;
        }

        auto diff = (m2->position() - pos);
        double dist = diff.length();

        if (do_collisions)
        {
            double r1 = (m1->radius() > 1) ? m1->radius() : 1;
            double r2 = (m2->radius() > 1) ? m2->radius() : 1;

            if (dist <= (r1 + r2))
            {
                m1->add_collision(m2);
            }
        }

        auto force = diff*(gravity_constant*m1->mass()*m2->mass()/
                           (dist*dist*dist));
        total_force += force;
    }
    return total_force/m1->mass();
}

void MassWindow::advance()
{
    struct timespec start_time;
    clock_gettime(CLOCK_REALTIME, &start_time);

    // Compute inter-mass forces.
    for (auto & m1 : _masses)
    {
        m1->clear_collisions();
        auto const & p1 = m1->position();
        auto const & v1 = m1->velocity();
        auto a1 = _compute_accel(&m1, p1, true);

        auto p2 = p1 + 0.5*v1*_dt;
        auto v2 = v1 + 0.5*a1*_dt;
        auto a2 = _compute_accel(&m1, p2, false);

        auto p3 = p1 + 0.5*v2*_dt;
        auto v3 = v1 + 0.5*a2*_dt;
        auto a3 = _compute_accel(&m1, p3, false);

        auto p4 = p1 + v3*_dt;
        auto v4 = v1 + a3*_dt;
        auto a4 = _compute_accel(&m1, p4, false);

        auto pos = p1 + (_dt/6.0)*(v1 + 2*v2 + 2*v3 + v4);
        auto vel = v1 + (_dt/6.0)*(a1 + 2*a2 + 2*a3 + a4);

        m1->set_next_state(pos, vel);
    }

    struct timespec end_time;
    clock_gettime(CLOCK_REALTIME, &end_time);
    auto cycle_time = delta_time(start_time, end_time);
    _sum_cycle_time += cycle_time;
    if (cycle_time > _max_cycle_time)
    {
        _max_cycle_time = cycle_time;
    }
    _num_cycles++;

    double max_speed_sqrd = 0.0;

    _collision_pairs.clear();

    for (auto & m1 : _masses)
    {
        m1->update(_dt);
        for (auto & m2 : m1->collisions())
        {
            _collision_pairs.push_back(std::make_pair(&m1, &m2));
        }

        auto speed_sqrd = QVector2D::dotProduct(m1->velocity(), m1->velocity());
        if (speed_sqrd > max_speed_sqrd)
        {
            max_speed_sqrd = speed_sqrd;
        }
    }

#if 0
    if (max_speed_sqrd > 0.0)
    {
        const double max_delta_position = 10.0;
        _dt = max_delta_position/sqrt(max_speed_sqrd);
        //std::cout << "dt " << _dt << std::endl;
    }
#endif

    if (_collision_pairs.empty())
    {
        return;
    }

    //std::cout << "Collisions:" << std::endl;
    for (size_t i = 0; i < _collision_pairs.size(); ++i)
    {
        auto & m1 = *_collision_pairs[i].first;
        auto & m2 = *_collision_pairs[i].second;

        bool skip = false;
        for (size_t j = 0; j < i; j++)
        {
            if ((m1 == *_collision_pairs[j].second) &&
                (m2 == *_collision_pairs[j].first))
            {
                skip = true;
                break;
            }
            if ((m1 == *_collision_pairs[j].first) ||
                (m1 == *_collision_pairs[j].second) ||
                (m2 == *_collision_pairs[j].first) ||
                (m2 == *_collision_pairs[j].second))
            {
                //std::cout << "multiple" << std::endl;
                break;
            }
        }
        if (skip)
        {
            continue;
        }
        //std::cout << "  pair " << m1 << ", " << m2 << std::endl;

        double totalMass = m1->mass() + m2->mass();
        double mass_fraction = m2->mass()/totalMass;

        auto diff = (m2->position() - m1->position());
        auto position = (m1->position() + diff*mass_fraction);

        auto momentum1 = m1->mass()*m1->velocity();
        auto momentum2 = m2->mass()*m2->velocity();

        auto velocity = (momentum1 + momentum2)/totalMass;

        double angle = (m1->angle() +
                        (m2->angle() - m1->angle())*mass_fraction);

        // Pretend we're adding sphere volumes.
        double radius = cbrt(m1->radius()*m1->radius()*m1->radius() +
                             m2->radius()*m2->radius()*m2->radius());

        double inertia1 = 0.5*m1->mass()*m1->radius()*m1->radius();
        double inertia2 = 0.5*m2->mass()*m2->radius()*m2->radius();
        double total_inertia = 0.5*totalMass*radius*radius;

        auto r1 = (m1->position() - position);
        auto r2 = (m2->position() - position);

        double angular_speed = (cross(r1, momentum1) +
                                cross(r2, momentum2) +
                                inertia1*m1->angular_speed() +
                                inertia2*m2->angular_speed())/total_inertia;

        m1->reset(position,
                  velocity,
                  angle,
                  angular_speed,
                  totalMass,
                  radius);

        _init_mass(m2, true);
    }
}

void MassWindow::print_stats() const
{
    std::cout << "Average cycle time: " << 1e3*_sum_cycle_time/_num_cycles << " ms" << std::endl;
    std::cout << "Max cycle time: " << 1e3*_max_cycle_time << " ms" << std::endl;
}
