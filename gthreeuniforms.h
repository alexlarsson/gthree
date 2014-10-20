#ifndef __GTHREE_UNIFORMS_H__
#define __GTHREE_UNIFORMS_H__

#include <gtk/gtk.h>
#include <graphene.h>

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
  GTHREE_UNIFORM_TYPE_COLOR,
  GTHREE_UNIFORM_TYPE_VEC2_ARRAY,
  GTHREE_UNIFORM_TYPE_VEC3_ARRAY,
  GTHREE_UNIFORM_TYPE_VEC4_ARRAY,
  GTHREE_UNIFORM_TYPE_MATRIX3_ARRAY,
  GTHREE_UNIFORM_TYPE_MATRIX4_ARRAY,
  GTHREE_UNIFORM_TYPE_TEXTURE,
  GTHREE_UNIFORM_TYPE_TEXTURE_ARRAY,
} GthreeUniformType;

typedef struct _GthreeUniform GthreeUniform;

typedef struct {
  GObject parent;
} GthreeUniforms;

typedef struct {
  GObjectClass parent_class;

} GthreeUniformsClass;

GType gthree_uniforms_get_type (void) G_GNUC_CONST;

typedef struct {
  const char *name;
  GthreeUniformType type;
  gpointer value;
} GthreeUniformsDefinition;

GthreeUniforms *gthree_uniforms_new   ();
GthreeUniforms *gthree_uniforms_new_from_definitions (GthreeUniformsDefinition *element, int len);

GthreeUniforms *gthree_get_uniforms_from_library (const char *name);

GthreeUniforms *gthree_uniforms_clone (GthreeUniforms *uniforms);
void            gthree_uniforms_merge (GthreeUniforms *uniforms,
                                       GthreeUniforms *source);
void            gthree_uniforms_add   (GthreeUniforms *uniforms,
                                       GthreeUniform  *uniform);
void           gthree_uniforms_load   (GthreeUniforms *uniforms);
GthreeUniform *gthree_uniforms_lookup (GthreeUniforms *uniforms,
                                       GQuark name);
GList  *gthree_uniforms_get_all (GthreeUniforms *uniforms);
GthreeUniform *gthree_uniforms_lookup_from_string (GthreeUniforms *uniforms,
                                                   const char *name);

void gthree_uniform_set_location (GthreeUniform *uniform,
                                  int location);
void gthree_uniform_set_float (GthreeUniform *uniform,
                               double value);
void gthree_uniform_set_color (GthreeUniform *uniform,
                               GdkRGBA *color);

const char *gthree_uniform_get_name (GthreeUniform *uniform);
GQuark gthree_uniform_get_qname (GthreeUniform *uniform);
void gthree_uniform_load (GthreeUniform *uniform);


G_END_DECLS

#endif /* __GTHREE_UNIFORMS_H__ */
