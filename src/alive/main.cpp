#include "application.h"

int main(int argc, char *argv[])
{
    alive::Application app(argc, argv);
    return app.execute();
}
