#pragma once

#include <QVector2D>
#include <QGraphicsItem>
#include <deque>

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

    void update(double const delta_time,
                QVector2D const & force);

private:
    std::deque <QVector2D> posq;
    QVector2D _position;
    QVector2D _velocity;

    double _angle;
    double _angular_speed;

    double _mass;
    double _radius;
};

