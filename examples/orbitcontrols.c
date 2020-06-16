#include "orbitcontrols.h"
#include "utils.h"

#define EPS 0.000001

enum
{
  CHANGED,

  LAST_SIGNAL
};

static guint object_signals[LAST_SIGNAL] = { 0, };

enum {
      STATE_NONE,
      STATE_ROTATE,
      STATE_DOLLY,
      STATE_PAN,
      STATE_TOUCH_ROTATE,
      STATE_TOUCH_DOLLY_PAN
};


struct _GthreeOrbitControlsClass {
  GObjectClass parent_class;
};

struct _GthreeOrbitControls {
  GObject parent;

  GthreeObject *object;
  GtkWidget *darea;

  gboolean enabled;

  // "target" sets the location of focus, where the object orbits around
  graphene_vec3_t target;

  // How far you can dolly in and out ( PerspectiveCamera only )
  float minDistance;
  float maxDistance;

  // How far you can zoom in and out ( OrthographicCamera only )
  float minZoom;
  float maxZoom;

  // How far you can orbit vertically, upper and lower limits.
  // Range is 0 to PI radians.
  float minPolarAngle;
  float maxPolarAngle;

  // How far you can orbit horizontally, upper and lower limits.
  // If set, must be a sub-interval of the interval [ - G_PI, G_PI ].
  float minAzimuthAngle;
  float maxAzimuthAngle;

  // Set to true to enable damping (inertia)
  gboolean enableDamping;
  float dampingFactor;

  // This option actually enables dollying in and out; left as "zoom" for backwards compatibility.
  // Set to false to disable zooming
  gboolean enableZoom;
  float zoomSpeed;

  // Set to false to disable rotating
  gboolean enableRotate;
  float rotateSpeed;

  // Set to false to disable panning
  gboolean enablePan;
  float panSpeed;
  gboolean screenSpacePanning; // if true, pan in screen-space
  float keyPanSpeed;	// pixels moved per arrow key push

  // Set to true to automatically rotate around the target
  gboolean autoRotate;
  float autoRotateSpeed; // 30 seconds per round when fps is 60

  // Set to false to disable use of the keys
  gboolean enableKeys;

  // for reset
  graphene_vec3_t target0;
  graphene_vec3_t position0;
  float zoom0;

  Spherical sphericalDelta;
  float scale;
  graphene_vec3_t pan_offset;
  gboolean zoomChanged;
  int state;

  graphene_vec3_t last_position;
  graphene_quaternion_t last_quaternion;

  guint tick_id;

  graphene_vec2_t dragStart;
};

G_DEFINE_TYPE (GthreeOrbitControls, gthree_orbit_controls, G_TYPE_OBJECT)

static gboolean gthree_orbit_controls_update (GthreeOrbitControls *orbit);

static void
drag_begin_cb (GtkGestureDrag *gesture,
               gdouble         start_x,
               gdouble         start_y,
               gpointer        user_data)
{
  GthreeOrbitControls *orbit = user_data;
  double x, y;
  GdkDevice *device;
  GdkModifierType mask;
  guint button;

  if (!orbit->enabled)
    return;

  if (!orbit->enableRotate)
    return;

  if (!gtk_gesture_drag_get_offset (gesture, &x, &y))
    return;

  device = gtk_gesture_get_device (GTK_GESTURE (gesture));
  gdk_device_get_state (device,
#ifdef USE_GTK4
                        gtk_native_get_surface (gtk_widget_get_native (orbit->darea)),
#else
                        gtk_widget_get_window (orbit->darea),
#endif
                        NULL, &mask);

  button = gtk_gesture_single_get_current_button (GTK_GESTURE_SINGLE (gesture));

  switch (button)
    {
    case 1:
      if ((mask & GDK_SHIFT_MASK) != 0 ||
          (mask & GDK_CONTROL_MASK) != 0 ||
          (mask & GDK_META_MASK) != 0)
        {
          if (!orbit->enablePan)
            return;

          orbit->state = STATE_PAN;
        }
      else
        {
          if (!orbit->enableRotate)
            return;

          orbit->state = STATE_ROTATE;
        }
      break;

    case 2:
      if (!orbit->enableZoom)
        return;

      orbit->state = STATE_DOLLY;
      break;

    case 3:
      if (!orbit->enablePan)
        return;

      orbit->state = STATE_PAN;
      break;

    default:
      orbit->state = STATE_NONE;
      break;
    }

  graphene_vec2_init (&orbit->dragStart, x, y);
}

static void
gthree_orbit_controls_rotate_left (GthreeOrbitControls *orbit,
                            float angle)
{
  orbit->sphericalDelta.theta -= angle;
}

static void
gthree_orbit_controls_rotate_up (GthreeOrbitControls *orbit,
                            float angle)
{
  orbit->sphericalDelta.phi -= angle;
}

static void
gthree_orbit_controls_pan_left (GthreeOrbitControls *orbit,
                         float distance,
                         const graphene_matrix_t *object_matrix)
{
  graphene_vec4_t row;
  graphene_vec3_t v;

  graphene_matrix_get_row (object_matrix, 0, &row);
  graphene_vec4_get_xyz (&row, &v);
  graphene_vec3_scale (&v, -distance, &v);
  graphene_vec3_add (&orbit->pan_offset, &v, &orbit->pan_offset);
}

static void
gthree_orbit_controls_pan_up (GthreeOrbitControls *orbit,
                       float distance,
                       const graphene_matrix_t *object_matrix)
{
  graphene_vec4_t row;
  graphene_vec3_t v;

  if (orbit->screenSpacePanning)
    {
      graphene_matrix_get_row (object_matrix, 1, &row);
      graphene_vec4_get_xyz (&row, &v);
    }
  else
    {
      graphene_matrix_get_row (object_matrix, 0, &row);
      graphene_vec4_get_xyz (&row, &v);

      graphene_vec3_cross (&v, gthree_object_get_up (orbit->object), &v);
    }

  graphene_vec3_scale (&v, distance, &v);
  graphene_vec3_add (&orbit->pan_offset, &v, &orbit->pan_offset);
}

// deltaX and deltaY are in pixels; right and down are positive
static void
gthree_orbit_controls_pan (GthreeOrbitControls *orbit,
                    float deltaX, float deltaY)
{
  if (GTHREE_IS_PERSPECTIVE_CAMERA (orbit->object))
    {
      const graphene_vec3_t *position = gthree_object_get_position (orbit->object);
      graphene_vec3_t offset;
      int height = gtk_widget_get_allocated_height (orbit->darea);
      float targetDistance;
      float fov = gthree_perspective_camera_get_fov (GTHREE_PERSPECTIVE_CAMERA (orbit->object));

      graphene_vec3_subtract (position, &orbit->target, &offset);
      targetDistance = graphene_vec3_length (&offset);

      // half of the fov is center to top of screen
      targetDistance *= tanf ((fov / 2) * G_PI / 180.0);

      // we use only clientHeight here so aspect ratio does not distort speed
      gthree_orbit_controls_pan_left (orbit, 2 * deltaX * targetDistance / height, gthree_object_get_matrix (orbit->object));
      gthree_orbit_controls_pan_up (orbit, 2 * deltaY * targetDistance / height, gthree_object_get_matrix (orbit->object));
    }
  else if (GTHREE_IS_ORTHOGRAPHIC_CAMERA (orbit->object))
    {
      GthreeOrthographicCamera *orthographic = GTHREE_ORTHOGRAPHIC_CAMERA (orbit->object);
      int width = gtk_widget_get_allocated_width (orbit->darea);
      int height = gtk_widget_get_allocated_height (orbit->darea);
      float object_zoom = 1.0; // TODO: orbit->object.zoom
      gthree_orbit_controls_pan_left (orbit,
                               deltaX * (gthree_orthographic_camera_get_right (orthographic) - gthree_orthographic_camera_get_left (orthographic)) / object_zoom / width,
                               gthree_object_get_matrix (orbit->object));
      gthree_orbit_controls_pan_left (orbit,
                               deltaY * (gthree_orthographic_camera_get_top (orthographic) - gthree_orthographic_camera_get_bottom (orthographic)) / object_zoom / height,
                               gthree_object_get_matrix (orbit->object));
    }
  else
    {
      // camera neither orthographic nor perspective
      g_warning ("WARNING: GthreeOrbitControls encountered an unknown camera type - pan disabled.");
      orbit->enablePan = FALSE;
    }
}

static void
gthree_orbit_controls_dolly_in (GthreeOrbitControls *orbit,
                         float dollyScale)
{
  if (GTHREE_IS_PERSPECTIVE_CAMERA (orbit->object))
    {
      orbit->scale /= dollyScale;
    }
  else if (GTHREE_IS_ORTHOGRAPHIC_CAMERA (orbit->object))
    {
      g_warning ("WARNING: GthreeOrbitControls dolly not supported for orthographic cameras yet.");
      orbit->enableZoom = FALSE;
      // TODO: Handle
      //scope.object.zoom = Math.max( scope.minZoom, Math.min( scope.maxZoom, scope.object.zoom * dollyScale ) );
      //scope.object.updateProjectionMatrix();
      //zoomChanged = true;
    }
  else
    {
      // camera neither orthographic nor perspective
      g_warning ("WARNING: GthreeOrbitControls encountered an unknown camera type - dolly disabled.");
      orbit->enableZoom = FALSE;
    }
}

static void
gthree_orbit_controls_dolly_out (GthreeOrbitControls *orbit,
                          float dollyScale)
{
  gthree_orbit_controls_dolly_in (orbit, 1.0 / dollyScale);
}


static float
get_zoom_scale (GthreeOrbitControls *orbit)
{
  return powf (0.95, orbit->zoomSpeed);
}

static void
drag_update_cb (GtkGestureDrag *gesture,
                gdouble         offset_x,
                gdouble         offset_y,
                gpointer        user_data)
{
  GthreeOrbitControls *orbit = user_data;
  int height = gtk_widget_get_allocated_height (orbit->darea);
  double x, y;
  graphene_vec2_t dragEnd;
  graphene_vec2_t delta;

  if (!orbit->enabled)
    return;

  if (!gtk_gesture_drag_get_offset (gesture, &x, &y))
    return;
  graphene_vec2_init (&dragEnd, x, y);

  switch (orbit->state)
    {
    case STATE_ROTATE:
      if (!orbit->enableRotate)
        return;

      graphene_vec2_subtract (&dragEnd, &orbit->dragStart, &delta);
      graphene_vec2_scale (&delta, orbit->rotateSpeed, &delta);
      gthree_orbit_controls_rotate_left (orbit, 2 * G_PI * graphene_vec2_get_x (&delta) / height ); // yes, height
      gthree_orbit_controls_rotate_up (orbit, 2 * G_PI * graphene_vec2_get_y (&delta) / height );

      break;

    case STATE_DOLLY:
      if (!orbit->enableZoom)
        return;

      graphene_vec2_subtract (&dragEnd, &orbit->dragStart, &delta);
      graphene_vec2_scale (&delta, orbit->rotateSpeed, &delta);

      if (graphene_vec2_get_y (&delta) > 0 )
        gthree_orbit_controls_dolly_in (orbit, get_zoom_scale (orbit));
      else if (graphene_vec2_get_y (&delta) < 0 )
        gthree_orbit_controls_dolly_out (orbit, get_zoom_scale (orbit));

      break;

    case STATE_PAN:
      if (!orbit->enablePan)
        return;

      graphene_vec2_subtract (&dragEnd, &orbit->dragStart, &delta);
      graphene_vec2_scale (&delta, orbit->panSpeed, &delta);
      gthree_orbit_controls_pan (orbit, graphene_vec2_get_x (&delta),graphene_vec2_get_y (&delta));
      break;
    }

  if (gthree_orbit_controls_update (orbit))
    gtk_widget_queue_draw (orbit->darea);

  orbit->dragStart = dragEnd;
}

static void
drag_end_cb (GtkGestureDrag *gesture,
             gdouble         offset_x,
             gdouble         offset_y,
             gpointer        user_data)
{
  GthreeOrbitControls *orbit = user_data;

  orbit->state = STATE_NONE;

  if (orbit->enabled)
    {
      //scope.dispatchEvent( endEvent );
    }
}

#ifdef USE_GTK4
static void
scroll_cb (GtkEventControllerScroll *controller,
           gdouble                   dx,
           gdouble                   dy,
           gpointer                  user_data)
{
  GthreeOrbitControls *orbit = user_data;

  if (!orbit->enabled || !orbit->enableZoom ||
      (orbit->state != STATE_NONE && orbit->state != STATE_ROTATE))
    return;

  if (dy < 0)
    {
      gthree_orbit_controls_dolly_in (orbit, get_zoom_scale (orbit));
    }
  else if (dy > 0)
    {
      gthree_orbit_controls_dolly_out (orbit, get_zoom_scale (orbit));
    }

  if (gthree_orbit_controls_update (orbit))
    gtk_widget_queue_draw (orbit->darea);
}
#else
static gboolean
scroll_event_cb (GtkWidget   *widget,
                 GdkEvent    *event,
                 gpointer     user_data)
{
  GthreeOrbitControls *orbit = user_data;
  GdkScrollDirection direction;

  if (!orbit->enabled || !orbit->enableZoom ||
      (orbit->state != STATE_NONE && orbit->state != STATE_ROTATE))
    return TRUE;

  if (gdk_event_get_scroll_direction (event, &direction))
    {
      if (direction == GDK_SCROLL_UP)
        gthree_orbit_controls_dolly_in (orbit, get_zoom_scale (orbit));
      else if (direction == GDK_SCROLL_DOWN)
        gthree_orbit_controls_dolly_out (orbit, get_zoom_scale (orbit));

      if (gthree_orbit_controls_update (orbit))
        gtk_widget_queue_draw (orbit->darea);
    }

  return TRUE;
}
#endif

static void
gthree_orbit_controls_finalize (GObject *obj)
{
  GthreeOrbitControls *orbit = GTHREE_ORBIT_CONTROLS (obj);

  if (orbit->tick_id != 0)
    {
      gtk_widget_remove_tick_callback (orbit->darea, orbit->tick_id);
      orbit->tick_id = 0;
    }

  // TODO: Disconnect all signal handlers

  g_object_unref (orbit->object);
  g_object_unref (orbit->darea);

  G_OBJECT_CLASS (gthree_orbit_controls_parent_class)->finalize (obj);
}

static void
gthree_orbit_controls_class_init (GthreeOrbitControlsClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = gthree_orbit_controls_finalize;

  object_signals[CHANGED] =
    g_signal_new ("changed",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

static void
gthree_orbit_controls_init (GthreeOrbitControls *orbit)
{
  orbit->enabled = TRUE;

  graphene_vec3_init (&orbit->target, 0, 0, 0);

  orbit->minDistance = 0;
  orbit->maxDistance = G_MAXFLOAT;
  orbit->minZoom = 0;
  orbit->maxZoom = G_MAXFLOAT;
  orbit->minPolarAngle = 0;
  orbit->maxPolarAngle = G_PI;
  orbit->minAzimuthAngle = -G_MAXFLOAT;
  orbit->maxAzimuthAngle = G_MAXFLOAT;
  orbit->enableDamping = FALSE;
  orbit->dampingFactor = 0.25;
  orbit->enableZoom = TRUE;
  orbit->zoomSpeed = 1.0;
  orbit->enableRotate = TRUE;
  orbit->rotateSpeed = 1.0;

  orbit->enablePan = TRUE;
  orbit->panSpeed = 1.0;
  orbit->screenSpacePanning = FALSE;
  orbit->keyPanSpeed = 7.0;

  orbit->autoRotate = FALSE;
  orbit->autoRotateSpeed = 2.0;

  orbit->enableKeys = TRUE;

  // for reset
  graphene_vec3_init (&orbit->target0, 0, 0, 0);
  graphene_vec3_init (&orbit->position0, 0, 0, 0);
  orbit->zoom0 = 1.0;

  spherical_init (&orbit->sphericalDelta, 0, 0, 0);
  orbit->scale = 1;
}

GthreeOrbitControls *
gthree_orbit_controls_new (GthreeObject *object, GtkWidget *darea)
{
  GthreeOrbitControls *orbit;
  GtkEventController *drag;
#ifdef USE_GTK4
  GtkEventController *scroll;
#endif

  orbit = g_object_new (GTHREE_TYPE_ORBIT_CONTROLS, NULL);

  orbit->object = g_object_ref (object);
  orbit->darea = g_object_ref (darea);

  orbit->position0 = *gthree_object_get_position (object);

  drag = drag_controller_for (darea);
  gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (drag), 0);

  g_signal_connect (drag, "drag-begin", (GCallback)drag_begin_cb, orbit);
  g_signal_connect (drag, "drag-update", (GCallback)drag_update_cb, orbit);
  g_signal_connect (drag, "drag-end", (GCallback)drag_end_cb, orbit);

#ifdef USE_GTK4
  scroll = scroll_controller_for (GTK_WIDGET (darea));
  g_signal_connect (scroll, "scroll", (GCallback)scroll_cb, orbit);
#else
  /* TODO: For whatever reason the scroll controller doesn't seem to work for gtk3 */
  gtk_widget_add_events (orbit->darea,
                         GDK_SCROLL_MASK
                         | GDK_BUTTON_PRESS_MASK
                         | GDK_BUTTON_RELEASE_MASK
                         | GDK_POINTER_MOTION_MASK);

  g_signal_connect (orbit->darea, "scroll-event",
                    G_CALLBACK (scroll_event_cb), orbit);
#endif

  return orbit;
}

static float
distanceToSquared (const graphene_vec3_t *a,
                   const graphene_vec3_t *b)
{
  graphene_vec3_t diff;
  graphene_vec3_subtract (a, b, &diff);
  return graphene_vec3_dot (&diff, &diff);
}

static gboolean
gthree_orbit_controls_update (GthreeOrbitControls *orbit)
{
  const graphene_vec3_t *position = gthree_object_get_position (orbit->object);
  const graphene_vec3_t *up = gthree_object_get_up (orbit->object);
  graphene_vec3_t offset;
  graphene_vec3_t new_position;
  graphene_quaternion_t quat, quat_inverse;
  Spherical spherical;

  graphene_vec3_subtract (position, &orbit->target, &offset);

  // rotate offset to "y-axis-is-up" space
  quaternion_from_unit_vectors (&quat, up, graphene_vec3_y_axis ());
  graphene_quaternion_invert (&quat, &quat_inverse);
  vec3_apply_quaternion (&offset, &quat, &offset);

  // angle from z-axis around y-axis
  spherical_set_from_vec3 (&spherical, &offset);

  if (orbit->autoRotate && orbit->state == STATE_NONE)
    gthree_orbit_controls_rotate_left (orbit, 2 * G_PI / 60 / 60 * orbit->autoRotateSpeed);

  spherical.theta += orbit->sphericalDelta.theta;
  spherical.phi += orbit->sphericalDelta.phi;

  // restrict theta to be between desired limits
  spherical.theta = fmaxf (orbit->minAzimuthAngle, fminf (orbit->maxAzimuthAngle, spherical.theta));

  // restrict phi to be between desired limits
  spherical.phi = fmaxf (orbit->minPolarAngle, fminf (orbit->maxPolarAngle, spherical.phi));

  spherical_make_safe (&spherical);

  spherical.radius *= orbit->scale;

  // restrict radius to be between desired limits
  spherical.radius = fmaxf (orbit->minDistance, fminf (orbit->maxDistance, spherical.radius));

  // move target to panned location
  graphene_vec3_add (&orbit->target, &orbit->pan_offset, &orbit->target);

  vec3_init_from_spherical (&offset, &spherical);

  // rotate offset back to "camera-up-vector-is-up" space
  vec3_apply_quaternion (&offset, &quat_inverse, &offset);

  graphene_vec3_add (&orbit->target, &offset, &new_position);
  gthree_object_set_position (orbit->object, &new_position);
  gthree_object_look_at (orbit->object, &orbit->target);

  if (orbit->enableDamping)
    {
      orbit->sphericalDelta.theta *= (1 - orbit->dampingFactor);
      orbit->sphericalDelta.phi *= (1 - orbit->dampingFactor);
      graphene_vec3_scale (&orbit->pan_offset, 1 - orbit->dampingFactor, &orbit->pan_offset);
    }
  else
    {
      spherical_init (&orbit->sphericalDelta, 0, 0, 0);
      graphene_vec3_init (&orbit->pan_offset, 0, 0, 0);
    }

  orbit->scale = 1;

  // update condition is:
  // min(camera displacement, camera rotation in radians)^2 > EPS
  // using small-angle approximation cos(x/2) = 1 - x^2 / 8
  if (orbit->zoomChanged ||
      distanceToSquared (&orbit->last_position, gthree_object_get_position (orbit->object)) > EPS ||
      8 * ( 1 - graphene_quaternion_dot (&orbit->last_quaternion, gthree_object_get_quaternion (orbit->object)) ) > EPS )
    {
      orbit->last_position = *gthree_object_get_position (orbit->object);
      orbit->last_quaternion = *gthree_object_get_quaternion (orbit->object);
      orbit->zoomChanged = FALSE;

      g_signal_emit (G_OBJECT (orbit), object_signals[CHANGED], 0);

      return TRUE;
    }

  return FALSE;
}

static gboolean
tick_cb (GtkWidget     *widget,
         GdkFrameClock *frame_clock,
         gpointer       user_data)
{
  GthreeOrbitControls *orbit = user_data;

  if (gthree_orbit_controls_update (orbit))
    gtk_widget_queue_draw (orbit->darea);

  return TRUE;
}

static void
gthree_orbit_controls_update_tick (GthreeOrbitControls *orbit)
{
  gboolean needs_tick = orbit->enableDamping || orbit->autoRotate;

  if (needs_tick)
    {
      if (orbit->tick_id == 0)
        orbit->tick_id = gtk_widget_add_tick_callback (orbit->darea, tick_cb, orbit, NULL);
    }
  else
    {
      if (orbit->tick_id != 0)
        {
          gtk_widget_remove_tick_callback (orbit->darea, orbit->tick_id);
          orbit->tick_id = 0;
        }
    }
}

void
gthree_orbit_controls_set_enable_damping (GthreeOrbitControls *orbit,
                                          gboolean enable_damping)
{
  orbit->enableDamping = enable_damping;
  gthree_orbit_controls_update_tick (orbit);
}

void
gthree_orbit_controls_set_auto_rotate (GthreeOrbitControls *orbit,
                                       gboolean auto_rotate)
{
  orbit->autoRotate = auto_rotate;
  gthree_orbit_controls_update_tick (orbit);
}

void
gthree_orbit_controls_set_auto_rotate_speed (GthreeOrbitControls *orbit,
                                             float speed)
{
  orbit->autoRotateSpeed = speed;
}

void
gthree_orbit_controls_set_damping_factor (GthreeOrbitControls *orbit,
                                          float damping_factor)
{
  orbit->dampingFactor = damping_factor;
}

void
gthree_orbit_controls_set_target (GthreeOrbitControls    *orbit,
                                  const graphene_vec3_t *target)
{
  orbit->target = *target;
}

void
gthree_orbit_controls_set_min_distance (GthreeOrbitControls    *orbit,
                                        float                  min)
{
  orbit->minDistance = min;
}

void
gthree_orbit_controls_set_max_distance (GthreeOrbitControls    *orbit,
                                        float                  max)
{
  orbit->maxDistance = max;
}

void
gthree_orbit_controls_set_min_zoom (GthreeOrbitControls    *orbit,
                                    float                  min)
{
  orbit->minZoom = min;
}

void
gthree_orbit_controls_set_max_zoom (GthreeOrbitControls    *orbit,
                                    float                  max)
{
  orbit->maxZoom = max;
}

void
gthree_orbit_controls_set_min_polar_angle (GthreeOrbitControls    *orbit,
                                           float                  min)
{
  orbit->minPolarAngle = min;
}

void
gthree_orbit_controls_set_max_polar_angle (GthreeOrbitControls    *orbit,
                                           float                  max)
{
  orbit->maxPolarAngle = max;
}

void
gthree_orbit_controls_set_enable_zoom (GthreeOrbitControls    *orbit,
                                       gboolean               enable)
{
  orbit->enableZoom = enable;
}

void
gthree_orbit_controls_set_enable_rotate (GthreeOrbitControls    *orbit,
                                         gboolean               enable)
{
  orbit->enableRotate = enable;
}

void
gthree_orbit_controls_set_enable_pan (GthreeOrbitControls    *orbit,
                                      gboolean               enable)
{
  orbit->enablePan = enable;
}

void
gthree_orbit_controls_set_screen_space_panning (GthreeOrbitControls    *orbit,
                                                gboolean               screen_space_panning)
{
}
