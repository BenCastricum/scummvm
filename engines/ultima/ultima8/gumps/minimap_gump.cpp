/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/gumps/minimap_gump.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/graphics/shape_info.h"
#include "ultima/ultima8/graphics/palette.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(MiniMapGump, Gump)

MiniMapGump::MiniMapGump(int x_, int y_) :
	Gump(x_, y_, MAP_NUM_CHUNKS * 2 + 2, MAP_NUM_CHUNKS * 2 + 2, 0,
	     FLAG_DRAGGABLE, LAYER_NORMAL), _minimap(), _lastMapNum(0) {
	_minimap._format = TEX_FMT_NATIVE;
	_minimap._width = _minimap._height = MAP_NUM_CHUNKS * MINMAPGUMP_SCALE;
	_minimap._buffer = _texBuffer[0];
}

MiniMapGump::MiniMapGump() : Gump() {
}

MiniMapGump::~MiniMapGump(void) {
}

void MiniMapGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	World *world = World::get_instance();
	CurrentMap *currentmap = world->getCurrentMap();
	int mapChunkSize = currentmap->getChunkSize();

	if (currentmap->getNum() != _lastMapNum) {
		Std::memset(_texBuffer, 0, sizeof(_texBuffer));
		_lastMapNum = currentmap->getNum();
	}

	surf->Fill32(0xFFFFAF00, 0, 0, MAP_NUM_CHUNKS * 2 + 1, 1);
	surf->Fill32(0xFFFFAF00, 0, 1, 1, MAP_NUM_CHUNKS * 2 + 1);
	surf->Fill32(0xFFFFAF00, 1, MAP_NUM_CHUNKS * 2 + 1, MAP_NUM_CHUNKS * 2 + 1, 1);
	surf->Fill32(0xFFFFAF00, MAP_NUM_CHUNKS * 2 + 1, 1, 1, MAP_NUM_CHUNKS * 2 + 1);

	for (int yv = 0; yv < MAP_NUM_CHUNKS; yv++) {
		for (int xv = 0; xv < MAP_NUM_CHUNKS; xv++) {
			if (currentmap->isChunkFast(xv, yv)) {
				for (int j = 0; j < MINMAPGUMP_SCALE; j++) for (int i = 0; i < MINMAPGUMP_SCALE; i++) {
					if (_texBuffer[yv * MINMAPGUMP_SCALE + j][xv * MINMAPGUMP_SCALE + i] == 0)
						_texBuffer[yv * MINMAPGUMP_SCALE + j][xv * MINMAPGUMP_SCALE + i] = sampleAtPoint(
							xv * mapChunkSize + mapChunkSize / (MINMAPGUMP_SCALE * 2) + (mapChunkSize * i) / MINMAPGUMP_SCALE,
							yv * mapChunkSize + mapChunkSize / (MINMAPGUMP_SCALE * 2) + (mapChunkSize * j) / MINMAPGUMP_SCALE,
							currentmap);
				}
			}
		}
	}

	// Center on avatar
	int sx = 0, sy = 0, ox = 0, oy = 0, lx = 0, ly = 0;

	MainActor *av = getMainActor();
	int32 ax, ay, az;
	av->getLocation(ax, ay, az);

	ax = ax / (mapChunkSize / MINMAPGUMP_SCALE);
	ay = ay / (mapChunkSize / MINMAPGUMP_SCALE);

	sx = ax - (mapChunkSize / (4 * 2));
	sy = ay - (mapChunkSize / (4 * 2));
	ax = ax - sx;
	ay = ay - sy;

	if (sx < 0) {
		ox = -sx;
		surf->Fill32(0, 1, 1, ox, MAP_NUM_CHUNKS * 2);
	} else if ((sx + MAP_NUM_CHUNKS * 2) > (MAP_NUM_CHUNKS * MINMAPGUMP_SCALE)) {
		lx = (sx + MAP_NUM_CHUNKS * 2) - (MAP_NUM_CHUNKS * MINMAPGUMP_SCALE);
		surf->Fill32(0, 1 + (MAP_NUM_CHUNKS * 2) - lx, 1, lx, MAP_NUM_CHUNKS * 2);
	}

	if (sy < 0) {
		oy = -sy;
		surf->Fill32(0, 1, 1, MAP_NUM_CHUNKS * 2, oy);
	} else if ((sy + MAP_NUM_CHUNKS * 2) > (MAP_NUM_CHUNKS * MINMAPGUMP_SCALE)) {
		ly = (sy + MAP_NUM_CHUNKS * 2) - (MAP_NUM_CHUNKS * MINMAPGUMP_SCALE);
		surf->Fill32(0, 1, 1 + (MAP_NUM_CHUNKS * 2) - ly, MAP_NUM_CHUNKS * 2, ly);
	}

	surf->Blit(&_minimap, sx + ox, sy + oy, MAP_NUM_CHUNKS * 2 - (ox + lx), MAP_NUM_CHUNKS * 2 - (oy + ly), 1 + ox, 1 + oy);

	surf->Fill32(0xFFFFFF00, 1 + ax - 2, 1 + ay + 0, 2, 1);
	surf->Fill32(0xFFFFFF00, 1 + ax + 0, 1 + ay - 2, 1, 2);
	surf->Fill32(0xFFFFFF00, 1 + ax + 1, 1 + ay + 0, 2, 1);
	surf->Fill32(0xFFFFFF00, 1 + ax + 0, 1 + ay + 1, 1, 2);
}

uint32 MiniMapGump::sampleAtPoint(int x_, int y_, CurrentMap *currentmap) {
	Item *item = currentmap->traceTopItem(x_, y_, 1 << 15, -1, 0, ShapeInfo::SI_ROOF | ShapeInfo::SI_OCCL | ShapeInfo::SI_LAND | ShapeInfo::SI_SEA);

	if (item) {
		int32 ix, iy, iz, idx, idy, idz;
		item->getLocation(ix, iy, iz);
		item->getFootpadWorld(idx, idy, idz);

		ix -= x_;
		iy -= y_;

		Shape *sh = item->getShapeObject();
		if (!sh) return 0;

		ShapeFrame *frame = sh->getFrame(item->getFrame());
		if (!frame) return 0;

		const Palette *pal = sh->getPalette();
		if (!pal) return 0;

		// Screenspace bounding box bottom x_ coord (RNB x_ coord)
		int sx = (ix - iy) / 4;
		// Screenspace bounding box bottom extent  (RNB y_ coord)
		int sy = (ix + iy) / 8 + idz;

		uint16 r = 0, g = 0, b = 0, c = 0;

		for (int j = 0; j < 2; j++) for (int i = 0; i < 2; i++) {
				if (!frame->hasPoint(i - sx, j - sy)) continue;

				uint16 r2, g2, b2;
				UNPACK_RGB8(pal->_native_untransformed[frame->getPixelAtPoint(i - sx, j - sy)], r2, g2, b2);
				r += RenderSurface::Gamma22toGamma10[r2];
				g += RenderSurface::Gamma22toGamma10[g2];
				b += RenderSurface::Gamma22toGamma10[b2];
				c++;
			}
		if (!c) return 0;

		return PACK_RGB8(RenderSurface::Gamma10toGamma22[r / c], RenderSurface::Gamma10toGamma22[g / c], RenderSurface::Gamma10toGamma22[b / c]);
	} else return 0;
}

void MiniMapGump::saveData(ODataSource *ods) {
	Gump::saveData(ods);
}

bool MiniMapGump::loadData(IDataSource *ids, uint32 version) {
	if (!Gump::loadData(ids, version)) return false;

	_lastMapNum = 0;
	_minimap._format = TEX_FMT_NATIVE;
	_minimap._width = _minimap._height = MAP_NUM_CHUNKS * MINMAPGUMP_SCALE;
	_minimap._buffer = _texBuffer[0];

	return true;
}

uint16 MiniMapGump::TraceObjId(int32 mx, int32 my) {
	uint16 objId = Gump::TraceObjId(mx, my);

	if (!objId || objId == 65535)
		if (PointOnGump(mx, my))
			objId = getObjId();

	return objId;
}

} // End of namespace Ultima8
} // End of namespace Ultima
