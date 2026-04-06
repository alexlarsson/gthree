#ifndef __GTHREE_PMREM_GENERATOR_PRIVATE_H__
#define __GTHREE_PMREM_GENERATOR_PRIVATE_H__

#include <gthree/gthreetypes.h>
#include <gthree/gthreetexture.h>
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

GType gthree_pmrem_generator_get_type (void) G_GNUC_CONST;

GthreePMREMGenerator *gthree_pmrem_generator_new (GthreeRenderer *renderer);
void gthree_pmrem_generator_unrealize (GthreePMREMGenerator *gen);

GthreeTexture *gthree_pmrem_generator_from_cubemap (GthreePMREMGenerator *generator,
                                                    GthreeTexture        *cube_texture,
                                                    int                   cube_size);

G_END_DECLS

#endif /* __GTHREE_PMREM_GENERATOR_PRIVATE_H__ */
