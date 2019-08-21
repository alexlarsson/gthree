#ifndef __GTHREE_TYPES_H__
#define __GTHREE_TYPES_H__

typedef struct _GthreeScene GthreeScene;
typedef struct _GthreeCamera GthreeCamera;
typedef struct _GthreeGroup GthreeGroup;
typedef struct _GthreeBone GthreeBone;
typedef struct _GthreeSkeleton GthreeSkeleton;
typedef struct _GthreePerspectiveCamera GthreePerspectiveCamera;
typedef struct _GthreeOrthographicCamera GthreeOrthographicCamera;
typedef struct _GthreeRenderer GthreeRenderer;
typedef struct _GthreeMaterial GthreeMaterial;
typedef struct _GthreeSpriteMaterial GthreeSpriteMaterial;
typedef struct _GthreePointsMaterial GthreePointsMaterial;
typedef struct _GthreeMeshMaterial GthreeMeshMaterial;
typedef struct _GthreeMeshBasicMaterial GthreeMeshBasicMaterial;
typedef struct _GthreeMeshLambertMaterial GthreeMeshLambertMaterial;
typedef struct _GthreeMeshPhongMaterial GthreeMeshPhongMaterial;
typedef struct _GthreeMeshStandardMaterial GthreeMeshStandardMaterial;
typedef struct _GthreeMeshNormalMaterial GthreeMeshNormalMaterial;
typedef struct _GthreeMeshDepthMaterial GthreeMeshDepthMaterial;
typedef struct _GthreeMeshDistanceMaterial GthreeMeshDistanceMaterial;
typedef struct _GthreeShaderMaterial GthreeShaderMaterial;
typedef struct _GthreeProgram GthreeProgram;
typedef struct _GthreeAmbientLight GthreeAmbientLight;
typedef struct _GthreePointLight GthreePointLight;
typedef struct _GthreeSpotLight GthreeSpotLight;
typedef struct _GthreeDirectionalLight GthreeDirectionalLight;
typedef struct _GthreeLightShadow GthreeLightShadow;
typedef struct _GthreeLightSetup GthreeLightSetup;
typedef struct _GthreeResource GthreeResource;
typedef struct _GthreeTexture GthreeTexture;
typedef struct _GthreeCubeTexture GthreeCubeTexture;
typedef struct _GthreeGeometry GthreeGeometry;
typedef struct _GthreeAttribute GthreeAttribute;
typedef struct _GthreeAttributeArray GthreeAttributeArray;
typedef struct _GthreeRenderList GthreeRenderList;
typedef struct _GthreeMaterialProperties GthreeMaterialProperties;
typedef struct _GthreeAnimationAction GthreeAnimationAction;
typedef struct _GthreeAnimationMixer GthreeAnimationMixer;
typedef struct _GthreeRaycaster GthreeRaycaster;
typedef int GthreeAttributeName;

#if defined (_MSC_VER) && defined (GTHREE_COMPILATION)
# define GTHREE_API __declspec(dllexport)
#else
# define GTHREE_API
#endif

#endif /* __GTHREE_TYPES_H__ */
