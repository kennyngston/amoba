
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
  set_name("Am�ba man�");
  set_id( ({"mano","amoba"}) );
  set_short("%^BOLD%^%^GREEN%^Am�ba man�%^RESET%^");
  set_long("Egy apr�cska man� a v�lladon. Egy �si j�t�k �rz�je �s mestere, kiss� szakadt ruh�jan rengeteg zsebet l�tsz. Ha megk�rdezed t�le a %^BOLD%^%^WHITE%^szab�lyok%^RESET%^at, %^BOLD%^%^WHITE%^kihiv%^RESET%^hatsz valakit egy j�tszm�ra, vagy megn�zheted a %^BOLD%^%^WHITE%^pontok%^RESET%^at!");
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
	rules+="%^BOLD%^%^GREEN%^A man� el�vesz egy pergament egyik zseb�b�l, �s mint a betanult sz�veget suttogja f�ledbe:\n\n";
	rules+="%^YELLOW%^A j�t�kban k�t j�t�kos vesz r�szt. A kezd�s v�letlenszer�, k�t l�p�s k�z�tt maximum egy perc telhet el!\n";
	rules+="A j�t�k c�lja, hogy a j�t�kos lerakjon 5 egyforma jelet egym�ssal �rint�legesen, egy vonalban, b�rmely ir�nyban.\n";
  rules+="Amelyik j�t�kosnak ez el�bb siker�l, az nyer.\n\n";
  rules+="A j�t�kban haszn�lhat� parancsok:\n";
  rules+="rak <oszlop> <sor>    -jel rak�sa a pap�rra\n";
  rules+="felad                 -a j�t�k felad�sa\n\n";
  rules+="%^GREEN%^A man� f�ltekeri a pergament, �s elteszi egyik zseb�be.%^RESET%^\n";
  tell_object(TP,rules);
  tell_room(environment(TP),"%^BOLD%^%^GREEN%^A %^WHITE%^"+TP->query_cap_name()+"%^GREEN%^ v�ll�n �l� man� el�vesz egy tekercset, suttog valamit f�l�be, majd elrakja a tekercset.%^RESET%^\n",TP);
  return 1;
}

int kihiv(string kit){
	//kerdezo_mano: seged cucc a kihivottnak
	object ob;
	int i;
	//ha foldon van
	if(!userp(environment(TO)) && environment(TO)->query_name()!="csicska") ERR("%^BOLD%^%^GREEN%^A man� �rtetlen�l n�z fel r�d.%^RESET%^");
	if(TP->getenv("AMOBAZIK")==1) ERR("%^BOLD%^%^GREEN%^A man� a f�ledbe suttog:%^RESET%^ Ejnye.. egyszerre egy j�t�k nem el�g?\n");
	if(!kit) ERR("%^BOLD%^%^GREEN%^A man� a f�ledbe suttog:%^RESET%^ Kit szeretn�l kih�vni?");
	//hulye user kezeles
	ob=find_player(kit);
	if(!ob) ERR("%^BOLD%^%^GREEN%^A man� a f�ledbe suttog:%^RESET%^ Csak olyan j�t�kost h�vhatsz ki, aki be is van l�pve!\n");
	if(ob==TP) ERR("%^BOLD%^%^GREEN%^A man� nagyon but�n n�z r�d..%^RESET%^\n");
	if(ob->getenv("AMOBAZIK")==1) ERR("%^BOLD%^%^GREEN%^A man� a f�ledbe suttog:%^RESET%^ � most nem �r r�..\n");
	//kihivas megkuldese
	kerdezo_mano=new(ITEM_PATH + "amoba_mano",TO);
	tell_object(TP,"%^BOLD%^%^GREEN%^A man� a f�ledbe suttog:%^RESET%^ Elk�ldtem egy bar�tomat "+CAP(ob->query_name())+" v�ll�ra, nemsok�ra kider�l, hogy d�nt..\n");
	tell_object(ob,"%^BOLD%^%^GREEN%^Egy apr� man� jelenik meg a jobb v�lladon �s a f�ledbe suttog:%^RESET%^ "+CAP(TP->query_name())+" kih�vott egy am�ba j�tszm�ra!\n"
								 "%^BOLD%^%^GREEN%^A man� a f�ledbe suttog: %^WHITE%^Elfogad%^RESET%^od, vagy %^BOLD%^%^WHITE%^elutas�t%^RESET%^od a kih�v�st?\n");
	tell_room(environment(ob),"%^BOLD%^%^WHITE%^"+ob->query_cap_name()+"%^GREEN%^ v�ll�n egy man� jelenik meg, majd suttog valamit a f�l�be.%^RESET%^\n",ob);
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
		ERR("%^BOLD%^%^GREEN%^A man� a f�ledbe suttog:%^RESET%^ M�g senki sem nyert j�t�kot!\n");
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
		tell_object(TP,"%^BOLD%^%^GREEN%^A man� a f�ledbe suttog:%^RESET%^ A toplista jelenleg a k�vetkez� k�ppen n�z ki:\n\n"
		               "%^BOLD%^%^WHITE%^"+arrange_string("N�v",15)+"%^GREEN%^"+arrange_string("Nyert",6)+"%^YELLOW%^"+arrange_string("D�ntetlen",10)+"%^RED%^"+arrange_string("Vesztett",9)+""+arrange_string("Feladott",9)+"%^RESET%^\n");
		for(i=0;i<sizeof(segedarr);i++){
			if(i>9) return 1;
			tell_object(TP,"%^BOLD%^%^WHITE%^"+arrange_string(CAP(segedarr[i]),15)+"%^GREEN%^"+arrange_string(""+winners[segedarr[i]][0]+"",6)+"%^YELLOW%^"+arrange_string(""+winners[segedarr[i]][1]+"",10)+"%^RED%^"+arrange_string(""+winners[segedarr[i]][2]+"",9)+""+arrange_string(""+winners[segedarr[i]][3]+"",9)+"%^RESET%^");
		}
		return 1;
	}
	if(!winners[str]){
		ERR("%^BOLD%^%^GREEN%^A man� v�gigb�ng�szi list�j�t, de nem tal�l ilyen j�t�kost!\n");
	}
	tell_object(TP,"%^BOLD%^%^GREEN%^A man� v�gigbong�szi a list�j�t, majd r�b�k a k�vetkez� sorra:%^RESET%^\n"
	               "%^BOLD%^%^WHITE%^"+CAP(str)+"%^GREEN%^ j�tszm�i: %^BOLD%^%^WHITE%^"+winners[str][0]+"%^GREEN%^ nyertes,%^WHITE%^ "+winners[str][1]+"%^GREEN%^ d�ntetlen,%^WHITE%^ "+winners[str][2]+"%^GREEN%^ vesztes, valamint %^WHITE%^"+winners[str][3]+"%^GREEN%^ feladott j�tszma.%^RESET%^\n");
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
	//X es O kioszt�sa, random
	if(i==1){
		tell_object(kihivo,"%^BOLD%^%^WHITE%^A man� kezedbe nyom egy piros %^RED%^X%^WHITE%^-et, majd f�ledbe s�gja:%^RESET%^ Ezzel tudsz %^BOLD%^%^WHITE%^rak%^RESET%^ni!\n");
		tell_object(kihivott,"%^BOLD%^%^WHITE%^A man� kezedbe nyom egy z�ld %^GREEN%^O%^WHITE%^-t, majd f�ledbe s�gja:%^RESET%^ Ezzel tudsz %^BOLD%^%^WHITE%^rak%^RESET%^ni!\n");
		player1=kihivo;
		player2=kihivott;
		jatekos1["nev"]=kihivo->query_name();
		jatekos1["cnev"]=CAP(kihivo->query_name());
		jatekos2["nev"]=kihivott->query_name();
		jatekos2["cnev"]=CAP(kihivott->query_name());
	}else{
		tell_object(kihivott,"%^BOLD%^%^WHITE%^A man� kezedbe nyom egy piros %^RED%^X%^WHITE%^-et, majd f�ledbe s�gja:%^RESET%^ Ezzel tudsz %^BOLD%^%^WHITE%^rak%^RESET%^ni!\n");
		tell_object(kihivo,"%^BOLD%^%^WHITE%^A man� kezedbe nyom egy z�ld %^GREEN%^O%^WHITE%^-t, majd f�ledbe s�gja:%^RESET%^ Ezzel tudsz %^BOLD%^%^WHITE%^rak%^RESET%^ni!\n");
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
	tell_object(player1,"%^BOLD%^%^GREEN%^A man� el�vesz egy gy�r�tt pap�rt az egyik zseb�b�l, �s el�d tartja.%^RESET%^\n");
	tell_room(environment(player1),"%^BOLD%^%^GREEN%^Egy man� el�vesz egy gy�r�tt pap�rt egyik zseb�b�l, �s %^WHITE%^"+player1->query_cap_name()+"%^GREEN%^ el� tartja.%^RESET%^\n",player1);
	tell_object(player1,papir);
	tell_object(player2,"%^BOLD%^%^GREEN%^A man� el�vesz egy gy�r�tt pap�rt az egyik zseb�b�l, �s el�d tartja.%^RESET%^\n");
	tell_room(environment(player2),"%^BOLD%^%^GREEN%^Egy man� el�vesz egy gy�r�tt pap�rt egyik zseb�b�l, �s %^WHITE%^"+player2->query_cap_name()+"%^GREEN%^ el� tartja.%^RESET%^\n",player2);
	tell_object(player2,papir);
	return 1;
}

int elutasit(){
	kihivas=0;
	tell_object(kihivo,"A man� szomor�an �js�golja: Sajnos "+CAP(kihivott->query_name())+" eltas�totta a kih�v�sod, tal�n majd legk�zelebb!\n");
	tell_object(kihivott,"A man� hirtelen elt�nik v�lladr�l.\n");
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
		tell_object(player1,"%^BOLD%^%^RED%^Feladtad a j�t�kot, a man� fogja mag�t �s darabjaira t�pi sz�t a pap�rt.%^RESET%^\n");
		tell_object(player2,"%^BOLD%^%^GREEN%^A man� a f�ledbe suttog:%^RESET%^ Partnered feladta a j�t�kot!\n");
	}else{
		tell_object(player1,"%^BOLD%^%^GREEN%^A man� a f�ledbe suttog:%^RESET%^ Partnered feladta a j�t�kot!\n");
		tell_object(player2,"%^BOLD%^%^RED%^Feladtad a j�t�kot, a man� fogja mag�t �s darabjaira t�pi sz�t a pap�rt.%^RESET%^\n");
	}
	amoba_over();
	return 1;
}

int rak(string str){
  string s,tmp;
  int i;
  if(!qturn(TP)) ERR("%^BOLD%^%^GREEN%^A man� a f�ledbe suttog:%^RESET%^ Nem rajtad van a sor!\n");
  if(!str) ERR("%^BOLD%^%^GREEN%^A man� a f�ledbe suttog:%^RESET%^ Haszn�lat: %^BOLD%^%^WHITE%^rak <oszlop> <sor>%^RESET%^\n");
  if(sscanf(str,"%s %d",s,i)!=2) ERR("%^BOLD%^%^GREEN%^A man� a f�ledbe suttog:%^RESET%^ Haszn�lat: %^BOLD%^%^WHITE%^rak <oszlop> <sor>%^RESET%^\n");
  s=lower_case(s);
  i=i-1;
  if(cba(s)>OSZLOPOK-1 || i>SOROK-1 || cba(s)<0 || i<0) ERR("%^BOLD%^%^GREEN%^A man� �rtetlen�l pislog r�d.%^RESET%^\n");
  if(qt(cba(s),i)) ERR("%^BOLD%^%^GREEN%^A man� a f�ledbe suttog:%^RESET%^ Az a mez� m�r foglalt!\n");
  rakott(TP,cba(s),i);
  if(amoba(TP,cba(s),i)){
    tell_object(player1,papir);
    tell_object(player2,papir);
    tmp=read_file(STATFILE);
    if(qid(TP)==1){
    	tell_object(player1,"%^BOLD%^%^YELLOW%^A man� gratul�l, megnyerted a j�t�kot!%^RESET%^\n");
      tell_object(player2,"%^BOLD%^%^GREEN%^A man� a f�ledbe suttog:%^RESET%^ Ellenfeled megnyerte a j�t�kot!\n");
      if(J_MESTER) CHAT_D->do_chat("jatek",jatekos1["cnev"]+" gy�zedelmeskedett "+capitalize(jatekos2["nev"])+" f�l�tt am�b�ban!",1);
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
    tell_object(player2,"%^BOLD%^%^YELLOW%^A man� gratul�l, megnyerted a j�t�kot!%^RESET%^\n");
    tell_object(player1,"%^BOLD%^%^GREEN%^A man� a f�ledbe suttog:%^RESET%^ Ellenfeled megnyerte a j�t�kot!\n");
    if(J_MESTER) CHAT_D->do_chat("jatek",jatekos2["cnev"]+" gy�zedelmeskedett "+capitalize(jatekos1["nev"])+" f�l�tt am�b�ban!",1);
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
    tell_object(player1,"%^BOLD%^%^GREEN%^A man� a f�ledbe suttog:%^RESET%^ Nincs t�bb szabad hely a pap�ron, a j�t�k d�ntetlen!\n");
    tell_object(player2,"%^BOLD%^%^GREEN%^A man� a f�ledbe suttog:%^RESET%^ Nincs t�bb szabad hely a pap�ron, a j�t�k d�ntetlen!\n");
    if(J_MESTER) CHAT_D->do_chat("jatek",jatekos1["cnev"]+" �s "+jatekos2["cnev"]+" d�ntetlent j�tszottak az am�ba j�t�kban!",1);
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
		tell_object(player2,"%^BOLD%^%^GREEN%^A man� a f�ledbe suttog:%^RESET%^ Partnered kil�pett a j�t�kb�l, nyert�l!\n");
    if(J_MESTER) CHAT_D->do_chat("jatek",jatekos2["cnev"]+" nyert az am�ba j�t�kban!",1);
    amoba_over();
  }
  if(qid(TP)==2){
  	tell_object(player1,"%^BOLD%^%^GREEN%^A man� a f�ledbe suttog:%^RESET%^ Partnered kil�pett a j�t�kb�l, nyert�l!\n");
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
    if(J_MESTER) CHAT_D->do_chat("jatek",jatekos1["cnev"]+" nyert az am�ba j�t�kban!",1);
    amoba_over();
  }
}
/*
SEGEDFUGGVENYEK
*/
int player1timeout(){
	string tmp;
	tell_object(player1,"%^BOLD%^%^GREEN%^A man� a f�ledbe suttog:%^RESET%^ Lej�rt az id�d, elvesztetted a j�t�kot!\n");
	tell_object(player2,"%^BOLD%^%^GREEN%^A man� a f�ledbe suttog:%^RESET%^ Ellenfeled ideje lej�rt, megnyerted a j�t�kot!\n");
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
	tell_object(player1,"%^BOLD%^%^GREEN%^A man� a f�ledbe suttog:%^RESET%^ Ellenfeled ideje lej�rt, megnyerted a j�t�kot!\n");
	tell_object(player2,"%^BOLD%^%^GREEN%^A man� a f�ledbe suttog:%^RESET%^ Lej�rt az id�d, elvesztetted a j�t�kot!\n");
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
		tell_object(kihivo,"A man� szomor�an �js�golja: Sajnos "+CAP(kihivott->query_name())+" nem v�laszolt id�ben a kih�v�sodra, tal�n majd legk�zelebb..\n");
		tell_object(kihivott,"A man� megunta a v�rakoz�st, hirtelen elt�nik v�lladr�l.\n");
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
    papir+="%^BOLD%^%^WHITE%^"+kikov+" k�vetkezik.%^RESET%^\n\n";
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
  	 tell_object(player1,"%^BOLD%^%^GREEN%^A man� boldogan mosolyog, ahogy a pap�rdarabot l�tod szertefoszlani.%^RESET%^\n");
  	 tell_room(environment(player1),"%^BOLD%^%^GREEN%^A %^WHITE%^"+jatekos1["cnev"]+"%^GREEN%^ v�ll�n �l� man� boldogan mosolyog, ahogy egy pap�rdarab szertefoszlik.%^RESET%^\n",player1);
  	 tell_object(player2,"%^BOLD%^%^GREEN%^A man� boldogan mosolyog, ahogy a pap�rdarabot l�tod szertefoszlani.%^RESET%^\n");
  	 tell_room(environment(player2),"%^BOLD%^%^GREEN%^A %^WHITE%^"+jatekos2["cnev"]+"%^GREEN%^ v�ll�n �l� man� boldogan mosolyog, ahogy egy pap�rdarab szertefoszlik.%^RESET%^\n",player2);
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
    tell_object(player1,"%^BOLD%^%^GREEN%^A man� a f�ledbe suttog:%^RESET%^ Rakt�l a %^BOLD%^%^WHITE%^["+CAP(abc(x))+"]["+(y+1)+"]%^RESET%^ mez�re!\n");
    tell_object(player2,"%^BOLD%^%^GREEN%^A man� a f�ledbe suttog:%^RESET%^ "+jatekos1["cnev"]+" rakott a %^BOLD%^%^WHITE%^["+CAP(abc(x))+"]["+(y+1)+"]%^RESET%^ mez�re!\n");
  } else {
    t[x][y]=2;
    if(MODSZER){
      p[x][y]="%^BOLD%^%^RED%^XX%^RESET%^";
    }else{
      p[x][y]="%^BOLD%^%^RED%^X%^RESET%^";
    }
    tell_object(player2,"%^BOLD%^%^GREEN%^A man� a f�ledbe suttog:%^RESET%^ Rakt�l a %^BOLD%^%^WHITE%^["+CAP(abc(x))+"]["+(y+1)+"]%^RESET%^ mez�re!\n");
    tell_object(player1,"%^BOLD%^%^GREEN%^A man� a f�ledbe suttog:%^RESET%^ "+jatekos2["cnev"]+" rakott a %^BOLD%^%^WHITE%^["+CAP(abc(x))+"]["+(y+1)+"]%^RESET%^ mez�re!\n");
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
