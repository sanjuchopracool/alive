#ifndef COMPOSITIONTESTS_H
#define COMPOSITIONTESTS_H

#include <QTest>
namespace inae::model {

class CompositionTests : public QObject
{
    Q_OBJECT
public:
    explicit CompositionTests(QObject *parent = nullptr);

private slots:
    void parse_test();
};
} // namespace inae::model

#endif // COMPOSITIONTESTS_H
