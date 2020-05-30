
#include <iostream>
#include <QGraphicsEllipseItem>
#include "MassWindow.h"
#include "ui_MassWindow.h"


MassWindow::MassWindow(QWidget * parent):
    QDialog(parent),
    ui(new Ui::MassWindow),
    _dt(1)
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
    timer->start(200);

    resize(width*1.1, height*1.1);
}

void MassWindow::_init_mass(Mass * m,
                            bool at_edges)
{
    double mass = 1e3;
    double radius = 1;

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

    QVector2D velocity = 0.001*QVector2D(position.y(), -position.x());

    m->reset(position,
             velocity,
             M_PI*drand48(),
             0.0,
             mass,
             radius);
}

void MassWindow::add_mass()
{
    auto m = new Mass();
    _init_mass(m, false);
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

void MassWindow::advance()
{
    double max_speed_sqrd = 0.0;

    struct timespec start_time;
    clock_gettime(CLOCK_REALTIME, &start_time);

    // Compute inter-mass forces.
    _collision_pairs.clear();
    for (auto & m1 : _masses)
    {
        QVector2D total_force(0.0, 0.0);
        for (auto & m2 : _masses)
        {
            if (m2 == m1)
            {
                continue;
            }

            auto diff = (m2->position() - m1->position());
            double dist = diff.length();

            double r1 = (m1->radius() > 1) ? m1->radius() : 1;
            double r2 = (m2->radius() > 1) ? m2->radius() : 1;
            if ((dist <= (r1 + r2)))
            {
                _collision_pairs.push_back(std::make_pair(&m1, &m2));
            }
            else
            {
                auto force = diff*(1e-3*m1->mass()*m2->mass()/
                                   (dist*dist*dist));
                total_force += force;
            }
        }

        if (_collision_pairs.empty())
        {
            m1->update(_dt, total_force);
        }

        auto vel = m1->velocity().length();
        auto speed_sqrd = vel*vel;
        if (speed_sqrd > max_speed_sqrd)
        {
            max_speed_sqrd = speed_sqrd;
        }
    }

    struct timespec update_time;
    clock_gettime(CLOCK_REALTIME, &update_time);

    if (false && max_speed_sqrd > 0.0)
    {
        const double max_delta_position = 10.0;
        _dt = max_delta_position/sqrt(max_speed_sqrd);
        std::cout << "dt " << _dt << std::endl;
    }

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
                std::cout << "multiple" << std::endl;
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

    struct timespec end_time;
    clock_gettime(CLOCK_REALTIME, &end_time);
    std::cout << "time " << delta_time(start_time, update_time)
              << ", " << delta_time(start_time, end_time) << std::endl;
}
