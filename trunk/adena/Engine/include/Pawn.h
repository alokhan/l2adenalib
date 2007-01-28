/*
 * Pawn.h - Base class for any moving object in the game.
 * Created January 26, 2007, by Michael 'Bigcheese' Spencer.
 *
 * Copyright (C) 2007 Michael Spencer
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Michael Spencer - bigcheesegs@gmail.com
 */

#ifndef _ADENA_O_PAWN_H_
#define _ADENA_O_PAWN_H_

#include <Actor.h>
#include <IGameServerClient.h>

namespace adena
{
namespace game_server
{

	enum E_MoveState
	{
		EMS_Still = 0,
		EMS_Moving,
		EMS_RequestMove
	};

	class Pawn : public Actor
	{
	public:

		Pawn(IOObjectSystem* obj_sys);

		virtual ~Pawn();

		virtual void tick(irr::f32 delta_time);

		virtual irr::u32 getSpeed();

		virtual void moveToLocation(irr::core::vector3df Target);

		//virtual void teleportToLocation(irr::core::vector3di Target);

		// Event called when MoveTarget reached.
		virtual void onStopMove();

		virtual void onClick(COObject* event_instagator, bool shift_click);

		irr::core::vector3df MoveTarget;
		E_MoveState MoveState;

		IGameServerClient* Owner;

	};

}
}

#endif
