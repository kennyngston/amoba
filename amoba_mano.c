//ez a cucc ker�l a kih�votthoz!
//ennek adja az amoba_cucc.c a parancsokat!

#include <std.h>
#include "../amoba.h"

inherit OBJECT;

object ob;

int check_get(object who);

void create(object obj) {
  ::create();
  ob=obj;
  set_name("Am�ba man�");
  set_id(({"mano","amoba"}));
  set_short("%^BOLD%^%^GREEN%^Am�ba man�%^RESET%^");
  set_long("Egy man�, aki kih�v�st hozott neked. %^BOLD%^%^WHITE%^Elfogad%^RESET%^od vagy %^BOLD%^%^WHITE%^elutas�t%^RESET%^od a kih�v�st?");
  set_min_level(1);
  set_weight(0);
  set_value(0);
  set_prevent_put("Ezt nem teheted el!");
  set_property("no steal",1);
  set_property("extra_item",1);
  set_property("donated",1);
  set_prevent_drop("Ne..");
  set_prevent_get( (: check_get :) );
}

void init(){
	::init();
	if(!userp(environment(TO))&& environment(TO)->query_name()!="csicska") TO->remove();
	add_action("do_elfogad","elfogad");
	add_action("do_elutasit",({"kilep","elutasit"}));
}

int check_get(object who){
	if(who!=ob->get_kihivott() || who!=ob->get_kihivo()){
		return 0;
	}
	return 1;
}

int do_elfogad(){
	ob->elfogad();
	return 1;
}

int do_elutasit(){
	ob->elutasit();
	return 1;
}
