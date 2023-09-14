#ifndef QRETRO_GLOBAL_H
#define QRETRO_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QRETRO_INTERNAL)
#  define QRETRO_EXPORT
#elif defined(QRETRO_LIBRARY)
#  define QRETRO_EXPORT Q_DECL_EXPORT
#else
#  define QRETRO_EXPORT Q_DECL_IMPORT
#endif

#endif
