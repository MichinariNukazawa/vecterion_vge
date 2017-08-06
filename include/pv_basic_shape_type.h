/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_PV_BASIC_SHAPE_TYPE_H
#define include_PV_BASIC_SHAPE_TYPE_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>


typedef enum{
	PvBasicShapeKind_FigureShape,
	PvBasicShapeKind_Raster,
}PvBasicShapeKind;

typedef enum{
	PvElementBasicShapeAppearanceIndex_Translate,
	PvElementBasicShapeAppearanceIndex_Resize,
	PvElementBasicShapeAppearanceIndex_Rotate,
}PvElementBasicShapeAppearanceIndex;
#define Num_PvElementBasicShapeAppearance (3)


typedef struct{
	int dummy;
}PvFigureShapeData;

typedef struct{
	char		*path;
	GdkPixbuf	*pixbuf;
	GByteArray	*urischeme_byte_array;
}PvRasterData;


#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_BASIC_SHAPE_TYPE_H

