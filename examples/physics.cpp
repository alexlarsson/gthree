#include "physics.h"

#include <btBulletDynamicsCommon.h>
#include <vector>

struct PhysicsWorld {
  btDefaultCollisionConfiguration *config;
  btCollisionDispatcher *dispatcher;
  btBroadphaseInterface *broadphase;
  btSequentialImpulseConstraintSolver *solver;
  btDiscreteDynamicsWorld *world;

  std::vector<btRigidBody *> bodies;
  std::vector<btCollisionShape *> shapes;
};

extern "C" {

PhysicsWorld *
physics_world_new (float gravity_y)
{
  auto *pw = new PhysicsWorld;

  pw->config = new btDefaultCollisionConfiguration;
  pw->dispatcher = new btCollisionDispatcher (pw->config);
  pw->broadphase = new btDbvtBroadphase;
  pw->solver = new btSequentialImpulseConstraintSolver;
  pw->world = new btDiscreteDynamicsWorld (pw->dispatcher, pw->broadphase,
                                           pw->solver, pw->config);
  pw->world->setGravity (btVector3 (0, gravity_y, 0));

  return pw;
}

static void
clear_bodies (PhysicsWorld *pw)
{
  for (auto *body : pw->bodies)
    {
      pw->world->removeRigidBody (body);
      delete body->getMotionState ();
      delete body;
    }
  pw->bodies.clear ();

  for (auto *shape : pw->shapes)
    delete shape;
  pw->shapes.clear ();
}

void
physics_world_free (PhysicsWorld *pw)
{
  clear_bodies (pw);
  delete pw->world;
  delete pw->solver;
  delete pw->broadphase;
  delete pw->dispatcher;
  delete pw->config;
  delete pw;
}

void
physics_world_clear (PhysicsWorld *pw)
{
  clear_bodies (pw);
}

int
physics_world_add_box (PhysicsWorld *pw,
                       float x, float y, float z,
                       float w, float h, float d,
                       float rot_x, float rot_y, float rot_z,
                       float friction, float restitution)
{
  auto *shape = new btBoxShape (btVector3 (w / 2, h / 2, d / 2));
  pw->shapes.push_back (shape);

  btTransform transform;
  transform.setIdentity ();
  transform.setOrigin (btVector3 (x, y, z));

  btQuaternion qx, qy, qz;
  qx.setRotation (btVector3 (1, 0, 0), rot_x);
  qy.setRotation (btVector3 (0, 1, 0), rot_y);
  qz.setRotation (btVector3 (0, 0, 1), rot_z);
  transform.setRotation (qy * qx * qz);

  auto *motion = new btDefaultMotionState (transform);
  btRigidBody::btRigidBodyConstructionInfo info (0, motion, shape);
  info.m_friction = friction;
  info.m_restitution = restitution;

  auto *body = new btRigidBody (info);
  pw->world->addRigidBody (body);

  int id = (int) pw->bodies.size ();
  pw->bodies.push_back (body);
  return id;
}

int
physics_world_add_sphere (PhysicsWorld *pw,
                          float radius, float mass,
                          float x, float y, float z,
                          float friction, float restitution)
{
  auto *shape = new btSphereShape (radius);
  pw->shapes.push_back (shape);

  btVector3 inertia (0, 0, 0);
  shape->calculateLocalInertia (mass, inertia);

  btTransform transform;
  transform.setIdentity ();
  transform.setOrigin (btVector3 (x, y, z));

  auto *motion = new btDefaultMotionState (transform);
  btRigidBody::btRigidBodyConstructionInfo info (mass, motion, shape, inertia);
  info.m_friction = friction;
  info.m_restitution = restitution;
  info.m_rollingFriction = 0.05f;
  info.m_spinningFriction = 0.02f;

  auto *body = new btRigidBody (info);
  body->setActivationState (DISABLE_DEACTIVATION);
  pw->world->addRigidBody (body);

  int id = (int) pw->bodies.size ();
  pw->bodies.push_back (body);
  return id;
}

void
physics_world_step (PhysicsWorld *pw, float dt)
{
  pw->world->stepSimulation (dt, 10, 1.0f / 240.0f);
}

void
physics_world_get_position (PhysicsWorld *pw, int id,
                            float *x, float *y, float *z)
{
  btTransform transform;
  pw->bodies[id]->getMotionState ()->getWorldTransform (transform);
  const btVector3 &pos = transform.getOrigin ();
  *x = pos.x ();
  *y = pos.y ();
  *z = pos.z ();
}

void
physics_world_get_rotation (PhysicsWorld *pw, int id,
                            float *x, float *y, float *z, float *w)
{
  btTransform transform;
  pw->bodies[id]->getMotionState ()->getWorldTransform (transform);
  const btQuaternion &rot = transform.getRotation ();
  *x = rot.x ();
  *y = rot.y ();
  *z = rot.z ();
  *w = rot.w ();
}

void
physics_world_get_linear_velocity (PhysicsWorld *pw, int id,
                                   float *vx, float *vy, float *vz)
{
  const btVector3 &v = pw->bodies[id]->getLinearVelocity ();
  *vx = v.x ();
  *vy = v.y ();
  *vz = v.z ();
}

void
physics_world_set_linear_velocity (PhysicsWorld *pw, int id,
                                   float vx, float vy, float vz)
{
  pw->bodies[id]->setLinearVelocity (btVector3 (vx, vy, vz));
}

void
physics_world_set_angular_velocity (PhysicsWorld *pw, int id,
                                    float vx, float vy, float vz)
{
  pw->bodies[id]->setAngularVelocity (btVector3 (vx, vy, vz));
}

void
physics_world_apply_central_force (PhysicsWorld *pw, int id,
                                   float fx, float fy, float fz)
{
  pw->bodies[id]->applyCentralForce (btVector3 (fx, fy, fz));
}

void
physics_world_set_position (PhysicsWorld *pw, int id,
                            float x, float y, float z)
{
  btTransform transform;
  pw->bodies[id]->getMotionState ()->getWorldTransform (transform);
  transform.setOrigin (btVector3 (x, y, z));
  pw->bodies[id]->setWorldTransform (transform);
  pw->bodies[id]->getMotionState ()->setWorldTransform (transform);
  pw->bodies[id]->setLinearVelocity (btVector3 (0, 0, 0));
  pw->bodies[id]->setAngularVelocity (btVector3 (0, 0, 0));
  pw->bodies[id]->clearForces ();
}

void
physics_world_set_damping (PhysicsWorld *pw, int id,
                           float linear, float angular)
{
  pw->bodies[id]->setDamping (linear, angular);
}

void
physics_world_activate (PhysicsWorld *pw, int id)
{
  pw->bodies[id]->activate (true);
}

} /* extern "C" */
