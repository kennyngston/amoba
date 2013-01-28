/*
Malom by Slocum
*/
#include <std.h>
#include <daemons.h>

// TIMEOUT es TIMEOUT_2 osszege mindig legyen oszthato 60 al!
#define TIMEOUT 270  //figyelmeztetesig mp
#define TIMEOUT_2 30 //utso figyelmeztetes lepesre, ezutan kidobja
#define J_MESTER 1 //ha 1, jatekmester beirja a nyertest jatek csatira

inherit ROOM;

nez_tabla();
szinez();
szabalyok();

void create() {
  ::create();
  set_property("indoors", 1);
  set_property("light", 3);
  set_property("no walking mons", 1);
  set_property("no attack", 1);
  set_property("no teleport", 1);
  set_property("no steal", 1);
  set_property("no magic", 1);
  set_short("Dohos szoba");
  set_long("Egy jól megvilágított szobában vagy. Ahogy körbenézel, látod hogy a falakat\n"
           "fával burkoltak be, a padlót pedig puha szõnyeg takarja el a világ elõl.\n"
           "A szoba közepén egy %^ORANGE%^asztal%^RESET%^t látsz, két %^ORANGE%^székkel%^RESET%^. "
           "A falon egy %^BOLD%^%^YELLOW%^tábla%^RESET%^t pillantasz meg.\n"
           "A %^BOLD%^%^YELLOW%^tábla%^RESET%^ mellett egy cetlire a %^BOLD%^%^WHITE%^szabályok%^RESET%^ és %^BOLD%^%^WHITE%^parancsok%^RESET%^ vannak felírva.");
  set_items(([
    "tabla" : (: nez_tabla() :),
    "asztal" : "Egy régi asztal, egy malom tábla van rajta.",
    ({"szabalyok","parancsok","cetli"}) : (: szabalyok() :),
    ({"szek","szekek"}) : "Két, kényelmes fa szék.",
    ({"fal","falak"}) : "Fával burkolt fal.",
    "padlo" : "Régi szõnyeggel borított padló.",
    "szonyeg" : "Egy régi, kézzel szövött szönyeg."
  ]));
  set_exits( ([ "le" : "/domains/raknoor/rooms/kocsma4" ]) );
  set_smell2( ([
    "default" : "Füstölö illata terjeng a levegõben." ]) );
  set_listen2( ([
    "default" : "Malom bábúk kopogását hallod." ]) );
}

void init(){
  ::init();
  add_action("jelentkezik","jelentkezik");
  add_action("kiszall","kiszall");
  add_action("indit","indit");
  add_action("rak","rak");
  add_action("levesz","levesz");
  add_action("szabalyok",({"szabalyok","parancsok"}));
  add_action("felad","felad");
  add_action("kilep_a","kilep");
}

//szabalyok kiirasa
int szabalyok(){
  write("Malom szabályok és parancsok:\n"
        "Ket játékos játszik, mind a ketten %^BOLD%^%^YELLOW%^kilenc%^RESET%^ bábuval kezdenek. Elején ezeket a bábúkat kell lerakni a táblára.\n"
        "Ha három bábu egy sorban van, akkor van malom. Malomnál le lehet venni egyet az ellenfél bábui közül,\n"
        "kivéve ha a bábu malomban van. Ha az ellenfél összes bábuja malomban van, akkor nem lehet bábut levenni.\n"
        "Ha az összes bábut leraktad a kezedbõl, már csak tologathatsz, szomszédos mezõkre. Ha kevesebb mint %^BOLD%^%^YELLOW%^három%^RESET%^\n"
        "bábud van a táblán, akkor tudsz ugrálni, nem csak szomszádos mezõkre.\n"
        "Az nyer akinek sikerul az ellenfele bábuinak számát kettõre csökkentenie.\n"
        "Két lépés között maximum "+((TIMEOUT+TIMEOUT_2)/60)+" perc telhet el!\n\n"
        "Parancsok: %^CYAN%^jelentkezik%^RESET%^              -ezzel lehet a játekba jelentkezni, max 2 fõ\n"
        "           %^CYAN%^kiszall%^RESET%^                  -ezzel vonhatod vissza a jelentkezést\n"
        "           %^CYAN%^indit%^RESET%^                    -ezzel lehet elinditani a játékot\n"
        "           %^CYAN%^rak <hova/honnan> <hova>%^RESET%^ -ezzel tudod lerakni, és mozgatni a bábuid\n"
        "           %^CYAN%^levesz <honnan>%^RESET%^          -ha malmod van, ezzel tudsz levenni az ellenfeled bábui közül\n"
        "           %^CYAN%^felad%^RESET%^                    -ezzel tudod feladni a játékot\n\n\n"
        "                                                                    %^BOLD%^%^WHITE%^Jó játékot! - Slocum%^RESET%^");
  return 1;
}

/*
mapping felirasa a ket jatekosra, mezokre szinere, es a mezo tulajdonosara
jatekosoknal: id: jatekos ID-je a jatekban (1/2)
              jel: jelentkezett-e a jatekra (1/0)
              nev: a jatekos neve, ha nem jelentkezett senki, akkor ez "Senki"
              babuk: a jatekos kezben levo babuinak szama
              babuk2: a jatekos tablan levo babuinak szama
              malom: van-e aktiv malma a jatekosnak
              obabuk: ossz babu, levetelnel 1el csokken
int tombok:   adott jatekosnak a tulajdonaban levo mezoket tartalmazzak
mezo:         24 mezo van a tablan, ez a mapping a szineit tarolja, szurke,
              senkie, zold egyes jatekos, piros kettes jatekos
tulajdonos:   adott mezo kinek a tulajdonaban van, 0 senki, 1 egyes jatekos,
              2 kettes jatekos
*/
mapping jatekos1 = ([ "id" : 1, "jel" : 0,"nev" : "Senki", "babuk" : 9, "babuk2" : 0, "malom" : 0, "obabuk" : 9 ]);
mapping jatekos2 = ([ "id" : 2, "jel" : 0,"nev" : "Senki", "babuk" : 9, "babuk2" : 0, "malom" : 0, "obabuk" : 9 ]);
int *j1t=({});
int *j2t=({});
mapping mezo=([1:"001",2:"002",3:"003",4:"004",5:"005",6:"006",7:"007",8:"008",9:"009",10:"010",11:"011",12:"012",13:"013",14:"014",15:"015",16:"016",17:"017",18:"018",19:"019",20:"020",21:"021",22:"022",23:"023",24:"024"]);
mapping tulajdonos=([1:0,2:0,3:0,4:0,5:0,6:0,7:0,8:0,9:0,10:0,11:0,12:0,13:0,14:0,15:0,16:0,17:0,18:0,19:0,20:0,21:0,22:0,23:0,24:0]);

/*
Kezdo ertekek megadasa
megyajatek: ha megy a jatek, ez 1
whosnext:   melyik jatekos kovetkezik
holder_1,2: adott jatekosra varunk
tabla:      maga a tabla
*/
int megyajatek=0;
int whosnext=0;
int holder_1=0;
int holder_2=0;
string tabla;
string kovetkezik="A játék még nem indult el!";
//ossz babu szam jatek vegere
int qosszbabu(int i){
  if(i==1) return jatekos1["obabuk"];
  if(i==2) return jatekos2["obabuk"];
  return 0;
}

//tabla ertekenek frissitese, egyebkent szineket elbassza
void settabla(){
  tabla=("\n\n"+mezo[1]+"%^BOLD%^%^WHITE%^---------%^RESET%^"+mezo[2]+"%^BOLD%^%^WHITE%^---------%^RESET%^"+mezo[3]+"         %^BOLD%^%^GREEN%^"+jatekos1["nev"]+"%^RESET%^:\n"
         " %^BOLD%^%^WHITE%^|           |           |          %^BOLD%^%^YELLOW%^"+jatekos1["babuk"]+"%^WHITE%^ kézben.%^RESET%^\n"
         " %^BOLD%^%^WHITE%^|%^RESET%^  "+mezo[4]+"%^BOLD%^%^WHITE%^-----%^RESET%^"+mezo[5]+"%^BOLD%^%^WHITE%^-----%^RESET%^"+mezo[6]+"  %^BOLD%^%^WHITE%^|          %^BOLD%^%^YELLOW%^"+jatekos1["babuk2"]+"%^CYAN%^ táblán.%^RESET%^\n"
         " %^BOLD%^%^WHITE%^|   |       |       |   |          %^BOLD%^%^WHITE%^----------%^RESET%^\n"
         " %^BOLD%^%^WHITE%^|   |%^RESET%^  "+mezo[7]+"%^BOLD%^%^WHITE%^-%^RESET%^"+mezo[8]+"%^BOLD%^%^WHITE%^-%^RESET%^"+mezo[9]+"  %^BOLD%^%^WHITE%^|   |          %^BOLD%^%^RED%^"+jatekos2["nev"]+"%^RESET%^:\n"
         " %^BOLD%^%^WHITE%^|   |   |       |   |   |          %^BOLD%^%^YELLOW%^"+jatekos2["babuk"]+"%^WHITE%^ kézben.%^RESET%^\n"
         ""+mezo[10]+"%^BOLD%^%^WHITE%^-%^RESET%^"+mezo[11]+"%^BOLD%^%^WHITE%^-%^RESET%^"+mezo[12]+"     "+mezo[13]+"%^BOLD%^%^WHITE%^-%^RESET%^"+mezo[14]+"%^BOLD%^%^WHITE%^-%^RESET%^"+mezo[15]+"         %^BOLD%^%^YELLOW%^"+jatekos2["babuk2"]+"%^CYAN%^ táblán.%^RESET%^\n"
         " %^BOLD%^%^WHITE%^|   |   |       |   |   |%^RESET%^ \n"
         " %^BOLD%^%^WHITE%^|   |%^RESET%^  "+mezo[16]+"%^BOLD%^%^WHITE%^-%^RESET%^"+mezo[17]+"%^BOLD%^%^WHITE%^-%^RESET%^"+mezo[18]+"  %^BOLD%^%^WHITE%^|   |%^RESET%^ \n"
         " %^BOLD%^%^WHITE%^|   |       |       |   |%^RESET%^          %^BOLD%^%^YELLOW%^"+kovetkezik+"%^RESET%^\n"
         " %^BOLD%^%^WHITE%^|%^RESET%^  "+mezo[19]+"%^BOLD%^%^WHITE%^-----%^RESET%^"+mezo[20]+"%^BOLD%^%^WHITE%^-----%^RESET%^"+mezo[21]+"  %^BOLD%^%^WHITE%^|%^RESET%^\n"
         " %^BOLD%^%^WHITE%^|           |           |%^RESET%^ \n"
         ""+mezo[22]+"%^BOLD%^%^WHITE%^---------%^RESET%^"+mezo[23]+"%^BOLD%^%^WHITE%^---------%^RESET%^"+mezo[24]+"\n%^RESET%^");
}

//jatek vege, alapertekek visszallitasa
void malom_over(){
  int i;
  for(i=0;i<25;i++){
    szinez(TP,i,1);
    tulajdonos[i]=0;
  }
  jatekos1["jel"]=0;
  jatekos1["nev"]="Senki";
  jatekos1["malom"]=0;
  jatekos1["babuk"]=9;
  jatekos1["babuk2"]=0;
  jatekos1["obabuk"]=9;
  jatekos2["jel"]=0;
  jatekos2["nev"]="Senki";
  jatekos2["malom"]=0;
  jatekos2["babuk"]=9;
  jatekos2["babuk2"]=0;
  jatekos2["obabuk"]=9;
  whosnext=0;
  megyajatek=0;
  kovetkezik="A játék még nem indult el!";
  settabla();
}

//jatekos ID lekerdezese
int qid(object ob){
  if(capitalize(ob->query_name())==jatekos1["nev"]) return 1;
  if(capitalize(ob->query_name())==jatekos2["nev"]) return 2;
  return 0;
}

//mezo tulajdonos lekerdezese
int qt(int i){
  return tulajdonos[i];
}
//tulajdonos beallitasa
void st(object ob, int i){
  tulajdonos[i]=qid(ob);
}
//tulajdonos nullazasa
void st2(int i){
  tulajdonos[i]=0;
}

//h-mezorol lehet-e lepni
int qlephet(int h){
  switch (h){
    case 001 : if(qt(2)&&qt(10)) return 0;break;
    case 002 : if(qt(1)&&qt(3)&&qt(5)) return 0;break;
    case 003 : if(qt(2)&&qt(15)) return 0;break;
    case 004 : if(qt(5)&&qt(11)) return 0;break;
    case 005 : if(qt(2)&&qt(4)&&qt(6)&&qt(8)) return 0;break;
    case 006 : if(qt(5)&&qt(14)) return 0;break;
    case 007 : if(qt(8)&&qt(12)) return 0;break;
    case 008 : if(qt(5)&&qt(7)&&qt(9)) return 0;break;
    case 009 : if(qt(8)&&qt(13)) return 0;break;
    case 010 : if(qt(1)&&qt(11)&&qt(22)) return 0;break;
    case 011 : if(qt(4)&&qt(10)&&qt(12)&&qt(19)) return 0;break;
    case 012 : if(qt(7)&&qt(11)&&qt(16)) return 0;break;
    case 013 : if(qt(9)&&qt(14)&&qt(18)) return 0;break;
    case 014 : if(qt(6)&&qt(13)&&qt(15)&&qt(21)) return 0;break;
    case 015 : if(qt(3)&&qt(14)&&qt(24)) return 0;break;
    case 016 : if(qt(12)&&qt(17)) return 0;break;
    case 017 : if(qt(16)&&qt(18)&&qt(20)) return 0;break;
    case 018 : if(qt(13)&&qt(17)) return 0;break;
    case 019 : if(qt(11)&&qt(20)) return 0;break;
    case 020 : if(qt(17)&&qt(19)&&qt(21)&&qt(23)) return 0;break;
    case 021 : if(qt(14)&&qt(20)) return 0;break;
    case 022 : if(qt(10)&&qt(23)) return 0;break;
    case 023 : if(qt(20)&&qt(22)&&qt(24)) return 0;break;
    case 024 : if(qt(15)&&qt(23)) return 0;break;
  }
  return 1;
}

//tud-e lepni h-idju jatekos
int qlep(int h){
  int i,x=0;
  if (h==1){
    for(i=0;i<sizeof(j1t);i++){
      if(qlephet(j1t[i])){
        x++;
      }
    }
  }
  if (h==2){
    for(i=0;i<sizeof(j2t);i++){
      if(qlephet(j2t[i])){
        x++;
      }
    }
  }
  return x;
}

void kov_ketto(object ob){
  if(qid(ob)==1){
    if(holder_2){
      tell_room(TO,"%^BOLD%^%^RED%^"+jatekos2["nev"]+" nem reagált idõben.\n%^YELLOW%^"+jatekos1["nev"]+" megynerte a játékot!\n%^RESET%^");
      if(J_MESTER) CHAT_D->do_chat("jatek",jatekos1["nev"]+" nyert a malom játékban!",1);
      malom_over();
      return;
  }
  }
  if(qid(ob)==2){
    if(holder_1){
      tell_room(TO,"%^BOLD%^%^RED%^"+jatekos1["nev"]+" nem reagált idõben.\n%^YELLOW%^"+jatekos2["nev"]+" megynerte a játékot!\n%^RESET%^");
      if(J_MESTER) CHAT_D->do_chat("jatek",jatekos2["nev"]+" nyert a malom játékban!",1);
      malom_over();
      return;
    }
  }
}

void kov_egy(object ob){
  if(qid(ob)==1){
    if(holder_2){
      tell_object(find_player(lower_case(jatekos2["nev"])),"%^BOLD%^%^RED%^"+TIMEOUT_2+" másodperced van lépni, ha addig nem lépsz, elveszited a játékot!%^RESET%^\n");
      call_out("kov_ketto",TIMEOUT_2,ob);
      return;
    }
  }
  if(qid(ob)==2){
    if(holder_1){
      tell_object(find_player(lower_case(jatekos1["nev"])),"%^BOLD%^%^RED%^"+TIMEOUT_2+" másodperced van lépni, ha addig nem lépsz, elveszited a játékot!%^RESET%^\n");
      call_out("kov_ketto",TIMEOUT_2,ob);
      return;
    }
  }
}

//kovetkezo jatekos
void kovetkezo(object ob){
  if(qid(ob)==1){
    if(!qlep(2)&&jatekos2["babuk2"]>3&&jatekos2["babuk"]==0){
      tell_room(TO,"%^BOLD%^%^RED%^"+jatekos2["nev"]+"%^WHITE%^ nem tud lépni, %^GREEN%^"+jatekos1["nev"]+"%^WHITE%^ következik.%^RESET%^\n");
      holder_1=1;
      return;
    }
    whosnext=2;
    kovetkezik=(jatekos2["nev"]+" következik.");
    tell_room(TO,"%^BOLD%^%^RED%^"+jatekos2["nev"]+"%^WHITE%^ következik.%^RESET%^\n");
    holder_2=1;
    call_out("kov_egy",TIMEOUT,ob);
  }
  if(qid(ob)==2){
    if(!qlep(1)&&jatekos1["babuk2"]>3&&jatekos1["babuk"]==0){
      tell_room(TO,"%^BOLD%^%^GREEN%^"+jatekos1["nev"]+"%^WHITE%^ nem tud lépni, %^RED%^"+jatekos2["nev"]+"%^WHITE%^ következik.%^RESET%^\n");
      holder_2=1;
      return;
    }
    whosnext=1;
    kovetkezik=(jatekos1["nev"]+" következik.");
    tell_room(TO,"%^BOLD%^%^GREEN%^"+jatekos1["nev"]+"%^WHITE%^ következik.%^RESET%^\n");
    holder_1=1;
    call_out("kov_egy",TIMEOUT,ob);
  }
}

//ob-e a kovetkezo jatekos
int qturn(object ob){
  if(qid(ob)==whosnext) return 1;
  return 0;
}

//babuk lekerdezese
int qbabuk(object ob){
  if(qid(ob)==1) return jatekos1["babuk"];
  if(qid(ob)==2) return jatekos2["babuk"];
  return 0;
}
int qbabuk2(object ob){
  if(qid(ob)==1) return jatekos1["babuk2"];
  if(qid(ob)==2) return jatekos2["babuk2"];
  return 0;
}

//h hely malomban van-e
int qmalom2(int h){
  int i=qt(h);
  if(h==1 && qt(2)==i && qt(3)==i) return 1;
  if(h==1 && qt(10)==i && qt(22)==i) return 1;
  if(h==2 && qt(1)==i && qt(3)==i) return 1;
  if(h==2 && qt(5)==i && qt(8)==i) return 1;
  if(h==3 && qt(2)==i && qt(1)==i) return 1;
  if(h==3 && qt(15)==i && qt(24)==i) return 1;
  if(h==4 && qt(5)==i && qt(6)==i) return 1;
  if(h==4 && qt(11)==i && qt(19)==i) return 1;
  if(h==5 && qt(4)==i && qt(6)==i) return 1;
  if(h==5 && qt(2)==i && qt(8)==i) return 1;
  if(h==6 && qt(4)==i && qt(5)==i) return 1;
  if(h==6 && qt(14)==i && qt(21)==i) return 1;
  if(h==7 && qt(8)==i && qt(9)==i) return 1;
  if(h==7 && qt(12)==i && qt(16)==i) return 1;
  if(h==8 && qt(7)==i && qt(9)==i) return 1;
  if(h==8 && qt(2)==i && qt(5)==i) return 1;
  if(h==9 && qt(7)==i && qt(8)==i) return 1;
  if(h==9 && qt(13)==i && qt(18)==i) return 1;
  if(h==10 && qt(1)==i && qt(22)==i) return 1;
  if(h==10 && qt(11)==i && qt(12)==i) return 1;
  if(h==11 && qt(10)==i && qt(12)==i) return 1;
  if(h==11 && qt(4)==i && qt(19)==i) return 1;
  if(h==12 && qt(10)==i && qt(11)==i) return 1;
  if(h==12 && qt(7)==i && qt(16)==i) return 1;
  if(h==13 && qt(14)==i && qt(15)==i) return 1;
  if(h==13 && qt(9)==i && qt(18)==i) return 1;
  if(h==14 && qt(13)==i && qt(15)==i) return 1;
  if(h==14 && qt(6)==i && qt(21)==i) return 1;
  if(h==15 && qt(13)==i && qt(14)==i) return 1;
  if(h==15 && qt(3)==i && qt(24)==i) return 1;
  if(h==16 && qt(17)==i && qt(18)==i) return 1;
  if(h==16 && qt(7)==i && qt(12)==i) return 1;
  if(h==17 && qt(16)==i && qt(18)==i) return 1;
  if(h==17 && qt(20)==i && qt(23)==i) return 1;
  if(h==18 && qt(16)==i && qt(17)==i) return 1;
  if(h==18 && qt(9)==i && qt(13)==i) return 1;
  if(h==19 && qt(20)==i && qt(21)==i) return 1;
  if(h==19 && qt(4)==i && qt(11)==i) return 1;
  if(h==20 && qt(17)==i && qt(23)==i) return 1;
  if(h==20 && qt(19)==i && qt(21)==i) return 1;
  if(h==21 && qt(19)==i && qt(20)==i) return 1;
  if(h==21 && qt(6)==i && qt(14)==i) return 1;
  if(h==22 && qt(1)==i && qt(10)==i) return 1;
  if(h==22 && qt(23)==i && qt(24)==i) return 1;
  if(h==23 && qt(22)==i && qt(24)==i) return 1;
  if(h==23 && qt(17)==i && qt(20)==i) return 1;
  if(h==24 && qt(22)==i && qt(23)==i) return 1;
  if(h==24 && qt(3)==i && qt(15)==i) return 1;
  return 0;  
}
//ket mezo szomszedos-e
int szomszed(int i, int h){
  if(i==1){
    if(h==2 || h==10) return 1;
    return 0;
  }
  if(i==2){
    if(h==1 || h==3 || h==5) return 1;
    return 0;
  }
  if(i==3){
    if(h==2 || h==15) return 1;
    return 0;
  }
  if(i==4){
    if(h==5 || h==11) return 1;
    return 0;
  }
  if(i==5){
    if(h==2 || h==4 || h==6 || h==8) return 1;
    return 0;
  }
  if(i==6){
    if(h==5 || h==14) return 1;
    return 0;
  }
  if(i==7){
    if(h==8 || h==12) return 1;
    return 0;
  }
  if(i==8){
    if(h==5 || h==7 || h==9) return 1;
    return 0;
  }
  if(i==9){
    if(h==8 || h==13) return 1;
    return 0;
  }
  if(i==10){
    if(h==1 || h==11 || h==22) return 1;
    return 0;
  }
  if(i==11){
    if(h==4 || h==10 || h==12 || h==19) return 1;
    return 0;
  }
  if(i==12){
    if(h==7 || h==11 || h==16) return 1;
    return 0;
  }
  if(i==13){
    if(h==9 || h==14 || h==18) return 1;
    return 0;
  }
  if(i==14){
    if(h==6 || h==13 || h==15 || h==21) return 1;
    return 0;
  }
  if(i==15){
    if(h==3 || h==14 || h==24) return 1;
    return 0;
  }
  if(i==16){
    if(h==12 || h==17) return 1;
    return 0;
  }
  if(i==17){
    if(h==16 || h==18 || h==20) return 1;
    return 0;
  }
  if(i==18){
    if(h==13 || h==17) return 1;
    return 0;
  }
  if(i==19){
    if(h==11 || h==20) return 1;
    return 0;
  }
  if(i==20){
    if(h==17 || h==19 || h==21 || h==23) return 1;
    return 0;
  }
  if(i==21){
    if(h==14 || h==20) return 1;
    return 0;
  }
  if(i==22){
    if(h==10 || h==23) return 1;
    return 0;
  }
  if(i==23){
    if(h==20 || h==22 || h==24) return 1;
    return 0;
  }
  if(i==24){
    if(h==15 || h==23) return 1;
    return 0;
  }
  return 0;
}

//van-e leszedheto babu ob ellenfelenel
int qvanle(object ob){
  int i;
  int x=0;
  if(qid(ob)==1){
    for(i=0;i<sizeof(j2t);i++){
      if(qmalom2(j2t[i])) x++;
    }
    if(x==sizeof(j2t)) return 0;
    return 1;
  }
  if(qid(ob)==2){
    for(i=0;i<sizeof(j1t);i++){
      if(qmalom2(j1t[i])) x++;
    }
    if(x==sizeof(j1t)) return 0;
    return 1;
  }
}

//jatekosra iranyulo basztatas
//kezben levo babukbol levonas
void mbabuk(object ob){
  if(qid(ob)==1) jatekos1["babuk"]--;
  if(qid(ob)==2) jatekos2["babuk"]--;
}
//ob-nak aktiv malma van
void malom(object ob){
  if(qid(ob)==1) jatekos1["malom"]=1;
  if(qid(ob)==2) jatekos2["malom"]=1;
}

//jatekba jelentkezes
int jelentkezik(){
  if(qid(TP)){
    write("%^BOLD%^%^RED%^Már jelentkeztél a játékra!%^RESET%^\n");
    return 1;
  }
  if(jatekos1["jel"]==1){
    if(jatekos2["jel"]==1){
      return notify_fail("%^BOLD%^%^RED%^Sajnos a játék tele van.%^RESET%^\n");
    }
    jatekos2["jel"]=1;
    jatekos2["nev"]=capitalize(TP->query_name());
    settabla();
    write("%^BOLD%^%^YELLOW%^Sikeresen jelentkeztél a játékra. Az %^BOLD%^%^WHITE%^indit%^YELLOW%^ parancsal elindíthatjátok a játékot.%^RESET%^\n");
    say("%^BOLD%^%^RED%^"+capitalize(TP->query_name()) +"%^YELLOW%^ jelentkezett a játékra. Az %^BOLD%^%^WHITE%^indit%^YELLOW%^ parancsal elindíthatjátok a játékot.%^RESET%^\n");
    return 1;
  }
  jatekos1["jel"]=1;
  jatekos1["nev"]=capitalize(TP->query_name());
  settabla();
  if(jatekos2["jel"]==1){
    write("%^BOLD%^%^YELLOW%^Sikeresen jelentkeztél a játékra. Az %^BOLD%^%^WHITE%^indit%^YELLOW%^ parancsal elindíthatjátok a játékot.%^RESET%^\n");
    say("%^BOLD%^%^RED%^"+capitalize(TP->query_name()) +"%^YELLOW%^ jelentkezett a játékra. Az %^BOLD%^%^WHITE%^indit%^YELLOW%^ parancsal elindíthatjátok a játékot.%^RESET%^\n");
    return 1;
  }
  write("%^BOLD%^%^YELLOW%^Sikeresen jelentkeztél a játékra.%^RESET%^\n");
  say("%^BOLD%^%^GREEN%^"+jatekos1["nev"]+"%^YELLOW%^ jelentkezett a játékra.\n%^RESET%^");
  return 1;
}

//jatek inditasa
int indit(){
  int kikezd;
  if(!qid(TP)) return notify_fail("%^BOLD%^%^RED%^Csak a játékban lévõk indíthatják el a játékot!%^RESET%^\n");
  if(megyajatek) return notify_fail("%^BOLD%^%^RED%^Már megy a játék!%^RESET%^\n");
  if(jatekos1["jel"]==0 || jatekos2["jel"]==0) return notify_fail("%^BOLD%^%^RED%^Nem vagytok elegen a játékhoz!%^RESET%^\n");
  kikezd=random(2);
  megyajatek=1;
  tell_room(TO,"%^BOLD%^%^GREEN%^"+jatekos1["nev"]+"%^YELLOW%^ és %^BOLD%^%^RED%^"+jatekos2["nev"]+"%^YELLOW%^ új játékba kezdtek.%^RESET%^\n");
  if(kikezd==1){
    whosnext=1;
    kovetkezik=(jatekos1["nev"]+" következik.");
    tell_room(TO,"%^BOLD%^%^WHITE%^"+jatekos1["nev"]+" kezd!%^RESET%^");
  } else {
    whosnext=2;
    kovetkezik=(jatekos2["nev"]+" következik.");
    tell_room(TO,"%^BOLD%^%^WHITE%^"+jatekos2["nev"]+" kezd!%^RESET%^");
  }
  settabla();
  tell_room(TO,tabla);
  return 1;
}

//ha meg nem indult el a jatek, akkor jatekbol kiszallas
int kiszall(){
  if(!qid(TP)) return notify_fail("%^BOLD%^%^RED%^Te nem is jelentkeztél a játékra!%^RESET%^\n");
  if(megyajatek) return notify_fail("%^BOLD%^%^RED%^A játék már elkezdõdött, nem tudsz kiszállni!%^RESET%^\n");
  write("%^BOLD%^%^YELLOW%^Kiszállsz a játékból.%^RESET%^");
  if(qid(TP)==1){
    say("%^BOLD%^%^GREEN%^"+capitalize(TP->query_name())+"%^YELLOW%^ kiszállt a játékból.%^RESET%^\n");
    jatekos1["nev"]="Senki";
    jatekos1["jel"]=0;
    settabla();
    return 1;
  }
  say("%^BOLD%^%^RED%^"+capitalize(TP->query_name())+"%^YELLOW%^ kiszállt a játékból.%^RESET%^\n");
  jatekos2["nev"]="Senki";
  jatekos2["jel"]=0;
  settabla();
  return 1;
}

//mezok szinezese 0 szurke, 1 zold, 2 piros
//az elso modszer bugos
/*
void szinez(object ob, int msz, int e){
  int i= (!e ? qid(ob) : 0);
  if (i) mezo[msz] = "%^BOLD%^" + (i != 1 ? "GREEN" : "RED") + "%^" + sprintf("%.3d", i) + "%^RESET%^" ;
  else mezo[msz] = sprintf("%.3d", i) ;
}
*/

void szinez(object ob, int msz, int e){
  int i=qid(ob);
  if(e) i=0;
  //zoldre
  if(i==1){
      switch(msz){
      case 1 : mezo[1]="%^BOLD%^%^GREEN%^001%^RESET%^";break;
      case 2 : mezo[2]="%^BOLD%^%^GREEN%^002%^RESET%^";break;
      case 3 : mezo[3]="%^BOLD%^%^GREEN%^003%^RESET%^";break;
      case 4 : mezo[4]="%^BOLD%^%^GREEN%^004%^RESET%^";break;
      case 5 : mezo[5]="%^BOLD%^%^GREEN%^005%^RESET%^";break;
      case 6 : mezo[6]="%^BOLD%^%^GREEN%^006%^RESET%^";break;
      case 7 : mezo[7]="%^BOLD%^%^GREEN%^007%^RESET%^";break;
      case 8 : mezo[8]="%^BOLD%^%^GREEN%^008%^RESET%^";break;
      case 9 : mezo[9]="%^BOLD%^%^GREEN%^009%^RESET%^";break;
      case 10 : mezo[10]="%^BOLD%^%^GREEN%^010%^RESET%^";break;
      case 11 : mezo[11]="%^BOLD%^%^GREEN%^011%^RESET%^";break;
      case 12 : mezo[12]="%^BOLD%^%^GREEN%^012%^RESET%^";break;
      case 13 : mezo[13]="%^BOLD%^%^GREEN%^013%^RESET%^";break;
      case 14 : mezo[14]="%^BOLD%^%^GREEN%^014%^RESET%^";break;
      case 15 : mezo[15]="%^BOLD%^%^GREEN%^015%^RESET%^";break;
      case 16 : mezo[16]="%^BOLD%^%^GREEN%^016%^RESET%^";break;
      case 17 : mezo[17]="%^BOLD%^%^GREEN%^017%^RESET%^";break;
      case 18 : mezo[18]="%^BOLD%^%^GREEN%^018%^RESET%^";break;
      case 19 : mezo[19]="%^BOLD%^%^GREEN%^019%^RESET%^";break;
      case 20 : mezo[20]="%^BOLD%^%^GREEN%^020%^RESET%^";break;
      case 21 : mezo[21]="%^BOLD%^%^GREEN%^021%^RESET%^";break;
      case 22 : mezo[22]="%^BOLD%^%^GREEN%^022%^RESET%^";break;
      case 23 : mezo[23]="%^BOLD%^%^GREEN%^023%^RESET%^";break;
      case 24 : mezo[24]="%^BOLD%^%^GREEN%^024%^RESET%^";break;
    }
    settabla();
    return;
  }
  //pirosra
  if(i==2){
        switch(msz){
        case 1 : mezo[1]="%^BOLD%^%^RED%^001%^RESET%^";break;
        case 2 : mezo[2]="%^BOLD%^%^RED%^002%^RESET%^";break;
        case 3 : mezo[3]="%^BOLD%^%^RED%^003%^RESET%^";break;
        case 4 : mezo[4]="%^BOLD%^%^RED%^004%^RESET%^";break;
        case 5 : mezo[5]="%^BOLD%^%^RED%^005%^RESET%^";break;
        case 6 : mezo[6]="%^BOLD%^%^RED%^006%^RESET%^";break;
        case 7 : mezo[7]="%^BOLD%^%^RED%^007%^RESET%^";break;
        case 8 : mezo[8]="%^BOLD%^%^RED%^008%^RESET%^";break;
        case 9 : mezo[9]="%^BOLD%^%^RED%^009%^RESET%^";break;
        case 10 : mezo[10]="%^BOLD%^%^RED%^010%^RESET%^";break;
        case 11 : mezo[11]="%^BOLD%^%^RED%^011%^RESET%^";break;
        case 12 : mezo[12]="%^BOLD%^%^RED%^012%^RESET%^";break;
        case 14 : mezo[14]="%^BOLD%^%^RED%^014%^RESET%^";break;
        case 13 : mezo[13]="%^BOLD%^%^RED%^013%^RESET%^";break;
        case 15 : mezo[15]="%^BOLD%^%^RED%^015%^RESET%^";break;
        case 16 : mezo[16]="%^BOLD%^%^RED%^016%^RESET%^";break;
        case 17 : mezo[17]="%^BOLD%^%^RED%^017%^RESET%^";break;
        case 18 : mezo[18]="%^BOLD%^%^RED%^018%^RESET%^";break;
        case 19 : mezo[19]="%^BOLD%^%^RED%^019%^RESET%^";break;
        case 20 : mezo[20]="%^BOLD%^%^RED%^020%^RESET%^";break;
        case 21 : mezo[21]="%^BOLD%^%^RED%^021%^RESET%^";break;
        case 22 : mezo[22]="%^BOLD%^%^RED%^022%^RESET%^";break;
        case 23 : mezo[23]="%^BOLD%^%^RED%^023%^RESET%^";break;
        case 24 : mezo[24]="%^BOLD%^%^RED%^024%^RESET%^";break;
    }
    settabla();
    return;
  }
  //szurkere
  switch(msz){
        case 1 : mezo[1]="001";break;
        case 2 : mezo[2]="002";break;
        case 3 : mezo[3]="003";break;
        case 4 : mezo[4]="004";break;
        case 5 : mezo[5]="005";break;
        case 6 : mezo[6]="006";break;
        case 7 : mezo[7]="007";break;
        case 8 : mezo[8]="008";break;
        case 9 : mezo[9]="009";break;
        case 10 : mezo[10]="010";break;
        case 11 : mezo[11]="011";break;
        case 12 : mezo[12]="012";break;
        case 14 : mezo[14]="014";break;
        case 13 : mezo[13]="013";break;
        case 15 : mezo[15]="015";break;
        case 16 : mezo[16]="016";break;
        case 17 : mezo[17]="017";break;
        case 18 : mezo[18]="018";break;
        case 19 : mezo[19]="019";break;
        case 20 : mezo[20]="020";break;
        case 21 : mezo[21]="021";break;
        case 22 : mezo[22]="022";break;
        case 23 : mezo[23]="023";break;
        case 24 : mezo[24]="024";break;
    }
    settabla();
}

//babu mozgatashoz seged fuggvenyek
void leut(object ob, int h){
  szinez(TP,h,1);
  if(qt(h)==1){
    j1t-=({h});
    jatekos1["babuk2"]--;
    jatekos1["obabuk"]--;
    if(qosszbabu(1)==2) malom_over();
  }
  if(qt(h)==2){
    j2t-=({h});
    jatekos2["babuk2"]--;
    jatekos2["obabuk"]--;
    if(qosszbabu(2)==2) malom_over();
  }  
  st2(h);
  settabla();
}
void rakott(object ob,int h){
  szinez(TP,h,0);
  st(TP,h);
  if(qid(TP)==1){
    j1t+=({h});
    jatekos1["babuk2"]++;
  }
  if(qid(TP)==2){
    j2t+=({h});
    jatekos2["babuk2"]++;
  }
  settabla();
}
void levett(object ob, int h){
  szinez(TP,h,1);
  st2(h);
  if(qid(TP)==1){
    j1t-=({h});
    jatekos1["babuk2"]--;
  }
  if(qid(TP)==2){
    j2t-=({h});
    jatekos2["babuk2"]--;
  }
  settabla();
}

//ob-nak van-e aktiv malma
int qmalom3(object ob){
  if(qid(ob)==1) return jatekos1["malom"];
  if(qid(ob)==2) return jatekos2["malom"];
}

string rare(int h){
  string i;
  switch (h){
    case 1 : i="-es";break;
    case 2 : i="-es";break;
    case 3 : i="-as";break;
    case 4 : i="-es";break;
    case 5 : i="-os";break;
    case 6 : i="-os";break;
    case 7 : i="-es";break;
    case 8 : i="-as";break;
    case 9 : i="-as";break;
    case 10 : i="-es";break;
    case 11 : i="-es";break;
    case 12 : i="-es";break;
    case 13 : i="-as";break;
    case 14 : i="-es";break;
    case 15 : i="-os";break;
    case 16 : i="-os";break;
    case 17 : i="-es";break;
    case 18 : i="-as";break;
    case 19 : i="-es";break;
    case 20 : i="-as";break;
    case 21 : i="-es";break;
    case 22 : i="-es";break;
    case 23 : i="-as";break;
    case 24 : i="-es";break;                        
    default : i="-es";break;
  }
  return i;
}

//ezzel rak babut
int rak(string str){
  int h, h2;
  if(!qid(TP)) return notify_fail("%^BOLD%^%^RED%^Nem is játszol!%^RESET%^\n");
  if(!megyajatek) return notify_fail ("%^BOLD%^%^RED%^A játék még nem indult el!%^RESET%^\n");
  if(!qturn(TP)) return notify_fail("%^BOLD%^%^RED%^Nem te jössz!\n%^RESET%^");
  if(!str) return notify_fail("%^BOLD%^%^RED%^Használat: %^WHITE%^rak <hova/honnan> <hova>%^RESET%^\n");
  if(sscanf(str, "%d %d", h, h2) != 2 && sscanf(str, "%d", h) != 1) return notify_fail("%^BOLD%^%^RED%^Használat: %^WHITE%^rak <hova/honnan> <hova>%^RESET%^\n");
  if(h > 24 || h2 > 24) return notify_fail("%^BOLD%^%^RED%^Oda nem rakhatsz!%^RESET%^\n");
  if(qmalom3(TP)) return notify_fail("%^BOLD%^%^RED%^Elõbb vegyél le egy bábut az ellenfeledtõl!\n%^RESET%^");
  //ha meg van a kezeben babu akkor csak az elso parametert hasznalhatja
  if(qbabuk(TP)){
    //hulye user kezeles
    if(h2) return notify_fail("%^BOLD%^%^RED%^Még nem tudsz tologatni, elõbb fogyjanak el a bábúid a kezedbõl!\n%^RESET%^");
    if(qt(h)) return notify_fail("%^BOLD%^%^RED%^Az a mezõ már foglalt!%^RESET%^\n");
    remove_call_out("kov_egy");
    remove_call_out("kov_ketto");
    if(qid(TP)==1){
      tell_room(TO,"%^BOLD%^%^GREEN%^"+jatekos1["nev"]+"%^YELLOW%^ rakott a %^WHITE%^"+h+"%^YELLOW%^"+rare(h)+" mezõre.\n%^RESET%^");
      holder_1=0;
    }
    if(qid(TP)==2){
      tell_room(TO,"%^BOLD%^%^RED%^"+jatekos2["nev"]+"%^YELLOW%^ rakott a %^WHITE%^"+h+"%^YELLOW%^"+rare(h)+" mezõre.\n%^RESET%^");
      holder_2=0;
    }
    rakott(TP,h);
    mbabuk(TP);
//    settabla();
//    tell_room(TO,tabla);
    //van-e malom
    if(qmalom2(h)){
      tell_room(TO,"%^BOLD%^%^RED%^Malom!%^RESET%^\n");
      //egyatalan lehet-e babut levenni
      if(!qvanle(TP)){
        write("%^BOLD%^%^RED%^Ellenfeled összes bábuja malomban van, nem tudsz tõle bábut levenni.%^RESET%^\n");
        say("%^BOLD%^%^RED%^"+capitalize(TP->query_name())+" nem tud bábut levenni.%^RESET%^\n");
        kovetkezo(TP);
        settabla();
        tell_room(TO,tabla);
        return 1;
      }
      write("%^BOLD%^%^WHITE%^Levehetsz%^RED%^ egyet ellenfeled bábuiból!%^RESET%^\n");
      say("%^BOLD%^%^RED%^"+capitalize(TP->query_name())+" levehet egy bábut az ellenfelétõl!%^RESET%^\n");
      malom(TP);
      settabla();
      tell_room(TO,tabla);
      return 1;
    }
    kovetkezo(TP);
    settabla();
    tell_room(TO,tabla);
    return 1;
  }
  //ha a kezebol mar elfogytak a babuk
  //hulye user kezeles
  if(!h2) return notify_fail("%^BOLD%^%^RED%^Már elfogytak a kezedbõl a bábuk, helyes használat: %^WHITE%^rak <honnan> <hova>%^RESET%^\n");
  if(!qt(h)) return notify_fail("%^BOLD%^%^RED%^Az a mezõ üres!%^RESET%^\n");
  if(qt(h2)) return notify_fail("%^BOLD%^%^RED%^Az a mezõ már foglalt!%^RESET%^\n");
  if(qt(h)!=qid(TP)) return notify_fail("%^BOLD%^%^RED%^A saját bábuddal mozogj!%^RESET%^\n");
  if(qbabuk2(TP)>3 && !szomszed(h,h2)) return notify_fail("%^BOLD%^%^RED%^Még nem tudsz ugrálni!%^RESET%^\n");
  remove_call_out("kov_egy");
  remove_call_out("kov_ketto");
  if(qid(TP)==1){
    tell_room(TO,"%^BOLD%^%^GREEN%^"+jatekos1["nev"]+"%^YELLOW%^ átrakott egy bábut a %^WHITE%^"+h+"%^YELLOW%^"+rare(h)+" mezõrõl a %^WHITE%^"+h2+"%^YELLOW%^"+rare(h2)+" mezõre.\n%^RESET%^\n");
    holder_1=0;
  }
  if(qid(TP)==2){
    tell_room(TO,"%^BOLD%^%^RED%^"+jatekos2["nev"]+"%^YELLOW%^ átrakott egy bábut a %^WHITE%^"+h+"%^YELLOW%^"+rare(h)+" mezõrõl a %^WHITE%^"+h2+"%^YELLOW%^"+rare(h2)+" mezõre.\n%^RESET%^\n");
    holder_2=0;
  }
  levett(TP,h);
  rakott(TP,h2);
//  settabla();
//  tell_room(TO,tabla);
  //van-e malom
  if(qmalom2(h2)){
    tell_room(TO,"%^BOLD%^%^RED%^Malom!%^RESET%^\n");
    //lehet-e levenni babut
    if(!qvanle(TP)){
      write("%^BOLD%^%^RED%^Ellenfeled összes bábuja malomban van, nem tudsz tõle bábut levenni.%^RESET%^\n");
      say("%^BOLD%^%^RED%^"+capitalize(TP->query_name())+" nem tud bábut levenni.%^RESET%^\n");
      kovetkezo(TP);
      settabla();
      tell_room(TO,tabla);
      return 1;
    }
    write("%^BOLD%^%^WHITE%^Levehetsz%^RED%^ egyet ellenfeled bábuiból!%^RESET%^\n");
    say("%^BOLD%^%^RED%^"+capitalize(TP->query_name())+" levehet egy bábut az ellenfelétõl!%^RESET%^\n");
    malom(TP);
    settabla();
    tell_room(TO,tabla);
    return 1;
  }
  kovetkezo(TP);
  settabla();
  tell_room(TO,tabla);
  return 1;
}

//levesz parancs, aktiv malomnal
int levesz(string s){
  int h;
  if(!qid(TP)) return notify_fail("%^BOLD%^%^RED%^Te nem vagy játékban!%^RESET%^\n");
  if(!megyajatek) return notify_fail("%^BOLD%^%^RED%^A játék még nem kezdõdött el!%^RESET%^\n");
  if(!qmalom3(TP)) return notify_fail("%^BOLD%^%^RED%^Nincs is malmod!%^RESET%^\n");
  if(!s) return notify_fail("%^BOLD%^%^RED%^Használat: %^WHITE%^levesz <honnan>%^RESET%^\n");
  if(sscanf(s,"%d",h)!=1) return notify_fail("%^BOLD%^%^RED%^Használat: %^WHITE%^levesz <honnan>%^RESET%^\n");
  if(!qt(h)) return notify_fail("%^BOLD%^%^RED%^Üres mezõrõl nem tudsz bábut levenni!%^RESET%^\n");
  if(qt(h)==qid(TP)) return notify_fail("%^BOLD%^%^RED%^Az ellenfél bábuival probálkozz..%^RESET%^\n");
  if(qmalom2(h)) return notify_fail("%^BOLD%^%^RED%^Az a bábu malomban van, nem tudod levenni!%^RESET%^\n");
  write("%^BOLD%^%^YELLOW%^Leveszed a bábut a %^WHITE%^"+h+"%^YELLOW%^"+rare(h)+" mezõrõl.%^RESET%^\n");
  leut(TP,h);
  if(!megyajatek){
    tell_room(TO,"%^BOLD%^%^YELLOW%^"+capitalize(TP->query_name())+" megnyerte a játékot!%^RESET%^\n");
    if(J_MESTER) CHAT_D->do_chat("jatek",capitalize(TP->query_name())+" nyert a malom játékban!",1);
    //tell_room(TO,"%^BOLD%^%^YELLOW%^Már lehet jelentkezni új játékra!%^RESET%^");
    return 1;
  }
  if(qid(TP)==1){
    jatekos1["malom"]=0;
    say("%^BOLD%^%^GREEN%^"+jatekos1["nev"]+"%^YELLOW%^ leveszi a bábut a %^WHITE%^"+h+"%^YELLOW%^"+rare(h)+" mezõrõl.%^RESET%^\n");
  }
  if(qid(TP)==2){
    jatekos2["malom"]=0;
    say("%^BOLD%^%^RED%^"+jatekos2["nev"]+"%^YELLOW%^ leveszi a bábut a %^WHITE%^"+h+"%^YELLOW%^"+rare(h)+" mezõrõl.%^RESET%^\n");
  }
  kovetkezo(TP);
  settabla();
  tell_room(TO,tabla);
  return 1;
}


//tablara ranezes
string nez_tabla(){
  if(!megyajatek){
    if(jatekos1["jel"] && jatekos2["jel"]) return "%^BOLD%^%^GREEN%^"+jatekos1["nev"]+"%^YELLOW%^ és %^RED%^"+jatekos2["nev"]+"%^YELLOW%^ jelentkeztek a játékra, de a játék még nem indult el!%^RESET%^";
    if(jatekos1["jel"]) return "%^BOLD%^%^YELLOW%^Még csak %^GREEN%^"+jatekos1["nev"]+"%^YELLOW%^ jelentkezett a játékra!%^RESET%^";
    if(jatekos2["jel"]) return "%^BOLD%^%^YELLOW%^Még csak %^RED%^"+jatekos2["nev"]+"%^YELLOW%^ jelentkezett a játékra!%^RESET%^";
    return "%^BOLD%^%^YELLOW%^Még senki sem jelentkezett a játékra!%^RESET%^";
  }
  settabla();
  return tabla;
}

int felad(){
  if(!qid(TP)) return notify_fail("%^BOLD%^%^RED%^Te nem is játszol!\n%^RESET%^");
  if(!megyajatek) return notify_fail("%^BOLD%^%^RED%^A játék még nem indult el!\n%^RESET%^");
  write("%^BOLD%^%^RED%^Feladod a játékot.\n%^RESET%^");
  say("%^BOLD%^%^RED%^"+capitalize(TP->query_name())+" feladta a játékot!\n%^RESET%^");
  if(qid(TP)==1){
    tell_room(TO,"%^BOLD%^%^YELLOW%^"+jatekos2["nev"]+" megnyerte a játékot!\n%^RESET%^");
    if(J_MESTER) CHAT_D->do_chat("jatek",jatekos2["nev"]+" nyert a malom játékban!",1);
    malom_over();
    return 1;
  }
  tell_room(TO,"%^BOLD%^%^YELLOW%^"+jatekos1["nev"]+" megnyerte a játékot!\n%^RESET%^");
  if(J_MESTER) CHAT_D->do_chat("jatek",jatekos1["nev"]+" nyert a malom játékban!",1);
  malom_over();
  return 1;
}

int kilep_a(){
  if(qid(TP)){
    tell_room(TO,"%^BOLD%^%^RED%^Az egyik játékos kilépett a játékbol. A játéknak vége.\n%^RESET%^",({TP}));
    if(J_MESTER){
      switch(qid(TP)){
        case 1 : CHAT_D->do_chat("jatek",jatekos2["nev"]+" nyert a malom játékban!",1);break;
        case 2 : CHAT_D->do_chat("jatek",jatekos1["nev"]+" nyert a malom játékban!",1);break;
      }
    }
    malom_over();
  }
}
