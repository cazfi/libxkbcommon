/*
Copyright (c) 1993 by Silicon Graphics Computer Systems, Inc.

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of Silicon Graphics not be
used in advertising or publicity pertaining to distribution
of the software without specific prior written permission.
Silicon Graphics makes no representation about the suitability
of this software for any purpose. It is provided "as is"
without any express or implied warranty.

SILICON GRAPHICS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SILICON
GRAPHICS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "xkbgeom.h"
#include "X11/extensions/XKBcommon.h"
#include "XKBcommonint.h"
#include <X11/extensions/XKB.h>

static void
_XkbFreeGeomLeafElems(Bool freeAll, int first, int count,
                      unsigned short *num_inout, unsigned short *sz_inout,
                      char **elems, unsigned int elem_sz)
{
    if (freeAll || !(*elems)) {
        *num_inout = *sz_inout = 0;
        if (*elems) {
            _XkbFree(*elems);
            *elems = NULL;
        }
        return;
    }

    if ((first >= (*num_inout)) || (first < 0) || (count < 1))
        return;

    if (first + count >= (*num_inout))
        /* truncating the array is easy */
        (*num_inout) = first;
    else {
        char *ptr = *elems;
        int extra = ((*num_inout) - first + count) * elem_sz;

        if (extra > 0)
            memmove(&ptr[first * elem_sz], &ptr[(first + count) * elem_sz],
                    extra);

        (*num_inout) -= count;
    }
}

typedef void (*ContentsClearFunc)(char *priv);

static void
_XkbFreeGeomNonLeafElems(Bool freeAll, int first, int count,
                         unsigned short *num_inout, unsigned short *sz_inout,
                         char **elems, unsigned int elem_sz,
                         ContentsClearFunc freeFunc)
{
    int i;
    char *ptr;

    if (freeAll) {
        first = 0;
        count = *num_inout;
    }
    else if ((first >= (*num_inout)) || (first < 0) || (count < 1))
        return;
    else if (first + count > (*num_inout))
        count = (*num_inout) - first;

    if (!(*elems))
        return;

    if (freeFunc) {
        ptr = *elems;
        ptr += first * elem_sz;
        for (i = 0; i < count; i++) {
            (*freeFunc)(ptr);
            ptr += elem_sz;
        }
    }

    if (freeAll) {
        *num_inout = *sz_inout = 0;
        if (*elems) {
            _XkbFree(*elems);
            *elems = NULL;
        }
    }
    else if (first + count >= (*num_inout))
        *num_inout = first;
    else {
        i = ((*num_inout) - first + count) * elem_sz;
        ptr = *elems;
        memmove(&ptr[first * elem_sz], &ptr[(first + count) * elem_sz], i);
        (*num_inout) -= count;
    }
}

static void
_XkbClearProperty(char *prop_in)
{
    XkbPropertyPtr prop = (XkbPropertyPtr)prop_in;

    if (prop->name) {
        _XkbFree(prop->name);
        prop->name = NULL;
    }
    if (prop->value) {
        _XkbFree(prop->value);
        prop->value = NULL;
    }
}

void
XkbcFreeGeomProperties(XkbGeometryPtr geom, int first, int count, Bool freeAll)
{
    _XkbFreeGeomNonLeafElems(freeAll, first, count,
                             &geom->num_properties, &geom->sz_properties,
                             (char **)&geom->properties,
                             sizeof(XkbPropertyRec),
                             _XkbClearProperty);
}

void
XkbcFreeGeomKeyAliases(XkbGeometryPtr geom, int first, int count, Bool freeAll)
{
    _XkbFreeGeomLeafElems(freeAll, first, count,
                          &geom->num_key_aliases, &geom->sz_key_aliases,
                          (char **)&geom->key_aliases,
                          sizeof(XkbKeyAliasRec));
}

static void
_XkbClearColor(char *color_in)
{
    XkbColorPtr color = (XkbColorPtr)color_in;

    if (color->spec)
        _XkbFree(color->spec);
}

void
XkbcFreeGeomColors(XkbGeometryPtr geom, int first, int count, Bool freeAll)
{
    _XkbFreeGeomNonLeafElems(freeAll, first, count,
                             &geom->num_colors, &geom->sz_colors,
                             (char **)&geom->colors, sizeof(XkbColorRec),
                             _XkbClearColor);
}

void
XkbcFreeGeomPoints(XkbOutlinePtr outline, int first, int count, Bool freeAll)
{
    _XkbFreeGeomLeafElems(freeAll, first, count,
                          &outline->num_points, &outline->sz_points,
                          (char **)&outline->points, sizeof(XkbPointRec));
}

static void
_XkbClearOutline(char *outline_in)
{
    XkbOutlinePtr outline = (XkbOutlinePtr)outline_in;

    if (outline->points)
        XkbcFreeGeomPoints(outline, 0, outline->num_points, True);
}

void
XkbcFreeGeomOutlines(XkbShapePtr shape, int first, int count, Bool freeAll)
{
    _XkbFreeGeomNonLeafElems(freeAll, first, count,
                             &shape->num_outlines, &shape->sz_outlines,
                             (char **)&shape->outlines, sizeof(XkbOutlineRec),
                             _XkbClearOutline);
}

static void
_XkbClearShape(char *shape_in)
{
    XkbShapePtr shape = (XkbShapePtr)shape_in;

    if (shape->outlines)
        XkbcFreeGeomOutlines(shape, 0, shape->num_outlines, True);
}

void
XkbcFreeGeomShapes(XkbGeometryPtr geom, int first, int count, Bool freeAll)
{
    _XkbFreeGeomNonLeafElems(freeAll, first, count,
                             &geom->num_shapes, &geom->sz_shapes,
                             (char **)&geom->shapes, sizeof(XkbShapeRec),
                             _XkbClearShape);
}

void
XkbcFreeGeomOverlayKeys(XkbOverlayRowPtr row, int first, int count,
                        Bool freeAll)
{
    _XkbFreeGeomLeafElems(freeAll, first, count,
                          &row->num_keys, &row->sz_keys,
                          (char **)&row->keys, sizeof(XkbOverlayKeyRec));
}


static void
_XkbClearOverlayRow(char *row_in)
{
    XkbOverlayRowPtr row = (XkbOverlayRowPtr)row_in;

    if (row->keys)
        XkbcFreeGeomOverlayKeys(row, 0, row->num_keys, True);
}

void
XkbcFreeGeomOverlayRows(XkbOverlayPtr overlay, int first, int count,
                        Bool freeAll)
{
    _XkbFreeGeomNonLeafElems(freeAll, first, count,
                             &overlay->num_rows, &overlay->sz_rows,
                             (char **)&overlay->rows,
                             sizeof(XkbOverlayRowRec),
                             _XkbClearOverlayRow);
}


static void
_XkbClearOverlay(char *overlay_in)
{
    XkbOverlayPtr overlay = (XkbOverlayPtr)overlay_in;

    if (overlay->rows)
        XkbcFreeGeomOverlayRows(overlay, 0, overlay->num_rows, True);
}

void
XkbcFreeGeomOverlays(XkbSectionPtr section, int first, int count, Bool freeAll)
{
    _XkbFreeGeomNonLeafElems(freeAll, first, count,
                             &section->num_overlays, &section->sz_overlays,
                             (char **)&section->overlays,
                             sizeof(XkbOverlayRec),
                             _XkbClearOverlay);
}


void
XkbcFreeGeomKeys(XkbRowPtr row, int first, int count, Bool freeAll)
{
    _XkbFreeGeomLeafElems(freeAll, first, count,
                          &row->num_keys, &row->sz_keys,
                          (char **)&row->keys, sizeof(XkbKeyRec));
}


static void
_XkbClearRow(char *row_in)
{
    XkbRowPtr row = (XkbRowPtr)row_in;

    if (row->keys)
        XkbcFreeGeomKeys(row, 0, row->num_keys, True);
}

void
XkbcFreeGeomRows(XkbSectionPtr section, int first, int count, Bool freeAll)
{
    _XkbFreeGeomNonLeafElems(freeAll, first, count,
                             &section->num_rows, &section->sz_rows,
                             (char **)&section->rows, sizeof(XkbRowRec),
                             _XkbClearRow);
}


static void
_XkbClearSection(char *section_in)
{
    XkbSectionPtr section = (XkbSectionPtr)section_in;

    if (section->rows)
        XkbcFreeGeomRows(section, 0, section->num_rows, True);
    if (section->doodads) {
        XkbcFreeGeomDoodads(section->doodads, section->num_doodads, True);
        section->doodads = NULL;
    }
}

void
XkbcFreeGeomSections(XkbGeometryPtr geom, int first, int count, Bool freeAll)
{
    _XkbFreeGeomNonLeafElems(freeAll, first, count,
                             &geom->num_sections, &geom->sz_sections,
                             (char **)&geom->sections, sizeof(XkbSectionRec),
                             _XkbClearSection);
}


static void
_XkbClearDoodad(char *doodad_in)
{
    XkbDoodadPtr doodad = (XkbDoodadPtr)doodad_in;

    switch (doodad->any.type) {
    case XkbTextDoodad:
        if (doodad->text.text) {
            _XkbFree(doodad->text.text);
            doodad->text.text = NULL;
        }
        if (doodad->text.font) {
            _XkbFree(doodad->text.font);
            doodad->text.font = NULL;
        }
        break;

    case XkbLogoDoodad:
        if (doodad->logo.logo_name) {
            _XkbFree(doodad->logo.logo_name);
            doodad->logo.logo_name = NULL;
        }
        break;
    }
}

void
XkbcFreeGeomDoodads(XkbDoodadPtr doodads, int nDoodads, Bool freeAll)
{
    int i;
    XkbDoodadPtr doodad;

    if (doodads) {
        for (i = 0, doodad = doodads; i < nDoodads; i++, doodad++)
            _XkbClearDoodad((char *)doodad);
        if (freeAll)
            _XkbFree(doodads);
    }
}

void
XkbcFreeGeometry(XkbGeometryPtr geom, unsigned which, Bool freeMap)
{
    if (!geom)
        return;

    if (freeMap)
        which = XkbGeomAllMask;

    if ((which & XkbGeomPropertiesMask) && geom->properties)
        XkbcFreeGeomProperties(geom, 0, geom->num_properties, True);

    if ((which & XkbGeomColorsMask) && geom->colors)
        XkbcFreeGeomColors(geom, 0, geom->num_colors, True);

    if ((which & XkbGeomShapesMask) && geom->shapes)
        XkbcFreeGeomShapes(geom, 0, geom->num_shapes, True);

    if ((which & XkbGeomSectionsMask) && geom->sections)
        XkbcFreeGeomSections(geom, 0, geom->num_sections, True);

    if ((which & XkbGeomDoodadsMask) && geom->doodads) {
        XkbcFreeGeomDoodads(geom->doodads, geom->num_doodads, True);
        geom->doodads = NULL;
        geom->num_doodads = geom->sz_doodads = 0;
    }

    if ((which & XkbGeomKeyAliasesMask) && geom->key_aliases)
        XkbcFreeGeomKeyAliases(geom, 0, geom->num_key_aliases, True);

    if (freeMap) {
        if (geom->label_font) {
            _XkbFree(geom->label_font);
            geom->label_font = NULL;
        }
        _XkbFree(geom);
    }
}

static int
_XkbGeomAlloc(char **old, unsigned short *num, unsigned short *total,
              int num_new, size_t sz_elem)
{
    if (num_new < 1)
        return Success;

    if (!(*old))
        *num = *total = 0;

    if ((*num) + num_new <= (*total))
        return Success;

    *total = (*num) + num_new;

    if (*old)
        *old = (char *)_XkbRealloc(*old, (*total) * sz_elem);
    else
        *old = (char *)_XkbCalloc(*total, sz_elem);
    if (!(*old)) {
        *total = *num = 0;
        return BadAlloc;
    }

    if (*num > 0) {
        char *tmp = *old;
        bzero(&tmp[sz_elem * (*num)], num_new * sz_elem);
    }

    return Success;
}

#define _XkbAllocProps(g, n)    _XkbGeomAlloc((char **)&(g)->properties, \
                                              &(g)->num_properties, \
                                              &(g)->sz_properties, \
                                              (n), sizeof(XkbPropertyRec))
#define _XkbAllocColors(g, n)   _XkbGeomAlloc((char **)&(g)->colors, \
                                              &(g)->num_colors, \
                                              &(g)->sz_colors, \
                                              (n), sizeof(XkbColorRec))
#define _XkbAllocShapes(g, n)   _XkbGeomAlloc((char **)&(g)->shapes, \
                                              &(g)->num_shapes, \
                                              &(g)->sz_shapes, \
                                              (n), sizeof(XkbShapeRec))
#define _XkbAllocSections(g, n) _XkbGeomAlloc((char **)&(g)->sections, \
                                              &(g)->num_sections, \
                                              &(g)->sz_sections, \
                                              (n), sizeof(XkbSectionRec))
#define _XkbAllocDoodads(g, n)  _XkbGeomAlloc((char **)&(g)->doodads, \
                                              &(g)->num_doodads, \
                                              &(g)->sz_doodads, \
                                              (n), sizeof(XkbDoodadRec))
#define _XkbAllocKeyAliases(g, n)   _XkbGeomAlloc((char **)&(g)->key_aliases, \
                                                  &(g)->num_key_aliases, \
                                                  &(g)->sz_key_aliases, \
                                                  (n), sizeof(XkbKeyAliasRec))

#define _XkbAllocOutlines(s, n) _XkbGeomAlloc((char **)&(s)->outlines, \
                                              &(s)->num_outlines, \
                                              &(s)->sz_outlines, \
                                              (n), sizeof(XkbOutlineRec))
#define _XkbAllocRows(s, n)     _XkbGeomAlloc((char **)&(s)->rows, \
                                              &(s)->num_rows, \
                                              &(s)->sz_rows, \
                                              (n), sizeof(XkbRowRec))
#define _XkbAllocPoints(o, n)   _XkbGeomAlloc((char **)&(o)->points, \
                                              &(o)->num_points, \
                                              &(o)->sz_points, \
                                              (n), sizeof(XkbPointRec))
#define _XkbAllocKeys(r, n)     _XkbGeomAlloc((char **)&(r)->keys, \
                                              &(r)->num_keys, \
                                              &(r)->sz_keys, \
                                              (n), sizeof(XkbKeyRec))
#define _XkbAllocOverlays(s, n) _XkbGeomAlloc((char **)&(s)->overlays, \
                                              &(s)->num_overlays, \
                                              &(s)->sz_overlays, \
                                              (n), sizeof(XkbOverlayRec))
#define _XkbAllocOverlayRows(o, n)  _XkbGeomAlloc((char **)&(o)->rows, \
                                                  &(o)->num_rows, \
                                                  &(o)->sz_rows, \
                                                  (n), sizeof(XkbOverlayRowRec))
#define _XkbAllocOverlayKeys(r, n)  _XkbGeomAlloc((char **)&(r)->keys, \
                                                  &(r)->num_keys, \
                                                  &(r)->sz_keys, \
                                                  (n), sizeof(XkbOverlayKeyRec))

int
XkbcAllocGeomProps(XkbGeometryPtr geom, int nProps)
{
    return _XkbAllocProps(geom, nProps);
}

int
XkbcAllocGeomColors(XkbGeometryPtr geom, int nColors)
{
    return _XkbAllocColors(geom, nColors);
}

int
XkbcAllocGeomKeyAliases(XkbGeometryPtr geom, int nKeyAliases)
{
    return _XkbAllocKeyAliases(geom, nKeyAliases);
}

int
XkbcAllocGeomShapes(XkbGeometryPtr geom, int nShapes)
{
    return _XkbAllocShapes(geom, nShapes);
}

int
XkbcAllocGeomSections(XkbGeometryPtr geom, int nSections)
{
    return _XkbAllocSections(geom, nSections);
}

int
XkbcAllocGeomOverlays(XkbSectionPtr section, int nOverlays)
{
    return _XkbAllocOverlays(section, nOverlays);
}

int
XkbcAllocGeomOverlayRows(XkbOverlayPtr overlay, int nRows)
{
    return _XkbAllocOverlayRows(overlay, nRows);
}

int
XkbcAllocGeomOverlayKeys(XkbOverlayRowPtr row, int nKeys)
{
    return _XkbAllocOverlayKeys(row, nKeys);
}

int
XkbcAllocGeomDoodads(XkbGeometryPtr geom, int nDoodads)
{
    return _XkbAllocDoodads(geom, nDoodads);
}

int
XkbcAllocGeomSectionDoodads(XkbSectionPtr section, int nDoodads)
{
    return _XkbAllocDoodads(section, nDoodads);
}

int
XkbcAllocGeomOutlines(XkbShapePtr shape, int nOL)
{
    return _XkbAllocOutlines(shape, nOL);
}

int
XkbcAllocGeomRows(XkbSectionPtr section, int nRows)
{
    return _XkbAllocRows(section, nRows);
}

int
XkbcAllocGeomPoints(XkbOutlinePtr ol, int nPts)
{
    return _XkbAllocPoints(ol, nPts);
}

int
XkbcAllocGeomKeys(XkbRowPtr row, int nKeys)
{
    int ret = _XkbAllocKeys(row, nKeys);
    fprintf(stderr, "!!! allocated %d keys at %p\n", nKeys, row->keys);
    return ret;
}

int
XkbcAllocGeometry(XkbcDescPtr xkb, XkbGeometrySizesPtr sizes)
{
    XkbGeometryPtr geom;
    int rtrn;

    if (!xkb->geom) {
        xkb->geom = _XkbTypedCalloc(1, XkbGeometryRec);
        if (!xkb->geom)
            return BadAlloc;
    }
    geom = xkb->geom;

    if ((sizes->which & XkbGeomPropertiesMask) &&
        ((rtrn = _XkbAllocProps(geom, sizes->num_properties)) != Success))
        goto bail;

    if ((sizes->which & XkbGeomColorsMask) &&
        ((rtrn = _XkbAllocColors(geom, sizes->num_colors)) != Success))
        goto bail;

    if ((sizes->which & XkbGeomShapesMask) &&
        ((rtrn = _XkbAllocShapes(geom, sizes->num_shapes)) != Success))
        goto bail;

    if ((sizes->which & XkbGeomSectionsMask) &&
        ((rtrn = _XkbAllocSections(geom, sizes->num_sections)) != Success))
        goto bail;

    if ((sizes->which & XkbGeomDoodadsMask) &&
        ((rtrn = _XkbAllocDoodads(geom, sizes->num_doodads)) != Success))
        goto bail;

    if ((sizes->which & XkbGeomKeyAliasesMask) &&
        ((rtrn = _XkbAllocKeyAliases(geom, sizes->num_key_aliases)) != Success))
        goto bail;

    return Success;
bail:
    XkbcFreeGeometry(geom, XkbGeomAllMask, True);
    xkb->geom = NULL;
    return rtrn;
}

XkbPropertyPtr
XkbcAddGeomProperty(XkbGeometryPtr geom,char *name,char *value)
{
register int i;
register XkbPropertyPtr prop;

    if ((!geom)||(!name)||(!value))
	return NULL;
    for (i=0,prop=geom->properties;i<geom->num_properties;i++,prop++) {
	if ((prop->name)&&(strcmp(name,prop->name)==0)) {
	    if (prop->value)
		_XkbFree(prop->value);
	    prop->value= (char *)_XkbAlloc(strlen(value)+1);
	    if (prop->value)
		strcpy(prop->value,value);
	    return prop;
	}
    }
    if ((geom->num_properties>=geom->sz_properties)&&
					(_XkbAllocProps(geom,1)!=Success)) {
	return NULL;
    }
    prop= &geom->properties[geom->num_properties];
    prop->name= (char *)_XkbAlloc(strlen(name)+1);
    if (!name)
	return NULL;
    strcpy(prop->name,name);
    prop->value= (char *)_XkbAlloc(strlen(value)+1);
    if (!value) {
	_XkbFree(prop->name);
	prop->name= NULL;
	return NULL;
    }
    strcpy(prop->value,value);
    geom->num_properties++;
    return prop;
}

XkbKeyAliasPtr
XkbcAddGeomKeyAlias(XkbGeometryPtr geom,char *aliasStr,char *realStr)
{
register int i;
register XkbKeyAliasPtr alias;

    if ((!geom)||(!aliasStr)||(!realStr)||(!aliasStr[0])||(!realStr[0]))
	return NULL;
    for (i=0,alias=geom->key_aliases;i<geom->num_key_aliases;i++,alias++) {
	if (strncmp(alias->alias,aliasStr,XkbKeyNameLength)==0) {
	    bzero(alias->real,XkbKeyNameLength);
	    strncpy(alias->real,realStr,XkbKeyNameLength);
	    return alias;
	}
    }
    if ((geom->num_key_aliases>=geom->sz_key_aliases)&&
				(_XkbAllocKeyAliases(geom,1)!=Success)) {
	return NULL;
    }
    alias= &geom->key_aliases[geom->num_key_aliases];
    bzero(alias,sizeof(XkbKeyAliasRec));
    strncpy(alias->alias,aliasStr,XkbKeyNameLength);
    strncpy(alias->real,realStr,XkbKeyNameLength);
    geom->num_key_aliases++;
    return alias;
}

XkbColorPtr
XkbcAddGeomColor(XkbGeometryPtr geom,char *spec,unsigned int pixel)
{
register int i;
register XkbColorPtr color;

    if ((!geom)||(!spec))
	return NULL;
    for (i=0,color=geom->colors;i<geom->num_colors;i++,color++) {
	if ((color->spec)&&(strcmp(color->spec,spec)==0)) {
	    color->pixel= pixel;
	    return color;
	}
    }
    if ((geom->num_colors>=geom->sz_colors)&&
					(_XkbAllocColors(geom,1)!=Success)) {
	return NULL;
    }
    color= &geom->colors[geom->num_colors];
    color->pixel= pixel;
    color->spec= (char *)_XkbAlloc(strlen(spec)+1);
    if (!color->spec)
	return NULL;
    strcpy(color->spec,spec);
    geom->num_colors++;
    return color;
}

XkbOutlinePtr
XkbcAddGeomOutline(XkbShapePtr shape,int sz_points)
{
XkbOutlinePtr	outline;

    if ((!shape)||(sz_points<0))
	return NULL;
    if ((shape->num_outlines>=shape->sz_outlines)&&
					(_XkbAllocOutlines(shape,1)!=Success)) {
	return NULL;
    }
    outline= &shape->outlines[shape->num_outlines];
    bzero(outline,sizeof(XkbOutlineRec));
    if ((sz_points>0)&&(_XkbAllocPoints(outline,sz_points)!=Success))
	return NULL;
    shape->num_outlines++;
    return outline;
}

XkbShapePtr
XkbcAddGeomShape(XkbGeometryPtr geom,Atom name,int sz_outlines)
{
XkbShapePtr	shape;
register int	i;

    if ((!geom)||(!name)||(sz_outlines<0))
	return NULL;
    if (geom->num_shapes>0) {
	for (shape=geom->shapes,i=0;i<geom->num_shapes;i++,shape++) {
	    if (name==shape->name)
		return shape;
	}
    }
    if ((geom->num_shapes>=geom->sz_shapes)&&
					(_XkbAllocShapes(geom,1)!=Success))
	return NULL;
    shape= &geom->shapes[geom->num_shapes];
    bzero(shape,sizeof(XkbShapeRec));
    if ((sz_outlines>0)&&(_XkbAllocOutlines(shape,sz_outlines)!=Success))
	return NULL;
    shape->name= name;
    shape->primary= shape->approx= NULL;
    geom->num_shapes++;
    return shape;
}

XkbKeyPtr
XkbcAddGeomKey(XkbRowPtr row)
{
XkbKeyPtr	key;
    if (!row)
	return NULL;
    if ((row->num_keys>=row->sz_keys)&&(_XkbAllocKeys(row,1)!=Success))
	return NULL;
    key= &row->keys[row->num_keys++];
    bzero(key,sizeof(XkbKeyRec));
    return key;
}

XkbRowPtr
XkbcAddGeomRow(XkbSectionPtr section,int sz_keys)
{
XkbRowPtr	row;

    if ((!section)||(sz_keys<0))
	return NULL;
    if ((section->num_rows>=section->sz_rows)&&
    					(_XkbAllocRows(section,1)!=Success))
	return NULL;
    row= &section->rows[section->num_rows];
    bzero(row,sizeof(XkbRowRec));
    if ((sz_keys>0)&&(_XkbAllocKeys(row,sz_keys)!=Success))
	return NULL;
    section->num_rows++;
    return row;
}

XkbSectionPtr
XkbcAddGeomSection(	XkbGeometryPtr	geom,
			Atom		name,
			int		sz_rows,
			int		sz_doodads,
			int		sz_over)
{
register int	i;
XkbSectionPtr	section;

    if ((!geom)||(name==None)||(sz_rows<0))
	return NULL;
    for (i=0,section=geom->sections;i<geom->num_sections;i++,section++) {
	if (section->name!=name)
	    continue;
	if (((sz_rows>0)&&(_XkbAllocRows(section,sz_rows)!=Success))||
	    ((sz_doodads>0)&&(_XkbAllocDoodads(section,sz_doodads)!=Success))||
	    ((sz_over>0)&&(_XkbAllocOverlays(section,sz_over)!=Success)))
	    return NULL;
	return section;
    }
    if ((geom->num_sections>=geom->sz_sections)&&
					(_XkbAllocSections(geom,1)!=Success))
	return NULL;
    section= &geom->sections[geom->num_sections];
    if ((sz_rows>0)&&(_XkbAllocRows(section,sz_rows)!=Success))
	return NULL;
    if ((sz_doodads>0)&&(_XkbAllocDoodads(section,sz_doodads)!=Success)) {
	if (section->rows) {
	    _XkbFree(section->rows);
	    section->rows= NULL;
	    section->sz_rows= section->num_rows= 0;
	}
	return NULL;
    }
    section->name= name;
    geom->num_sections++;
    return section;
}

XkbDoodadPtr
XkbcAddGeomDoodad(XkbGeometryPtr geom,XkbSectionPtr section,Atom name)
{
XkbDoodadPtr	old,doodad;
register int	i,nDoodads;

    if ((!geom)||(name==None))
	return NULL;
    if ((section!=NULL)&&(section->num_doodads>0)) {
	old= section->doodads;
	nDoodads= section->num_doodads;
    }
    else {
	old= geom->doodads;
	nDoodads= geom->num_doodads;
    }
    for (i=0,doodad=old;i<nDoodads;i++,doodad++) {
	if (doodad->any.name==name)
	    return doodad;
    }
    if (section) {
	if ((section->num_doodads>=geom->sz_doodads)&&
	    (_XkbAllocDoodads(section,1)!=Success)) {
	    return NULL;
	}
	doodad= &section->doodads[section->num_doodads++];
    }
    else {
	if ((geom->num_doodads>=geom->sz_doodads)&&
					(_XkbAllocDoodads(geom,1)!=Success))
	    return NULL;
	doodad= &geom->doodads[geom->num_doodads++];
    }
    bzero(doodad,sizeof(XkbDoodadRec));
    doodad->any.name= name;
    return doodad;
}

XkbOverlayKeyPtr
XkbcAddGeomOverlayKey(	XkbOverlayPtr		overlay,
			XkbOverlayRowPtr 	row,
			char *			over,
			char *			under)
{
register int	i;
XkbOverlayKeyPtr key;
XkbSectionPtr	section;
XkbRowPtr	row_under;
Bool		found;

    if ((!overlay)||(!row)||(!over)||(!under))
	return NULL;
    section= overlay->section_under;
    if (row->row_under>=section->num_rows)
	return NULL;
    row_under= &section->rows[row->row_under];
    for (i=0,found=False;i<row_under->num_keys;i++) {
	if (strncmp(under,row_under->keys[i].name.name,XkbKeyNameLength)==0) {
	    found= True;
	    break;
	}
    }
    if (!found)
   	return NULL;
    if ((row->num_keys>=row->sz_keys)&&(_XkbAllocOverlayKeys(row,1)!=Success))
	return NULL;
    key= &row->keys[row->num_keys];
    strncpy(key->under.name,under,XkbKeyNameLength);
    strncpy(key->over.name,over,XkbKeyNameLength);
    row->num_keys++;
    return key;
}

XkbOverlayRowPtr
XkbcAddGeomOverlayRow(XkbOverlayPtr overlay,int row_under,int sz_keys)
{
register int		i;
XkbOverlayRowPtr	row;

    if ((!overlay)||(sz_keys<0))
	return NULL;
    if (row_under>=overlay->section_under->num_rows)
	return NULL;
    for (i=0;i<overlay->num_rows;i++) {
	if (overlay->rows[i].row_under==row_under) {
	    row= &overlay->rows[i];
	    if ((row->sz_keys<sz_keys)&&
				(_XkbAllocOverlayKeys(row,sz_keys)!=Success)) {
		return NULL;
	    }
	    return &overlay->rows[i];
	}
    }
    if ((overlay->num_rows>=overlay->sz_rows)&&
				(_XkbAllocOverlayRows(overlay,1)!=Success))
	return NULL;
    row= &overlay->rows[overlay->num_rows];
    bzero(row,sizeof(XkbOverlayRowRec));
    if ((sz_keys>0)&&(_XkbAllocOverlayKeys(row,sz_keys)!=Success))
	return NULL;
    row->row_under= row_under;
    overlay->num_rows++;
    return row;
}

XkbOverlayPtr
XkbcAddGeomOverlay(XkbSectionPtr section,Atom name,int sz_rows)
{
register int	i;
XkbOverlayPtr	overlay;

    if ((!section)||(name==None)||(sz_rows==0))
	return NULL;

    for (i=0,overlay=section->overlays;i<section->num_overlays;i++,overlay++) {
	if (overlay->name==name) {
	    if ((sz_rows>0)&&(_XkbAllocOverlayRows(overlay,sz_rows)!=Success))
		return NULL;
	    return overlay;
	}
    }
    if ((section->num_overlays>=section->sz_overlays)&&
				(_XkbAllocOverlays(section,1)!=Success))
	return NULL;
    overlay= &section->overlays[section->num_overlays];
    if ((sz_rows>0)&&(_XkbAllocOverlayRows(overlay,sz_rows)!=Success))
	return NULL;
    overlay->name= name;
    overlay->section_under= section;
    section->num_overlays++;
    return overlay;
}
