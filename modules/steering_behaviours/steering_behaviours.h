
#ifndef STERRING_BEHAVIOURS_H
#define STERRING_BEHAVIOURS_H

#include "scene/2d/node_2d.h"

class SteeringBehaviourConfiguration: public Resource {
    GDCLASS(SteeringBehaviourConfiguration, Resource);
public:
    enum BehaviourType {
        SEEK,
        FLEE,
        ARRIVE,
        EVADE,
        AVOID,
        WANDER,
        AVOID_WALLS,
        FREEZE,
        FOLLOW,
        SIZE
    };

    const String BehaviourTypeNames[SIZE] = {
        String("seek"),
        String("flee"),
        String("arrive"),
        String("evade"),
        String("avoid"),
        String("wander"),
        String("avoid_walls"),
        String("freeze"),
        String("follow")
    };

    enum BehaviourLocomotion {
        DIRECT_FORCE,
        SMOOTH_TORGUE,
        GHOST_FOLLOWING
    };
    enum BehaviourComposition {
        TRUNKATED_WEIGHTS,
        PRIORITY_WEIGHTS,
        PRIORITIZED_DITHERING
    };

    enum BehaviourProcessMode {
        IDLE,
        FIXED,
        STATIC
    };

    String name;
    BehaviourComposition composition;
    BehaviourLocomotion locomotion;
    float avoid_detect_distance;
    float wall_feeler_length;
    float ghost_distance;
    PoolIntArray order;
    PoolRealArray weights;

public:
    void _get_property_list(List<PropertyInfo> *list) const;
    bool _get(const String &p_name, Variant &r_ret) const;
    bool _set(const String &p_name, const Variant &p_value);

    SteeringBehaviourConfiguration();
    ~SteeringBehaviourConfiguration();
protected:
    static void _bind_methods();
};

class SteeringBehaviour;
class SteeringBehaviourWall;
class SteeringBehaviours: public Node2D {
    GDCLASS(SteeringBehaviours, Node2D);

public:

    enum BehaviourProcessMode {
        IDLE,
        FIXED,
        STATIC
    };

    Vector<SteeringBehaviour*> obstacles;
    Vector<SteeringBehaviourWall*> walls;
    BehaviourProcessMode mode;

    void process_behaviours();

    void set_behaviour_mode(int value);
    int get_behaviour_mode() const;

    void _notification(int what);

protected:
    static void _bind_methods();


};

class SteeringBehaviour: public Node2D {
    GDCLASS(SteeringBehaviour, Node2D);
public:
    enum DecelerationType {
        SLOW=3,
        NORMAL=2,
        FAST=1
    };

    enum BehaviourProcessMode {
        IDLE,
        FIXED,
        STATIC
    };

    void process_behaviours();

    void set_mass(float value);
    float get_mass() const;
    void set_size(float value);
    float get_size() const;
    void set_max_speed(float value);
    float get_max_speed() const;
    void set_max_turn_rate(float value);
    float get_max_turn_rate() const;
    void set_max_force(float value);
    float get_max_force() const;
    void set_behaviour_mode(int mode);
    int get_behaviour_mode() const;
    void set_smooth_frames(int mode);
    int get_smooth_frames() const;
    void set_target_position(Vector2 pos);
    Vector2 get_target_position() const;
    void set_follow_target(Variant target);
    Variant get_follow_target() const;
    void set_follow_offset(float target);
    float get_follow_offset() const;
    void set_avoid_mask_detect(int mask);
    int get_avoid_mask_detect() const;
    void set_avoid_mask_self(int mask);
    int get_avoid_mask_self() const;

    void set_configuration(const Ref<SteeringBehaviourConfiguration> &conf);
    Ref<SteeringBehaviourConfiguration> get_configuration() const;


    void _notification(int p_what);

    SteeringBehaviour();
    ~SteeringBehaviour();

protected:
    Ref<SteeringBehaviourConfiguration> configuration;
    Transform2D tr;
    PoolVector2Array smooth_position;
    PoolRealArray smooth_rotation;
    unsigned int smooth_frames;
    Vector2 velocity;
    bool first_frame;
    float ghost_max_speed;
    Vector2 ghost_velocity;
    Vector2 force;
    float speed;
    float size;
    float mass;
    float max_speed;
    float max_force;
    float max_turn_rate;
    SteeringBehaviours::BehaviourProcessMode behaviour_mode;
    WeakRef follow_target;
    float follow_offset;

    DecelerationType deceleration;
    Vector2 target_position;
    Vector2 emit_arrived_position;
    Vector2 emit_arriving_position;

    int avoid_mask_detect;
    int avoid_mask_self;

    void process_motion(float delta);
    bool accumulate_force(Vector2 &current, Vector2 force_to_add) const;
    Vector2 compose_trunkated_weights(SteeringBehaviours *controller);
    Vector2 compose_priority_weights(SteeringBehaviours *controller);
    Vector2 compose_prioritized_dithering(SteeringBehaviours *controller);

    Vector2 process_seek(SteeringBehaviours *controller);
    Vector2 process_flee(SteeringBehaviours *controller) const;
    Vector2 process_arrive(SteeringBehaviours *controller);
    Vector2 process_evade(SteeringBehaviours *controller) const;
    Vector2 process_wander(SteeringBehaviours *controller) const;
    Vector2 process_avoid(SteeringBehaviours *controller) const;
    Vector2 process_avoid_walls(SteeringBehaviours *controller) const;
    Vector2 process_follow(SteeringBehaviours *controller) const;
    Vector2 process_freeze(SteeringBehaviours *controller) const;

    static void _bind_methods();


};

class SteeringBehaviourWall: public Node2D {
    GDCLASS(SteeringBehaviourWall, Node2D);

public:
    enum WallType {
        INNER,
        OUTER
    };
protected:
    WallType wall_type;
    Vector2 half_size;

    static void _bind_methods();
    void _notification(int p_what);

public:
    Vector<Vector2> get_lines() const;
    Vector<Vector2> get_normals() const;
    void set_width(float p_width);
    Vector2 get_size() const;
    void set_size(Vector2 p_size);
    WallType get_wall_type() const;
    void set_wall_type(WallType p_wall_type);
    SteeringBehaviourWall();

};

VARIANT_ENUM_CAST(SteeringBehaviour::BehaviourProcessMode);
VARIANT_ENUM_CAST(SteeringBehaviours::BehaviourProcessMode);
VARIANT_ENUM_CAST(SteeringBehaviourConfiguration::BehaviourType);
VARIANT_ENUM_CAST(SteeringBehaviourConfiguration::BehaviourComposition);
VARIANT_ENUM_CAST(SteeringBehaviourConfiguration::BehaviourLocomotion);
VARIANT_ENUM_CAST(SteeringBehaviourConfiguration::BehaviourProcessMode);
VARIANT_ENUM_CAST(SteeringBehaviourWall::WallType);


#endif