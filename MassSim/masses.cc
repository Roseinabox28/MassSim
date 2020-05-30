
#include <iostream>
#include <QApplication>
#include "MassWindow.h"

int main( int argc, char **argv )
{
    auto seed = clock();
    std::cout << "Random seed = " << seed << std::endl;
    //srand48(seed);

    QApplication app(argc, argv);

    MassWindow window;

    for (int i = 0; i < 200; i++)
    {
        window.add_mass();
    }

    window.show();
 
    return app.exec();
}
