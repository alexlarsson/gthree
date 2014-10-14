all: gtkglobe

SOURCES= gtkglobe.c \
	gthreerenderer.h gthreerenderer.c gthreecamera.c gthreecamera.h \
	gthreescene.c gthreescene.h gthreeobject.h gthreeobject.c \
	gthreegeometry.h gthreegeometry.c gthreegeometry-utils.c \
	gthreeprivate.h gthreeface.c gthreeface.h \
	gthreemesh.h gthreemesh.c gthreematerial.h gthreematerial.c \
	gthreearea.h gthreearea.c

gtkglobe: ${SOURCES}
	gcc `pkg-config --cflags --libs gtk+-3.0 epoxy json-glib-1.0 graphene-1.0` ${SOURCES} -O1 -Wall -o gtkglobe
