
//By Slocum: 2011.07.11-2011.07.14

#include <std.h>
#include <daemons.h>
#include "../amoba.h"

inherit OBJECT;

#define MODSZER 1   // 1: dupla , 0 : siman irja a cuccot
#define J_MESTER 0  // 1 nel megy jatek csatira hir

#define KEZDO_ERTEK " " //sima modszerhez kezdo ertek, ne basztasd
#define KEZDO_ERTEK2 "  "//dupla modszerhez kezdo ertek, ezt se basztasd

//min 10x10
#define OSZLOPOK 13 //max 26
#define SOROK 13 //max unlimited (esszel)

#define COLOR "%^RESET%^%^BOLD%^BLACK%^" //racs szin

void create() {
  ::create();
  set_name("Amõba manó");
  set_id( ({"mano","amoba"}) );
  set_short("%^BOLD%^%^GREEN%^Amõba manó%^RESET%^");
  set_long("Egy aprócska manó a válladon. Egy õsi játék õrzõje és mestere, kissé szakadt ruhájan rengeteg zsebet látsz. Ha megkérdezed tõle a %^BOLD%^%^WHITE%^szabályok%^RESET%^at, %^BOLD%^%^WHITE%^kihiv%^RESET%^hatsz valakit egy játszmára, vagy megnézheted a %^BOLD%^%^WHITE%^pontok%^RESET%^at!");
  set_min_level(1);
  set_weight(15);
  set_value(0);
  set_property("no steal",1);
}

void init(){
	::init();
	add_action("kihiv","kihiv");
	add_action("szabalyok","szabalyok");
	add_action("pontok","pontok");
}

//fg prototipusok
void initm();
void setpapir();
int cba(string s);
string abc(int i);
int qid(object ob);
int qturn(object ob);
int qt(int x,int y);
void rakott(object ob, int x, int y);
int amoba(object ob,int x,int y);
int nincs_szabad();
void kov(object ob);
void kihivas_timeout();
void amoba_over();
int player1timeout();
int player2timeout();

//kezdo valtozok
//objectek:
//kihivo, kihivott egyertelmu
//kerdezo mano: a mano amelyik viszi a hirt a kihivasrol
//kihivas: 1 ha folyamatban van kihivas
//mano1 mano2 csak valtozoban mano, egyebkent seged cuccok remote cmdkhez
//megyajatek: kell, kulonben amobA_over nem tudja mikor kell kivenni a ket seged cuccot
object kihivo, kihivott, kerdezo_mano,player1,player2;
object mano1,mano2;
int kihivas;
int megyajatek;
int kovetkezo;
int xs,ys;

string kikov,papir;

mapping jatekos1=([]);
mapping jatekos2=([]);
mapping winners;

array p,t,amobaban;
//local parancsok
int szabalyok(){
	string rules="";
	rules+="%^BOLD%^%^GREEN%^A manó elõvesz egy pergament egyik zsebébõl, és mint a betanult szöveget suttogja füledbe:\n\n";
	rules+="%^YELLOW%^A játékban két játékos vesz részt. A kezdés véletlenszerû, két lépés között maximum egy perc telhet el!\n";
	rules+="A játék célja, hogy a játékos lerakjon 5 egyforma jelet egymással érintõlegesen, egy vonalban, bármely irányban.\n";
  rules+="Amelyik játékosnak ez elõbb sikerül, az nyer.\n\n";
  rules+="A játékban használható parancsok:\n";
  rules+="rak <oszlop> <sor>    -jel rakása a papírra\n";
  rules+="felad                 -a játék feladása\n\n";
  rules+="%^GREEN%^A manó föltekeri a pergament, és elteszi egyik zsebébe.%^RESET%^\n";
  tell_object(TP,rules);
  tell_room(environment(TP),"%^BOLD%^%^GREEN%^A %^WHITE%^"+TP->query_cap_name()+"%^GREEN%^ vállán ülõ manó elõvesz egy tekercset, suttog valamit fülébe, majd elrakja a tekercset.%^RESET%^\n",TP);
  return 1;
}

int kihiv(string kit){
	//kerdezo_mano: seged cucc a kihivottnak
	object ob;
	int i;
	//ha foldon van
	if(!userp(environment(TO)) && environment(TO)->query_name()!="csicska") ERR("%^BOLD%^%^GREEN%^A manó értetlenül néz fel rád.%^RESET%^");
	if(TP->getenv("AMOBAZIK")==1) ERR("%^BOLD%^%^GREEN%^A manó a füledbe suttog:%^RESET%^ Ejnye.. egyszerre egy játék nem elég?\n");
	if(!kit) ERR("%^BOLD%^%^GREEN%^A manó a füledbe suttog:%^RESET%^ Kit szeretnél kihívni?");
	//hulye user kezeles
	ob=find_player(kit);
	if(!ob) ERR("%^BOLD%^%^GREEN%^A manó a füledbe suttog:%^RESET%^ Csak olyan játékost hívhatsz ki, aki be is van lépve!\n");
	if(ob==TP) ERR("%^BOLD%^%^GREEN%^A manó nagyon bután néz rád..%^RESET%^\n");
	if(ob->getenv("AMOBAZIK")==1) ERR("%^BOLD%^%^GREEN%^A manó a füledbe suttog:%^RESET%^ Õ most nem ér rá..\n");
	//kihivas megkuldese
	kerdezo_mano=new(ITEM_PATH + "amoba_mano",TO);
	tell_object(TP,"%^BOLD%^%^GREEN%^A manó a füledbe suttog:%^RESET%^ Elküldtem egy barátomat "+CAP(ob->query_name())+" vállára, nemsokára kiderül, hogy dönt..\n");
	tell_object(ob,"%^BOLD%^%^GREEN%^Egy apró manó jelenik meg a jobb válladon és a füledbe suttog:%^RESET%^ "+CAP(TP->query_name())+" kihívott egy amõba játszmára!\n"
								 "%^BOLD%^%^GREEN%^A manó a füledbe suttog: %^WHITE%^Elfogad%^RESET%^od, vagy %^BOLD%^%^WHITE%^elutasít%^RESET%^od a kihívást?\n");
	tell_room(environment(ob),"%^BOLD%^%^WHITE%^"+ob->query_cap_name()+"%^GREEN%^ vállán egy manó jelenik meg, majd suttog valamit a fülébe.%^RESET%^\n",ob);
	kerdezo_mano->move(ob);
	kihivott=ob;
	kihivo=TP;
	kihivas=1;
	kihivo->setenv("AMOBAZIK",1);
	kihivott->setenv("AMOBAZIK",1);
	call_out("kihivas_timeout",30);
	return 1;
}

int pontok(string str){
	string *kulcsok, tmp, maxkulcs;
	string *segedarr;
	int i,j,idmax,id1;
	tmp=read_file(STATFILE);
	if(!tmp){
		ERR("%^BOLD%^%^GREEN%^A manó a füledbe suttog:%^RESET%^ Még senki sem nyert játékot!\n");
	}
	winners=restore_variable(tmp);
	if(!str){
		idmax=0;
		kulcsok=keys(winners);
		segedarr=allocate(sizeof(kulcsok));
		id1=sizeof(kulcsok);
		for(j=0;j<sizeof(kulcsok);j++){
			for(i=0;i<id1;i++){
				if(winners[kulcsok[i]][0]>idmax){
					idmax=winners[kulcsok[i]][0];
					maxkulcs=kulcsok[i];
				}
			}
			kulcsok-=({maxkulcs});
			segedarr[j]=maxkulcs;
			idmax=0;
			id1--;
		}
		segedarr[sizeof(segedarr)-1]=kulcsok[0];
		tell_object(TP,"%^BOLD%^%^GREEN%^A manó a füledbe suttog:%^RESET%^ A toplista jelenleg a következõ képpen néz ki:\n\n"
		               "%^BOLD%^%^WHITE%^"+arrange_string("Név",15)+"%^GREEN%^"+arrange_string("Nyert",6)+"%^YELLOW%^"+arrange_string("Döntetlen",10)+"%^RED%^"+arrange_string("Vesztett",9)+""+arrange_string("Feladott",9)+"%^RESET%^\n");
		for(i=0;i<sizeof(segedarr);i++){
			if(i>9) return 1;
			tell_object(TP,"%^BOLD%^%^WHITE%^"+arrange_string(CAP(segedarr[i]),15)+"%^GREEN%^"+arrange_string(""+winners[segedarr[i]][0]+"",6)+"%^YELLOW%^"+arrange_string(""+winners[segedarr[i]][1]+"",10)+"%^RED%^"+arrange_string(""+winners[segedarr[i]][2]+"",9)+""+arrange_string(""+winners[segedarr[i]][3]+"",9)+"%^RESET%^");
		}
		return 1;
	}
	if(!winners[str]){
		ERR("%^BOLD%^%^GREEN%^A manó végigböngészi listáját, de nem talál ilyen játékost!\n");
	}
	tell_object(TP,"%^BOLD%^%^GREEN%^A manó végigbongészi a listáját, majd rábök a következõ sorra:%^RESET%^\n"
	               "%^BOLD%^%^WHITE%^"+CAP(str)+"%^GREEN%^ játszmái: %^BOLD%^%^WHITE%^"+winners[str][0]+"%^GREEN%^ nyertes,%^WHITE%^ "+winners[str][1]+"%^GREEN%^ döntetlen,%^WHITE%^ "+winners[str][2]+"%^GREEN%^ vesztes, valamint %^WHITE%^"+winners[str][3]+"%^GREEN%^ feladott játszma.%^RESET%^\n");
  return 1;
}
				
//remote parancsok, csak akkor aktivak, ha a kihivas el lett fogadva, es a jatek elkezdodott
int elfogad(){
	int i;
	kihivas=0;
	megyajatek=1;
	i=random(2);
	kerdezo_mano->remove();
	remove_call_out("kihivas_timeout");
	//X es O kiosztása, random
	if(i==1){
		tell_object(kihivo,"%^BOLD%^%^WHITE%^A manó kezedbe nyom egy piros %^RED%^X%^WHITE%^-et, majd füledbe súgja:%^RESET%^ Ezzel tudsz %^BOLD%^%^WHITE%^rak%^RESET%^ni!\n");
		tell_object(kihivott,"%^BOLD%^%^WHITE%^A manó kezedbe nyom egy zöld %^GREEN%^O%^WHITE%^-t, majd füledbe súgja:%^RESET%^ Ezzel tudsz %^BOLD%^%^WHITE%^rak%^RESET%^ni!\n");
		player1=kihivo;
		player2=kihivott;
		jatekos1["nev"]=kihivo->query_name();
		jatekos1["cnev"]=CAP(kihivo->query_name());
		jatekos2["nev"]=kihivott->query_name();
		jatekos2["cnev"]=CAP(kihivott->query_name());
	}else{
		tell_object(kihivott,"%^BOLD%^%^WHITE%^A manó kezedbe nyom egy piros %^RED%^X%^WHITE%^-et, majd füledbe súgja:%^RESET%^ Ezzel tudsz %^BOLD%^%^WHITE%^rak%^RESET%^ni!\n");
		tell_object(kihivo,"%^BOLD%^%^WHITE%^A manó kezedbe nyom egy zöld %^GREEN%^O%^WHITE%^-t, majd füledbe súgja:%^RESET%^ Ezzel tudsz %^BOLD%^%^WHITE%^rak%^RESET%^ni!\n");
		player1=kihivott;
		player2=kihivo;
		jatekos1["nev"]=kihivott->query_name();
		jatekos1["cnev"]=CAP(kihivott->query_name());
		jatekos2["nev"]=kihivo->query_name();
		jatekos2["cnev"]=CAP(kihivo->query_name());
	}
	
	
	mano1=new(ITEM_PATH + "amoba_o",TO);
	mano2=new(ITEM_PATH + "amoba_x",TO);
	mano1->move(player1);
	mano2->move(player2);
	
	i=random(2);
	//kezdes eldontese
	if(i==1){
		kovetkezo=1;
		kikov=jatekos1["cnev"];
		tell_object(player1,"Te kezdesz!\n");
		tell_object(player2,jatekos1["cnev"]+" kezd!\n");
		call_out("player1timeout",60);
	}else{
		kovetkezo=2;
		kikov=jatekos2["cnev"];
		tell_object(player2,"Te kezdesz!\n");
		tell_object(player1,jatekos2["cnev"]+" kezd!\n");
		call_out("player2timeout",60);
	}
	initm();
	setpapir();
	tell_object(player1,"%^BOLD%^%^GREEN%^A manó elõvesz egy gyûrött papírt az egyik zsebébõl, és eléd tartja.%^RESET%^\n");
	tell_room(environment(player1),"%^BOLD%^%^GREEN%^Egy manó elõvesz egy gyûrött papírt egyik zsebébõl, és %^WHITE%^"+player1->query_cap_name()+"%^GREEN%^ elé tartja.%^RESET%^\n",player1);
	tell_object(player1,papir);
	tell_object(player2,"%^BOLD%^%^GREEN%^A manó elõvesz egy gyûrött papírt az egyik zsebébõl, és eléd tartja.%^RESET%^\n");
	tell_room(environment(player2),"%^BOLD%^%^GREEN%^Egy manó elõvesz egy gyûrött papírt egyik zsebébõl, és %^WHITE%^"+player2->query_cap_name()+"%^GREEN%^ elé tartja.%^RESET%^\n",player2);
	tell_object(player2,papir);
	return 1;
}

int elutasit(){
	kihivas=0;
	tell_object(kihivo,"A manó szomorúan újságolja: Sajnos "+CAP(kihivott->query_name())+" eltasította a kihívásod, talán majd legközelebb!\n");
	tell_object(kihivott,"A manó hirtelen eltünik válladról.\n");
	kerdezo_mano->remove();
	remove_call_out("kihivas_timeout");
	amoba_over();
	return 1;
}
void look_at_paper(){
	tell_object(TP,papir);
}
int felad(){
	if(qid(TP)==1){
		tell_object(player1,"%^BOLD%^%^RED%^Feladtad a játékot, a manó fogja magát és darabjaira tépi szét a papírt.%^RESET%^\n");
		tell_object(player2,"%^BOLD%^%^GREEN%^A manó a füledbe suttog:%^RESET%^ Partnered feladta a játékot!\n");
	}else{
		tell_object(player1,"%^BOLD%^%^GREEN%^A manó a füledbe suttog:%^RESET%^ Partnered feladta a játékot!\n");
		tell_object(player2,"%^BOLD%^%^RED%^Feladtad a játékot, a manó fogja magát és darabjaira tépi szét a papírt.%^RESET%^\n");
	}
	amoba_over();
	return 1;
}

int rak(string str){
  string s,tmp;
  int i;
  if(!qturn(TP)) ERR("%^BOLD%^%^GREEN%^A manó a füledbe suttog:%^RESET%^ Nem rajtad van a sor!\n");
  if(!str) ERR("%^BOLD%^%^GREEN%^A manó a füledbe suttog:%^RESET%^ Használat: %^BOLD%^%^WHITE%^rak <oszlop> <sor>%^RESET%^\n");
  if(sscanf(str,"%s %d",s,i)!=2) ERR("%^BOLD%^%^GREEN%^A manó a füledbe suttog:%^RESET%^ Használat: %^BOLD%^%^WHITE%^rak <oszlop> <sor>%^RESET%^\n");
  s=lower_case(s);
  i=i-1;
  if(cba(s)>OSZLOPOK-1 || i>SOROK-1 || cba(s)<0 || i<0) ERR("%^BOLD%^%^GREEN%^A manó értetlenül pislog rád.%^RESET%^\n");
  if(qt(cba(s),i)) ERR("%^BOLD%^%^GREEN%^A manó a füledbe suttog:%^RESET%^ Az a mezõ már foglalt!\n");
  rakott(TP,cba(s),i);
  if(amoba(TP,cba(s),i)){
    tell_object(player1,papir);
    tell_object(player2,papir);
    tmp=read_file(STATFILE);
    if(qid(TP)==1){
    	tell_object(player1,"%^BOLD%^%^YELLOW%^A manó gratulál, megnyerted a játékot!%^RESET%^\n");
      tell_object(player2,"%^BOLD%^%^GREEN%^A manó a füledbe suttog:%^RESET%^ Ellenfeled megnyerte a játékot!\n");
      if(J_MESTER) CHAT_D->do_chat("jatek",jatekos1["cnev"]+" gyõzedelmeskedett "+capitalize(jatekos2["nev"])+" fölött amõbában!",1);
      if(!tmp){
        winners=([jatekos1["nev"] : ({ 1,0,0,0 }),jatekos2["nev"] : ({0,0,1,0}) ]);
      } else {
        winners=restore_variable(tmp);
       	if(!winners[jatekos1["nev"]]){
        	winners[jatekos1["nev"]]=({1,0,0,0});
       	}else{
        	winners[jatekos1["nev"]][0]+=1;
        }
        if(!winners[jatekos2["nev"]]){
        	winners[jatekos2["nev"]]=({0,1,0,0});
        }else{
        	winners[jatekos2["nev"]][2]+=1;
        }
      }
      tmp=save_variable(winners);
      write_file(STATFILE,tmp,1);
      amoba_over();
      return 1;
    }
    tell_object(player2,"%^BOLD%^%^YELLOW%^A manó gratulál, megnyerted a játékot!%^RESET%^\n");
    tell_object(player1,"%^BOLD%^%^GREEN%^A manó a füledbe suttog:%^RESET%^ Ellenfeled megnyerte a játékot!\n");
    if(J_MESTER) CHAT_D->do_chat("jatek",jatekos2["cnev"]+" gyõzedelmeskedett "+capitalize(jatekos1["nev"])+" fölött amõbában!",1);
    if(!tmp){
      winners=([jatekos1["nev"] : ({0,0,1,0}), jatekos2["nev"] : ({1,0,0,0}) ]);
    } else {
    	winners=restore_variable(tmp);
    	if(!winners[jatekos1["nev"]]){
    		winners[jatekos1["nev"]]=({0,0,1,0});
    	}else{
    		winners[jatekos1["nev"]][2]+=1;
    	}
    	if(!winners[jatekos2["nev"]]){
    		winners[jatekos2["nev"]]=({1,0,0,0});
    	}else{
    		winners[jatekos2["nev"]][0]+=1;
    	}
    }
    tmp=save_variable(winners);
    write_file(STATFILE,tmp,1);
    amoba_over();
    return 1;
  }
  if(nincs_szabad()){
    tell_object(player1,"%^BOLD%^%^GREEN%^A manó a füledbe suttog:%^RESET%^ Nincs több szabad hely a papíron, a játék döntetlen!\n");
    tell_object(player2,"%^BOLD%^%^GREEN%^A manó a füledbe suttog:%^RESET%^ Nincs több szabad hely a papíron, a játék döntetlen!\n");
    if(J_MESTER) CHAT_D->do_chat("jatek",jatekos1["cnev"]+" és "+jatekos2["cnev"]+" döntetlent játszottak az amõba játékban!",1);
    tmp=read_file(STATFILE);
    if(!tmp){
      winners=([jatekos1["nev"] : ({0,1,0,0}), jatekos2["nev"] : ({0,1,0,0}) ]);
    }else{
    	winners=restore_variable(tmp);
    	if(!winners[jatekos1["nev"]]){
    		winners[jatekos1["nev"]]=({0,1,0,0});
    	}else{
    		winners[jatekos1["nev"]][1]+=1;
    	}
    	if(!winners[jatekos2["nev"]]){
    		winners[jatekos2["nev"]]=({0,1,0,0});
    	}else{
    		winners[jatekos2["nev"]][1]+=1;
    	}
    }
    tmp=save_variable(winners);
    write_file(STATFILE,tmp,1);
    amoba_over();
    return 1;
  }
  if(qid(TP)==1){
  	call_out("player2timeout",60);
  	remove_call_out("player1timeout");
  }else{
  	call_out("player1timeout",60);
  	remove_call_out("player2timeout");
  }
  kov(TP);
  tell_object(player1,papir);
  tell_object(player2,papir);
  return 1;
}
int kilep_a(){
	string tmp;
	tmp=read_file(STATFILE);
	if(qid(TP)==1){
		if(!tmp){
	  	winners=([jatekos1["nev"] : ({0,0,0,1}), jatekos2["nev"] : ({1,0,0,0}) ]);
	  }else{
	  	winners=restore_variable(tmp);
	  	if(!winners[jatekos1["nev"]]){
	  		winners[jatekos1["nev"]]=({0,0,0,1});
	  	}else{
	  		winners[jatekos1["nev"]][3]+=1;
	  	}
	  	if(!winners[jatekos2["nev"]]){
	  		winners[jatekos2["nev"]]=({1,0,0,0});
	  	}else{
	  		winners[jatekos2["nev"]][0]+=1;
	  	}
	  }
	  tmp=save_variable(winners);
	  write_file(STATFILE,tmp,1);
		tell_object(player2,"%^BOLD%^%^GREEN%^A manó a füledbe suttog:%^RESET%^ Partnered kilépett a játékból, nyertél!\n");
    if(J_MESTER) CHAT_D->do_chat("jatek",jatekos2["cnev"]+" nyert az amõba játékban!",1);
    amoba_over();
  }
  if(qid(TP)==2){
  	tell_object(player1,"%^BOLD%^%^GREEN%^A manó a füledbe suttog:%^RESET%^ Partnered kilépett a játékból, nyertél!\n");
  	if(!tmp){
    	winners=([jatekos1["nev"] : ({1,0,0,0}), jatekos2["nev"] : ({0,0,0,1}) ]);
    }else{
    	winners=restore_variable(tmp);
    	if(!winners[jatekos1["nev"]]){
    		winners[jatekos1["nev"]]=({1,0,0,0});
    	}else{
    		winners[jatekos1["nev"]][0]+=1;
    	}
    	if(!winners[jatekos2["nev"]]){
    		winners[jatekos2["nev"]]=({0,0,0,1});
    	}else{
    		winners[jatekos2["nev"]][3]+=1;
    	}
    }
    tmp=save_variable(winners);
    write_file(STATFILE,tmp,1);
    if(J_MESTER) CHAT_D->do_chat("jatek",jatekos1["cnev"]+" nyert az amõba játékban!",1);
    amoba_over();
  }
}
/*
SEGEDFUGGVENYEK
*/
int player1timeout(){
	string tmp;
	tell_object(player1,"%^BOLD%^%^GREEN%^A manó a füledbe suttog:%^RESET%^ Lejárt az idõd, elvesztetted a játékot!\n");
	tell_object(player2,"%^BOLD%^%^GREEN%^A manó a füledbe suttog:%^RESET%^ Ellenfeled ideje lejárt, megnyerted a játékot!\n");
	tmp=read_file(STATFILE);
	if(!tmp){
  	winners=([jatekos1["nev"] : ({0,0,0,1}), jatekos2["nev"] : ({1,0,0,0}) ]);
	}else{
		winners=restore_variable(tmp);
		if(!winners[jatekos1["nev"]]){
			winners[jatekos1["nev"]]=({0,0,0,1});
		}else{
			winners[jatekos1["nev"]][3]+=1;
		}
		if(!winners[jatekos2["nev"]]){
			winners[jatekos2["nev"]]=({1,0,0,0});
		}else{
			winners[jatekos2["nev"]][0]+=1;
		}
	}
	tmp=save_variable(winners);
	write_file(STATFILE,tmp,1);
	amoba_over();
	return 1;
}
int player2timeout(){
	string tmp;
	tell_object(player1,"%^BOLD%^%^GREEN%^A manó a füledbe suttog:%^RESET%^ Ellenfeled ideje lejárt, megnyerted a játékot!\n");
	tell_object(player2,"%^BOLD%^%^GREEN%^A manó a füledbe suttog:%^RESET%^ Lejárt az idõd, elvesztetted a játékot!\n");
	tmp=read_file(STATFILE);
	if(!tmp){
	  winners=([ jatekos1["nev"] : ({1,0,0,0}), jatekos2["nev"] : ({0,0,0,1}) ]);
	}else{
		winners=restore_variable(tmp);
		if(!winners[jatekos1["nev"]]){
			winners[jatekos1["nev"]]=({1,0,0,0});
		}else{
			winners[jatekos1["nev"]][0]+=1;
		}
		if(!winners[jatekos2["nev"]]){
			winners[jatekos2["nev"]]=({0,0,0,1});
		}else{
			winners[jatekos2["nev"]][3]+=1;
		}
	}
	tmp=save_variable(winners);
	write_file(STATFILE,tmp,1);
	amoba_over();
	return 1;
}
void kihivas_timeout(){
	if(kihivas){
		tell_object(kihivo,"A manó szomorúan újságolja: Sajnos "+CAP(kihivott->query_name())+" nem válaszolt idõben a kihívásodra, talán majd legközelebb..\n");
		tell_object(kihivott,"A manó megunta a várakozást, hirtelen eltünik válladról.\n");
		kerdezo_mano->remove();
		kihivas=0;
		amoba_over();
	}
}

void allocater(int i){
  int j;
  for(j=0;j<i;j++){
    p[j]=allocate(SOROK);
    t[j]=allocate(SOROK);
  }
}

void initm(){
  int i,j;
  amobaban=allocate(5);
  p=allocate(OSZLOPOK);
  t=allocate(OSZLOPOK);
  allocater(OSZLOPOK);
  for(i=0;i<OSZLOPOK;i++){
    for(j=0;j<SOROK;j++){
      if(MODSZER){
        p[i][j]=KEZDO_ERTEK2;
      }else{
        p[i][j]=KEZDO_ERTEK;
      }
      t[i][j]=0;
    }
  }
}

void setpapir(){
  int i,j,b;
    papir="";
    papir+="%^BOLD%^%^WHITE%^"+kikov+" következik.%^RESET%^\n\n";
    papir+="  "+COLOR+"|%^RESET%^";
    for(i=0;i<OSZLOPOK;i++){
      if(MODSZER){
        papir+="%^BOLD%^%^YELLOW%^"+CAP(abc(i))+" "+COLOR+"|%^RESET%^";  
      }else{
        papir+="%^BOLD%^%^YELLOW%^"+CAP(abc(i))+""+COLOR+"|%^RESET%^";  
      }
      if(i==OSZLOPOK-1){
        if(MODSZER){
          papir+="\n"+COLOR+"--+%^RESET%^";
          for(j=0;j<OSZLOPOK-1;j++) papir+=""+COLOR+"--+%^RESET%^";
          papir+=""+COLOR+"--+--\n%^RESET%^"; //elso elvalaszto sor utso zaroja
        }else{
          papir+="\n"+COLOR+"--+%^RESET%^";
          for(j=0;j<OSZLOPOK-1;j++) papir+=""+COLOR+"-+%^RESET%^";
          papir+=""+COLOR+"-+--\n%^RESET%^"; //elso elvalaszto sor utso zaroja
        }
      }
    }
    for(j=0;j<SOROK;j++){
      if(j<9) papir+="%^BOLD%^%^YELLOW%^ "+(j+1)+""+COLOR+"|%^RESET%^";
          else papir+="%^BOLD%^%^YELLOW%^"+(j+1)+""+COLOR+"|%^RESET%^";
     for(i=0;i<OSZLOPOK;i++){
       papir+=p[i][j]+""+COLOR+"|%^RESET%^";
       if(i==OSZLOPOK-1){
         papir+="%^BOLD%^%^YELLOW%^"+(j+1)+"%^RESET%^";
         if(j==SOROK-1){//UTSO SORELVALASZTO!!
           if(MODSZER){
             papir+="\n"+COLOR+"--+%^RESET%^";//utso sorelvalaszto elso resze
             for(b=0;b<OSZLOPOK-1;b++) papir+=""+COLOR+"--+%^RESET%^";
             papir+=""+COLOR+"--+--%^RESET%^\n";//utolso sorelvalaszto utso resze
           } else{
              papir+="\n"+COLOR+"--+%^RESET%^";//utso sorelvalaszto elso resze
             for(b=0;b<OSZLOPOK-1;b++) papir+=""+COLOR+"-+%^RESET%^";
             papir+=""+COLOR+"-+--%^RESET%^\n"; //utso sorelvalaszto utso resze
           }
         } else {
           if(MODSZER){
             papir+="\n"+COLOR+"--+%^RESET%^";
             for(b=0;b<OSZLOPOK-1;b++) papir+=""+COLOR+"--+%^RESET%^";
             papir+=""+COLOR+"--+--%^RESET%^"; //normal sorelvalasztok utso resze
             papir+="\n";
           }else{
             papir+="\n"+COLOR+"--+%^RESET%^";       
             for(b=0;b<OSZLOPOK-1;b++) papir+=""+COLOR+"-+%^RESET%^";
             papir+=""+COLOR+"-+--%^RESET%^"; //normal sorelvalasztok utso resze
             papir+="\n";
           }
         }
       }
     }
   }
   papir+="  "+COLOR+"|%^RESET%^";
   for(i=0;i<OSZLOPOK;i++){
     if(MODSZER){
       papir+="%^BOLD%^%^YELLOW%^"+CAP(abc(i))+" "+COLOR+"|%^RESET%^";  
     }else{
       papir+="%^BOLD%^%^YELLOW%^"+CAP(abc(i))+""+COLOR+"|%^RESET%^";  
     }
   }
   papir+="\n";
}

int cba(string s){
  switch(s){
    case "a" : return 0;
    case "b" : return 1;
    case "c" : return 2;
    case "d" : return 3;
    case "e" : return 4;
    case "f" : return 5;
    case "g" : return 6;
    case "h" : return 7;
    case "i" : return 8;
    case "j" : return 9;
    case "k" : return 10;
    case "l" : return 11;
    case "m" : return 12;
    case "n" : return 13;
    case "o" : return 14;
    case "p" : return 15;
    case "q" : return 16;
    case "r" : return 17;
    case "s" : return 18;
    case "t" : return 19;
    case "u" : return 20;
    case "v" : return 21;
    case "w" : return 22;
    case "x" : return 23;
    case "y" : return 24;
    case "z" : return 25;
  }
  return -1;
}

string abc(int i){
  switch(i){
    case 0 : return "a";
    case 1 : return "b";
    case 2 : return "c";
    case 3 : return "d";
    case 4 : return "e";
    case 5 : return "f";
    case 6 : return "g";
    case 7 : return "h";
    case 8 : return "i";
    case 9 : return "j";
    case 10 : return "k";
    case 11 : return "l";
    case 12 : return "m";
    case 13 : return "n";
    case 14 : return "o";
    case 15 : return "p";
    case 16 : return "q";
    case 17 : return "r";
    case 18 : return "s";
    case 19 : return "t";
    case 20 : return "u";
    case 21 : return "v";
    case 22 : return "w";
    case 23 : return "x";
    case 24 : return "y";
    case 25 : return "z";
  }
}
int qid(object ob){
  if(ob->query_name()==jatekos1["nev"]) return 1;
  if(ob->query_name()==jatekos2["nev"]) return 2;
  return 0;
}
void amoba_over(){
  kihivo->setenv("AMOBAZIK",0);
  kihivott->setenv("AMOBAZIK",0);
  if(megyajatek){
  	 mano1->remove();
  	 mano2->remove();
  	 megyajatek=0;
  	 remove_call_out("player1timeout");
  	 remove_call_out("player2timeout");
  	 tell_object(player1,"%^BOLD%^%^GREEN%^A manó boldogan mosolyog, ahogy a papírdarabot látod szertefoszlani.%^RESET%^\n");
  	 tell_room(environment(player1),"%^BOLD%^%^GREEN%^A %^WHITE%^"+jatekos1["cnev"]+"%^GREEN%^ vállán ülõ manó boldogan mosolyog, ahogy egy papírdarab szertefoszlik.%^RESET%^\n",player1);
  	 tell_object(player2,"%^BOLD%^%^GREEN%^A manó boldogan mosolyog, ahogy a papírdarabot látod szertefoszlani.%^RESET%^\n");
  	 tell_room(environment(player2),"%^BOLD%^%^GREEN%^A %^WHITE%^"+jatekos2["cnev"]+"%^GREEN%^ vállán ülõ manó boldogan mosolyog, ahogy egy papírdarab szertefoszlik.%^RESET%^\n",player2);
  }
  map_delete(jatekos2,"nev");
  map_delete(jatekos2,"cnev");
  map_delete(jatekos1,"nev");
  map_delete(jatekos1,"cnev");
}
int qturn(object ob){
  if(qid(ob)==kovetkezo) return 1;
  return 0;
}

int qt(int x,int y){
  return t[x][y];
}
void rakott(object ob, int x, int y){
  if(qid(ob)==1){
    t[x][y]=1;
    if(MODSZER){
      p[x][y]="%^BOLD%^%^GREEN%^OO%^RESET%^";
    }else{
      p[x][y]="%^BOLD%^%^GREEN%^O%^RESET%^";
    }
    tell_object(player1,"%^BOLD%^%^GREEN%^A manó a füledbe suttog:%^RESET%^ Raktál a %^BOLD%^%^WHITE%^["+CAP(abc(x))+"]["+(y+1)+"]%^RESET%^ mezõre!\n");
    tell_object(player2,"%^BOLD%^%^GREEN%^A manó a füledbe suttog:%^RESET%^ "+jatekos1["cnev"]+" rakott a %^BOLD%^%^WHITE%^["+CAP(abc(x))+"]["+(y+1)+"]%^RESET%^ mezõre!\n");
  } else {
    t[x][y]=2;
    if(MODSZER){
      p[x][y]="%^BOLD%^%^RED%^XX%^RESET%^";
    }else{
      p[x][y]="%^BOLD%^%^RED%^X%^RESET%^";
    }
    tell_object(player2,"%^BOLD%^%^GREEN%^A manó a füledbe suttog:%^RESET%^ Raktál a %^BOLD%^%^WHITE%^["+CAP(abc(x))+"]["+(y+1)+"]%^RESET%^ mezõre!\n");
    tell_object(player1,"%^BOLD%^%^GREEN%^A manó a füledbe suttog:%^RESET%^ "+jatekos2["cnev"]+" rakott a %^BOLD%^%^WHITE%^["+CAP(abc(x))+"]["+(y+1)+"]%^RESET%^ mezõre!\n");
  }
  setpapir();
}

int cc(int i){
  switch(i){
    case 1 : return 8;
    case 2 : return 7;
    case 3 : return 6;
    case 4 : return 5;
    case 5 : return 4;
    case 6 : return 3;
    case 7 : return 2;
    case 8 : return 1;
  }
}

int papiron(int x,int y){
  if(x<0 || y<0 || x>OSZLOPOK-1 || y>SOROK-1) return 0;
  return 1;
}

//x:y irany-beli szomszedja beallitasa xs ys re
void getter(int x,int y,int irany){
  switch(irany){
    case 1 : xs=x-1;ys=y-1;break;
    case 2 : xs=x;ys=y-1;break;
    case 3 : xs=x+1;ys=y-1;break;
    case 4 : xs=x-1;ys=y;break;
    case 5 : xs=x+1;ys=y;break;
    case 6 : xs=x-1;ys=y+1;break;
    case 7 : xs=x;ys=y+1;break;
    case 8 : xs=x+1;ys=y+1;break;
  }
}

void make_it_flash(){
	string tmp;
	int i;
	for(i=0;i<5;i++){
  	tmp=p[amobaban[i][0]][amobaban[i][1]];
	  p[amobaban[i][0]][amobaban[i][1]]="";
	  p[amobaban[i][0]][amobaban[i][1]]="%^FLASH%^"+tmp+"";
  }
  setpapir();
}

int amoba(object ob,int x,int y){
  int id=qid(ob);
  int i;
  int szamol=0;
  amobaban[0]=({ x , y });
  //induljunk el balra fel
  for(i=1;i<8;i++){
    getter(x,y,i);
    if(papiron(xs,ys)){//1 letezik X1
      if(qt(xs,ys)==id){//1 TP X2
      	amobaban[1]=({xs,ys});
        getter(xs,ys,i);
        if(papiron(xs,ys)){ //2 letezik X2
          if(qt(xs,ys)==id){ //2 TP X3
          	amobaban[2]=({xs,ys});
            getter(xs,ys,i);
            if(papiron(xs,ys)){ // 3 letezik X3
              if(qt(xs,ys)==id){ // 3 TP X4
              	amobaban[3]=({xs,ys});
                getter(xs,ys,i);
                if(papiron(xs,ys)){ // 4 letezik X4
                  if(qt(xs,ys)==id){ // 4 TP X5
                  	amobaban[4]=({xs,ys});
                    szamol++;
                    make_it_flash();
                    return 1;
                  }// 4 NTP X4
                  getter(x,y,cc(i));
                  if(papiron(xs,ys)){ // 4 NTP & 5 letezik X4
                    if(qt(xs,ys)==id){ // 4 NTP & 5 TP X5
                    	amobaban[4]=({xs,ys});
                    	make_it_flash();
                      szamol++;
                      return 1;
                    }// 4 NTP & 5 NTP X4
                    //return 0;
                  }//4NTP & 5 NPP X4
                  //return 0;
                }// 4 NPP X4
                getter(x,y,cc(i));
                if(papiron(xs,ys)){ // 4 NPP & 5 letezik X4
                  if(qt(xs,ys)==id){ // 4 NPP & 5 TP X5
                  	amobaban[4]=({xs,ys});
                  	make_it_flash();
                    szamol++;
                    return 1;
                  }//4NPP & 5 NTP X4
                  //return 0;
                }// 4 NPP & 5 NTP X4
                //return 0;
              }// 3 NTP X3
              getter(x,y,cc(i));
              if(papiron(xs,ys)){//3NTP & 5 letezik X3
                if(qt(xs,ys)==id){ //3NTP & 5 TP X4
                	amobaban[3]=({xs,ys});
                  getter(xs,ys,cc(i));
                  if(papiron(xs,ys)){ // 3 NTP & 6 letezik X4
                    if(qt(xs,ys)==id){ // 3 NTP & 6 TP X5
                    	amobaban[4]=({xs,ys});
                    	make_it_flash();
                      szamol++;
                      return 1;
                    }// 3 NTP & 6 NTP X4
                    //return 0;
                  }//3 NTP & 6 NPP X4
                  //return 0;
                }// 3 NTP & 5 NTP X3
                //return 0;
              }// 3 NTP & 5 NPP X3
              //return 0;
            }//3 NPP X3
            getter(x,y,cc(i));
            if(papiron(xs,ys)){//3NPP & 5 letezik X3
              if(qt(xs,ys)==id){ //3NPP & 5 TP X4
              	amobaban[3]=({xs,ys});
                getter(xs,ys,cc(i));
                if(papiron(xs,ys)){ //3NPP & 6 letezik X4
                  if(qt(xs,ys)==id){ //3NPP & 6 TP X5
                  	amobaban[4]=({xs,ys});
                  	make_it_flash();
                    szamol++;
                    return 1;
                  }//3NPP & 6 NTP X4
                  //return 0;
                }//3NPP & 6 NPP X4
                //return 0;
              }//3NP & 5 NTP X3
              //return 0;
            }//3NPP & 5 NPP X3
            //return 0;
          }//2NTP X2
          getter(x,y,cc(i));
          if(papiron(xs,ys)){//2NTP & 5 letezik X2
            if(qt(xs,ys)==id){//2NTP & 5 TP X3
            	amobaban[2]=({xs,ys});
              getter(xs,ys,cc(i));
              if(papiron(xs,ys)){//2NTP & 6 letezik X3
                if(qt(xs,ys)==id){ //2NTP & 6 TP X4
                	amobaban[3]=({xs,ys});
                  getter(xs,ys,cc(i));
                  if(papiron(xs,ys)){//2NTP & 7 letezik X4
                    if(qt(xs,ys)==id){//2NTP & 7 TP X5
                    	amobaban[4]=({xs,ys});
                    	make_it_flash();
                      szamol++;
                      return 1;
                    }//2NTP & 7 NTP X4
                    //return 0;
                  }//2NTP & 7 NPP X4
                  //return 0;
                }//2NTP & 6 NTP X3
                //return 0;
              }//2NTP & 6 NPP
              //return 0;
            }//2NTP & 5 NTP X2
            //return 0;
          }//2NTP & 5 NPP X2
          //return 0;
        }//2NPP X2
        getter(x,y,cc(i));
        if(papiron(xs,ys)){//2NPP & 5 letezik X2
          if(qt(xs,ys)==id){ //2NPP & 5 TP X3
          	amobaban[2]=({xs,ys});
            getter(xs,ys,cc(i));
            if(papiron(xs,ys)){//2NPP & 6 letezik X3
              if(qt(xs,ys)==id){//2NPP & 6 TP X4
              	amobaban[3]=({xs,ys});
                getter(xs,ys,cc(i));
                if(papiron(xs,ys)){//2NPP & 7 letezik X4
                  if(qt(xs,ys)==id){//2NPP & 7 TP X5
                  	amobaban[4]=({xs,ys});
                  	make_it_flash();
                    szamol++;
                    return 1;
                  }//2NPP & 7 NTP X4
                  //return 0;
                }//2NPP & 7 NPP X4
                //return 0;
              }//2NPP & 6 NTP X3
              //return 0;
            }//2NPP & 6 NPP X3
            //return 0;
          }//2NPP & 6 NTP X2
          //return 0;
        }//2NPP & 5 NPP X2
        //return 0;
      }//1NTP X1
      getter(x,y,cc(i));
      if(papiron(xs,ys)){//1NTP & 5 letezik X1
        if(qt(xs,ys)==id){//1NTP & 5 TP X2
        	amobaban[1]=({xs,ys});
          getter(xs,ys,cc(i));
          if(papiron(xs,ys)){//1NTP & 6 letezik X2
            if(qt(xs,ys)==id){//1NTP & 6 TP X3
            	amobaban[2]=({xs,ys});
              getter(xs,ys,cc(i));
              if(papiron(xs,ys)){//1NTP&7 letezik X3
                if(qt(xs,ys)==id){ // 1 NTP & 7 TP X4
                	amobaban[3]=({xs,ys});
                  getter(xs,ys,cc(i));
                  if(papiron(xs,ys)){//1NTP & 8 letezik X4
                    if(qt(xs,ys)==id){//1NTP & 8 TP X5
                    	amobaban[4]=({xs,ys});
                    	make_it_flash();
                      szamol++;
                      return 1;
                    }//1NTP & 8 NTP X4
                    //return 0;
                  }//1NTP & 8 NPP X4
                  //return 0;
                }//1NTP & 7 NTP X3
                //return 0;
              }//1NTP & 7 NPP X3
              //return 0;
            }//1NTP & 6 NTP X2
            //return 0;
          }//1NTP & 6 NPP X2
          //return 0;
        }//1NTP & 5 NTP X1
        //return 0;
      }//1NTP & 5 NPP X1
      //return 0;
    }//1NPP X1
    getter(x,y,cc(i));
    if(papiron(xs,ys)){//1NPP & 5 letezik X1
      if(qt(xs,ys)==id){//1NPP & 5 TP X2
      	amobaban[1]=({xs,ys,});
        getter(xs,ys,cc(i));
        if(papiron(xs,ys)){//1NPP & 6 letezik X2
          if(qt(xs,ys)==id){//1NPP & 6 TP X3
          	amobaban[2]=({xs,ys});
            getter(xs,ys,cc(i));
            if(papiron(xs,ys)){//1NPP & 7 letezik X3
              if(qt(xs,ys)==id){//1NPP & 7 TP X4
              	amobaban[3]=({xs,ys});
                getter(xs,ys,cc(i));
                if(papiron(xs,ys)){//1NPP & 8 letezik X4
                  if(qt(xs,ys)==id){//1NPP & 8 TP X5
                  	amobaban[4]=({xs,ys});
                  	make_it_flash();
                    szamol++;
                    return 1;
                  }//1NPP & 8 NTP X4
                  //return 0;
                }//1NPP & 8 NPP X4
                //return 0;
              }//1NPP & 7 NTP X3
              //return 0;
            }//1NPP & 7 NPP X3
            //return 0;
          }//1NPP & 6 NTP X2
          //return 0;
        }//1NPP & 6 NPP X2
        //return 0;
      }//1NPP & 5 NTP X1
      //return 0;
    }//1NPP & 5 NPP X1
    //return 0;
  }
  if(szamol) return 1;
  return 0;
}
int nincs_szabad(){
  int seged=0;
  int seged2=0;
  int i,j;
  for(i=0;i<OSZLOPOK;i++){
    for(j=0;j<SOROK;j++){
      seged2++;
      if(t[i][j]!=0) seged++;
    }
  }
  if(seged==seged2) return 1;
  return 0;
}
void kov(object ob){
  if(qid(ob)==1){
    kovetkezo=2;
    kikov=jatekos2["cnev"];
  }
  if(qid(ob)==2){
    kovetkezo=1;
    kikov=jatekos1["cnev"];
  }
  setpapir();
}
object get_kihivo(){
	return kihivo;
}
object get_kihivott(){
	return kihivott;
}
