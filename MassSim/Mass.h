#pragma once

#include <QVector2D>
#include <QGraphicsItem>
#include <vector>


class Mass: public QGraphicsItem
{
public:
    Mass();

    void reset(const QVector2D  & position,
               const QVector2D  & velocity,
               const double     angle,
               const double     angularSpeed,
               const double     mass,
               const double     radius);

    QRectF boundingRect() const override;

    void paint(QPainter * painter,
               QStyleOptionGraphicsItem const * option,
               QWidget * widget) override;

    const QVector2D & position()     const { return _position; }
    const QVector2D & velocity()     const { return _velocity; }
    double    angle()        const { return _angle; }
    double    angular_speed() const { return _angular_speed; }
    double    mass()         const { return _mass; }
    double    radius()       const { return _radius; }

    void set_next_state(QVector2D const & new_pos,
                        QVector2D const & new_vel)
    {
        _next_pos = new_pos;
        _next_vel = new_vel;
    }

    void update(double const delta_time);

    void clear_collisions() { _collisions.clear(); }
    void add_collision(Mass * mass) { _collisions.push_back(mass); }
    std::vector<Mass *> & collisions() { return _collisions; }


private:
    QVector2D _position;
    QVector2D _velocity;

    QVector2D _next_pos;
    QVector2D _next_vel;

    double _angle;
    double _angular_speed;

    double _mass;
    double _radius;

    std::vector<Mass *> _collisions;
};
