#include "model/composition_tests.h"
#include "model/property/numeric_property_tests.h"
#include "model/property/property_descriptor_tests.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(FILES);
    QCoreApplication app(argc, argv);
    using namespace inae::model;

    int status = 0;
    status |= QTest::qExec(new PropertyDescriptorTests, argc, argv);
    status |= QTest::qExec(new PrimitivePropertyTests, argc, argv);
    status |= QTest::qExec(new CompositionTests, argc, argv);
    // status |= QTest::qExec(new TestObject, argc, argv);
    // status |= QTest::qExec(new ..., argc, argv);

    return status;
}
