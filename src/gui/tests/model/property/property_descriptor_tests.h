#ifndef PROPERTYDESCRIPTORTESTS_H
#define PROPERTYDESCRIPTORTESTS_H

#include <QTest>

namespace inae::model {
class PropertyDescriptorTests : public QObject
{
    Q_OBJECT
private slots:
    void pass();
};

} // namespace inae::model

#endif // PROPERTYDESCRIPTORTESTS_H
