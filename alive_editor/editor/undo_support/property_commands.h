#ifndef PROPERTYCOMMANDS_H
#define PROPERTYCOMMANDS_H

#include <core/model/property/dynamic_property.h>
#include <core/model/property/property_type.h>
#include <editor/document/document.h>
#include <editor/undo_support/command.h>
#include <editor/undo_support/command_enums.h>
#include <editor/undo_support/document_undo_manager.h>

namespace alive {

class PropertyCommand : public Command
{
public:
    PropertyCommand(model::Property *property,
                    CommandType type,
                    const std::string &text = std::string())
        : Command(type, text)
        , m_property(property)
    {}

    const model::Property *property() const { return m_property; }

protected:
    void update_document()
    {
        if (m_document) {
            m_document->undo_manager()->updated_item(m_property, m_roles);
        }
    }
    model::Property *m_property;
};

template<typename ValueType>
class SetPropertyValueCommand : public PropertyCommand
{
public:
    SetPropertyValueCommand(
        model::DynamicProperty<ValueType> *property,
        KeyFrameTime time,
        ValueType &new_value,
        std::function<void(const ValueType &)> f = [](const ValueType &) {},
        const std::string &text = std::string())
        : PropertyCommand(property, CommandType::e_PropertySetValue, text)
        , m_dynamic_property(property)
        , m_new_value(new_value)
        , m_time(time)
        , m_function(f)
    {}

    const ValueType &new_value() const { return m_new_value; }
    void redo() override
    {
        m_old_value = m_dynamic_property->get_value();
        m_keyframe = m_dynamic_property->set_value(m_new_value, m_time);
        update_document();
        m_function(m_new_value);
    }

    void undo() override
    {
        if (m_keyframe) {
            m_dynamic_property->remove_keyframe(m_keyframe);
        } else {
            m_dynamic_property->set_value(m_old_value, m_time);
        }
        update_document();
        m_function(m_old_value);
    }

    bool merge_with(const Command *other) override
    {
        const PropertyCommand *other_command = static_cast<const PropertyCommand *>(other);
        if (m_property == other_command->property()) {
            auto *other_set_prop_command = static_cast<const SetPropertyValueCommand<ValueType> *>(
                other);
            if (m_keyframe == other_set_prop_command->m_keyframe) {
                m_new_value = other_set_prop_command->new_value();
                return true;
            }
        }
        return false;
    }

private:
    model::DynamicProperty<ValueType> *m_dynamic_property;
    ValueType m_old_value;
    ValueType m_new_value;
    KeyFrameTime m_time;
    std::function<void(const ValueType &)> m_function;
    model::KeyFrame *m_keyframe = nullptr;
};

class SetPropertyExpressionCommand : public PropertyCommand
{
public:
    SetPropertyExpressionCommand(model::Property *property,
                                 const std::string &expression,
                                 KeyFrameTime time)
        : PropertyCommand(property, CommandType::e_PropertySetExpression)
        , m_expression(expression)
        , m_time(time)
    {}

    void redo() override
    {
        m_old_value_type = m_property->value_type();
        m_old_expression = m_property->get_expression(m_time);
        m_enabled = m_property->expression_enabled();
        m_keyframe_ptr = m_property->set_expression(m_expression, m_time);
        m_new_keyframe.reset(m_keyframe_ptr);
        update_document();
    }

    void undo() override
    {
        if (m_new_keyframe) {
            m_property->remove_keyframe(m_keyframe_ptr);
        } else {
            m_property->set_expression(m_old_expression, m_time);
            m_property->enable_expression(m_enabled);
        }
        update_document();
    }

private:
    std::string m_expression;
    std::string m_old_expression;
    bool m_enabled;
    model::PropertyAnimationType m_old_value_type;
    KeyFrameTime m_time;
    model::KeyFrame *m_keyframe_ptr = nullptr;
    std::unique_ptr<model::KeyFrame> m_new_keyframe;
};

class PropertyDisableExpressionCommand : public PropertyCommand
{
public:
    PropertyDisableExpressionCommand(model::Property *property, bool disable)
        : PropertyCommand(property, CommandType::e_PropertyDisableExpression)
        , m_disable(disable)
    {}

    void redo() override
    {
        m_property->enable_expression(!m_disable);
        update_document();
    }

    void undo() override
    {
        m_property->enable_expression(m_disable);
        update_document();
    }

private:
    bool m_disable;
};

template<typename T>
Command *create_dynamic_property_change_command(model::Property *property,
                                                KeyFrameTime time,
                                                const T &value)
{
    model::DynamicProperty<T> *d_prop = static_cast<model::DynamicProperty<T> *>(property);
    const T &old_val = d_prop->get_value();
    T new_val = value;
    d_prop->validate(new_val);
    if (new_val != old_val) {
        return new SetPropertyValueCommand<T>(d_prop, time, new_val);
    }
    return nullptr;
}

template<typename T>
Command *create_dynamic_property_change_command(
    model::DynamicProperty<T> *d_prop,
    KeyFrameTime time,
    const T &value,
    std::function<void(const T &)> f = [](const T &) {})
{
    const T &old_val = d_prop->get_value();
    T new_val = value;
    d_prop->validate(new_val);
    if (new_val != old_val) {
        return new SetPropertyValueCommand<T>(d_prop, time, new_val, f);
    }
    return nullptr;
}

} // namespace alive

#endif // PROPERTYCOMMANDS_H
