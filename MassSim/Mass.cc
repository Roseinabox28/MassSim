#include <iostream>
#include <QPainter>
#include <math.h>
#include "Mass.h"

Mass::Mass()
{
}

void Mass::reset(const QVector2D  & position,
                 const QVector2D  & velocity,
                 const double     angle,
                 const double     angular_speed,
                 const double     mass,
                 const double     radius)
{
    _position = position;
    _velocity = velocity;
    _angle = angle;
    _angular_speed = angular_speed;
    _mass = mass;
    _radius = radius;

    setRotation(_angle);
    setPos(_position.x(),
           _position.y());
}

QRectF Mass::boundingRect() const
{
    qreal pen_width = 1;
    return QRect(-_radius - pen_width/2,
                 -_radius - pen_width/2,
                 2*_radius + pen_width,
                 2*_radius + pen_width);
}

void Mass::paint(QPainter * painter,
                 QStyleOptionGraphicsItem const *,
                 QWidget *)
{
    QPen pen(Qt::white);
    painter->setPen(pen);
    QRectF rect = boundingRect();
    painter->drawEllipse(rect);
    QVector2D direction(cos(_angle), sin(_angle));
    auto p1 = (_radius*direction);
    auto p2 = (-_radius*direction);
    painter->drawLine(p1.toPointF(), p2.toPointF());
#if 0
    for(auto pos: posq)
    {
        painter->drawPoint(pos.toPointF());
    }
#endif
}

void Mass::update(double const delta_time,
                  QVector2D const & force)
{
    posq.push_front(_position);
    if (posq.size() >= 10)
    {
        posq.pop_back();
    }
    _position += delta_time*_velocity;
    _velocity += delta_time*force/_mass;

    _angle    += delta_time*_angular_speed;
    _angle     = fmod(_angle, M_PI);

    setRotation(_angle);
    setPos(_position.x(),
           _position.y());
}
