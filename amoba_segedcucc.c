//ez a cucc kerül a kihívotthoz!
//ennek adja az amoba_cucc.c a parancsokat!

#include <std.h>
#include "../amoba.h"

inherit OBJECT;

object ob;

void create(object obj) {
  ::create();
  ob=obj;
  set_name("Amoba manó");
  set_id(({"mano","amoba"}));
  set_short("Amóba manó");
  set_long("Amóba manó hosszú leírás");
  set_min_level(0);
  set_weight(10);
  set_value(0);
  set_prevent_put("Ezt nem teheted el!");
  set_property("no steal",1);
  set_property("extra_item",1);
  set_property("donated",1);
  set_prevent_drop("Ne..");
}

void init(){
	::init();
	MSG("Mittomenma");
	add_action("do_elfogad","elfogad");
}

int do_elfogad(){
	ob->elfogad();
	return 1;
}
