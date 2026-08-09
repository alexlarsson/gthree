#ifndef PHYSICS_H
#define PHYSICS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PhysicsWorld PhysicsWorld;

PhysicsWorld *physics_world_new          (float gravity_y);
void          physics_world_free         (PhysicsWorld *world);
void          physics_world_clear        (PhysicsWorld *world);

int  physics_world_add_box    (PhysicsWorld *world,
                               float x, float y, float z,
                               float w, float h, float d,
                               float rot_x, float rot_y, float rot_z,
                               float friction, float restitution);

int  physics_world_add_sphere (PhysicsWorld *world,
                               float radius, float mass,
                               float x, float y, float z,
                               float friction, float restitution);

void physics_world_step       (PhysicsWorld *world, float dt);

void physics_world_get_position (PhysicsWorld *world, int id,
                                 float *x, float *y, float *z);

void physics_world_get_rotation (PhysicsWorld *world, int id,
                                 float *x, float *y, float *z, float *w);

void physics_world_get_linear_velocity (PhysicsWorld *world, int id,
                                        float *vx, float *vy, float *vz);

void physics_world_set_linear_velocity (PhysicsWorld *world, int id,
                                        float vx, float vy, float vz);

void physics_world_set_angular_velocity (PhysicsWorld *world, int id,
                                         float vx, float vy, float vz);

void physics_world_apply_central_force (PhysicsWorld *world, int id,
                                        float fx, float fy, float fz);

void physics_world_set_position (PhysicsWorld *world, int id,
                                 float x, float y, float z);

void physics_world_set_damping (PhysicsWorld *world, int id,
                                float linear, float angular);

void physics_world_activate (PhysicsWorld *world, int id);

#ifdef __cplusplus
}
#endif

#endif /* PHYSICS_H */
