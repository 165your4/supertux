//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <config.h>

#include "dispenser.hpp"
#include "object/bullet.hpp"
#include "random_generator.hpp"

Dispenser::Dispenser(const lisp::Lisp& reader)
	: BadGuy(reader, "images/creatures/dispenser/dispenser.sprite")
{
  set_colgroup_active(COLGROUP_MOVING_STATIC);
  sound_manager->preload("sounds/squish.wav");
  reader.get("cycle", cycle);
  reader.get_vector("badguy", badguys);
  random = false; // default
  reader.get("random", random);
  type = "dropper"; //default
  reader.get("type", type);
  next_badguy = 0;
  autotarget = false;
  swivel = false;
  broken = false;

  if (badguys.size() <= 0)
    throw std::runtime_error("No badguys in dispenser.");

  if (type == "rocket_launcher") {
    sprite->set_action(dir == LEFT ? "working-left" : "working-right");
    set_colgroup_active(COLGROUP_MOVING); //if this were COLGROUP_MOVING_STATIC MrRocket would explode on launch.

    if (start_dir == AUTO) {
      autotarget = true;
    }
  } else if (type == "cannon") {
    sprite->set_action("working");
  } else {
    sprite->set_action("dropper");
  }

  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
  countMe = false;
}

void
Dispenser::write(lisp::Writer& writer)
{
  writer.start_list("dispenser");

  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);
  writer.write_float("cycle", cycle);
  writer.write_bool("random", random);
  writer.write_string("type", type);
  writer.write_string_vector("badguy", badguys);

  writer.end_list("dispenser");
}

void
Dispenser::activate()
{
   if( broken ){
     return;
   }
   if( autotarget && !swivel ){ // auto cannon sprite might be wrong
      Player* player = this->get_nearest_player();
      if( player ){
        dir = (player->get_pos().x > get_pos().x) ? RIGHT : LEFT;
        sprite->set_action(dir == LEFT ? "working-left" : "working-right");
      }
   }
   dispense_timer.start(cycle, true);
   launch_badguy();
}

void
Dispenser::deactivate()
{
   dispense_timer.stop();
}

//TODO: Add launching velocity to certain badguys
bool
Dispenser::collision_squished(GameObject& object)
{
  //Cannon launching MrRocket can be broken by jumping on it
  //other dispencers are not that fragile.
  if (broken || type != "rocket_launcher") {
    return false;
  }

  sprite->set_action(dir == LEFT ? "broken-left" : "broken-right");
  dispense_timer.start(0);
  set_colgroup_active(COLGROUP_MOVING_STATIC); // Tux can stand on broken cannon.
  Player* player = dynamic_cast<Player*>(&object);
  if (player){
    player->bounce(*this);
  }
  sound_manager->play("sounds/squish.wav", get_pos());
  broken = true;
  return true;
}

HitResponse
Dispenser::collision(GameObject& other, const CollisionHit& hit)
{
  Player* player = dynamic_cast<Player*> (&other);
  if(player) {
    // hit from above?
    if (player->get_bbox().p2.y < (bbox.p1.y + 16)) {
      collision_squished(*player);
      return FORCE_MOVE;
    }
    if(frozen){
      unfreeze();
    }
    return FORCE_MOVE;
  }

  Bullet* bullet = dynamic_cast<Bullet*> (&other);
  if(bullet){
    return collision_bullet(*bullet, hit);
  }

  return FORCE_MOVE;
}


void
Dispenser::active_update(float )
{
  if (dispense_timer.check()) {
    // auto always shoots in Tux's direction
    if( autotarget ){ 
      if( sprite->animation_done()) {
        sprite->set_action(dir == LEFT ? "working-left" : "working-right");
        swivel = false;
      }

      Player* player = this->get_nearest_player();
      if( player && !swivel ){
        Direction targetdir = (player->get_pos().x > get_pos().x) ? RIGHT : LEFT;
        if( dir != targetdir ){ // no target: swivel cannon 
          swivel = true;
          dir = targetdir;
          sprite->set_action(dir == LEFT ? "swivel-left" : "swivel-right", 1);
        } else { // tux in sight: shoot
          launch_badguy();
        }
      }
    } else {
      launch_badguy();
    }
  }
}

void
Dispenser::launch_badguy()
{
  //FIXME: Does is_offscreen() work right here?
  if (!is_offscreen()) {
    Direction launchdir = dir;
    if( !autotarget && start_dir == AUTO ){
      Player* player = this->get_nearest_player();
      if( player ){
        launchdir = (player->get_pos().x > get_pos().x) ? RIGHT : LEFT;
      } 
    } 

    if (badguys.size() > 1) {
      if (random) {
        next_badguy = systemRandom.rand(badguys.size());
      }
      else {
        next_badguy++;

        if (next_badguy >= badguys.size())
          next_badguy = 0;
      }
    }

    std::string badguy = badguys[next_badguy];
    GameObject* badguy_object = NULL;

    if (type == "dropper")
      badguy_object = create_badguy_object(badguy, Vector(get_pos().x, get_pos().y+32), launchdir);
    else if (type == "cannon")
      badguy_object = create_badguy_object(badguy, Vector(get_pos().x + (launchdir == LEFT ? -32 : 32), get_pos().y), launchdir);
    else if (type == "rocket_launcher")
      badguy_object = create_badguy_object(badguy, Vector(get_pos().x + (launchdir == LEFT ? -32 : 32), get_pos().y), launchdir);

    if (badguy_object)
      Sector::current()->add_object(badguy_object);
  }
}

void
Dispenser::freeze()
{
  BadGuy::freeze();
  dispense_timer.stop();
}

void
Dispenser::unfreeze()
{
  BadGuy::unfreeze();
  activate();
}

bool
Dispenser::is_freezable() const
{
  return true;
}
IMPLEMENT_FACTORY(Dispenser, "dispenser")
