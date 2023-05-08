# FUSEE A EAU
<p align="center">
<img src="Mise en situation.jpg" alt="Illustration" height=500>


### 1- PROJET
Dans le cadre de la spécialité sciences de l’ingénieur, le projet consiste à suivre les différentes étapes d’ingénierie nécessaires pour la conception d’une fusée à propulsion à eau et air comprimé et de sa base de lancement optimisées afin de répondre à un cahier des charges précis.



### 2- LE CAHIER DES CHARGES
Concevoir une fusée à propulsion à eau similaire aux fusées qui participent au concours [ROCKETRY CHALLENGE](https://www.planete-sciences.org/espace/Rocketry-Challenge/Presentation) organisé par planette science.
  
Dans ce concours, il s'agit de propulser un œuf de poule cru d’une masse comprise entre 55 et 61 grammes et d’un diamètre de 45 mm maximum (orientation libre). 
  
L’œuf doit revenir du vol sans aucune fissure ni dommage externe.
  
Les principaux éléments du règlement et du cahier des charges sont reproduits (et parfois simplifiés) dans le [cahier des charges fonctionnel](FUSEE%20A%20EAU_CDCF%20PROJET.pdf)



### 3 - LES SIMULATIONS :

- Calcul du CX à partir d'une simulation aérodynamique

<img src="simulation fusée V1 25 m.s-1.png" alt="Simulation aerodynamique" height=200>  <img src="Maitre couple (surface frontale).png" alt="maitre couple" height=200> 

- Calcul de l'apogée théorique à partir de simulation Mécanique

<img src="Simulation MECA3D Apogée.png" alt="Simulation mécanique" height=200> <img src="Courbe Force poussée.png" alt="Courbe poussée" height=200> <img src="Courbe poids eau.png" alt="Courbe poid eau" height=200>



### 4 - MATERIEL :
Le système est décomposé en plusieurs sous ensembles :
- La RAMPE de lancement

- Le système de LARGAGE

<img src="Lanceur fermé.jpg" alt="FERME" height=300> <img src="Lanceur ouvert.jpg" alt="OUVERT" height=300> <img src="Largage fermé.png" alt="OUVERT" height=300> <img src="Largage ouvert.png" alt="OUVERT" height=300>

- La VALISE d'automatisation

    Elle est controlée par une carte à microcontroleur Arduino MEGA [programme ARDUINO pour carte MEGA](Fusee_a_eau_Commande_BT_MEGA.ino)
  
    Les différentes consignes de commande peuvent se faire en mode manuel avec 4 boutons poussoirs mais aussi en Bluetooth grâce à une application ANDROID programmé avec [APP INVENTOR](https://gallery.appinventor.mit.edu/?galleryid=ee9647f1-101d-46b4-a1c1-f9438da695b3)

<img src="Valise.jpg" alt="VALISE" height=300>

- la fusée à eau (en cours de fabrication)

<img src="Fusée à eau MECA3D.png" alt="Simulation mécanique de la FUSEE" height=300> <img src="Montage.jpg" alt="Montage de la FUSEE" height=300>



### 5 - LES TESTS :
- Banc de TEST DE RESISTANCE à la pression
  
    Permet de vérifier la tenue à la pression des bouteilles de soda utilisées ( explosion à plus de 12 bars), mais aussi l'expension des dimensions de la fusée pendant la compression.
  
<img src="Banc test pression.jpg" alt="BANC TEST PRESSION" height=300> <img src="Bouteille explosée.jpg" alt="CRASH TEST" height=300>

  
- Banc de TEST DE POUSSEE
  
    Il s'agit d'un banc équipé d'un capteur de FORCE permettant d'évaluer la courbe de poussée de l'étage de propulsion en fonction du volume d'eau et de la pression de l'air.
  
<img src="Banc test poussée.jpg" alt="BANC TEST POUSSEE" height=300> <img src="Banc test poussée2.jpg" alt="BANC TEST POUSSEE2" height=300> <img src="Banc test poussée capteur.jpg" alt="CAPTEUR DE FORCE" height=300>

 [Vidéo test poussée](https://youtube.com/shorts/VaxSK4kgV1k)
 
 
-LES PREMIERS ESSAIS EN EXTERIEUR

  [Premier test](https://youtube.com/shorts/n9TUfji-JI4)
  
  [Largage au ralenti](https://youtube.com/shorts/VSt-5mvT0KY?feature=share)



  

  
