//ez a cucc kerül a kihívotthoz!
//ennek adja az amoba_cucc.c a parancsokat!
//zold mano

#include <std.h>
#include "../amoba.h"

inherit OBJECT;

object ob;

int check_get(object who);

void create(object obj) {
  ::create();
  ob=obj;
  set_name("Zöld O");
  set_id(({"o","zold","karika","kor","jel"}));
  set_short("%^BOLD%^%^GREEN%^Zöld O%^RESET%^");
  set_long("Egy apró zöld %^BOLD%^%^GREEN%^O%^RESET%^. Egy manótól kaptad a %^BOLD%^%^WHITE%^szabályok%^RESET%^kal egyetemben, ezzel tudsz %^BOLD%^%^WHTIE%^rak%^RESET%^ni, illetve %^BOLD%^%^WHITE%^felad%^RESET%^ni a játékot.\n");
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
	add_action("do_rak","rak");
	add_action("do_felad","felad");
	add_action("do_kilep","kilep");
	add_action("do_szabalyok","szabalyok");
	add_action("do_nez_papir","nez");
}
int do_nez_papir(string str){
	if(str=="papir"){
		ob->look_at_paper();
		tell_room(environment(TP),"%^BOLD%^%^WHITE%^"+TP->query_cap_name()+"%^GREEN%^ nézegeti a manó papírját.%^RESET%^\n",TP);
		return 1;
	}
	return 0;
}
int check_get(object who){
	if(who!=ob->get_kihivott() || who!=ob->get_kihivo()){
		return 0;
	}
	return 1;
}

int do_szabalyok(){
	ob->szabalyok();
	return 1;
}

int do_rak(string str){
	ob->rak(str);
	return 1;
}
int do_felad(){
	ob->felad();
	return 1;
}
int do_kilep(){
	ob->kilep_a();
}
