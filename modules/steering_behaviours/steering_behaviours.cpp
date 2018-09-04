
#include "steering_behaviours.h"
#include "core/engine.h"

#ifndef MAXFLOAT
#define MAXFLOAT 10000000.0
#endif

void SteeringBehaviourConfiguration::_get_property_list(List<PropertyInfo> *list) const {
    list->push_back(PropertyInfo(Variant::INT, "composition", PROPERTY_HINT_ENUM, "TRUNKATED_WEIGHTS,PRIORITY_WEIGHTS,PRIORITIZED_DITHERING"));
    list->push_back(PropertyInfo(Variant::INT, "locomotion", PROPERTY_HINT_ENUM, "DIRECT_FORCE,SMOOTH_TORGUE,GHOST_FOLLOWING"));
    list->push_back(PropertyInfo(Variant::REAL, "avoid_detect_distance"));
    list->push_back(PropertyInfo(Variant::REAL, "wall_feeler_length"));
    list->push_back(PropertyInfo(Variant::REAL, "ghost_distance"));

    Vector<String> possible_types;
    possible_types.push_back("Select...");
    for (int j=0;j<BehaviourType::SIZE;j++){
        possible_types.push_back(BehaviourTypeNames[j]);
    }
        
    for (int j=0; j<order.size(); j++){
        String type_name = BehaviourTypeNames[order[j]];
        String prefix = type_name+"/";
        list->push_back(PropertyInfo(Variant::REAL, prefix+"weight"));
        list->push_back(PropertyInfo(Variant::STRING, prefix+"move", PROPERTY_HINT_ENUM, "Select...,up,down,away", PROPERTY_USAGE_EDITOR));
        int type_idx = possible_types.find(type_name);
        if (type_idx >= 0){
            possible_types.remove(type_idx);
        }
    }
    list->push_back(PropertyInfo(Variant::STRING, "add", PROPERTY_HINT_ENUM, String(",").join(possible_types), PROPERTY_USAGE_EDITOR));
}

bool SteeringBehaviourConfiguration::_get(const String &p_name, Variant &r_ret) const {
    Vector<String> parts = p_name.split("/");

    if (parts[0] == "composition"){
        r_ret = Variant(composition);
        return true;
    }

    if (parts[0] == "locomotion"){
        r_ret = Variant(locomotion);
        return true;
    }

    if (parts[0] == "avoid_detect_distance"){
        r_ret = Variant(avoid_detect_distance);
        return true;
    }

    if (parts[0] == "wall_feeler_length"){
        r_ret = Variant(wall_feeler_length);
        return true;
    }
    if (parts[0] == "ghost_distance"){
        r_ret = Variant(ghost_distance);
        return true;
    }

    if (parts.size() == 1) return false;

    int type_id = -1;
    for (int i=0; i<BehaviourType::SIZE; i++){
        if (BehaviourTypeNames[i] == parts[0]){
            type_id = i;
            break;
        }
    }
    if (type_id < 0) return false;

    int type_idx = -1;
    for (int i=0; i<order.size(); i++){
        if (order[i] == type_id){
            type_idx = i;
            break;
        }
    }
    if (type_idx < 0) return false;

    if (parts.size()<2){
        return false;
    }

    if (parts[1]=="weight"){
        if (weights.size()<type_idx+1)
            r_ret = 0.0;
        else 
            r_ret = Variant(weights[type_idx]);
        return true;
    }
    return false;
}

bool SteeringBehaviourConfiguration::_set(const String &p_name, const Variant &p_value){
    Vector<String> parts = p_name.split("/");
    if (parts[0] == "composition"){
        composition = (BehaviourComposition)(int)p_value;
        return true;
    }

    if (parts[0] == "locomotion"){
        locomotion = (BehaviourLocomotion)(int)p_value;
        return true;
    }

    if (parts[0] == "avoid_detect_distance"){
        avoid_detect_distance = p_value;
        return true;
    }

    if (parts[0] == "wall_feeler_length"){
        wall_feeler_length = p_value;
        return true;
    }

    if (parts[0] == "ghost_distance"){
        ghost_distance = p_value;
        return true;
    }

    if (parts[0] == "add"){
        int type_idx = -1;
        for (int i=0; i<BehaviourType::SIZE; i++){
            if (BehaviourTypeNames[i]==p_value){
                type_idx = i;
                break;
            }
        }
        order.push_back(type_idx);
        weights.push_back(1.0);
        _change_notify();
        return true;
    }

    int type_id = -1;
    for (int i=0; i<BehaviourType::SIZE; i++){
        //print_line("check " + BehaviourTypeNames[i] + ", vs " + parts[0]);
        if (BehaviourTypeNames[i] == parts[0]){
            type_id = i;
            break;
        }
    }
    //print_line("found type_id: " + itos(type_id));
    if (type_id == -1) return false;

    int type_idx = -1;
    for (int i=0; i<order.size(); i++){
        if (order[i]==type_id){
            type_idx = i;
            break;
        }
    }
    if (type_idx == -1){
        type_idx = order.size();
        order.append(type_id);
        weights.append(1.0);
    }
    //print_line("name: " + p_name + ", idx: " + itos(type_idx));
    if (parts[1]=="weight"){
        //print_line("set weights " + itos(type_idx) + " to " + p_value + " (" + rtos((float)p_value)+")");
        weights.set(type_idx, (float)p_value);
        return true;
    }
    if (parts[1]=="move"){
        if (p_value == "up"){
            if (type_idx>0){
                float w = weights[type_idx-1];
                int t = order[type_idx-1];
                weights.set(type_idx-1, weights[type_idx]);
                weights.set(type_idx, w);
                order.set(type_idx-1, order[type_idx]);
                order.set(type_idx, t);
                _change_notify();
            }
            return true;
        } else if (p_value == "down"){
            if (type_idx<order.size()-1){
                float w = weights[type_idx+1];
                int t = order[type_idx+1];
                weights.set(type_idx+1, weights[type_idx]);
                weights.set(type_idx, w);
                order.set(type_idx+1, order[type_idx]);
                order.set(type_idx, t);
                _change_notify();
            }
            return true;
        } else {
            weights.remove(type_idx);
            order.remove(type_idx);
            _change_notify();
            return true;
        }
    }
    return false;
}

void SteeringBehaviourConfiguration::_bind_methods(){
    BIND_ENUM_CONSTANT(SEEK);
    BIND_ENUM_CONSTANT(FLEE);
    BIND_ENUM_CONSTANT(ARRIVE);
    BIND_ENUM_CONSTANT(EVADE);
    BIND_ENUM_CONSTANT(AVOID);
    BIND_ENUM_CONSTANT(WANDER);
    BIND_ENUM_CONSTANT(AVOID_WALLS);    
    BIND_ENUM_CONSTANT(FREEZE);    
    BIND_ENUM_CONSTANT(FOLLOW);    
    BIND_ENUM_CONSTANT(SIZE);

    BIND_ENUM_CONSTANT(TRUNKATED_WEIGHTS);
    BIND_ENUM_CONSTANT(PRIORITY_WEIGHTS);
    BIND_ENUM_CONSTANT(PRIORITIZED_DITHERING);

    BIND_ENUM_CONSTANT(DIRECT_FORCE);
    BIND_ENUM_CONSTANT(SMOOTH_TORGUE);
    BIND_ENUM_CONSTANT(GHOST_FOLLOWING);
}

SteeringBehaviourConfiguration::SteeringBehaviourConfiguration(){
    composition = BehaviourComposition::PRIORITY_WEIGHTS;
    locomotion = BehaviourLocomotion::DIRECT_FORCE;
    avoid_detect_distance = 100.0;
    wall_feeler_length = 100.0;
    ghost_distance = 30.0;

    
}

SteeringBehaviourConfiguration::~SteeringBehaviourConfiguration(){
    order.resize(0);
    weights.resize(0);
}



void SteeringBehaviours::process_behaviours(){
    int cc = get_child_count();
    /*
    distances.resize(cc*cc);
    for (int i=0; i<cc; i++){
        SteeringBehaviour *bi = Object::cast_to<SteeringBehaviour>(get_child(i));
        if (bi == NULL) continue;
        for (int j=0; j<cc; j++){
            SteeringBehaviour *bj = Object::cast_to<SteeringBehaviour>(get_child(j));
            if (bj == NULL) continue;
            int idx0 = i*cc+j;
            int idx1 = j*cc+i;
            float dist = bi->get_position().distance_to(bj->get_position());
            distances.set(idx0, dist);
            distances.set(idx1, dist);
        }
    }
    */

    for (int i=0; i<cc; i++){
        SteeringBehaviour *b = Object::cast_to<SteeringBehaviour>(get_child(i));
        if (b == NULL) continue;
        b->process_behaviours();
    }
}


void SteeringBehaviours::_notification(int what){
    switch (what){
        case NOTIFICATION_PROCESS:
            if (mode == SteeringBehaviours::IDLE) process_behaviours();
            break;
        case NOTIFICATION_PHYSICS_PROCESS:
            if (mode == SteeringBehaviours::FIXED) process_behaviours();
    }
}

int SteeringBehaviours::get_behaviour_mode() const { return mode; }
void SteeringBehaviours::set_behaviour_mode(int p_mode) { 
    mode = (SteeringBehaviours::BehaviourProcessMode)p_mode;
    if (Engine::get_singleton()->is_editor_hint()) return;
    switch (mode){
        case SteeringBehaviours::IDLE:
            set_process(true);
            set_physics_process(false);
            break;
        case SteeringBehaviours::FIXED:
            set_process(false);
            set_physics_process(true);
            break;
        default:
            set_process(false);
            set_physics_process(false);
            break;
    }
}

void SteeringBehaviours::_bind_methods(){
    ClassDB::bind_method(D_METHOD("set_behaviour_mode"), &SteeringBehaviours::set_behaviour_mode);
    ClassDB::bind_method(D_METHOD("get_behaviour_mode"), &SteeringBehaviours::get_behaviour_mode);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "behaviour_mode", PROPERTY_HINT_ENUM, "idle,fixed,disabled"), "set_behaviour_mode", "get_behaviour_mode");

    BIND_ENUM_CONSTANT(IDLE);
    BIND_ENUM_CONSTANT(FIXED);
    BIND_ENUM_CONSTANT(STATIC);

}

void SteeringBehaviour::process_behaviours(){
    SteeringBehaviours *ctrl = Object::cast_to<SteeringBehaviours>(get_parent());
    if (ctrl == NULL) return;
    if (!configuration.is_valid()) return;
    if (configuration->locomotion != SteeringBehaviourConfiguration::GHOST_FOLLOWING || first_frame) {
        first_frame = false;
        tr.set_origin(get_position());
    }
    switch (configuration->composition){
        case SteeringBehaviourConfiguration::TRUNKATED_WEIGHTS:
            force = compose_trunkated_weights(ctrl);
            break;
        case SteeringBehaviourConfiguration::PRIORITY_WEIGHTS:
            force = compose_priority_weights(ctrl);
            break;
        case SteeringBehaviourConfiguration::PRIORITIZED_DITHERING:
            break;
    }
    //print_line("result force " + rtos(force.x) + ", " + rtos(force.y));
}

void SteeringBehaviour::process_motion(float time){
    SteeringBehaviours *ctrl = Object::cast_to<SteeringBehaviours>(get_parent());
    if (ctrl == NULL) return;
    if (!configuration.is_valid()) return;
    switch (configuration->locomotion){
        case SteeringBehaviourConfiguration::SMOOTH_TORGUE: {
            Vector2 fn = force.normalized();
            Vector2 vn = speed > 0.1 ? velocity.normalized() : fn;
            if (ABS(fn.x) < 0.1 && ABS(fn.y) < 0.1 || Math::is_nan(fn.x) || Math::is_nan(fn.y)) fn = Vector2(1.0,0.0);
            if (ABS(vn.x) < 0.1 && ABS(vn.y) < 0.1 || Math::is_nan(vn.x) || Math::is_nan(vn.y)) vn = Vector2(1.0,0.0);
            float dot = vn.dot(fn);
            float sign = vn.dot(Vector2(fn.y, -fn.x)) > 0 ? 1.0 : -1.0;
            float angle = sign*MIN(max_turn_rate*time, 0.75*Math_PI*(1-dot));
            float acceleration = dot >= 0 ? max_force/mass : -0.5*max_force/mass;
            float min_speed = MIN(speed, acceleration > 0 ? 0.0 : 0.5 * max_speed);
            speed = CLAMP(speed+acceleration*time, min_speed, max_speed);
            if (Math::is_nan(speed)) speed = 0.0;
            Vector2 v = vn.rotated(angle);
            velocity = v*speed;
            //print_line(String("fn: ") + Variant(fn)+" vn: " + Variant(vn)+" dot:" + Variant(dot) + " sign"+Variant(sign)+" angle:"+Variant(angle)+" speed:"+Variant(speed)+" v:"+Variant(v)+" velocity:"+Variant(velocity)+" mass:"+Variant(mass)+" acceleration"+Variant(acceleration));
            float rot = velocity.angle();
            tr.set_rotation(rot);
            if (smooth_frames <= 1){
                set_rotation(rot);
            } else {
                Vector2 vp;
                if (smooth_position.size() == smooth_frames)
                    smooth_position.remove(0);
                smooth_position.append(v);
                for (int i=0; i<smooth_position.size(); i++){
                    vp += smooth_position[i];
                }
                set_rotation(vp.angle());
            }
            set_position(get_position() + velocity*time);
        }; break;
        case SteeringBehaviourConfiguration::DIRECT_FORCE: {
            Vector2 acceleration = force/mass;
            velocity = (velocity + acceleration*time).clamped(max_speed);
            //print_line("force: " + String(Variant(force)) + " acceleration: "+Variant(acceleration) + " velocity: " + Variant(velocity) + " pos: " + Variant(get_position()) + " tpos:" + Variant(target_position));
            Vector2 step = velocity*time;
            Vector2 pos = get_position() + step;
            float rot = velocity.angle();
            tr.set_rotation(rot);
            set_position(pos);
            if (smooth_frames <= 1){
                set_rotation(rot);
            } else {
                if (smooth_position.size() == smooth_frames)
                    smooth_position.remove(0);
                
                smooth_position.append(step);
                Vector2 v;
                for (int i=0; i<smooth_position.size(); i++){
                    v += smooth_position[i];
                }
                set_rotation(v.angle());
            }
        }; break;
        case SteeringBehaviourConfiguration::GHOST_FOLLOWING: {
            // ghost motion
            Vector2 to_ghost = tr.get_origin() - get_position();
            Vector2 ghost_acceleration = 1.5*force/mass;
            if (to_ghost.length() > configuration->ghost_distance){
                ghost_max_speed = MAX(max_speed*0.5, ghost_max_speed-time*max_force/mass);
            } else {
                ghost_max_speed = MIN(max_speed, ghost_max_speed+time*max_force/mass);
            }
            ghost_velocity = (ghost_velocity + ghost_acceleration*time).clamped(ghost_max_speed);
            Vector2 step = ghost_velocity*time;
            Vector2 pos = tr.get_origin() + step;
            tr.set_rotation(ghost_velocity.angle());
            tr.set_origin(pos);


            // body motion
            Vector2 fn = to_ghost.normalized();
            Vector2 vn = speed > 000.1 ? velocity.normalized() : fn;
            float dot = vn.dot(fn);
            float sign = vn.dot(Vector2(fn.y, -fn.x)) > 0 ? 1.0 : -1.0;
            float angle = sign*MIN(max_turn_rate*time, 0.5*Math_PI*(1-dot));
            float acceleration = dot > 0 ? max_force/mass : -max_force/mass;
            float min_speed = MIN(speed, acceleration > 0 ? 0.0 : 0.5 * max_speed);
            speed = CLAMP(speed+acceleration*time, min_speed, max_speed);
            Vector2 v = vn.rotated(angle);
            velocity = v*speed;
            float rot = velocity.angle();
            //print_line("vn: " + rtos(vn.x)+","+rtos(vn.y)+" v: "+rtos(v.x)+","+rtos(v.y)+" velocity:"+rtos(velocity.x)+","+rtos(velocity.y));
            //print_line("ghost velocity: " + rtos(ghost_velocity.x)+","+rtos(ghost_velocity.y)+" velocity: " + rtos(velocity.x)+","+rtos(velocity.y) + " speed:" + rtos(speed));
            //print_line("ghost acceleration: " + rtos(ghost_acceleration.x)+","+rtos(ghost_acceleration.y)+" force: " + rtos(force.x)+","+rtos(force.y) + " to_ghost: " + rtos(to_ghost.x) + ","+rtos(to_ghost.y));;
            //print_line("tr origin: " + rtos(tr.get_origin().x)+","+rtos(tr.get_origin().y)+" pos: " + rtos(get_position().x)+","+rtos(get_position().y));
            if (smooth_frames <= 1){
                set_rotation(rot);
            } else {
                Vector2 vp;
                if (smooth_position.size() == smooth_frames)
                    smooth_position.remove(0);
                smooth_position.append(v);
                for (int i=0; i<smooth_position.size(); i++){
                    vp += smooth_position[i];
                }
                set_rotation(vp.angle());
            }
            set_position(get_position() + velocity*time);

        }; break;
    }


    /*
    //float full_steers = Math::acos( force.normalized().dot( velocity.normalized() ) ) / (max_turn_rate);
    //float steer = full_steers * (Vector2(force.y, -force.x).dot(velocity) > 0 ? -1 : 1);
    //print_line("force: " + String(force) + ", full_steers: " + String(Variant(full_steers)) + ", steer: " + String(Variant(steer)));
    //Vector2 acceleration = force/mass;
    //velocity = Vector2(1,1).rotated(steer*time)*max_speed;
    velocity = v;
    //velocity = (velocity + acceleration*time).clamped(max_speed);
    Vector2 pos = get_position() + velocity*time;
    float rot = velocity.angle();
    tr.set_rotation(rot);
    set_position(pos);
    if (smooth_frames <= 1){
        set_rotation(rot);
    } else {
        //if (smooth_position.size() == smooth_frames)
        //    smooth_position.remove(0);
        if (smooth_rotation.size() == smooth_frames)
            smooth_rotation.remove(0);
        //smooth_position.append(pos);
        smooth_rotation.append(rot);
        //Vector2 sp = Vector2();
        float r = 0;
        for (int i=0; i<smooth_rotation.size(); i++){
            //sp += smooth_position[i];
            r += smooth_rotation[i];
        }
        //set_position(sp/smooth_position.size());
        set_rotation(r/smooth_rotation.size());
    }
    */
}

Vector2 SteeringBehaviour::compose_trunkated_weights(SteeringBehaviours *ctrl){
    Vector2 force = Vector2();
    for (int i=0; i<configuration->order.size(); i++){
        SteeringBehaviourConfiguration::BehaviourType type = (SteeringBehaviourConfiguration::BehaviourType)configuration->order[i];
        float weight = configuration->weights[i];
        switch (type){
            case SteeringBehaviourConfiguration::SEEK:      force += process_seek(ctrl) * weight; break;
            case SteeringBehaviourConfiguration::FLEE:      force += process_flee(ctrl) * weight; break;
            case SteeringBehaviourConfiguration::ARRIVE:    force += process_arrive(ctrl) * weight; break;
            case SteeringBehaviourConfiguration::EVADE:     force += process_evade(ctrl) * weight; break;
            case SteeringBehaviourConfiguration::AVOID:     force += process_avoid(ctrl) * weight; break;
            case SteeringBehaviourConfiguration::WANDER:    force += process_wander(ctrl) * weight; break;
            case SteeringBehaviourConfiguration::AVOID_WALLS: force += process_avoid_walls(ctrl) * weight; break;
            case SteeringBehaviourConfiguration::FREEZE:    force += process_freeze(ctrl) * weight; break;
            case SteeringBehaviourConfiguration::FOLLOW:    force += process_follow(ctrl) * weight; break;
        }
    }
    return force;
}

Vector2 SteeringBehaviour::compose_priority_weights(SteeringBehaviours *controller){
    Vector2 force = Vector2(0,0);
    for (int i=0; i<configuration->order.size(); i++){
        SteeringBehaviourConfiguration::BehaviourType type = (SteeringBehaviourConfiguration::BehaviourType)configuration->order[i];
        float weight = configuration->weights[i];
        Vector2 cf;
        switch (type){
            case SteeringBehaviourConfiguration::SEEK:
                if (weight > 0) cf = process_seek(controller) * weight; 
                if (!accumulate_force(force, cf)) return force;
                break;
            case SteeringBehaviourConfiguration::FLEE:
                if (weight > 0) cf = process_flee(controller) * weight;
                if (!accumulate_force(force, cf)) return force;
                break;
            case SteeringBehaviourConfiguration::ARRIVE:
                if (weight > 0) cf = process_arrive(controller) * weight; 
                if (!accumulate_force(force, cf)) return force;
                break;
            case SteeringBehaviourConfiguration::EVADE:
                if (weight > 0) cf = process_evade(controller) * weight; 
                if (!accumulate_force(force, cf)) return force;
                break;
            case SteeringBehaviourConfiguration::AVOID:
                if (weight > 0) cf = process_avoid(controller) * weight; 
                if (!accumulate_force(force, cf)) return force;
                break;
            case SteeringBehaviourConfiguration::WANDER:
                if (weight > 0) cf = process_wander(controller) * weight; 
                if (!accumulate_force(force, cf)) return force;
                break;
            case SteeringBehaviourConfiguration::AVOID_WALLS:
                if (weight > 0) cf = process_avoid_walls(controller) * weight; 
                if (!accumulate_force(force, cf)) return force;
                break;
            case SteeringBehaviourConfiguration::FREEZE:
                if (weight > 0) cf = process_freeze(controller) * weight; 
                if (!accumulate_force(force, cf)) return force;
                break;
            case SteeringBehaviourConfiguration::FOLLOW:
                if (weight > 0) cf = process_follow(controller) * weight; 
                if (!accumulate_force(force, cf)) return force;
                break;
        }
    }
    return force;
}

Vector2 SteeringBehaviour::compose_prioritized_dithering(SteeringBehaviours *controller){
    return Vector2();
}

bool SteeringBehaviour::accumulate_force(Vector2 &current, Vector2 force_to_add) const {
    float magnitude_so_far = current.length();
    float magnitude_remaining = max_force - magnitude_so_far;
    if (magnitude_remaining <= 0) return false;

    float magnitude_to_add = force_to_add.length();
    if (magnitude_to_add < magnitude_remaining)
        current += force_to_add;
    else
        current += force_to_add.normalized()*magnitude_remaining;
    return true;

}


Vector2 SteeringBehaviour::process_seek(SteeringBehaviours *controller) {
    Vector2 to_target = target_position - tr.get_origin();
    Vector2 desired_velocity = (to_target).normalized() * max_speed;
    const float arriving_dist_sq = 150.0*150.0;
    if (emit_arriving_position != target_position && to_target.length_squared() <= arriving_dist_sq){
        emit_arriving_position = target_position;
        emit_signal("arriving");
    }
    return desired_velocity - velocity;
}

Vector2 SteeringBehaviour::process_flee(SteeringBehaviours *controller) const{
    Vector2 desired_velocity = (tr.get_origin() - target_position).normalized() * max_speed;
    return desired_velocity - velocity;
}

Vector2 SteeringBehaviour::process_arrive(SteeringBehaviours *controller) {
    Vector2 to_target = target_position - tr.get_origin();
    float dist = to_target.length();
    if (dist <= 5 && emit_arrived_position != target_position) {
        emit_arrived_position = target_position;
        emit_signal("arrived");
        return Vector2();
    }
    if (dist <= 150 && emit_arriving_position != target_position){
        emit_arriving_position = target_position;
        emit_signal("arriving");
    }

    const float deceleration_tweak = 0.3;
    float speed = dist/((float)deceleration * deceleration_tweak);
    speed = MIN(speed, max_speed);
    Vector2 desired_velocity = to_target*speed/dist;
    Vector2 res =  desired_velocity - velocity;
    //print_line("result result force: " +rtos(res.x) + "," + rtos(res.y) + " pos: " + Variant(tr.get_origin())+ " target_pos: " + Variant(target_position));
    return res;
}

Vector2 SteeringBehaviour::process_freeze(SteeringBehaviours *controller) const {
    Vector2 res;
    if (ABS(velocity.x > 0.0001) || ABS(velocity.y) > 0.0001)
        res = velocity * -max_force/max_speed;
    else
        res = Vector2();
    //print_line(String("process_freeze ")+Variant(res)  + ", velocity: " + Variant(velocity));
    return res;
}

Vector2 SteeringBehaviour::process_follow(SteeringBehaviours *controller) const {
    Node2D *target = Object::cast_to<Node2D>(follow_target.get_ref());
    if (target == NULL) return Vector2();
    Vector2 to_target = target->get_position() - get_position();
    float dist = to_target.length();
    float speed = Math::lerp(0, max_speed, MIN(follow_offset, dist)/follow_offset);
    Vector2 desired_velocity = to_target*speed/dist;
    return desired_velocity - velocity;
}

const float DETECTION_BOX_MIN_LENGTH = 400.0;
const float BREAKING_WEIGHT = 0.2;

Vector2 SteeringBehaviour::process_avoid(SteeringBehaviours *controller) const{
    float box_len = configuration->avoid_detect_distance + speed/max_speed*configuration->avoid_detect_distance;
    float box_sq_len = box_len * box_len;
    float dist_to_closest = MAXFLOAT;
    SteeringBehaviour *closest = NULL;
    Vector2 local_pos_of_closest = Vector2();

    for (int i=0; i<controller->obstacles.size(); i++){
        SteeringBehaviour *obstacle = controller->obstacles[i];
        if (obstacle == NULL || obstacle == this || !(avoid_mask_detect & obstacle->avoid_mask_self)) continue;
        if (obstacle->get_position().distance_squared_to(tr.get_origin()) > box_sq_len) continue;
        Vector2 local_pos = tr.xform_inv(obstacle->get_position());
        if (local_pos.x<0) continue;

        float expanded_size = size + obstacle->size;
        if (ABS(local_pos.y) >= expanded_size) continue;

        float sqrt_part = sqrt(expanded_size*expanded_size - local_pos.y*local_pos.y);
        float ip = local_pos.x - sqrt_part;
        if (ip <= 0) ip = local_pos.x + sqrt_part;
        if (ip < dist_to_closest) {
            dist_to_closest = ip;
            closest = obstacle;
            local_pos_of_closest = local_pos;
        }
    }

    Vector2 force;
    if (closest == NULL) return force;
    float mult = 1.0 + (box_len - local_pos_of_closest.x) / box_len;
    force.y = (closest->size - local_pos_of_closest.y) * mult;
    force.x = (closest->size - local_pos_of_closest.x) * BREAKING_WEIGHT;
    Vector2 res =  tr.xform(force) - get_position();
    return res;
}

Vector2 SteeringBehaviour::process_wander(SteeringBehaviours *controller) const{
    return Vector2();
}

Vector2 SteeringBehaviour::process_evade(SteeringBehaviours *controller) const{
    return Vector2();
}

Vector2 SteeringBehaviour::process_avoid_walls(SteeringBehaviours *controller) const{
    Vector2 feelers[3] = {
        tr.xform(Vector2(configuration->wall_feeler_length, 0)),
        tr.xform(Vector2(configuration->wall_feeler_length*0.5,0).rotated(Math_PI*0.25)),
        tr.xform(Vector2(configuration->wall_feeler_length*0.5,0).rotated(-Math_PI*0.25))
    };

    Vector2 steering_force;
    Vector2 point;

    for (int flr=0; flr<3; flr++){
        float dist_to_this_ip = 0.0;
        float dist_to_closest_ip = MAXFLOAT;
        Vector2 closest_point;
        SteeringBehaviourWall *closest_wall = NULL;
        int closest_wall_idx = -1;
        for (int i=0; i<controller->walls.size(); i++){
            SteeringBehaviourWall *wall = controller->walls[i];
            Vector<Vector2> lines = wall->get_lines();
            for (int wi=0; wi<4; wi++){
                if (Geometry::segment_intersects_segment_2d(get_position(), feelers[flr], lines[wi*2], lines[wi*2+1], &point)){
                    dist_to_this_ip = get_position().distance_to(point);
                    if (dist_to_this_ip < dist_to_closest_ip){
                        dist_to_closest_ip = dist_to_this_ip;
                        closest_wall = wall;
                        closest_wall_idx = wi;
                        closest_point = point;
                    }
                }
            }
        }
        if (closest_wall != NULL){
            Vector2 overshoot = feelers[flr] - closest_point;
            steering_force += closest_wall->get_normals()[closest_wall_idx] * overshoot.length();
        }
    }
    //print_line("avoid wall force " + rtos(steering_force.x) + ", " + rtos(steering_force.y));
    return steering_force;
}



void SteeringBehaviour::set_mass(float value){ mass = value; }
float SteeringBehaviour::get_mass() const { return mass; }
void SteeringBehaviour::set_size(float value){ size = value; }
float SteeringBehaviour::get_size() const { return size; }
void SteeringBehaviour::set_max_speed(float value) { max_speed = value; ghost_max_speed = value; }
float SteeringBehaviour::get_max_speed() const { return max_speed; }
void SteeringBehaviour::set_max_turn_rate(float value) { max_turn_rate = value; }
float SteeringBehaviour::get_max_turn_rate() const { return max_turn_rate; }
void SteeringBehaviour::set_max_force(float value) { max_force = value; }
float SteeringBehaviour::get_max_force() const { return max_force; }
int SteeringBehaviour::get_behaviour_mode() const { return behaviour_mode; }
void SteeringBehaviour::set_smooth_frames(int frames) { smooth_frames = MAX(0, frames); }
int SteeringBehaviour::get_smooth_frames() const { return smooth_frames; }
void SteeringBehaviour::set_target_position(Vector2 pos) { target_position = pos; }
void SteeringBehaviour::set_follow_target(Variant target) { follow_target.set_obj(target); }
Variant SteeringBehaviour::get_follow_target()const { return Object::cast_to<Node2D>(follow_target.get_ref()); }
void SteeringBehaviour::set_follow_offset(float offset) { follow_offset = offset; }
float SteeringBehaviour::get_follow_offset()const { return follow_offset; }
void SteeringBehaviour::set_avoid_mask_detect(int mask) { avoid_mask_detect = mask; }
int SteeringBehaviour::get_avoid_mask_detect()const { return avoid_mask_detect; }
void SteeringBehaviour::set_avoid_mask_self(int mask) { avoid_mask_self = mask; }
int SteeringBehaviour::get_avoid_mask_self()const { return avoid_mask_self; }
void SteeringBehaviour::set_configuration(const Ref<SteeringBehaviourConfiguration> &conf) { configuration = conf; }
Ref<SteeringBehaviourConfiguration> SteeringBehaviour::get_configuration() const { return configuration; }
Vector2 SteeringBehaviour::get_target_position() const { return target_position; }
void SteeringBehaviour::set_behaviour_mode(int mode) { 
    behaviour_mode = (SteeringBehaviours::BehaviourProcessMode)mode;
    if (Engine::get_singleton()->is_editor_hint()) return;
    SteeringBehaviours *ctrl = Object::cast_to<SteeringBehaviours>(get_parent());
    int idx = ctrl == NULL ? -2 : ctrl->obstacles.find(this);
    switch (behaviour_mode){
        case IDLE:
            set_process(true);
            set_physics_process(false);
            if (ctrl != NULL && idx >= 0) ctrl->obstacles.remove(idx);
            break;
        case FIXED:
            set_process(false);
            set_physics_process(true);
            if (ctrl != NULL && idx >= 0) ctrl->obstacles.remove(idx);
            break;
        default:
            set_process(false);
            set_physics_process(false);
            if (ctrl != NULL && idx < 0) ctrl->obstacles.push_back(this);
            break;
    }
}

void SteeringBehaviour::_notification(int what){
    switch (what){
        case NOTIFICATION_ENTER_TREE:
            if (behaviour_mode == STATIC){
                SteeringBehaviours *ctrl = Object::cast_to<SteeringBehaviours>(get_parent());
                if (ctrl!= NULL){
                    ctrl->obstacles.push_back(this);
                }
            }
            break;
        case NOTIFICATION_EXIT_TREE:
            if (behaviour_mode == STATIC){
                SteeringBehaviours *ctrl = Object::cast_to<SteeringBehaviours>(get_parent());
                if (ctrl!= NULL){
                    int idx = ctrl->obstacles.find(this);
                    if (idx >= 0) ctrl->obstacles.remove(idx);
                }
            }
        case NOTIFICATION_PROCESS: 
            if (behaviour_mode == IDLE) process_motion(get_process_delta_time());
            break;
        case NOTIFICATION_PHYSICS_PROCESS:
            if (behaviour_mode == FIXED) process_motion(get_physics_process_delta_time());
            break;

    }
}

void SteeringBehaviour::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_mass", "mass"), &SteeringBehaviour::set_mass);
    ClassDB::bind_method(D_METHOD("get_mass"), &SteeringBehaviour::get_mass);
    ClassDB::bind_method(D_METHOD("set_size", "size"), &SteeringBehaviour::set_size);
    ClassDB::bind_method(D_METHOD("get_size"), &SteeringBehaviour::get_size);
    ClassDB::bind_method(D_METHOD("set_smooth_frames", "size"), &SteeringBehaviour::set_smooth_frames);
    ClassDB::bind_method(D_METHOD("get_smooth_frames"), &SteeringBehaviour::get_smooth_frames);
    ClassDB::bind_method(D_METHOD("set_max_speed", "max_speed"), &SteeringBehaviour::set_max_speed);
    ClassDB::bind_method(D_METHOD("get_max_speed"), &SteeringBehaviour::get_max_speed);
    ClassDB::bind_method(D_METHOD("set_max_force", "max_force"), &SteeringBehaviour::set_max_force);
    ClassDB::bind_method(D_METHOD("get_max_force"), &SteeringBehaviour::get_max_force);
    ClassDB::bind_method(D_METHOD("set_max_turn_rate", "max_turn_rate"), &SteeringBehaviour::set_max_turn_rate);
    ClassDB::bind_method(D_METHOD("get_max_turn_rate"), &SteeringBehaviour::get_max_turn_rate);
    ClassDB::bind_method(D_METHOD("set_behaviour_mode", "behaviour_mode"), &SteeringBehaviour::set_behaviour_mode);
    ClassDB::bind_method(D_METHOD("get_behaviour_mode"), &SteeringBehaviour::get_behaviour_mode);
    ClassDB::bind_method(D_METHOD("set_target_position", "target_position"), &SteeringBehaviour::set_target_position);
    ClassDB::bind_method(D_METHOD("get_target_position"), &SteeringBehaviour::get_target_position);
    ClassDB::bind_method(D_METHOD("set_follow_target", "follow_target"), &SteeringBehaviour::set_follow_target);
    ClassDB::bind_method(D_METHOD("get_follow_target"), &SteeringBehaviour::get_follow_target);    
    ClassDB::bind_method(D_METHOD("set_follow_offset", "follow_offset"), &SteeringBehaviour::set_follow_offset);
    ClassDB::bind_method(D_METHOD("get_follow_offset"), &SteeringBehaviour::get_follow_offset);    
    ClassDB::bind_method(D_METHOD("set_avoid_mask_detect", "avoid_mask_detect"), &SteeringBehaviour::set_avoid_mask_detect);
    ClassDB::bind_method(D_METHOD("get_avoid_mask_detect"), &SteeringBehaviour::get_avoid_mask_detect);    
    ClassDB::bind_method(D_METHOD("set_avoid_mask_self", "avoid_mask_self"), &SteeringBehaviour::set_avoid_mask_self);
    ClassDB::bind_method(D_METHOD("get_avoid_mask_self"), &SteeringBehaviour::get_avoid_mask_self);
    ClassDB::bind_method(D_METHOD("set_configuration", "configuration"), &SteeringBehaviour::set_configuration);
    ClassDB::bind_method(D_METHOD("get_configuration"), &SteeringBehaviour::get_configuration);

    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "configuration", PROPERTY_HINT_RESOURCE_TYPE, "SteeringBehaviourConfiguration"), "set_configuration", "get_configuration");
    ADD_PROPERTY(PropertyInfo(Variant::REAL, "mass"), "set_mass", "get_mass");
    ADD_PROPERTY(PropertyInfo(Variant::REAL, "size"), "set_size", "get_size");
    ADD_PROPERTY(PropertyInfo(Variant::REAL, "max_speed"), "set_max_speed", "get_max_speed");
    ADD_PROPERTY(PropertyInfo(Variant::REAL, "max_force"), "set_max_force", "get_max_force");
    ADD_PROPERTY(PropertyInfo(Variant::REAL, "max_turn_rate"), "set_max_turn_rate", "get_max_turn_rate");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "smooth_frames", PROPERTY_HINT_RANGE, "0,100,1"), "set_smooth_frames", "get_smooth_frames");
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "target_position"), "set_target_position", "get_target_position");
    ADD_PROPERTY(PropertyInfo(Variant::NIL, "follow_target"), "set_follow_target", "get_follow_target");    
    ADD_PROPERTY(PropertyInfo(Variant::REAL, "follow_offset"), "set_follow_offset", "get_follow_offset");    
    ADD_PROPERTY(PropertyInfo(Variant::INT, "behaviour_mode", PROPERTY_HINT_ENUM, "idle,fixed,static"), "set_behaviour_mode", "get_behaviour_mode");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "avoid_mask_detect"), "set_avoid_mask_detect", "get_avoid_mask_detect");    
    ADD_PROPERTY(PropertyInfo(Variant::INT, "avoid_mask_self"), "set_avoid_mask_self", "get_avoid_mask_self");    

    ADD_SIGNAL(MethodInfo("arrived"));
    ADD_SIGNAL(MethodInfo("arriving"));

    BIND_ENUM_CONSTANT(STATIC);
    BIND_ENUM_CONSTANT(IDLE);
    BIND_ENUM_CONSTANT(FIXED);
    
}


SteeringBehaviour::SteeringBehaviour(){
    first_frame = true;    
    smooth_frames = 1;
    velocity = Vector2();
    force = Vector2();
    speed = 0.0;
    size = 50.0;
    mass = 1.0;
    max_speed = 100.0;
    max_force = 100.0;
    max_turn_rate = 1.57;
    follow_offset = 1.f;
    avoid_mask_detect = 1;
    avoid_mask_self = 1;
}

SteeringBehaviour::~SteeringBehaviour(){
    smooth_position.resize(0);
    smooth_rotation.resize(0);
}


Vector<Vector2> SteeringBehaviourWall::get_lines() const {
    Vector<Vector2> lines;
    lines.resize(8);
    lines.write[0] = get_transform().xform(half_size*Vector2(-1,-1));
    lines.write[1] = get_transform().xform(half_size*Vector2(1,-1));
    lines.write[2] = get_transform().xform(half_size*Vector2(1,-1));
    lines.write[3] = get_transform().xform(half_size*Vector2(1,1));
    lines.write[4] = get_transform().xform(half_size*Vector2(1,1));
    lines.write[5] = get_transform().xform(half_size*Vector2(-1,1));
    lines.write[6] = get_transform().xform(half_size*Vector2(-1,1));
    lines.write[7] = get_transform().xform(half_size*Vector2(-1,-1));
    return lines;
}

Vector<Vector2> SteeringBehaviourWall::get_normals() const {
    Vector<Vector2> normals;
    normals.resize(4);
    switch (wall_type){
        case WallType::INNER:
            normals.write[0] = Vector2(0,1).rotated(get_rotation());
            normals.write[1] = Vector2(-1,0).rotated(get_rotation());
            normals.write[2] = Vector2(0,-1).rotated(get_rotation());
            normals.write[3] = Vector2(1,0).rotated(get_rotation());
            break;
        case WallType::OUTER:
            normals.write[0] = Vector2(0,-1).rotated(get_rotation());
            normals.write[1] = Vector2(1,0).rotated(get_rotation());
            normals.write[2] = Vector2(0,1).rotated(get_rotation());
            normals.write[3] = Vector2(-1,0).rotated(get_rotation());
            break;
    }
    return normals;
}

void SteeringBehaviourWall::_notification(int p_what) {
    #ifdef TOOLS_ENABLED
    if (p_what == NOTIFICATION_DRAW && Engine::get_singleton()->is_editor_hint()){
        Color border_color = get_self_modulate();
        Color fill_color = get_self_modulate();
        border_color.a = 0.4;
        fill_color.a = 0.2;
        Rect2 rect = Rect2(-half_size, half_size*2.0);
        switch (wall_type){
            case (WallType::OUTER): draw_rect(rect, fill_color, true); break;
            case (WallType::INNER):
                draw_rect(Rect2(-half_size.x-10, -half_size.y-10, 2*half_size.x+20, 10), fill_color, true);
                draw_rect(Rect2(-half_size.x-10, half_size.y, 2*half_size.x+20.0, 10), fill_color, true);
                draw_rect(Rect2(-half_size.x-10, -half_size.y, 10, half_size.y*2), fill_color, true);
                draw_rect(Rect2(half_size.x, -half_size.y, 10, 2*half_size.y), fill_color, true);
                break;
        }
        draw_rect(rect, border_color, false);
        return;
    }
    #endif
    SteeringBehaviours *ctrl = Object::cast_to<SteeringBehaviours>(get_parent());
    switch (p_what){
        case NOTIFICATION_ENTER_TREE:
            if (ctrl!= NULL){
                ctrl->walls.push_back(this);
            }
            break;
        case NOTIFICATION_EXIT_TREE:
            if (ctrl!= NULL){
                int idx = ctrl->walls.find(this);
                if (idx >= 0) ctrl->walls.remove(idx);
            }
            break;
    }
}

Vector2 SteeringBehaviourWall::get_size() const {
    return half_size*2;
}

void SteeringBehaviourWall::set_size(Vector2 p_size) {
    half_size = p_size*0.5;
    if (Engine::get_singleton()->is_editor_hint()) update();
}

SteeringBehaviourWall::WallType SteeringBehaviourWall::get_wall_type() const {
    return wall_type;
}

void SteeringBehaviourWall::set_wall_type(WallType p_wall_type){
    wall_type = p_wall_type;
    if (Engine::get_singleton()->is_editor_hint()) update();
}

void SteeringBehaviourWall::_bind_methods(){
    ClassDB::bind_method(D_METHOD("set_size", "p_size"), &SteeringBehaviourWall::set_size);
    ClassDB::bind_method(D_METHOD("get_size"), &SteeringBehaviourWall::get_size);
    ClassDB::bind_method(D_METHOD("set_wall_type", "p_wall_type"), &SteeringBehaviourWall::set_wall_type);
    ClassDB::bind_method(D_METHOD("get_wall_type"), &SteeringBehaviourWall::get_wall_type);

    ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "size"), "set_size", "get_size");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "wall_type", PROPERTY_HINT_ENUM, "inner,outer"), "set_wall_type", "get_wall_type");

    BIND_ENUM_CONSTANT(INNER);
    BIND_ENUM_CONSTANT(OUTER);
}

SteeringBehaviourWall::SteeringBehaviourWall(){
    half_size = Vector2(50,50);
}