
#include <std.h>
#include <daemons.h>
#include "amoba.h"

#define J_MESTER 1 //ha 1, a jatekmester infozik
#define MODSZER 1   // 1: dupla , 0 : siman irja a cuccot
#define LONGER 1 // ha 1, tobb string ben kuldi ki a papirt, ha 0, nem (nagy tablahoz)

#define KEZDO_ERTEK " " //sima modszerhez kezdo ertek, ne basztasd
#define KEZDO_ERTEK2 "  "//dupla modszerhez kezdo ertek, ezt se basztasd

#define TIMEOUT 240
#define TIMEOUT_2 60

//min 10x10
#define OSZLOPOK 15 //max 26
#define SOROK 15 //max unlimited (esszel)

#define COLOR "%^RESET%^%^BOLD%^BLACK%^" //racs szin

/*
Fuggveny prototipusok
*/

void kov_ketto(object ob);
void kov_egy(object ob);
void writepapir();
string nez_papir();
int qid(object ob);
void amoba_over();
void setpapir();
void initm();
string abc(int i);
int qturn(object ob);
string abc(int i);
int cba(string s);
void rakott(object ob,int x,int y);
int amoba(object ob,int x,int y);
void kov(object ob);
int qt(int x,int y);
int nincs_szabad();
string write_szabalyok();
int szabalyok();

/*
-----------------------------------
Kezdo valtozok
-----------------------------------
*/
mapping jatekos1=([]);
mapping jatekos2=([]);
array p,t;
int megyajatek,kovetkezo,xs,ys,holder_1,holder_2;
string papir,kikov,papir_a,papir_b,papir_c;

/*
-----------------------------------
Jatekos parancsok 
-----------------------------------
*/
string write_szabalyok(){
  string rules="";
  rules+="Amõba játék menete, és szabályai:\n\n";
  rules+="A játékban két játékos vesz részt. A kezdés véletlenszerû. A játék célja, hogy a játékos\n";
  rules+="lerakjon 5 egyforma jelet egymással érintõlegesen, egy sorban, bármely irányban.\n";
  rules+="Amelyik játékos elõbb tudja megvalósítani az 5 jelet egymással érintõlegesen, egy sorban, bármely irányban,\n";
  rules+="az a játékos nyeri meg a játékot.\n";
  rules+="Két lépés között maximum "+((TIMEOUT+TIMEOUT_2)/60)+" perc telhet el.\n";
  rules+="Amennyiben a "+((TIMEOUT+TIMEOUT_2)/60)+" perc letelik, a soron következõ játékos elveszti a játékot.\n\n";
  rules+="Játékban használható parancsok:\n";
  rules+="%^BOLD%^%^CYAN%^jelentkezik/beszall/jatszik%^RESET%^ - %^CYAN%^játékba jelentkezés\n";
  rules+="%^BOLD%^kezd/elkezd/indit/elindit%^RESET%^   - %^CYAN%^játék elindítása\n";
  rules+="%^BOLD%^kiszall/felad%^RESET%^               - %^CYAN%^játékból kiszállás\n";
  rules+="%^BOLD%^rak <oszlop> <sor>%^RESET%^          - %^CYAN%^jel rakása a papírra\n";
  rules+="%^BOLD%^szabalyok/parancsok%^RESET%^         - %^CYAN%^megjeleníti ezt a szöveget\n\n";
  rules+="%^BOLD%^%^WHITE%^Jó játékot mindenkinek! -- Slocum%^RESET%^";
  return rules;
}
int szabalyok(){
  string rules="";
  rules+="Amõba játék menete, és szabályai:\n\n";
  rules+="A játékban két játékos vesz részt. A kezdés véletlenszerû. A játék célja, hogy a játékos\n";
  rules+="lerakjon 5 egyforma jelet egymással érintõlegesen, egy sorban, bármely irányban.\n";
  rules+="Amelyik játékos elõbb tudja megvalósítani az 5 jelet egymással érintõlegesen, egy vonalban, bármely irányban,\n";
  rules+="az a játékos nyeri meg a játékot.\n";
  rules+="Két lépés között maximum "+((TIMEOUT+TIMEOUT_2)/60)+" perc telhet el.\n";
  rules+="Amennyiben a "+((TIMEOUT+TIMEOUT_2)/60)+" perc letelik, a soron következõ játékos elveszti a játékot.\n\n";
  rules+="Játékban használható parancsok:\n";
  rules+="jelentkezik/beszall/jatszik - játékba jelentkezés\n";
  rules+="kezd/elkezd/indit/elindit   - játék elindítása\n";
  rules+="kiszall/felad               - játékból kiszállás\n";
  rules+="rak <oszlop> <sor>          - jel rakása a papírra\n";
  rules+="szabalyok/parancsok         - megjeleníti ezt a szöveget\n\n";
  rules+="Jó játékot mindenkinek! -- Slocum";
  write(rules);
  return 1;
}
int signup(){
  if(qid(TP)) return notify_fail("%^BOLD%^%^RED%^Már jelentkeztél a játékra!%^RESET%^\n");
  if(OSZLOPOK>26) return notify_fail("%^BOLD%^%^RED%^A játék nem indulhat el, túl nagy lenne a papír, szóljatok egy wiznek!%^RESET%^\n");
  if(OSZLOPOK<10 || SOROK < 10) return notify_fail("%^BOLD%^%^RED%^A játék nem indulhat el, túl kicsi a papír, szóljatok egy wiznek!%^RESET%^\n");
  if(jatekos1["jatszik"]==1){
    if(jatekos2["jatszik"]==1) return notify_fail("%^BOLD%^%^RED%^A játék megtelt!%^RESET%^\n");
    jatekos2["jatszik"]=1;
    jatekos2["nev"]=TP->query_name();
    jatekos2["cnev"]=capitalize(TP->query_name());
    write("%^BOLD%^%^YELLOW%^Sikeresen jelentkeztél a játékra!%^RESET%^\n");
    say("%^BOLD%^%^RED%^"+jatekos2["cnev"]+"%^YELLOW%^ jelentkezett a játékra!%^RESET%^\n");
    if(jatekos1["jatszik"]==1) tell_object(TO,"%^BOLD%^%^YELLOW%^Az %^WHITE%^indít%^YELLOW%^ paranccsal elindíthatjátok a játékot!%^RESET%^\n");
    return 1;
  }
  jatekos1["jatszik"]=1;
  jatekos1["nev"]=TP->query_name();
  jatekos1["cnev"]=capitalize(TP->query_name());
  write("%^BOLD%^%^YELLOW%^Sikeresen jelentkeztél a játékra!%^RESET%^\n");
  say("BOLD%^%^GREEN%^"+jatekos1["cnev"]+"%^YELLOW%^ jelentkezett a játékra!%^RESET%^\n");
  if(jatekos2["jatszik"]==1) tell_object(TO,"%^BOLD%^%^YELLOW%^Az %^WHITE%^indít%^YELLOW%^ paranccsal elindíthatjátok a játékot!%^RESET%^\n");
  return 1;
}
int starter(){
  object pl1;
  object pl2;
  int i=random(2);
  if(!qid(TP)) return notify_fail("%^BOLD%^%^RED%^Te nem játszol!%^RESET%^\n");
  if(!jatekos1["jatszik"] || !jatekos2["jatszik"]) return notify_fail("%^BOLD%^%^RED%^Nem vagytok elegen a játékhoz!%^RESET%^\n");
  if(megyajatek) return notify_fail("%^BOLD%^%^RED%^Már megy a játék!%^RESET%^\n");
  megyajatek=1;
  pl1=find_player(jatekos1["nev"]);
  pl2=find_player(jatekos2["nev"]);
  if(i==1){
    kovetkezo=1;
    holder_1=1;
    call_out("kov_egy",TIMEOUT,pl2);
    kikov=jatekos1["cnev"];
    tell_room(TO,"%^BOLD%^%^WHITE%^"+jatekos1["cnev"]+" kezdi a játékot!%^RESET%^\n");
  } else {
    kovetkezo=2;
    holder_2=1;
    call_out("kov_egy",TIMEOUT,pl1);
    kikov=jatekos2["cnev"];
    tell_room(TO,"%^BOLD%^%^WHITE%^"+jatekos2["cnev"]+" kezdi a játékot!%^RESET%^\n");
  }
  initm();
  setpapir();
  if(LONGER){
    writepapir();
  }else{
    tell_room(TO,papir);
  }
  return 1;
}
int quitter(){
  if(!qid(TP)) return notify_fail("%^BOLD%^%^RED%^Te nem vagy játékban!%^RESET%^\n");
  if(megyajatek){
    write("%^BOLD%^%^RED%^Feladod a játékot!%^RESET%^\n");
    if(qid(TP)==1){
      say("%^BOLD%^%^GREEN%^"+jatekos1["cnev"]+" %^RED%^feladta a játékot!\n%^RED%^"+jatekos2["cnev"]+"%^YELLOW%^ megnyerte a játékot!%^RESET%^\n");
      if(J_MESTER) CHAT_D->do_chat("jatek",jatekos2["cnev"]+" nyert az amöba játékban!",1);
      amoba_over();
    }
    if(qid(TP)==2){
      say("%^BOLD%^%^RED%^"+jatekos2["cnev"]+" %^RED%^feladta a játékot!\n%^GREEN%^"+jatekos1["cnev"]+"%^YELLOW%^ megnyerte a játékot!%^RESET%^\n");
      if(J_MESTER) CHAT_D->do_chat("jatek",jatekos1["cnev"]+" nyert az amöba játékban!",1);
      amoba_over();
    }
    return 1;
  }
  if(qid(TP)==1){
    write("%^BOLD%^%^RED%^Visszavonod a jelentkezésed!%^RESET%^\n");
    say("%^BOLD%^%^GREEN%^"+jatekos1["cnev"]+"%^YELLOW%^ visszavonta jelentkezését!%^RESET%^\n");
    map_delete(jatekos1,"jatszik");
    map_delete(jatekos1,"nev");
    map_delete(jatekos1,"cnev");
  }
  if(qid(TP)==2){
    write("%^BOLD%^%^RED%^Visszavonod a jelentkezésed!%^RESET%^\n");
    say("%^BOLD%^%^RED%^"+jatekos2["cnev"]+"%^YELLOW%^ visszavonta jelentkezését!%^RESET%^\n");
    map_delete(jatekos2,"jatszik");
    map_delete(jatekos2,"nev");
    map_delete(jatekos2,"cnev");
  }
  return 1;
}

int rak(string str){
  string s;
  int i;
  if(!qid(TP)) return notify_fail("%^BOLD%^%^RED%^Nem vagy játékban!%^RESET%^\n");
  if(!megyajatek) return notify_fail("%^BOLD%^%^RED%^A játék még nem indult el!%^RESET%^\n");
  if(!str) return notify_fail("%^BOLD%^%^RED%^Használat: %^WHITE%^rak <oszlop> <sor>%^RESET%^\n");
  if(!qturn(TP)) return notify_fail("%^BOLD%^%^RED%^Nem rajtad van a sor!%^RESET%^\n");
  if(sscanf(str,"%s %d",s,i)!=2) return notify_fail("%^BOLD%^%^RED%^Használat: %^WHITE%^rak <oszlop> <sor>%^RESET%^\n");
  s=lower_case(s);
  i=i-1;
  if(cba(s)>OSZLOPOK-1 || i>SOROK-1) return notify_fail("%^BOLD%^%^RED%^Rossz paraméter!%^RESET%^\n");
  if(qt(cba(s),i)) return notify_fail("%^BOLD%^%^RED%^Az a mezõ már foglalt!%^RESET%^\n");
  rakott(TP,cba(s),i);
  if(qid(TP)==1) holder_1=0;
  if(qid(TP)==2) holder_2=0;
  remove_call_out("kov_egy");
  remove_call_out("kov_ketto");
  //tell_room(TO,papir);
  if(amoba(TP,cba(s),i)){
    if(LONGER){
      writepapir();
    }else{
      tell_room(TO,papir);
    }
    if(qid(TP)==1){
      tell_room(TO,"%^BOLD%^%^GREEN%^"+jatekos1["cnev"]+"%^YELLOW%^ megnyerte a játékot!%^RESET%^\n");
      if(J_MESTER) CHAT_D->do_chat("jatek",jatekos1["cnev"]+" gyõzedelmeskedett "+capitalize(jatekos2["nev"])+" fölött amõbában!",1);
      amoba_over();
      return 1;
    }
    tell_room(TO,"%^BOLD%^%^RED%^"+jatekos2["cnev"]+"%^YELLOW%^ megnyerte a játékot!%^RESET%^\n");
    if(J_MESTER) CHAT_D->do_chat("jatek",jatekos2["cnev"]+" gyõzedelmeskedett "+capitalize(jatekos1["nev"])+" fölött amõbában!",1);
    amoba_over();
    return 1;
  }
  if(nincs_szabad()){
    tell_room(TO,"%^BOLD%^%^RED%^Nincs több szabad hely a papíron, a játék döntetlen!%^RESET%^\n");
    if(J_MESTER) CHAT_D->do_chat("jatek",jatekos1["cnev"]+" és "+jatekos2["cnev"]+" döntetlent játszottak az amõba játékban!",1);
    amoba_over();
    return 1;
  }
  kov(TP);
  if(LONGER){
    writepapir();
  }else{
    tell_room(TO,papir);
  }
  return 1;
}

int kilep_a(){
  if(megyajatek){
    if(qid(TP)) tell_room(TO,"Az egyik játékos kilépett a játékbol. A játéknak vége.\n",({TP}));
    if(qid(TP)==1){
      if(J_MESTER) CHAT_D->do_chat("jatek",jatekos2["cnev"]+" nyert az amõba játékban!",1);
      amoba_over();
    }
    if(qid(TP)==2){
      if(J_MESTER) CHAT_D->do_chat("jatek",jatekos1["cnev"]+" nyert az amõba játékban!",1);
      amoba_over();
    }
  }
  if(qid(TP)){
    if(qid(TP)==1){
      tell_room(TO,"%^BOLD%^%^GREEN%^"+jatekos1["cnev"]+"%^YELLOW%^ visszavonta jelentkezését!%^RESET%^");
      map_delete(jatekos1,"jatszik");
      map_delete(jatekos1,"nev");
      map_delete(jatekos1,"cnev");
    }
    if(qid(TP)==2){
      tell_room(TO,"%^BOLD%^%^RED%^"+jatekos2["cnev"]+"%^YELLOW%^ visszavonta jelentkezését!%^RESET%^");
      map_delete(jatekos2,"jatszik");
      map_delete(jatekos2,"nev");
      map_delete(jatekos2,"cnev");
    }
  }
}
/*
-----------------------
Seged fuggvenyek
-----------------------
*/
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
  
void kov_ketto(object ob){
  if(qid(ob)==1){
    if(holder_2){
      tell_room(TO,"%^BOLD%^%^RED%^"+jatekos2["cnev"]+" nem reagált idõben.\n"+jatekos1["cnev"]+" megynerte a játékot!%^RESET%^\n");
      if(J_MESTER) CHAT_D->do_chat("jatek",jatekos1["cnev"]+" nyert az amõba játékban!",1);
      amoba_over();
      return;
  }
  }
  if(qid(ob)==2){
    if(holder_1){
      tell_room(TO,"%^BOLD%^%^RED%^"+jatekos1["cnev"]+" nem reagált idõben.\n"+jatekos2["cnev"]+" megynerte a játékot!%^RESET%^\n");
      if(J_MESTER) CHAT_D->do_chat("jatek",jatekos2["cnev"]+" nyert az amõba játékban!",1);
      amoba_over();
      return;
    }
  }
}

void kov_egy(object ob){
  if(qid(ob)==1){
    if(holder_2){
      tell_object(find_player(lower_case(jatekos2["nev"])),""+TIMEOUT_2+" másodperced van lépni, ha addig nem lépsz, elveszited a játékot!\n");
      call_out("kov_ketto",TIMEOUT_2,ob);
      return;
    }
  }
  if(qid(ob)==2){
    if(holder_1){
      tell_object(find_player(lower_case(jatekos1["nev"])),""+TIMEOUT_2+" másodperced van lépni, ha addig nem lépsz, elveszited a játékot!\n");
      call_out("kov_ketto",TIMEOUT_2,ob);
      return;
    }
  }
}

void kov(object ob){
  if(qid(ob)==1){
    kovetkezo=2;
    holder_2=1;
    call_out("kov_egy",TIMEOUT,ob);
    kikov=jatekos2["cnev"];
  }
  if(qid(ob)==2){
    kovetkezo=1;
    holder_1=1;
    call_out("kov_egy",TIMEOUT,ob);
    kikov=jatekos1["cnev"];
  }
  setpapir();
}

void amoba_over(){
  map_delete(jatekos2,"jatszik");
  map_delete(jatekos2,"nev");
  map_delete(jatekos2,"cnev");
  map_delete(jatekos1,"jatszik");
  map_delete(jatekos1,"nev");
  map_delete(jatekos1,"cnev");
  megyajatek=0;
}

int qt(int x,int y){
  return t[x][y];
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

int amoba(object ob,int x,int y){
  int id=qid(ob);
  int i;
  int szamol=0;
  //induljunk el balra fel
  for(i=1;i<8;i++){
    getter(x,y,i);
    if(papiron(xs,ys)){//1 letezik X1
      if(qt(xs,ys)==id){//1 TP X2
        getter(xs,ys,i);
        if(papiron(xs,ys)){ //2 letezik X2
          if(qt(xs,ys)==id){ //2 TP X3
            getter(xs,ys,i);
            if(papiron(xs,ys)){ // 3 letezik X3
              if(qt(xs,ys)==id){ // 3 TP X4
                getter(xs,ys,i);
                if(papiron(xs,ys)){ // 4 letezik X4
                  if(qt(xs,ys)==id){ // 4 TP X5
                    szamol++;
                    //return 1;
                  }// 4 NTP X4
                  getter(x,y,cc(i));
                  if(papiron(xs,ys)){ // 4 NTP & 5 letezik X4
                    if(qt(xs,ys)==id){ // 4 NTP & 5 TP X5
                      szamol++;
                      //return 1;
                    }// 4 NTP & 5 NTP X4
                    //return 0;
                  }//4NTP & 5 NPP X4
                  //return 0;
                }// 4 NPP X4
                getter(x,y,cc(i));
                if(papiron(xs,ys)){ // 4 NPP & 5 letezik X4
                  if(qt(xs,ys)==id){ // 4 NPP & 5 TP X5
                    szamol++;
                    //return 1;
                  }//4NPP & 5 NTP X4
                  //return 0;
                }// 4 NPP & 5 NTP X4
                //return 0;
              }// 3 NTP X3
              getter(x,y,cc(i));
              if(papiron(xs,ys)){//3NTP & 5 letezik X3
                if(qt(xs,ys)==id){ //3NTP & 5 TP X4
                  getter(xs,ys,cc(i));
                  if(papiron(xs,ys)){ // 3 NTP & 6 letezik X4
                    if(qt(xs,ys)==id){ // 3 NTP & 6 TP X5
                      szamol++;
                      //return 1;
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
                getter(xs,ys,cc(i));
                if(papiron(xs,ys)){ //3NPP & 6 letezik X4
                  if(qt(xs,ys)==id){ //3NPP & 6 TP X5
                    szamol++;
                    //return 1;
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
              getter(xs,ys,cc(i));
              if(papiron(xs,ys)){//2NTP & 6 letezik X3
                if(qt(xs,ys)==id){ //2NTP & 6 TP X4
                  getter(xs,ys,cc(i));
                  if(papiron(xs,ys)){//2NTP & 7 letezik X4
                    if(qt(xs,ys)==id){//2NTP & 7 TP X5
                      szamol++;
                      //return 1;
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
            getter(xs,ys,cc(i));
            if(papiron(xs,ys)){//2NPP & 6 letezik X3
              if(qt(xs,ys)==id){//2NPP & 6 TP X4
                getter(xs,ys,cc(i));
                if(papiron(xs,ys)){//2NPP & 7 letezik X4
                  if(qt(xs,ys)==id){//2NPP & 7 TP X5
                    szamol++;
                    //return 1;
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
          getter(xs,ys,cc(i));
          if(papiron(xs,ys)){//1NTP & 6 letezik X2
            if(qt(xs,ys)==id){//1NTP & 6 TP X3
              getter(xs,ys,cc(i));
              if(papiron(xs,ys)){//1NTP&7 letezik X3
                if(qt(xs,ys)==id){ // 1 NTP & 7 TP X4
                  getter(xs,ys,cc(i));
                  if(papiron(xs,ys)){//1NTP & 8 letezik X4
                    if(qt(xs,ys)==id){//1NTP & 8 TP X5
                      szamol++;
                      //return 1;
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
        getter(xs,ys,cc(i));
        if(papiron(xs,ys)){//1NPP & 6 letezik X2
          if(qt(xs,ys)==id){//1NPP & 6 TP X3
            getter(xs,ys,cc(i));
            if(papiron(xs,ys)){//1NPP & 7 letezik X3
              if(qt(xs,ys)==id){//1NPP & 7 TP X4
                getter(xs,ys,cc(i));
                if(papiron(xs,ys)){//1NPP & 8 letezik X4
                  if(qt(xs,ys)==id){//1NPP & 8 TP X5
                    szamol++;
                    //return 1;
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

void rakott(object ob, int x, int y){
  if(qid(ob)==1){
    t[x][y]=1;
    if(MODSZER){
      p[x][y]="%^BOLD%^%^GREEN%^OO%^RESET%^";
    }else{
      p[x][y]="%^BOLD%^%^GREEN%^O%^RESET%^";
    }
    tell_room(TO,"%^BOLD%^%^GREEN%^"+jatekos1["cnev"]+"%^YELLOW%^ rakott a %^WHITE%^["+CAP(abc(x))+"]["+(y+1)+"]%^YELLOW%^ mezõre!%^RESET%^\n");
  } else {
    t[x][y]=2;
    if(MODSZER){
      p[x][y]="%^BOLD%^%^RED%^XX%^RESET%^";
    }else{
      p[x][y]="%^BOLD%^%^RED%^X%^RESET%^";
    }
    tell_room(TO,"%^BOLD%^%^RED%^"+jatekos2["cnev"]+"%^YELLOW%^ rakott a %^WHITE%^["+CAP(abc(x))+"]["+(y+1)+"]%^YELLOW%^ mezõre!%^RESET%^\n");
  }
  setpapir();
}

int qturn(object ob){
  if(qid(ob)==kovetkezo) return 1;
  return 0;
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

void writepapir(){
  string tmp;
  int k;
  string *ss,*ss_a,*ss_b,*ss_c;
  ss=explode(papir, "\n");
  for(k=0;k<sizeof(ss);k++){
    tell_room(TO,ss[k]);
  }
  /*foreach(string tmp in ss){
    tell_room(TO,tmp);
  }*/
  /*ss_a=explode(papir_a,"\n");
  ss_b=explode(papir_b,"\n");
  ss_c=explode(papir_c,"\n");
  foreach(tmp in ss_a){
    tell_room(TO,tmp);
  }
  foreach(tmp in ss_b){
    tell_room(TO,tmp);
  }
  foreach(tmp in ss_c){
    tell_room(TO,tmp);
  }*/
  /*
  tell_room(TO,papir_a);
  tell_room(TO,papir_b);
  tell_room(TO,papir_c);*/
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
        papir+="%^BOLD%^%^YELLOW%^"+(j+1)+"%^RESET%^";
        if(MODSZER){
          papir+="\n"+COLOR+"--+%^RESET%^";
          for(j=0;j<OSZLOPOK-1;j++) papir+=""+COLOR+"--+%^RESET%^";
          papir+=""+COLOR+"--|\n%^RESET%^";
        }else{
          papir+="\n"+COLOR+"--+%^RESET%^";
          for(j=0;j<OSZLOPOK-1;j++) papir+=""+COLOR+"-+%^RESET%^";
          papir+=""+COLOR+"-|\n%^RESET%^";
        }
      }
    }
    /*-----------------*/
    for(j=0;j<SOROK;j++){
      if(j<9) papir+="%^BOLD%^%^YELLOW%^ "+(j+1)+""+COLOR+"|%^RESET%^";
          else papir+="%^BOLD%^%^YELLOW%^"+(j+1)+""+COLOR+"|%^RESET%^";
     for(i=0;i<OSZLOPOK;i++){
       papir+=p[i][j]+""+COLOR+"|%^RESET%^";
       if(i==OSZLOPOK-1){
         papir+="%^BOLD%^%^YELLOW%^"+(j+1)+"%^RESET%^";
         if(j==SOROK-1){
           if(MODSZER){
             papir+="\n"+COLOR+"---%^RESET%^";
             for(b=0;b<OSZLOPOK-1;b++) papir+=""+COLOR+"---%^RESET%^";
             papir+=""+COLOR+"--/%^RESET%^\n";
           } else{
              papir+="\n"+COLOR+"--%^RESET%^";
             for(b=0;b<OSZLOPOK-1;b++) papir+=""+COLOR+"--%^RESET%^";
             papir+=""+COLOR+"-/%^RESET%^\n";
           }
         } else {
           if(MODSZER){
             papir+="\n"+COLOR+"--+%^RESET%^";
             for(b=0;b<OSZLOPOK-1;b++) papir+=""+COLOR+"--+%^RESET%^";
             papir+=""+COLOR+"--|%^RESET%^";
             papir+="\n";
           }else{
             papir+="\n"+COLOR+"--+%^RESET%^";       
             for(b=0;b<OSZLOPOK-1;b++) papir+=""+COLOR+"-+%^RESET%^";
             papir+=""+COLOR+"-|%^RESET%^";
             papir+="\n";
           }
         }
       }
     }
   }
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

string nez_papir(){
  if(!megyajatek) return "Egy üres papirfecni.";
  if(LONGER){
    writepapir();
    return;
  }
  return papir;
}
