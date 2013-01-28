#ifndef _amoba_dirs_
#define _amoba_dirs_

#define CAP capitalize
#define BASE_PATH "/realms/slocum/amoba/"
#define ROOM_PATH BASE_PATH + "rooms/"
#define MONS_PATH BASE_PATH + "mons/"
#define ASCII BASE_PATH + "ascii/"
#define ITEM_PATH BASE_PATH + "items/"

#define STATFILE BASE_PATH + "AMOBASTAT"

#define MSG(message) Debug(message, "slocum") ;
#define ERR(message) {write(""message"");return 1;}

#endif
