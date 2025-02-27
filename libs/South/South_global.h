#ifndef SOUTH_GLOBAL_H
#define SOUTH_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(SOUTH_LIBRARY)
#define SOUTH_EXPORT Q_DECL_EXPORT
#else
#define SOUTH_EXPORT Q_DECL_IMPORT
#endif

#endif // SOUTH_GLOBAL_H
