#include "layer_color_model.h"

#include <random>
#include <vector>

namespace alive::gui {

namespace {
static LayerColorModel *the_instance = nullptr;
std::mt19937 g_rng;
std::uniform_int_distribution<uint32_t> g_color_dist;
} // namespace

struct LayerColorsData
{
    std::vector<EditorLayerColorValue> colors = {
        {"none", {102, 102, 102}},
        {"Pink", {"#ff69b4"}},
        {"Purple", {"#640D5F"}},
        {"Blue", {"#242BCE"}},
        {"Yellow", {"#FEF481"}},
        {"Red", {"#C63C51"}},
        {"Green", {"#0DAC56"}},
        {"White", {"#F0F0F0"}},
        {"Brown", {"#705C53"}},
        {"Cyan", {"#009BAB"}},
        {"Dark Khaki	", {"#BDB76B"}},
        {"Orange", {"#FF8343"}},
        {"Peach", {"#FFC0CB"}},
        {"Light Green", {"#D8E9A8"}},
    };
};

EditorLayerColorValue editor_color_at_index(int index)
{
    return LayerColorModel::instance()->editor_color_at_index(index);
}

int editor_color_max_index()
{
    return LayerColorModel::instance()->rowCount(QModelIndex());
}

LayerColorModel::~LayerColorModel() {}

int LayerColorModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_d->colors.size();
}

QVariant LayerColorModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        return m_d->colors[index.row()].name;
        break;
    case Qt::DecorationRole:
        return m_d->colors[index.row()].color;
        break;
    default:
        break;
    }
    return {};
}

void LayerColorModel::init()
{
    if (!the_instance) {
        the_instance = new LayerColorModel();
    }
}

void LayerColorModel::deinit(bool qml)
{
    if (!qml && the_instance) {
        delete the_instance;
        the_instance = nullptr;
    }
}

LayerColorModel *LayerColorModel::instance()
{
    return the_instance;
}

EditorLayerColorValue LayerColorModel::editor_color_at_index(int index)
{
    if (index >= m_d->colors.size())
        index = 0;
    return m_d->colors[index];
}

int LayerColorModel::count() const
{
    return m_d->colors.size();
}

int LayerColorModel::get_random_color_index() const
{
    return g_color_dist(g_rng);
}

LayerColorModel::LayerColorModel(QObject *parent)
    : QAbstractListModel{parent}
    , m_d(std::make_unique<LayerColorsData>())
{
    g_rng.seed(std::rand());
    g_color_dist = std::uniform_int_distribution<uint32_t>(1, m_d->colors.size() - 1);
}

} // namespace alive::gui
