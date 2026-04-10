#include "property_descriptor_tests.h"
#include <model/property/property_descriptor.h>

namespace inae::model {
void PropertyDescriptorTests::pass()
{
    PropertyDescriptor pd_int("Int", PropertyDescriptor::e_Int, PropertyDescriptor::e_NoFlags);
    QCOMPARE("Int", pd_int.get_name());
    QCOMPARE(PropertyDescriptor::e_Int, pd_int.type());
    QCOMPARE(PropertyDescriptor::e_NoFlags, pd_int.flags());

    int flags = PropertyDescriptor::e_Animated | PropertyDescriptor::e_ReadOnly;
    PropertyDescriptor pd_float("Float", PropertyDescriptor::e_Float, flags);
    QCOMPARE("Float", pd_float.get_name());
    QCOMPARE(PropertyDescriptor::e_Float, pd_float.type());
    QCOMPARE(flags, pd_float.flags());
}
} // namespace inae::model
