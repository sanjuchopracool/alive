#ifndef NUMERICPROPERTYTESTS_H
#define NUMERICPROPERTYTESTS_H

#include <QTest>

#include "model/property/object.h"
#include "model/property/property_macros.h"
#include <model/transform.h>

namespace inae::model {
class TransformObj : public QObject
{
    Q_OBJECT
    Q_PROPERTY(const Transform *transform READ get_transform)
public:
    TransformObj(QObject *parent = nullptr);

    const Transform *get_transform() { return &m_transform; }

private:
    Transform m_transform;
};

class PrimitivePropertyTests : public QObject
{
    Q_OBJECT
private slots:
    void basic_tests();
    void script_test();
};

} // namespace inae::model

Q_DECLARE_METATYPE(inae::model::TransformObj)

#endif // NUMERICPROPERTYTESTS_H
