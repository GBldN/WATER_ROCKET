/* ################# PROGRAMME DE COMMANDE DE LA VALISE FUSEE A EAU EN BLUETOOTH #################
 * Ce programme permet de piloter une valise de remplissage, de mise en pression, de commande d'avortement du tir et de commande de larguage pour des fusées à eau. 
 * La valise est équipée de 
 *  - 4 actionneurs :
 *      Relais 1 - Une électrovanne 12V permettant l'AVORTEMENT du tir en libérant la pression dans la fusée
 *      Relais 2 - Une pompe électrique 12V pour le REMPLISSAGE de l'eau dans la fusée
 *      Relais 3 - Un Relais_Compresseur 12V 
 *      Relais 4 - Non utilisé ici
 *      Servomoteur - système de déclenchement du tir basé sur un servomoteur entrainant une crémaillère
 *  - 2 capteurs :
 *      Débitmêtre - Permettant de mesurer le débit d'eau
 *      Préssostat - Capteur de pression capable de mesurer la pression
 *  - 4 boutons : 1 bascule et 3 poussoirs pour la commande manuelle
 *  - 1 carte 4 RELAIS qui communique avec l'arduino en I2C : https://wiki.seeedstudio.com/Grove-4-Channel_SPDT_Relay/
 *  - 1 module bluetooth GROVE V3 pour la communication avec le smartphone https://wiki.seeedstudio.com/Grove-Serial_Bluetooth_v3.0/
 *  - 1 module ECRAN LCD GROVE : https://wiki.seeedstudio.com/Grove-LCD_RGB_Backlight/
 *  
 * Tous les actionneurs, ainsi que le capteur de pression doivent être alimentés avec une tension nominale de 12V hormis le servomoteur qui utilise un module BEC pour être alimenté en 5V.
 * 
 * La CARTE 4 RELAIS utilise une bibliothèque spécifique : https://github.com/Seeed-Studio/Multi_Channel_Relay_Arduino_Library 
 * 
 * Le CAPTEUR DE PRESSION SMC ISE30A-01-E : https://static.smc.eu/binaries/content/assets/smc_global/product-documentation/operation-manuals/fr/zise30a_fr.pdf
 *    - Mesure de pression jusqu'à 10 bars (supporte 15 bars maxi)
 *    - Alimentation VCC entre 12V et 24V
 *    - sortie OUT 1 (non utilisée) est une sortie pnp à collecteur ouvert activée / désactivé à partir de 2 seils de pression qui peut alimenter avec VCC une charge de 80 mA maxi
 *    - sortie OUT 2 (ANALOGIQUE) donne l'image de pression entre 0V et 5V. !ATTENTION! Lorsque la pression est à 0 bars la sortie OUT2 envoie 1V ! Celà permet de mesurer des dépressions
 *    - Il possède un petit écran LCD pour afficher la pression et effectuer les réglages comme les valeurs des seils.
 * 
 * Le CAPTEUR DE DEBIT : GEMS FT-110 : https://www.gemssensors.com/docs/default-source/resource-files/catalog-pages/catalog-f_ft110-series.pdf?Status=Master&sfvrsn=178e9bc2_6
 *    - A effet HALL, possède une turbine et renvoie un signal carré fonction de la rotation d'une turbine à l'intérieur
 *    - 6900 tics / litre
 *    - Se branche sur une entrée interruption ici INT0 (entrée digitale 2)
 *    - Doit servir à mesurer un volume Relais_Compris entre 100 ml et 1000 ml
 * 
 * Ce programme permet donc de communiquer avec le smartphone dans les 2 sens emmission / réception les chois retenus sont : 
 *    - Utilisation d'une variable "etat" qui correspond à l'état actuel du système :
 *        . 00 = NON-CONNECTE 
 *        . 01 = AVORTEMENT 
 *        . 02 = VIDE
 *        . 03 = REMPLISSAGE 
 *        . 04 = REMPLI 
 *        . 05 = MISE EN PRESSION 
 *        . 06 = PRET 
 *        . 07 = TIR EN COURS
 *        . 08 = INITIALISATION
 *        . 09 = MODE MANUEL
 *    - Choix de l'état d'AVORTEMENT PRIORITAIRE si le signal BT n'est plus reçu ou si l'avortement est demandé.
 *    - Choix d'utiliser 2 chaines de caractères pour la communication BT (problème d'asynchronisme de la liaison bluetooth) :
 *        
 *        . Pour la communication sortante : "E##V$$$$P££.£Fin  " (espaces en fin de chaine) pour l'envoie à l'application smartphone
 *            ##   : état actuel du système [2 caractères]
 *            $$$$ : Valeur du volume d'eau rempli en temps réel ( en ml ) [4 caractères]
 *            ££.£ : Valeur de la pression en temps réel ( en bars )       [4 caractères]
 *       
 *       . Pour la communication entrante : "C**V_REG%%%%P_REG§§Fin  " (espaces en fin de chaine) reçue de l'application smartphone
 *            **   : consigne de l'état souhaité [2 caractères]
 *            %%%% : valeur du volume d'eau désiré en ml   [4 caractères]
 *            $$$  : valeur de la pression  désiré en bars [3 caractères]
 *    
 *  Le programme permet aussi la commande avec 4 boutons poussoirs et un sélecteur de mode MANUEL / BT
 *  Et l'affichage de l'activité des actinneurs avec 4 voyants
 *  
 *  Le système dispose aussi d'un écran LCD 16x2 pour l'affichage des informations principales.
 *   
 *  Il est nécessaire de commander l'ouverture de l'électrovanne pendant le remplissage d'eau pour laisser échapper la pression
 *    
 *  L'application smartphone pour la commande est disponible ici : https://gallery.appinventor.mit.edu/?galleryid=ee9647f1-101d-46b4-a1c1-f9438da695b3
 *    
 *    
 */

/* ##### DEFINITION DES MACROS ##### */
   
/* ##### DEFINITION DES CONNEXIONS ##### */
  #define PressionPin             A0                              //Port de connexion du signal analogique du capteur de pression
  #define DebitmetrePin           2                               //Port de connexion du signal A du codeur qui doit être reliée à l'interruption INT0 (digital 2) 
  #define BP_Avort_pin            4                               //Port de connexion du bouton N°1
  #define BP_Rempl_pin            5                               //Port de connexion du bouton N°2
  #define BP_Compr_pin            6                               //Port de connexion du bouton N°3
  #define BP_Lance_pin            7                               //Port de connexion du bouton N°4
  #define Servo_Pin               8                               //Port de connexion du pilotage du servomoteur
  #define LED_Avort_pin           10                              //Port de connexion de la DEL N°1
  #define LED_Rempl_pin           11                              //Port de connexion de la DEL N°2
  #define LED_Compr_pin           12                              //Port de connexion de la DEL N°3
  #define LED_Lance_pin           13                              //Port de connexion de la DEL N°4
  #define SWITCH_ManuBT           A2                              //Port de connexion du sélecteur de mode manuel ou BT
 
    
  
/* ##### PARAMETRES POUR LE MATERIEL ##### */
  #define t_declench              1500                            //Temps de pause après le déclenchement du tir pour laisser le temps au système d'agir...
  #define Relais_Vanne            2                               //Relais de connexion de l'électrovanne
  #define Relais_Pompe            3                               //Relais de connexion de la pompe
  #define Relais_Compr            4                               //Relais de connexion du Compresseur
  #define Relais_Alarm            1                               //Relais de connexion de l'alarme 
  #define Tir_OFF                 180                             //Valeur du servomoteur lorsque le tir n'est pas commandé
  #define Tir_ON                  0                               //Valeur du servomoteur lorsque le tir est commandé
  #define add_eeprom              0                               //1er octect de l'adresse de l'eeprom de l'arduino où stocker les valeurs de l'état, du volume d'eau, et de la pression.

  
/* ##### DEFINITION DES BIBLIOTHEQUES ET DES VARIABLES ##### */
/* -- Intégrations des bibiothèques -- */
  #include "EEPROM.h"                                             //Intégration de la bibliothèque "EEPROM" pour stocker en mémoire l'état et la valeur du volume d'eau
  #include <multi_channel_relay.h>                                //Intégration de la bibliothèque de gestion de la carte 4 relais
  #include <Wire.h>                                               //Intégration de la bibliothèque "wire" pour la gestion de l'I2C
  #include "rgb_lcd.h"                                            //Intégration de la bibliothèque de gestion de l'écran LCD
  #include <Servo.h>                                              //Intégration de la biblitothèque "servo" pour le controle du servomoteur

/* -- Définition des variables pour l'état TOR ou les valeurs numériques des entrées -- */
  String        etat_reel       = "00";                           //état réel du système ("00", "01", "02"...)
  String        etat_connecte   = "00";                           //mémorisation de la connexion
  String        etat_reg        = "00";                           //état demandé par la consigne de l'application smartphone ("01", "02"...)
  volatile long tics            = 0;                              //Variable pour enregister le nombre de tics total
  int           v_reg           = 100;                            //volume réglé en ml
  float         p_reg           = 1;                              //pression réglée en bars
  String        mot             = "";

/* -- Déclaration des variable pour les calculs intermédiares -- */
  int           v_init          = 0;                              //Volume initial enregistré dans l'EPROM
  int           v_reel          = 0;                              //Volume réel rempli calculé à partir du débitmètre en ml
  float         p_reel          = 0;                              //Pression réelle mesurée par le capteur de pression en bars

  char          chaine_recue[60];                                 //Tableau des caractères reçus en bluetooth non synchronisée
  int           longueur;                                         //nombre de caractères lus sur la liaison série
  String        msg_consignes   = "Non connecté";                 //code du message permettant de transmettre les consignes extraite des caractères recus du type "C**V_REG%%%%P_REG§§§Fin  "
  
  unsigned long t0              = 0;                              //Permet de stocker le temps
  unsigned long t_lcd           = 0;                              //Permet de stocker le temps
  unsigned long t2              = 0;                              //Permet de stocker le temps
  unsigned long t_secu          = 0;                              //Permet de stocker le temps

/* -- Déclaration des fonctions utiles au programme -- */
  Multi_Channel_Relay relay;                                      //Initialisation de la fonction Relay
  rgb_lcd lcd;                                                    //Inttialisation de la fonction LCD
  Servo Servo_TIR;                                                //Création de la fonction de gestion du servomoteur


/**************************************************************
      ROUTINE D'INITIALISATION  (exécutée une seule fois)
 **************************************************************/
void setup()
{
/* -- Configuration des broches en ENTREE ou SORTIE -- */

  pinMode(DebitmetrePin,  INPUT_PULLUP);
  pinMode(PressionPin,    INPUT);
  pinMode(BP_Avort_pin,   INPUT_PULLUP);
  pinMode(BP_Rempl_pin,   INPUT_PULLUP);
  pinMode(BP_Compr_pin,   INPUT_PULLUP);
  pinMode(BP_Lance_pin,   INPUT_PULLUP);
  pinMode(SWITCH_ManuBT,  INPUT_PULLUP);
  pinMode(LED_Avort_pin,  OUTPUT);
  pinMode(LED_Rempl_pin,  OUTPUT);
  pinMode(LED_Compr_pin,  OUTPUT);
  pinMode(LED_Lance_pin,  OUTPUT);
  pinMode(Servo_Pin,      OUTPUT);

/* -- configuration des fonctions spéciales nécessaires au programme */  
  Servo_TIR.attach(Servo_Pin);
  Servo_TIR.write(Tir_OFF);                                       //Pilotage du servomoteur en position initiale
  Serial.begin(115200);                                           //Initialisation de la liaison série pour l'affichage de valeurs sur le moniteur série
  Serial1.begin(9600);                                            //Initialisation de la liaison série pour la connexion Bluetooth
  Serial1.setTimeout(200);                                        //Timeout en ms (temps au bout duquel la réception stoppe si aucune donnée n'est reçue)

  relay.begin(0x11);                                              //Initialisation de l'adresse I2C de la carte relais
  relay.channelCtrl(0);                                           //Désactivation des 4 relais

  lcd.begin(16, 2);
  lcd.setRGB(0, 0, 255);

  attachInterrupt(digitalPinToInterrupt(DebitmetrePin), Comptage, RISING);  //Création de l'interruption sur la broche INT0 : D2 qui appelle la fonction Compatage à chaque front montant
  

/* -- affichage du titre du programme sur le LCD --*/
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("- VALISE FUSEE -");
  lcd.setCursor(0, 1);
  lcd.print("-   A EAU BT   -");
  
  setupBlueToothConnection_GROVE();                                //appel de la routine de configuration de liaison série bluetooth (module grove)
  Serial.println("Reglage BT OK");
  delay(1500);                                                     //délais pour la prise en compte des réglages
  Serial.println("delai OK");
  
/* -- Paramétrage des valeurs initiales -- */
/*  EEPROM.get(add_eeprom,etat_reel);                                //Lecture de la valeur de l'état       stockée dans l'eeprom
  EEPROM.get(add_eeprom+10,v_init);                                //Lecture de la valeur du volume d'eau stockée dans l'eeprom
  etat_reel="00";
  Serial.print("lecture EEPROM OK : etat = ");
  Serial.print(etat_reel);
  Serial.print(" - volume = ");
  Serial.print(v_init);
  */  
  t0  = millis();                                                  //Enregistre le temps au démarrage
  t_lcd = millis();                                                 //Enregistre le temps au démarrage
  t2  = millis();                                                  //Enregistre le temps au démarrage
}

/**************************************************************
           BOUCLE PRINCIPALE (exécutée en permanence)
 **************************************************************/
void loop()
{ 
/* -- Lecture des valeurs de entrées -- */
  int an0 = analogRead(PressionPin);                                                              //Lecture de l'entrée analogique pour la mesure de la pression

/* -- Calcul des valeurs  -- */
  v_reel = v_init + tics * 1000 / 6900 ;                                                          //Calcul du volume de remplissage de l'eau
  p_reel = 2.5 * ( float(an0) * 5 / 1023) - 2.5 ;                                                 //Calcul de la pression P=a.U-b, à partir du CAN 10 bits ( variation linéaire 0 bars = 1V et 10 bars = 5V )

/* -- Formatage de la chaine de sortie pour une longueur constante de type E##V$$$$P££.£Fin   -- */
  String v_affich, p_affich;
  if (String(v_reel).length() == 1)  { v_affich = "000" + String(v_reel); }
  if (String(v_reel).length() == 2)  { v_affich = "00" + String(v_reel); }
  if (String(v_reel).length() == 3)  { v_affich = "0" + String(v_reel); }
  if (String(v_reel).length() == 4)  { v_affich = String(v_reel); }
  if (p_reel > 0 and String(p_reel,1) !=  "10.0" ) { p_affich = "0" + String(p_reel,1); }
  else                              { p_affich = String(p_reel,1);}
  String  msg_reel = "E" + etat_reel + "V" + v_affich + "P" +  p_affich + "Fin  ";


/* -- Lecture des informations venant de l'application smartphone en BT ( envoyées sous forme d'une chaine de caractères du type "C**V%%%%P§§§Fin ") -- */
  if( digitalRead(SWITCH_ManuBT) and Serial1.available())                                        //Si mode BT et si un caractère est reçu sur la liaison série
  {
    longueur = Serial1.readBytes(chaine_recue,60);                                                //Lecture des données reçues dans le tableau de caractères "chaine_recue"
    mot = String(chaine_recue);
    if ( ( mot.indexOf("C",0) < mot.indexOf("Fin",mot.indexOf("C",0)) ) )
    {
      etat_connecte = "OK";                                                                       //Passage en mode connecté
      msg_consignes  = mot.substring(mot.indexOf("C",0),mot.indexOf("Fin",mot.indexOf("C",0)));   //Extrait un message valide de la chaine complète
      etat_reg = msg_consignes.substring(1,3);                                                    //Extrait la chaine correspondant à ** pour la consigne des actions souhaitées
      v_reg    = msg_consignes.substring(4,8).toInt();                                            //Extrait les %%%% et transforme le texte en valeur numérique décimale
      p_reg    = msg_consignes.substring(9,12).toFloat();                                         //Extrait les %%%% et transforme le texte en valeur numérique décimale
      t0 = millis();
    }
    else
    {
      msg_consignes = "Non valide";
      if ( ( millis() - t0 ) / 1000 >= 3.0 )                                                      //Si aucun message valide pendant 3s passage en mode non connecté
      {
        etat_connecte = "OFF";
        msg_consignes = "Non connecté";
        msg_reel = "Non connecté";
        etat_reel = "00";
        //t0 = millis();
      }
    }

    /* -- Envoie du message sur la liaison Bluetooth pour l'application smartphone qui doit ressembler à "E##V$$$$P££Fin  "-- */
      Serial1.print("E");
      Serial1.print(etat_reel);
      Serial1.print("V");
      Serial1.print(v_affich);
      Serial1.print("P");
      Serial1.print(p_affich);
      Serial1.print("Fin ");
  }
  else
  {
    if ( ( millis() - t0 ) / 1000 >= 3.0 )                                                  //déconnexion au bout de 3s et passage en mode non connecté
    {
      etat_connecte = "OFF";
      msg_consignes = "Non connecté";
      msg_reel = "Non connecté";
      etat_reel = "00";
    }
  }
  
 if ( digitalRead(SWITCH_ManuBT) == 0)  etat_reel = "08";                                   //Passage en mode manuel

/* -- Gestion de l'avortement en cas d'appui sur le bouton -- */
 if ( digitalRead(BP_Avort_pin) == 0 and digitalRead(SWITCH_ManuBT) )  etat_reel = "01";    //Bouton d'avortement prioritaire en mode BT

/* -- Affichage sur le moniteur série -- */
  Serial.print("ETAT : réel : "); Serial.print(etat_reel);  
  Serial.print(" / consigne : ");  Serial.print(etat_reg);
  Serial.print(" | VOLUME réglé = "); Serial.print(v_reg);  Serial.print(" ml / réel = "); Serial.print(v_reel);  Serial.print(" ml");
  Serial.print("`| PRESSION réglé = "); Serial.print(p_reg,1);  Serial.print(" bars / réelle = "); Serial.print(p_reel,1);  Serial.print(" bars");
  
  Serial.print(" | BT ");
  //Serial.print(" Msg recu : "); Serial.print(chaine_recue);  
  Serial.print(" - Msg_consignes : "); Serial.print(msg_consignes);
  Serial.print(" - Msg envoyé : "); Serial.print(msg_reel);
  Serial.println();

/* -- Affichage sur l'écran LCD tout les 400 ms pour éviter le scintillement -- */
  lcd.setCursor(0,0);
  lcd.print(etat_reel); lcd.print("#P:"); lcd.print(p_reel,1); lcd.print("  ");
  lcd.setCursor(9,0);
  lcd.print("=>");  lcd.print(p_reg,1); lcd.print("bars       ");
  lcd.setCursor(0, 1);
  lcd.print("   V:"); lcd.print(v_reel);  lcd.print("     ");
  lcd.setCursor(9,1);
  lcd.print("=>");  lcd.print(v_reg); lcd.print(" ml         ");
   
  Gestion_ETATS();                                                            //Appel de la routine de gestion des états
}


/********************************************* 
 *          GESTION DES ETATS                *
 *  00 = NON-CONNECTE                        *
 *  01 = AVORTEMENT                          *
 *  02 = VIDE                                *
 *  03 = REMPLISSAGE                         *
 *  04 = REMPLI                              *
 *  05 = MISE EN PRESSION                    *
 *  06 = PRET                                *
 *  07 = TIR EN COURS                        *
 *********************************************/

void Gestion_ETATS(void)
{
if ( etat_reel == "00"  )     //NON CONNECTE
  {
     if ( p_reel > 0.2 )  
     { etat_reel = "01";
       relay.turn_off_channel(Relais_Compr);                  //COMMANDE de désactivation du Relais_Compresseur
       relay.turn_off_channel(Relais_Pompe);                  //COMMANDE de désactivation de la pompe
       relay.turn_off_channel(Relais_Alarm);                    //COMMANDE de désactivation de l'alarme
       Servo_TIR.write(Tir_OFF);                              //Pilotage du servomoteur en position initiale
       
       digitalWrite(LED_Avort_pin, LOW);
       digitalWrite(LED_Rempl_pin, LOW);
       digitalWrite(LED_Compr_pin, LOW);
       digitalWrite(LED_Lance_pin, LOW);
     }
     else
     {
       if (etat_connecte = "OK")  etat_reel ="02";
       if ( v_reel >= v_reg ) etat_reel = "04";               //Passage directement à l'état REMPLI si déjà fait!
     }
  }

  if ( etat_reel == "01" )    //EN COURS D'AVORTEMENT
  {
     relay.turn_off_channel(Relais_Compr);                    //COMMANDE de désactivation du Relais_Compresseur
     relay.turn_off_channel(Relais_Pompe);                    //COMMANDE de désactivation de la pompe
     relay.turn_off_channel(Relais_Alarm);                    //COMMANDE de désactivation de l'alarme
     Servo_TIR.write(Tir_OFF);                                //Pilotage du servomoteur en position initiale
     
     digitalWrite(LED_Avort_pin, HIGH);
     digitalWrite(LED_Rempl_pin, LOW);
     digitalWrite(LED_Compr_pin, LOW);
     digitalWrite(LED_Lance_pin, LOW);
     
     if ( p_reel >= 0 ) relay.turn_on_channel(Relais_Vanne);  //COMMANDE d'activation de l'électrovanne jusqu'à ce qu'il n'y ai plus de pression
     else 
     {
      relay.turn_off_channel(Relais_Vanne);                   //COMMANDE de désactivation de l'électrovanne
      etat_reel = "00";
     }
     
  }
  
  if ( etat_reel == "02" )    //VIDE Pret à remplir
  {
     relay.turn_off_channel(Relais_Compr);                    //COMMANDE de désactivation du Relais_Compresseur
     relay.turn_off_channel(Relais_Pompe);                    //COMMANDE de désactivation de la pompe
     relay.turn_off_channel(Relais_Vanne);                    //COMMANDE de désactivation de l'électrovanne
     relay.turn_off_channel(Relais_Alarm);                    //COMMANDE de désactivation de l'alarme
     Servo_TIR.write(Tir_OFF);                                //Pilotage du servomoteur en position initiale

     digitalWrite(LED_Avort_pin, LOW);
     digitalWrite(LED_Rempl_pin, LOW);
     digitalWrite(LED_Compr_pin, LOW);
     digitalWrite(LED_Lance_pin, LOW);
     if (etat_reg == "03")  etat_reel = "03";                 //Changement d'état si demande
  }
  
  if ( etat_reel == "03"   )                                  //REMPLISSAGE EN COURS
  {
     relay.turn_off_channel(Relais_Compr);                    //COMMANDE de désactivation du Relais_Compresseur
     relay.turn_off_channel(Relais_Alarm);                    //COMMANDE de désactivation de l'alarme
     Servo_TIR.write(Tir_OFF);                                //Pilotage du servomoteur en position initiale

     digitalWrite(LED_Avort_pin, LOW);
     digitalWrite(LED_Rempl_pin, HIGH);
     digitalWrite(LED_Compr_pin, LOW);
     digitalWrite(LED_Lance_pin, LOW);
     
     if ( v_reel < v_reg)                                     //si le volume souhaité n'est pas atteint
     {
      relay.turn_on_channel(Relais_Vanne);                    //COMMANDE d'activation de l'électrovanne
      relay.turn_on_channel(Relais_Pompe);                    //COMMANDE d'activation de la pompe
     }
     else
     {
      relay.turn_off_channel(Relais_Pompe);                   //COMMANDE de désactivation de la pompe
      relay.turn_off_channel(Relais_Vanne);                   //COMMANDE de désactivation de l'électrovanne
      Servo_TIR.write(Tir_OFF);                               //Pilotage du servomoteur en position initiale
      etat_reel = "04";                                       //Passage à l'état REMPLI
     }
  }

  if ( etat_reel == "04"  )   //REMPLI
  {
     relay.turn_off_channel(Relais_Compr);                    //COMMANDE de désactivation du Compresseur
     relay.turn_off_channel(Relais_Pompe);                    //COMMANDE de désactivation de la pompe
     relay.turn_off_channel(Relais_Vanne);                    //COMMANDE de désactivation de l'électrovanne
     relay.turn_off_channel(Relais_Alarm);                    //COMMANDE de désactivation de l'alarme
     Servo_TIR.write(Tir_OFF);                                //Pilotage du servomoteur en position initiale

     digitalWrite(LED_Avort_pin, LOW);
     digitalWrite(LED_Rempl_pin, LOW);
     digitalWrite(LED_Compr_pin, LOW);
     digitalWrite(LED_Lance_pin, LOW);
     
     if (etat_reg == "05")  etat_reel = "05";                 //Changement d'état si demande
  }

  if ( etat_reel == "05"   )                                  //MISE EN PRESSION EN COURS
  {
     if (etat_reg == "01" ) etat_reel="01";                   //Si demande d'avortement
     else
     {
       relay.turn_off_channel(Relais_Vanne);                  //COMMANDE de désactivation de l'électrovanne
       relay.turn_off_channel(Relais_Pompe);                  //COMMANDE de désactivation de la pompe
       relay.turn_on_channel(Relais_Alarm);                   //COMMANDE d'activation de l'alarme
       Servo_TIR.write(Tir_OFF);                              //Pilotage du servomoteur en position initiale

       digitalWrite(LED_Avort_pin, LOW);
       digitalWrite(LED_Rempl_pin, LOW);
       digitalWrite(LED_Compr_pin, HIGH);
       digitalWrite(LED_Lance_pin, LOW);
       
       if ( p_reel <= p_reg)                                  // si le volume souhaité n'est pas atteint
       {
        relay.turn_on_channel(Relais_Compr);                  //COMMANDE d'activation du Relais_Compresseur
        /* Mise en route de l'alarme !!!! */
       }
       else
       {
        relay.turn_off_channel(Relais_Compr);                 //COMMANDE de désactivation du Relais_Compresseur
        etat_reel = "06";                                     //Passage à l'état PRET AU TIR
       }
     } 
  }

  if ( etat_reel == "06"  )   //PRET AU TIR
  {
     if (etat_reg == "01" ) etat_reel="01";                   //demande d'avortement
     else
     {
       relay.turn_off_channel(Relais_Compr);                  //COMMANDE de désactivation du Relais_Compresseur
       relay.turn_off_channel(Relais_Pompe);                  //COMMANDE de désactivation de la pompe
       relay.turn_off_channel(Relais_Vanne);                  //COMMANDE de désactivation de l'électrovanne
       relay.turn_on_channel(Relais_Alarm);                   //COMMANDE d'activation de l'alarme
       Servo_TIR.write(Tir_OFF);                              //Pilotage du servomoteur en position initiale
       
       digitalWrite(LED_Avort_pin, LOW);
       digitalWrite(LED_Rempl_pin, LOW);
       digitalWrite(LED_Compr_pin, LOW);
       digitalWrite(LED_Lance_pin, LOW);
       
       if (etat_reg == "07")  
       { 
        etat_reel = "07";                                     //Changement d'état si demande
        t2  = millis();
       }
     }
  }

  if ( etat_reel == "07"   )  //TIR EN COURS
  {
     relay.turn_off_channel(Relais_Compr);                    //COMMANDE de désactivation du esseur
     relay.turn_off_channel(Relais_Pompe);                    //COMMANDE de désactivation de la pompe
     relay.turn_off_channel(Relais_Vanne);                    //COMMANDE de désactivation de l'électrovanne
     delay(1000);
     Servo_TIR.write(Tir_ON);                                 //Pilotage du servomoteur en position de lancement du tir
     
     digitalWrite(LED_Avort_pin, LOW);
     digitalWrite(LED_Rempl_pin, LOW);
     digitalWrite(LED_Compr_pin, LOW);
     digitalWrite(LED_Lance_pin, HIGH);
     
     if ( (millis() - t2) > t_declench)                       //Création d'un délai avant la remise à zero pour laisser un temps d'ouverture du système de largage.
     {
      tics = 0;
      etat_reel = "01";
      relay.turn_off_channel(Relais_Alarm);                    //COMMANDE de désactivation de l'alarme
     }
  }
  
  if ( etat_reel == "08" )    //MODE MANUEL
  {
      //Avortement prioritaire 
      if ( digitalRead(BP_Avort_pin) == 0 )
      { 
        if ( digitalRead(BP_Lance_pin) == 0 )  v_reel = 0;      //Initialisation du volume d'eau réel en cas d'appui sur les 2 boutons Avortemment et TIR
        relay.turn_on_channel(Relais_Vanne);
        digitalWrite(LED_Avort_pin, HIGH);
      }
      else                                  
      { 
        relay.turn_off_channel (Relais_Vanne);
        digitalWrite(LED_Avort_pin, LOW);

        //Gestion du remplissage si la pression est inférieure à 0.5 bars
        if ( digitalRead(BP_Rempl_pin) == 0 and p_reel < 0.5 )
        { 
          relay.turn_on_channel (Relais_Pompe);
          digitalWrite(LED_Rempl_pin, HIGH);
        }
        else                                  
        { 
          relay.turn_off_channel (Relais_Pompe);
          digitalWrite(LED_Rempl_pin, LOW);
        }

        //Gestion de la pression jusqu'à 6 bars maxi
        if ( digitalRead(BP_Compr_pin) == 0 and p_reel < 6 ) 
        { 
          relay.turn_on_channel(Relais_Compr);
          digitalWrite(LED_Compr_pin, HIGH);
        }
        else  
        { 
          relay.turn_off_channel(Relais_Compr);
          digitalWrite(LED_Compr_pin, LOW);
        }
        
        //Gestion du tir si la pression est inférieure à 1.5 bars ou que le bouton reste appuyé plus de 5 secondes
        if ( digitalRead(BP_Lance_pin) == 0  ) 
        { 
          if ( p_reel < 1.5 or ( p_reel >=1.5 and ( millis() - t_secu > 5000 ) ) )     
          {
            Servo_TIR.write(Tir_ON);
            digitalWrite(LED_Lance_pin, HIGH);
          }
        }
        else
        { 
          Servo_TIR.write(Tir_OFF);
          digitalWrite(LED_Lance_pin, LOW);
          t_secu = millis();
        }

        
      }


      
        
  }
  
  
/* -- Enregistrement des valeurs dans l'EEPROM si différentes -- */
/*  String test1;
  EEPROM.get(add_eeprom,test1);
  if ( test1 != etat_reel )  EEPROM.put(add_eeprom,etat_reel);     //Ectriture de la valeur de l'état       dans l'eeprom
  int    test2;
  EEPROM.get(add_eeprom+10,test2);
  if ( test2 != v_reel )     EEPROM.put(add_eeprom+10,v_reel);     //Ectriture de la valeur de l'état       dans l'eeprom
*/
}




/********************************************* 
 *  ROUTINE APPELEE LORS DE L'interruption   *
 ********************************************/
void Comptage(void)
{
  tics++; //Incrémente la variable
}

/********************************************* 
 *  ROUTINE DE CONFIGURATION DU BLUETOOTH    *
 *********************************************/
void setupBlueToothConnection_GROVE()
{   
  Serial1.begin(9600);                  //Réglage de la vitesse de transmission
  Serial1.print("AT");                  //Démarrage des commandes AT (possible uniquement quand le bluetooth est déconnecté)
  delay(400);                           //Temporisation nécessaire pour les commandes AT
  Serial1.print("AT+AUTH1");            //Commande AT d'activation de l'autentification           
  delay(400);
  Serial1.print("AT+NAMEFUSEE_A_EAU");  //Commande AT de changement de nom AT+NAME suivi du nom sans espace
  delay(400);
  }
