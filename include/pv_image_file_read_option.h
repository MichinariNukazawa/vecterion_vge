/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_PV_IMAGE_FILE_READ_OPTION_H
#define include_PV_IMAGE_FILE_READ_OPTION_H

typedef struct{
	bool is_strict;
}PvImageFileReadOption;

static const PvImageFileReadOption PvImageFileReadOption_Default = {
	false,
};

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_IMAGE_FILE_READ_OPTION_H

