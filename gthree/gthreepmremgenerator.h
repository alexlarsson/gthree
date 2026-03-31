#ifndef __GTHREE_PMREM_GENERATOR_H__
#define __GTHREE_PMREM_GENERATOR_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreetypes.h>
#include <gthree/gthreetexture.h>
#include <gthree/gthreecubetexture.h>
#include <gthree/gthreerenderer.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_PMREM_GENERATOR      (gthree_pmrem_generator_get_type ())
#define GTHREE_PMREM_GENERATOR(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                      GTHREE_TYPE_PMREM_GENERATOR, \
                                                                      GthreePMREMGenerator))
#define GTHREE_IS_PMREM_GENERATOR(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                      GTHREE_TYPE_PMREM_GENERATOR))

typedef struct _GthreePMREMGenerator GthreePMREMGenerator;
typedef struct _GthreePMREMGeneratorClass GthreePMREMGeneratorClass;

struct _GthreePMREMGenerator {
  GObject parent;
};

struct _GthreePMREMGeneratorClass {
  GObjectClass parent_class;
};

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreePMREMGenerator, g_object_unref)

GTHREE_API
GType gthree_pmrem_generator_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreePMREMGenerator *gthree_pmrem_generator_new (GthreeRenderer *renderer);

GTHREE_API
GthreeTexture *gthree_pmrem_generator_from_cubemap (GthreePMREMGenerator *generator,
                                                    GthreeCubeTexture    *cubemap);

GTHREE_API
void gthree_pmrem_generator_dispose (GthreePMREMGenerator *generator);

G_END_DECLS

#endif /* __GTHREE_PMREM_GENERATOR_H__ */
