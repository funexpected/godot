// MIT License

// Copyright (c) 2021 Yakov Borevich, Funexpected LLC

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#ifdef MODULE_ANIMATION_STATE_ENABLED

#include "animation_state_nodes.h"
#include "core/message_queue.h"
#include "scene/gui/control.h"
#ifdef TOOLS_ENABLED
#include "editor/plugins/animation_tree_editor_plugin.h"
#endif


/*
 *          Empty Frame
 */

float AnimationNodeEmpty::process(float p_time, bool p_seek) {
    return 0.0;
}

/*
 *          Delay Frame
 */


void AnimationNodeDelay::set_min_delay(float p_delay) {
    min_delay = MAX(0, MIN(max_delay, p_delay));
}

float AnimationNodeDelay::get_min_delay() const {
    return min_delay;
}

void AnimationNodeDelay::set_max_delay(float p_delay) {
    max_delay = MAX(min_delay, p_delay);
}

float AnimationNodeDelay::get_max_delay() const {
    return max_delay;
}

void AnimationNodeDelay::get_parameter_list(List<PropertyInfo> *r_list) const {
	r_list->push_back(PropertyInfo(Variant::REAL, time, PROPERTY_HINT_NONE, "", 0));
    r_list->push_back(PropertyInfo(Variant::REAL, delay, PROPERTY_HINT_NONE, "", 0));
}

float AnimationNodeDelay::process(float p_time, bool p_seek) {
    float delay = get_parameter(this->delay);
    float time = get_parameter(this->time);
    if (p_seek) {
		time = p_time;
        delay = Math::random(min_delay, max_delay);
        set_parameter(this->delay, delay);
	} else {
		time = MAX(0, time + p_time);
	}
    
    set_parameter(this->time, time);
    return MAX(0, delay - time);
}

void AnimationNodeDelay::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_min_delay", "p_delay"), &AnimationNodeDelay::set_min_delay);
    ClassDB::bind_method(D_METHOD("get_min_delay"), &AnimationNodeDelay::get_min_delay);
    ClassDB::bind_method(D_METHOD("set_max_delay", "p_delay"), &AnimationNodeDelay::set_max_delay);
    ClassDB::bind_method(D_METHOD("get_max_delay"), &AnimationNodeDelay::get_max_delay);

    ADD_PROPERTY(PropertyInfo(Variant::REAL, "min_delay", PROPERTY_HINT_EXP_RANGE, "0.0,10.0,0.1,or_greater"), "set_min_delay", "get_min_delay");
    ADD_PROPERTY(PropertyInfo(Variant::REAL, "max_delay", PROPERTY_HINT_EXP_RANGE, "0.0,10.0,0.1,or_greater"), "set_max_delay", "get_max_delay");
}

AnimationNodeDelay::AnimationNodeDelay() {
    min_delay = 0.0;
    max_delay = 1.0;
    delay = StringName("delay");
    time = StringName("time");
}

/*
 *          State Update
 */

bool AnimationNodeStateUpdate::_set(const StringName &p_name, const Variant &p_value) {
    String name = p_name;
    if (p_name == "action/edit") {
        String value = p_value;
        if (value.begins_with("Add ")) {
            state_update[value.replace_first("Add ", "")] = Variant();
        } else if (value.begins_with("Remove ")) {
            state_update.erase(value.replace_first("Remove ", ""));
        }
        property_list_changed_notify();
        return true;
    } else if (name.begins_with("update/")) {
        // print_line(String("_set state update ") + p_name + " " + p_value));
        state_update[name.replace_first("update/", "")] = p_value;
        return true;
    }
    return false;
    
}

bool AnimationNodeStateUpdate::_get(const StringName &p_name, Variant &r_ret) const {
    String name = p_name;
    if (name.begins_with("update/")) {
        String prop_name = name.replace_first("update/", "");
        if (state_update.has(prop_name)) {
            r_ret = state_update[prop_name];
            return true;
        }
    }
    return false;
}

void AnimationNodeStateUpdate::_default_property_list(List<PropertyInfo> *p_list) const {
    for (int i=0; i<state_update.size(); i++) {
        String prop_name = state_update.get_key_at_index(i);
        Variant prop_value = state_update.get_value_at_index(i);
        p_list->push_back(PropertyInfo(prop_value.get_type(), String("update/") + prop_name));
    }
}


void AnimationNodeStateUpdate::_get_property_list(List<PropertyInfo> *p_list) const {
    Node* target;
#ifdef TOOLS_ENABLED
    AnimationTreeEditor *editor = AnimationTreeEditor::get_singleton();
    if (!editor || !editor->is_visible()) return _default_property_list(p_list);

    AnimationTree *tree = Object::cast_to<AnimationTree>(editor->get_tree());
    if (!tree || !tree->has_node(tree->get_animation_player())) return _default_property_list(p_list);
    
    AnimationPlayer* ap = Object::cast_to<AnimationPlayer>(tree->get_node(tree->get_animation_player()));
    if (!ap) return _default_property_list(p_list);

    target = Object::cast_to<Node>(ap->get_node(ap->get_root()));
    if (!target) return _default_property_list(p_list);
#else
    return;
#endif

    HashMap<String, PropertyInfo> state_props;
    List<PropertyInfo> state_props_list;
    target->get_property_list(&state_props_list);
    Array defined_prop_names = state_update.keys();
    Dictionary default_values;

    for (List<PropertyInfo>::Element *E = state_props_list.front(); E; E = E->next()) {
        if (!E->get().name.begins_with("state/")) {
            continue;
        }
        String prop_name = E->get().name.substr(strlen("state/"));
        PropertyInfo prop_info = E->get();
        state_props.set(prop_name, prop_info);
        Variant default_value;
        if (prop_info.type == Variant::STRING) {
            default_value = prop_info.hint_string.split(",")[0];
        } else {
            Variant::CallError ce;
            default_value = Variant::construct(prop_info.type, NULL, 0, ce);
        }
        default_values[prop_name] = default_value;
    }

    
    for (int i=0; i<defined_prop_names.size(); i++) {
        String prop_name = defined_prop_names[i];
        if (!state_props.has(prop_name)) {
            continue;
        }
        PropertyInfo prop_info = state_props[prop_name];
        p_list->push_back(PropertyInfo(prop_info.type, String("update/") + prop_name, prop_info.hint, prop_info.hint_string));
    }

    Vector<String> actions;
    for (List<PropertyInfo>::Element *E = state_props_list.front(); E; E = E->next()) {
        if (!E->get().name.begins_with("state/")) {
            continue;
        }
        String prop_name = E->get().name.substr(strlen("state/"));
        if (!state_props.has(prop_name)) {
            continue;
        }
        if (defined_prop_names.has(prop_name)) {
            actions.push_back(String("Remove ") + prop_name);
        } else {
            actions.push_back(String("Add ") + prop_name);
        }
    }
    actions.sort();
    if (actions.size() > 0) {
        String hint = String("[select],") + String(",").join(actions);
        p_list->push_back(PropertyInfo(Variant::STRING, "action/edit", PROPERTY_HINT_ENUM, hint, PROPERTY_USAGE_EDITOR));
    }

    MessageQueue::get_singleton()->push_call(get_instance_id(), "_set_default_property_values", default_values);
}

void AnimationNodeStateUpdate::_bind_methods() {
    ClassDB::bind_method(D_METHOD("rename_state_property", "from", "to"), &AnimationNodeStateUpdate::rename_state_property);
    ClassDB::bind_method(D_METHOD("_set_default_property_values"), &AnimationNodeStateUpdate::_set_default_property_values);
}

void AnimationNodeStateUpdate::_set_default_property_values(Dictionary default_values) {
    Array defined_values = state_update.keys();
    for (int i=0; i<defined_values.size(); i++) {
        String prop_name = defined_values[i];
        if (!default_values.has(prop_name)) {
            continue;
        }
        Variant value = state_update[prop_name];
        if (value == Variant()) {
            state_update[prop_name] = default_values[prop_name];
        }
    }
}

void AnimationNodeStateUpdate::get_parameter_list(List<PropertyInfo> *r_list) const {

}

void AnimationNodeStateUpdate::rename_state_property(const String &p_from, const String &p_to) {
    if (state_update.has(p_from)) {
        Variant value = state_update[p_from];
        state_update.erase(p_from);
        state_update[p_to] = value;
    }
}

String AnimationNodeStateUpdate::get_caption() const {
	return "StateUpdate";
}

float AnimationNodeStateUpdate::process(float p_time, bool p_seek) {
	AnimationPlayer *ap = state->player;
	ERR_FAIL_COND_V(!ap, 0);
    Node *target = Object::cast_to<Node>(ap->get_node(ap->get_root()));
    ERR_FAIL_COND_V(!target, 0);
    for (int i=0; i<state_update.size(); i++) {
        target->set(String("state/") + state_update.get_key_at_index(i), state_update.get_value_at_index(i));
    }

    return 0.0;
}

AnimationNodeStateUpdate::AnimationNodeStateUpdate() {
	time = "time";
}

#endif