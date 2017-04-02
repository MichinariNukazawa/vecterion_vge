/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_ET_TOOL_ID_H
#define include_ET_TOOL_ID_H

typedef int EtToolId;

enum{
	EtToolId_EditElement,
	EtToolId_EditAnchorPoint,
	EtToolId_AddAnchorPoint,
	EtToolId_EditAnchorPointHandle,
	EtToolId_AddBasicShapeElement,
	EtToolId_KnifeAnchorPoint,
	EtToolId_InsertAnchorPoint,
};

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_TOOL_ID_H

