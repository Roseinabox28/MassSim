
#include <iostream>
#include <unistd.h>
#include <math.h>
#include <QApplication>
#include "MassWindow.h"

int main(int argc,
         char * argv[])
{
    bool benchmark = false;
    double dt = 40;
    double graphics_interval = 0.2;
    int num_masses = 1000;
    int num_threads = 1;
    auto seed = clock();

    int opt;
    while ((opt = getopt(argc, argv, "bd:i:n:s:t:")) != -1)
    {
        switch (opt)
        {
        case 'b':
            benchmark = true;
            break;

        case 'd':
            dt = atof(optarg);
            break;

        case 'i':
            graphics_interval = atof(optarg);
            break;

        case 'n':
            num_masses = atoi(optarg);
            break;

        case 's':
            seed = atoi(optarg);
            break;

        case 't':
            num_threads = atoi(optarg);
            break;

        default: /* '?' */
            std::cerr << "USAGE: " << argv[0]
                      << " [-d <delta time (s)>]"
                      << " [-i <graphics interval (s)>]"
                      << " [-n <number of masses>]"
                      << " [-s <random seed>]"
                      << " [-t <num threads>]" << std::endl;
            return 1;
        }
    }

    std::cout << "Random seed = " << seed << std::endl;
    //srand48(seed);

    QApplication app(argc, argv);

    MassWindow window(dt, graphics_interval, num_threads);

    auto * sun = new Mass();
    sun->reset(QVector2D(0.0, 0.0),
               QVector2D(0.0, 0.0),
               0.0,
               0.0,
               1e3,
               10);
    window.add_mass(sun);

    if (false)
    {
        // test integrator precision
        auto * test = new Mass();
        QVector2D pos(20.0, 0.0);
        double r = pos.length();
        double v = sqrt(MassWindow::gravity_constant*sun->mass()/r);
        QVector2D vel = QVector2D(0.0, -v);
        test->reset(pos,
                    vel,
                    0.0,
                    0.0,
                    1e-4,
                    1);
        window.add_mass(test);

#if 1
        double time = 1000;
        for (int i = 0; i < int(time/dt); ++i)
        {
            window.advance();
        }

        std::cout << test->position().x() << ", " << test->position().y() << std::endl;

        return 0;
#endif
    }

    for (int i = 0; i < num_masses; ++i)
    {
        window.add_init_mass();
    }

    if (benchmark)
    {
        double time = 1000;
        for (int i = 0; i < int(time/dt); ++i)
        {
            window.advance();
        }

        window.print_stats();
        return 0;
    }

    window.show();

    auto status = app.exec();

    window.print_stats();

    return status;
}
