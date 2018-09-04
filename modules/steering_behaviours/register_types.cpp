#include "register_types.h"
#include "steering_behaviours.h"

void register_steering_behaviours_types(){
    ClassDB::register_class<SteeringBehaviour>();
    ClassDB::register_class<SteeringBehaviours>();
    ClassDB::register_class<SteeringBehaviourConfiguration>();
    ClassDB::register_class<SteeringBehaviourWall>();
}

void unregister_steering_behaviours_types(){
    
}