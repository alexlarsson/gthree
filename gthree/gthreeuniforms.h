#ifndef __GTHREE_UNIFORMS_H__
#define __GTHREE_UNIFORMS_H__

#include <glib-object.h>
#include <gdk/gdk.h>
#include <graphene.h>

#include <gthree/gthreetypes.h>
#include <gthree/gthreetexture.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_UNIFORMS      (gthree_uniforms_get_type ())
#define GTHREE_UNIFORMS(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                             GTHREE_TYPE_UNIFORMS, \
                                                             GthreeUniforms))
#define GTHREE_IS_UNIFORMS(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                             GTHREE_TYPE_UNIFORMS))

typedef enum {
  GTHREE_UNIFORM_TYPE_INT,
  GTHREE_UNIFORM_TYPE_FLOAT,
  GTHREE_UNIFORM_TYPE_FLOAT2,
  GTHREE_UNIFORM_TYPE_FLOAT3,
  GTHREE_UNIFORM_TYPE_FLOAT4,
  GTHREE_UNIFORM_TYPE_INT_ARRAY,
  GTHREE_UNIFORM_TYPE_INT3_ARRAY,
  GTHREE_UNIFORM_TYPE_FLOAT_ARRAY,
  GTHREE_UNIFORM_TYPE_FLOAT2_ARRAY,
  GTHREE_UNIFORM_TYPE_FLOAT3_ARRAY,
  GTHREE_UNIFORM_TYPE_FLOAT4_ARRAY,
  GTHREE_UNIFORM_TYPE_MATRIX3,
  GTHREE_UNIFORM_TYPE_MATRIX4,
  GTHREE_UNIFORM_TYPE_VECTOR2,
  GTHREE_UNIFORM_TYPE_VECTOR3,
  GTHREE_UNIFORM_TYPE_VECTOR4,
  GTHREE_UNIFORM_TYPE_VEC2_ARRAY,
  GTHREE_UNIFORM_TYPE_VEC3_ARRAY,
  GTHREE_UNIFORM_TYPE_VEC4_ARRAY,
  GTHREE_UNIFORM_TYPE_MATRIX3_ARRAY,
  GTHREE_UNIFORM_TYPE_MATRIX4_ARRAY,
  GTHREE_UNIFORM_TYPE_TEXTURE,
  GTHREE_UNIFORM_TYPE_TEXTURE_ARRAY,
  GTHREE_UNIFORM_TYPE_UNIFORMS_ARRAY,
} GthreeUniformType;

typedef struct _GthreeUniform GthreeUniform;

typedef struct {
  GObject parent;
} GthreeUniforms;

typedef struct {
  GObjectClass parent_class;

} GthreeUniformsClass;

GTHREE_API
GType gthree_uniforms_get_type (void) G_GNUC_CONST;

typedef struct {
  const char *name;
  GthreeUniformType type;
  gpointer value;
} GthreeUniformsDefinition;

GTHREE_API
GthreeUniforms *gthree_uniforms_new   ();
GTHREE_API
GthreeUniforms *gthree_uniforms_new_from_definitions (GthreeUniformsDefinition *element, int len);

GTHREE_API
GthreeUniforms *gthree_get_uniforms_from_library (const char *name);

GTHREE_API
GthreeUniforms *gthree_uniforms_clone              (GthreeUniforms  *uniforms);
GTHREE_API
void            gthree_uniforms_merge              (GthreeUniforms  *uniforms,
                                                    GthreeUniforms  *source);
GTHREE_API
void            gthree_uniforms_copy_values        (GthreeUniforms *uniforms,
                                                    GthreeUniforms *source);
GTHREE_API
void            gthree_uniforms_add                (GthreeUniforms  *uniforms,
                                                    GthreeUniform   *uniform);
GTHREE_API
void            gthree_uniforms_load               (GthreeUniforms  *uniforms,
                                                    GthreeRenderer  *renderer);
GTHREE_API
GthreeUniform * gthree_uniforms_lookup             (GthreeUniforms  *uniforms,
                                                    GQuark           name);
GTHREE_API
GList  *        gthree_uniforms_get_all            (GthreeUniforms  *uniforms);
GTHREE_API
GthreeUniform * gthree_uniforms_lookup_from_string (GthreeUniforms  *uniforms,
                                                    const char      *name);
GTHREE_API
void            gthree_uniforms_set_float          (GthreeUniforms  *uniforms,
                                                    const char      *name,
                                                    double           value);
GTHREE_API
void            gthree_uniforms_set_float_array    (GthreeUniforms  *uniforms,
                                                    const char      *name,
                                                    GArray          *array);
GTHREE_API
void            gthree_uniforms_set_float3_array   (GthreeUniforms  *uniforms,
                                                    const char      *name,
                                                    GArray          *array);
GTHREE_API
void            gthree_uniforms_set_int            (GthreeUniforms  *uniforms,
                                                    const char      *name,
                                                    int              value);
GTHREE_API
void            gthree_uniforms_set_vec4           (GthreeUniforms  *uniforms,
                                                    const char      *name,
                                                    graphene_vec4_t *value);
GTHREE_API
void            gthree_uniforms_set_vec3           (GthreeUniforms  *uniforms,
                                                    const char      *name,
                                                    graphene_vec3_t *value);
GTHREE_API
void            gthree_uniforms_set_vec2           (GthreeUniforms  *uniforms,
                                                    const char      *name,
                                                    graphene_vec2_t *value);
GTHREE_API
void            gthree_uniforms_set_texture        (GthreeUniforms  *uniforms,
                                                    const char      *name,
                                                    GthreeTexture   *value);
GTHREE_API
void            gthree_uniforms_set_uarray         (GthreeUniforms   *uniforms,
                                                    const char      *name,
                                                    GPtrArray       *uarray,
                                                    gboolean         update_existing);

GTHREE_API
void        gthree_uniform_set_location     (GthreeUniform   *uniform,
                                             int              location);
GTHREE_API
void        gthree_uniform_set_needs_update (GthreeUniform   *uniform,
                                             gboolean         needs_update);
GTHREE_API
void        gthree_uniform_copy_value       (GthreeUniform   *uniform,
                                             GthreeUniform   *source);
GTHREE_API
void        gthree_uniform_set_float        (GthreeUniform   *uniform,
                                             double           value);
GTHREE_API
void        gthree_uniform_set_float_array  (GthreeUniform   *uniform,
                                             GArray          *array);
GTHREE_API
void        gthree_uniform_set_float3_array (GthreeUniform   *uniform,
                                             GArray          *array);
GTHREE_API
void        gthree_uniform_set_float4_array (GthreeUniform   *uniform,
                                             GArray          *array);
GTHREE_API
void        gthree_uniform_set_int          (GthreeUniform   *uniform,
                                             int              value);
GTHREE_API
void        gthree_uniform_set_vec2         (GthreeUniform   *uniform,
                                             graphene_vec2_t *value);
GTHREE_API
void        gthree_uniform_set_vec3         (GthreeUniform   *uniform,
                                             graphene_vec3_t *value);
GTHREE_API
void        gthree_uniform_set_vec4         (GthreeUniform   *uniform,
                                             graphene_vec4_t *value);
GTHREE_API
void        gthree_uniform_set_texture      (GthreeUniform   *uniform,
                                             GthreeTexture   *value);
GTHREE_API
void        gthree_uniform_set_uarray       (GthreeUniform   *uniform,
                                             GPtrArray       *uarray,
                                             gboolean         update_existing);
GTHREE_API
GPtrArray  *gthree_uniform_get_uarray       (GthreeUniform   *uniform);
GTHREE_API
GthreeUniformType gthree_uniform_get_type   (GthreeUniform   *uniform);
GTHREE_API
const char *gthree_uniform_get_name         (GthreeUniform   *uniform);
GTHREE_API
GQuark      gthree_uniform_get_qname        (GthreeUniform   *uniform);
GTHREE_API
void        gthree_uniform_load             (GthreeUniform   *uniform,
                                             GthreeRenderer  *renderer);

G_END_DECLS

#endif /* __GTHREE_UNIFORMS_H__ */
